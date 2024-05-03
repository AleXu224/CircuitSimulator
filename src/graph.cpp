#include "graph.hpp"
#include "color.hpp"
#include "compiledShaders/linefrag.hpp"
#include "compiledShaders/linevert.hpp"
#include "ranges"
#include "window.hpp"


using namespace squi;

Graph::Pipeline *Graph::pipeline = nullptr;

namespace stdr = std::ranges;
namespace stdv = std::views;

float getMultiplier(float val) {
	if (val <= 0.1f) return 0.1f;
	if (val <= 0.2f) return 0.2f;
	if (val <= 0.25f) return 0.25f;
	if (val <= 0.3) return 0.3;
	if (val <= 0.4) return 0.4;
	if (val <= 0.5) return 0.5;
	if (val <= 0.6) return 0.6;
	if (val <= 0.7) return 0.7;
	if (val <= 0.75) return 0.75;
	if (val <= 0.8) return 0.8;
	if (val <= 0.9) return 0.9;
	return 1.f;
}

float roundTick(float tick) {
	auto x = std::ceil(std::log10(tick));
	float pow10x = std::pow(10.f, x);
	float roundedTickRange = getMultiplier(tick / pow10x) * pow10x;
	return roundedTickRange;
}

void Graph::Impl::prepareData() {
	if (data.size() < 1) {
		axes = Axes{};
		onInfoUpdate.notify(axes);
		return;
	}

	stdr::sort(data, [](const auto &val1, const auto &val2) {
		return val1.x < val2.x;
	});

	axes.x.minValue = data.front().x;
	axes.x.maxValue = data.front().x;
	axes.y.minValue = data.front().value;
	axes.y.maxValue = data.front().value;

	for (const auto &val: data) {
		axes.x.minValue = std::min(axes.x.minValue, val.x);
		axes.y.minValue = std::min(axes.y.minValue, val.value);

		axes.x.maxValue = std::max(axes.x.maxValue, val.x);
		axes.y.maxValue = std::max(axes.y.maxValue, val.value);
	}

	auto xTick = roundTick((axes.x.maxValue - axes.x.minValue) / 8.f);
	auto yTick = roundTick((axes.y.maxValue - axes.y.minValue) / 8.f);

	axes.x.minValue = xTick * std::floor(axes.x.minValue / xTick);
	axes.y.minValue = yTick * std::floor(axes.y.minValue / yTick);

	axes.x.maxValue = xTick * std::ceil(axes.x.maxValue / xTick);
	axes.y.maxValue = yTick * std::ceil(axes.y.maxValue / yTick);

	axes.x.step = xTick;
	axes.y.step = yTick;
	onInfoUpdate.notify(axes);
}

void Graph::Impl::regenerateQuads() {
	quads.clear();
	if (data.size() <= 1) return;
	const auto offset = getContentPos().withYOffset(getContentSize().y);
	for (const auto &[v1, v2]: stdv::pairwise(data)) {
		quads.emplace_back(Quad::Args{
			.color = Color(0xFFFFFFFF),
			.lineStart = offset + getPosOf(v1),
			.lineEnd = offset + getPosOf(v2),
			.lineWidth = 1.f,
		});
	}
}

vec2 Graph::Impl::getPosOf(const LineValue &val) {
	const auto xVal = val.x - axes.x.minValue;
	const auto yVal = val.value - axes.y.minValue;

	const auto relativeMaxX = axes.x.maxValue - axes.x.minValue;
	const auto relativeMaxY = axes.y.maxValue - axes.y.minValue;

	const auto contentSize = getContentSize();
	return {
		xVal / relativeMaxX * contentSize.x,
		-yVal / relativeMaxY * contentSize.y,
	};
}

Graph::Impl::Impl(const Graph &args)
	: Widget(args.widget, Widget::FlagsArgs::Default()),
	  data(args.data),
	  onInfoUpdate(args.onInfoUpdate),
	  onDataUpdate(args.onDataUpdate.observe([&self = *this](const std::vector<LineValue> &newData) {
		  self.data = newData;
		  self.dataChanged = true;
	  })) {
}

void Graph::Impl::onUpdate() {
	if (dataChanged) {
		prepareData();
		dataChanged = false;
	}
}

void Graph::Impl::postArrange(squi::vec2 & /*pos*/) {
	regenerateQuads();
}

void Graph::Impl::onDraw() {
	if (!pipeline) {
		auto &instance = Window::of(this).engine.instance;
		pipeline = &instance.createPipeline<Pipeline>(Pipeline::Args{
			.vertexShader = Engine::Shaders::linevert,
			.fragmentShader = Engine::Shaders::linefrag,
			.instance = instance,
		});
	}

	pipeline->bind();
	for (auto &quad: quads) {
		auto [vi, ii] = pipeline->getIndexes();
		pipeline->addData(quad.getData(vi, ii));
	}
}
