#include "../Variable.hpp"
#include "../Constant.hpp"
#include "../UnaryOperator.hpp"
#include "../BinaryOperator.hpp"
#include "../Converter.hpp"
#include "../ModelChecker.hpp"
#include "../Resolution.hpp"
#include "../ForwardChaining.hpp"
#include "../NaturalDeduction.hpp"

#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

const int NAME_STR_LENGTH = 32;
const int RESULT_STR_LENGTH = 10;

void printTestHeader() {
	cout << left << setw(NAME_STR_LENGTH) << "Name";
	cout << left << setw(RESULT_STR_LENGTH) << "Result";
	cout << "   Details" << endl;
}

void printTestItem(const string& name, bool pass, string addInfo = "") {
	assert(name.length() <= (NAME_STR_LENGTH - 2));
	cout << left << setw(NAME_STR_LENGTH) << name;
	cout << (pass ? "\033[32m" : "\033[31m") <<
		left << setw(RESULT_STR_LENGTH) <<
		(pass ? "PASS" : "FAIL") << "\033[0m";
	if (!addInfo.empty())
		cout << "   " << addInfo;
	cout << endl;
}

void testModelChecking(const string& proposition, bool valid) {
	Converter converter;
	converter.skipParenthesisIfBinOpIsAssociative(false);
	auto prop = converter.fromString(proposition);

	NaiveModelChecker checker;
	bool pass = (valid == checker.isValid(prop));
	printTestItem("ModelChecking", pass, converter.toString(prop));
}

void testResolution(const string& proposition, bool valid, ofstream& logFile) {
	Converter converter;
	converter.skipParenthesisIfBinOpIsAssociative(false);
	auto prop = converter.fromString(proposition);

	string proofString;
	bool pass = (valid == Resolution::isValid(prop, &proofString));
	printTestItem("Resolution", pass, converter.toString(prop));
	if (logFile.is_open())
		logFile << proofString << endl;
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
	bool pass = kb.isEntailed(6) == true;
	printTestItem("ForwardChaining", pass);
}

void testNaturalDeduction(vector<string> premises, string conclusion, ofstream& logFile) {
	Converter converter;
	NaturalDeduction nd;
	nd.addJasInferenceRules();
	for(auto& premise : premises)
		nd.addPremise(converter.fromString(premise));
	nd.setConclusion(converter.fromString(conclusion));
	const int STEP_LIMIT = 1000;
	uint64_t i;
	for (i = 1; nd.step() && i < STEP_LIMIT; i++);
	bool pass = nd.isProofFound() == true;
	string addInfo = "Number of steps: " + to_string(i);
	printTestItem("NaturalDeduction", pass, addInfo);
	if (logFile.is_open())
		logFile << nd.getProofString() << endl;
}

//void testNaturalDeduction(vector<string> premises, string conclusion, ofstream& logFile) {
//	Converter converter;
//	NaturalDeduction nd;
//	nd.addJasInferenceRules();
//	for (auto& premise : premises) {
//		auto p = converter.fromString(premise);
//		if (Resolution::isValid(p)) {
//			cout << "Valid: " << premise << endl;
//			nd.addPremise(p);
//		}
//		else
//			cout << "Invalid\n";
//	}
//	const int STEP_LIMIT = 10000;
//	uint64_t i;
//	for (i = 1; nd.step() && i < STEP_LIMIT; i++);
//
//	auto graph = nd.getFullGraph();
//	cout << "Graph size: " << graph.size() << endl;
//
//	for (int i = 0; i < 10000 && logFile.is_open(); i++) {
//		int idx = i * (graph.size() / 10000);
//		auto prop = graph[idx].proposition;
//		if (Resolution::isValid(prop)) {
//			cout << converter.toString(graph[idx].proposition) << endl;
//			logFile << converter.toString(graph[idx].proposition) << endl;
//		}
//		else
//			cout << "Invalid generated proposition\n";
//	}
//}

