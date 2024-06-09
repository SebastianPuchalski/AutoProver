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

void Constant::getVariableIds(std::vector<int>& variableIds) const {
}

uint64_t Constant::evaluate(const std::vector<uint64_t>& varValues) const {
    return value == TRUE ? ULLONG_MAX : 0;
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
