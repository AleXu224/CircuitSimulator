#include "topBar.hpp"
#include "align.hpp"
#include "box.hpp"
#include "button.hpp"
#include "components/componentStore.hpp"
#include "gestureDetector.hpp"
#include "msdfImage.hpp"
#include "row.hpp"
#include <algorithm>
#include <functional>



using namespace squi;

TopBar::operator squi::Child() const {
	auto storage = std::make_shared<Storage>(Storage{
		.componentSelectorObserver = componentSelectorObserver,
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
			.children = std::invoke([&storage] {
				Children ret{};
				for (const auto &comp: ComponentStore::components) {
					if (comp.get().hidden) continue;
					auto maxSize = std::max(comp.get().width, comp.get().height);
					auto width = (static_cast<float>(comp.get().width) / static_cast<float>(maxSize)) * 40.f;
					auto height = (static_cast<float>(comp.get().height) / static_cast<float>(maxSize)) * 40.f;
					ret.emplace_back(Button{
						.widget{
							.width = 48.f,
							.height = 48.f,
							.margin = 0.f,
							.padding = 0.f,
						},
						.style = ButtonStyle::Subtle(),
						.onClick = [storage, &comp](GestureDetector::Event  /*event*/){
							storage->componentSelectorObserver->notify(comp.get());
						},
						.child = Align{
							.child = MsdfImage{
								.widget{
									.width{width},
									.height{height},
								},
								.texture = comp.get().texture,
								.color{0xFFFFFFFF},
								.uvTopLeft{comp.get().uvTopLeft},
								.uvBottomRight{comp.get().uvBottomRight},
							},
						},
					});
				}
				return ret;
			}),
		},
	};
}
