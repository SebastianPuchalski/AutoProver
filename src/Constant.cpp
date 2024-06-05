#include "Constant.hpp"

Constant::Constant(Value value) : value(value) {}

Constant::Type Constant::getType() const {
    return CONSTANT;
}

Constant::Value Constant::getValue() const {
    return value;
}

void Constant::setValue(Value value) {
    this->value = value;
}

std::shared_ptr<Proposition> Constant::copy() const {
    return std::make_shared<Constant>(*this);
}

std::shared_ptr<Proposition> Constant::transformXnorToImp() {
    return nullptr;
}

std::shared_ptr<Proposition> Constant::transformImpToOr() {
    return nullptr;
}

std::shared_ptr<Proposition> Constant::eliminateDoubleNot(bool& anyChange) {
    return nullptr;
}

std::shared_ptr<Proposition> Constant::moveNotInwardsOp(int binaryOp, bool& anyChange) {
    return nullptr;
}

std::shared_ptr<Proposition> Constant::distributeOrAnd(bool orOverAnd, bool& anyChange) {
    return nullptr;
}

std::shared_ptr<Proposition> Constant::reduce(bool& anyChange) {
    return nullptr;
}
