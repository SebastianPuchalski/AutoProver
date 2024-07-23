#include "../Variable.hpp"
#include "../Constant.hpp"
#include "../UnaryOperator.hpp"
#include "../BinaryOperator.hpp"
#include "../Converter.hpp"
#include "../ModelChecker.hpp"
#include "../Resolution.hpp"
#include "../ForwardChaining.hpp"
#include "../NaturalDeduction.hpp"
#include "../CnfSat.hpp"

#include <iostream>

using namespace std;

void printConverterSettings(const Converter& converter) {
	vector<string> variables;
	converter.getStringsForVariables(variables);
	cout << "Variables: ";
	for (auto str : variables)
		cout << str << " ";
	cout << endl;

	vector<pair<string, UnaryOperator::Op>> unaryOperators;
	vector<pair<string, BinaryOperator::Op>> binaryOperators;
	converter.getStringsForOperators(unaryOperators, binaryOperators);
	cout << "Unary operators: ";
	for (auto pair : unaryOperators)
		cout << pair.first << " ";
	cout << endl;
	cout << "Binary operators: ";
	for (auto pair : binaryOperators)
		cout << pair.first << " ";
	cout << endl;

	string trueConstant;
	string falseConstant;
	string openingParenthesis;
	string closingParenthesis;
	char whitespace;
	converter.getStringsForSymbols(trueConstant, falseConstant, openingParenthesis, closingParenthesis, whitespace);
	cout << "Constants: " << trueConstant << " " << falseConstant << endl;
	cout << "Parenthesis: " << openingParenthesis << " " << closingParenthesis << endl;
}

void modelChecking(PropositionSP proposition) {
	cout << "Model checking: ";
	vector<int> variableIds;
	proposition->getVariableIds(variableIds);
	const int VARIABLE_NUMBER_THRESHOLD = 30;
	bool isValid;
	bool isContradiction;
	if (variableIds.size() <= VARIABLE_NUMBER_THRESHOLD) {
		NaiveModelChecker checker;
		isValid = checker.isValid(proposition);
		isContradiction = checker.isContradiction(proposition);
	}
	else {
		isValid = DpllCnfSat::isPropValid(proposition);
		isContradiction = DpllCnfSat::isPropContradiction(proposition);
	}
	if (isValid)
		cout << "valid";
	else if (isContradiction)
		cout << "contradiction";
	else
		cout << "contingent";
	cout << endl;
}

void resolution(PropositionSP proposition) {
	string proofString;
	bool valid = Resolution::isValid(proposition, &proofString);
	cout << "Resolution: ";
	cout << (valid ? "valid" : "not valid");
	cout << "\n\n";
	cout << proofString;
}

int main() {
	Converter converter;
	//converter.skipParenthesisIfBinOpIsAssociative(false);
	printConverterSettings(converter);
	cout << endl;

	bool work = true;
	while (work) {
		string str;
		getline(cin, str);
		if (str.empty())
			continue;
		if (str[0] == ';') {
			if (str == ";exit")
				work = false;
		}
		else {
			PropositionSP proposition = converter.fromString(str);
			if (proposition) {
				modelChecking(proposition);
				resolution(proposition);
			}
			else
				cout << "Error during parsing!\n";
			cout << endl;
		}
	}
	return 0;
}
