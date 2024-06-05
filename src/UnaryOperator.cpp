#include "UnaryOperator.hpp"

#include "BinaryOperator.hpp"

#include <cassert>

UnaryOperator::UnaryOperator(std::shared_ptr<Proposition> operand, Op op) :
    operand(operand), op(op) {}

UnaryOperator::Type UnaryOperator::getType() const {
    return UNARY;
}

std::shared_ptr<Proposition> UnaryOperator::getOperand() const {
    return operand;
}

UnaryOperator::Op UnaryOperator::getOp() const {
    return op;
}

void UnaryOperator::setOperand(std::shared_ptr<Proposition> operand) {
    this->operand = operand;
}

void UnaryOperator::setOp(Op op) {
    this->op = op;
}

std::shared_ptr<Proposition> UnaryOperator::copy() const {
    return std::make_shared<UnaryOperator>(operand->copy(), op);
}

std::shared_ptr<Proposition> UnaryOperator::transformXnorToImp() {
    auto transOperand = operand->transformXnorToImp();
    if (transOperand)
        operand = transOperand;
    return nullptr;
}

std::shared_ptr<Proposition> UnaryOperator::transformImpToOr() {
    auto transOperand = operand->transformImpToOr();
    if (transOperand)
        operand = transOperand;
    return nullptr;
}

std::shared_ptr<Proposition> UnaryOperator::eliminateDoubleNot(bool& anyChange) {
    if (op == NOT && operand->getType() == Proposition::UNARY) {
        auto unaryOperand = std::static_pointer_cast<UnaryOperator>(operand);
        if (unaryOperand->getOp() == NOT) {
            anyChange = true;
            auto newOperator = unaryOperand->operand;
            auto elimNewOperator = newOperator->eliminateDoubleNot(anyChange);
            if (elimNewOperator)
                newOperator = elimNewOperator;
            return newOperator;
        }
    }

    auto transOperand = operand->eliminateDoubleNot(anyChange);
    if (transOperand)
        operand = transOperand;
    return nullptr;
}

std::shared_ptr<Proposition> UnaryOperator::moveNotInwardsOp(int binaryOp, bool& anyChange) {
    assert(binaryOp == BinaryOperator::AND || binaryOp == BinaryOperator::OR);
    if (op == NOT && operand->getType() == BINARY) {
        auto binaryOperand = std::static_pointer_cast<BinaryOperator>(operand);
        if (binaryOperand->getOp() == binaryOp) {
            anyChange = true;
            if (binaryOp == BinaryOperator::AND)
                binaryOperand->setOp(BinaryOperator::OR);
            if (binaryOp == BinaryOperator::OR)
                binaryOperand->setOp(BinaryOperator::AND);
            std::shared_ptr<Proposition> left = std::make_shared<UnaryOperator>(binaryOperand->getLeft(), NOT);
            std::shared_ptr<Proposition> right = std::make_shared<UnaryOperator>(binaryOperand->getRight(), NOT);
            auto movedLeft = left->moveNotInwardsOp(binaryOp, anyChange);
            if (movedLeft)
                left = movedLeft;
            auto movedRight = right->moveNotInwardsOp(binaryOp, anyChange);
            if (movedRight)
                right = movedRight;
            binaryOperand->setLeft(left);
            binaryOperand->setRight(right);
            return operand;
        }
    }

    auto movedOperand = operand->moveNotInwardsOp(binaryOp, anyChange);
    if (movedOperand)
        operand = movedOperand;
    return nullptr;
}

std::shared_ptr<Proposition> UnaryOperator::distributeOrAnd(bool orOverAnd, bool& anyChange) {
    auto distOperand = operand->distributeOrAnd(orOverAnd, anyChange);
    if (distOperand)
        operand = distOperand;
    return nullptr;
}

std::shared_ptr<Proposition> UnaryOperator::reduce(bool& anyChange) {
    auto reducedOperand = operand->reduce(anyChange);
    if (reducedOperand)
        operand = reducedOperand;
    return nullptr;
}
