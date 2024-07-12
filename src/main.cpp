#include "Variable.hpp"
#include "Constant.hpp"
#include "UnaryOperator.hpp"
#include "BinaryOperator.hpp"
#include "Converter.hpp"
#include "ModelChecker.hpp"
#include "Resolution.hpp"
#include "ForwardChaining.hpp"
#include "NaturalDeduction.hpp"

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

void testForwardChaining() {
	ForwardChainingKB kb;
	kb.addFact(1);
	kb.addFact(2);
	kb.addClause(ForwardChainingKB::DefiniteClause({ 5 }, 6));
	kb.addClause(ForwardChainingKB::DefiniteClause({ 3, 4 }, 5));
	kb.addClause(ForwardChainingKB::DefiniteClause({ 2, 3 }, 4));
	kb.addClause(ForwardChainingKB::DefiniteClause({ 1, 5 }, 3));
	kb.addClause(ForwardChainingKB::DefiniteClause({ 1, 2 }, 3));
	bool result = kb.isEntailed(6);
	cout << "Chaining result: " << (result ? "Entailed" : "Not entailed") << endl;
}

void testNaturalDeduction() {
	Converter converter;
	NaturalDeduction nd;
	nd.addJasInferenceRules();

	/*nd.addPremise(converter.fromString("a"));
	nd.addPremise(converter.fromString("a -> b"));
	nd.addPremise(converter.fromString("b <-> c"));
	nd.addPremise(converter.fromString("(c & a) -> f"));
	nd.setConclusion(converter.fromString("f"));*/

	nd.addPremise(converter.fromString("p & s"));
	nd.addPremise(converter.fromString("(p -> q) & (q -> r)"));
	nd.addPremise(converter.fromString("(s & r) -> t"));
	nd.setConclusion(converter.fromString("t"));

	/*nd.addPremise(converter.fromString("(a <-> b) <-> c"));
	nd.setConclusion(converter.fromString("a <-> (b <-> c)"));*/

	uint64_t i;
	for (i = 1; nd.step(); i++)
		if(i % 1000 == 0)
			cout << "Step: " << i << endl;
	cout << "Proof " << (nd.isProofFound() ? "" : "not ") << "found in " << i << " steps\n";
	cout << nd.getProofString() << endl;
}

int main() {
	Converter converter;
	converter.setParenthesisIfBinOpIsAssociative(false);
	printConverterSettings(converter);
	cout << endl;

	while (true) {
		string str;
		getline(cin, str);
		if (str == "test fc") {
			testForwardChaining();
			continue;
		}
		if (str == "test nd") {
			testNaturalDeduction();
			continue;
		}
		shared_ptr<Proposition> proposition = converter.fromString(str);

		if (proposition) {
			cout << converter.toString(proposition) << endl;

			NaiveModelChecker checker;
			bool isTautology = checker.isValid(proposition);
			bool isContradiction = checker.isContradiction(proposition);
			if (isTautology)
				cout << "Model checking: valid\n";
			else if (isContradiction)
				cout << "Model checking: contradiction\n";
			else
				cout << "Model checking: contingent proposition\n";

			bool valid = Resolution::isValid(proposition);
			//cout << "Resolution: " << (valid ? "valid" : "not valid") << endl;

			/*cout << "CNF: " << converter.toString(proposition->toCNF()) << endl;
			cout << "DNF: " << converter.toString(proposition->toDNF()) << endl;*/
		}
		else {
			cout << "Error during parsing!\n";
		}
		cout << endl;
	}

	return 0;
}
