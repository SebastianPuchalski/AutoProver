#include "BinaryOperator.hpp"

#include "UnaryOperator.hpp"

BinaryOperator::BinaryOperator(std::shared_ptr<Proposition> left, Op op,
    std::shared_ptr<Proposition> right) :
    left(left), op(op), right(right) {}

BinaryOperator::Type BinaryOperator::getType() const {
    return BINARY;
}

std::shared_ptr<Proposition> BinaryOperator::getLeft() const {
    return left;
}

std::shared_ptr<Proposition> BinaryOperator::getRight() const {
    return right;
}

BinaryOperator::Op BinaryOperator::getOp() const {
    return op;
}

void BinaryOperator::setLeft(std::shared_ptr<Proposition> left) {
    this->left = left;
}

void BinaryOperator::setRight(std::shared_ptr<Proposition> right) {
    this->right = right;
}

void BinaryOperator::setOp(Op op) {
    this->op = op;
}

std::shared_ptr<Proposition> BinaryOperator::copy() const {
    return std::make_shared<BinaryOperator>(left->copy(), op, right->copy());
}

bool BinaryOperator::isCommutative() const {
    return op == FALSE || op == AND || op == XOR || op == OR ||
        op == NOR || op == XNOR || op == NAND || op == TRUE;
}

bool BinaryOperator::isAssociative() const {
    return op == FALSE || op == AND || op == A || op == B ||
        op == XOR || op == OR || op == XNOR || op == TRUE;
}

std::shared_ptr<Proposition> BinaryOperator::transformXnorToImp() {
    auto transLeft = left->transformXnorToImp();
    auto transRight = right->transformXnorToImp();
    if (transLeft)
        left = transLeft;
    if (transRight)
        right = transRight;

    if (op == XNOR) {
        auto l = std::make_shared<BinaryOperator>(left, IMP, right);
        auto r = std::make_shared<BinaryOperator>(right->copy(), IMP, left->copy());
        return std::make_shared<BinaryOperator>(l, AND, r);
    }
    return nullptr;
}

std::shared_ptr<Proposition> BinaryOperator::transformImpToOr() {
    auto transLeft = left->transformImpToOr();
    auto transRight = right->transformImpToOr();
    if (transLeft)
        left = transLeft;
    if (transRight)
        right = transRight;

    if (op == IMP) {
        auto negLeft = std::make_shared<UnaryOperator>(left, UnaryOperator::NOT);
        return std::make_shared<BinaryOperator>(negLeft, OR, right);
    }
    return nullptr;
}

std::shared_ptr<Proposition> BinaryOperator::eliminateDoubleNot(bool& anyChange) {
    auto elimLeft = left->eliminateDoubleNot(anyChange);
    auto elimRight = right->eliminateDoubleNot(anyChange);
    if (elimLeft)
        left = elimLeft;
    if (elimRight)
        right = elimRight;
    return nullptr;
}

std::shared_ptr<Proposition> BinaryOperator::moveNotInwardsOp(int binaryOp, bool& anyChange) {
    auto movedLeft = left->moveNotInwardsOp(binaryOp, anyChange);
    auto movedRight = right->moveNotInwardsOp(binaryOp, anyChange);
    if (movedLeft)
        left = movedLeft;
    if (movedRight)
        right = movedRight;
    return nullptr;
}

std::shared_ptr<Proposition> BinaryOperator::distributeOrAnd(bool orOverAnd, bool& anyChange) {
    Op over, under;
    if (orOverAnd) {
        over = OR;
        under = AND;
    }
    else {
        over = AND;
        under = OR;
    }
    if (op == over) {
        if (right->getType() == BINARY) {
            auto binaryRight = std::static_pointer_cast<BinaryOperator>(right);
            if (binaryRight->getOp() == under) {
                anyChange = true;
                std::shared_ptr<Proposition> l = std::make_shared<BinaryOperator>(left, over, binaryRight->left);
                std::shared_ptr<Proposition> r = std::make_shared<BinaryOperator>(left->copy(), over, binaryRight->right);
                auto distLeft = l->distributeOrAnd(orOverAnd, anyChange);
                if (distLeft)
                    l = distLeft;
                auto distRight = r->distributeOrAnd(orOverAnd, anyChange);
                if (distRight)
                    r = distRight;
                return std::make_shared<BinaryOperator>(l, under, r);
            }
        }
        if (left->getType() == BINARY) {
            auto binaryLeft = std::static_pointer_cast<BinaryOperator>(left);
            if (binaryLeft->getOp() == under) {
                anyChange = true;
                std::shared_ptr<Proposition> l = std::make_shared<BinaryOperator>(binaryLeft->left, over, right);
                std::shared_ptr<Proposition> r = std::make_shared<BinaryOperator>(binaryLeft->right, over, right->copy());
                auto distLeft = l->distributeOrAnd(orOverAnd, anyChange);
                if (distLeft)
                    l = distLeft;
                auto distRight = r->distributeOrAnd(orOverAnd, anyChange);
                if (distRight)
                    r = distRight;
                return std::make_shared<BinaryOperator>(l, under, r);
            }
        }
    }

    auto distLeft = left->distributeOrAnd(orOverAnd, anyChange);
    auto distRight = right->distributeOrAnd(orOverAnd, anyChange);
    if (distLeft)
        left = distLeft;
    if (distRight)
        right = distRight;
    return nullptr;
}

// (((a & b) & c) | ((d & e) & f)) <-> (((((a | d) & (b | d)) & (c | d)) & (((a | e) & (b | e)) & (c | e))) & (((a | f) & (b | f)) & (c | f)))