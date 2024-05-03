#include "acSimulation.hpp"
#include "complex"
#include "numbers"
#include "ranges"


ACSimulation::ACSimulation(const GraphDescriptor &graph) {
	using namespace std::literals;
	if (graph.nodes.size() < 2) return;
	auto incidenceMatrix = generateIncidenceMatrix(graph);
	auto A = Eigen::MatrixXf(incidenceMatrix.bottomRows(incidenceMatrix.rows() - 1));
	auto [mat, piv] = Utils::calculateNonzeroPivots(A);
	auto Ar = Eigen::MatrixXf(A(Eigen::all, piv));
	std::vector<int64_t> indices = [&]() {
		auto temp = Eigen::Vector<int64_t, Eigen::Dynamic>::LinSpaced(mat.cols(), 0, mat.cols() - 1);
		return std::vector<int64_t>(temp.begin(), temp.end());
	}();
	indices.erase(
		std::remove_if(
			indices.begin(),
			indices.end(),
			[&](const int &val) mutable -> bool {
				auto res = std::equal_range(piv.begin(), piv.end(), val);
				return res.first != res.second;
			}
		),
		indices.end()
	);
	auto Ac = A(Eigen::all, indices);

	auto D = Ar.inverse() * Ac;

	auto Br = Eigen::MatrixXf(-D.transpose());
	auto Bc = Eigen::MatrixXf::Identity(Br.rows(), Br.rows());

	auto B = Eigen::MatrixXf(Br.rows(), Br.cols() + Bc.cols());
	B << Br, Bc;

	std::vector<int64_t> Lp{};
	std::vector<int64_t> Le{};
	std::vector<int64_t> Lj{};
	for (const auto &[index, elem]: graph.elements | std::views::enumerate) {
		const auto id = elem.element.component.get().id;
		if (id == 2)
			Le.emplace_back(index);
		else if (id == 3)
			Lj.emplace_back(index);
		else
			Lp.emplace_back(index);
	}

	auto Ap = A(Eigen::all, Lp);
	auto Ae = A(Eigen::all, Le);
	auto Aj = A(Eigen::all, Lj);

	auto Bp = B(Eigen::all, Lp);
	auto Be = B(Eigen::all, Le);
	auto Bj = B(Eigen::all, Lj);

	auto Z = Eigen::VectorXcf::Zero(static_cast<int64_t>(Lp.size())).eval();
	auto Enum = Eigen::VectorXcf::Zero(static_cast<int64_t>(Le.size())).eval();
	auto Jnum = Eigen::VectorXcf::Zero(static_cast<int64_t>(Lj.size())).eval();
	int64_t Zi = 0;
	int64_t Ei = 0;
	int64_t Ji = 0;
	const auto frequency = 50.f;
	const auto omega = 2.f * std::numbers::pi_v<float>  * frequency;
	for (const auto &[index, elem]: graph.elements | std::views::enumerate) {
		const auto &element = elem.element;
		const auto id = elem.element.component.get().id;
		if (id == 2) {
			// Voltage source
			if (element.propertySetIndex == 0) {
				Enum(Ei++) = 0;
				continue;
			}
			const auto amplitude = getFloat(element.propertiesValues.at(0));
			const auto phase = getFloat(element.propertiesValues.at(1)) * std::numbers::pi_v<float> / 180.f;
			Enum(Ei++) = amplitude * std::exp(1if * phase);
		} else if (id == 3) {
			// Current source
			if (element.propertySetIndex == 0) {
				Jnum(Ji++) = 0;
				continue;
			}
			const auto amplitude = getFloat(element.propertiesValues.at(0));
			const auto phase = getFloat(element.propertiesValues.at(1)) * 180.f / std::numbers::pi_v<float>;
			Jnum(Ji++) = amplitude * std::exp(1if * phase);
		} else if (id == 4) {
			// Resistor
			Z(Zi++) = getFloat(element.propertiesValues.at(0));
		} else if (id == 6) {
			// Capacitor
			Z(Zi++) = -1if * (1.f / (omega * getFloat(element.propertiesValues.at(0))));
		} else if (id == 7) {
			// Inductor
			Z(Zi++) = 1if * omega * getFloat(element.propertiesValues.at(0));
		}
	}
	auto Znum = Z.asDiagonal();

	auto Ynum = Znum.inverse();
	auto temp1 = Ap * Ynum * Ap.transpose();

	auto Pb = Eigen::MatrixXcf(temp1.rows() + Le.size(), temp1.cols() + Ae.cols());
	Pb << temp1,
		Ae,
		Ae.transpose(),
		Eigen::MatrixXcf::Zero(static_cast<int64_t>(Le.size()), static_cast<int64_t>(Le.size()));

	Eigen::MatrixXcf Qb;
	if (Jnum.size() != 0) {
		auto _ = Aj * Jnum;
		Qb.resize(_.rows() + Enum.rows(), _.cols());
		Qb << -_, -Enum;
	} else {
		Qb.resize(static_cast<int64_t>(graph.nodes.size() - 1) + Enum.rows(), 1);
		Qb << -Eigen::MatrixXcf::Zero(static_cast<int64_t>(graph.nodes.size() - 1), 1), -Enum;
	}

	auto Xb = Pb.inverse() * Qb;

	auto V_numeric = Xb.topRows(graph.nodes.size() - 1).eval();
	auto Ie_numeric_b = Xb.bottomRows(Xb.rows() - (graph.nodes.size() - 1)).eval();
	std::println("V_numeric");
	std::cout << V_numeric << std::endl;
	std::println("Ie_numeric_b");
	std::cout << Ie_numeric_b << std::endl;

	currents.reserve(Ie_numeric_b.rows());
	for (const auto &[val, id]: std::views::zip(Ie_numeric_b.reshaped(), Le)) {
		currents.emplace_back(id, val);
	}

	voltages.reserve(V_numeric.rows());
	for (auto &val: V_numeric.reshaped()) {
		voltages.emplace_back(val);
	}
}

Eigen::MatrixXf ACSimulation::generateIncidenceMatrix(const GraphDescriptor &graph) {
	Eigen::MatrixXf ret{};
	ret.resize(
		static_cast<int64_t>(graph.nodes.size()),
		static_cast<int64_t>(graph.elements.size())
	);
	ret.fill(0);
	for (const auto &[index, elem]: graph.elements | std::views::enumerate) {
		ret.col(index)(elem.nodes.at(0)) = 1;
		ret.col(index)(elem.nodes.at(1)) = -1;
	}

	return ret;
}
