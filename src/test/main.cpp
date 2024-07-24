#include "../Variable.hpp"
#include "../Constant.hpp"
#include "../UnaryOperator.hpp"
#include "../BinaryOperator.hpp"
#include "../Converter.hpp"
#include "../ModelChecker.hpp"
#include "../Resolution.hpp"
#include "../ForwardChaining.hpp"
#include "../NaturalDeduction.hpp"
#include "../NormalForm.hpp"
#include "../CnfSat.hpp"

#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

const int NAME_STR_LENGTH = 32;
const int RESULT_STR_LENGTH = 10;
const int ADD_INFO_MAX_LENGTH = 78;

void printTestHeader() {
	cout << left << setw(NAME_STR_LENGTH) << "Name";
	cout << left << setw(RESULT_STR_LENGTH) << "Result";
	cout << "Details" << endl;
}

void printTestItem(const string& name, bool pass, string addInfo = "") {
	assert(name.length() <= (NAME_STR_LENGTH - 2));
	cout << left << setw(NAME_STR_LENGTH) << name;
	cout << (pass ? "\033[32m" : "\033[31m") <<
		left << setw(RESULT_STR_LENGTH) <<
		(pass ? "PASS" : "FAIL") << "\033[0m";
	if (!addInfo.empty()) {
		if (addInfo.length() > ADD_INFO_MAX_LENGTH) {
			const std::string ellipsis = "...";
			addInfo.resize(ADD_INFO_MAX_LENGTH - ellipsis.length());
			addInfo += ellipsis;
		}
		cout << addInfo;
	}
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

void testDpll(const string& proposition, bool satisfiable) {
	Converter converter;
	auto prop = converter.fromString(proposition);
	Cnf cnf;
	propositionToCnf(cnf, prop);
	DpllCnfSat dpll(cnf);
	bool result = dpll.isSatisfiable();
	bool pass = (satisfiable == result);
	if (pass && result) {
		auto model = dpll.getModel();
		std::vector<int> variableIds;
		prop->getVariableIds(variableIds);
		std::sort(variableIds.begin(), variableIds.end());
		pass = pass && model.size() == variableIds.size();
		if (pass) {
			std::vector<uint64_t> varValues;
			for (int i = 0; i < model.size(); i++) {
				if (varValues.size() < variableIds[i] + 1)
					varValues.resize(variableIds[i] + 1);
				varValues[variableIds[i]] = model[i];
			}
			pass = pass && prop->evaluate(varValues) != 0;
		}
	}
	printTestItem("DPLL", pass, converter.toString(prop));
}

void testWalkSat(const string& proposition, bool satisfiable) {
	Converter converter;
	auto prop = converter.fromString(proposition);
	Cnf cnf;
	propositionToCnf(cnf, prop);
	WalkSat walkSat(cnf);
	bool result = walkSat.isSatisfiable();
	bool pass = (satisfiable == result);
	if (pass && result) {
		auto model = walkSat.getModel();
		std::vector<int> variableIds;
		prop->getVariableIds(variableIds);
		std::sort(variableIds.begin(), variableIds.end());
		pass = pass && model.size() == variableIds.size();
		if (pass) {
			std::vector<uint64_t> varValues;
			for (int i = 0; i < model.size(); i++) {
				if (varValues.size() < variableIds[i] + 1)
					varValues.resize(variableIds[i] + 1);
				varValues[variableIds[i]] = model[i];
			}
			pass = pass && prop->evaluate(varValues) != 0;
		}
	}
	printTestItem("WalkSAT", pass, converter.toString(prop));
}

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
	//testNaturalDeduction({"(a <-> b) <-> c"}, "a <-> (b <->c)", ndLogFile); // it is taking too much time
	ndLogFile.close();

	testDpll("(a | ~b) <-> ((c & d) -> e)", true);
	testDpll("(a & b & c) <-> ~(a & b & c)", false);
	testDpll("((((m & n) | o) -> (p & ~q)) <-> (r | (s & (t -> u)))) & (~v | ((w <-> x) & (y | (~z & a))))", true);
	testDpll("~(((a & ~b) | c) <-> (d -> (e & f)) <-> ((a & ~b) | c) <-> (d -> (e & f)))", false);
	testDpll("(((a & ~b) -> (c | d)) <-> ((e & f) | (g -> ~h))) & (((i | j) <-> (k & l)) -> ((m & ~n) | (o -> p)))", true);
	testDpll("~(((a | b | c) & (d | e | f) & (g | h | i) & (j | k | l) & (m | n | o)) <-> ~((~a & ~b & ~c) | (~d & ~e & ~f) | (~g & ~h & ~i) | (~j & ~k & ~l) | (~m & ~n & ~o)))", false);
	testDpll("~((((x & y) -> z) <-> (a | (b & ~c))) & (((d -> e) | f) <-> ((g & h) -> (i | (j & ~k))))) & (((l & m) | ~n) -> ((o <-> p) | (q & r)))", true);
	testDpll("~((((a & b & e) -> (c | d)) <-> (e | ~a)) & ((f -> (~g & h)) <-> (i | (j & ~k))) -> (((a & b) -> (c | d)) <-> (e | ~a)) & ((f -> (~g & h)) <-> (i | (j & ~k))))", false);
	testDpll("a & b & c & d & e & f & g & h & i & j & k & l & m & n & o & p & q & r & s & t & u & v & w & x & y & z & a1 & b1 & c1 & d1 & e1 & f1 & g1 & h1 & i1 & j1 & k1 & l1 & m1 & n1 & o1 & p1 & q1 & r1 & s1 & t1 & u1 & v1 & w1 & x1 & y1 & z1", true);
	//testDpll("~((((a -> b) & (~b -> ~a) & (c <-> (d | e)) & (f <-> (g & h))) -> (((i | (j & k)) -> (l | (m & n))) & ((o & p) -> (q & (r | s))) & ((t | (u & v)) -> (w | (x & y))) & ((z & a) -> (b & (c | d))))) <-> (((a -> b) & (~b -> ~a) & (c <-> (d | e)) & (f <-> (g & h))) -> (((i | (j & k)) -> (l | (m & n))) & ((o & p) -> (q & (r | s))) & ((t | (u & v)) -> (w | (x & y))) & ((z & a) -> (b & (c | d))))))", false);

	testWalkSat("(a | ~b) <-> ((c & d) -> e)", true);
	testWalkSat("(a & b & c) <-> ~(a & b & c)", false);
	testWalkSat("((((m & n) | o) -> (p & ~q)) <-> (r | (s & (t -> u)))) & (~v | ((w <-> x) & (y | (~z & a))))", true);
	testWalkSat("~(((a & ~b) | c) <-> (d -> (e & f)) <-> ((a & ~b) | c) <-> (d -> (e & f)))", false);
	testWalkSat("(((a & ~b) -> (c | d)) <-> ((e & f) | (g -> ~h))) & (((i | j) <-> (k & l)) -> ((m & ~n) | (o -> p)))", true);
	testWalkSat("~(((a | b | c) & (d | e | f) & (g | h | i) & (j | k | l) & (m | n | o)) <-> ~((~a & ~b & ~c) | (~d & ~e & ~f) | (~g & ~h & ~i) | (~j & ~k & ~l) | (~m & ~n & ~o)))", false);
	testWalkSat("a & b & c & d & e & f & g & h & i & j & k & l & m & n & o & p & q & r & s & t & u & v & w & x & y & z & a1 & b1 & c1 & d1 & e1 & f1 & g1 & h1 & i1 & j1 & k1 & l1 & m1 & n1 & o1 & p1 & q1 & r1 & s1 & t1 & u1 & v1 & w1 & x1 & y1 & z1", true);

	return 0;
}
