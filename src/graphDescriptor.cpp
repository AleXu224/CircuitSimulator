#include "graphDescriptor.hpp"
#include "boardLine.hpp"
#include "element.hpp"
#include <chrono>
#include <functional>
#include <iterator>
#include <optional>
#include <print>
#include <unordered_map>
#include <unordered_set>
#include <vector>

GraphDescriptor::GraphDescriptor(BoardStorage &board) {
	exploreBoard(board);
}

struct RefWrapperHasher {
	size_t operator()(const std::reference_wrapper<Element> &elem) const {
		return std::hash<uint32_t>{}(elem.get().id);
	}
};

struct ElementHasher {
	size_t operator()(const Element &elem) const {
		return std::hash<uint32_t>{}(elem.id);
	}
};

struct Node {
	std::unordered_set<Element, ElementHasher> elements{};
	std::unordered_set<Element, ElementHasher> lines{};
};

struct ExpandNodeResult {
	std::vector<Element> elements{};
	std::vector<Element> lines{};
};

std::optional<ExpandNodeResult> expandNode(
	const BoardStorage &board,
	const Coords &coords,
    uint32_t initialId,
	const std::unordered_set<uint32_t> &exploredElements,
	const std::unordered_set<uint32_t> &exploredLines
) {
	struct Args {
		ExpandNodeResult ret{};
		bool alreadyExplored = false;
		std::unordered_set<uint32_t> locallyExploredLines{};
		std::unordered_set<uint32_t> locallyExploredElements{};
        uint32_t initialId;
		const BoardStorage &board;
		const std::unordered_set<uint32_t> &exploredElements;
		const std::unordered_set<uint32_t> &exploredLines;
	} args{
        .initialId = initialId,
		.board = board,
        .exploredElements = exploredElements,
		.exploredLines = exploredLines,
	};

	auto crawler = [](this auto &&self, Args &args, const Coords &coords) -> void {
		auto elems = args.board.nodes.at(coords).second;
		for (auto &elem: elems) {
			if (elem.expired()) continue;
			auto widget = elem.lock();
			auto &elemData = widget->customState.get<Element>();
            // Early returns:
            // Both in the case of elements and lines if one is found that has already been explored before
            // then it is not worth exploring this any further since it is guaranteed that another
            // node exploration has already explored this or will do so in the future
			switch (elemData.type) {
				case ElementType::Other: {
                    args.ret.elements.push_back(elemData);
                    if (elemData.id == args.initialId) continue;
                    
					if (args.exploredElements.contains(elemData.id)) {
                        args.alreadyExplored = true;
                        return;
                    }
                    if (args.locallyExploredElements.contains(elemData.id)) {
                        continue;
                    }
                    args.locallyExploredElements.emplace(elemData.id);
					continue;
				}
				case ElementType::Line: {
					if (args.exploredLines.contains(elemData.id)) {
						args.alreadyExplored = true;
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
	uint32_t nodeIdCounter = 0;
	std::unordered_set<uint32_t> exploredElements{};
	std::unordered_set<uint32_t> exploredLines{};
	std::unordered_map<uint32_t, Node> nodes{};
	std::unordered_set<Element, ElementHasher> elementsToTraverse{board.elements.front().lock()->customState.get<Element>()};

	while (!elementsToTraverse.empty()) {
		// Copy the elements to traverse since the container will need to be modified while iterating through it
		// which could cause the iterators to be invalidated
		auto elemsToTraverse = elementsToTraverse;

		for (const auto &elem: elemsToTraverse) {
			auto removeFromQueue = [&]() {
				if (auto el = elementsToTraverse.find(elem); el != elementsToTraverse.end()) {
					elementsToTraverse.erase(el);
				}
			};
			if (exploredElements.contains(elem.id)) {
				removeFromQueue();
				continue;
			}
			elementsToTraverse.erase(elementsToTraverse.find(elem));
			exploredElements.emplace(elem.id);

			for (const auto &node: elem.nodes) {
				auto res = expandNode(board, node + elem.pos, elem.id, exploredElements, exploredLines);
				// Check if the expanded node has been explored before
				if (!res.has_value()) continue;
				auto &resVal = res.value();
				for (auto &line: resVal.lines) {
					exploredLines.insert(line.id);
				}
				for (auto &element: resVal.elements) {
					if (exploredElements.contains(element.id)) continue;
					elementsToTraverse.insert(element);
				}
				nodes.emplace(
					nodeIdCounter++,
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
	std::println("Lines explored: {}", exploredLines.size());
	std::println("Elements explored: {}", exploredElements.size());
	for (auto &[nodeId, node]: nodes) {
		std::println("Node: {}, with {} lines and {} elements", nodeId, node.lines.size(), node.elements.size());
	}
}
