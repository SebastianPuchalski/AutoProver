#include "Converter.hpp"

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
		int bestLength = 0;
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

	stringToTokenMap.push_back(std::pair<std::string, Token>(openingParenthesis, Token(Token::OPEN_PAR)));
	stringToTokenMap.push_back(std::pair<std::string, Token>(closingParenthesis, Token(Token::CLOSE_PAR)));

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

#include <iostream>

std::shared_ptr<Proposition> Converter::fromString(const std::string& str) {
	std::vector<Token> tokens;
	int error = stringToTokens(tokens, str, stringToTokenMap, whitespace);
	if (error) {
		std::string s = str;
		s.erase(0, str.size() - error);
		std::cout << "Error in " << s;
		return std::shared_ptr<Proposition>();
	}

	for (int i = 0; i < stringToTokenMap.size(); i++) {
		std::cout << "String: " << stringToTokenMap[i].first << ", Type: " << stringToTokenMap[i].second.type << ", Index: " << stringToTokenMap[i].second.index << std::endl;
	}

	for (int i = 0; i < tokens.size(); i++) {
		std::cout << "Type: " << tokens[i].type << ", Index: " << tokens[i].index << std::endl;
	}

	return std::shared_ptr<Proposition>();
}

std::string Converter::toString(std::shared_ptr<Proposition> proposition) {
	return std::string();
}
