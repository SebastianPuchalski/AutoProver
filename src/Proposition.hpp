#pragma once

#include "common.hpp"	

#include <string>
#include <memory>

class Proposition {
public:
    virtual ~Proposition() = default;

    enum Type {VARIABLE, CONSTANT, UNARY, BINARY};
    virtual Type getType() const = 0;
};

class Variable : public Proposition {
public:
    explicit Variable(int id) : id(id) {}

    virtual Type getType() const {
        return VARIABLE;
    }

    int getId() const {
        return id;
    }

private:
    int id;
};

class Constant : public Proposition {
public:
    /*
    0: FALSE
    1: TRUE
    */
    enum Value {FALSE = 0, TRUE};

    Constant(Value value) : value(value) {}

    virtual Type getType() const {
        return CONSTANT;
    }

    Value getValue() const {
        return value;
    }

    void setValue(Value value) {
        this->value = value;
    }

private:
    Value value;
};

class UnaryOperator : public Proposition {
public:
    /*
    A=0, A=1
    00: FALSE
    01: TRANSFER
    10: NOT
    11: TRUE
    */
    enum Op {FALSE = 0, TRANSFER, NOT, TRUE};

    UnaryOperator(std::shared_ptr<Proposition> operand, Op op = NOT) :
        operand(operand), op(op) {}

    virtual Type getType() const {
        return UNARY;
    }

    std::shared_ptr<Proposition> getOperand() const {
        return operand;
    }

    Op getOp() const {
        return op;
    }

    void setOperand(std::shared_ptr<Proposition> operand) {
        this->operand = operand;
    }

    void setOp(Op op) {
        this->op = op;
    }

private:
    std::shared_ptr<Proposition> operand;
    Op op;
};

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
    enum Op {FALSE, AND, NIMP, A, NRIMP, B, XOR, OR, NOR, XNOR, NB, RIMP, NA, IMP, NAND, TRUE};

    BinaryOperator(std::shared_ptr<Proposition> left, Op op, std::shared_ptr<Proposition> right) :
        left(left), op(op), right(right) {}

    virtual Type getType() const {
        return BINARY;
    }

    std::shared_ptr<Proposition> getLeft() const {
        return left;
    }

    std::shared_ptr<Proposition> getRight() const {
        return right;
    }

    Op getOp() const {
        return op;
    }

    void setLeft(std::shared_ptr<Proposition> left) {
        this->left = left;
    }

    void setRight(std::shared_ptr<Proposition> right) {
        this->right = right;
    }

    void setOp(Op op) {
        this->op = op;
    }

private:
    std::shared_ptr<Proposition> left;
    std::shared_ptr<Proposition> right;
    Op op;
};
