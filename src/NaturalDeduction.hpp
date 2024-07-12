#pragma once

#include "Proposition.hpp"

#include <vector>
#include <string>
#include <set>

struct InferenceRule {
	std::vector<PropositionSP> premises;
	PropositionSP conclusion;
	std::string name;
	// premise1 & premise2 & ... -> conclusion
};

struct PropositionItem {
	const PropositionSP proposition;
	const std::string infRuleName;
	const int src1;
	const int src2;

	PropositionItem(PropositionSP proposition, std::string infRuleName = "", int src1 = -1, int src2 = -1);
	bool operator<(const PropositionItem& rhs) const;

private:
	static int idCounter;
	const int itemId;
	const int propLength;
};

class NaturalDeduction {
public:
	NaturalDeduction();
	virtual ~NaturalDeduction() = default;

	void addJasInferenceRules();
	void addInferenceRule(const InferenceRule& rule);

	void addPremise(PropositionSP premise);
	void setConclusion(PropositionSP conclusion);

	bool step(); // continue as long as it returns true
	bool isProofFound() const;

	std::vector<PropositionItem> getFullGraph() const;
	std::vector<PropositionItem> getProofGraph(PropositionSP conclusion = nullptr) const;
	std::string getProofString() const;

protected:
	std::vector<InferenceRule> unaryInferenceRules;
	std::vector<InferenceRule> binaryInferenceRules;

	std::vector<PropositionItem> procPropositions;
	std::set<PropositionItem> unprocPropositions;
	PropositionSP conclusion;
	bool proofFound;

	void addProposition(PropositionItem item);
	inline PropositionSP applyRule(const InferenceRule& rule, PropositionSP prop) const;
	inline PropositionSP applyRule(const InferenceRule& rule, PropositionSP prop1, PropositionSP prop2) const;
	bool traverseRulePremise(std::vector<PropositionSP>& substTable, PropositionSP rule, PropositionSP prop) const;
	PropositionSP traverseRuleConclusion(const std::vector<PropositionSP>& substTable, PropositionSP rule) const;
};
