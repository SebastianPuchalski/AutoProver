#pragma once

#include "Proposition.hpp"

class Variable : public Proposition {
public:
    explicit Variable(int id);

    virtual Type getType() const;
    int getId() const;
    void setId(int id);

    virtual bool isEquivalent(PropositionSP proposition) const;
    virtual PropositionSP copy() const;

    virtual int getLength() const;
    virtual void getVariableIds(std::vector<int>& variableIds) const;
    virtual uint64_t evaluate(const std::vector<uint64_t>& varValues) const;

private:
    int id;

    virtual PropositionSP transformXnorToImp();
    virtual PropositionSP transformImpToOr();
    virtual PropositionSP eliminateDoubleNot(bool& anyChange);
    virtual PropositionSP moveNotInwardsOp(int binaryOp, bool& anyChange);
    virtual PropositionSP distributeOrAnd(bool orOverAnd, bool& anyChange);
    virtual PropositionSP reduce(bool& anyChange);
};
