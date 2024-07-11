#include "UnaryOperator.hpp"

#include "BinaryOperator.hpp"

#include <cassert>

UnaryOperator::UnaryOperator(PropositionSP operand, Op op) :
    operand(operand), op(op) {}

UnaryOperator::Type UnaryOperator::getType() const {
    return UNARY;
}

PropositionSP UnaryOperator::getOperand() const {
    return operand;
}

UnaryOperator::Op UnaryOperator::getOp() const {
    return op;
}

void UnaryOperator::setOperand(PropositionSP operand) {
    this->operand = operand;
}

void UnaryOperator::setOp(Op op) {
    this->op = op;
}

bool UnaryOperator::isEquivalent(PropositionSP proposition) const {
    if (!proposition || proposition->getType() != Proposition::UNARY)
        return false;
    auto prop = std::static_pointer_cast<UnaryOperator>(proposition);
    if (op != prop->op)
        return false;
    return operand->isEquivalent(prop->operand);
}

PropositionSP UnaryOperator::copy() const {
    return std::make_shared<UnaryOperator>(operand->copy(), op);
}

int UnaryOperator::getLength() const {
    return operand->getLength() + 1;
}

void UnaryOperator::getVariableIds(std::vector<int>& variableIds) const {
    operand->getVariableIds(variableIds);
}

uint64_t UnaryOperator::evaluate(const std::vector<uint64_t>& varValues) const {
    uint64_t a = operand->evaluate(varValues);
    switch (op) {
    case UnaryOperator::FALSE:
        return 0;
    case UnaryOperator::TRANSFER:
        return a;
    case UnaryOperator::NOT:
        return ~a;
    case UnaryOperator::TRUE:
        return ULLONG_MAX;
    default:
        assert(!"Unsupported operation");
    }
    return 0;
}

PropositionSP UnaryOperator::transformXnorToImp() {
    auto transOperand = operand->transformXnorToImp();
    if (transOperand)
        operand = transOperand;
    return nullptr;
}

PropositionSP UnaryOperator::transformImpToOr() {
    auto transOperand = operand->transformImpToOr();
    if (transOperand)
        operand = transOperand;
    return nullptr;
}

PropositionSP UnaryOperator::eliminateDoubleNot(bool& anyChange) {
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

PropositionSP UnaryOperator::moveNotInwardsOp(int binaryOp, bool& anyChange) {
    assert(binaryOp == BinaryOperator::AND || binaryOp == BinaryOperator::OR);
    if (op == NOT && operand->getType() == BINARY) {
        auto binaryOperand = std::static_pointer_cast<BinaryOperator>(operand);
        if (binaryOperand->getOp() == binaryOp) {
            anyChange = true;
            if (binaryOp == BinaryOperator::AND)
                binaryOperand->setOp(BinaryOperator::OR);
            if (binaryOp == BinaryOperator::OR)
                binaryOperand->setOp(BinaryOperator::AND);
            PropositionSP left = std::make_shared<UnaryOperator>(binaryOperand->getLeft(), NOT);
            PropositionSP right = std::make_shared<UnaryOperator>(binaryOperand->getRight(), NOT);
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

PropositionSP UnaryOperator::distributeOrAnd(bool orOverAnd, bool& anyChange) {
    auto distOperand = operand->distributeOrAnd(orOverAnd, anyChange);
    if (distOperand)
        operand = distOperand;
    return nullptr;
}

PropositionSP UnaryOperator::reduce(bool& anyChange) {
    auto reducedOperand = operand->reduce(anyChange);
    if (reducedOperand)
        operand = reducedOperand;
    return nullptr;
}
