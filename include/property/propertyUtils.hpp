#pragma once

#include "../component.hpp"
#include "../utils.hpp"
#include "floatProperty.hpp"


using PropertiesTypes = std::tuple<NumberProperty>;
using PropertyVariant = Utils::TransferParams<std::variant, PropertiesTypes>::type;

template<class T>
using CleanedType = std::remove_const_t<std::remove_reference_t<T>>;

template<class T>
concept PropertyLike = requires(T a) {
	{ CleanedType<T>::fromData(std::declval<const PropertyData &>()) } -> std::same_as<CleanedType<T>>;
	{ a.createInput(squi::VoidObservable{}, squi::Observable<bool>{}) } -> std::same_as<squi::Child>;
	{ a.serialize() } -> std::same_as<std::vector<std::byte>>;
	{ CleanedType<T>::deserialize(std::declval<std::span<const std::byte>>(), std::declval<const PropertyData &>()) }
	  -> std::same_as<CleanedType<T>>;
};

template<class T>
concept DisplayableProperty = requires(T a) {
	{ a.display() } -> std::same_as<std::string>;
};

template<size_t... I>
consteval bool AllPropertiesValid(std::index_sequence<I...> /*idx*/) {
	return (PropertyLike<std::tuple_element_t<I, PropertiesTypes>> && ...);
}

static_assert(AllPropertiesValid(std::make_index_sequence<std::tuple_size_v<PropertiesTypes>>{}), "All properties must satisfy PropertyLike");

template<PropertyLike T>
constexpr PropertyIndex PropertyIndexOf = Utils::getIndexFromTuple<T, PropertiesTypes>();

static inline PropertyVariant createProperty(const PropertyData &data) {
	auto variant = Utils::FromIndex<PropertiesTypes>(data.type);
	return std::visit(
		[&](auto &&prop) {
			return std::remove_reference_t<decltype(prop)>::type::fromData(data);
		},
		variant
	);
}

static inline std::vector<PropertyVariant> getProperties(size_t index, const Component &component) {
	if (component.properties.empty()) return {};

	std::vector<PropertyVariant> ret{};
	const auto &propertySet = component.properties.at(index);
	ret.reserve(propertySet.properties.size());

	for (const auto &data: propertySet.properties) {
		ret.emplace_back(createProperty(data));
	}

	return ret;
}

static inline float getFloat(const PropertyVariant &property) {
	return std::visit(
		[](auto &&prop) {
			if (std::is_same_v<NumberProperty, std::decay_t<decltype(prop)>>) {
				return prop.value;
			}
			return 0.f;
		},
		property
	);
}