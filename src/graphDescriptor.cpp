#include "graphDescriptor.hpp"
#include "boardLine.hpp"
#include "element.hpp"
#include <algorithm>
#include <chrono>
#include <functional>
#include <optional>
#include <print>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>

GraphDescriptor::GraphDescriptor(BoardStorage &board) {
	exploreBoard(board);
	generateGraphMatrix();
	generateIncidenceMatrix();
	auto A = Eigen::MatrixXf(incidenceMatrix.topRows(incidenceMatrix.rows() - 1));
	auto [mat, piv] = GraphDescriptor::calculateNonzeroPivots(A);
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
	std::println("A");
	std::cout << A << std::endl;
	std::println("Ar");
	std::cout << Ar << std::endl;
	std::println("Ac");
	std::cout << Ac << std::endl;

	auto D = Ar.inverse() * Ac;
	std::println("D");
	std::cout << D << std::endl;

	auto Br = Eigen::MatrixXf(-D.transpose());
	auto Bc = Eigen::MatrixXf::Identity(Br.rows(), Br.rows());

	auto B = Eigen::MatrixXf(Br.rows(), Br.cols() + Bc.cols());
	B << Br, Bc;
	std::println("B");
	std::cout << B << std::endl;

	std::println("Br");
	std::cout << Br << std::endl;

	std::println("Bc");
	std::cout << Bc << std::endl;

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
	std::println("Lp");
	for (const auto &val: Lp) {
		std::print("{} ", val);
	}
	std::print("\n");
	std::println("Le");
	for (const auto &val: Le) {
		std::print("{} ", val);
	}
	std::print("\n");
	std::println("Lj");
	for (const auto &val: Lj) {
		std::print("{} ", val);
	}
	std::print("\n");

	auto Ap = A(Eigen::all, Lp);
	auto Ae = A(Eigen::all, Le);
	auto Aj = A(Eigen::all, Lj);
	std::println("Ap");
	std::cout << Ap << std::endl;
	std::println("Ae");
	std::cout << Ae << std::endl;
	std::println("Aj");
	std::cout << Aj << std::endl;

	// std::vector<int64_t> col_order_B{};
	// col_order_B.reserve(graphMatrix.cols());
	// for (const auto &el: piv) col_order_B.emplace_back(el);
	// for (const auto &el: indices) col_order_B.emplace_back(el);
	// std::println("col_order_B");
	// for (const auto &val: col_order_B) {
	// 	std::print("{} ", val);
	// }
	// std::print("\n");

	auto Bp = B(Eigen::all, Lp);
	auto Be = B(Eigen::all, Le);
	auto Bj = B(Eigen::all, Lj);
	// TODO: Up Ue Uj
	// TODO: params
	auto params = graphMatrix.row(4);
	std::println("params");
	std::cout << params << std::endl;
	auto R = Eigen::MatrixXf(params(Lp).asDiagonal());
	auto E = params(Le).transpose();
	auto J = params(Lj).transpose();

	std::println("R");
	std::cout << R << std::endl;
	std::println("E");
	std::cout << E << std::endl;
	std::println("J");
	std::cout << J << std::endl;

	auto G = R.inverse();
	auto temp1 = Ap * G * Ap.transpose();

	auto Pb = Eigen::MatrixXf(temp1.rows() + Le.size(), temp1.cols() + Ae.cols());
	Pb << temp1, Ae,
		Ae.transpose(), Eigen::MatrixXf::Zero(static_cast<int64_t>(Le.size()), static_cast<int64_t>(Le.size()));

	std::println("Pb");
	std::cout << Pb << std::endl;

	Eigen::MatrixXf Qb;
	if (J.size() != 0) {
		auto _ = Aj * J;
		Qb.resize(_.rows() + E.rows(), _.cols());
		Qb << -_, -E;
	} else {
		Qb.resize(static_cast<int64_t>(nodes.size() - 1) + E.rows(), 1);
		Qb << -Eigen::MatrixXf::Zero(static_cast<int64_t>(nodes.size() - 1), 1), -E;
	}

	std::println("Qb");
	std::cout << Qb << std::endl;

	auto Xb = Pb.inverse() * Qb;

	std::println("Xb");
	std::cout << Xb << std::endl;

	auto V_numeric = Xb.topRows(nodes.size() - 1);
	auto Ie_numeric_b = Xb.bottomRows(Xb.rows() - (nodes.size() - 1));
	std::println("V_numeric");
	std::cout << V_numeric << std::endl;
	std::println("Ie_numeric_b");
	std::cout << Ie_numeric_b << std::endl;
}

