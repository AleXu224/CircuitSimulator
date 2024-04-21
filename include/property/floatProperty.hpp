#pragma once
#include "../elementProperty.hpp"
#include "observer.hpp"
#include "widget.hpp"
#include <span>

struct NumberProperty {
	std::string name{};
	std::string suffix{};
	mutable float value;

	[[nodiscard]] static NumberProperty fromData(const PropertyData &data);
	[[nodiscard]] squi::Child createInput(const squi::VoidObservable &closeObs, const squi::Observable<bool> &focusObs) const;

	[[nodiscard]] std::vector<std::byte> serialize() const;
	[[nodiscard]] static NumberProperty deserialize(const std::span<const std::byte> &bytes, const PropertyData &data);

	[[nodiscard]] std::string display() const;

	struct Input {
		// Args
		// squi::Widget::Args widget{};
		squi::VoidObservable closeObs{};
		squi::Observable<bool> focusObs{};
		std::string_view name;
		float &value;

		struct Storage {
			// Data
			float newValue;
			float &value;
		};

		operator squi::Child() const;
	};
};
