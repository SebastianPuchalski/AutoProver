#include "NaturalDeduction.hpp"

#include "Converter.hpp"

#include <cassert>
#include <queue>

const size_t MAX_QUEUE_SIZE = 50000000;

PropositionItem::PropositionItem(PropositionSP proposition, std::string infRuleName, int src1, int src2) :
	proposition(proposition), infRuleName(infRuleName), src1(src1), src2(src2),
	itemId(idCounter++), propLength(proposition->getLength()) {}

bool PropositionItem::operator<(const PropositionItem& rhs) const {
	if (propLength != rhs.propLength)
		return propLength < rhs.propLength;
	if (proposition->isEquivalent(rhs.proposition))
		return false;
	return itemId < rhs.itemId;
}

int PropositionItem::idCounter = 0;

NaturalDeduction::NaturalDeduction() : proofFound(false) {}

void NaturalDeduction::addJasInferenceRules() {
	Converter converter;
	const bool org = true;

	InferenceRule ro;
	ro.premises.push_back(converter.fromString("a -> b"));
	ro.premises.push_back(converter.fromString("a"));
	ro.conclusion = converter.fromString("b");
	ro.name = "RO";
	addInferenceRule(ro);

	InferenceRule dk;
	dk.premises.push_back(converter.fromString("a"));
	dk.premises.push_back(converter.fromString("b"));
	dk.conclusion = converter.fromString("a & b");
	dk.name = "DK";
	addInferenceRule(dk);

	InferenceRule ok1;
	ok1.premises.push_back(converter.fromString("a & b"));
	ok1.conclusion = converter.fromString("a");
	ok1.name = "OK";
	addInferenceRule(ok1);

	InferenceRule ok2;
	ok2.premises.push_back(converter.fromString("a & b"));
	ok2.conclusion = converter.fromString("b");
	ok2.name = "OK";
	addInferenceRule(ok2);

	InferenceRule da1;
	da1.premises.push_back(converter.fromString("a"));
	da1.premises.push_back(converter.fromString("b"));
	da1.conclusion = converter.fromString("a | b");
	da1.name = "DA";
	addInferenceRule(da1);

	InferenceRule da2;
	da2.premises.push_back(converter.fromString("b"));
	da2.premises.push_back(converter.fromString("a"));
	da2.conclusion = converter.fromString("a | b");
	da2.name = "DA";
	addInferenceRule(da2);

	if (!org) {
		InferenceRule oa1;
		oa1.premises.push_back(converter.fromString("a | b"));
		oa1.premises.push_back(converter.fromString("~a"));
		oa1.conclusion = converter.fromString("b");
		oa1.name = "OA";
		addInferenceRule(oa1);

		InferenceRule oa2;
		oa2.premises.push_back(converter.fromString("a | b"));
		oa2.premises.push_back(converter.fromString("~b"));
		oa2.conclusion = converter.fromString("a");
		oa2.name = "OA";
		addInferenceRule(oa2);
	}

	InferenceRule dr;
	dr.premises.push_back(converter.fromString("a -> b"));
	dr.premises.push_back(converter.fromString("b -> a"));
	dr.conclusion = converter.fromString("a <-> b");
	dr.name = "DR";
	addInferenceRule(dr);

	InferenceRule or1;
	or1.premises.push_back(converter.fromString("a <-> b"));
	or1.conclusion = converter.fromString("a -> b");
	or1.name = "OR";
	addInferenceRule(or1);

	InferenceRule or2;
	or2.premises.push_back(converter.fromString("a <-> b"));
	or2.conclusion = converter.fromString("b -> a");
	or2.name = "OR";
	addInferenceRule(or2);

	if (org) {
		InferenceRule rk;
		rk.premises.push_back(converter.fromString("~a -> ~b"));
		rk.conclusion = converter.fromString("b -> a");
		rk.name = "RK";
		addInferenceRule(rk);

		InferenceRule dp;
		dp.premises.push_back(converter.fromString("a -> c"));
		dp.premises.push_back(converter.fromString("b -> c"));
		dp.conclusion = converter.fromString("(a | b) -> c");
		dp.name = "DP";
		addInferenceRule(dp);
	}
}

