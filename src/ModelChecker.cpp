#include "ModelChecker.hpp"

#include "Variable.hpp"
#include "Constant.hpp"
#include "UnaryOperator.hpp"
#include "BinaryOperator.hpp"

#include <cassert>
#include <stdexcept>
#include <algorithm>

bool NaiveModelChecker::isValid(const std::shared_ptr<Proposition>& proposition) const {
	const int BIT_COUNT = sizeof(uint64_t) * 8;
	const int LOG_BIT_COUNT = 6;
	assert((1 << LOG_BIT_COUNT) == BIT_COUNT);

	std::vector<int> variableIds;
	proposition->getVariableIds(variableIds);
	std::sort(variableIds.begin(), variableIds.end());
	if (variableIds.size() > BIT_COUNT)
		throw std::runtime_error("NaiveModelChecker supports max 64 variables");
	const int modelCount = (static_cast<uint64_t>(1) << variableIds.size());

	std::vector<uint64_t> varValues;
	if (variableIds.empty())
		return proposition->evaluate(varValues) != 0;

	assert(*std::min_element(variableIds.begin(), variableIds.end()) >= 0);
	int maxId = *std::max_element(variableIds.begin(), variableIds.end());
	varValues = std::vector<uint64_t>(maxId + 1);

	if (variableIds.size() < LOG_BIT_COUNT) {
		for (uint64_t model = 0; model < modelCount; model++) {
			for (int i = 0; i < variableIds.size(); i++) {
				bool varValue = (model & (static_cast<uint64_t>(1) << i)) != 0;
				varValues[variableIds[i]] = varValue ? ULLONG_MAX : 0;
			}
			if (proposition->evaluate(varValues) == 0)
				return false;
		}
	}
	else {
		for (int i = 0; i < LOG_BIT_COUNT; i++) {
			uint64_t mask = 0;
			for (uint64_t model = 0; model < BIT_COUNT; model++) {
				bool varValue = (model & (static_cast<uint64_t>(1) << i)) != 0;
				if (varValue)
					mask |= (static_cast<uint64_t>(1) << model);
			}
			varValues[variableIds[i]] = mask;
		}
		for (uint64_t model = 0; model < modelCount; model += BIT_COUNT) {
			for (int i = LOG_BIT_COUNT; i < variableIds.size(); i++) {
				bool varValue = (model & (static_cast<uint64_t>(1) << i)) != 0;
				varValues[variableIds[i]] = varValue ? ULLONG_MAX : 0;
			}
			if (proposition->evaluate(varValues) != ULLONG_MAX)
				return false;
		}
	}
	return true;
}

bool NaiveModelChecker::isContradiction(const std::shared_ptr<Proposition>& proposition) const {
	auto notProposition = std::make_shared<UnaryOperator>(proposition, UnaryOperator::NOT);
	return isValid(notProposition);
}
