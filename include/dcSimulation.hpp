#pragma once
#include "graphDescriptor.hpp"

struct DCSimulation {
	struct Result {
		ElementId id;
		float value;
	};
	std::vector<Result> currents{};
	std::vector<float> voltages{};

	DCSimulation(const GraphDescriptor &);

	[[nodiscard]] static Eigen::MatrixXf generateIncidenceMatrix(const GraphDescriptor &graph);
};