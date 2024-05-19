#include "graphDescriptor.hpp"
#include "boardLine.hpp"
#include "element.hpp"
#include <algorithm>
#include <chrono>
#include <functional>
#include <optional>
#include <print>
#include <queue>
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

GraphDescriptor::ExpandNodeResult GraphDescriptor::expandNode(
	size_t nodeIndex,
	const Coords &coords,
	ExplorationState &state
) {
	ExpandArgs args{
		.nodeIndex = nodeIndex,
		.state = state,
		.self = *this,
	};

	auto crawler = [](this auto &&self, ExpandArgs &args, const Coords &coords) -> void {
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

			const auto elemType = elemData.component.get().type;
			switch (elemType) {
				case ElementType::Other: {
					args.ret.elements.emplace_back(elemData);

					// Set the element connection to the current node
					auto [it, _] = args.self.elements.emplace(elemData.id, GraphElement{.element{elemData}});
					it->second.nodes.at(connection.nodeIndex) = args.state.nodeIdCounter;

					continue;
				}
				case ElementType::Ground:
				case ElementType::Line: {
					if (args.locallyExploredLines.contains(elemData.id)) {
						continue;
					}
					args.locallyExploredLines.emplace(elemData.id);
					args.ret.lines.emplace_back(elemData);

					std::vector<Coords> connectionCoords{};
					if (elemType == ElementType::Line) {
						const auto dist = (elemData.nodes.at(0) - elemData.nodes.at(1)).abs();
						// Go along all the points on the line
						// If any connections are found then add them to the list
						// This will catch any straggler connections that are attached to the middle of the line
						for (auto x: std::views::iota(elemData.pos.x) | std::views::take(dist.x + 1)) {
							for (auto y: std::views::iota(elemData.pos.y) | std::views::take(dist.y + 1)) {
								if (args.state.board.connections.contains(Coords{x, y}) && args.state.board.connections.at(Coords{x, y}).connections.size() > 0)
									connectionCoords.emplace_back(Coords{x, y});
							}
						}
					} else if (elemType == ElementType::Ground) {
						for (const auto &elem: args.state.board.elements) {
							if (elem.element.component.get().type != ElementType::Ground) continue;
							for (const auto &node: elem.element.nodes) {
								connectionCoords.emplace_back(node + elem.element.pos);
							}
						}
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

	// Will add the first ground element to the traverseQueue
	// this is to ensure that the ground node always gets index 0
	auto groundIt = std::find_if(
		board.elements.begin(),
		board.elements.end(),
		[](const ElementData &elem) {
			return elem.element.component.get().type == ElementType::Ground;
		}
	);

	std::queue<std::reference_wrapper<const Element>> traverseQueue{};
	traverseQueue.emplace(
		groundIt == board.elements.end()
			? board.elements.front().element
			: groundIt->element
	);

	while (!traverseQueue.empty()) {
		const auto &elem = traverseQueue.front().get();
		traverseQueue.pop();

		for (const auto &[index, node]: elem.nodes | std::views::enumerate) {
			const auto coords = node + elem.pos;
			if (state.exploredConnections.contains(coords)) continue;

			auto res = expandNode(index, coords, state);

			for (auto &element: res.elements) {
				traverseQueue.emplace(element);
			}
			nodes.emplace(
				state.nodeIdCounter++,
				Node{
					.elements{res.elements.begin(), res.elements.end()},
					.lines{res.lines.begin(), res.lines.end()},
				}
			);
		}
	}


	// Find the elements that have all their connections at the same node
	std::vector<decltype(elements)::const_iterator> elemsToErase{};
	for (auto it = elements.begin(); it != elements.end(); it++) {
		if (std::ranges::adjacent_find(it->second.nodes, std::ranges::not_equal_to()) == it->second.nodes.end()) {
			elemsToErase.emplace_back(it);
		}
	}

	auto endTime = std::chrono::steady_clock::now();
	std::println("----------------");
	
	for (auto &it: elemsToErase) {
		std::println("Note: {}{} ignored due to having all connections on the same node", it->second.element.component.get().prefix, it->second.element.id);
		elements.erase(it);
	}

	std::println("Time taken: {}", endTime - startTime);
	for (auto &[nodeId, node]: nodes) {
		std::println("Node: {}, with {} lines and {} elements", nodeId, node.lines.size(), node.elements.size());
	}
	for (const auto &[id, element]: elements) {
		std::stringstream vectorNodeIds{};
		for (const auto &node: element.nodes) {
			vectorNodeIds << "N" << node << " ";
		}
		std::println("{} #{} {}", element.element.component.get().name, element.element.id, vectorNodeIds.str());
	}
}
