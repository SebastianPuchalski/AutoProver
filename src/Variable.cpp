#include "Variable.hpp"

Variable::Variable(int id) : id(id) {}

Variable::Type Variable::getType() const {
    return VARIABLE;
}

int Variable::getId() const {
    return id;
}

void Variable::setId(int id) {
    this->id = id;
}

std::shared_ptr<Proposition> Variable::copy() const {
    return std::make_shared<Variable>(*this);
}

std::shared_ptr<Proposition> Variable::transformXnorToImp() {
    return nullptr;
}

std::shared_ptr<Proposition> Variable::transformImpToOr() {
    return nullptr;
}

std::shared_ptr<Proposition> Variable::eliminateDoubleNot(bool& anyChange) {
    return nullptr;
}

std::shared_ptr<Proposition> Variable::moveNotInwardsOp(int binaryOp, bool& anyChange) {
    return nullptr;
}

std::shared_ptr<Proposition> Variable::distributeOrAnd(bool orOverAnd, bool& anyChange) {
    return nullptr;
}
