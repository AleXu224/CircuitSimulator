#include "msdfImage.hpp"
#include "compiledShaders/msdfQuadfrag.hpp"
#include "compiledShaders/msdfQuadvert.hpp"
#include "window.hpp"


using namespace squi;

MsdfImage::Pipeline *MsdfImage::pipeline = nullptr;

MsdfImage::Impl::Impl(const MsdfImage &args)
	: Widget(args.widget, Widget::FlagsArgs::Default()), texture(args.texture), quad({.color = args.color}) {
}

void MsdfImage::Impl::postLayout(squi::vec2 &size) {
	quad.size = size;
}

void MsdfImage::Impl::postArrange(squi::vec2 &pos) {
	quad.position = pos;
}

void MsdfImage::Impl::onDraw() {
	if (!pipeline) {
		auto &instance = Window::of(this).engine.instance;
		pipeline = &instance.createPipeline<Pipeline>(Pipeline::Args{
			.vertexShader = Engine::Shaders::msdfQuadvert,
			.fragmentShader = Engine::Shaders::msdfQuadfrag,
			.instance = instance,
		});
	}

    if (!texture) return;

	pipeline->bindWithSampler(texture.value());
	auto [vi, ii] = pipeline->getIndexes();
	pipeline->addData(quad.getData(vi, ii));
}