void NaturalDeduction::addCustomInferenceRules() {
	Converter converter;

	// Modus Ponens (MP)
	InferenceRule mp;
	mp.premises.push_back(converter.fromString("a -> b"));
	mp.premises.push_back(converter.fromString("a"));
	mp.conclusion = converter.fromString("b");
	mp.name = "MP";
	addInferenceRule(mp);

	// Modus Tollens (MT)
	InferenceRule mt;
	mt.premises.push_back(converter.fromString("a -> b"));
	mt.premises.push_back(converter.fromString("~b"));
	mt.conclusion = converter.fromString("~a");
	mt.name = "MT";
	addInferenceRule(mt);

	// Hypothetical Syllogism (HS)
	InferenceRule hs;
	hs.premises.push_back(converter.fromString("a -> b"));
	hs.premises.push_back(converter.fromString("b -> c"));
	hs.conclusion = converter.fromString("a -> c");
	hs.name = "HS";
	addInferenceRule(hs);

	// Disjunction Introduction (DI) - modified
	InferenceRule di;
	di.premises.push_back(converter.fromString("a"));
	di.premises.push_back(converter.fromString("b"));
	di.conclusion = converter.fromString("a | b");
	di.name = "DI";
	addInferenceRule(di);

	// Conjunction Introduction (CI)
	InferenceRule ci;
	ci.premises.push_back(converter.fromString("a"));
	ci.premises.push_back(converter.fromString("b"));
	ci.conclusion = converter.fromString("a & b");
	ci.name = "CI";
	addInferenceRule(ci);

	// Conjunction Elimination (CE1)
	InferenceRule ce1;
	ce1.premises.push_back(converter.fromString("a & b"));
	ce1.conclusion = converter.fromString("a");
	ce1.name = "CE1";
	addInferenceRule(ce1);

	// Conjunction Elimination (CE2)
	InferenceRule ce2;
	ce2.premises.push_back(converter.fromString("a & b"));
	ce2.conclusion = converter.fromString("b");
	ce2.name = "CE2";
	addInferenceRule(ce2);

	// Disjunction Elimination (DE)
	InferenceRule de;
	de.premises.push_back(converter.fromString("a | b"));
	de.premises.push_back(converter.fromString("a -> c"));
	de.premises.push_back(converter.fromString("b -> c"));
	de.conclusion = converter.fromString("c");
	de.name = "DE";
	addInferenceRule(de);

	// Double Negation (DN1)
	InferenceRule dn1;
	dn1.premises.push_back(converter.fromString("a"));
	dn1.conclusion = converter.fromString("~~a");
	dn1.name = "DN1";
	addInferenceRule(dn1);

	// Double Negation (DN2)
	InferenceRule dn2;
	dn2.premises.push_back(converter.fromString("~~a"));
	dn2.conclusion = converter.fromString("a");
	dn2.name = "DN2";
	addInferenceRule(dn2);

	// De Morgan's Laws (DM1a)
	InferenceRule dm1a;
	dm1a.premises.push_back(converter.fromString("~(a & b)"));
	dm1a.conclusion = converter.fromString("~a | ~b");
	dm1a.name = "DM1a";
	addInferenceRule(dm1a);

	// De Morgan's Laws (DM1b)
	InferenceRule dm1b;
	dm1b.premises.push_back(converter.fromString("~a | ~b"));
	dm1b.conclusion = converter.fromString("~(a & b)");
	dm1b.name = "DM1b";
	addInferenceRule(dm1b);

	// De Morgan's Laws (DM2a)
	InferenceRule dm2a;
	dm2a.premises.push_back(converter.fromString("~(a | b)"));
	dm2a.conclusion = converter.fromString("~a & ~b");
	dm2a.name = "DM2a";
	addInferenceRule(dm2a);

	// De Morgan's Laws (DM2b)
	InferenceRule dm2b;
	dm2b.premises.push_back(converter.fromString("~a & ~b"));
	dm2b.conclusion = converter.fromString("~(a | b)");
	dm2b.name = "DM2b";
	addInferenceRule(dm2b);

	// Exportation (EX1)
	InferenceRule ex1;
	ex1.premises.push_back(converter.fromString("(a & b) -> c"));
	ex1.conclusion = converter.fromString("a -> (b -> c)");
	ex1.name = "EX1";
	addInferenceRule(ex1);

	// Exportation (EX2)
	InferenceRule ex2;
	ex2.premises.push_back(converter.fromString("a -> (b -> c)"));
	ex2.conclusion = converter.fromString("(a & b) -> c");
	ex2.name = "EX2";
	addInferenceRule(ex2);

	// Equivalence (EQ1)
	InferenceRule eq1;
	eq1.premises.push_back(converter.fromString("a <-> b"));
	eq1.conclusion = converter.fromString("(a -> b) & (b -> a)");
	eq1.name = "EQ1";
	addInferenceRule(eq1);

	// Equivalence (EQ2)
	InferenceRule eq2;
	eq2.premises.push_back(converter.fromString("(a -> b) & (b -> a)"));
	eq2.conclusion = converter.fromString("a <-> b");
	eq2.name = "EQ2";
	addInferenceRule(eq2);

	// Equivalence Associativity - redundant?
	InferenceRule eqAssoc;
	eqAssoc.premises.push_back(converter.fromString("(a <-> b) <-> c"));
	eqAssoc.conclusion = converter.fromString("a <-> (b <-> c)");
	eqAssoc.name = "EQ_ASSOC";
	addInferenceRule(eqAssoc);
}

