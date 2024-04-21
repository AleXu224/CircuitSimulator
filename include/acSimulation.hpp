#pragma once
#include "graphDescriptor.hpp"

struct ACSimulation {
	struct Result {
		ElementId id{};
		std::complex<float> value;
	};
	std::vector<Result> currents{};
	std::vector<std::complex<float>> voltages{};

	ACSimulation(const GraphDescriptor &);

	[[nodiscard]] static Eigen::MatrixXf generateIncidenceMatrix(const GraphDescriptor &graph);
};