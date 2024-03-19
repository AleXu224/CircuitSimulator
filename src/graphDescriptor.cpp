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
		std::unordered_set<uint32_t> locallyExploredElements{};
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
		auto elems = args.state.board.nodes.at(coords).second;
		for (auto &elem: elems) {
			if (elem.widget.expired()) continue;
			auto widget = elem.widget.lock();
			auto &elemData = elem.element.get();
			// Early returns:
			// Both in the case of elements and lines if one is found that has already been explored before
			// then it is not worth exploring this any further since it is guaranteed that another
			// node exploration has already explored this or will do so in the future
			switch (elemData.type) {
				case ElementType::Other: {
					args.ret.elements.push_back(elemData);
					if (elemData.id == args.graphElement.element.id) continue;

					if (args.state.exploredElements.contains(elemData.id)) {
						args.alreadyExplored = true;
						return;
					}
					// Set the element connection to the current node
					for (const auto &connection: args.state.board.nodes.at(coords).second) {
						if (connection.element.get().id == elemData.id) {
							auto [it, _] = args.self.elements.emplace(GraphElement{.element{elemData}});
							it->nodes.at(connection.nodeIndex) = args.state.nodeIdCounter;
							break;
						}
					}
					args.locallyExploredElements.emplace(elemData.id);

					continue;
				}
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
					auto lineStorage = widget->customState.get<BoardLine::Storage>();

					for (const auto &lineNodeCoords: {lineStorage.startPos, lineStorage.endPos}) {
						if (*lineNodeCoords == coords) continue;

						self(args, *lineNodeCoords);
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
				auto res = expandNode(index, node + elem.pos, *graphElement, state);
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
	const size_t vectorSizeReq = 1 /*Element id*/ + 1 /*Component id*/ + maxNodeConnections;
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
		vecs.emplace_back(std::move(ret));
	}

	std::sort(vecs.begin(), vecs.end(), [](const auto &a, const auto &b) {
		return a[1] < b[1];
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
