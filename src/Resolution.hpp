#pragma once

#include "Proposition.hpp"

#include <string>

namespace Resolution {
	bool isValid(const PropositionSP& proposition, std::string* proof = nullptr);
	bool isContradiction(const PropositionSP& proposition, std::string* proof = nullptr);

} // namespace Resolution
