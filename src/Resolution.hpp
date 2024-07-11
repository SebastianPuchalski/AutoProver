#pragma once

#include "Proposition.hpp"

namespace Resolution {
	bool isValid(const PropositionSP& proposition);
	bool isContradiction(const PropositionSP& proposition);

} // namespace Resolution
