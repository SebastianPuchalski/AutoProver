#include "Proposition.hpp"
#include "Converter.hpp"

#include <iostream>

using namespace std;

int main() {
	Converter conv;

	while (true) {
		std::string str;
		std::getline(std::cin, str);
		std::shared_ptr<Proposition> proposition = conv.fromString(str);
		if (proposition) {
			cout << conv.toString(proposition) << endl;
		}
		else {
			cout << "Error during parsing!\n";
		}
		cout << endl;
	}

	return 0;
}
