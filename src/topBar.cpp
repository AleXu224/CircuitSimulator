#include "topBar.hpp"
#include "align.hpp"
#include "box.hpp"
#include "button.hpp"
#include "column.hpp"
#include "components/componentStore.hpp"
#include "contextMenu.hpp"
#include "fontIcon.hpp"
#include "gestureDetector.hpp"
#include "msdfImage.hpp"
#include "row.hpp"
#include "widget.hpp"
#include "window.hpp"
#include <functional>
#include <print>


using namespace squi;

struct TopBarButton {
	// Args
	squi::Widget::Args widget{};
	std::function<void(GestureDetector::Event)> onClick{};
	Child child{};

	operator squi::Child() const {
		return Button{
			.widget{
				.width = 48.f,
				.height = 48.f,
				.margin = 0.f,
				.padding = 0.f,
			},
			.style = ButtonStyle::Subtle(),
			.onClick{onClick},
			.child = Align{.child{child}},
		};
	}
};

struct IconTextCombo {
	// Args
	squi::Widget::Args widget{};
	char32_t icon{};
	std::string_view text{};

	operator squi::Child() const {
		return Column{
			.widget{
				.height = Size::Shrink,
			},
			.alignment = squi::Column::Alignment::center,
			.spacing = 4.f,
			.children{
				FontIcon{
					.icon = icon,
					.size = 16.f,
					.color = 0xFFFFFFFF,
				},
				Text{
					.text = text,
				},
			},
		};
	}
};

struct Separator {
	// Args
	squi::Widget::Args widget{};

	operator squi::Child() const {
		return Box{
			.widget{
				.width = 1.f,
				.margin = Margin(2.f, 4.f),
			},
			.color = Color(1.f, 1.f, 1.f, 0.1f),
		};
	}
};

TopBar::operator squi::Child() const {
	auto storage = std::make_shared<Storage>(Storage{
		.componentSelectorObserver = componentSelectorObserver,
		.boardStorage = boardStorage,
	});

	return Box{
		.widget{
			.height = 60.f,
			.padding = 6.f,
		},
		.color{0x202020FF},
		.borderColor{0, 0, 0, 0.1f},
		.borderWidth{0, 0, 1, 0},
		.borderPosition = squi::Box::BorderPosition::inset,
		.child = Row{
			.spacing = 2.f,
			.children = std::invoke([&storage, onRun = onRun] {
				Children ret{
					TopBarButton{
						.onClick = [onRun = onRun](GestureDetector::Event event) {
							Window::of(&event.widget).addOverlay(ContextMenu{
								.position = event.widget.getPos() + event.widget.state.padding->getPositionOffset() + event.widget.getSize().withX(0.f),
								.items{
									ContextMenu::Item{
										.text = "D.C. Simulation",
										.content = [onRun]() {
											onRun.notify(SimulationType::dcSim);
										},
									},
									ContextMenu::Item{
										.text = "A.C. Simulation",
										.content = [onRun]() {
											onRun.notify(SimulationType::acSim);
										},
									},
								},
							});
						},
						.child = IconTextCombo{
							.icon = 0xF5B0,
							.text = "Run",
						},
					},
					TopBarButton{
						.onClick = [storage](GestureDetector::Event) {
							storage->boardStorage.load();
						},
						.child = IconTextCombo{
							.icon = 0xED43,
							.text = "Load",
						},
					},
					TopBarButton{
						.onClick = [storage](GestureDetector::Event) {
							storage->boardStorage.save();
						},
						.child = IconTextCombo{
							.icon = 0xEA35,
							.text = "Save",
						},
					},
					Separator{},
				};
				for (const auto &comp: ComponentStore::components) {
					if (comp.get().hidden) continue;
					ret.emplace_back(Button{
						.widget{
							.width = 48.f,
							.height = 48.f,
							.margin = 0.f,
							.padding = 0.f,
						},
						.style = ButtonStyle::Subtle(),
						.onClick = [storage, &comp](GestureDetector::Event /*event*/) {
							storage->componentSelectorObserver.notify(comp.get());
						},
						.child = Align{
							.child = MsdfImage{
								.widget{
									.width{40.f},
									.height{40.f},
								},
								.texture = comp.get().textureThumb,
								.color{0xFFFFFFFF},
							},
						},
					});
				}
				return ret;
			}),
		},
	};
}
