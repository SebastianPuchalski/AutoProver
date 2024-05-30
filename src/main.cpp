#include "Variable.hpp"
#include "Constant.hpp"
#include "UnaryOperator.hpp"
#include "BinaryOperator.hpp"
#include "Converter.hpp"
#include "ModelChecker.hpp"

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

int main() {
	Converter converter;
	converter.setParenthesisIfBinOpIsAssociative(true);
	printConverterSettings(converter);
	cout << endl;

	while (true) {
		string str;
		getline(cin, str);
		shared_ptr<Proposition> proposition = converter.fromString(str);

		if (proposition) {
			cout << converter.toString(proposition) << endl;
			NaiveModelChecker checker;
			bool isTautology = checker.isTautology(proposition);
			bool isContradiction = checker.isContradiction(proposition);
			if (isTautology)
				cout << "Tautology\n";
			else if (isContradiction)
				cout << "Contradiction\n";
			else
				cout << "Contingent proposition\n";
			cout << "CNF: " << converter.toString(proposition->toCNF()) << endl;
			cout << "DNF: " << converter.toString(proposition->toDNF()) << endl;
		}
		else {
			cout << "Error during parsing!\n";
		}
		cout << endl;
	}

	return 0;
}
