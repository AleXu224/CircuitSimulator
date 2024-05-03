#include "acResultsViewer.hpp"
#include "align.hpp"
#include "button.hpp"
#include "gestureDetector.hpp"
#include "graphView.hpp"
#include "numbers"
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
				.margin = 8.f,
			},
			.text = text,
			.fontSize = 20.f,
			.font = FontStore::defaultFontBold,
		};
	}
};

inline float getPhase(const std::complex<float> &val) {
	return std::atan2(val.imag(), val.real()) / std::numbers::pi_v<float> * 180.f;
}

std::vector<Graph::LineValue> generateGraphData(const std::complex<float> &val) {
	std::vector<Graph::LineValue> data{};
	data.reserve(1000);
	const auto phase = std::atan2(val.imag(), val.real());
	const auto magnitude = std::sqrt(val.real() * val.real() + val.imag() * val.imag());
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
				Children voltRet{
					Heading{
						.text = "Voltages",
					}
				};
				for (const auto &[index, val]: simulation.voltages | std::views::enumerate) {
					voltRet.emplace_back(ResultsItem{
						.text = std::format(
							"N{} {}V {}°",
							index + 1,
							std::sqrt(val.real() * val.real() + val.imag() * val.imag()),
							getPhase(val)
						),
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
						.text = std::format(
							"V{} {}A, {}°",
							val.id,
							std::sqrt(val.value.real() * val.value.real() + val.value.imag() * val.value.imag()),
							getPhase(val.value)
						),
						.onClick = [elementSelector = elementSelector, id = val.id, graphDataUpdater, val]() {
							elementSelector.notify({id});

							graphDataUpdater.notify(generateGraphData(val.value));
						},
					});
				}
				ret.emplace_back(Card{.child{Column{.children = currRet}}});
			}

			return ret;
		}(),
	};
}
