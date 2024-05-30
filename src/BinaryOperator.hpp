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
    1001: XNOR
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

    BinaryOperator(std::shared_ptr<Proposition> left, Op op,
        std::shared_ptr<Proposition> right);

    virtual Type getType() const;
    std::shared_ptr<Proposition> getLeft() const;
    std::shared_ptr<Proposition> getRight() const;
    Op getOp() const;
    void setLeft(std::shared_ptr<Proposition> left);
    void setRight(std::shared_ptr<Proposition> right);
    void setOp(Op op);

    virtual std::shared_ptr<Proposition> copy() const;
    bool isCommutative() const;
    bool isAssociative() const;

private:
    std::shared_ptr<Proposition> left;
    std::shared_ptr<Proposition> right;
    Op op;

    virtual std::shared_ptr<Proposition> transformXnorToImp();
    virtual std::shared_ptr<Proposition> transformImpToOr();
    virtual std::shared_ptr<Proposition> eliminateDoubleNot(bool& anyChange);
    virtual std::shared_ptr<Proposition> moveNotInwardsOp(int binaryOp, bool& anyChange);
    virtual std::shared_ptr<Proposition> distributeOrAnd(bool orOverAnd, bool& anyChange);
};
