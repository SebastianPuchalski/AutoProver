#pragma once

#include "Proposition.hpp"

class NaiveModelChecker {
public:
	bool isValid(const PropositionSP& proposition) const;
	bool isContradiction(const PropositionSP& proposition) const;
};
