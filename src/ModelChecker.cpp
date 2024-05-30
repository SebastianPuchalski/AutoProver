#include "ModelChecker.hpp"

#include <cassert>

bool NaiveModelChecker::isTautology(const std::shared_ptr<Proposition>& proposition) const {
	std::vector<int> variableIds;
	variableIdsFromProposition(proposition, variableIds);
	std::vector<bool> varValues;
	if (variableIds.empty())
		return evaluate(proposition, varValues);

	int maxId = *std::max_element(variableIds.begin(), variableIds.end());
	varValues = std::vector<bool>(maxId + 1);

	for (unsigned long long model = 0; model < ((unsigned long long)1 << variableIds.size()); model++) {
		for (int i = 0; i < variableIds.size(); i++) {
			bool varValue = (model & ((unsigned long long)1 << i)) > 0;
			int id = variableIds[i];
			varValues[id] = varValue;
		}
		bool result = evaluate(proposition, varValues);
		if (!result)
			return false;
	}
	return true;
}

bool NaiveModelChecker::isContradiction(const std::shared_ptr<Proposition>& proposition) const {
	auto nProposition = std::make_shared<UnaryOperator>(proposition, UnaryOperator::NOT);
	return isTautology(nProposition);
}

void NaiveModelChecker::variableIdsFromProposition(const std::shared_ptr<Proposition>& proposition,
	                                               std::vector<int>& variableIds) const {
	Proposition::Type type = proposition->getType();

	if (type == Proposition::VARIABLE) {
		int id = std::static_pointer_cast<Variable>(proposition)->getId();
		bool alreadyExists = false;
		for (auto item : variableIds) {
			if (item == id)
				alreadyExists = true;
		}
		if(!alreadyExists)
			variableIds.push_back(id);
	}

	if (type == Proposition::UNARY) {
		std::shared_ptr<UnaryOperator> unaryOp = std::static_pointer_cast<UnaryOperator>(proposition);
		variableIdsFromProposition(unaryOp->getOperand(), variableIds);
	}

	if (type == Proposition::BINARY) {
		std::shared_ptr<BinaryOperator> binaryOp = std::static_pointer_cast<BinaryOperator>(proposition);
		variableIdsFromProposition(binaryOp->getLeft(), variableIds);
		variableIdsFromProposition(binaryOp->getRight(), variableIds);
	}
}

bool NaiveModelChecker::evaluate(const std::shared_ptr<Proposition>& proposition,
	                             const std::vector<bool>& varValues) const {
	Proposition::Type type = proposition->getType();

	if (type == Proposition::VARIABLE) {
		std::shared_ptr<Variable> variable = std::static_pointer_cast<Variable>(proposition);
		assert(variable->getId() < varValues.size());
		return varValues[variable->getId()];
	}

	if (type == Proposition::CONSTANT) {
		std::shared_ptr<Constant> constant = std::static_pointer_cast<Constant>(proposition);
		return constant->getValue();
	}

	if (type == Proposition::UNARY) {
		std::shared_ptr<UnaryOperator> unaryOp = std::static_pointer_cast<UnaryOperator>(proposition);
		bool a = evaluate(unaryOp->getOperand(), varValues);
		bool b = ((unsigned)unaryOp->getOp() & ((unsigned)1 << (int)!a)) > 0;
		#ifndef NDEBUG
		bool bDebug = false;
		switch (unaryOp->getOp()) {
		case UnaryOperator::FALSE:
			bDebug = false; break;
		case UnaryOperator::TRANSFER:
			bDebug = a; break;
		case UnaryOperator::NOT:
			bDebug = !a; break;
		case UnaryOperator::TRUE:
			bDebug = true; break;
		}
		assert(bDebug == b);
		#endif
		return b;
	}

	if (type == Proposition::BINARY) {
		std::shared_ptr<BinaryOperator> binaryOp = std::static_pointer_cast<BinaryOperator>(proposition);
		bool a = evaluate(binaryOp->getLeft(), varValues);
		bool b = evaluate(binaryOp->getRight(), varValues);
		bool c = ((unsigned)binaryOp->getOp() & (1 << (!a * 2 + !b))) > 0;
		#ifndef NDEBUG
		bool cDebug = false;
		switch (binaryOp->getOp()) {
		case BinaryOperator::FALSE:
			cDebug = false; break;
		case BinaryOperator::AND:
			cDebug = a && b; break;
		case BinaryOperator::NIMP:
			cDebug = a && !b; break;
		case BinaryOperator::A:
			cDebug = a; break;
		case BinaryOperator::NRIMP:
			cDebug = b && !a; break;
		case BinaryOperator::B:
			cDebug = b; break;
		case BinaryOperator::XOR:
			cDebug = a != b; break;
		case BinaryOperator::OR:
			cDebug = a || b; break;
		case BinaryOperator::NOR:
			cDebug = !(a || b); break;
		case BinaryOperator::XNOR:
			cDebug = a == b; break;
		case BinaryOperator::NB:
			cDebug = !b; break;
		case BinaryOperator::RIMP:
			cDebug = !b || a; break;
		case BinaryOperator::NA:
			cDebug = !a; break;
		case BinaryOperator::IMP:
			cDebug = !a || b; break;
		case BinaryOperator::NAND:
			cDebug = !(a && b); break;
		case BinaryOperator::TRUE:
			cDebug = true; break;
		}
		assert(cDebug == c);
		#endif
		return c;
	}

	assert(!"Evaluation error");
	return false;
}
