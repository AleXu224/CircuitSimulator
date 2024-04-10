#include "topBar.hpp"
#include "align.hpp"
#include "box.hpp"
#include "button.hpp"
#include "components/componentStore.hpp"
#include "fontIcon.hpp"
#include "gestureDetector.hpp"
#include "msdfImage.hpp"
#include "row.hpp"
#include "widget.hpp"
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
						.onClick = [onRun = onRun](GestureDetector::Event) {
							onRun.notify();
						},
						.child = FontIcon{
							.icon = 0xF5B0,
							.size = 24.f,
							.color = 0xFFFFFFFF,
						},
					},
					TopBarButton{
						.onClick = [storage](GestureDetector::Event) {
							storage->boardStorage.load();
						},
						.child = FontIcon{
							.icon = 0xED43,
							.size = 24.f,
							.color = 0xFFFFFFFF,
						},
					},
					TopBarButton{
						.onClick = [storage](GestureDetector::Event) {
							storage->boardStorage.save();
						},
						.child = FontIcon{
							.icon = 0xEA35,
							.size = 24.f,
							.color = 0xFFFFFFFF,
						},
					},
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
