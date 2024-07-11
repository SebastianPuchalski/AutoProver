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

bool Constant::isEquivalent(PropositionSP proposition) const {
    if (!proposition || proposition->getType() != Proposition::CONSTANT)
        return false;
    auto prop = std::static_pointer_cast<Constant>(proposition);
    return value == prop->value;
}

PropositionSP Constant::copy() const {
    return std::make_shared<Constant>(*this);
}

int Constant::getLength() const {
    return 1;
}

void Constant::getVariableIds(std::vector<int>& variableIds) const {
}

uint64_t Constant::evaluate(const std::vector<uint64_t>& varValues) const {
    return value == TRUE ? ULLONG_MAX : 0;
}

PropositionSP Constant::transformXnorToImp() {
    return nullptr;
}

PropositionSP Constant::transformImpToOr() {
    return nullptr;
}

PropositionSP Constant::eliminateDoubleNot(bool& anyChange) {
    return nullptr;
}

PropositionSP Constant::moveNotInwardsOp(int binaryOp, bool& anyChange) {
    return nullptr;
}

PropositionSP Constant::distributeOrAnd(bool orOverAnd, bool& anyChange) {
    return nullptr;
}

PropositionSP Constant::reduce(bool& anyChange) {
    return nullptr;
}
