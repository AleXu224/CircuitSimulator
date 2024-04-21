#include "saveData.hpp"

#include "utils.hpp"

std::vector<std::byte> SaveData::serialize() const {
	std::vector<std::byte> data{};
	data.reserve(
		headerSize +
		sizeof(ElementSaveData) * elements.size() /*Elements*/ +
		sizeof(LineSaveData) * lines.size() /*Lines*/
	);
	Utils::addBytes(data, std::array<char8_t, 4>{'s', 'q', 'c', 's'});
	Utils::addBytes(data, saveDataVersion);
	Utils::addBytes(data, static_cast<uint64_t>(sizeof(ElementSaveData)));
	Utils::addBytes(data, static_cast<uint64_t>(sizeof(LineSaveData)));
	size_t propertyTotalSize = 0;
	for (const auto &property: properties) {
		propertyTotalSize += property.size();
	}
	Utils::addBytes(data, static_cast<uint64_t>(propertyTotalSize));
	Utils::addBytes(data, static_cast<uint64_t>(elements.size()));
	Utils::addBytes(data, static_cast<uint64_t>(lines.size()));

	for (const auto &element: elements) {
		Utils::addBytes(data, element);
	}

	for (const auto &line: lines) {
		Utils::addBytes(data, line);
	}

	for (const auto &property: properties) {
		for (const auto &byte: property) {
			data.emplace_back(byte);
		}
	}

	return data;
}

bool SaveData::deserialize(const std::span<const std::byte> &bytes) {
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
	const auto savePropertyDataTotalSize = *std::bit_cast<uint64_t *>(&bytes[21]);
	const auto saveElementCount = *std::bit_cast<uint64_t *>(&bytes[29]);
	const auto saveLineCount = *std::bit_cast<uint64_t *>(&bytes[37]);

	const auto totalExpectedSize = headerSize + saveElementSize * saveElementCount + saveLineSize * saveLineCount + savePropertyDataTotalSize;
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
		savePropertyDataTotalSize
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

	for (auto it = propertiesData.begin(); it != propertiesData.end();) {
		const auto &size = Utils::staticBytesTo<size_t>(it);
		if (size == 0) {
			throw std::runtime_error("Property size cannot be 0. Save data might be corrupted");
		}
		properties.emplace_back(it, it + static_cast<int64_t>(size));
		it += static_cast<int64_t>(size);
	}

	return true;
}
