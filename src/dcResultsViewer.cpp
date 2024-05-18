#include "dcResultsViewer.hpp"
#include "resultsCard.hpp"
#include "resultsItem.hpp"
#include "scrollableFrame.hpp"

using namespace squi;

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
				Children voltRet{};
				for (const auto &[index, val]: simulation.voltages | std::views::enumerate) {
					voltRet.emplace_back(ResultsItem{
						.items{
							std::format("Node #{}", index + 1),
							std::format("{:.2f}V", val),
						},
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
				ret.emplace_back(ResultsCard{
					.title = "Voltages",
					.columns{"Index", "Value"},
					.children = voltRet,
				});
			}

			if (!simulation.currents.empty()) {
				Children currRet{};
				for (const auto &val: simulation.currents) {
					currRet.emplace_back(ResultsItem{
						.items{
							std::format("{} #{}", board.getElement(val.id)->get().element.component.get().name, val.id),
							std::format("{:.2f}A", val.value),
						},
						.onClick = [elementSelector = elementSelector, id = val.id]() {
							elementSelector.notify({id});
						},
					});
				}
				ret.emplace_back(ResultsCard{
					.title = "Currents",
					.columns{"Index", "Value"},
					.children = currRet,
				});
			}

			return ret;
		}(),
	};
}
