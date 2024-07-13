#pragma once

#include "Variable.hpp"
#include "Constant.hpp"
#include "UnaryOperator.hpp"
#include "BinaryOperator.hpp"

#include <string>
#include <vector>

class Converter {
public:
	Converter();
	virtual ~Converter() = default;

	void getStringsForVariables(std::vector<std::string>& variables) const;
	void getStringsForOperators(std::vector<std::pair<std::string, UnaryOperator::Op>>& unaryOperators,
		                        std::vector<std::pair<std::string, BinaryOperator::Op>>& binaryOperators) const;
	void getStringsForSymbols(std::string& trueConstant, std::string& falseConstant,
		                      std::string& openingParenthesis, std::string& closingParenthesis,
		                      char& whitespace) const;
	void getBinaryOpPrecedenceLevels(std::vector<int>& precedenceLevels) const;

	void setStringsForVariables(const std::vector<std::string>& variables);
	void setStringsForOperators(const std::vector<std::pair<std::string, UnaryOperator::Op>>& unaryOperators,
		                        const std::vector<std::pair<std::string, BinaryOperator::Op>>& binaryOperators);
	void setStringsForSymbols(const std::string& trueConstant = "T", const std::string& falseConstant = "F",
						      const std::string& openingParenthesis = "(", const std::string& closingParenthesis = ")",
		                      char whitespace = ' ');
	void setBinaryOpPrecedenceLevels(const std::vector<int>& precedenceLevels);

	void skipParenthesisIfBinOpIsAssociative(bool skip);

	PropositionSP fromString(const std::string& str);
	std::string toString(PropositionSP proposition, bool addParenthesis = false);

private:
	std::vector<std::string> variables;
	std::vector<std::pair<std::string, UnaryOperator::Op>> unaryOperators;
	std::vector<std::pair<std::string, BinaryOperator::Op>> binaryOperators;
	std::string trueConstant;
	std::string falseConstant;
	std::string openingParenthesis;
	std::string closingParenthesis;
	char whitespace;
	std::vector<int> binaryOpPrecedenceLevels;
	bool skipParenthesisIfAssociative;

	struct Token {
		enum Type {UNDEFINED, OPEN_PAR, CLOSE_PAR, VARIABLE, CONSTANT, UNARY, BINARY};
		Type type;
		int value;
		/*
		* The "value" variable contains:
		* - Distance to the matching parenthesis (for OPEN_PAR or CLOSE_PAR)
		* - Id of the variable (for VARIABLE)
		* - Value (TRUE or FALSE) of the constant (for CONSTANT)
		* - Type of unary operator (for UNARY)
		* - Type of binary operator (for BINARY)
		*/
		Token() : type(UNDEFINED), value(0) {}
		Token(Type type, int value) : type(type), value(value) {}

	};
	std::vector<std::pair<std::string, Token>> stringToTokenMap;

	void prepareStringToTokenMap();
	bool pairParentheses(std::vector<Token>& tokens);
	PropositionSP fromTokens(std::vector<Token>::iterator begin, std::vector<Token>::iterator end);
};
