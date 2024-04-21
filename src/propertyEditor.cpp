#include "propertyEditor.hpp"
#include "align.hpp"
#include "box.hpp"
#include "button.hpp"
#include "column.hpp"
#include "container.hpp"
#include "contextMenu.hpp"
#include "expander.hpp"
#include "fontIcon.hpp"
#include "gestureDetector.hpp"
#include "observer.hpp"
#include "row.hpp"
#include "scrollableFrame.hpp"
#include "stack.hpp"
#include "window.hpp"


using namespace squi;

struct PropertySetChooser {
	// Args
	squi::Widget::Args widget{};
	std::shared_ptr<PropertyEditor::Storage> storage;

	operator squi::Child() const {
		return Button{
			.style = ButtonStyle::Standard(),
			.onClick = [storage = storage](GestureDetector::Event event) {
				Window::of(&event.widget).addOverlay(ContextMenu{
					.position = event.widget.getPos().withYOffset(event.widget.getLayoutSize().y),
					.items = [&]() {
						std::vector<ContextMenu::Item> ret{};
						for (const auto &[index, propertySet]: storage->element.component.get().properties | std::views::enumerate) {
							ret.emplace_back(
								ContextMenu::Item{
									.text = propertySet.name,
									.content = [index = index, storage = storage]() {
										storage->propIndex = index;
										storage->propIndexChanged.notify();
									},
								}
							);
						}
						return ret;
					}(),
				});
			},
			.child = Row{
				.alignment = squi::Row::Alignment::center,
				.spacing = 4.f,
				.children{
					Container{
						.child = Align{
							.xAlign = 0.f,
							.child = Text{
								.text = storage->element.component.get().properties.at(storage->propIndex).name,
							},
						},
					},
					FontIcon{
						.icon = 0xE972,
						.size = 12.f,
					},
				},
			},
		};
	}
};

struct Content {
	// Args
	squi::Widget::Args widget{};
	std::shared_ptr<PropertyEditor::Storage> storage;
	VoidObservable saveObs;

	static inline Children getChildren(const std::shared_ptr<PropertyEditor::Storage> &storage, const VoidObservable &saveObs) {
		const auto &element = storage->element;
		Children ret{
			Text{
				.text{std::format("{} {} Properties", element.component.get().name, element.id)},
				.fontSize = 20.f,
			},
			storage->element.component.get().properties.size() > 1
				? PropertySetChooser{
					  .storage = storage,
				  }
				: Child{},
		};
		ret.reserve(ret.size() + element.component.get().properties.size());

		for (const auto &[prop, focusObservable]: std::views::zip(storage->props, storage->focusObservables)) {
			std::visit(
				[&](PropertyLike auto &&item) {
					ret.emplace_back(item.createInput(saveObs, focusObservable));
				},
				prop
			);
		}

		return ret;
	}

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
					.height = Size::Shrink,
					.padding = 24.f,
				},
				.scrollableWidget{
					.onInit = [storage = storage, saveObs = saveObs](Widget &w) {
						w.customState.add(storage->propIndexChanged.observe([&w, storage, saveObs]() {
							storage->props = getProperties(storage->propIndex, storage->element.component.get());
							storage->focusObservables.resize(storage->props.size(), {});
							w.setChildren(getChildren(storage, saveObs));
							storage->focusIndex = 0;
							if (!storage->focusObservables.empty()) storage->focusObservables.front().notify(true);
						}));
					},
				},
				.spacing = 8.f,
				.children = getChildren(storage, saveObs),
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
	auto storage = std::make_shared<Storage>(Storage{
		.propIndex = element.propertySetIndex,
		.props = element.propertiesValues,
		.focusObservables = std::vector<Observable<bool>>(element.propertiesValues.size()),
		.element = element,
	});

	return Stack{
		.widget{
			.onInit = [storage](Widget &w) {
				w.customState.add(storage->closeObs.observe([&w, storage](bool saved) {
					if (saved) {
						storage->element.propertySetIndex = storage->propIndex;
						storage->saveObs.notify();
						storage->element.propertiesValues = storage->props;
					}
					w.deleteLater();
				}));
			},
			.afterInit = [storage](Widget &) {
				if (storage->focusObservables.empty()) return;
				storage->focusObservables.front().notify(true);
			},
			.onUpdate = [storage](Widget &) {
				auto newIndex = storage->focusIndex;
				if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_TAB)) {
					newIndex = (storage->focusIndex + 1) % storage->focusObservables.size();
				} else if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_TAB, GLFW_MOD_SHIFT)) {
					newIndex = (storage->focusIndex + storage->focusObservables.size() - 1) % storage->focusObservables.size();
				}
				if (storage->focusIndex == newIndex) return;
				storage->focusObservables.at(storage->focusIndex).notify(false);
				storage->focusObservables.at(newIndex).notify(true);
				storage->focusIndex = newIndex;
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
							Content{.storage = storage, .saveObs = storage->saveObs},
							ButtonBar{.closeObs = storage->closeObs},
						},
					},

				},
			},
		},
	};
}
