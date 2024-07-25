#pragma once

#include "Proposition.hpp"

#include <vector>
#include <random>

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

void generateClause(Clause& clause, int literalNum, int variableNum);
void generateCnf(Cnf& clauses, int literalNum, int clauseNum, int variableNum);
void generateClause(Clause& clause, int literalNum, int variableNum, std::mt19937& gen);
void generateCnf(Cnf& clauses, int literalNum, int clauseNum, int variableNum, std::mt19937& gen);

void squeezeVariableIds(Cnf& clauses);
