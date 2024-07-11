#include "Converter.hpp"

#include <cassert>
#include <stack>

bool isStringPrefix(const std::string& str, const std::string& prefix) {
	if (str.size() < prefix.size())
		return false;
	for (int i = 0; i < prefix.size(); i++)
		if (str[i] != prefix[i])
			return false;
	return true;
}

template <typename T>
int stringToTokens(std::vector<T>& result, std::string str,
	const std::vector<std::pair<std::string, T>>& stringToTokenMap,
	char whitespace = ' ') {
	while (!str.empty()) {
		while(str[0] == whitespace) {
			str.erase(0, 1);
			continue;
		}
		size_t bestLength = 0;
		T bestToken;
		for (auto pair : stringToTokenMap) {
			std::string& tokenStr = pair.first;
			T& token = pair.second;
			if (isStringPrefix(str, tokenStr)) {
				if (tokenStr.size() > bestLength) {
					bestLength = tokenStr.size();
					bestToken = token;
				}
			}
		}
		if (bestLength > 0) {
			str.erase(0, bestLength);
			result.push_back(bestToken);
		}
		else {
			return str.size();
		}
	}
	return 0;
}

Converter::Converter() {
	const int VARIABLE_SETS_NUM = 2;
	for (int i = 0; i < VARIABLE_SETS_NUM; i++) {
		for (char c = 'a'; c <= 'z'; c++) {
			std::string name(1, c);
			if (i > 0)
				name += std::to_string(i);
			variables.push_back(name);
		}
	}

	unaryOperators.push_back(std::pair<std::string, UnaryOperator::Op>("~", UnaryOperator::NOT));
	binaryOperators.push_back(std::pair<std::string, BinaryOperator::Op>("&", BinaryOperator::AND));
	binaryOperators.push_back(std::pair<std::string, BinaryOperator::Op>("|", BinaryOperator::OR));
	binaryOperators.push_back(std::pair<std::string, BinaryOperator::Op>("<->", BinaryOperator::XNOR));
	binaryOperators.push_back(std::pair<std::string, BinaryOperator::Op>("->", BinaryOperator::IMP));

	setStringsForSymbols();

	const int BINARY_OP_COUNT = 16;
	int level = 0;
	binaryOpPrecedenceLevels = std::vector<int>(BINARY_OP_COUNT, level);
	binaryOpPrecedenceLevels[BinaryOperator::XNOR] = ++level;
	binaryOpPrecedenceLevels[BinaryOperator::IMP] = ++level;
	binaryOpPrecedenceLevels[BinaryOperator::OR] = ++level;
	binaryOpPrecedenceLevels[BinaryOperator::AND] = ++level;

	skipParenthesisIfAssociative = false;
}

void Converter::getStringsForVariables(std::vector<std::string>& variables) const {
	variables = this->variables;
}

void Converter::getStringsForOperators(std::vector<std::pair<std::string, UnaryOperator::Op>>& unaryOperators,
	                                   std::vector<std::pair<std::string, BinaryOperator::Op>>& binaryOperators) const {
	unaryOperators = this->unaryOperators;
	binaryOperators = this->binaryOperators;
}

void Converter::getStringsForSymbols(std::string& trueConstant, std::string& falseConstant,
	                                 std::string& openingParenthesis, std::string& closingParenthesis,
	                                 char& whitespace) const {
	trueConstant = this->trueConstant;
	falseConstant = this->falseConstant;
	openingParenthesis = this->openingParenthesis;
	closingParenthesis = this->closingParenthesis;
	whitespace = this->whitespace;
}

void Converter::getBinaryOpPrecedenceLevels(std::vector<int>& precedenceLevels) const {
	precedenceLevels = binaryOpPrecedenceLevels;
}

void Converter::setStringsForVariables(const std::vector<std::string>& variables) {
	for (auto item : variables) assert(!item.empty());
	this->variables = variables;
	stringToTokenMap.clear();
}

void Converter::setStringsForOperators(const std::vector<std::pair<std::string, UnaryOperator::Op>>& unaryOperators,
	                                   const std::vector<std::pair<std::string, BinaryOperator::Op>>& binaryOperators) {
	for (auto item : unaryOperators)
		assert(!item.first.empty() && item.second >= UnaryOperator::FALSE && item.second <= UnaryOperator::TRUE);
	for (auto item : binaryOperators)
		assert(!item.first.empty() && item.second >= BinaryOperator::FALSE && item.second <= BinaryOperator::TRUE);
	this->unaryOperators = unaryOperators;
	this->binaryOperators = binaryOperators;
	stringToTokenMap.clear();
}

