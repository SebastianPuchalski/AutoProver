#pragma once

#include <vector>
#include <queue>

class ForwardChainingKB {
public:
	using Literal = unsigned; // positive literals only
	struct DefiniteClause {
		DefiniteClause(std::vector<Literal> body, Literal head) :
			body(body), head(head) {}
		std::vector<Literal> body;
		Literal head;
		// b0 & b1 & ... -> h
	};

	virtual ~ForwardChainingKB() = default;

	void addFact(Literal fact);
	void addFacts(const std::vector<Literal>& facts);
	void addClause(const DefiniteClause& clause);
	void addClauses(const std::vector<DefiniteClause>& clauses);
	bool isEntailed(Literal query);

protected:
	using Quantity = unsigned;
	using Index = unsigned;

	std::queue<Literal> facts;
	std::vector<std::pair<DefiniteClause, Quantity>> clauses;
	std::vector<std::pair<std::vector<Index>, bool>> buckets;

	inline void extendBucketCount(Literal literal);
};
