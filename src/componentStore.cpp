#include "components/componentStore.hpp"
#include "components/resistor.hpp"
#include "components/voltageSource.hpp"
#include "components/conductor.hpp"
#include "components/node.hpp"
#include "components/capacitor.hpp"
#include "components/inductor.hpp"

const std::vector<std::reference_wrapper<const Component>> ComponentStore::components{
    resistor,
    voltageSource,
    conductor,
    node,
    capacitor,
    inductor,
};