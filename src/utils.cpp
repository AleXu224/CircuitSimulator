#include "utils.hpp"
#include "config.hpp"

Coords Utils::screenToGridRounded(const squi::vec2 &screen, const squi::vec2 &offset, const squi::vec2 &boardPos) {
	const auto virtualPos = screen - boardPos - offset;

	auto gridPos = virtualPos / gridSize;
	return Coords{
		.x = static_cast<int32_t>(std::round(gridPos.x)),
		.y = static_cast<int32_t>(std::round(gridPos.y)),
	};
}

Coords Utils::screenToGridFloored(const squi::vec2 &screen, const squi::vec2 &offset, const squi::vec2 &boardPos) {
	const auto virtualPos = screen - boardPos - offset;

	auto gridPos = virtualPos / gridSize;
	return Coords{
		.x = static_cast<int32_t>(std::floor(gridPos.x)),
		.y = static_cast<int32_t>(std::floor(gridPos.y)),
	};
}

Coords Utils::componentSizeWithRotation(const Component &component, uint32_t rotation) {
	Coords ret(static_cast<int32_t>(component.width), static_cast<int32_t>(component.height));
	if (rotation % 2 == 0) return ret;
	return ret.rotate();
}

Utils::RotatedElementData Utils::rotateElement(uint32_t rotation, const Component &comp) {
	RotatedElementData ret{};
	switch (rotation % 4) {
		case 0: {
			ret.newTopLeft = {comp.uvTopLeft.x, comp.uvTopLeft.y};
			ret.newTopRight = {comp.uvBottomRight.x, comp.uvTopLeft.y};
			ret.newBottomRight = {comp.uvBottomRight.x, comp.uvBottomRight.y};
			ret.newBottomLeft = {comp.uvTopLeft.x, comp.uvBottomRight.y};
			ret.newNodes = comp.nodes;
			break;
		}
		case 1: {
			ret.newTopLeft = {comp.uvTopLeft.x, comp.uvBottomRight.y};
			ret.newTopRight = {comp.uvTopLeft.x, comp.uvTopLeft.y};
			ret.newBottomRight = {comp.uvBottomRight.x, comp.uvTopLeft.y};
			ret.newBottomLeft = {comp.uvBottomRight.x, comp.uvBottomRight.y};
			for (const auto &node: comp.nodes) {
				ret.newNodes.emplace_back(Coords{
					.x = static_cast<int>(comp.height - node.y),
					.y = node.x,
				});
			}
			break;
		}
		case 2: {
			ret.newTopLeft = {comp.uvBottomRight.x, comp.uvBottomRight.y};
			ret.newTopRight = {comp.uvTopLeft.x, comp.uvBottomRight.y};
			ret.newBottomRight = {comp.uvTopLeft.x, comp.uvTopLeft.y};
			ret.newBottomLeft = {comp.uvBottomRight.x, comp.uvTopLeft.y};
			for (const auto &node: comp.nodes) {
				ret.newNodes.emplace_back(Coords{
					.x = static_cast<int>(comp.width - node.x),
					.y = static_cast<int>(comp.height - node.y),
				});
			}
			break;
		}
		case 3: {
			ret.newTopLeft = {comp.uvBottomRight.x, comp.uvTopLeft.y};
			ret.newTopRight = {comp.uvBottomRight.x, comp.uvBottomRight.y};
			ret.newBottomRight = {comp.uvTopLeft.x, comp.uvBottomRight.y};
			ret.newBottomLeft = {comp.uvTopLeft.x, comp.uvTopLeft.y};
			for (const auto &node: comp.nodes) {
				ret.newNodes.emplace_back(Coords{
					.x = node.y,
					.y = static_cast<int>(comp.width - node.x),
				});
			}
			break;
		}
	}

	return ret;
}

std::tuple<Eigen::MatrixXf, std::vector<int64_t>> Utils::calculateNonzeroPivots(Eigen::MatrixXf &input) {
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
