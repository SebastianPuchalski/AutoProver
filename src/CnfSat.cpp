#include "CnfSat.hpp"

#include "UnaryOperator.hpp"

#include <cassert>
#include <random>

DpllCnfSat::DpllCnfSat(const Cnf& cnf) : clauses(cnf) {
	squeezeVariableIds(clauses);
	clauses.shrink_to_fit();
	int variableCount = 0;
	for (auto& clause : clauses)
		for (auto& literal : clause)
			variableCount = std::max(literal.varId + 1, variableCount);
	if (variableCount > 0) {
		variableAssigned.resize(variableCount);
		variableValues.resize(variableCount);
		negativeLiteralCount.resize(variableCount);
		positiveLiteralCount.resize(variableCount);
	}
}

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
			VariableId id = literal.varId;
			if (variableAssigned[id]) {
				if (variableValues[id] != literal.neg) {
					falseClause = false;
					trueClause = true;
					break;
				}
			}
			else {
				falseClause = false;
				literalCount++;
				lastLiteralVarId = literal.varId;
				lastLiteralNeg = literal.neg;
			}
		}
		if (!falseClause && !trueClause) {
			for (auto& literal : clause) {
				VariableId id = literal.varId;
				if (!variableAssigned[id]) {
					if (literal.neg)
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

std::vector<bool> DpllCnfSat::getModel() const {
	return variableValues;
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

WalkSat::WalkSat(const Cnf& cnf) : clauses(cnf) {
	squeezeVariableIds(clauses);
	clauses.shrink_to_fit();
	int variableCount = 0;
	for (auto& clause : clauses)
		for (auto& literal : clause)
			variableCount = std::max(literal.varId + 1, variableCount);
	if (variableCount > 0) {
		model.resize(variableCount);
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dist(0, 1);
		for (int i = 0; i < model.size(); i++)
			model[i] = dist(gen);
	}
}

bool WalkSat::isSatisfiable(uint64_t maxFlipNumber, float randWalkP) {
	if (clauses.empty())
		return true;
	for (auto& clause : clauses)
		if (clause.empty())
			return false;

	std::random_device rd;
	std::mt19937 gen(rd());
	const int maxValue = static_cast<int>(clauses.size() + model.size());
	std::uniform_int_distribution<> iDist(0, maxValue);
	std::uniform_real_distribution<float> fDist(0.f, 1.f);

	for (uint64_t i = 0; i < maxFlipNumber; i++) {
		falseClauses.clear();
		bool trueSentence = true;
		for (auto& clause : clauses) {
			bool trueClause = false;
			for (auto& literal : clause) {
				if (model[literal.varId] != literal.neg) {
					trueClause = true;
					break;
				}
			}
			if (!trueClause) {
				trueSentence = false;
				falseClauses.push_back(&clause);
				break;
			}
		}
		if (trueSentence)
			return true;

		VariableId variableToflip = -1;
		int randFalseClauseIdx = iDist(gen) % falseClauses.size();
		Clause& randFalseClause = *falseClauses[randFalseClauseIdx];
		if (fDist(gen) < randWalkP) {
			int randLiteralIdx = iDist(gen) % randFalseClause.size();
			variableToflip = randFalseClause[randLiteralIdx].varId;
		}
		else {
			int bestTrueClauseCount = -1;
			for (auto& literal : randFalseClause) {
				VariableId id = literal.varId;
				model[id] = !model[id];
				int trueClauseCount = 0;
				for (auto& clause : clauses) {
					for (auto& literal : clause) {
						if (model[literal.varId] != literal.neg) {
							trueClauseCount++;
							break;
						}
					}
				}
				model[id] = !model[id];
				if (trueClauseCount > bestTrueClauseCount) {
					variableToflip = id;
					bestTrueClauseCount = trueClauseCount;
				}
			}
		}
		assert(variableToflip > -1);
		model[variableToflip] = !model[variableToflip];
	}
	return false;
}

std::vector<bool> WalkSat::getModel() const {
	return model;
}
