#include "window.hpp"
#include "boardview.hpp"
#include <print>

int main(int /*unused*/, char** /*unused*/){
    using namespace squi;

    Window window;
    window.addChild(BoardView{});
	window.run();
}
