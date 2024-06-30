#include "Resolution.hpp"

#include "Variable.hpp"
#include "Constant.hpp"
#include "UnaryOperator.hpp"
#include "BinaryOperator.hpp"
#include "Converter.hpp"

#include <vector>
#include <cassert>
#include <string>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include <chrono>
#include <bit>

//size_t p_prod = 0;
//size_t p_onecomp = 0;
//size_t p_new = 0;
//size_t p_set = 0;
//size_t p_main = 0;
//size_t p_add = 0;
//size_t p_remove = 0;

struct BitClause {
	static const int VARIABLE_COUNT = sizeof(uint64_t) * 8;
	uint64_t pLiterals;
	uint64_t nLiterals;

	BitClause() : pLiterals(0), nLiterals(0) {}

	bool operator==(const BitClause& rhs) const {
		return pLiterals == rhs.pLiterals &&
			nLiterals == rhs.nLiterals;
	}

	bool operator!=(const BitClause& rhs) const {
		return !operator==(rhs);
	}

	bool isSubset(const BitClause& rhs) const {
		return (pLiterals & ~rhs.pLiterals) == 0 &&
			(nLiterals & ~rhs.nLiterals) == 0;
	}

	bool isProperSubset(const BitClause& rhs) const {
		return isSubset(rhs) && *this != rhs;
	}

	bool operator<(const BitClause& rhs) const {
		/*auto popcount = [](uint64_t x) {
			const uint64_t m1 = 0x5555555555555555;
			const uint64_t m4 = 0x0f0f0f0f0f0f0f0f;
			const uint64_t h01 = 0x0101010101010101;
			const uint64_t m2 = 0x3333333333333333;
			x -= (x >> 1) & m1;
			x = (x & m2) + ((x >> 2) & m2);
			x = (x + (x >> 4)) & m4;
			return (x * h01) >> 56;
			};
		int oneCount = popcount(pLiterals | nLiterals);
		int rhsOneCount = popcount(rhs.pLiterals | rhs.nLiterals);
		assert(std::popcount(pLiterals | nLiterals) == oneCount);
		assert(std::popcount(rhs.pLiterals | rhs.nLiterals) == rhsOneCount);
		if (oneCount != rhsOneCount)
			return oneCount < rhsOneCount;*/
		if (pLiterals != rhs.pLiterals)
			return pLiterals < rhs.pLiterals;
		return nLiterals < rhs.nLiterals;
	}
};

namespace std {
    template<>
    struct hash<BitClause> {
        size_t operator()(const BitClause& bc) const {
            return mix(bc.pLiterals, bc.nLiterals);
        }

    private:
        size_t mix(uint64_t a, uint64_t b) const {
            size_t hashA = simpleHash(a);
            size_t hashB = simpleHash(b);
            return hashA ^ (hashB + 0x9e3779b9 + (hashA << 6) + (hashA >> 2));
        }

        size_t simpleHash(uint64_t x) const {
			x ^= rotateLeft(x, 33);
            x *= 0xff51afd7ed558ccd;
            x ^= rotateLeft(x, 47);
            return static_cast<size_t>(x);
        }

        uint64_t rotateLeft(uint64_t x, int r) const {
            return (x << r) | (x >> (64 - r));
        }
    };
}

struct BucketBuff {
	std::vector<std::set<BitClause>> pBuckets;
	std::vector<std::set<BitClause>> nBuckets;

	BucketBuff() : pBuckets(BitClause::VARIABLE_COUNT),
		           nBuckets(BitClause::VARIABLE_COUNT) {}

	void addClause(BitClause clause) {
		assert(pBuckets.size() == BitClause::VARIABLE_COUNT);
		assert(nBuckets.size() == BitClause::VARIABLE_COUNT);
		for (int v = 0; v < BitClause::VARIABLE_COUNT; v++) {
			uint64_t mask = static_cast<uint64_t>(1) << v;
			if (clause.pLiterals & mask) {
				//p_add++;
				BitClause newClause = clause;
				newClause.pLiterals &= ~mask;
				pBuckets[v].insert(newClause);
			}
			if (clause.nLiterals & mask) {
				//p_add++;
				BitClause newClause = clause;
				newClause.nLiterals &= ~mask;
				nBuckets[v].insert(newClause);
			}
		}
	}

