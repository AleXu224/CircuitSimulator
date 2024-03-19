#include "boardStorage.hpp"
#include "element.hpp"
#include "unordered_set"
#include "Eigen/Eigen"
#include <Eigen/src/Core/Matrix.h>


struct GraphDescriptor {
	GraphDescriptor(BoardStorage &);


private:
	void exploreBoard(BoardStorage &);
	void generateGraphMatrix();
	void generateIncidenceMatrix();

	struct ExpandNodeResult {
		std::vector<Element> elements{};
		std::vector<Element> lines{};
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
		std::unordered_set<uint32_t> exploredElements{};
		std::unordered_map<uint32_t, uint32_t> exploredLines{};
		std::unordered_set<Element, ElementHasher> elementsToTraverse{board.elements.front().lock()->customState.get<Element>()};
	};
	std::optional<ExpandNodeResult> expandNode(
		size_t nodeIndex,
		const Coords &coords,
		const GraphElement &graphElement,
		ExplorationState &state
	);

public:
	std::unordered_set<GraphElement, GraphElementHasher> elements{};
	std::unordered_map<uint32_t, Node> nodes{};
	Eigen::MatrixXf graphMatrix{};
	Eigen::MatrixXf incidenceMatrix{};
};