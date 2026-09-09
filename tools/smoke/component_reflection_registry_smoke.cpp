#include "developer/reflection/ComponentReflectionRegistry.h"

#include <cassert>
#include <iostream>

int main()
{
    subspace::ComponentReflectionRegistry registry;
    std::string stored = "0";
    registry.RegisterComponent({"Transform", "Transform", {{"x", "float", "X", true}}},
        [&](std::uint64_t, const std::string&) { return stored; },
        [&](std::uint64_t, const std::string&, const std::string& value, std::string& message) {
            stored = value;
            message = "ok";
            return true;
        });
    std::string message;
    assert(registry.SetField(1, "Transform", "x", "12", message));
    assert(registry.GetField(1, "Transform", "x") == "12");
    std::cout << "ComponentReflectionRegistry smoke test passed.\n";
}
