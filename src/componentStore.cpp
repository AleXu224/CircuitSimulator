#include "components/componentStore.hpp"
#include "components/resistor.hpp"
#include "components/voltageSource.hpp"

const std::vector<std::reference_wrapper<const Component>> ComponentStore::components{
    resistor,
    voltageSource,
};