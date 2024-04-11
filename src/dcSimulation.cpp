#include "dcSimulation.hpp"
#include "ranges"

DCSimulation::DCSimulation(const GraphDescriptor &graph) {
    if (graph.nodes.size() < 2) return;
	auto graphMatrix = generateGraphMatrix(graph);
	auto incidenceMatrix = generateIncidenceMatrix(graph, graphMatrix);
	auto A = Eigen::MatrixXf(incidenceMatrix.bottomRows(incidenceMatrix.rows() - 1));
	auto [mat, piv] = Utils::calculateNonzeroPivots(A);
	auto Ar = Eigen::MatrixXf(A(Eigen::all, piv));
	std::vector<int64_t> indices = [&]() {
		auto temp = Eigen::Vector<int64_t, Eigen::Dynamic>::LinSpaced(mat.cols(), 0, mat.cols() - 1);
		return std::vector<int64_t>(temp.begin(), temp.end());
	}();
	indices.erase(std::remove_if(indices.begin(), indices.end(), [&](const int &val) mutable -> bool {
					  auto res = std::equal_range(piv.begin(), piv.end(), val);
					  return res.first != res.second;
				  }),
				  indices.end());
	auto Ac = A(Eigen::all, indices);

	auto D = Ar.inverse() * Ac;

	auto Br = Eigen::MatrixXf(-D.transpose());
	auto Bc = Eigen::MatrixXf::Identity(Br.rows(), Br.rows());

	auto B = Eigen::MatrixXf(Br.rows(), Br.cols() + Bc.cols());
	B << Br, Bc;

	std::vector<int64_t> Lp{};
	std::vector<int64_t> Le{};
	std::vector<int64_t> Lj{};
	for (int64_t i = 0; i < graphMatrix.cols(); i++) {
		auto val = graphMatrix(1, i);
		if (val == 2)
			Le.emplace_back(i);
		else if (val == 3)
			Lj.emplace_back(i);
		else
			Lp.emplace_back(i);
	}

	auto Ap = A(Eigen::all, Lp);
	auto Ae = A(Eigen::all, Le);
	auto Aj = A(Eigen::all, Lj);

	auto Bp = B(Eigen::all, Lp);
	auto Be = B(Eigen::all, Le);
	auto Bj = B(Eigen::all, Lj);

	auto params = graphMatrix.row(4);
	auto R = Eigen::MatrixXf(params(Lp).asDiagonal());
	auto E = params(Le).transpose();
	auto J = params(Lj).transpose();

	auto G = R.inverse();
	auto temp1 = Ap * G * Ap.transpose();

	auto Pb = Eigen::MatrixXf(temp1.rows() + Le.size(), temp1.cols() + Ae.cols());
	Pb << temp1,
		Ae,
		Ae.transpose(),
		Eigen::MatrixXf::Zero(static_cast<int64_t>(Le.size()), static_cast<int64_t>(Le.size()));

	Eigen::MatrixXf Qb;
	if (J.size() != 0) {
		auto _ = Aj * J;
		Qb.resize(_.rows() + E.rows(), _.cols());
		Qb << -_, -E;
	} else {
		Qb.resize(static_cast<int64_t>(graph.nodes.size() - 1) + E.rows(), 1);
		Qb << -Eigen::MatrixXf::Zero(static_cast<int64_t>(graph.nodes.size() - 1), 1), -E;
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

Eigen::MatrixXf DCSimulation::generateGraphMatrix(const GraphDescriptor &graph) {
	constexpr size_t maxNodeConnections = 2;
	const size_t vectorSizeReq = 1 /*Element id*/ + 1 /*Component id*/ + maxNodeConnections + 1 /*Property value*/;
	std::vector<Eigen::VectorXf> vecs{};
	vecs.reserve(graph.elements.size());
	for (const auto &element: graph.elements) {
		Eigen::VectorXf ret{};
		ret.resize(vectorSizeReq);
		ret[0] = static_cast<float>(element.element.id);
		ret[1] = static_cast<float>(element.element.component.get().id);
		int64_t index = 2;
		for (auto &connection: element.nodes | std::views::take(maxNodeConnections)) {
			ret[index++] = static_cast<float>(connection);
		}
		ret[4] = element.element.propertiesValues.front();
		vecs.emplace_back(std::move(ret));
	}

	std::sort(vecs.begin(), vecs.end(), [](const auto &a, const auto &b) {
		return a[0] < b[0];
	});
	Eigen::MatrixXf ret{};
	ret.resize(vectorSizeReq, static_cast<int64_t>(vecs.size()));
	for (const auto &[index, vec]: vecs | std::views::enumerate) {
		ret.col(index) = vec;
	}

	return ret;
}

Eigen::MatrixXf DCSimulation::generateIncidenceMatrix(const GraphDescriptor &graph, const Eigen::MatrixXf &graphMatrix) {
	Eigen::MatrixXf ret{};
	ret.resize(
		static_cast<int64_t>(graph.nodes.size()),
		static_cast<int64_t>(graph.elements.size())
	);
	ret.fill(0);
	for (int64_t i = 0; i < graphMatrix.cols(); i++) {
		const auto &graphCol = graphMatrix.col(i);
		ret.col(i)(static_cast<int64_t>(std::round(graphCol[2]))) = 1;
		ret.col(i)(static_cast<int64_t>(std::round(graphCol[3]))) = -1;
	}

	return ret;
}
