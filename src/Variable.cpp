#include "Variable.hpp"

#include <cassert>

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

void Variable::getVariableIds(std::vector<int>& variableIds) const {
    bool alreadyExists = false;
    for (auto item : variableIds) {
        if (item == id)
            alreadyExists = true;
    }
    if (!alreadyExists)
        variableIds.push_back(id);
}

uint64_t Variable::evaluate(const std::vector<uint64_t>& varValues) const {
    assert(id < varValues.size());
    return varValues[id];
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

std::shared_ptr<Proposition> Variable::reduce(bool& anyChange) {
    return nullptr;
}
