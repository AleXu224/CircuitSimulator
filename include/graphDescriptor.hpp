#pragma once

#include "boardStorage.hpp"
#include "element.hpp"
#include "unordered_set"


struct GraphDescriptor {
	GraphDescriptor(BoardStorage &);


private:
	void exploreBoard(BoardStorage &);

	struct ExpandNodeResult {
		std::vector<std::reference_wrapper<const Element>> elements{};
		std::vector<std::reference_wrapper<const Element>> lines{};
	};
	struct GraphElement {
		Element element;
		mutable std::vector<uint32_t> nodes = std::vector<uint32_t>(element.component.get().nodes.size());

		bool operator==(const GraphElement &other) const {
			return element.id == other.element.id;
		}
	};
	struct GraphElementHasher {
		size_t operator()(const GraphElement &elem) const {
			return std::hash<uint32_t>{}(elem.element.id);
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
	struct ExplorationState {
		uint32_t nodeIdCounter = 0;
		const BoardStorage &board;
		std::unordered_set<Coords> exploredConnections{};
	};

	struct ExpandArgs {
		const size_t nodeIndex;
		ExpandNodeResult ret{};
		std::unordered_set<uint32_t> locallyExploredLines{};
		ExplorationState &state;
		GraphDescriptor &self;
	};

	GraphDescriptor::ExpandNodeResult
	expandNode(
		size_t nodeIndex,
		const Coords &coords,
		ExplorationState &state
	);

public:
	std::unordered_map<ElementId, GraphElement> elements{};
	std::unordered_map<uint32_t, Node> nodes{};
};