#include "Resolution.hpp"

#include "Variable.hpp"
#include "Constant.hpp"
#include "UnaryOperator.hpp"
#include "BinaryOperator.hpp"

#include <vector>
#include <cassert>
#include <string>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <chrono>

struct BitClause {
	static const int VARIABLE_COUNT = sizeof(uint64_t) * 8;
	uint64_t pLiterals;
	uint64_t nLiterals;

	BitClause() : pLiterals(0), nLiterals(0) {}

	bool operator==(const BitClause& rhs) const {
		return pLiterals == rhs.pLiterals &&
			nLiterals == rhs.nLiterals;
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
	std::vector<std::vector<BitClause>> pBuckets;
	std::vector<std::vector<BitClause>> nBuckets;

	BucketBuff() : pBuckets(BitClause::VARIABLE_COUNT),
		           nBuckets(BitClause::VARIABLE_COUNT) {}

	void addClause(BitClause clause) {
		assert(pBuckets.size() == BitClause::VARIABLE_COUNT);
		assert(nBuckets.size() == BitClause::VARIABLE_COUNT);
		for (int v = 0; v < BitClause::VARIABLE_COUNT; v++) {
			uint64_t mask = static_cast<uint64_t>(1) << v;
			if (clause.pLiterals & mask) {
				BitClause newClause = clause;
				newClause.pLiterals &= ~mask;
				pBuckets[v].push_back(newClause);
			}
			if (clause.nLiterals & mask) {
				BitClause newClause = clause;
				newClause.nLiterals &= ~mask;
				nBuckets[v].push_back(newClause);
			}
		}
	}
};

namespace Resolution {

typedef int VariableId;
typedef std::pair<VariableId, bool> Literal;
typedef std::vector<Literal> Clause;
typedef std::pair<BitClause, BitClause> ClausePair;
typedef std::unordered_map<BitClause, ClausePair> Graph;

struct ProofItem {
	BitClause clause;
	int index1;
	int index2;
};
typedef std::vector<ProofItem> Proof;

const bool RECORD_GRAPH = true;

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
		BitClause bitClause;
		for (auto& literal : clause) {
			if (literal.first >= sizeof(uint64_t) * 8)
				return false;
			uint64_t mask = static_cast<uint64_t>(1) << literal.first;
			if (literal.second)
				bitClause.nLiterals |= mask;
			else
				bitClause.pLiterals |= mask;
		}
		if ((bitClause.pLiterals & bitClause.nLiterals) == 0)
			bitClauses.push_back(bitClause);
	}
	return true;
}

bool resolve(std::vector<BitClause>& target,
	         std::unordered_set<BitClause>& allClauses, Graph& graph,
	         const std::vector<std::vector<BitClause>>& pBuckets,
	         const std::vector<std::vector<BitClause>>& nBuckets) {
	assert(pBuckets.size() == BitClause::VARIABLE_COUNT);
	assert(nBuckets.size() == BitClause::VARIABLE_COUNT);
	for (int v = 0; v < BitClause::VARIABLE_COUNT; v++) {
		const std::vector<BitClause>& pClauses = pBuckets[v];
		const std::vector<BitClause>& nClauses = nBuckets[v];
		const size_t TILE_SIZE = 256;
		for (int i = 0; i < pClauses.size(); i += TILE_SIZE) {
			for (int j = 0; j < nClauses.size(); j += TILE_SIZE) {
				int xEnd = std::min(i + TILE_SIZE, pClauses.size());
				int yEnd = std::min(j + TILE_SIZE, nClauses.size());
				for (int x = i; x < xEnd; x++) {
					for (int y = j; y < yEnd; y++) {
						auto pClause = pClauses[x];
						auto nClause = nClauses[y];
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
						if ((newClause.pLiterals & newClause.nLiterals) == 0) { // make sure it's ok
							if (allClauses.find(newClause) == allClauses.end()) {
								allClauses.insert(newClause);
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
	}
	return false;
}

bool resolve(Graph& graph, std::vector<BitClause> clauses) {
	std::unordered_set<BitClause> allClauses;
	BucketBuff procClauses;
	BucketBuff currClauses;

	for (int i = 0; i < clauses.size(); i++) {
		auto result = allClauses.insert(clauses[i]);
		if (!result.second) {
			std::swap(clauses[i], clauses.back());
			clauses.pop_back();
			i--;
		}
	}

	while (clauses.size()) {
		for (auto clause : clauses)
			currClauses.addClause(clause);
		clauses.clear();

		if (resolve(clauses, allClauses, graph, currClauses.pBuckets, procClauses.nBuckets)) return true;
		if (resolve(clauses, allClauses, graph, procClauses.pBuckets, currClauses.nBuckets)) return true;
		if (resolve(clauses, allClauses, graph, currClauses.pBuckets, currClauses.nBuckets)) return true;

		for (int v = 0; v < BitClause::VARIABLE_COUNT; v++) {
			procClauses.pBuckets[v].insert(procClauses.pBuckets[v].end(),
										   currClauses.pBuckets[v].begin(),
										   currClauses.pBuckets[v].end());
			currClauses.pBuckets[v].clear();
			procClauses.nBuckets[v].insert(procClauses.nBuckets[v].end(),
										   currClauses.nBuckets[v].begin(),
										   currClauses.nBuckets[v].end());
			currClauses.nBuckets[v].clear();
		}
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

	std::string result;
	for (int i = 0; i < proof.size(); i++)
	{
		std::string line;
		line += std::to_string(i + 1) + ". ";
		if (proof[i].index1 == -1) {
			assert(proof[i].index2 == -1);
			line += "CNF: ";
		}
		else {
			line += std::to_string(proof[i].index1 + 1);
			line += std::string(" and ");
			line += std::to_string(proof[i].index2 + 1);
			line += ": ";
		}
		line.insert(line.length(), 25 - line.length(), ' ');
		line += bitClauseToStr(proof[i].clause) + "\n";
		result += line;
	}
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
	std::cout << "Duration: " << (float)duration.count() / 1000 << std::endl;

	if (result)
		std::cout << renderProof(graph);

	return result;
}

} // namespace Resolution
