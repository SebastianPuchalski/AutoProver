#include "Proposition.hpp"

#include "BinaryOperator.hpp"

std::shared_ptr<Proposition> Proposition::toCnf() const {
    return toNormalForm(true);
}

std::shared_ptr<Proposition> Proposition::toDnf() const {
    return toNormalForm(false);
}

std::shared_ptr<Proposition> Proposition::toNormalForm(bool cnf) const {
    auto proposition = copy();
    std::shared_ptr<Proposition> newProposition;

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