std::optional<GraphDescriptor::ExpandNodeResult> GraphDescriptor::expandNode(
	size_t nodeIndex,
	const Coords &coords,
	const GraphElement &graphElement,
	ExplorationState &state
) {
	struct Args {
		const size_t nodeIndex;
		ExpandNodeResult ret{};
		bool alreadyExplored = false;
		std::unordered_set<uint32_t> locallyExploredLines{};
		const GraphElement &graphElement;
		ExplorationState &state;
		GraphDescriptor &self;
	} args{
		.nodeIndex = nodeIndex,
		.graphElement = graphElement,
		.state = state,
		.self = *this,
	};

	auto crawler = [](this auto &&self, Args &args, const Coords &coords) -> void {
		const auto connections = args.state.board.connections.at(coords).connections;
		for (const auto &connection: connections) {
			const Element &elemData = std::invoke([&]() -> const Element & {
				if (auto _ = args.state.board.getElement(connection.elementId); _.has_value()) return _->get().element;
				return args.state.board.getLine(connection.elementId)->get().element;
			});
			// Early returns:
			// Both in the case of elements and lines if one is found that has already been explored before
			// then it is not worth exploring this any further since it is guaranteed that another
			// node exploration has already explored this or will do so in the future
			switch (elemData.component.get().type) {
				case ElementType::Other: {
					args.ret.elements.push_back(elemData);
					if (elemData.id == args.graphElement.element.id) continue;

					if (args.state.exploredElements.contains(elemData.id)) {
						args.alreadyExplored = true;
						return;
					}
					// Set the element connection to the current node
					for (const auto &connection: args.state.board.connections.at(coords).connections) {
						if (connection.elementId == elemData.id) {
							auto [it, _] = args.self.elements.emplace(GraphElement{.element{elemData}});
							it->nodes.at(connection.nodeIndex) = args.state.nodeIdCounter;
							args.state.exploredConnections.insert(coords);
							break;
						}
					}

					continue;
				}
				case ElementType::Ground:
				case ElementType::Line: {
					if (args.state.exploredLines.contains(elemData.id)) {
						args.alreadyExplored = true;
						args.graphElement.nodes.at(args.nodeIndex) = args.state.exploredLines.at(elemData.id);
						return;
					}
					if (args.locallyExploredLines.contains(elemData.id)) {
						continue;
					}
					args.locallyExploredLines.emplace(elemData.id);
					args.ret.lines.push_back(elemData);

					const std::vector<Coords> connectionCoords = [&]() -> std::vector<Coords> {
						switch (elemData.component.get().type) {
							case ElementType::Line: {
								return {
									elemData.nodes.at(0) + elemData.pos,
									elemData.nodes.at(1) + elemData.pos,
								};
							}
							case ElementType::Ground: {
								std::vector<Coords> ret{};
								for (const auto &elem: args.state.board.elements) {
									if (elem.element.component.get().type != ElementType::Ground) continue;
									for (const auto &node: elem.element.nodes) {
										ret.emplace_back(node + elem.element.pos);
									}
								}
								return ret;
							}
							default:
								std::unreachable();
						}
					}();
					for (const auto &connectionCoord: connectionCoords) {
						if (connectionCoord == coords) continue;

						self(args, connectionCoord);
						if (args.alreadyExplored) return;
					}
				}
			}
		}
	};
	crawler(args, coords);
	if (args.alreadyExplored) return std::nullopt;

	return args.ret;
}

