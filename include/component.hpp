#pragma once

#include "coords.hpp"
#include "samplerUniform.hpp"
#include <cstdint>
#include <optional>
#include <vector>

struct Component{
    std::string name;
    uint32_t width;
    uint32_t height;
    std::vector<Coords> nodes{};
    std::string texturePath;
    std::optional<Engine::SamplerUniform> texture{};
	squi::vec2 uvTopLeft{0, 0};
	squi::vec2 uvBottomRight{1, 1};
    bool hidden = false;
};