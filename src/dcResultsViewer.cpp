#include "dcResultsViewer.hpp"
#include "align.hpp"
#include "button.hpp"
#include "gestureDetector.hpp"
#include "scrollableFrame.hpp"
#include "text.hpp"

using namespace squi;

struct Card {
	// Args
	Child child{};

	operator squi::Child() const {
		return Box{
			.widget{
				.height = Size::Shrink,
				.padding = 4.f,
			},
			.color = Color(1.f, 1.f, 1.f, 0.0512f),
			.borderColor = Color(0.f, 0.f, 0.f, 0.1f),
			.borderWidth{1.f},
			.borderRadius{8.f},
			.borderPosition = squi::Box::BorderPosition::outset,
			.child = child,
		};
	}
};

struct ResultsItem {
	// Args
	std::string_view text;
	static inline uint32_t counter = 0;
	std::function<void()> onClick;

	operator squi::Child() const {
		return Button{
			.widget{
				.width = Size::Expand,
				.margin = Margin{4.f, 2.f},
			},
			.style = ButtonStyle::Subtle(),
			.onClick = [onClick = onClick](GestureDetector::Event) {
				if (onClick) onClick();
			},
			.child = Align{
				.xAlign = 0.f,
				.child = Text{
					.text = text,
					.fontSize = 14.f,
				},
			},
		};
	}
};

struct Heading {
	// Args
	std::string_view text;

	operator squi::Child() const {
		return Text{
			.widget{
				.margin = Margin{16.f, 4.f, 8.f, 8.f},
			},
			.text = text,
			.fontSize = 20.f,
			.font = FontStore::defaultFontBold,
		};
	}
};

DCResultsViewer::operator squi::Child() const {
	auto storage = std::make_shared<Storage>();

	return ScrollableFrame{
		.widget{widget},
		.scrollableWidget{
			.padding = 4.f,
		},
		.spacing = 4.f,
		.children = [&]() -> Children {
			Children ret{};
			if (!simulation.voltages.empty()) {
				Children voltRet{
					Heading{
						.text = "Voltages",
					}
				};
				for (const auto &[index, val]: simulation.voltages | std::views::enumerate) {
					voltRet.emplace_back(ResultsItem{
						.text = std::format("N{}: {:.2f}V", index + 1, val),
						.onClick = [elementSelector = elementSelector, node = graph.nodes.at(index + 1)]() {
							std::vector<ElementId> ret{};
							ret.reserve(node.lines.size());
							for (const auto &line: node.lines) {
								ret.emplace_back(line.id);
							}
							elementSelector.notify(ret);
						},
					});
				}
				ret.emplace_back(Card{.child{Column{.children = voltRet}}});
			}

			if (!simulation.currents.empty()) {
				Children currRet{
					Heading{
						.text = "Currents",
					}
				};
				for (const auto &val: simulation.currents) {
					currRet.emplace_back(ResultsItem{
						.text = std::format("V{}: {:.2f}A", val.id, val.value),
						.onClick = [elementSelector = elementSelector, id = val.id]() {
							elementSelector.notify({id});
						},
					});
				}
				ret.emplace_back(Card{.child{Column{.children = currRet}}});
			}

			return ret;
		}(),
	};
}
