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
}

std::vector<Connection> getAdditionalConnections(const BoardStorage &board, const Coords &coords, const std::vector<Connection> &connections) {
	std::vector<Connection> additionalConnections{};
	if (!board.lineTiles.contains(coords)) return additionalConnections;
	const auto &lines = board.lineTiles.at(coords);
	for (const auto &lineId: lines) {
		bool hasLine = false;
		for (const auto &connection: connections) {
			if (connection.elementId == lineId) {
				hasLine = true;
				break;
			}
		}
		if (!hasLine) {
			auto lineIt = board.getLine(lineId);
			if (!lineIt.has_value()) continue;
			for (const auto &[nodeIndex, _]: lineIt->get().element.nodes | std::views::enumerate) {
				additionalConnections.emplace_back(Connection{
					.nodeIndex = static_cast<size_t>(nodeIndex),
					.elementId = lineId,
				});
			}
		}
	}

	return additionalConnections;
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
		if (args.state.exploredConnections.contains(coords)) return;
		args.state.exploredConnections.emplace(coords);

		const auto connections = {
			args.state.board.connections.at(coords).connections,
			getAdditionalConnections(
				args.state.board,
				coords,
				args.state.board.connections.at(coords).connections
			),
		};

		for (const auto &connection: std::views::join(connections)) {
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
					// if (args.state.exploredElements.contains(elemData.id)) {
					// 	continue;
					// }
					args.ret.elements.push_back(elemData);
					// if (elemData.id == args.graphElement.element.id) continue;

					// Set the element connection to the current node
					auto [it, _] = args.self.elements.emplace(GraphElement{.element{elemData}});
					it->nodes.at(connection.nodeIndex) = args.state.nodeIdCounter;

					continue;
				}
				case ElementType::Ground:
				case ElementType::Line: {
					if (args.locallyExploredLines.contains(elemData.id)) {
						continue;
					}
					args.locallyExploredLines.emplace(elemData.id);
					args.ret.lines.push_back(elemData);

					std::vector<Coords> connectionCoords{};
					switch (elemData.component.get().type) {
						case ElementType::Line: {
							const auto dist = (elemData.nodes.at(0) - elemData.nodes.at(1)).abs();
							for (auto x: std::views::iota(elemData.pos.x) | std::views::take(dist.x + 1)) {
								for (auto y: std::views::iota(elemData.pos.y) | std::views::take(dist.y + 1)) {
									if (args.state.board.connections.contains(Coords{x, y}) && args.state.board.connections.at(Coords{x, y}).connections.size() > 0)
										connectionCoords.emplace_back(Coords{x, y});
								}
							}
						}
						case ElementType::Ground: {
							std::vector<Coords> connectionCoords{};
							for (const auto &elem: args.state.board.elements) {
								if (elem.element.component.get().type != ElementType::Ground) continue;
								for (const auto &node: elem.element.nodes) {
									connectionCoords.emplace_back(node + elem.element.pos);
								}
							}
						}
						default:
							std::unreachable();
					}
					for (const auto &connectionCoord: connectionCoords) {
						if (connectionCoord == coords) continue;

						self(args, connectionCoord);
					}
				}
			}
		}
	};
	crawler(args, coords);

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

				// elements.find(GraphElement{.element{elem}})->nodes.at(index) = state.nodeIdCounter;
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

	{
		// Erase all elements that have all their connections at the same node
		std::vector<decltype(elements)::iterator> elemsToErase{};
		for (auto it = elements.begin(); it != elements.end(); it++) {
			if (std::ranges::adjacent_find(it->nodes, std::ranges::not_equal_to()) == it->nodes.end()) {
				elemsToErase.emplace_back(it);
			}
		}
		for (auto &it: elemsToErase) {
			elements.erase(it);
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
