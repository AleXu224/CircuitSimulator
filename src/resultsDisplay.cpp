#include "resultsDisplay.hpp"
#include "box.hpp"
#include "gestureDetector.hpp"
#include "observer.hpp"
#include "registerEvent.hpp"
#include "row.hpp"
#include "window.hpp"
#include "GLFW/glfw3.h"

using namespace squi;

constexpr float minDrawerWidth = 450.f;

struct ResizeBar {
	// Args
	Observable<float> resizeDragObs{};
	Observable<bool> cursorObs{};

	struct Storage {
		// Data
		bool hovered = false;
		bool focused = false;
		float width = minDrawerWidth;
		std::unique_ptr<GLFWcursor, void (*)(GLFWcursor *)> cursor{
			glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR),
			[](GLFWcursor *ptr) {
				glfwDestroyCursor(ptr);
			}
		};

		[[nodiscard]] bool shouldSetCursor() const {
			return hovered || focused;
		}
	};

	operator squi::Child() const {
		auto storage = std::make_shared<Storage>();

		return GestureDetector{
			.onEnter = [storage, cursorObs = cursorObs](GestureDetector::Event) {
				storage->hovered = true;
				cursorObs.notify(storage->shouldSetCursor());
			},
			.onLeave = [storage, cursorObs = cursorObs](GestureDetector::Event) {
				storage->hovered = false;
				cursorObs.notify(storage->shouldSetCursor());
			},
			.onFocus = [storage, cursorObs = cursorObs](GestureDetector::Event) {
				storage->focused = true;
				cursorObs.notify(storage->shouldSetCursor());
			},
			.onFocusLoss = [storage, cursorObs = cursorObs](GestureDetector::Event) {
				storage->focused = false;
				cursorObs.notify(storage->shouldSetCursor());

				storage->width = std::max(minDrawerWidth, storage->width);
			},
			.onDrag = [storage, resizeDragObs = resizeDragObs](GestureDetector::Event event) {
				storage->width -= event.state.getDragDelta().x;
				resizeDragObs.notify(std::max(minDrawerWidth, storage->width));
			},
			.child = Box{
				.widget{
					.width = 4.f,
					.onInit = [storage, cursorObs = cursorObs](Widget &w) {
						w.customState.add(cursorObs.observe([&w, storage](bool setCursor) {
							auto *windowPtr = Window::of(&w).engine.instance.window.ptr;
							auto &box = w.as<Box::Impl>();
							if (setCursor) {
								glfwSetCursor(windowPtr, storage->cursor.get());
								box.setColor(0xFFFFFFFF);
							} else {
								glfwSetCursor(windowPtr, nullptr);
								box.setColor(0x0);
							}
						}));
					},
				},
				.color{0x0},
			},
		};
	}
};

ResultsDisplay::operator squi::Child() const {
	auto dragObs = Observable<float>();
	auto storage = std::make_shared<Storage>();

	return RegisterEvent{
		.onInit = [storage, dragObs, destroyObs = destroyObs](Widget &w) {
			w.customState.add(dragObs.observe([&w, storage](float newWidth) {
				w.state.width = newWidth;
			}));
			w.customState.add(destroyObs.observe([&w]() {
				w.deleteLater();
			}));
		},
		.child = Box{
			.widget{widget.withDefaultWidth(minDrawerWidth)},
			.color{0x202020FF},
			.child = Row{
				.children{
					ResizeBar{.resizeDragObs = dragObs},
					child,
				},
			},
		},
	};
}
