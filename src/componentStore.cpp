#include "components/componentStore.hpp"
#include "components/resistor.hpp"
#include "components/voltageSource.hpp"
#include "components/currentSource.hpp"
#include "components/conductor.hpp"
#include "components/node.hpp"
#include "components/capacitor.hpp"
#include "components/inductor.hpp"
#include "components/ground.hpp"

const std::vector<std::reference_wrapper<const Component>> ComponentStore::components{
    conductor,
    node,
    voltageSource,
    currentSource,
    resistor,
    ground,
    capacitor,
    inductor,
};