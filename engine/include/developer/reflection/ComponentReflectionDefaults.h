#pragma once

#include "developer/reflection/ComponentReflectionRegistry.h"

#include <vector>

namespace subspace {

class ComponentReflectionDefaults {
public:
    static std::vector<ReflectedComponentType> BuildCommonComponentTypes();
    static void RegisterCommonComponentTypes(ComponentReflectionRegistry& registry);
};

} // namespace subspace
