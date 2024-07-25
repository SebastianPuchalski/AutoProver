#include "NormalForm.hpp"

#include "Variable.hpp"
#include "Constant.hpp"
#include "UnaryOperator.hpp"
#include "BinaryOperator.hpp"

#include <cassert>
#include <map>

bool traverseLiteral(std::vector<Literal>& literals, const PropositionSP& literal, bool negation) {
	if (literal->getType() == Proposition::UNARY &&
		std::static_pointer_cast<UnaryOperator>(literal)->getOp() == UnaryOperator::NOT) {
		assert(!negation);
		auto operand = std::static_pointer_cast<UnaryOperator>(literal)->getOperand();
		return traverseLiteral(literals, operand, true);
	}
	else if (literal->getType() == Proposition::CONSTANT) {
		auto constant = std::static_pointer_cast<Constant>(literal);
		return static_cast<bool>(constant->getValue()) == negation; // remove clause if literal equals True
	}
	else if (literal->getType() == Proposition::VARIABLE) {
		auto variable = std::static_pointer_cast<Variable>(literal);
		literals.push_back(Literal(variable->getId(), negation));
	}
	else assert(!"Unexpected literal");
	return true;
	// returns false if literal is True and should be removed
}

bool traverseClause(std::vector<Literal>& literals, const PropositionSP& clause) {
	if (clause->getType() == Proposition::BINARY &&
		std::static_pointer_cast<BinaryOperator>(clause)->getOp() == BinaryOperator::OR) {
		auto binaryProp = std::static_pointer_cast<BinaryOperator>(clause);
		return traverseClause(literals, binaryProp->getLeft()) &&
			traverseClause(literals, binaryProp->getRight());
	}
	return traverseLiteral(literals, clause, false);
	// returns false if clause is True and should be removed
}

void cnfPropToVec(std::vector<Clause>& clauses, const PropositionSP& cnf) {
	if (cnf->getType() == Proposition::BINARY &&
		std::static_pointer_cast<BinaryOperator>(cnf)->getOp() == BinaryOperator::AND) {
		auto binaryProp = std::static_pointer_cast<BinaryOperator>(cnf);
		cnfPropToVec(clauses, binaryProp->getLeft());
		cnfPropToVec(clauses, binaryProp->getRight());
	}
	else {
		std::vector<Literal> clause;
		if (traverseClause(clause, cnf))
			clauses.push_back(clause);
	}
	// if there is no clause then proposition is True
	// if there is at least one empty clause then proposition is False
}

// removing redundant literals and clauses with complementary literals
bool removeRedundancy(std::vector<Clause>& cnf) {
	bool anyChange = false;
	std::vector<Clause> newCnf;
	for (int c = 0; c < cnf.size(); c++) {
		auto clause = cnf[c];
		bool removeClause = false;
		Clause newClause;
		for (int i = 0; i < clause.size(); i++) {
			auto literal = clause[i];
			bool removeLiteral = false;
			for (int j = 0; j < i; j++) {
				if (literal.varId == clause[j].varId) {
					if (clause[i].neg != clause[j].neg) {
						removeClause = anyChange = true;
						i = j = clause.size();
					}
					else {
						removeLiteral = anyChange = true;
						break;
					}
				}
			}
			if (!removeLiteral)
				newClause.push_back(literal);
		}
		if (!removeClause)
			newCnf.push_back(newClause);
	}
	cnf = newCnf;
	return anyChange;
}

void propositionToCnf(Cnf& clauses, PropositionSP proposition) {
	auto cnf = proposition->toCnf();
	cnfPropToVec(clauses, cnf);
	removeRedundancy(clauses);
	clauses.shrink_to_fit();
}

PropositionSP clauseToProposition(const Clause& clause) {
	if (clause.empty())
		return std::make_shared<Constant>(Constant::FALSE);
	if (clause.size() == 1) {
		auto& literal = clause.front();
		PropositionSP result = std::make_shared<Variable>(literal.varId);
		if(literal.neg)
			result = std::make_shared<UnaryOperator>(result, UnaryOperator::NOT);
		return result;
	}

	int middle = clause.size() / 2;
	Clause part1(clause.begin(), clause.begin() + middle);
	Clause part2(clause.begin() + middle, clause.end());
	auto prop1 = clauseToProposition(part1);
	auto prop2 = clauseToProposition(part2);
	return std::make_shared<BinaryOperator>(prop1, BinaryOperator::OR, prop2);
}

PropositionSP cnfToProposition(const Cnf& clauses) {
	if(clauses.empty())
		return std::make_shared<Constant>(Constant::TRUE);
	if (clauses.size() == 1)
		return clauseToProposition(clauses.front());

	int middle = clauses.size() / 2;
	Cnf part1(clauses.begin(), clauses.begin() + middle);
	Cnf part2(clauses.begin() + middle, clauses.end());
	auto prop1 = cnfToProposition(part1);
	auto prop2 = cnfToProposition(part2);
	return std::make_shared<BinaryOperator>(prop1, BinaryOperator::AND, prop2);
}

void generateClause(Clause& clause, int literalNum, int variableNum, std::mt19937& gen) {
	std::uniform_int_distribution<> varDist(0, variableNum - 1);
	std::uniform_real_distribution<float> negDist(0.f, 1.f);

	for (int i = 0; i < literalNum; i++) {
		Literal literal(varDist(gen), negDist(gen) > 0.5f);
		clause.push_back(literal);
	}
}

void generateClause(Clause& clause, int literalNum, int variableNum) {
	std::random_device rd;
	std::mt19937 gen(rd());
	generateClause(clause, literalNum, variableNum, gen);
}

void generateCnf(Cnf& clauses, int literalNum, int clauseNum, int variableNum, std::mt19937& gen) {
	clauses.clear();
	for (int i = 0; i < clauseNum; i++) {
		Clause clause;
		generateClause(clause, literalNum, variableNum, gen);
		clauses.push_back(clause);
	}
}

void generateCnf(Cnf& clauses, int literalNum, int clauseNum, int variableNum) {
	std::random_device rd;
	std::mt19937 gen(rd());
	generateCnf(clauses, literalNum, clauseNum, variableNum, gen);
}

void squeezeVariableIds(Cnf& clauses) {
	std::map<VariableId, VariableId> map;
	for (auto& clause : clauses)
		for (auto& literal : clause)
			map[literal.varId] = 0;

	VariableId newId = 0;
	for (auto& pair : map)
		pair.second = newId++;

	for (auto& clause : clauses)
		for (auto& literal : clause)
			literal.varId = map[literal.varId];
}
