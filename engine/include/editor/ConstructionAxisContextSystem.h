#pragma once

#include <cstddef>

namespace subspace {

// Context-neutral authoring axes.  Ship-specific words such as port/starboard
// are presentation aliases only and must never determine transform math.
enum class ConstructionAxis : int { X = 0, Y = 1, Z = 2 };

enum class ConstructionAuthoringContext : int {
    Generic = 0,
    Ship,
    Station,
    Vehicle,
    Prop,
    Interior
};

struct ConstructionAxisDescriptor {
    const char* token = "X";
    const char* dimension = "WIDTH";
    const char* negative = "-X";
    const char* positive = "+X";
};

class ConstructionAxisContextSystem {
public:
    static constexpr ConstructionAxisDescriptor Describe(ConstructionAxis axis) noexcept {
        switch(axis){
        case ConstructionAxis::X:return {"X","WIDTH","-X","+X"};
        case ConstructionAxis::Y:return {"Y","LENGTH","-Y","+Y"};
        case ConstructionAxis::Z:return {"Z","HEIGHT","-Z","+Z"};
        }
        return {"X","WIDTH","-X","+X"};
    }

    static constexpr const char* ContextName(ConstructionAuthoringContext context) noexcept {
        switch(context){
        case ConstructionAuthoringContext::Ship:return "SHIP";
        case ConstructionAuthoringContext::Station:return "STATION";
        case ConstructionAuthoringContext::Vehicle:return "VEHICLE";
        case ConstructionAuthoringContext::Prop:return "PROP";
        case ConstructionAuthoringContext::Interior:return "INTERIOR";
        default:return "OBJECT";
        }
    }

    static constexpr ConstructionAxis FromIndex(std::size_t index) noexcept {
        return index==1?ConstructionAxis::Y:(index==2?ConstructionAxis::Z:ConstructionAxis::X);
    }

    static constexpr int Index(ConstructionAxis axis) noexcept {
        return static_cast<int>(axis);
    }
};

} // namespace subspace
