#pragma once

#include "coords.hpp"
#include "samplerUniform.hpp"
#include <cstdint>
#include <optional>
#include <vector>

struct Component{
    uint32_t id = 0;
    std::string name;
    uint32_t width;
    uint32_t height;
    std::vector<Coords> nodes{};
    std::string texturePath;
    std::string textureThumbPath;
    std::optional<Engine::SamplerUniform> texture{};
    std::optional<Engine::SamplerUniform> textureThumb{};
	squi::vec2 uvTopLeft{0, 0};
	squi::vec2 uvBottomRight{1, 1};
    bool hidden = false;
};