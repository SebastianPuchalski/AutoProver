#pragma once

#include "Proposition.hpp"

class NaiveModelChecker {
public:
	bool isValid(const std::shared_ptr<Proposition>& proposition) const;
	bool isContradiction(const std::shared_ptr<Proposition>& proposition) const;
};
