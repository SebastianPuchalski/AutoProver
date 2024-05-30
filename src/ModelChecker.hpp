#pragma once

#include "Variable.hpp"
#include "Constant.hpp"
#include "UnaryOperator.hpp"
#include "BinaryOperator.hpp"

#include <vector>

class NaiveModelChecker {
public:
	NaiveModelChecker() = default;
	virtual ~NaiveModelChecker() = default;

	bool isValid(const std::shared_ptr<Proposition>& proposition) const;
	bool isContradiction(const std::shared_ptr<Proposition>& proposition) const;

private:
	void variableIdsFromProposition(const std::shared_ptr<Proposition>& proposition, std::vector<int>& variableIds) const;
	bool evaluate(const std::shared_ptr<Proposition>& proposition, const std::vector<bool>& varValues) const;
};
