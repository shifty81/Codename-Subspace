#pragma once

#include "core/ecs/Entity.h"

namespace subspace {

/// Base interface for all ECS components.
struct IComponent {
    EntityId entityId = InvalidEntityId;
    virtual ~IComponent() = default;
};

} // namespace subspace
