#pragma once

#include "boardBackgroundQuad.hpp"
#include "boardStorage.hpp"
#include "component.hpp"
#include "gestureDetector.hpp"
#include "observer.hpp"
#include "pipeline.hpp"
#include "widget.hpp"
#include <functional>
#include <optional>


struct BoardView {
	// Args
	squi::Widget::Args widget;
	using Quad = Engine::BoardBackgroundQuad;
	using Pipeline = Engine::Pipeline<Quad::Vertex>;

	static Pipeline *pipeline;
	class Impl : public squi::Widget {
		// Data
		BoardStorage boardStorage{};
		squi::GestureDetector::State &gd;
		Quad quad;
		squi::vec2 viewOffset{};
		bool loadedComponents = false;
		using ComponentObservable = squi::Observable<std::reference_wrapper<const Component>>;

	public:
		std::shared_ptr<ComponentObservable> componentSelectorObservable = ComponentObservable::create();

	private:
		std::shared_ptr<ComponentObservable::Observer> observer;
		std::optional<std::reference_wrapper<const Component>> selectedComponent{};
		std::optional<squi::Child> selectedComponentWidget{};
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
		~Impl() override;
		Impl(const Impl &) = delete;
		Impl(Impl &&) = delete;
		Impl &operator=(const Impl &) = delete;
		Impl &operator=(Impl &&) = delete;
	};

	operator squi::Child() const;
};