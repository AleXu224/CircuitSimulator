#include "nodeIndexDisplay.hpp"
#include "align.hpp"
#include "box.hpp"
#include "config.hpp"
#include "text.hpp"

using namespace squi;

NodeIndexDisplay::operator squi::Child() const {
	auto storage = std::make_shared<Storage>();

	return Box{
		.widget{
			.width = Size::Shrink,
			.height = 20.f,
            .sizeConstraints{
                .minWidth = 20.f,
            },
            .padding = Padding{4.f, 0.f},
            .onArrange = [coords = pos](Widget &w, vec2& pos){
                pos += coords.toVec() * gridSize - w.getSize() / 2.f;
            },
		},
		.color{0x000000FF},
        .borderColor{0xFFFFFFFF},
        .borderWidth{2.f},
		.borderRadius{4.f},
        .child = Align{
            .child = Text{
                .text = std::format("{}", nodeIndex),
            },
        },
	};
}
