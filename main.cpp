#include "boardview.hpp"
#include "window.hpp"


int main(int /*unused*/, char ** /*unused*/) {
	using namespace squi;

	Window window{};
	window.addChild(BoardView{});
	window.run();
}
