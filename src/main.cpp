#include "Proposition.hpp"
#include "Converter.hpp"

#include <iostream>

using namespace std;

int main() {
	auto a = std::make_shared<Variable>("A");
	auto b = std::make_shared<Variable>("B");
	auto ab = std::make_shared<BinaryOperator>(a, BinaryOperator::AND, b);

	cout << "Operator type: " << ab->getOp() << endl;
	cout << "Node type: " << ab->getType() << endl;
	cout << std::dynamic_pointer_cast<Variable>(ab->getLeft())->getName() << endl;
	cout << std::dynamic_pointer_cast<Variable>(ab->getRight())->getName() << endl;

	Converter conv;
	std::shared_ptr<Proposition> prop = conv.fromString("((a & T) <-> ~c) -> (d | F)");
	
	return 0;
}
