#include "boardview.hpp"
#include "box.hpp"
#include "gestureDetector.hpp"
#include <print>

using namespace squi;

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
						  .widget{.width = 20.f, .height = 20.f},
					  },
				  };
				  this->reLayout();
			  }
		  },
	  }
			 .mount(*this)) {
	board[Coords{1, 1}] = BoardElement{.child = Box{.widget{.width = 20.f, .height = 20.f}}};
}

void BoardView::Impl::onUpdate() {
	static bool first = true;
	if (gd.focused) {
		if (!first) {
			first = true;
		} else if (GestureDetector::getMouseDelta().length() != 0.f) {
			viewOffset += GestureDetector::getMouseDelta();
			this->reArrange();
		}
	}
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
