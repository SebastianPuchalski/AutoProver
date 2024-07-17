#pragma once

#include "Proposition.hpp"

#include <vector>

using VariableId = int; // non-negative values only
using Literal = std::pair<VariableId, bool>; // id, negation
using Clause = std::vector<Literal>;
using Cnf = std::vector<Clause>;

void propositionToCnf(Cnf& clauses, PropositionSP proposition);
void squeezeVariableIds(Cnf& clauses);
