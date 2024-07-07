#include "ForwardChaining.hpp"

#include <cassert>

void ForwardChainingKB::addFact(Literal fact) {
	extendBucketCount(fact);
	facts.push(fact);
}

void ForwardChainingKB::addFacts(const std::vector<Literal>& facts) {
	for (auto fact : facts)
		addFact(fact);
}

void ForwardChainingKB::addClause(const DefiniteClause& clause) {
	unsigned quantity = 0;
	extendBucketCount(clause.head);
	if (!buckets[clause.head].second) {
		for (auto literal : clause.body) {
			extendBucketCount(literal);
			if (!buckets[literal].second) {
				quantity++;
				Index index = clauses.size();
				buckets[literal].first.push_back(index);
			}
		}
		if (quantity == 0)
			facts.push(clause.head);
	}
	clauses.push_back(std::pair(clause, quantity));
}

void ForwardChainingKB::addClauses(const std::vector<DefiniteClause>& clauses) {
	for (auto& clause : clauses)
		addClause(clause);
}

bool ForwardChainingKB::isEntailed(Literal query) {
	if (query >= buckets.size())
		return false;
	if(buckets[query].second)
		return true;
	while (!facts.empty()) {
		auto fact = facts.front();
		if (fact == query)
			return true;
		facts.pop();
		bool queryFound = false;
		if (!buckets[fact].second) {
			for (auto index : buckets[fact].first) {
				assert(clauses[index].second > 0);
				clauses[index].second--;
				if (clauses[index].second == 0) {
					auto conclusion = clauses[index].first.head;
					facts.push(conclusion);
					if (conclusion == query)
						queryFound = true;
				}
			}
			buckets[fact].second = true;
			buckets[fact].first.clear();
		}
		if (queryFound)
			return true;
	}
	return false;
}

void ForwardChainingKB::extendBucketCount(Literal literal) {
	if (literal >= buckets.size())
		buckets.resize(literal + 1, std::pair(std::vector<Index>(), false));
}
