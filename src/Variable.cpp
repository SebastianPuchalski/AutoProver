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

bool Variable::isEquivalent(PropositionSP proposition) const {
    if (!proposition || proposition->getType() != Proposition::VARIABLE)
        return false;
    auto prop = std::static_pointer_cast<Variable>(proposition);
    return id == prop->id;
}

PropositionSP Variable::copy() const {
    return std::make_shared<Variable>(*this);
}

int Variable::getLength() const {
    return 1;
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

PropositionSP Variable::transformXnorToImp() {
    return nullptr;
}

PropositionSP Variable::transformImpToOr() {
    return nullptr;
}

PropositionSP Variable::eliminateDoubleNot(bool& anyChange) {
    return nullptr;
}

PropositionSP Variable::moveNotInwardsOp(int binaryOp, bool& anyChange) {
    return nullptr;
}

PropositionSP Variable::distributeOrAnd(bool orOverAnd, bool& anyChange) {
    return nullptr;
}

PropositionSP Variable::reduce(bool& anyChange) {
    return nullptr;
}
