#include "BinaryOperator.hpp"

#include "Variable.hpp"
#include "Constant.hpp"
#include "UnaryOperator.hpp"

#include <cassert>

BinaryOperator::BinaryOperator(PropositionSP left, Op op,
    PropositionSP right) :
    left(left), op(op), right(right) {}

BinaryOperator::Type BinaryOperator::getType() const {
    return BINARY;
}

PropositionSP BinaryOperator::getLeft() const {
    return left;
}

PropositionSP BinaryOperator::getRight() const {
    return right;
}

BinaryOperator::Op BinaryOperator::getOp() const {
    return op;
}

void BinaryOperator::setLeft(PropositionSP left) {
    this->left = left;
}

void BinaryOperator::setRight(PropositionSP right) {
    this->right = right;
}

void BinaryOperator::setOp(Op op) {
    this->op = op;
}

bool BinaryOperator::isEquivalent(PropositionSP proposition) const {
    if (!proposition || proposition->getType() != Proposition::BINARY)
        return false;
    auto prop = std::static_pointer_cast<BinaryOperator>(proposition);
    if (op != prop->op)
        return false;
    return left->isEquivalent(prop->left) && right->isEquivalent(prop->right);
}

PropositionSP BinaryOperator::copy() const {
    return std::make_shared<BinaryOperator>(left->copy(), op, right->copy());
}

int BinaryOperator::getLength() const {
    return left->getLength() + right->getLength() + 1;
}

void BinaryOperator::getVariableIds(std::vector<int>& variableIds) const {
    left->getVariableIds(variableIds);
    right->getVariableIds(variableIds);
}

uint64_t BinaryOperator::evaluate(const std::vector<uint64_t>& varValues) const {
    uint64_t a = left->evaluate(varValues);
    uint64_t b = right->evaluate(varValues);
    switch (op) {
    case BinaryOperator::FALSE:
        return 0;
    case BinaryOperator::AND:
        return a & b;
    case BinaryOperator::NIMP:
        return a & ~b;
    case BinaryOperator::A:
        return a;
    case BinaryOperator::NRIMP:
        return b & ~a;
    case BinaryOperator::B:
        return b;
    case BinaryOperator::XOR:
        return a ^ b;
    case BinaryOperator::OR:
        return a | b;
    case BinaryOperator::NOR:
        return ~(a | b);
    case BinaryOperator::XNOR:
        return ~(a ^ b);
    case BinaryOperator::NB:
        return ~b;
    case BinaryOperator::RIMP:
        return ~b | a;
    case BinaryOperator::NA:
        return ~a;
    case BinaryOperator::IMP:
        return ~a | b;
    case BinaryOperator::NAND:
        return ~(a & b);
    case BinaryOperator::TRUE:
        return ULLONG_MAX;
    default:
        assert(!"Unsupported operator");
    }
    return 0;
}

bool BinaryOperator::isCommutative() const {
    return op == FALSE || op == AND || op == XOR || op == OR ||
        op == NOR || op == XNOR || op == NAND || op == TRUE;
}

bool BinaryOperator::isAssociative() const {
    return op == FALSE || op == AND || op == A || op == B ||
        op == XOR || op == OR || op == XNOR || op == TRUE;
}

PropositionSP BinaryOperator::transformXnorToImp() {
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

PropositionSP BinaryOperator::transformImpToOr() {
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

PropositionSP BinaryOperator::eliminateDoubleNot(bool& anyChange) {
    auto elimLeft = left->eliminateDoubleNot(anyChange);
    auto elimRight = right->eliminateDoubleNot(anyChange);
    if (elimLeft)
        left = elimLeft;
    if (elimRight)
        right = elimRight;
    return nullptr;
}

PropositionSP BinaryOperator::moveNotInwardsOp(int binaryOp, bool& anyChange) {
    auto movedLeft = left->moveNotInwardsOp(binaryOp, anyChange);
    auto movedRight = right->moveNotInwardsOp(binaryOp, anyChange);
    if (movedLeft)
        left = movedLeft;
    if (movedRight)
        right = movedRight;
    return nullptr;
}

PropositionSP BinaryOperator::distributeOrAnd(bool orOverAnd, bool& anyChange) {
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
                PropositionSP l = std::make_shared<BinaryOperator>(left, over, binaryRight->left);
                PropositionSP r = std::make_shared<BinaryOperator>(left->copy(), over, binaryRight->right);
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
                PropositionSP l = std::make_shared<BinaryOperator>(binaryLeft->left, over, right);
                PropositionSP r = std::make_shared<BinaryOperator>(binaryLeft->right, over, right->copy());
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

PropositionSP BinaryOperator::reduce(bool& anyChange) {
    if (op == AND || op == OR) {
        if (left->getType() == CONSTANT) {
            auto constLeft = std::static_pointer_cast<Constant>(left);
            bool value = constLeft->getValue();
            anyChange = true;
            if (op == AND) {
                if (value)
                    return right;
                else
                    std::make_shared<Constant>(Constant::FALSE);
            }
            else {
                if (value)
                    std::make_shared<Constant>(Constant::TRUE);
                else
                    return right;
            }
        }

        if (right->getType() == CONSTANT) {
            auto constRight = std::static_pointer_cast<Constant>(right);
            bool value = constRight->getValue();
            anyChange = true;
            if (op == AND) {
                if (value)
                    return left;
                else
                    std::make_shared<Constant>(Constant::FALSE);
            }
            else {
                if (value)
                    std::make_shared<Constant>(Constant::TRUE);
                else
                    return left;
            }
        }

        bool leftIsLiteral = false;
        bool leftNeg;
        int leftVarId;
        if (left->getType() == VARIABLE) {
            auto varLeft = std::static_pointer_cast<Variable>(left);
            leftIsLiteral = true;
            leftNeg = false;
            leftVarId = varLeft->getId();
        }
        if (left->getType() == UNARY) {
            auto unaryLeft = std::static_pointer_cast<UnaryOperator>(left);
            if (unaryLeft->getOp() == UnaryOperator::NOT) {
                if (unaryLeft->getOperand()->getType() == VARIABLE) {
                    auto varLeft = std::static_pointer_cast<Variable>(unaryLeft->getOperand());
                    leftIsLiteral = true;
                    leftNeg = true;
                    leftVarId = varLeft->getId();
                }
            }
        }

        bool rightIsLiteral = false;
        bool rightNeg;
        int rightVarId;
        if (right->getType() == VARIABLE) {
            auto varRight = std::static_pointer_cast<Variable>(right);
            rightIsLiteral = true;
            rightNeg = false;
            rightVarId = varRight->getId();
        }
        if (right->getType() == UNARY) {
            auto unaryRight = std::static_pointer_cast<UnaryOperator>(right);
            if (unaryRight->getOp() == UnaryOperator::NOT) {
                if (unaryRight->getOperand()->getType() == VARIABLE) {
                    auto varLeft = std::static_pointer_cast<Variable>(unaryRight->getOperand());
                    rightIsLiteral = true;
                    rightNeg = true;
                    rightVarId = varLeft->getId();
                }
            }
        }

        if (leftIsLiteral && rightIsLiteral) {
            if (leftVarId == rightVarId) {
                anyChange = true;
                if (leftNeg == rightNeg)
                    return left;
                else
                    return std::make_shared<Constant>(Constant::Value(op == OR));
            }
        }
    }

    auto reducedLeft = left->reduce(anyChange);
    auto reducedRight = right->reduce(anyChange);
    if (reducedLeft)
        left = reducedLeft;
    if (reducedRight)
        right = reducedRight;
    return nullptr;
}
