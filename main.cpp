#include "window.hpp"
#include "boardview.hpp"

int main(int, char**){
    using namespace squi;

    Window window;
    window.addChild(BoardView{});
	window.run();
}