int main() {
	printTestHeader();

	testModelChecking("((a <-> b) <-> c) <-> (a <-> (b <-> c))", true);
	testModelChecking("((a & ~b) | c) <-> (d -> (e & f)) <-> ((a & ~b) | c) <-> (d -> (e & f))", true);
	testModelChecking("(a & b & c) <-> ~(a & b & c)", false);
	testModelChecking("~(((p -> q) & (r | ~s)) <-> ((t <-> u) | (v & (w -> ~x)))) & ((y & z) -> (a | (b <-> ~c)))", false);

	std::ofstream resLogFile("resolution.log");
	testResolution("((a <-> b) <-> c) <-> (a <-> (b <-> c))", true, resLogFile);
	testResolution("((a & ~b) | c) <-> (d -> (e & f)) <-> ((a & ~b) | c) <-> (d -> (e & f))", true, resLogFile);
	testResolution("(a & b & c) <-> ~(a & b & c)", false, resLogFile);
	testResolution("~(((p -> q) & (r | ~s)) <-> ((t <-> u) | (v & (w -> ~x)))) & ((y & z) -> (a | (b <-> ~c)))", false, resLogFile);
	resLogFile.close();

	testForwardChaining();

	std::ofstream ndLogFile("natural deduction.log");
	testNaturalDeduction({"p & s", "(p -> q) & (q -> r)", "(s & r) -> t"}, "t", ndLogFile);
	testNaturalDeduction({"a", "a -> b", "b <-> c", "(c & a) -> f"}, "f", ndLogFile);
	//testNaturalDeduction({ "p | ~p", "(a -> b) <-> (~b -> ~a)", "(p & q) -> p", "(x | y) <-> (~x -> y)", "(r -> s) | (s -> r)", "((a & b) -> c) <-> (a -> (b -> c))", "(d | e) -> (d -> e)", "(f & (g | h)) <-> ((f & g) | (f & h))", "(i -> (j -> k)) <-> ((i & j) -> k)", "(l | m) <-> ((l -> m) -> m)", "(n & o) | (n & ~o) | (~n & o) | (~n & ~o)", "(p -> q) <-> (~q -> ~p)", "(r | s) <-> ~(~r & ~s)", "(t -> (u -> v)) <-> ((t & u) -> v)", "(w & x) | (~w & ~x) | (w & ~x) | (~w & x)", "((p -> q) & (q -> r)) -> (p -> r)", "(a <-> b) <-> ((a -> b) & (b -> a))", "(c | d) <-> ~(~c & ~d)", "(e -> f) | (f -> e)", "(g & h) -> g", "(i | j) -> (i -> j)", "(k & l) <-> ~(~k | ~l)", "(m -> n) <-> (~m | n)", "(o & (p | q)) <-> ((o & p) | (o & q))", "(r -> (s -> t)) <-> ((r & s) -> t)", "(u | v) <-> ((u -> v) -> v)", "(w -> x) <-> (~w | x)", "(y & z) | (~y & ~z)", "(a -> b) -> ((b -> c) -> (a -> c))", "(d & e) -> (d <-> e)", "(f | g) <-> ~(~f & ~g)", "(h -> (i & j)) <-> ((h -> i) & (h -> j))", "(k | (l & m)) <-> ((k | l) & (k | m))", "(n -> o) <-> (~o -> ~n)", "(p & q) <-> ~(~p | ~q)", "(r -> s) <-> (~s -> ~r)", "(t | u) -> (t -> u)", "(v & (w -> x)) -> ((v & w) -> x)", "(y -> z) | (z -> y)", "(a & b) | (a & ~b) | (~a & b) | (~a & ~b)", "(c -> (d -> e)) <-> ((c & d) -> e)", "(f | g) <-> ((f -> g) -> g)", "(h & i) -> h", "(j -> k) <-> (~j | k)", "(l & (m | n)) <-> ((l & m) | (l & n))", "(o -> p) | (p -> o)", "(q & r) <-> ~(~q | ~r)", "(s -> (t & u)) <-> ((s -> t) & (s -> u))", "(v | w) <-> ~(~v & ~w)", "(x -> y) <-> (~y -> ~x)", "(z & (a | b)) -> ((z & a) | (z & b))", "(((g & h) -> g) | (((a & b) -> c) -> (a -> (b -> c)))) | ((~(~r & ~s) -> (r | s)) | ((j -> k) <-> (~j | k)))", "(((j -> k) <-> (~j | k)) | (~(~r & ~s) -> (r | s))) | (((~m | n) <-> (m -> n)) | ((w -> x) <-> (~w | x)))", "((~(~r & ~s) -> (r | s)) & ((j -> k) <-> (~j | k))) & (((p -> q) <-> (~q -> ~p)) | ((o -> p) | (p -> o)))", "(((~x -> y) <-> (x | y)) | ((~m | n) <-> (m -> n))) & ((~(~r & ~s) -> (r | s)) | ((j -> k) <-> (~j | k)))" }, "", ndLogFile);
	//testNaturalDeduction({"(a <-> b) <-> c"}, "a <-> (b <->c)", ndLogFile); // it is taking too much time
	ndLogFile.close();

	return 0;
}
