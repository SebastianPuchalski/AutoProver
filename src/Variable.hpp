#pragma once

#include "Proposition.hpp"

class Variable : public Proposition {
public:
    explicit Variable(int id);

    virtual Type getType() const;
    int getId() const;
    void setId(int id);

    virtual std::shared_ptr<Proposition> copy() const;

    virtual void getVariableIds(std::vector<int>& variableIds) const;
    virtual uint64_t evaluate(const std::vector<uint64_t>& varValues) const;

private:
    int id;

    virtual std::shared_ptr<Proposition> transformXnorToImp();
    virtual std::shared_ptr<Proposition> transformImpToOr();
    virtual std::shared_ptr<Proposition> eliminateDoubleNot(bool& anyChange);
    virtual std::shared_ptr<Proposition> moveNotInwardsOp(int binaryOp, bool& anyChange);
    virtual std::shared_ptr<Proposition> distributeOrAnd(bool orOverAnd, bool& anyChange);
    virtual std::shared_ptr<Proposition> reduce(bool& anyChange);
};
