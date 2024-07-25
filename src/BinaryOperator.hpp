#pragma once

#include "Proposition.hpp"

class BinaryOperator : public Proposition {
public:
    /*
    (A=0, B=0), (A=0, B=1), (A=1, B=0), (A=1, B=1)
    0000: FALSE
    0001: AND
    0010: NIMP, ~(A -> B)
    0011: A
    0100: NRIMP, ~(B -> A)
    0101: B
    0110: XOR
    0111: OR
    1000: NOR
    1001: XNOR, A <-> B
    1010: NB, ~B
    1011: RIMP, B -> A
    1100: NA, ~A
    1101: IMP, A -> B
    1110: NAND
    1111: TRUE
    */
    enum Op {
        FALSE, AND, NIMP, A, NRIMP, B, XOR, OR,
        NOR, XNOR, NB, RIMP, NA, IMP, NAND, TRUE
    };

    BinaryOperator(PropositionSP left, Op op,
        PropositionSP right);

    virtual Type getType() const;
    PropositionSP getLeft() const;
    PropositionSP getRight() const;
    Op getOp() const;
    void setLeft(PropositionSP left);
    void setRight(PropositionSP right);
    void setOp(Op op);

    virtual bool isEquivalent(PropositionSP proposition) const;
    virtual PropositionSP copy() const;

    virtual int getLength() const;
    virtual void getVariableIds(std::vector<int>& variableIds) const;
    virtual uint64_t evaluate(const std::vector<uint64_t>& varValues) const;
    bool isCommutative() const;
    bool isAssociative() const;

private:
    PropositionSP left;
    PropositionSP right;
    Op op;

    virtual PropositionSP transformXnorToImp();
    virtual PropositionSP transformImpToOr();
    virtual PropositionSP eliminateDoubleNot(bool& anyChange);
    virtual PropositionSP moveNotInwardsOp(int binaryOp, bool& anyChange);
    virtual PropositionSP distributeOrAnd(bool orOverAnd, bool& anyChange);
    virtual PropositionSP reduce(bool& anyChange);
};
