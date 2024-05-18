#include "acResultsViewer.hpp"
#include "card.hpp"
#include "graphView.hpp"
#include "numbers"
#include "resultsCard.hpp"
#include "resultsItem.hpp"
#include "scrollableFrame.hpp"


using namespace squi;

static inline std::vector<std::string_view> columnNames{
	"Index",
	"Value",
	"Phase",
};

inline float getPhase(const std::complex<float> &val) {
	return std::atan2(val.imag(), val.real()) / std::numbers::pi_v<float> * 180.f;
}

std::vector<Graph::LineValue> generateGraphData(const std::complex<float> &val) {
	std::vector<Graph::LineValue> data{};
	data.reserve(1000);
	const auto phase = std::atan2(val.imag(), val.real());
	const auto magnitude = std::sqrt(val.real() * val.real() + val.imag() * val.imag()) * std::sqrt(2.f);
	const auto angularFrequency = 50.f * 2.f * std::numbers::pi_v<float>;
	for (int i = 0; i < 100; i++) {
		data.emplace_back(Graph::LineValue{
			.value = magnitude * std::sin(angularFrequency * static_cast<float>(i) / 1000.f + phase),
			.x = static_cast<float>(i),
		});
	}

	return data;
}

ACResultsViewer::operator squi::Child() const {
	auto storage = std::make_shared<Storage>();

	return ScrollableFrame{
		.widget{widget},
		.scrollableWidget{
			.padding = 4.f,
		},
		.spacing = 4.f,
		.children = [&]() -> Children {
			Observable<const std::vector<Graph::LineValue> &> graphDataUpdater{};
			Children ret{
				Card{
					.child = GraphView{
						.widget{
							.height = 300.f,
							.margin = 4.f,
						},
						.updateData = graphDataUpdater,
						.values{},
					},
				},
			};

			if (!simulation.voltages.empty()) {
				Children voltRet{};
				for (const auto &[index, val]: simulation.voltages | std::views::enumerate) {
					voltRet.emplace_back(ResultsItem{
						.items{
							std::format("Node #{}", index + 1),
							std::format("{}V", std::sqrt(val.real() * val.real() + val.imag() * val.imag())),
							std::format("{}°", getPhase(val)),
						},
						.onClick = [elementSelector = elementSelector, node = graph.nodes.at(index + 1), graphDataUpdater, val]() {
							std::vector<ElementId> ret{};
							ret.reserve(node.lines.size());
							for (const auto &line: node.lines) {
								ret.emplace_back(line.id);
							}
							elementSelector.notify(ret);

							graphDataUpdater.notify(generateGraphData(val));
						},
					});
				}
				ret.emplace_back(ResultsCard{
					.title = "Voltages",
					.columns = columnNames,
					.children = voltRet,
				});
			}

			if (!simulation.currents.empty()) {
				Children currRet{};
				for (const auto &val: simulation.currents) {
					currRet.emplace_back(ResultsItem{
						.items{
							std::format("{} #{}", board.getElement(val.id).value().get().element.component.get().name, val.id),
							std::format("{}V", std::sqrt(val.value.real() * val.value.real() + val.value.imag() * val.value.imag())),
							std::format("{}°", getPhase(val.value)),
						},
						.onClick = [elementSelector = elementSelector, id = val.id, graphDataUpdater, val]() {
							elementSelector.notify({id});

							graphDataUpdater.notify(generateGraphData(val.value));
						},
					});
				}
				ret.emplace_back(ResultsCard{
					.title = "Currents",
					.columns = columnNames,
					.children = currRet,
				});
			}

			return ret;
		}(),
	};
}
