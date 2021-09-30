#include "Primitive.h"

vector<float> readNumbers(const string& command, int now) {
	vector<float> nums;
	nums.reserve(10);
	while (now<command.size()) {
		while (now < command.size() && (!isdigit(command[now]) && command[now] != '.' && command[now] != '-'))
			now++;
		int pos = command.find(" ", now);
		if (pos == std::string::npos) {
			nums.push_back(stof(command.substr(now)));
			break;
		}
		else {
			nums.push_back(stof(command.substr(now, pos)));
			now = pos+1;
		}
	}

	return nums;
}

vector<vector<float>> matrixMultiplication(const vector<vector<float>>& matrixA, const vector<vector<float>>& matrixB) {
	if (matrixA[0].size() != matrixB.size()) {
		cout << "matrices size not match" << endl;
		return {};
	}

	vector<vector<float>> resultMatrix(matrixA.size(), vector<float>(matrixB[0].size(), 0.0f));
	for (int i = 0; i < matrixA.size(); i++) {
		for (int j = 0; j < matrixB[0].size(); j++) {
			for (int k = 0; k < matrixB.size(); k++) {
				resultMatrix[i][j] += matrixA[i][k] * matrixB[k][j];
			}
		}
	}

	return resultMatrix;
}