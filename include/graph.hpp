#pragma once
#include "lineQuad.hpp"
#include "observer.hpp"
#include "widget.hpp"


struct Graph {
	// Args
	struct LineValue {
		float value;
		float x;
	};

	struct AxisInfo {
		float minValue = 0.f;
		float maxValue = 0.f;
		float step = 0.1f;
	};

	struct Axes {
		AxisInfo x{};
		AxisInfo y{};
	};

	squi::Widget::Args widget{};
	std::vector<LineValue> data{};
	squi::Observable<const Axes &> onInfoUpdate{};
	squi::Observable<const std::vector<LineValue> &> onDataUpdate{};

	using Quad = Engine::LineQuad;
	using Pipeline = Engine::Pipeline<Quad::Vertex>;
	static Pipeline *pipeline;


	class Impl : public squi::Widget {
		// Data
        bool dataChanged = true;
		std::vector<LineValue> data{};
		squi::Observable<const Axes &> onInfoUpdate{};
		squi::Observer<const std::vector<LineValue> &> onDataUpdate{};
		std::vector<Quad> quads{};
		Axes axes{};

		void prepareData();
		void regenerateQuads();

		squi::vec2 getPosOf(const LineValue &val);

	public:
		Impl(const Graph &args);
        void onUpdate() override;
		void postArrange(squi::vec2 &pos) override;
		void onDraw() override;
	};

	operator squi::Child() const {
		return std::make_shared<Impl>(*this);
	}
};