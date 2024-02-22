#pragma once

#include "observer.hpp"
#include <memory>
enum class ElementState {
    placed,
    placing,
    selected,
    unselected,
    removed,
};

using StateObservable = std::shared_ptr<squi::Observable<ElementState>>;