void GraphDescriptor::exploreBoard(BoardStorage &board) {
	if (board.elements.empty()) return;
	auto startTime = std::chrono::steady_clock::now();
	ExplorationState state{
		.board = board,
	};

	// Will add the first ground element to the list of elementsToTraverse
	// this is to ensure that the ground node always gets the index 0
	auto groundIt = std::find_if(
		board.elements.begin(),
		board.elements.end(),
		[](const ElementData &elem) {
			return elem.element.component.get().type == ElementType::Ground;
		}
	);
	state.elementsToTraverse.insert(
		groundIt == board.elements.end()
			? board.elements.begin()->element
			: groundIt->element
	);

	while (!state.elementsToTraverse.empty()) {
		// Copy the elements to traverse since the container will need to be modified while iterating through it
		// which could cause the iterators to be invalidated
		auto elemsToTraverse = state.elementsToTraverse;

		for (const auto &elem: elemsToTraverse) {
			auto removeFromQueue = [&]() {
				if (auto el = state.elementsToTraverse.find(elem); el != state.elementsToTraverse.end()) {
					state.elementsToTraverse.erase(el);
				}
			};
			if (state.exploredElements.contains(elem.id)) {
				removeFromQueue();
				continue;
			}
			state.elementsToTraverse.erase(state.elementsToTraverse.find(elem));
			state.exploredElements.emplace(elem.id);

			auto [graphElement, _] = elements.emplace(GraphElement{
				.element = elem,
			});

			for (const auto &[index, node]: elem.nodes | std::views::enumerate) {
				const auto coords = node + elem.pos;
				if (state.exploredConnections.contains(coords)) {
					continue;
				}
				auto res = expandNode(index, coords, *graphElement, state);
				// Check if the expanded node has been explored before
				if (!res.has_value()) {
					continue;
				}

				elements.find(GraphElement{.element{elem}})->nodes.at(index) = state.nodeIdCounter;
				auto &resVal = res.value();
				for (auto &line: resVal.lines) {
					state.exploredLines.emplace(line.id, state.nodeIdCounter);
				}
				for (auto &element: resVal.elements) {
					if (state.exploredElements.contains(element.id)) continue;
					if (state.elementsToTraverse.contains(element)) continue;
					state.elementsToTraverse.insert(element);
				}
				nodes.emplace(
					state.nodeIdCounter++,
					Node{
						.elements{std::make_move_iterator(resVal.elements.begin()), std::make_move_iterator(resVal.elements.end())},
						.lines{std::make_move_iterator(resVal.lines.begin()), std::make_move_iterator(resVal.lines.end())},
					}
				);
			}
		}
	}

	{
		const auto it = std::find_if(
			elements.begin(),
			elements.end(),
			[](const GraphElement &elem) {
				return elem.element.component.get().type == ElementType::Ground;
			}
		);
		if (it != elements.end()) elements.erase(it);
	}

	auto endTime = std::chrono::steady_clock::now();
	std::println("----------------");
	std::println("Time taken: {}", endTime - startTime);
	std::println("Lines explored: {}", state.exploredLines.size());
	std::println("Elements explored: {}", state.exploredElements.size());
	for (auto &[nodeId, node]: nodes) {
		std::println("Node: {}, with {} lines and {} elements", nodeId, node.lines.size(), node.elements.size());
	}
	for (const auto &element: elements) {
		std::stringstream vectorNodeIds{};
		for (const auto &node: element.nodes) {
			vectorNodeIds << "N" << node << " ";
		}
		std::println("{} #{} {}", element.element.component.get().name, element.element.id, vectorNodeIds.str());
	}
}

void GraphDescriptor::generateGraphMatrix() {
	constexpr size_t maxNodeConnections = 2;
	const size_t vectorSizeReq = 1 /*Element id*/ + 1 /*Component id*/ + maxNodeConnections + 1 /*Property value*/;
	std::vector<Eigen::VectorXf> vecs{};
	vecs.reserve(elements.size());
	for (const auto &element: elements) {
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
	graphMatrix.resize(vectorSizeReq, static_cast<int64_t>(vecs.size()));
	for (const auto &[index, vec]: vecs | std::views::enumerate) {
		graphMatrix.col(index) = vec;
	}

	std::println("Graph Matrix:");
	std::cout << graphMatrix << std::endl;
}

void GraphDescriptor::generateIncidenceMatrix() {
	incidenceMatrix.resize(
		static_cast<int64_t>(nodes.size()),
		static_cast<int64_t>(elements.size())
	);
	incidenceMatrix.fill(0);
	for (int64_t i = 0; i < graphMatrix.cols(); i++) {
		const auto &graphCol = graphMatrix.col(i);
		incidenceMatrix.col(i)(static_cast<int64_t>(std::round(graphCol[2]))) = 1;
		incidenceMatrix.col(i)(static_cast<int64_t>(std::round(graphCol[3]))) = -1;
	}

	std::println("Incidence Matrix:");
	std::cout << incidenceMatrix << std::endl;
}

std::tuple<Eigen::MatrixXf, std::vector<int64_t>> GraphDescriptor::calculateNonzeroPivots(Eigen::MatrixXf &input) {
	auto ret{input};
	std::vector<int64_t> nonzeroPivots{};
	const auto rows = input.rows();
	const auto columns = input.cols();

	for (int64_t i = 0, j = 0; i < rows && j < columns;) {
		auto &elem = ret(i, j);

		if (elem == 0) {
			bool swapped = false;
			for (auto i2 = i + 1; i2 < rows; i2++) {
				if (ret(i2, j) != 0) {
					ret.row(i).swap(ret.row(i2));
					swapped = true;
					break;
				}
			}
			if (!swapped) {
				j++;
				continue;
			}
		}

		for (auto i2 = i + 1; i2 < rows; i2++) {
			if (ret(i2, j) != 0) {
				ret.row(i2) += ret.row(i);
			}
		}

		nonzeroPivots.emplace_back(j);
		i++;
		j++;
	}

	return {ret, nonzeroPivots};
}
