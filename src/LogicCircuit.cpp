#include "LogicCircuit.hpp"

#include "Converter.hpp"

#include <cassert>

LogicCircuit::LogicCircuit() : inputSize(0) {
	// c <-> calcOperation(a, b, d, e, f, g)
	// c <-> ((~a & ~b & d) | (~a & b & e) | (a & ~b & f) | (a & b & g))
	// (~a | ~b | ~c | g) & (~a | ~b | c | ~g) & (~a | b | ~c | f) & (~a | b | c | ~f) & (a | ~b | ~c | e) & (a | ~b | c | ~e) & (a | b | ~c | d) & (a | b | c | ~d)
	const std::string propStr = "(~a | ~b | ~c | g) & (~a | ~b | c | ~g) & (~a | b | ~c | f) & (~a | b | c | ~f) & (a | ~b | ~c | e) & (a | ~b | c | ~e) & (a | b | ~c | d) & (a | b | c | ~d)";
	propositionToCnf(operationTrainCnf, Converter().fromString(propStr));
}

void LogicCircuit::setFcArch(const std::vector<int>& sizes) {
	if (sizes.size() < 2) {
		assert(0);
		return;
	}
	auto isPowerOfTwo = [](int n) { return (n > 0) && !(n & (n - 1)); };
	for (int i = 0; i < sizes.size() - 1; i++) {
		if (!isPowerOfTwo(sizes[i]) || sizes[i] < 2) {
			assert(0);
			return;
		}
	}

	inputSize = sizes.front();
	std::vector<int> inIndexes;
	std::vector<int> outIndexes(inputSize);
	for (int i = 0; i < outIndexes.size(); i++)
		outIndexes[i] = i;
	for (int i = 1; i < sizes.size(); i++) {
		inIndexes = outIndexes;
		outIndexes.resize(sizes[i]);
		for (int j = 0; j < outIndexes.size(); j++) {
			std::vector<int> treeInIndexes = inIndexes;
			while (treeInIndexes.size() > 1) {
				for (int k = 0; k < treeInIndexes.size() / 2; k++) {
					Operation operation;
					operation.arg0Idx = treeInIndexes[k * 2];
					operation.arg1Idx = treeInIndexes[k * 2 + 1];
					operation.paramIdx = parameters.size();
					treeInIndexes[k] = operations.size() + inputSize;
					operations.push_back(operation);
					parameters.push_back(uint8_t());
				}
				treeInIndexes.resize(treeInIndexes.size() / 2);
			}
			outIndexes[j] = treeInIndexes.front();
		}
	}
	outputIndexes = outIndexes;
}

LogicCircuit::BitSequence LogicCircuit::infer(const BitSequence& input) const {
	assert(inputSize == input.size());
	BitSequence values(input.size() + operations.size());

	for (int i = 0; i < inputSize; i++)
		values[i] = input[i];

	int index = inputSize;
	for (auto& operation : operations) {
		assert(operation.arg0Idx < index);
		assert(operation.arg1Idx < index);
		assert(operation.paramIdx < parameters.size());
		bool value0 = values[operation.arg0Idx];
		bool value1 = values[operation.arg1Idx];
		uint8_t params = parameters[operation.paramIdx];
		values[index] = calcOperation(value0, value1, params);
		index++;
	}

	BitSequence output(outputIndexes.size());
	for (int i = 0; i < output.size(); i++)
		output[i] = values[outputIndexes[i]];
	return output;
}

void LogicCircuit::getTrainCnf(Cnf& cnf, const std::vector<DataSample>& data) const {
	int varIdOffset = parameters.size() * paramsPerOp;
	for (int s = 0; s < data.size(); s++) {
		auto& sample = data[s];
		assert(sample.input.size() == inputSize);
		assert(sample.output.size() == outputIndexes.size());
		for (int op = 0; op < operations.size(); op++) {
			auto& operation = operations[op];
			auto opCnf = cnfFromOperation(operation.arg0Idx + varIdOffset,
				                          operation.arg1Idx + varIdOffset,
				                          op + inputSize + varIdOffset,
				                          {operation.paramIdx * paramsPerOp + 0,
										   operation.paramIdx * paramsPerOp + 1,
										   operation.paramIdx * paramsPerOp + 2,
										   operation.paramIdx * paramsPerOp + 3});
			cnf.insert(cnf.end(), opCnf.begin(), opCnf.end());
		}
		for (int i = 0; i < inputSize; i++) {
			Clause clause;
			clause.push_back(Literal(i + varIdOffset, !sample.input[i]));
			cnf.push_back(clause);
		}
		for (int i = 0; i < outputIndexes.size(); i++) {
			Clause clause;
			clause.push_back(Literal(outputIndexes[i] + varIdOffset, !sample.output[i]));
			cnf.push_back(clause);
		}
		varIdOffset += inputSize + operations.size();
	}
}

void LogicCircuit::setTrainModel(const BitSequence& model) {
	assert(model.size() >= parameters.size() * paramsPerOp);
	int modelIdx = 0;
	for (int i = 0; i < parameters.size(); i++) {
		uint8_t params = 0;
		for (int j = 0; j < paramsPerOp; j++)
			if(model[modelIdx++])
				params |= (1 << j);
		parameters[i] = params;
	}
}

bool LogicCircuit::calcOperation(bool a, bool b, uint8_t parameters) const {
	int truthTableIdx = a * 2 + b;
	return ((1 << truthTableIdx) & parameters) != 0;
}

Cnf LogicCircuit::cnfFromOperation(VariableId in0, VariableId in1, VariableId out,
	                               std::vector<VariableId> params) const {
	assert(params.size() == paramsPerOp);
	// a: in0, b: in1, c: out, d: p0, e: p1, f: p2, g: p3
	VariableId map[] = { in0, in1, out, params[0], params[1], params[2], params[3] };
	Cnf cnf = operationTrainCnf;
	for (auto& clause : cnf) {
		for (auto& literal : clause) {
			literal.varId = map[literal.varId];
		}
	}
	return cnf;
}
