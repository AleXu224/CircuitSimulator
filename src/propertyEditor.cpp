#include "propertyEditor.hpp"
#include "align.hpp"
#include "box.hpp"
#include "button.hpp"
#include "column.hpp"
#include "container.hpp"
#include "expander.hpp"
#include "gestureDetector.hpp"
#include "observer.hpp"
#include "row.hpp"
#include "scrollableFrame.hpp"
#include "stack.hpp"
#include "textBox.hpp"


using namespace squi;

struct PropertyInput {
	// Args
	squi::Widget::Args widget{};
    std::string_view name;
    float value;

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
                TextBox{
                    .text = std::to_string(value),
                },
			},
		};
	}
};

struct Content {
	// Args
	squi::Widget::Args widget{};
	const Element &element;

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
							.text{std::format("{} Properties", element.component.get().name)},
							.fontSize = 20.f,
						},
					};
					ret.reserve(ret.size() + element.component.get().properties.size());

					for (const auto &[prop, value]: std::views::zip(element.component.get().properties, element.propertiesValues)) {
						ret.emplace_back(PropertyInput{
                            .name = prop.name,
                            .value = value,
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
	VoidObservable closeObs;

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
						closeObs.notify();
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
						closeObs.notify();
					},
				},
			},
		};
	}
};

PropertyEditor::operator squi::Child() const {
	VoidObservable closeObs{};

	return Stack{
		.widget{
			.onInit = [closeObs = closeObs](Widget &w) {
				w.customState.add(closeObs.observe([&w]() {
					w.deleteLater();
				}));
			},
		},
		.children{
			GestureDetector{
				.onClick = [closeObs = closeObs](GestureDetector::Event /*event*/) {
					closeObs.notify();
				},
                .onUpdate = [closeObs](auto){
                    if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_ESCAPE)) {
						closeObs.notify();
                        return;
					}

                    if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_ENTER)) {
                        closeObs.notify();
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
							Content{.element = element},
							ButtonBar{.closeObs = closeObs},
						},
					},

				},
			},
		},
	};
}
