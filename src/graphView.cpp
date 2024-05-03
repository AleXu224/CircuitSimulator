#include "graphView.hpp"
#include "box.hpp"
#include "column.hpp"
#include "row.hpp"
#include "stack.hpp"
#include "text.hpp"

using namespace squi;

constexpr inline float bottomBarSize = 30.f;

struct LinesStacks {
	// Args
	squi::Widget::Args widget{};
	Observable<const Graph::Axes &> onInfoUpdate{};

	operator squi::Child() const {

		return Stack{
			.widget{
				.onInit = [onInfoUpdate = onInfoUpdate](Widget &w) {
					w.customState.add(onInfoUpdate.observe([&w](const Graph::Axes &axes) {
						Children ret{};
						const int labelCountVert = 1 + static_cast<int>(std::round((axes.y.maxValue - axes.y.minValue) / axes.y.step));
						const int labelCountHoriz = 1 + static_cast<int>(std::round((axes.x.maxValue - axes.x.minValue) / axes.x.step));
						ret.reserve(labelCountVert + labelCountHoriz);
						if (labelCountVert == 1) {
							ret.emplace_back(Box{
								.widget{
									.height = 1.f,
									.onArrange = [](Widget &w, vec2 &pos) {
										pos.y += w.state.parent->getContentSize().y - w.getLayoutSize().y;
									},
								},
								.color = Color(1.f, 1.f, 1.f, 0.2f),
								.shouldClipContent = false,
							});
						}
						if (labelCountHoriz == 1) {
							ret.emplace_back(Box{
								.widget{
									.width = 1.f,
								},
								.color = Color(1.f, 1.f, 1.f, 0.2f),
								.shouldClipContent = false,
							});
						}
						for (int i: std::views::iota(0, labelCountVert)) {
							ret.emplace_back(Box{
								.widget{
									.height = 1.f,
									.onArrange = [offset = (static_cast<float>(i) * axes.y.step) / (axes.y.maxValue - axes.y.minValue)](Widget &w, vec2 &pos) {
										const auto parentSize = w.state.parent->getContentSize();
										pos.y += std::clamp(
											parentSize.y - parentSize.y * offset - w.getLayoutSize().y / 2.f,
											0.f,
											parentSize.y - w.getLayoutSize().y
										);
										pos.x += parentSize.x - w.getLayoutSize().x;
									},
								},
								.color = Color(1.f, 1.f, 1.f, 0.2f),
								.shouldClipContent = false,
							});
						}

						for (int i: std::views::iota(0, labelCountHoriz)) {
							ret.emplace_back(Box{
								.widget{
									.width = 1.f,
									.onArrange = [offset = (static_cast<float>(i) * axes.x.step) / (axes.x.maxValue - axes.x.minValue)](Widget &w, vec2 &pos) {
										const auto parentSize = w.state.parent->getContentSize();
										pos.x += std::clamp(
											parentSize.x * offset - w.getLayoutSize().x / 2.f,
											0.f,
											parentSize.x - w.getLayoutSize().x
										);
									},
								},
								.color = Color(1.f, 1.f, 1.f, 0.2f),
								.shouldClipContent = false,
							});
						}

						w.setChildren(ret);
					}));
				},
			},
		};
	}
};

GraphView::operator squi::Child() const {
	Observable<const Graph::Axes &> onInfoUpdate{};
	auto storage = std::make_shared<Storage>();

	storage->onInfoChangeObs = onInfoUpdate.observe([storage = std::weak_ptr<Storage>(storage)](const Graph::Axes &axes) {
		if (auto _ = storage.lock()) {
			_->axes = axes;
		}
	});
	return Row{
		.widget = widget,
		.children{
			Stack{
				.widget{
					.width = Size::Shrink,
					.padding = Padding(0.f, 10.f, bottomBarSize, 0),
					.onInit = [storage, onInfoUpdate](Widget &w) {
						w.customState.add(onInfoUpdate.observe([&w](const Graph::Axes &axes) {
							Children ret{};
							const int labelCount = 1 + static_cast<int>(std::round((axes.y.maxValue - axes.y.minValue) / axes.y.step));
							ret.reserve(labelCount);
							for (int i: std::views::iota(0, labelCount)) {
								ret.emplace_back(Text{
									.widget{
										.onArrange = [offset = (static_cast<float>(i) * axes.y.step) / (axes.y.maxValue - axes.y.minValue)](Widget &w, vec2 &pos) {
											const auto parentSize = w.state.parent->getContentSize();
											pos.y += std::clamp(
												parentSize.y - parentSize.y * offset - w.getLayoutSize().y / 2.f,
												0.f,
												parentSize.y - w.getLayoutSize().y
											);
											pos.x += parentSize.x - w.getLayoutSize().x;
										},
									},
									.text = std::format("{:.2f}", axes.y.minValue + static_cast<float>(i) * axes.y.step),
									.color = Color(1.f, 1.f, 1.f, 0.8f),
								});
							}

							w.setChildren(ret);
						}));
					},
				},
			},
			Column{
				.children{
					Stack{
						.children{
							LinesStacks{
								.onInfoUpdate = onInfoUpdate,
							},
							Graph{
								.data = values,
								.onInfoUpdate = onInfoUpdate,
								.onDataUpdate = updateData,
							},
						},
					},
					Stack{
						.widget{
							.height = bottomBarSize,
							.padding = Padding(10.f, 0, 0, 0),
							.onInit = [storage, onInfoUpdate](Widget &w) {
								w.customState.add(onInfoUpdate.observe([&w](const Graph::Axes &axes) {
									Children ret{};
									const int labelCount = 1 + static_cast<int>(std::round((axes.x.maxValue - axes.x.minValue) / axes.x.step));
									ret.reserve(labelCount);
									for (int i: std::views::iota(0, labelCount)) {
										ret.emplace_back(Text{
											.widget{
												.onArrange = [offset = (static_cast<float>(i) * axes.x.step) / (axes.x.maxValue - axes.x.minValue)](Widget &w, vec2 &pos) {
													const auto parentSize = w.state.parent->getContentSize();
													pos.x += std::clamp(
														parentSize.x * offset - w.getLayoutSize().x / 2.f,
														0.f,
														parentSize.x - w.getLayoutSize().x
													);
												},
											},
											.text = std::format("{:.2f}", axes.x.minValue + static_cast<float>(i) * axes.x.step),
											.color = Color(1.f, 1.f, 1.f, 0.8f),
										});
									}

									w.setChildren(ret);
								}));
							},
						},
					},
				},
			},
		},
	};
}
