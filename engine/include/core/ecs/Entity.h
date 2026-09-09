#pragma once

#include <cstdint>
#include <string>

namespace subspace {

/// Unique entity identifier.
using EntityId = uint64_t;
constexpr EntityId InvalidEntityId = 0;

/// Represents a game entity with a unique identifier.
struct Entity {
    EntityId id = InvalidEntityId;
    std::string name;
    bool isActive = true;
};

} // namespace subspace