	void removeClause(BitClause clause) {
		assert(pBuckets.size() == BitClause::VARIABLE_COUNT);
		assert(nBuckets.size() == BitClause::VARIABLE_COUNT);
		for (int v = 0; v < BitClause::VARIABLE_COUNT; v++) {
			uint64_t mask = static_cast<uint64_t>(1) << v;
			if (clause.pLiterals & mask) {
				//p_remove++;
				BitClause removedClause = clause;
				removedClause.pLiterals &= ~mask;
				pBuckets[v].erase(removedClause);
			}
			if (clause.nLiterals & mask) {
				//p_remove++;
				BitClause removedClause = clause;
				removedClause.nLiterals &= ~mask;
				nBuckets[v].erase(removedClause);
			}
		}
	}
};

namespace Resolution {

using VariableId = int;
using Literal = std::pair<VariableId, bool>;
using Clause = std::vector<Literal>;
using ClausePair = std::pair<BitClause, BitClause>;
using Graph = std::unordered_map<BitClause, ClausePair>;

struct ProofItem {
	BitClause clause;
	int index1;
	int index2;
};
using Proof = std::vector<ProofItem>;

const bool RECORD_GRAPH = false;

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

void cnfPropToVec(std::vector<Clause>& clauses, const std::shared_ptr<Proposition>& cnf) {
	if (cnf->getType() == Proposition::BINARY &&
		std::static_pointer_cast<BinaryOperator>(cnf)->getOp() == BinaryOperator::AND) {
		auto binaryProp = std::static_pointer_cast<BinaryOperator>(cnf);
		cnfPropToVec(clauses, binaryProp->getLeft());
		cnfPropToVec(clauses, binaryProp->getRight());
	}
	else {
		std::vector<Literal> clause;
		if(traverseClause(clause, cnf))
			clauses.push_back(clause);
	}
	// if there is no clause then proposition is True
	// if there is at least one empty clause then proposition is False
}

std::string getVariableName(VariableId id) {
	std::string result;
	const int CHARACTER_COUNT = 'z' - 'a' + 1;
	int character = id % CHARACTER_COUNT;
	int number = id / CHARACTER_COUNT;
	result += static_cast<char>('a' + character);
	if (number > 0)
		result += std::to_string(number);
	return result;
}

void printCnf(const std::vector<Clause>& clauses) {
	for (int i = 0; i < clauses.size(); i++) {
		std::cout << "(";
		auto literals = clauses[i];
		for (int j = 0; j < literals.size(); j++) {
			auto literal = literals[j];
			if (literal.second)
				std::cout << "~";
			VariableId id = literal.first;
			std::cout << getVariableName(id);
			if (j + 1 < literals.size())
				std::cout << " | ";
		}
		std::cout << ")";
		if (i + 1 < clauses.size())
			std::cout << " & ";
	}
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

void squeezeVariableIds(std::vector<Clause>& clauses) {
	std::map<VariableId, VariableId> map;
	for (auto& clause : clauses)
		for (auto& literal : clause)
			map[literal.first] = 0;

	VariableId newId = 0;
	for (auto& pair : map)
		pair.second = newId++;

	for (auto& clause : clauses)
		for (auto& literal : clause)
			literal.first = map[literal.first];
}

bool clausesToBitClauses(std::vector<BitClause>& bitClauses, const std::vector<Clause>& clauses) {
	for (auto& clause : clauses) {
		BitClause newClause;
		for (auto& literal : clause) {
			if (literal.first >= sizeof(uint64_t) * 8)
				return false;
			uint64_t mask = static_cast<uint64_t>(1) << literal.first;
			if (literal.second)
				newClause.nLiterals |= mask;
			else
				newClause.pLiterals |= mask;
		}
		if ((newClause.pLiterals & newClause.nLiterals) == 0) {
			bool notExists = true;
			for (auto existingClause : bitClauses)
				if (newClause == existingClause)
					notExists = false;
			if (notExists)
				bitClauses.push_back(newClause);
		}
	}
	for (int i = 0; i < bitClauses.size(); i++) {
		for (int j = 0; j < bitClauses.size(); j++) {
			if (bitClauses[j].isProperSubset(bitClauses[i])) {
				std::swap(bitClauses[i], bitClauses.back());
				bitClauses.pop_back();
				i--;
				break;
			}
		}
	}
	return true;
}

bool resolve(std::vector<BitClause>& target,
	         std::vector<BitClause>& removedClauses,
			 Graph& graph,
	         std::unordered_set<BitClause>& allClauses,
			 std::vector<BitClause>& mainClauses,
	         const std::vector<std::set<BitClause>>& pBuckets,
	         const std::vector<std::set<BitClause>>& nBuckets) {
	assert(pBuckets.size() == BitClause::VARIABLE_COUNT);
	assert(nBuckets.size() == BitClause::VARIABLE_COUNT);
	for (int v = 0; v < BitClause::VARIABLE_COUNT; v++) {
		const auto& pClauses = pBuckets[v];
		const auto& nClauses = nBuckets[v];
		for (auto pIt = pClauses.begin(); pIt != pClauses.end(); pIt++) {
			for (auto nIt = nClauses.begin(); nIt != nClauses.end(); nIt++) {
				//p_prod++;
				auto pClause = *pIt;
				auto nClause = *nIt;
				BitClause newClause;
				newClause.pLiterals = pClause.pLiterals | nClause.pLiterals;
				newClause.nLiterals = pClause.nLiterals | nClause.nLiterals;
				if ((newClause.pLiterals | newClause.nLiterals) == 0) {
					if (RECORD_GRAPH) {
						uint64_t mask = static_cast<uint64_t>(1) << v;
						pClause.pLiterals |= mask;
						nClause.nLiterals |= mask;
						graph[newClause] = ClausePair(pClause, nClause);
					}
					return true; // contradiction
				}
				if ((newClause.pLiterals & newClause.nLiterals) == 0) {
					//p_onecomp++;
					if (allClauses.find(newClause) == allClauses.end()) {
						//p_new++;
						allClauses.insert(newClause);
						bool add = true;
						for (int m = 0; m < mainClauses.size(); m++) {
							//p_set++;
							auto& mainClause = mainClauses[m];
							if (mainClause.isSubset(newClause)) {
								add = false;
								break;
							}
							if (newClause.isProperSubset(mainClause)) {
								removedClauses.push_back(mainClause);
								std::swap(mainClause, mainClauses.back());
								mainClauses.pop_back();
								m--;
							}
						}
						if (add) {
							//p_main++;
							mainClauses.push_back(newClause);
							target.push_back(newClause);
							if (RECORD_GRAPH) {
								uint64_t mask = static_cast<uint64_t>(1) << v;
								pClause.pLiterals |= mask;
								nClause.nLiterals |= mask;
								graph[newClause] = ClausePair(pClause, nClause);
							}
						}
					}
				}
			}
		}
	}
	return false;
}

bool resolve(Graph& graph, std::vector<BitClause> clauses) {
	std::unordered_set<BitClause> allClauses;
	std::vector<BitClause> mainClauses;
	BucketBuff procClauses;
	BucketBuff currClauses;
	std::vector<BitClause> removedClauses;

	for (int i = 0; i < clauses.size(); i++) {
		auto result = allClauses.insert(clauses[i]);
		if (!result.second) {
			std::swap(clauses[i], clauses.back());
			clauses.pop_back();
			i--;
		}
	}

	while (clauses.size()) {
		//std::cout << clauses.size() << " " << allClauses.size() << " " << mainClauses.size() << std::endl;
		for (auto clause : clauses)
			currClauses.addClause(clause);
		clauses.clear();

		for (auto clause : removedClauses) {
			currClauses.removeClause(clause);
			procClauses.removeClause(clause);
		}
		removedClauses.clear();

		if (resolve(clauses, removedClauses, graph, allClauses, mainClauses, currClauses.pBuckets, procClauses.nBuckets)) return true;
		if (resolve(clauses, removedClauses, graph, allClauses, mainClauses, procClauses.pBuckets, currClauses.nBuckets)) return true;
		if (resolve(clauses, removedClauses, graph, allClauses, mainClauses, currClauses.pBuckets, currClauses.nBuckets)) return true;

		/*std::unordered_set<BitClause> removedClausesSet;
		for (auto clause : removedClauses)
			removedClausesSet.insert(clause);
		for (int i = 0; i < clauses.size(); i++) {
			if (removedClausesSet.find(clauses[i]) != removedClausesSet.end()) {
				std::swap(clauses[i], clauses.back());
				clauses.pop_back();
				i--;
				removedClausesSet.erase(clauses[i]);
			}
		}
		removedClauses.clear();
		removedClauses.insert(removedClauses.end(), removedClausesSet.begin(), removedClausesSet.end());
		removedClausesSet.clear();*/

		for (int v = 0; v < BitClause::VARIABLE_COUNT; v++) {
			procClauses.pBuckets[v].insert(currClauses.pBuckets[v].begin(),
										   currClauses.pBuckets[v].end());
			currClauses.pBuckets[v].clear();
			procClauses.nBuckets[v].insert(currClauses.nBuckets[v].begin(),
										   currClauses.nBuckets[v].end());
			currClauses.nBuckets[v].clear();
		}

		/*std::cout << "p_prod: " << p_prod << std::endl;
		std::cout << "p_onecomp: " << p_onecomp << std::endl;
		std::cout << "p_new: " << p_new << std::endl;
		std::cout << "p_set: " << p_set << std::endl;
		std::cout << "p_main: " << p_main << std::endl;
		std::cout << "p_add: " << p_add << std::endl;
		std::cout << "p_remove: " << p_remove << std::endl;
		std::cout << std::endl;*/
	}

	return false;
}

int traverseProof(Proof& proof, const Graph& graph, BitClause clause) {
	auto it = graph.find(clause);
	ProofItem item;
	item.clause = clause;
	if (it != graph.end()) {
		ClausePair pair = it->second;
		item.index1 = traverseProof(proof, graph, pair.first);
		item.index2 = traverseProof(proof, graph, pair.second);
	}
	else {
		item.index1 = -1;
		item.index2 = -1;
	}
	proof.push_back(item);
	return proof.size() - 1;
}

std::string bitClauseToStr(BitClause clause) {
	std::string result;
	if ((clause.pLiterals | clause.nLiterals) == 0)
		return "F";
	const std::string op = " | ";
	for (int v = 0; v < BitClause::VARIABLE_COUNT; v++) {
		auto varName = getVariableName(v);
		uint64_t mask = static_cast<uint64_t>(1) << v;
		if (clause.pLiterals & mask)
			result += varName + op;
		if (clause.nLiterals & mask)
			result += std::string("~") + varName + op;
	}
	result.erase(result.length() - op.length());
	return result;
}

std::string renderProof(const Graph& graph) {
	Proof proof;
	traverseProof(proof, graph, BitClause());

	std::vector<std::string> lines;
	size_t lineMaxLength = 0;
	for (int i = 0; i < proof.size(); i++)
	{
		std::string line;
		line += std::to_string(i + 1) + ". ";
		if (proof[i].index1 == -1) {
			assert(proof[i].index2 == -1);
			line += std::string("(");
			line += bitClauseToStr(proof[i].clause);
			line += std::string(")");
		}
		else {
			line += std::string("(");
			line += bitClauseToStr(proof[proof[i].index1].clause);
			line += std::string(") & (");
			line += bitClauseToStr(proof[proof[i].index2].clause);
			line += std::string(") -> (");
			line += bitClauseToStr(proof[i].clause);
			line += std::string(")");
		}
		lines.push_back(line);
		lineMaxLength = std::max(lineMaxLength, line.length());
	}
	const int SPACE_SIZE = 4;
	std::string result;
	for (int i = 0; i < proof.size(); i++)
	{
		std::string line = lines[i];
		line.insert(line.length(), (lineMaxLength + SPACE_SIZE) - line.length(), ' ');
		if (proof[i].index1 == -1) {
			line += "[cnf transformation 0]";
		}
		else {
			line += std::string("[resolution ");
			line += std::to_string(proof[i].index1 + 1);
			line += std::string(", ");
			line += std::to_string(proof[i].index2 + 1);
			line += std::string("]");
		}
		result += line + "\n";
	}
	result += "Refutation found\n";
	return result;
}

bool isValid(const std::shared_ptr<Proposition>& proposition) {
	auto notProposition = std::make_shared<UnaryOperator>(proposition, UnaryOperator::NOT);
	return isContradiction(notProposition);
}

bool isContradiction(const std::shared_ptr<Proposition>& proposition) {
	auto cnf = proposition->toCnf();

	std::vector<Clause> clauses;
	cnfPropToVec(clauses, cnf);
	cnf = nullptr;

	//removeRedundancy(clauses);
	//clauses.shrink_to_fit();
	//printCnf(clauses);
	//std::cout << std::endl;

	if(!RECORD_GRAPH)
		squeezeVariableIds(clauses);
	std::vector<BitClause> bitClauses;
	if (!clausesToBitClauses(bitClauses, clauses))
		throw std::runtime_error("Variable id is greater than 63");
	clauses.clear();
	clauses.shrink_to_fit();

	auto start = std::chrono::high_resolution_clock::now();

	Graph graph;
	auto result = resolve(graph, bitClauses);

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	if (RECORD_GRAPH && result) {
		std::string str = "Proof by refutation:\n";
		str += "0. ";
		Converter converter;
		str += converter.toString(proposition) + "\n";
		str += renderProof(graph);
		std::cout << str;
	}
	std::cout << "Elapsed time: " << (float)duration.count() / 1000 << "s" << std::endl;

	return result;
}

} // namespace Resolution
