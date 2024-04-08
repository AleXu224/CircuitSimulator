#include "propertyEditor.hpp"
#include "align.hpp"
#include "box.hpp"
#include "button.hpp"
#include "column.hpp"
#include "container.hpp"
#include "expander.hpp"
#include "gestureDetector.hpp"
#include "numberBox.hpp"
#include "observer.hpp"
#include "row.hpp"
#include "scrollableFrame.hpp"
#include "stack.hpp"


using namespace squi;

struct PropertyInput {
	// Args
	squi::Widget::Args widget{};
	std::string_view name;
	float &value;

	struct Storage {
		// Data
	};

	operator squi::Child() const {
		return Row{
			.widget{
				.height = Size::Shrink,
			},
			.alignment = Row::Alignment::center,
			.children{
				Text{
					.text{name},
				},
				Container{},
				NumberBox{
					.value = value,
					.onChange = [&value = value](float newVal) {
						value = newVal;
					},
				},
			},
		};
	}
};

struct Content {
	// Args
	squi::Widget::Args widget{};
	const Element &element;
	std::vector<float> &values;

	struct Storage {
		// Data
	};

	operator squi::Child() const {
		return Box{
			.widget{
				.sizeConstraints{
					.minHeight = 128.f,
					.maxHeight = 400.f,
				},
				.margin = Margin{1.f, 1.f, 0.f, 1.f},
			},
			.color{1.f, 1.f, 1.f, 0.0538f},
			.borderColor{0.f, 0.f, 0.f, 0.1f},
			.borderWidth{0.f, 0.f, 1.f, 0.f},
			.borderRadius{7.f, 7.f, 0.f, 0.f},
			.borderPosition = Box::BorderPosition::outset,
			.child = ScrollableFrame{
				.widget{
					.padding = 24.f,
				},
				.spacing = 8.f,
				.children = std::invoke([&]() {
					Children ret{
						Text{
							.text{std::format("{} {} Properties", element.component.get().name, element.id)},
							.fontSize = 20.f,
						},
					};
					ret.reserve(ret.size() + element.component.get().properties.size());
					values.reserve(element.component.get().properties.size());

					for (const auto &[prop, value]: std::views::zip(element.component.get().properties, element.propertiesValues)) {
						auto &_ = values.emplace_back(value);
						ret.emplace_back(PropertyInput{
							.name = prop.suffix.empty() ? prop.name : std::format("{} ({})", prop.name, prop.suffix),
							.value = _,
						});
					}

					return ret;
				}),
			},
		};
	}
};

struct ButtonBar {
	// Args
	squi::Widget::Args widget{};
	Observable<bool> closeObs;

	struct Storage {
		// Data
	};

	operator squi::Child() const {
		return Row{
			.widget{
				.height = 80.f,
				.padding = 24.f,
			},
			.spacing = 8.f,
			.children{
				Button{
					.widget{
						.width = Size::Expand,
					},
					.onClick = [closeObs = closeObs](auto) {
						closeObs.notify(true);
					},
					.child = Align{
						.child = Text{
							.text{"Ok"},
							.color{0.f, 0.f, 0.f, 1.f},
						},
					},
				},
				Button{
					.widget{
						.width = Size::Expand,
					},
					.text{"Cancel"},
					.style = ButtonStyle::Standard(),
					.onClick = [closeObs = closeObs](auto) {
						closeObs.notify(false);
					},
				},
			},
		};
	}
};

PropertyEditor::operator squi::Child() const {
	auto storage = std::make_shared<Storage>(Storage{.element = element});

	return Stack{
		.widget{
			.onInit = [storage](Widget &w) {
				w.customState.add(storage->closeObs.observe([&w, storage](bool save) {
					if (save) {
						for (auto [val, prop]: std::views::zip(storage->values, storage->element.propertiesValues)) {
							const_cast<float &>(prop) = val;
						}
					}
					w.deleteLater();
				}));
			},
		},
		.children{
			GestureDetector{
				.onClick = [storage](GestureDetector::Event /*event*/) {
					storage->closeObs.notify(false);
				},
				.onUpdate = [storage](auto) {
					if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_ESCAPE)) {
						storage->closeObs.notify(false);
						return;
					}

					if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_ENTER)) {
						storage->closeObs.notify(true);
					}
				},
				.child = Box{
					.color{0.f, 0.f, 0.f, 0.5f},
				},
			},
			Align{
				.child = Box{
					.widget{
						.width = 450.f,
						.height = Size::Shrink,
					},
					.color{0x202020FF},
					.borderColor{0x75757566},
					.borderWidth{1.f},
					.borderRadius{8.f},
					.child = Column{
						.children{
							Content{.element = element, .values = storage->values},
							ButtonBar{.closeObs = storage->closeObs},
						},
					},

				},
			},
		},
	};
}
