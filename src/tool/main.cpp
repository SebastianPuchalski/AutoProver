#include "../Variable.hpp"
#include "../Constant.hpp"
#include "../UnaryOperator.hpp"
#include "../BinaryOperator.hpp"
#include "../Converter.hpp"
#include "../ModelChecker.hpp"
#include "../Resolution.hpp"
#include "../ForwardChaining.hpp"
#include "../NaturalDeduction.hpp"

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
	converter.skipParenthesisIfBinOpIsAssociative(false);
	printConverterSettings(converter);
	cout << endl;

	while (true) {
		string str;
		getline(cin, str);
		shared_ptr<Proposition> proposition = converter.fromString(str);

		if (proposition) {
			cout << "Model checking: ";
			vector<int> variableIds;
			proposition->getVariableIds(variableIds);
			const int MAX_VARIABLE_NUMBER = 34; // it is too slow for large number of variables
			if (variableIds.size() <= MAX_VARIABLE_NUMBER) {
				NaiveModelChecker checker;
				bool isTautology = checker.isValid(proposition);
				bool isContradiction = checker.isContradiction(proposition);
				if (isTautology)
					cout << "valid";
				else if (isContradiction)
					cout << "contradiction";
				else
					cout << "contingent proposition";
			}
			else
				cout << "too many variables";
			cout << endl;

			string proofString;
			bool valid = Resolution::isValid(proposition, &proofString);
			cout << "Resolution: ";
			cout << (valid ? "valid" : "not valid");
			cout << "\n\n";
			cout << proofString;
		}
		else {
			cout << "Error during parsing!\n";
		}
		cout << endl;
	}
	return 0;
}
