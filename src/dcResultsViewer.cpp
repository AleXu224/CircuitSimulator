#include "dcResultsViewer.hpp"
#include "align.hpp"
#include "button.hpp"
#include "gestureDetector.hpp"
#include "scrollableFrame.hpp"
#include "text.hpp"

using namespace squi;

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
		};
	}
};

DCResultsViewer::operator squi::Child() const {
	auto storage = std::make_shared<Storage>();

	return ScrollableFrame{
		.widget{widget},
		.children = [&]() -> Children {
			Children ret{};
			if (!simulation.voltages.empty()) {
				ret.emplace_back(
					Heading{
						.text = "Voltages",
					}
				);
				for (const auto &[index, val]: simulation.voltages | std::views::enumerate) {
					ret.emplace_back(ResultsItem{
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
			}

			if (!simulation.currents.empty()) {
				ret.emplace_back(
					Heading{
						.text = "Currents",
					}
				);
				for (const auto &val: simulation.currents) {
					ret.emplace_back(ResultsItem{
						.text = std::format("V{}: {:.2f}A", val.id, val.value),
						.onClick = [elementSelector = elementSelector, id = val.id]() {
							elementSelector.notify({id});
						},
					});
				}
			}

			return ret;
		}(),
	};
}
