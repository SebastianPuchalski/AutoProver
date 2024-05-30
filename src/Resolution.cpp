#include "Resolution.hpp"

#include "Variable.hpp"
#include "Constant.hpp"
#include "UnaryOperator.hpp"
#include "BinaryOperator.hpp"

#include <vector>
#include <cassert>
#include <iostream>

namespace Resolution {

typedef int VariableId;
typedef std::pair<VariableId, bool> Literal;
typedef std::vector<Literal> Clause;
typedef long long unsigned uint64;

struct BitClause {
	uint64 pLiterals;
	uint64 nLiterals;

	BitClause() : pLiterals(0), nLiterals(0) {}

	bool operator==(const BitClause& rhs) const {
		return pLiterals == rhs.pLiterals &&
			   nLiterals == rhs.nLiterals;
	}
};

void printCnf(const std::vector<Clause>& clauses) {
	for (int i = 0; i < clauses.size(); i++) {
		std::cout << "(";
		auto literals = clauses[i];
		for (int j = 0; j < literals.size(); j++) {
			auto literal = literals[j];
			if (literal.second)
				std::cout << "~";
			std::cout << literal.first;
			if (j + 1 < literals.size())
				std::cout << " | ";
		}
		std::cout << ")";
		if (i + 1 < clauses.size())
			std::cout << " & ";
	}
}

bool traverseLiteral(std::vector<Literal>& literals, const std::shared_ptr<Proposition>& literal, bool negation) {
	if (literal->getType() == Proposition::UNARY &&
		std::static_pointer_cast<UnaryOperator>(literal)->getOp() == UnaryOperator::NOT) {
		assert(!negation);
			auto operand = std::static_pointer_cast<UnaryOperator>(literal)->getOperand();
			return traverseLiteral(literals, operand, true);
	}
	else if (literal->getType() == Proposition::CONSTANT) {
		auto constant = std::static_pointer_cast<Constant>(literal);
		return constant->getValue() == negation; // remove clause if literal equals True
	}
	else if (literal->getType() == Proposition::VARIABLE) {
		auto variable = std::static_pointer_cast<Variable>(literal);
		literals.push_back(Literal(variable->getId(), negation));
	}
	else assert(!"Unexpected literal");
	return true;
	// returns false if literal is True and should be removed
}

bool traverseClause(std::vector<Literal>& literals, const std::shared_ptr<Proposition>& clause) {
	if (clause->getType() == Proposition::BINARY &&
		std::static_pointer_cast<BinaryOperator>(clause)->getOp() == BinaryOperator::OR) {
		auto binaryProp = std::static_pointer_cast<BinaryOperator>(clause);
		return traverseClause(literals, binaryProp->getLeft()) &&
			   traverseClause(literals, binaryProp->getRight());
	}
	return traverseLiteral(literals, clause, false);
	// returns false if clause is True and should be removed
}

void traverseCnf(std::vector<Clause>& clauses, const std::shared_ptr<Proposition>& cnf) {
	if (cnf->getType() == Proposition::BINARY &&
		std::static_pointer_cast<BinaryOperator>(cnf)->getOp() == BinaryOperator::AND) {
		auto binaryProp = std::static_pointer_cast<BinaryOperator>(cnf);
		traverseCnf(clauses, binaryProp->getLeft());
		traverseCnf(clauses, binaryProp->getRight());
	}
	else {
		std::vector<Literal> clause;
		if(traverseClause(clause, cnf))
			clauses.push_back(clause);
	}
	// if there is no clause then proposition is True
	// if there is at least one empty clause then proposition is False
}

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
				if (literal.first == clause[j].first) {
					if (clause[i].second != clause[j].second) {
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

bool clausesToBitClauses(std::vector<BitClause>& bitClauses, const std::vector<Clause>& clauses) {
	for (auto& clause : clauses) {
		BitClause bitClause;
		for (auto& literal : clause) {
			if (literal.first >= sizeof(uint64) * 8)
				return false;
			uint64 mask = static_cast<uint64>(1) << literal.first;
			if (literal.second)
				bitClause.nLiterals |= mask;
			else
				bitClause.pLiterals |= mask;
		}
		if (!(bitClause.pLiterals & bitClause.nLiterals))
			bitClauses.push_back(bitClause);
	}
	return true;
}

bool resolve(std::vector<BitClause>& clauses, BitClause a, BitClause b) {
	uint64 complMask = (a.pLiterals & b.nLiterals) | (a.nLiterals & b.pLiterals);
	if (complMask) {
		for (int i = 0; i < (sizeof(uint64) * 8); i++) {
			uint64 indexMask = static_cast<uint64>(1) << i;
			if (complMask & indexMask) {
				BitClause newClause;
				newClause.pLiterals = a.pLiterals | b.pLiterals;
				newClause.nLiterals = a.nLiterals | b.nLiterals;
				newClause.pLiterals &= ~indexMask;
				newClause.nLiterals &= ~indexMask;
				newClause.pLiterals |= a.pLiterals & b.pLiterals;
				newClause.nLiterals |= a.nLiterals & b.nLiterals;
				if (!(newClause.pLiterals | newClause.nLiterals))
					return true; // contradiction
				clauses.push_back(newClause);
			}
		}
	}
	return false;
}

bool resolve(std::vector<BitClause> clauses) {
	std::vector<BitClause> oldClauses;
	std::vector<BitClause> newClauses;

	while (!clauses.empty()) {
		for (int i = 0; i < clauses.size(); i++)
			for (int j = i + 1; j < clauses.size(); j++)
				if (resolve(newClauses, clauses[i], clauses[j]))
					return true;
		for (int i = 0; i < clauses.size(); i++)
			for (int j = 0; j < oldClauses.size(); j++)
				if (resolve(newClauses, clauses[i], oldClauses[j]))
					return true;
		oldClauses.insert(oldClauses.end(), clauses.begin(), clauses.end());
		clauses.clear();

		for (auto& newClause : newClauses) {
			bool reallyNew = true;
			for (auto& clause : clauses) {
				if (clause == newClause) {
					reallyNew = false;
					break;
				}
			}
			if (reallyNew) {
				for (auto& clause : oldClauses) {
					if (clause == newClause) {
						reallyNew = false;
						break;
					}
				}
			}
			if (reallyNew)
				clauses.push_back(newClause);
		}
		newClauses.clear();
	}
	return false;
}

bool isValid(const std::shared_ptr<Proposition>& proposition) {
	auto notProposition = std::make_shared<UnaryOperator>(proposition, UnaryOperator::NOT);
	return isContradiction(notProposition);
}

bool isContradiction(const std::shared_ptr<Proposition>& proposition) {
	auto cnf = proposition->toCNF();

	std::vector<Clause> clauses;
	traverseCnf(clauses, cnf);
	//removeRedundancy(clauses);

	std::vector<BitClause> bitClauses;
	if (!clausesToBitClauses(bitClauses, clauses))
		throw std::runtime_error("Variable id is greater than 63");

	return resolve(bitClauses);
}

} // namespace Resolution