void NaturalDeduction::addInferenceRule(const InferenceRule& rule) {
	assert(rule.premises.size() == 1 || rule.premises.size() == 2);
	if (rule.premises.size() == 1)
		unaryInferenceRules.push_back(rule);
	if (rule.premises.size() == 2)
		binaryInferenceRules.push_back(rule);
}

void NaturalDeduction::addPremise(PropositionSP premise) {
	addProposition(PropositionItem(premise));
}

void NaturalDeduction::setConclusion(PropositionSP conclusion) {
	this->conclusion = conclusion;
	proofFound = false;
}

bool NaturalDeduction::step() {
	if (unprocPropositions.empty() || proofFound)
		return false;
	auto unprocItem = *unprocPropositions.begin();
	auto unprocProp = unprocItem.proposition;
	auto unprocIndex = procPropositions.size();
	if (conclusion && unprocProp->isEquivalent(conclusion)) {
		proofFound = true;
		return false;
	}
	unprocPropositions.erase(unprocPropositions.begin());

	for (auto& procItem : procPropositions)
		if (procItem.proposition->isEquivalent(unprocProp))
			return true;

	for (auto& rule : unaryInferenceRules) {
		auto result = applyRule(rule, unprocProp);
		if(result)
			addProposition(PropositionItem(result, rule.name, unprocIndex));
	}

	for(int procIndex = 0; procIndex < procPropositions.size(); procIndex++) {
		for (auto& rule : binaryInferenceRules) {
			PropositionSP procProp = procPropositions[procIndex].proposition;
			auto result1 = applyRule(rule, procProp, unprocProp);
			if (result1)
				addProposition(PropositionItem(result1, rule.name, procIndex, unprocIndex));
			auto result2 = applyRule(rule, unprocProp, procProp);
			if (result2)
				addProposition(PropositionItem(result2, rule.name, unprocIndex, procIndex));
		}
	}

	procPropositions.push_back(unprocItem);
	return !proofFound;
}

bool NaturalDeduction::isProofFound() const {
	return proofFound;
}

std::vector<PropositionItem> NaturalDeduction::getFullGraph() const {
	std::vector<PropositionItem> fullGraph;
	for (auto& item : procPropositions)
		fullGraph.push_back(item);
	for (auto& item : unprocPropositions)
		fullGraph.push_back(item);
	return fullGraph;
}

std::vector<PropositionItem> NaturalDeduction::getProofGraph(PropositionSP conclusion) const {
	if (!conclusion)
		conclusion = this->conclusion;
	auto graph = getFullGraph();
	int proofIndex = -1;
	for (int i = 0; i < graph.size(); i++)
		if (graph[i].proposition->isEquivalent(conclusion))
			proofIndex = i;
	if (proofIndex == -1)
		return std::vector<PropositionItem>();

	std::vector<int> newIndexes(graph.size(), -1);
	std::queue<int> queue;
	queue.push(proofIndex);
	while (!queue.empty()) {
		int index = queue.front();
		queue.pop();
		assert(index < graph.size());
		if (newIndexes[index] < 0) {
			newIndexes[index] = 0;
			auto& item = graph[index];
			if (item.src1 >= 0)
				queue.push(item.src1);
			if (item.src2 >= 0)
				queue.push(item.src2);
		}
	}
	std::vector<PropositionItem> proofGraph;
	for (int i = 0; i <= proofIndex; i++) {
		if (newIndexes[i] == 0) {
			newIndexes[i] = proofGraph.size();
			auto& item = graph[i];
			int src1 = item.src1;
			int src2 = item.src2;
			if (src1 >= 0) src1 = newIndexes[item.src1];
			if (src2 >= 0) src2 = newIndexes[item.src2];
			PropositionItem proofItem(item.proposition, item.infRuleName, src1, src2);
			proofGraph.push_back(proofItem);
		}
	}
	return proofGraph;
}

std::string NaturalDeduction::getProofString() const {
	std::vector<PropositionItem> proof = getProofGraph();
	Converter converter;

	std::vector<std::string> lines;
	size_t lineMaxLength = 0;
	for (int i = 0; i < proof.size(); i++)
	{
		std::string line;
		line += std::to_string(i + 1) + ". ";
		line += converter.toString(proof[i].proposition);
		lines.push_back(line);
		lineMaxLength = std::max(lineMaxLength, line.length());
	}

	const int SPACE_SIZE = 4;
	std::string result;
	for (int i = 0; i < proof.size(); i++)
	{
		auto& line = lines[i];
		auto& item = proof[i];
		line.insert(line.length(), (lineMaxLength + SPACE_SIZE) - line.length(), ' ');
		if (item.infRuleName.empty()) {
			line += "Premise";
		}
		else {
			line += item.infRuleName + " ";
			if (item.src1 >= 0) {
				line += std::to_string(item.src1 + 1);
				if (item.src2 >= 0) {
					line += std::string(", ");
					line += std::to_string(item.src2 + 1);
				}
			}
		}
		result += line + "\n";
	}
	return result;
}

