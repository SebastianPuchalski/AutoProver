#pragma once

#include "Proposition.hpp"

#include <string>
#include <vector>

class Converter {
public:
	Converter();
	virtual ~Converter() = default;

	void setSymbolStrings(const std::vector<std::string>& variables,
		                  const std::vector<std::pair<std::string, UnaryOperator::Op>>& unaryOperators,
		                  const std::vector<std::pair<std::string, BinaryOperator::Op>>& binaryOperators,
		                  const std::string& trueConstant = "T", const std::string& falseConstant = "F",
		                  const std::string& openingParenthesis = "(", const std::string& closingParenthesis = ")",
		                  char whitespace = ' ');

	std::shared_ptr<Proposition> fromString(const std::string& str);
	std::string toString(std::shared_ptr<Proposition> proposition, bool parenthesis = false);

private:
	std::vector<std::string> variables;
	std::vector<std::pair<std::string, UnaryOperator::Op>> unaryOperators;
	std::vector<std::pair<std::string, BinaryOperator::Op>> binaryOperators;
	std::string trueConstant;
	std::string falseConstant;
	std::string openingParenthesis;
	std::string closingParenthesis;
	char whitespace;

	struct Token {
		enum Type {OPEN_PAR, CLOSE_PAR, VARIABLE, CONSTANT, UNARY, BINARY};
		Type type;
		int value;

		Token() : value(-1) {}
		Token(Type type, int value = 0) : type(type), value(value) {}

	};
	std::vector<std::pair<std::string, Token>> stringToTokenMap;

	bool pairParentheses(std::vector<Token>& tokens);
	std::shared_ptr<Proposition> fromTokens(std::vector<Token>::iterator begin, std::vector<Token>::iterator end);
};
