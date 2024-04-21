#pragma once
#include <cstdint>
#include <print>
#include <span>
#include <vector>


struct ElementSaveData {
	uint32_t id;
	uint32_t type;
	int32_t posX;
	int32_t posY;
	uint32_t rotation;
	uint32_t propertySetIndex;
	uint32_t propertyIndex;
	uint32_t propertyCount;
};

struct LineSaveData {
	uint32_t id;
	int32_t startPosX;
	int32_t startPosY;
	int32_t endPosX;
	int32_t endPosY;
};

using PropertySaveData = std::vector<std::byte>;

struct SaveData {
	std::vector<ElementSaveData> elements{};
	std::vector<LineSaveData> lines{};
	std::vector<PropertySaveData> properties{};

	static constexpr uint8_t saveDataVersion = 3;
	static constexpr size_t headerSize = 4 /*Tag*/ +
										 1 /*version*/ +
										 8 /*ElementSaveData sizeof*/ +
										 8 /*LineSaveData sizeof*/ +
										 8 /*Property data total size*/ +
										 8 /*Element count*/ +
										 8 /*Line count*/;

	[[nodiscard]] std::vector<std::byte> serialize() const;

	bool deserialize(const std::span<const std::byte> &bytes);
};