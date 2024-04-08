#pragma once
#include "array"
#include "bit"
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

using PropertySaveData = float;
// struct PropertySaveData {
// 	float value;
// };

template<class T>
void addBytes(std::vector<std::byte> &to, const T &from) {
	const auto &bytes = *std::bit_cast<std::array<std::byte, sizeof(T)> *>(&from);
	for (const auto &byte: bytes) {
		to.emplace_back(byte);
	}
}

struct SaveData {
	std::vector<ElementSaveData> elements{};
	std::vector<LineSaveData> lines{};
	std::vector<PropertySaveData> properties{};

	static constexpr uint8_t saveDataVersion = 2;
	static constexpr size_t headerSize = 4 /*Tag*/ +
										 1 /*version*/ +
										 8 /*ElementSaveData sizeof*/ +
										 8 /*LineSaveData sizeof*/ +
										 8 /*Property sizeof*/ +
										 8 /*Element count*/ +
										 8 /*Line count*/ +
										 8 /*Property count*/;

	[[nodiscard]] std::vector<std::byte>
	serialize() const {
		std::vector<std::byte> data{};
		data.reserve(
			headerSize +
			sizeof(ElementSaveData) * elements.size() /*Elements*/ +
			sizeof(LineSaveData) * lines.size() /*Lines*/
		);
		addBytes(data, std::array<char8_t, 4>{'s', 'q', 'c', 's'});
		addBytes(data, saveDataVersion);
		addBytes(data, static_cast<uint64_t>(sizeof(ElementSaveData)));
		addBytes(data, static_cast<uint64_t>(sizeof(LineSaveData)));
		addBytes(data, static_cast<uint64_t>(sizeof(PropertySaveData)));
		addBytes(data, static_cast<uint64_t>(elements.size()));
		addBytes(data, static_cast<uint64_t>(lines.size()));
		addBytes(data, static_cast<uint64_t>(properties.size()));

		for (const auto &element: elements) {
			addBytes(data, element);
		}

		for (const auto &line: lines) {
			addBytes(data, line);
		}

		for (const auto &property: properties) {
			addBytes(data, property);
		}

		return data;
	}

	bool deserialize(const std::span<const std::byte> &bytes) {
		if (bytes.size() < headerSize) {
			std::println("Invalid data to deserialize");
			return false;
		}

		const std::string_view str(std::bit_cast<const char *>(&bytes.front()), 4);
		if (str != "sqcs") {
			std::println("Invalid file format or file corrupted");
			return false;
		}
		const auto version = std::bit_cast<uint8_t>(bytes[4]);
		if (version > saveDataVersion) {
			std::println("Save file version ({}) is incompatible with the version of the program ({})", version, saveDataVersion);
			return false;
		}

		const auto saveElementSize = *std::bit_cast<uint64_t *>(&bytes[5]);
		const auto saveLineSize = *std::bit_cast<uint64_t *>(&bytes[13]);
		const auto savePropertySize = *std::bit_cast<uint64_t *>(&bytes[21]);
		const auto saveElementCount = *std::bit_cast<uint64_t *>(&bytes[29]);
		const auto saveLineCount = *std::bit_cast<uint64_t *>(&bytes[37]);
		const auto savePropertyCount = *std::bit_cast<uint64_t *>(&bytes[45]);

		const auto totalExpectedSize = headerSize + saveElementSize * saveElementCount + saveLineSize * saveLineCount + savePropertySize * savePropertyCount;
		if (totalExpectedSize != bytes.size()) {
			std::println("Unexpected save file size, file might be corrupted");
			return false;
		}

		elements.reserve(saveElementCount);
		lines.reserve(saveLineCount);

		int64_t offset = headerSize;

		auto elementsData = std::span<const std::byte>(
			bytes.begin() + offset,
			saveElementSize * saveElementCount
		);

		offset += static_cast<int64_t>(elementsData.size());
		
		auto linesData = std::span<const std::byte>(
			bytes.begin() + offset,
			saveLineSize * saveLineCount
		);

		offset += static_cast<int64_t>(linesData.size());

		auto propertiesData = std::span<const std::byte>(
			bytes.begin() + offset,
			savePropertySize * savePropertyCount
		);

		for (auto it = elementsData.begin(); it != elementsData.end(); it += static_cast<int64_t>(saveElementSize)) {
			if ((it + (static_cast<int64_t>(saveElementSize) - 1)) == elementsData.end()) {
				throw std::runtime_error("Bad maths, memcpy would read memory outside the byte array");
			}
			auto &elem = elements.emplace_back();
			memcpy(&elem, &*it, saveElementSize);
		}

		for (auto it = linesData.begin(); it != linesData.end(); it += static_cast<int64_t>(saveLineSize)) {
			if ((it + (static_cast<int64_t>(saveLineSize) - 1)) == linesData.end()) {
				throw std::runtime_error("Bad maths, memcpy would read memory outside the byte array");
			}
			auto &elem = lines.emplace_back();
			memcpy(&elem, &*it, saveLineSize);
		}

		for (auto it = propertiesData.begin(); it != propertiesData.end(); it += static_cast<int64_t>(savePropertySize)) {
			if ((it + (static_cast<int64_t>(savePropertySize) - 1)) == propertiesData.end()) {
				throw std::runtime_error("Bad maths, memcpy would read memory outside the byte array");
			}
			auto &prop = properties.emplace_back();
			memcpy(&prop, &*it, savePropertySize);
		}

		return true;
	}
};