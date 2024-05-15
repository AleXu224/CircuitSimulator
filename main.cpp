#include "boardview.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>


int main(int /*unused*/, char ** /*unused*/) {
	using namespace squi;

	Window window{};
	glfwSetWindowTitle(window.engine.instance.window.ptr, "Circuit Simulator");
	window.addChild(BoardView{});
	window.run();
}
