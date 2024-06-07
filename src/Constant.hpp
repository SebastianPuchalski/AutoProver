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

    virtual std::shared_ptr<Proposition> copy() const;

    virtual void getVariableIds(std::vector<int>& variableIds) const;
    virtual uint64 evaluate(const std::vector<uint64>& varValues) const;

private:
    Value value;

    virtual std::shared_ptr<Proposition> transformXnorToImp();
    virtual std::shared_ptr<Proposition> transformImpToOr();
    virtual std::shared_ptr<Proposition> eliminateDoubleNot(bool& anyChange);
    virtual std::shared_ptr<Proposition> moveNotInwardsOp(int binaryOp, bool& anyChange);
    virtual std::shared_ptr<Proposition> distributeOrAnd(bool orOverAnd, bool& anyChange);
    virtual std::shared_ptr<Proposition> reduce(bool& anyChange);
};
