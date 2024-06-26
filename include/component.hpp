#pragma once

#include "coords.hpp"
#include "elementProperty.hpp"
#include "samplerUniform.hpp"
#include <cstdint>
#include <optional>
#include <vector>


enum class ElementType {
	Line,
	Ground,
	Other,
};

struct Component {
	uint32_t id = 0;
	std::string name;
	std::string prefix{};
	uint32_t width;
	uint32_t height;
	ElementType type = ElementType::Other;
	std::vector<Coords> nodes{};
	std::string texturePath;
	std::string textureThumbPath;
	std::optional<Engine::SamplerUniform> texture{};
	std::optional<Engine::SamplerUniform> textureThumb{};
	squi::vec2 uvTopLeft{0, 0};
	squi::vec2 uvBottomRight{1, 1};
	bool hidden = false;
	std::vector<PropertySet> properties{};
};