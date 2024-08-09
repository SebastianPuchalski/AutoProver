#pragma once

#include "NormalForm.hpp"

class DpllCnfSat {
public:
	DpllCnfSat(const Cnf& cnf);
	~DpllCnfSat() = default;

	bool isSatisfiable();

	static bool isPropValid(const PropositionSP& proposition);
	static bool isPropContradiction(const PropositionSP& proposition);
	std::vector<bool> getModel() const; // squeezed variable ids

private:
	Cnf clauses;
	std::vector<bool> variableAssigned;
	std::vector<bool> variableValues;
	std::vector<int> negativeLiteralCount;
	std::vector<int> positiveLiteralCount;

	bool dpll(VariableId id, bool value);
};

class WalkSat {
public:
	WalkSat(const Cnf& cnf);
	~WalkSat() = default;

	// isSatisfiable returns true if satisfiable and false if probably not
	bool isSatisfiable(uint64_t maxFlipNumber = 1000, float p = 0.5f);
	std::vector<bool> getModel() const; // squeezed variable ids

private:
	Cnf clauses;
	std::vector<bool> model;
	std::vector<Clause*> falseClauses;
};
