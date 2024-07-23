#include "Resolution.hpp"

#include "Variable.hpp"
#include "Constant.hpp"
#include "UnaryOperator.hpp"
#include "BinaryOperator.hpp"
#include "NormalForm.hpp"
#include "Converter.hpp"

#include <vector>
#include <cassert>
#include <string>
//#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <chrono>
//#include <bit>

// TODO: This module requires refactoring (after optimizations?)!

const bool RECORD_GRAPH = true;

//size_t p_iter = 0;
//size_t p_prod = 0;
//size_t p_current = 0;
//size_t p_exists = 0;
//size_t p_removetest = 0;
//size_t p_proc = 0;
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
		uint64_t literalsLhs = pLiterals | nLiterals;
		uint64_t literalsRhs = rhs.pLiterals | rhs.nLiterals;
		if(literalsLhs != literalsRhs)
			return literalsLhs < literalsRhs;
		return pLiterals < rhs.pLiterals;
		/*if (pLiterals != rhs.pLiterals)
			return pLiterals < rhs.pLiterals;
		return nLiterals < rhs.nLiterals;*/
	}

	bool empty() const {
		return (pLiterals | nLiterals) == 0;
	}

	bool isSubset(const BitClause& rhs) const {
		return (pLiterals & ~rhs.pLiterals) == 0 &&
			(nLiterals & ~rhs.nLiterals) == 0;
	}

	bool isProperSubset(const BitClause& rhs) const {
		return isSubset(rhs) && *this != rhs;
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
	std::vector<std::unordered_set<BitClause>> pBuckets;
	std::vector<std::unordered_set<BitClause>> nBuckets;

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

using ClausePair = std::pair<BitClause, BitClause>;
using Graph = std::unordered_map<BitClause, ClausePair>;

struct ProofItem {
	ProofItem(BitClause clause = BitClause(), int index1 = -1, int index2 = -1) :
		clause(clause), index1(index1), index2(index2) {}

	BitClause clause;
	int index1;
	int index2;
};
using Proof = std::vector<ProofItem>;

bool clausesToBitClauses(std::vector<BitClause>& bitClauses, const Cnf& clauses) {
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

bool resolve(std::set<BitClause>& unprocClauses, Graph& graph,
	         const BucketBuff& procClauses, BitClause clause) {
	for (int v = 0; v < BitClause::VARIABLE_COUNT; v++) {
		const uint64_t mask = static_cast<uint64_t>(1) << v;
		if (mask & (clause.pLiterals | clause.nLiterals)) {
			const bool positive = mask & clause.pLiterals;
			const auto& bucket = positive ? procClauses.nBuckets[v] : procClauses.pBuckets[v];
			for (auto it = bucket.begin(); it != bucket.end(); it++) {
				//p_prod++;
				auto procClause = *it;
				auto maskedClause = clause;
				(positive ? maskedClause.pLiterals : maskedClause.nLiterals) &= ~mask;
				BitClause newClause;
				newClause.pLiterals = procClause.pLiterals | maskedClause.pLiterals;
				newClause.nLiterals = procClause.nLiterals | maskedClause.nLiterals;
				if ((newClause.pLiterals & newClause.nLiterals) == 0) {
					//p_current++;
					unprocClauses.insert(newClause);
					if (RECORD_GRAPH) {
						(positive ? procClause.nLiterals : procClause.pLiterals) |= mask;
						graph.emplace(newClause, ClausePair(procClause, clause));
					}
					if (newClause.empty())
						return true; // contradiction
				}
			}
		}
	}
	return false;
}

bool resolve(Graph& graph, const std::vector<BitClause>& clauses) {
	std::vector<BitClause> procClauses;
	BucketBuff procClausesB;
	std::set<BitClause> unprocClauses;

	for (auto clause : clauses) {
		unprocClauses.insert(clause);
		if (RECORD_GRAPH)
			graph[clause] = ClausePair();
	}

	while (unprocClauses.size()) {
		//p_iter++;
		auto it = unprocClauses.begin();
		BitClause clause = *it;
		unprocClauses.erase(it);

		bool add = true;
		for (int i = 0; i < procClauses.size(); i++) {
			//p_exists++;
			if (procClauses[i].isSubset(clause)) {
				add = false;
				break;
			}
		}
		if (add) {
			//p_proc++;
			for (int i = 0; i < procClauses.size(); i++) {
				//p_removetest++;
				auto& procClause = procClauses[i];
				if (clause.isProperSubset(procClause)) {
					procClausesB.removeClause(procClause);
					std::swap(procClause, procClauses.back());
					procClauses.pop_back();
					i--;
				}
			}
			if (resolve(unprocClauses, graph, procClausesB, clause))
				return true;
			procClausesB.addClause(clause);
			procClauses.push_back(clause);
		}
	}

	return false;
}

int traverseProof(Proof& proof, const Graph& graph, BitClause clause = BitClause()) {
	for (int i = 0; i < proof.size(); i++)
		if (proof[i].clause == clause)
			return i;
	ProofItem item(clause);
	auto it = graph.find(clause);
	assert(it != graph.end());
	const ClausePair& pair = it->second;
	if (!pair.first.empty()) {
		assert(!pair.second.empty());
		item.index1 = traverseProof(proof, graph, pair.first);
		item.index2 = traverseProof(proof, graph, pair.second);
	}
	proof.push_back(item);
	return proof.size() - 1;
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
	traverseProof(proof, graph);

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
	return result;
}

bool isValid(const PropositionSP& proposition, std::string* proof) {
	auto notProposition = std::make_shared<UnaryOperator>(proposition, UnaryOperator::NOT);
	bool result = isContradiction(notProposition, proof);
	if (proof)
		*proof = "Proof by refutation:\n" + *proof;
	return result;
}

bool isContradiction(const PropositionSP& proposition, std::string* proof) {
	std::vector<Clause> clauses;
	propositionToCnf(clauses, proposition);
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
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	/*std::cout << "p_iter: " << p_iter << std::endl;
	std::cout << "p_prod: " << p_prod << std::endl;
	std::cout << "p_current: " << p_current << std::endl;
	std::cout << "p_exists: " << p_exists << std::endl;
	std::cout << "p_removetest: " << p_removetest << std::endl;
	std::cout << "p_proc: " << p_proc << std::endl;
	std::cout << "p_add: " << p_add << std::endl;
	std::cout << "p_remove: " << p_remove << std::endl;
	std::cout << std::endl;*/

	if (proof) {
		if (result) {
			std::string str;
			if (RECORD_GRAPH) {
				str = "0. ";
				Converter converter;
				str += converter.toString(proposition) + "\n";
				str += renderProof(graph);
			}
			*proof += str;
			*proof += "Contradiction found\n";
			if(!RECORD_GRAPH)
				*proof += "Proof unavailable\n";
		}
		else
			*proof += "Contradiction not found\n";
		*proof += "Elapsed time: ";
		*proof += std::to_string((double)duration.count() / 1000000) + "s\n";
	}

	return result;
}

} // namespace Resolution
