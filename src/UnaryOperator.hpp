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

    UnaryOperator(std::shared_ptr<Proposition> operand, Op op = NOT);

    virtual Type getType() const;
    std::shared_ptr<Proposition> getOperand() const;
    Op getOp() const;
    void setOperand(std::shared_ptr<Proposition> operand);
    void setOp(Op op);

    virtual std::shared_ptr<Proposition> copy() const;

    virtual void getVariableIds(std::vector<int>& variableIds) const;
    virtual uint64_t evaluate(const std::vector<uint64_t>& varValues) const;

private:
    std::shared_ptr<Proposition> operand;
    Op op;

    virtual std::shared_ptr<Proposition> transformXnorToImp();
    virtual std::shared_ptr<Proposition> transformImpToOr();
    virtual std::shared_ptr<Proposition> eliminateDoubleNot(bool& anyChange);
    virtual std::shared_ptr<Proposition> moveNotInwardsOp(int binaryOp, bool& anyChange);
    virtual std::shared_ptr<Proposition> distributeOrAnd(bool orOverAnd, bool& anyChange);
    virtual std::shared_ptr<Proposition> reduce(bool& anyChange);
};
