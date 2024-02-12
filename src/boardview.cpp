#include "boardview.hpp"
#include "box.hpp"
#include "compiledShaders/boardBackgroundfrag.hpp"
#include "compiledShaders/boardBackgroundvert.hpp"
#include "gestureDetector.hpp"
#include "image.hpp"
#include "window.hpp"
#include <print>


using namespace squi;

BoardView::Pipeline *BoardView::pipeline = nullptr;

BoardView::Impl::Impl(const BoardView &args)
	: Widget(args.widget, Widget::FlagsArgs::Default()),
	  gd(GestureDetector{
		  .onClick = [&](GestureDetector::Event event) {
			  if ((GestureDetector::getMousePos() - event.state.getDragStartPos()).length() > 5.f) return;
			  const auto mousePos = GestureDetector::getMousePos();
			  const auto widgetPos = event.widget.getPos();
			  const auto virtualPos = mousePos - widgetPos - viewOffset;

			  auto gridPos = virtualPos / gridWidth;
			  gridPos.x = std::floor(gridPos.x);
			  gridPos.y = std::floor(gridPos.y);

			  gridPos.print();

			  if (!board.contains({static_cast<int>(gridPos.x), static_cast<int>(gridPos.y)})) {
				  board[{static_cast<int>(gridPos.x), static_cast<int>(gridPos.y)}] = BoardElement{
					  .child = Box{
						  .widget{.width = 40.f, .height = 80.f},
						  .color = 0x0,
						  .child{
							  Image{
								  .image = Image::Data::fromFile(R"(C:\Users\Squizell\Downloads\msdfgen-1.11-win64\msdfgen\output2.png)"),
							  },
						  },
					  },
				  };
				  this->reLayout();
			  }
		  },
	  }
			 .mount(*this)),
	  quad(Quad::Args{
		  .position{0, 0},
		  .size{0, 0},
		  .offset{0, 0},
	  }) {
	board[Coords{1, 1}] = BoardElement{.child = Box{.widget{.width = 20.f, .height = 20.f}}};
}

void BoardView::Impl::onUpdate() {
	static bool first = true;
	if (gd.focused) {
		if (!first) {
			first = true;
		} else if (GestureDetector::getMouseDelta().length() != 0.f) {
			viewOffset += GestureDetector::getMouseDelta();
			quad.offset = viewOffset;
			this->reArrange();
		}
	}
}

void BoardView::Impl::postArrange(squi::vec2 &pos) {
	quad.position = pos;
}

void BoardView::Impl::postLayout(squi::vec2 &size) {
	quad.size = size;
}

void BoardView::Impl::onDraw() {
	if (!BoardView::pipeline) {
		BoardView::pipeline = &Window::of(this).engine.instance.createPipeline<BoardView::Pipeline>(BoardView::Pipeline::Args{
			.vertexShader = Engine::Shaders::boardBackgroundvert,
			.fragmentShader = Engine::Shaders::boardBackgroundfrag,
			.instance = Window::of(this).engine.instance,
		});
	}

	pipeline->bind();
	auto [vi, ii] = pipeline->getIndexes();
	pipeline->addData(quad.getData(vi, ii));
}

void BoardView::Impl::updateChildren() {
	for (auto [key, val]: board) {
		val.child->state.parent = this;
		val.child->state.root = *state.root;
		val.child->update();
	}
}

squi::vec2 BoardView::Impl::layoutChildren(squi::vec2 maxSize, squi::vec2 minSize, ShouldShrink shouldShrink) {
	for (auto [key, val]: board) {
		if (!val.child) continue;
		val.child->layout(vec2::infinity(), vec2{}, {false, false});
	}
	return {};
}

void BoardView::Impl::arrangeChildren(squi::vec2 &pos) {
	const auto newPos = pos + viewOffset;
	for (auto [key, val]: board) {
		if (!val.child) continue;
		val.child->arrange(newPos +
						   vec2{
							   static_cast<float>(key.x) * gridWidth,
							   static_cast<float>(key.y) * gridWidth,
						   });
	}
}

void BoardView::Impl::drawChildren() {
	for (auto [key, val]: board) {
		val.child->draw();
	}
}
