#pragma once

#include "Proposition.hpp"

class Constant : public Proposition {
public:
    /*
    0: FALSE
    1: TRUE
    */
    enum Value { FALSE = 0, TRUE };

    Constant(Value value);

    virtual Type getType() const;
    Value getValue() const;
    void setValue(Value value);

    virtual bool isEquivalent(PropositionSP proposition) const;
    virtual PropositionSP copy() const;

    virtual int getLength() const;
    virtual void getVariableIds(std::vector<int>& variableIds) const;
    virtual uint64_t evaluate(const std::vector<uint64_t>& varValues) const;

private:
    Value value;

    virtual PropositionSP transformXnorToImp();
    virtual PropositionSP transformImpToOr();
    virtual PropositionSP eliminateDoubleNot(bool& anyChange);
    virtual PropositionSP moveNotInwardsOp(int binaryOp, bool& anyChange);
    virtual PropositionSP distributeOrAnd(bool orOverAnd, bool& anyChange);
    virtual PropositionSP reduce(bool& anyChange);
};
