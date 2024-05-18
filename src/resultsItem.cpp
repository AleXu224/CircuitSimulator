#include "resultsItem.hpp"
#include "button.hpp"
#include "container.hpp"
#include "row.hpp"


using namespace squi;
ResultsItem::operator squi::Child() const {
	return Button{
		.widget{
			.width = Size::Expand,
			.margin = Margin{4.f, 2.f},
		},
		.style = ButtonStyle::Subtle(),
		.onClick = [onClick = onClick](GestureDetector::Event) {
			if (onClick) onClick();
		},
		.child = Row{
			.spacing = 1.f,
			.children = [&]() {
				Children ret{};
				for (const auto &item: items) {
					ret.emplace_back(Container{
						.widget{
							.padding = Padding(12.f, 0.f),
						},
						.child = Align{
							.xAlign = 0.f,
							.child = Text{
								.text = item,
								.fontSize = 14.f,
							},
						},
					});
				}
				return ret;
			}(),
		},
	};
}
