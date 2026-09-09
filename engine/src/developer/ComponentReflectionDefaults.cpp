#include "developer/reflection/ComponentReflectionDefaults.h"

namespace subspace {

std::vector<ReflectedComponentType> ComponentReflectionDefaults::BuildCommonComponentTypes() {
    return {
        ReflectedComponentType{
            "Transform",
            "Transform",
            {
                {"position.x", "float", "Position X", true},
                {"position.y", "float", "Position Y", true},
                {"position.z", "float", "Position Z", true},
                {"rotation.x", "float", "Rotation X", true},
                {"rotation.y", "float", "Rotation Y", true},
                {"rotation.z", "float", "Rotation Z", true},
                {"scale.x", "float", "Scale X", true},
                {"scale.y", "float", "Scale Y", true},
                {"scale.z", "float", "Scale Z", true},
            }
        },
        ReflectedComponentType{
            "Identity",
            "Identity / Metadata",
            {
                {"name", "string", "Name", true},
                {"kind", "string", "Kind", false},
                {"sourceAsset", "asset_ref", "Source Asset", false},
                {"runtimeOnly", "bool", "Runtime Only", false},
            }
        },
        ReflectedComponentType{
            "ShipRuntimeState",
            "Ship Runtime State",
            {
                {"shipId", "string", "Ship ID", false},
                {"blueprintId", "asset_ref", "Blueprint", false},
                {"dirty", "bool", "Dirty", false},
                {"needsRenderRebuild", "bool", "Needs Render Rebuild", false},
                {"needsCollisionRebuild", "bool", "Needs Collision Rebuild", false},
                {"mass", "float", "Mass", false},
                {"integrity", "float", "Integrity", false},
            }
        },
        ReflectedComponentType{
            "PhysicsDebug",
            "Physics Debug",
            {
                {"velocity.x", "float", "Velocity X", false},
                {"velocity.y", "float", "Velocity Y", false},
                {"velocity.z", "float", "Velocity Z", false},
                {"bounds.min", "vec3", "Bounds Min", false},
                {"bounds.max", "vec3", "Bounds Max", false},
                {"collisionEnabled", "bool", "Collision Enabled", true},
            }
        },
        ReflectedComponentType{
            "InventorySummary",
            "Inventory Summary",
            {
                {"slotsUsed", "int", "Slots Used", false},
                {"capacity", "int", "Capacity", false},
                {"massUsed", "float", "Mass Used", false},
            }
        }
    };
}

void ComponentReflectionDefaults::RegisterCommonComponentTypes(ComponentReflectionRegistry& registry) {
    for (auto type : BuildCommonComponentTypes()) {
        registry.RegisterComponent(std::move(type));
    }
}

} // namespace subspace
