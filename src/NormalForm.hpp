#pragma once

#include "Proposition.hpp"

#include <vector>

using VariableId = int; // non-negative values only

struct Literal {
	VariableId varId;
	bool neg;

	Literal(VariableId varId, bool neg) : varId(varId), neg(neg) {}
	~Literal() = default;
};

using Clause = std::vector<Literal>;
using Cnf = std::vector<Clause>;

void propositionToCnf(Cnf& clauses, PropositionSP proposition);
PropositionSP clauseToProposition(const Clause& clause);
PropositionSP cnfToProposition(const Cnf& clauses);

void squeezeVariableIds(Cnf& clauses);
