#include "CnfSat.hpp"

#include "UnaryOperator.hpp"

#include <cassert>

DpllCnfSat::DpllCnfSat(const Cnf& cnf) : clauses(cnf) {
	squeezeVariableIds(clauses);
	clauses.shrink_to_fit();
	int variableCount = 0;
	for (auto& clause : clauses)
		for (auto& literal : clause)
			variableCount = std::max(literal.first + 1, variableCount);
	if (variableCount > 0) {
		variableAssigned.resize(variableCount);
		variableValues.resize(variableCount);
		negativeLiteralCount.resize(variableCount);
		positiveLiteralCount.resize(variableCount);
	}
}

DpllCnfSat::~DpllCnfSat() {}

bool DpllCnfSat::isSatisfiable() {
	if (clauses.empty())
		return true;
	for (auto& clause : clauses)
		if (clause.empty())
			return false;
	for (int i = 0; i < variableAssigned.size(); i++)
		variableAssigned[i] = false;
	return dpll(0, false) || dpll(0, true);
}

bool DpllCnfSat::dpll(VariableId id, bool value) {
	variableAssigned[id] = true;
	variableValues[id] = value;

	assert(negativeLiteralCount.size() == positiveLiteralCount.size());
	for (int i = 0; i < positiveLiteralCount.size(); i++) {
		negativeLiteralCount[i] = 0;
		positiveLiteralCount[i] = 0;
	}

	bool falseSentence = false;
	bool trueSentence = true;
	VariableId unitClauseVarId = -1;
	bool unitClauseLiteralNeg;
	for (auto& clause : clauses) {
		bool falseClause = true;
		bool trueClause = false;
		int literalCount = 0;
		VariableId lastLiteralVarId;
		bool lastLiteralNeg;
		for (auto& literal : clause) {
			VariableId id = literal.first;
			if (variableAssigned[id]) {
				if (variableValues[id] != literal.second) {
					falseClause = false;
					trueClause = true;
					break;
				}
			}
			else {
				falseClause = false;
				literalCount++;
				lastLiteralVarId = literal.first;
				lastLiteralNeg = literal.second;
			}
		}
		if (!falseClause && !trueClause) {
			for (auto& literal : clause) {
				VariableId id = literal.first;
				if (!variableAssigned[id]) {
					if (literal.second)
						negativeLiteralCount[id]++;
					else
						positiveLiteralCount[id]++;
				}
			}
			if (literalCount == 1 && unitClauseVarId == -1) {
				unitClauseVarId = lastLiteralVarId;
				unitClauseLiteralNeg = lastLiteralNeg;
			}
		}
		if (falseClause) {
			falseSentence = true;
			trueSentence = false;
			break;
		}
		if (!trueClause)
			trueSentence = false;
	}
	if (falseSentence || trueSentence) {
		variableAssigned[id] = false;
		return trueSentence;
	}

	VariableId pureLiteralVarId = -1;
	bool pureLiteralNeg;
	for (int i = 0; i < positiveLiteralCount.size(); i++) {
		int nlc = negativeLiteralCount[i];
		int plc = positiveLiteralCount[i];
		if ((nlc == 0 && plc > 0) || (nlc > 0 && plc == 0)) {
			pureLiteralVarId = i;
			pureLiteralNeg = nlc > 0;
			break;
		}
	}
	if (pureLiteralVarId != -1) {
		bool result = dpll(pureLiteralVarId, !pureLiteralNeg);
		variableAssigned[id] = false;
		return result;
	}

	if (unitClauseVarId != -1) {
		bool result = dpll(unitClauseVarId, !unitClauseLiteralNeg);
		variableAssigned[id] = false;
		return result;
	}

	VariableId unassignVarId = 0;
	while (unassignVarId < variableAssigned.size() && variableAssigned[unassignVarId])
		unassignVarId++;
	assert(unassignVarId < variableAssigned.size());
	bool result = dpll(unassignVarId, false) || dpll(unassignVarId, true);
	variableAssigned[id] = false;
	return result;
}

bool DpllCnfSat::isPropValid(const PropositionSP& proposition) {
	auto notProposition = std::make_shared<UnaryOperator>(proposition, UnaryOperator::NOT);
	return isPropContradiction(notProposition);
}

bool DpllCnfSat::isPropContradiction(const PropositionSP& proposition) {
	Cnf clauses;
	propositionToCnf(clauses, proposition);
	DpllCnfSat dpll(clauses);
	return !dpll.isSatisfiable();
}
