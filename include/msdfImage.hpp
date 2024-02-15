#include "msdfQuad.hpp"
#include "pipeline.hpp"
#include "samplerUniform.hpp"
#include "widget.hpp"
#include "color.hpp"

struct MsdfImage {
	// Args
	squi::Widget::Args widget;
	const std::optional<Engine::SamplerUniform> &texture;
    squi::Color color{1.f, 1.f, 1.f, 1.f};

	using Quad = Engine::MsdfQuad;
	using Pipeline = Engine::Pipeline<Quad::Vertex, true>;
	static Pipeline *pipeline;

	class Impl : public squi::Widget {
		// Data
		const std::optional<Engine::SamplerUniform> &texture;
		Quad quad;

	public:
		Impl(const MsdfImage &args);

        void postLayout(squi::vec2 &size) override;
        void postArrange(squi::vec2 &pos) override;
        void onDraw() override;
	};

	operator squi::Child() const {
		return std::make_shared<Impl>(*this);
	}
};