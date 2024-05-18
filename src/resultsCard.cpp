#include "resultsCard.hpp"
#include "align.hpp"
#include "box.hpp"
#include "card.hpp"
#include "column.hpp"
#include "container.hpp"
#include "row.hpp"
#include "stack.hpp"
#include "text.hpp"


using namespace squi;

struct TableHeadElement {
	// Args
	std::string_view text{};

	operator squi::Child() const {
		return Container{
			.widget{
				.padding = Padding(12.f, 0.f),
			},
			.child = Align{
				.xAlign = 0.f,
				.child = Text{
					.text = text,
					.color{1.f, 1.f, 1.f, 0.8f},
				},
			},
		};
	}
};

struct Table {
	// Args
	std::vector<std::string_view> columns{};
	Children children{};

	operator squi::Child() const {
		return Stack{
			.children{
				Row{
					.widget{
						.height = 40.f,
						.margin = Margin(16.f, 0.f),
					},
					.spacing = 1.f,
					.children = [&]() {
						Children ret{};
						for (const auto &column: columns) {
							ret.emplace_back(TableHeadElement{column});
						}
						return ret;
					}(),
				},
				Row{
					.widget{
						.padding = Padding(40.f, 16.f, 0.f, 16.f),
					},
					.children = [&]() {
						Children ret{};
						if (columns.empty()) return ret;
						for (const auto &_: std::views::iota(0ull, columns.size() - 1)) {
							ret.emplace_back(Container{});
							ret.emplace_back(Box{
								.widget{
									.width = 1.f,
								},
								.color{1.f, 1.f, 1.f, .1f},
							});
						}
						ret.emplace_back(Container{});
						return ret;
					}(),
				},
				Column{
					.widget{
						.padding = Padding(40.f, 0.f, 0.f, 0.f),
					},
					.children = children,
				},
			},
		};
	}
};

struct Heading {
	// Args
	std::string_view text;

	operator squi::Child() const {
		return Text{
			.widget{
				.margin = 8.f,
			},
			.text = text,
			.fontSize = 20.f,
			.font = FontStore::defaultFontBold,
		};
	}
};

ResultsCard::operator squi::Child() const {
	return Card{
		.child = Column{
			.children{
				Heading{
					.text = title,
				},
				Table{
					.columns = columns,
					.children = children,
				},
			},
		},
	};
}
