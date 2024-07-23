#pragma once

#include "NormalForm.hpp"

class DpllCnfSat {
public:
	DpllCnfSat(const Cnf& cnf);
	virtual ~DpllCnfSat();

	bool isSatisfiable();

	static bool isPropValid(const PropositionSP& proposition);
	static bool isPropContradiction(const PropositionSP& proposition);

private:
	Cnf clauses;
	std::vector<bool> variableAssigned;
	std::vector<bool> variableValues;
	std::vector<int> negativeLiteralCount;
	std::vector<int> positiveLiteralCount;

	bool dpll(VariableId id, bool value);
};

