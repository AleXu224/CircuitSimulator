#pragma once

#include "boardBackgroundQuad.hpp"
#include "boardStorage.hpp"
#include "component.hpp"
#include "gestureDetector.hpp"
#include "observer.hpp"
#include "pipeline.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include <functional>
#include <optional>
#include <unordered_set>


struct BoardView {
	// Args
	squi::Widget::Args widget;
	squi::VoidObservable onRun;
	using Quad = Engine::BoardBackgroundQuad;
	using Pipeline = Engine::Pipeline<Quad::Vertex>;

	static Pipeline *pipeline;
	class Impl : public squi::Widget {
		// Data
		squi::GestureDetector::State &gd;
		Quad quad;
		squi::vec2 viewOffset{};
		bool loadedComponents = false;
		using ComponentObservable = squi::Observable<std::reference_wrapper<const Component>>;

	public:
		BoardStorage boardStorage{};
		ComponentObservable componentSelectorObservable{};
		squi::Observable<const squi::Child &> resultsAdder{};
		squi::Observable<const std::vector<ElementId>&> elementSelector{};

	private:
		squi::VoidObservable resultsDestroyer{};
		ComponentObservable::Observer observer;
		std::optional<squi::Child> selectedComponentWidget{};
		std::optional<squi::Child> selectedLineWidget{};
		std::optional<squi::Child> selectionWidget{};
		std::unordered_set<ElementId> selectedWidgets{};
		std::vector<squi::Child> nodeIndexes{};
		static constexpr float gridWidth = 20.f;

		void onUpdate() override;
		void postArrange(squi::vec2 &pos) override;
		void postLayout(squi::vec2 &size) override;
		void onDraw() override;

		void updateChildren() override;
		squi::vec2 layoutChildren(squi::vec2 maxSize, squi::vec2 minSize, ShouldShrink shouldShrink) override;
		void arrangeChildren(squi::vec2 &pos) override;
		void drawChildren() override;

		void clickElement(squi::GestureDetector::Event);
		void unselectAll();
		void deleteSelected();
		void clearNodeIndexes();
		void hideResults();

	public:
		Impl(const BoardView &args);
		~Impl() override;
		Impl(const Impl &) = delete;
		Impl(Impl &&) = delete;
		Impl &operator=(const Impl &) = delete;
		Impl &operator=(Impl &&) = delete;
	};

	operator squi::Child() const;
};