void Converter::setStringsForSymbols(const std::string& trueConstant, const std::string& falseConstant,
									 const std::string& openingParenthesis, const std::string& closingParenthesis,
									 char whitespace) {
	this->trueConstant = trueConstant;
	this->falseConstant = falseConstant;
	this->openingParenthesis = openingParenthesis;
	this->closingParenthesis = closingParenthesis;
	this->whitespace = whitespace;
	stringToTokenMap.clear();
}

void Converter::setBinaryOpPrecedenceLevels(const std::vector<int>& precedenceLevels) {
	const int BINARY_OP_COUNT = 16;
	assert(precedenceLevels.size() == BINARY_OP_COUNT);
	binaryOpPrecedenceLevels = precedenceLevels;
}

void Converter::setParenthesisIfBinOpIsAssociative(bool skip) {
	skipParenthesisIfAssociative = skip;
}

PropositionSP Converter::fromString(const std::string& str) {
	if(stringToTokenMap.empty())
		prepareStringToTokenMap();

	std::vector<Token> tokens;

	int error = stringToTokens(tokens, str, stringToTokenMap, whitespace);
	if (error) 
		return PropositionSP();

	if(!pairParentheses(tokens))
		return PropositionSP();

	return fromTokens(tokens.begin(), tokens.end());
}

std::string Converter::toString(PropositionSP proposition, bool addParenthesis) {
	Proposition::Type type = proposition->getType();

	if (type == Proposition::VARIABLE) {
		std::shared_ptr<Variable> variable = std::static_pointer_cast<Variable>(proposition);
		if(variable->getId() < variables.size())
			return variables[variable->getId()];
		assert(!"Cannot find variable");
		return std::string(" VARIABLE_ERROR ");
	}

	if (type == Proposition::CONSTANT) {
		std::shared_ptr<Constant> constant = std::static_pointer_cast<Constant>(proposition);
		switch (constant->getValue()) {
		case Constant::FALSE:
			assert(!falseConstant.empty());
			return falseConstant;
		case Constant::TRUE:
			assert(!trueConstant.empty());
			return trueConstant;
		default:
			assert(!"Wrong constant value");
			return std::string(" CONSTANT_ERROR ");
		}
	}

	if (type == Proposition::UNARY) {
		std::shared_ptr<UnaryOperator> unaryOp = std::static_pointer_cast<UnaryOperator>(proposition);
		for (auto pair : unaryOperators) {
			if (pair.second == unaryOp->getOp()) {
				return pair.first + toString(unaryOp->getOperand(), true);
			}
		}
		assert(!"Cannot find unary operator");
		return std::string(" UNARY_ERROR ");
	}

	if (type == Proposition::BINARY) {
		std::shared_ptr<BinaryOperator> binaryOp = std::static_pointer_cast<BinaryOperator>(proposition);
		for (auto pair : binaryOperators) {
			bool leftAddPar = true;
			bool rightAddPar = true;
			if (pair.second == binaryOp->getOp()) {
				if (skipParenthesisIfAssociative && binaryOp->isAssociative()) {
					if (binaryOp->getLeft()->getType() == Proposition::BINARY)
						if (std::static_pointer_cast<BinaryOperator>(binaryOp->getLeft())->getOp() == binaryOp->getOp())
							leftAddPar = false;
					if (binaryOp->getRight()->getType() == Proposition::BINARY)
						if (std::static_pointer_cast<BinaryOperator>(binaryOp->getRight())->getOp() == binaryOp->getOp())
							rightAddPar = false;
				}
				std::string leftStr = toString(binaryOp->getLeft(), leftAddPar);
				std::string rightStr = toString(binaryOp->getRight(), rightAddPar);
				std::string result = leftStr + whitespace + pair.first + whitespace + rightStr;
				if (addParenthesis)
					result = openingParenthesis + result + closingParenthesis;
				return result;
			}
		}
		assert(!"Cannot find binary operator");
		return std::string(" BINARY_ERROR ");
	}

	assert(!"Error during proposition printing");
	return std::string(" ERROR ");
}

