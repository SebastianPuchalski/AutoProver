#pragma once

#include "Proposition.hpp"

class UnaryOperator : public Proposition {
public:
    /*
    A=0, A=1
    00: FALSE
    01: TRANSFER
    10: NOT
    11: TRUE
    */
    enum Op { FALSE = 0, TRANSFER, NOT, TRUE };

    UnaryOperator(PropositionSP operand, Op op = NOT);

    virtual Type getType() const;
    PropositionSP getOperand() const;
    Op getOp() const;
    void setOperand(PropositionSP operand);
    void setOp(Op op);

    virtual bool isEquivalent(PropositionSP proposition) const;
    virtual PropositionSP copy() const;

    virtual int getLength() const;
    virtual void getVariableIds(std::vector<int>& variableIds) const;
    virtual uint64_t evaluate(const std::vector<uint64_t>& varValues) const;

private:
    PropositionSP operand;
    Op op;

    virtual PropositionSP transformXnorToImp();
    virtual PropositionSP transformImpToOr();
    virtual PropositionSP eliminateDoubleNot(bool& anyChange);
    virtual PropositionSP moveNotInwardsOp(int binaryOp, bool& anyChange);
    virtual PropositionSP distributeOrAnd(bool orOverAnd, bool& anyChange);
    virtual PropositionSP reduce(bool& anyChange);
};
