#include "Proposition.hpp"

#include "BinaryOperator.hpp"

PropositionSP Proposition::toCnf() const {
    return toNormalForm(true);
}

PropositionSP Proposition::toDnf() const {
    return toNormalForm(false);
}

PropositionSP Proposition::toNormalForm(bool cnf) const {
    auto proposition = copy();
    PropositionSP newProposition;

    newProposition = proposition->transformXnorToImp();
    if (newProposition)
        proposition = newProposition;

    newProposition = proposition->transformImpToOr();
    if (newProposition)
        proposition = newProposition;

    bool anyChange = true;
    while (anyChange) {
        anyChange = false;

        newProposition = proposition->eliminateDoubleNot(anyChange);
        if (newProposition)
            proposition = newProposition;

        newProposition = proposition->moveNotInwardsOp(BinaryOperator::AND, anyChange);
        if (newProposition)
            proposition = newProposition;

        newProposition = proposition->moveNotInwardsOp(BinaryOperator::OR, anyChange);
        if (newProposition)
            proposition = newProposition;
    }

    anyChange = true;
    while (anyChange) {
        anyChange = false;

        newProposition = proposition->distributeOrAnd(cnf, anyChange);
        if (newProposition)
            proposition = newProposition;

        /*bool anyChangeR = true;
        while (anyChangeR) {
            anyChangeR = false;
            newProposition = proposition->reduce(anyChangeR);
            if (newProposition)
                proposition = newProposition;
        }*/
    }

    return proposition;
}