void Converter::prepareStringToTokenMap() {
	if(!openingParenthesis.empty())
		stringToTokenMap.push_back(std::pair<std::string, Token>(openingParenthesis, Token(Token::OPEN_PAR, 0)));
	if (!closingParenthesis.empty())
		stringToTokenMap.push_back(std::pair<std::string, Token>(closingParenthesis, Token(Token::CLOSE_PAR, 0)));

	for (int i = 0; i < variables.size(); i++) {
		Token token(Token::VARIABLE, i);
		stringToTokenMap.push_back(std::pair<std::string, Token>(variables[i], token));
	}

	if (!falseConstant.empty())
		stringToTokenMap.push_back(std::pair<std::string, Token>(falseConstant, Token(Token::CONSTANT, Constant::Value::FALSE)));
	if (!trueConstant.empty())
		stringToTokenMap.push_back(std::pair<std::string, Token>(trueConstant, Token(Token::CONSTANT, Constant::Value::TRUE)));

	for (int i = 0; i < unaryOperators.size(); i++) {
		Token token(Token::UNARY, unaryOperators[i].second);
		stringToTokenMap.push_back(std::pair<std::string, Token>(unaryOperators[i].first, token));
	}

	for (int i = 0; i < binaryOperators.size(); i++) {
		Token token(Token::BINARY, binaryOperators[i].second);
		stringToTokenMap.push_back(std::pair<std::string, Token>(binaryOperators[i].first, token));
	}

	for (auto element : stringToTokenMap) {
		assert(!element.first.empty());
	}

	for (int i = 0; i < stringToTokenMap.size(); i++) {
		assert(!stringToTokenMap[i].first.empty());
		assert(stringToTokenMap[i].second.type != Token::UNDEFINED);
		for (int j = i + 1; j < stringToTokenMap.size(); j++) {
			assert(stringToTokenMap[i].first != stringToTokenMap[j].first);
		}
	}
}

bool Converter::pairParentheses(std::vector<Token>& tokens) {
	std::stack<int> stack;
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i].type == Token::OPEN_PAR) {
			stack.push(i);
		}
		else if (tokens[i].type == Token::CLOSE_PAR) {
			if (stack.empty())
				return false;
			int openIndex = stack.top();
			stack.pop();
			tokens[i].value = openIndex - i;
			tokens[openIndex].value = i - openIndex;
		}
	}
	return stack.empty();
}

PropositionSP Converter::fromTokens(std::vector<Token>::iterator begin,
	                                               std::vector<Token>::iterator end) {
	if (end - begin <= 0)
		return PropositionSP();

	if ((*begin).type == Token::OPEN_PAR && end - 1 - begin == (*begin).value) {
		assert((*(end - 1)).type == Token::CLOSE_PAR);
		assert(begin - (end - 1) == (*(end - 1)).value);
		return fromTokens(begin + 1, end - 1);
	}

	if (end - begin == 1) {
		if ((*begin).type == Token::VARIABLE) {
			return std::make_shared<Variable>((*begin).value);
		}
		else if ((*begin).type == Token::CONSTANT) {
			return std::make_shared<Constant>((Constant::Value)(*begin).value);
		}
		else {
			return PropositionSP();
		}
	}

	std::vector<Token>::iterator unary = end;
	std::vector<Token>::iterator binary = end;
	int binaryBestLevel = INT_MAX;
	for (std::vector<Token>::iterator it = begin; it < end; it++) {
		if ((*it).type == Token::OPEN_PAR) {
			assert((*(it + (*it).value)).type == Token::CLOSE_PAR);
			assert((*(it + (*it).value)).value == -(*it).value);
			it += (*it).value;
			continue;
		}
		if ((*it).type == Token::UNARY && unary == end) {
			unary = it;
		}
		if ((*it).type == Token::BINARY) {
			assert((*it).value < binaryOpPrecedenceLevels.size());
			int level = binaryOpPrecedenceLevels[(*it).value];
			if (level <= binaryBestLevel) {
				binaryBestLevel = level;
				binary = it;
			}
		}
	}

	if (binary != end) {
		PropositionSP left = fromTokens(begin, binary);
		PropositionSP right = fromTokens(binary + 1, end);
		if(left && right)
			return std::make_shared<BinaryOperator>(left, (BinaryOperator::Op)(*binary).value, right);
		return PropositionSP();
	}
	if (unary != end) {
		if (unary == begin) {
			PropositionSP operand = fromTokens(unary + 1, end);
			if (operand)
				return std::make_shared<UnaryOperator>(operand, (UnaryOperator::Op)(*unary).value);
		}
		return PropositionSP();
	}

	return PropositionSP();
}
