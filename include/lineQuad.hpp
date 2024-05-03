#pragma once
#include "pipeline.hpp"
#include "vulkanIncludes.hpp"
#include <array>
#include <cstddef>


namespace Engine {
	struct LineQuad {
		struct Vertex {
			alignas(16) glm::vec4 color;
			alignas(8) glm::vec2 size;
			alignas(8) glm::vec2 pos;
			alignas(8) glm::vec2 uv;
			alignas(8) glm::vec2 lineStart;
			alignas(8) glm::vec2 lineEnd;
			alignas(4) glm::float32 lineWidth;

			static std::array<vk::VertexInputAttributeDescription, 7> describe() {
				using Desc = vk::VertexInputAttributeDescription;
				return {
					Desc{
						.location = 0,
						.binding = 0,
						.format = vk::Format::eR32G32B32A32Sfloat,
						.offset = offsetof(Vertex, color),
					},
					Desc{
						.location = 1,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, size),
					},
					Desc{
						.location = 2,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, pos),
					},
					Desc{
						.location = 3,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, uv),
					},
					Desc{
						.location = 4,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, lineStart),
					},
					Desc{
						.location = 5,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, lineEnd),
					},
					Desc{
						.location = 6,
						.binding = 0,
						.format = vk::Format::eR32Sfloat,
						.offset = offsetof(Vertex, lineWidth),
					},
				};
			}
		};


		struct Args {
			glm::vec4 color{1.f};
			glm::vec2 lineStart{0, 0};
			glm::vec2 lineEnd{0, 0};
			glm::float32 lineWidth{1.f};
		};

	private:
		std::array<Vertex, 4> vertices{};
		std::array<uint16_t, 6> indices{};

	public:
		void setPos(const glm::vec2 &startPos, const glm::vec2 &endPos) {
			const auto [pos, size] = getSizeAndPos(startPos, endPos);
			vertices[0].size = size;
			vertices[0].pos = pos;
			vertices[0].lineStart = startPos;
			vertices[0].lineEnd = endPos;

			vertices[1].size = size;
			vertices[1].pos = pos;
			vertices[1].lineStart = startPos;
			vertices[1].lineEnd = endPos;

			vertices[2].size = size;
			vertices[2].pos = pos;
			vertices[2].lineStart = startPos;
			vertices[2].lineEnd = endPos;

			vertices[3].size = size;
			vertices[3].pos = pos;
			vertices[3].lineStart = startPos;
			vertices[3].lineEnd = endPos;
		}

		[[nodiscard]] static std::tuple<glm::vec2, glm::vec2> getSizeAndPos(const glm::vec2 &startPos, const glm::vec2 &endPos) {
			glm::vec2 minPos{
				std::min(startPos.x, endPos.x),
				std::min(startPos.y, endPos.y),
			};
			glm::vec2 maxPos{
				std::max(startPos.x, endPos.x),
				std::max(startPos.y, endPos.y),
			};

			// Add one pixel of padding around line for the anti aliasing
			return {
				minPos - 1.f,
				(maxPos - minPos) + 2.f,
			};
		}

		LineQuad(const Args &args) {
			const auto [pos, size] = getSizeAndPos(args.lineStart, args.lineEnd);

			vertices[0] = {
				.color = args.color,
				.size = size,
				.pos = pos,
				.uv = {0, 0},
				.lineStart = args.lineStart,
				.lineEnd = args.lineEnd,
				.lineWidth = args.lineWidth,
			};
			vertices[1] = {
				.color = args.color,
				.size = size,
				.pos = pos,
				.uv = {1, 0},
				.lineStart = args.lineStart,
				.lineEnd = args.lineEnd,
				.lineWidth = args.lineWidth,
			};
			vertices[2] = {
				.color = args.color,
				.size = size,
				.pos = pos,
				.uv = {1, 1},
				.lineStart = args.lineStart,
				.lineEnd = args.lineEnd,
				.lineWidth = args.lineWidth,
			};
			vertices[3] = {
				.color = args.color,
				.size = size,
				.pos = pos,
				.uv = {0, 1},
				.lineStart = args.lineStart,
				.lineEnd = args.lineEnd,
				.lineWidth = args.lineWidth,
			};
		}

		Pipeline<Vertex>::Data getData(size_t vi, size_t ii) {
			(void) vi;
			(void) ii;

			indices[0] = 0 + vi;
			indices[1] = 1 + vi;
			indices[2] = 2 + vi;
			indices[3] = 0 + vi;
			indices[4] = 2 + vi;
			indices[5] = 3 + vi;

			return {
				.vertexes = vertices,
				.indexes = indices,
			};
		}
	};
}// namespace Engine