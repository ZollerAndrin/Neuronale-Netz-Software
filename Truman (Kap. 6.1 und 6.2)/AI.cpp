#include "AI.h"

double AI::sigmoidFunction(double x) {
	return 1 / (1 + exp(-x));
}
double AI::ReLUFunction(double x) {
	return (x > 0) ? x : 0;
}
double AI::sigmoidFunction_derivative(double x) {
	return AI::sigmoidFunction(x) * (1 - AI::sigmoidFunction(x));
}
double AI::ReLUFunction_derivative(double x) {
	return (x > 0) ? 1 : 0;
}
std::vector<double> AI::softmax(std::vector<double> activations) {
	std::vector<double> outVector;
	double divider = 0;
	for (auto a : activations) {
		divider += a;
	}
	for (int i = 0; i < activations.size(); ++i) {
		outVector.push_back(activations[i] / divider);
	}
	return outVector;
}
std::vector<double> AI::softmax_derivative(std::vector<double> activations) {
	std::vector<double> outVector(9,0);
	std::vector<double> Softmax = softmax(activations);
	for (int i = 0; i < activations.size(); ++i) {
		//rest = sum of activations from all outputneurons except for i
		double rest = 0;
		for (int j = 0; j < activations.size(); ++j) {
			rest += activations[j];
		}
		rest -= activations[i];
		outVector[i] = rest / pow(rest + activations[i], 2);
	}
	return outVector;
}
double AI::costfunction(double activation, double expected_output) {
	return 1.0 / 2.0 * pow(activation - expected_output, 2.0);
}
double AI::costfunction_derivative(double activation, double expected_output) {
	return activation - expected_output;
}
double AI::crossentropyCostFuntion(double activation, double expected_output) {
	if (expected_output == 1) {
		return -log(activation);
	}
	else {
		return -log(1 - activation);
	}
}
double AI::crossentropyCostFuntion_derivative(double activation, double expected_output) {
	return 0;
}
AI::AI(std::vector<int> extent) {
	//each integer in extent represents one layer of neurons(including input & output),
	//holding the number of neurons in that layer
	this->extent = extent;
	MatrixGenerator(this->normalDistribution, this->weights, parameter::weight);
	MatrixGenerator(this->normalDistribution, this->biases, parameter::bias);
}
AI::AI(std::vector<std::vector<std::vector<double>>> weights, std::vector<std::vector<std::vector<double>>> biases) {
	//initializing nn with specific parameters
	this->extent.push_back((signed)weights[0][0].size());
	for (int a = 0; a < (signed)weights.size(); ++a) {
		this->extent.push_back((signed)weights[a].size());
	}
	this->weights = weights;
	this->biases = biases;
}
void AI::MatrixGenerator(double f(), std::vector<std::vector<std::vector<double>>>& matrix, parameter p) {
	//generate bias and weight matrices with values produced by f()
	for (int a = 1; a < (signed)this->extent.size(); ++a) {
		std::vector<std::vector<double>> m2;
		for (int b = 0; b < this->extent[a]; ++b) {
			std::vector<double> m1;
			if (p == parameter::weight) {
				for (int c = 0; c < this->extent[a - 1]; ++c) {
					m1.push_back(f());
				}
			}
			else {
				m1.push_back(f());
			}
			m2.push_back(m1);
			m1.clear();
		}
		matrix.push_back(m2);
		m2.clear();
	}
}
double AI::normalDistribution() {
	//generate number in normal distribution range
	unsigned seed = (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine gen(seed);
	std::normal_distribution<double> dist(0.0, 1.0);
	double r = dist(gen);
	return r;
}
double AI::zero() {
	return 0.0;
}
std::vector<double> AI::feedforward(std::vector<double> input, std::vector<std::vector<double>>& activations, std::vector<std::vector<double>>& weightedInputs) {
	//input processed through all layers
	std::vector<double> output;
	activations.push_back(input);
	for (int a = 0; a < (signed)this->extent.size() - 1; ++a) {
		output = {};
		std::vector<double> wI;
		for (int b = 0; b < this->extent[a + 1]; ++b) {
			wI.push_back(0);
			for (int c = 0; c < (signed)input.size(); ++c) {
				wI[b] += this->weights[a][b][c] * input[c];
			}
			if (a < (signed)this->extent.size() - 2) {
				output.push_back(this->ReLUFunction(wI[b]));
			}
		}
		if (a == (signed)this->extent.size() - 2) {
			output = softmax(wI);
		}
		input = output;
		activations.push_back(output);
		weightedInputs.push_back(wI);
		wI.clear();
	}
	return output;
	output.clear();
}
void AI::backpropagation(std::vector<double> input, std::vector<double> output, std::vector<std::vector<std::vector<double>>>& deltaW, std::vector<std::vector<std::vector<double>>>& deltaB) {
	//find direction in which the parameters have to be changed, to minimize the costfunction by going through the function backwards

	//fill matrices with 0's to the same extent as the weights and biases matrices to store the deltas
	MatrixGenerator(this->zero, deltaW, parameter::weight);
	MatrixGenerator(this->zero, deltaB, parameter::bias);
	std::vector<std::vector<double>> activations;
	std::vector<std::vector<double>> weightedInputs;
	//calling feedforwardfunction to find activations and weightedinputs 
	this->feedforward(input, activations, weightedInputs);

	//finding deltas for the parameters using the derivative of the costfunction
	std::vector<double> smd = softmax_derivative(activations.end()[-1]);
	for (int b = 0; b < this->extent.end()[-1]; ++b) {
		deltaB.end()[-1][b][0] = costfunction_derivative(activations.end()[-1][b], output[b]) * smd[b];
		for (int c = 0; c < this->extent.end()[-2]; ++c) {
			deltaW.end()[-1][b][c] = deltaB.end()[-1][b][0] * activations.end()[-2][c];
		}
	}
	//find deltas for the parameters by using the chainrule
	for (int a = (signed)this->extent.size() - 3; a >= 0; a--) {
		for (int b = 0; b < this->extent[a + 1]; ++b) {
			for (int c = 0; c < this->extent[a + 2]; ++c) {
				deltaB[a][b][0] += deltaW[a + 1][c][b];
			}
			deltaB[a][b][0] *= ReLUFunction_derivative(weightedInputs[a][b]);
			for (int c = 0; c < this->extent[a]; ++c) {
				deltaW[a][b][c] = deltaB[a][b][0] * activations[a][c];
			}
		}
	}
	activations.clear();
	weightedInputs.clear();
}

void AI::updateparams(std::vector<std::vector<double>> inputs, std::vector<std::vector<double>> outputs, double learningrate) {
	//Using Gradient Descent to change the weights and biases accordingly
	int batchsize = (signed)inputs.size();
	std::vector<std::vector<std::vector<double>>> deltaW;
	std::vector<std::vector<std::vector<double>>> deltaB;
	MatrixGenerator(this->zero, deltaW, parameter::weight);
	MatrixGenerator(this->zero, deltaB, parameter::bias);
	std::vector<std::thread> threads;
	for (int i = 0; i < batchsize; ++i) {
		/*std::thread t(&AI::batchupdate, this, deltaW, deltaB, inputs[i], outputs[i]);
		threads.push_back(t);*/
		std::vector<std::vector<std::vector<double>>> partial_deltaW;
		std::vector<std::vector<std::vector<double>>> partial_deltaB;
		backpropagation(inputs[i], outputs[i], partial_deltaW, partial_deltaB);
		for (int a = 0; a < (signed)extent.size() - 1; ++a) {
			for (int b = 0; b < extent[a + 1]; ++b) {
				deltaB[a][b][0] += partial_deltaB[a][b][0];
				for (int c = 0; c < extent[a]; ++c) {
					deltaW[a][b][c] += partial_deltaW[a][b][c];
				}
			}
		}
		partial_deltaB.clear();
		partial_deltaW.clear();
	}
	for (int a = 0; a < (signed)this->extent.size() - 1; ++a) {
		for (int b = 0; b < this->extent[a + 1]; ++b) {
			this->biases[a][b][0] -= learningrate * deltaB[a][b][0] / (double)batchsize;
			for (int c = 0; c < this->extent[a]; ++c) {
				this->weights[a][b][c] -= learningrate * deltaW[a][b][c] / (double)batchsize;
			}
		}
	}
	deltaW.clear();
	deltaB.clear();
}
void AI::train(std::vector<std::vector<std::vector<double>>> training_data, std::vector<std::vector<double>> test_inputs, std::vector<std::vector<double>> test_outputs, int iterations, int batchsize, double learningrate) {
	//calculate the number of batches by dividing the amount of traininginputssamples by size of one batch given by user
	int batchCount = (signed)training_data[0].size() / batchsize;
	std::vector<std::vector<std::vector<double>>> batchesIn;
	std::vector<std::vector<std::vector<double>>> batchesOut;
	//shuffle trainingdata
	std::vector<int> indexes;
	for (int i = 0; i < (signed)training_data[0].size(); ++i) {
		indexes.push_back(i);
	}
	std::random_shuffle(indexes.begin(), indexes.end());
	std::vector<std::vector<std::vector<double>>> training_data_temp = { {},{} };
	for (int i = 0; i < (signed)training_data[0].size(); ++i) {
		training_data_temp[0].push_back(training_data[0][indexes[i]]);
		training_data_temp[1].push_back(training_data[1][indexes[i]]);
	}
	training_data = training_data_temp;
	training_data_temp.clear();
	//fill batchmatrices
	for (int a = 0; a < batchCount; ++a) {
		if ((signed)training_data[0][a].size() > 0) {
			std::vector<std::vector<double>> batchIn;
			std::vector<std::vector<double>> batchOut;
			for (int b = 0; b < batchsize; ++b) {
				if ((signed)training_data[0][a * b].size() > 0) {
					batchIn.push_back(training_data[0][a * b]);
					batchOut.push_back(training_data[1][a * b]);
				}
				else {
					--a;
					--batchCount;
				}
			}
			if (batchIn.size() > 0) {
				batchesIn.push_back(batchIn);
				batchesOut.push_back(batchOut);
				batchIn.clear();
				batchOut.clear();
			}
		}
	}
	//go trough each batch and train the NN on it <-- doing this iteration times
	for (int i = 0; i < iterations; ++i) {
		for (int a = 0; a < batchCount; ++a) {
			this->updateparams(batchesIn[a], batchesOut[a], learningrate);
		}
		std::cout << "						loss" << i << ": " << this->evaluate(test_inputs, test_outputs) << std::endl;
	}
	batchesIn.clear();
	batchesOut.clear();
}
void AI::getparams(std::vector<std::vector<std::vector<double>>>& weights, std::vector<std::vector<std::vector<double>>>& biases) {
	//get parameters through a public class for later use(such as saving them on a txt file and using them again later)	
	weights = this->weights;
	biases = this->biases;
}
std::vector<double> AI::run(std::vector<double> input) {
	//slightly faster version of feedforward
	std::vector<double> output;
	for (int a = 0; a < (signed)this->extent.size() - 1; ++a) {
		output = {};
		std::vector<double> wI;
		for (int b = 0; b < this->extent[a + 1]; ++b) {
			wI.push_back(0);
			for (int c = 0; c < (signed)input.size(); ++c) {
				wI[b] += this->weights[a][b][c] * input[c];
			}
			if (a < (signed)this->extent.size() - 2) {
				output.push_back(this->ReLUFunction(wI[b]));
			}
		}
		if (a == (signed)this->extent.size() - 2) {
			output = softmax(wI);
		}
		input = output;
	}
	return output;
	output.clear();
}
double AI::evaluate(std::vector<std::vector<double>> inputs, std::vector<std::vector<double>> expected_outputs) {
	//return the average loss over all outputneurons and inputsamples
	std::vector<std::vector<double>> outputs;
	double loss = 0;
	double scale = (double)(signed)inputs.size();
	for (int a = 0; a < (signed)inputs.size(); ++a) {
		outputs.push_back(this->run(inputs[a]));
		std::vector<double> activationsAfterCorrection{ 0,0,0,0,0,0,0,0,0 };
		int moveMade = 0;
		for (int b = 0; b < extent.end()[-1]; b++) {
			if (activationsAfterCorrection[moveMade] < outputs[a][b]) {
				activationsAfterCorrection[b] = outputs[a][b];
				activationsAfterCorrection[moveMade] = 0;
				moveMade = b;
			}
		}
		loss += this->costfunction(activationsAfterCorrection[moveMade], expected_outputs[a][moveMade]) / scale;

	}
	outputs.clear();
	return loss;
}
double AI::getloss(std::vector<std::vector<double>> inputs, std::vector<std::vector<double>> expected_outputs) {
	//public function to return loss and prints weights and biases to console for testing purposes
	for (int a = 0; a < (signed)this->extent.size() - 1; ++a) {
		for (int b = 0; b < this->extent[a + 1]; ++b) {
			std::cout << "weight : " << std::endl << "	";
			for (int c = 0; c < this->extent[a]; ++c) {
				std::cout << this->weights[a][b][c] << ", ";
			}
			std::cout << std::endl << std::endl;
		}
	}
	for (int a = 0; a < (signed)this->extent.size() - 1; ++a) {
		std::cout << "bias : " << std::endl << "	";
		for (int b = 0; b < this->extent[a + 1]; ++b) {
			std::cout << this->biases[a][b][0] << ", ";
		}
		std::cout << std::endl << std::endl;
	}
	std::vector<std::vector<double>> activations, weightedInputs;
	feedforward(inputs[0], activations, weightedInputs);

	for (int a = 0; a < (signed)activations.size() - 1; ++a) {
		std::cout << "bias : " << std::endl << "	";
		for (int b = 0; b < (signed)activations[a].size(); ++b) {
			std::cout << activations[a][b] << ", ";
		}
		std::cout << std::endl << std::endl;
	}
	for (int a = 0; a < (signed)weightedInputs.size() - 1; ++a) {
		std::cout << "bias : " << std::endl << "	";
		for (int b = 0; b < (signed)weightedInputs[a].size(); ++b) {
			std::cout << weightedInputs[a][b] << ", ";
		}
		std::cout << std::endl << std::endl;
	}
	return this->evaluate(inputs, expected_outputs);
}