void NaturalDeduction::addProposition(PropositionItem item) {
	unprocPropositions.insert(item);
	if (conclusion && item.proposition->isEquivalent(conclusion))
		proofFound = true;
	if(unprocPropositions.size() > MAX_QUEUE_SIZE)
		unprocPropositions.erase(std::prev(unprocPropositions.end()));
}

inline PropositionSP NaturalDeduction::applyRule(
	const InferenceRule& rule, PropositionSP prop) const {
	assert(rule.premises.size() == 1);
	const int MAX_VARIABLE_ID = 7;
	std::vector<PropositionSP> substTable(MAX_VARIABLE_ID + 1);
	if (!traverseRulePremise(substTable, rule.premises[0], prop))
		return nullptr;
	return traverseRuleConclusion(substTable, rule.conclusion);
}

inline PropositionSP NaturalDeduction::applyRule(
	const InferenceRule& rule, PropositionSP prop1, PropositionSP prop2) const {
	assert(rule.premises.size() == 2);
	const int MAX_VARIABLE_ID = 7;
	std::vector<PropositionSP> substTable(MAX_VARIABLE_ID + 1);
	if (!traverseRulePremise(substTable, rule.premises[0], prop1))
		return nullptr;
	if (!traverseRulePremise(substTable, rule.premises[1], prop2))
		return nullptr;
	return traverseRuleConclusion(substTable, rule.conclusion);
}

bool NaturalDeduction::traverseRulePremise(std::vector<PropositionSP>& substTable,
	                                  PropositionSP rule, PropositionSP prop) const {
	assert(rule && prop);
	if (rule->getType() != Proposition::VARIABLE && rule->getType() != prop->getType())
		return false;
	switch (rule->getType()) {
	case Proposition::VARIABLE:
	{
		auto variableRule = std::static_pointer_cast<Variable>(rule);
		assert(variableRule->getId() < substTable.size());
		auto& subst = substTable[variableRule->getId()];
		if (subst)
			if (!subst->isEquivalent(prop))
				return false;
		subst = prop;
		break;
	}
	case Proposition::CONSTANT:
	{
		auto constRule = std::static_pointer_cast<Constant>(rule);
		auto constProp = std::static_pointer_cast<Constant>(prop);
		if (constRule->getValue() != constProp->getValue())
			return false;
		break;
	}
	case Proposition::UNARY:
	{
		auto unaryRule = std::static_pointer_cast<UnaryOperator>(rule);
		auto unaryProp = std::static_pointer_cast<UnaryOperator>(prop);
		if (unaryRule->getOp() != unaryProp->getOp())
			return false;
		if (!traverseRulePremise(substTable, unaryRule->getOperand(), unaryProp->getOperand()))
			return false;
		break;
	}
	case Proposition::BINARY:
	{
		auto binaryRule = std::static_pointer_cast<BinaryOperator>(rule);
		auto binaryProp = std::static_pointer_cast<BinaryOperator>(prop);
		if (binaryRule->getOp() != binaryProp->getOp())
			return false;
		if (!traverseRulePremise(substTable, binaryRule->getLeft(), binaryProp->getLeft()))
			return false;
		if (!traverseRulePremise(substTable, binaryRule->getRight(), binaryProp->getRight()))
			return false;
		break;
	}
	}
	return true;
}

PropositionSP NaturalDeduction::traverseRuleConclusion(const std::vector<PropositionSP>& substTable,
	                                            PropositionSP rule) const {
	assert(rule);
	switch (rule->getType()) {
	case Proposition::VARIABLE:
		assert(std::static_pointer_cast<Variable>(rule)->getId() < substTable.size());
		return substTable[std::static_pointer_cast<Variable>(rule)->getId()];
	case Proposition::CONSTANT:
		return rule->copy();
	case Proposition::UNARY:
	{
		auto unaryRule = std::static_pointer_cast<UnaryOperator>(rule);
		auto result = traverseRuleConclusion(substTable, unaryRule->getOperand());
		if (!result)
			return nullptr;
		return std::make_shared<UnaryOperator>(result, unaryRule->getOp());
	}
	case Proposition::BINARY:
	{
		auto binaryRule = std::static_pointer_cast<BinaryOperator>(rule);
		auto left = traverseRuleConclusion(substTable, binaryRule->getLeft());
		auto right = traverseRuleConclusion(substTable, binaryRule->getRight());
		if (!left || !right)
			return nullptr;
		return std::make_shared<BinaryOperator>(left, binaryRule->getOp(), right);
	}
	}
}
