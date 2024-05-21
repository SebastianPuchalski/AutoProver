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
	std::vector<std::string> variables;
	for (char c = 'a'; c <= 'z'; c++)
		variables.push_back(std::string(1, c));

	std::vector<std::pair<std::string, UnaryOperator::Op>> unaryOperators;
	unaryOperators.push_back(std::pair<std::string, UnaryOperator::Op>("~", UnaryOperator::NOT));

	std::vector<std::pair<std::string, BinaryOperator::Op>> binaryOperators;
	binaryOperators.push_back(std::pair<std::string, BinaryOperator::Op>("&", BinaryOperator::AND));
	binaryOperators.push_back(std::pair<std::string, BinaryOperator::Op>("|", BinaryOperator::OR));
	binaryOperators.push_back(std::pair<std::string, BinaryOperator::Op>("<->", BinaryOperator::XNOR));
	binaryOperators.push_back(std::pair<std::string, BinaryOperator::Op>("->", BinaryOperator::IMP));

	setSymbolStrings(variables, unaryOperators, binaryOperators);

	const int BINARY_OP_COUNT = 16;
	int level = 0;
	std::vector<int> binaryOpPrecedenceLevels(BINARY_OP_COUNT, level);
	binaryOpPrecedenceLevels[BinaryOperator::XNOR] = ++level;
	binaryOpPrecedenceLevels[BinaryOperator::IMP] = ++level;
	binaryOpPrecedenceLevels[BinaryOperator::OR] = ++level;
	binaryOpPrecedenceLevels[BinaryOperator::AND] = ++level;
	setBinaryOpPrecedenceLevels(binaryOpPrecedenceLevels);
}

void Converter::setSymbolStrings(const std::vector<std::string>& variables,
	const std::vector<std::pair<std::string, UnaryOperator::Op>>& unaryOperators,
	const std::vector<std::pair<std::string, BinaryOperator::Op>>& binaryOperators,
	const std::string& trueConstant, const std::string& falseConstant,
	const std::string& openingParenthesis, const std::string& closingParenthesis,
	char whitespace) {

	this->variables = variables;
	this->unaryOperators = unaryOperators;
	this->binaryOperators = binaryOperators;
	this->trueConstant = trueConstant;
	this->falseConstant = falseConstant;
	this->openingParenthesis = openingParenthesis;
	this->closingParenthesis = closingParenthesis;
	this->whitespace = whitespace;

	stringToTokenMap.push_back(std::pair<std::string, Token>(openingParenthesis, Token(Token::OPEN_PAR, 0)));
	stringToTokenMap.push_back(std::pair<std::string, Token>(closingParenthesis, Token(Token::CLOSE_PAR, 0)));

	for(int i = 0; i < variables.size(); i++) {
		Token token(Token::VARIABLE, i);
		stringToTokenMap.push_back(std::pair<std::string, Token>(variables[i], token));
	}

	stringToTokenMap.push_back(std::pair<std::string, Token>(falseConstant, Token(Token::CONSTANT, Constant::Value::FALSE)));
	stringToTokenMap.push_back(std::pair<std::string, Token>(trueConstant, Token(Token::CONSTANT, Constant::Value::TRUE)));

	for (int i = 0; i < unaryOperators.size(); i++) {
		Token token(Token::UNARY, unaryOperators[i].second);
		stringToTokenMap.push_back(std::pair<std::string, Token>(unaryOperators[i].first, token));
	}

	for (int i = 0; i < binaryOperators.size(); i++) {
		Token token(Token::BINARY, binaryOperators[i].second);
		stringToTokenMap.push_back(std::pair<std::string, Token>(binaryOperators[i].first, token));
	}
}

void Converter::setBinaryOpPrecedenceLevels(const std::vector<int>& precedenceLevels) {
	assert(precedenceLevels.size() == 16);
	binaryOpPrecedenceLevels = precedenceLevels;
}

std::shared_ptr<Proposition> Converter::fromString(const std::string& str) {
	std::vector<Token> tokens;
	int error = stringToTokens(tokens, str, stringToTokenMap, whitespace);
	if (error) 
		return std::shared_ptr<Proposition>();

	if(!pairParentheses(tokens))
		return std::shared_ptr<Proposition>();

	return fromTokens(tokens.begin(), tokens.end());
}

std::string Converter::toString(std::shared_ptr<Proposition> proposition, bool parenthesis) {
	Proposition::Type type = proposition->getType();

	if (type == Proposition::VARIABLE) {
		std::shared_ptr<Variable> variable = std::static_pointer_cast<Variable>(proposition);
		assert(variable->getId() < variables.size());
		return variables[variable->getId()];
	}

	if (type == Proposition::CONSTANT) {
		std::shared_ptr<Constant> constant = std::static_pointer_cast<Constant>(proposition);
		switch (constant->getValue()) {
		case Constant::FALSE:
			return falseConstant;
		case Constant::TRUE:
			return trueConstant;
		default:
			assert(!"Wrong value");
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
	}

	if (type == Proposition::BINARY) {
		std::shared_ptr<BinaryOperator> binaryOp = std::static_pointer_cast<BinaryOperator>(proposition);
		for (auto pair : binaryOperators) {
			if (pair.second == binaryOp->getOp()) {
				std::string leftStr = toString(binaryOp->getLeft(), true);
				std::string rightStr = toString(binaryOp->getRight(), true);
				std::string result = leftStr + whitespace + pair.first + whitespace + rightStr;
				if (parenthesis)
					result = openingParenthesis + result + closingParenthesis;
				return result;
			}
		}
		assert(!"Cannot find binary operator");
	}

	assert(!"Error during printing");
	return std::string();
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

std::shared_ptr<Proposition> Converter::fromTokens(std::vector<Token>::iterator begin,
	                                               std::vector<Token>::iterator end) {
	if (end - begin <= 0)
		return std::shared_ptr<Proposition>();

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
			return std::shared_ptr<Proposition>();
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
		std::shared_ptr<Proposition> left = fromTokens(begin, binary);
		std::shared_ptr<Proposition> right = fromTokens(binary + 1, end);
		if(left && right)
			return std::make_shared<BinaryOperator>(left, (BinaryOperator::Op)(*binary).value, right);
		return std::shared_ptr<Proposition>();
	}
	if (unary != end) {
		if (unary == begin) {
			std::shared_ptr<Proposition> operand = fromTokens(unary + 1, end);
			if (operand)
				return std::make_shared<UnaryOperator>(operand, (UnaryOperator::Op)(*unary).value);
		}
		return std::shared_ptr<Proposition>();
	}

	return std::shared_ptr<Proposition>();
}
