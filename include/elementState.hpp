#pragma once

#include "observer.hpp"
enum class ElementState {
    selected,
    unselected,
    removed,
};

using StateObservable = squi::Observable<ElementState>;
