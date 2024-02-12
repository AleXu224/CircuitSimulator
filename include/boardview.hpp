#include "boardElement.hpp"
#include "gestureDetector.hpp"
#include "boardBackgroundQuad.hpp"
#include "pipeline.hpp"
#include "widget.hpp"
#include <unordered_map>


struct Coords {
	int x = 0;
	int y = 0;

	bool operator==(const Coords &other) const {
		return x == other.x && y == other.y;
	}
};

namespace std {
	template<>
	struct hash<Coords> {
		std::size_t operator()(const Coords &coords) const {
			std::size_t h1 = std::hash<int>{}(coords.x);
			std::size_t h2 = std::hash<int>{}(coords.y);
			return h1 ^ (h2 << 1);
		}
	};
}// namespace std

struct BoardView {
	// Args
	squi::Widget::Args widget;
	using Quad = Engine::BoardBackgroundQuad;
	using Pipeline = Engine::Pipeline<Quad::Vertex>;

	static Pipeline* pipeline;
	class Impl : public squi::Widget {
		// Data
		std::unordered_map<Coords, BoardElement> board{};
		squi::GestureDetector::State &gd;
		Quad quad;
        squi::vec2 viewOffset{};
        static constexpr float gridWidth = 20.f;

		void onUpdate() override;
		void postArrange(squi::vec2 &pos) override;
		void postLayout(squi::vec2 &size) override;
		void onDraw() override;

        void updateChildren() override;
        squi::vec2 layoutChildren(squi::vec2 maxSize, squi::vec2 minSize, ShouldShrink shouldShrink) override;
        void arrangeChildren(squi::vec2 &pos) override;
        void drawChildren() override;

	public:
		Impl(const BoardView &args);
	};

	operator squi::Child() const { return std::make_shared<Impl>(*this); }
};