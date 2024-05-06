#include "property/floatProperty.hpp"
#include "container.hpp"
#include "numberBox.hpp"
#include "property/propertyUtils.hpp"
#include "row.hpp"
#include "text.hpp"

using namespace squi;

NumberProperty::Input::operator squi::Child() const {
	VoidObservable selectAllObs{};
	auto storage = std::make_shared<Storage>(value, value);

	return Row{
		.widget{
			.height = Size::Shrink,
			.onInit = [submitObs = closeObs, storage](Widget &w) {
				w.customState.add(submitObs.observe([storage]() {
					storage->value = storage->newValue;
				}));
			},
		},
		.alignment = Row::Alignment::center,
		.children{
			Text{
				.text{name},
			},
			Container{},
			NumberBox{
				.widget{
					.afterInit = [focusObs = focusObs, selectAllObs](Widget &w) {
						w.customState.add(
							focusObs.observe([selectAllObs](bool focus) {
								if (focus) {
									selectAllObs.notify();
								}
							})
						);
					},
				},
				.value = value,
				.onChange = [storage](float newVal) {
					storage->newValue = newVal;
				},
				.controller{
					.focus = focusObs,
					.selectAll = selectAllObs,
				},
			},
		},
	};
}

NumberProperty NumberProperty::fromData(const PropertyData &data) {
	return {
		.name = data.name,
		.suffix = data.suffix,
		.value = 0.f,
	};
}

squi::Child NumberProperty::createInput(const squi::VoidObservable &closeObs, const squi::Observable<bool> &focusObs) const {
	return Input{
		.closeObs = closeObs,
		.focusObs = focusObs,
		.name = name,
		.value = value,
	};
}

constexpr auto ind = Utils::getIndexFromTuple<NumberProperty, PropertiesTypes>();
const uint64_t size = sizeof(uint64_t) + sizeof(ind) + sizeof(NumberProperty::value);
std::vector<std::byte> NumberProperty::serialize() const {
	std::vector<std::byte> ret{};
	ret.reserve(size);
	Utils::addBytes(ret, size);
	Utils::addBytes(ret, ind);
	Utils::addBytes(ret, value);

	return ret;
}

NumberProperty NumberProperty::deserialize(const std::span<const std::byte> &bytes, const PropertyData &data) {
	NumberProperty ret{};

	auto it = bytes.begin();// Skip the first value

	auto dataSize = Utils::bytesTo<uint64_t>(it);
	auto dataIndex = Utils::bytesTo<decltype(ind)>(it);
	if (dataIndex != data.type) {
		throw std::runtime_error("Data type and property type don't match");
	}
	if (dataIndex != ind) {
		throw std::runtime_error("Deserialize called on the wrong property");
	}
	if (dataSize != size) {
		throw std::runtime_error("Data size does not match the size of the property");
	}
	ret.name = data.name;
	ret.suffix = data.suffix;
	ret.value = Utils::bytesTo<decltype(ret.value)>(it);

	return ret;
}

std::string NumberProperty::display() const {
	return std::format("{}{}", value, suffix);
}
