#pragma once

#include "Proposition.hpp"

namespace Resolution {

	bool isValid(const std::shared_ptr<Proposition>& proposition);
	bool isContradiction(const std::shared_ptr<Proposition>& proposition);

} // namespace Resolution
