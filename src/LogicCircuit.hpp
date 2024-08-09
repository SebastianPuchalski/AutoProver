#pragma once

#include "NormalForm.hpp"

#include <vector>

class LogicCircuit {
public:
	using BitSequence = std::vector<bool>;
	struct DataSample {
		BitSequence input;
		BitSequence output;

		DataSample() {}
		DataSample(BitSequence input, BitSequence output) : input(input), output(output) {}
	};

	LogicCircuit();
	virtual ~LogicCircuit() = default;

	void setFcArch(const std::vector<int>& sizes);
	BitSequence infer(const BitSequence& input) const;

	void getTrainCnf(Cnf& cnf, const std::vector<DataSample>& data) const;
	void setTrainModel(const BitSequence& model);

private:
	struct Operation {
		int arg0Idx;
		int arg1Idx;
		int paramIdx;
	};

	static const int paramsPerOp = 4;
	Cnf operationTrainCnf;

	int inputSize;
	std::vector<Operation> operations;
	std::vector<int> outputIndexes;
	std::vector<uint8_t> parameters;

	bool calcOperation(bool a, bool b, uint8_t parameter) const;
	Cnf cnfFromOperation(VariableId in0, VariableId in1, VariableId out, std::vector<VariableId> params) const;
};
