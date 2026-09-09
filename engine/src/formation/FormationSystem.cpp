#include "formation/FormationSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// FormationPattern
// ---------------------------------------------------------------------------

std::vector<FormationSlot> FormationPattern::ComputeSlots(int unitCount) const {
    switch (type) {
        case FormationType::Line:    return ComputeLineSlots(unitCount);
        case FormationType::V:       return ComputeVSlots(unitCount);
        case FormationType::Diamond: return ComputeDiamondSlots(unitCount);
        case FormationType::Circle:  return ComputeCircleSlots(unitCount);
        case FormationType::Wedge:   return ComputeWedgeSlots(unitCount);
        case FormationType::Column:  return ComputeColumnSlots(unitCount);
    }
    return ComputeLineSlots(unitCount);
}

std::vector<FormationSlot> FormationPattern::ComputeLineSlots(int unitCount) const {
    std::vector<FormationSlot> slots;
    slots.reserve(static_cast<size_t>(unitCount));
    float centerOffset = static_cast<float>(unitCount - 1) / 2.0f;
    for (int i = 0; i < unitCount; ++i) {
        FormationSlot slot;
        slot.slotIndex = i;
        slot.offset = Vector3((static_cast<float>(i) - centerOffset) * spacing, 0.0f, 0.0f);
        slots.push_back(slot);
    }
    return slots;
}

std::vector<FormationSlot> FormationPattern::ComputeVSlots(int unitCount) const {
    std::vector<FormationSlot> slots;
    slots.reserve(static_cast<size_t>(unitCount));

    // Slot 0: leader at origin
    FormationSlot leader;
    leader.slotIndex = 0;
    leader.offset = Vector3(0.0f, 0.0f, 0.0f);
    slots.push_back(leader);

    for (int i = 1; i < unitCount; ++i) {
        FormationSlot slot;
        slot.slotIndex = i;
        // Even indices on left (-X), odd on right (+X)
        // Each pair steps back along -Z and spreads along +/-X
        int pair = (i + 1) / 2;  // pair 1: slots 1,2; pair 2: slots 3,4; etc.
        bool leftSide = (i % 2 == 1);
        float x = static_cast<float>(pair) * spacing * (leftSide ? -1.0f : 1.0f);
        float z = -static_cast<float>(pair) * spacing;
        slot.offset = Vector3(x, 0.0f, z);
        slots.push_back(slot);
    }
    return slots;
}

std::vector<FormationSlot> FormationPattern::ComputeDiamondSlots(int unitCount) const {
    std::vector<FormationSlot> slots;
    slots.reserve(static_cast<size_t>(unitCount));

    // Slot 0: center
    {
        FormationSlot slot;
        slot.slotIndex = 0;
        slot.offset = Vector3(0.0f, 0.0f, 0.0f);
        slots.push_back(slot);
    }

    // Slot 1: front (+Z)
    if (unitCount > 1) {
        FormationSlot slot;
        slot.slotIndex = 1;
        slot.offset = Vector3(0.0f, 0.0f, spacing);
        slots.push_back(slot);
    }

    // Slot 2: left (-X)
    if (unitCount > 2) {
        FormationSlot slot;
        slot.slotIndex = 2;
        slot.offset = Vector3(-spacing, 0.0f, 0.0f);
        slots.push_back(slot);
    }

    // Slot 3: right (+X)
    if (unitCount > 3) {
        FormationSlot slot;
        slot.slotIndex = 3;
        slot.offset = Vector3(spacing, 0.0f, 0.0f);
        slots.push_back(slot);
    }

    // Slot 4: back (-Z)
    if (unitCount > 4) {
        FormationSlot slot;
        slot.slotIndex = 4;
        slot.offset = Vector3(0.0f, 0.0f, -spacing);
        slots.push_back(slot);
    }

    // Remaining slots wrap around in a ring at 1.5x spacing
    constexpr float kPi = 3.14159265358979323846f;
    int remaining = unitCount - 5;
    for (int i = 0; i < remaining; ++i) {
        float angle = (2.0f * kPi * static_cast<float>(i)) / static_cast<float>(remaining);
        FormationSlot slot;
        slot.slotIndex = 5 + i;
        slot.offset = Vector3(
            std::cos(angle) * spacing * 1.5f,
            0.0f,
            std::sin(angle) * spacing * 1.5f
        );
        slots.push_back(slot);
    }

    return slots;
}

std::vector<FormationSlot> FormationPattern::ComputeCircleSlots(int unitCount) const {
    std::vector<FormationSlot> slots;
    slots.reserve(static_cast<size_t>(unitCount));

    // Slot 0: center
    FormationSlot center;
    center.slotIndex = 0;
    center.offset = Vector3(0.0f, 0.0f, 0.0f);
    slots.push_back(center);

    // Remaining slots at equal angles around circle of radius = spacing
    constexpr float kPi = 3.14159265358979323846f;
    int ringCount = unitCount - 1;
    for (int i = 0; i < ringCount; ++i) {
        float angle = (2.0f * kPi * static_cast<float>(i)) / static_cast<float>(ringCount);
        FormationSlot slot;
        slot.slotIndex = 1 + i;
        slot.offset = Vector3(
            std::cos(angle) * spacing,
            0.0f,
            std::sin(angle) * spacing
        );
        slots.push_back(slot);
    }
    return slots;
}

std::vector<FormationSlot> FormationPattern::ComputeWedgeSlots(int unitCount) const {
    std::vector<FormationSlot> slots;
    slots.reserve(static_cast<size_t>(unitCount));

    // Slot 0: leader at origin
    FormationSlot leader;
    leader.slotIndex = 0;
    leader.offset = Vector3(0.0f, 0.0f, 0.0f);
    slots.push_back(leader);

    // Like V but more compact: half the lateral spread
    for (int i = 1; i < unitCount; ++i) {
        FormationSlot slot;
        slot.slotIndex = i;
        int pair = (i + 1) / 2;
        bool leftSide = (i % 2 == 1);
        float x = static_cast<float>(pair) * (spacing / 2.0f) * (leftSide ? -1.0f : 1.0f);
        float z = -static_cast<float>(pair) * spacing;
        slot.offset = Vector3(x, 0.0f, z);
        slots.push_back(slot);
    }
    return slots;
}

std::vector<FormationSlot> FormationPattern::ComputeColumnSlots(int unitCount) const {
    std::vector<FormationSlot> slots;
    slots.reserve(static_cast<size_t>(unitCount));
    for (int i = 0; i < unitCount; ++i) {
        FormationSlot slot;
        slot.slotIndex = i;
        slot.offset = Vector3(0.0f, 0.0f, -static_cast<float>(i) * spacing);
        slots.push_back(slot);
    }
    return slots;
}

std::string FormationPattern::GetFormationName(FormationType type) {
    switch (type) {
        case FormationType::Line:    return "Line";
        case FormationType::V:       return "V-Formation";
        case FormationType::Diamond: return "Diamond";
        case FormationType::Circle:  return "Circle";
        case FormationType::Wedge:   return "Wedge";
        case FormationType::Column:  return "Column";
    }
    return "Line";
}

int FormationPattern::GetMaxRecommendedSize(FormationType type) {
    switch (type) {
        case FormationType::Line:    return 10;
        case FormationType::V:       return 8;
        case FormationType::Diamond: return 9;
        case FormationType::Circle:  return 12;
        case FormationType::Wedge:   return 7;
        case FormationType::Column:  return 6;
    }
    return 10;
}

// ---------------------------------------------------------------------------
// FormationComponent
// ---------------------------------------------------------------------------

void FormationComponent::AddMember(EntityId id) {
    if (!HasMember(id)) {
        members.push_back(id);
    }
}

bool FormationComponent::RemoveMember(EntityId id) {
    auto it = std::find(members.begin(), members.end(), id);
    if (it != members.end()) {
        members.erase(it);
        return true;
    }
    return false;
}

int FormationComponent::GetMemberCount() const {
    return static_cast<int>(members.size());
}

bool FormationComponent::HasMember(EntityId id) const {
    return std::find(members.begin(), members.end(), id) != members.end();
}

void FormationComponent::ReassignSlots(const FormationPattern& pattern) {
    auto slots = pattern.ComputeSlots(static_cast<int>(members.size()));
    for (size_t i = 0; i < members.size() && i < slots.size(); ++i) {
        slots[i].assignedEntity = members[i];
    }
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData FormationComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "FormationComponent";
    cd.data["formationType"] = std::to_string(static_cast<int>(formationType));
    cd.data["spacing"]       = std::to_string(spacing);
    cd.data["leaderId"]      = std::to_string(leaderId);
    cd.data["slotIndex"]     = std::to_string(slotIndex);
    cd.data["isLeader"]      = isLeader ? "1" : "0";
    cd.data["targetOffset_x"] = std::to_string(targetOffset.x);
    cd.data["targetOffset_y"] = std::to_string(targetOffset.y);
    cd.data["targetOffset_z"] = std::to_string(targetOffset.z);
    cd.data["memberCount"]   = std::to_string(members.size());

    for (size_t i = 0; i < members.size(); ++i) {
        std::string key = "member_" + std::to_string(i);
        cd.data[key] = std::to_string(members[i]);
    }
    return cd;
}

void FormationComponent::Deserialize(const ComponentData& data) {
    auto getStr = [&](const std::string& key) -> std::string {
        auto it = data.data.find(key);
        return it != data.data.end() ? it->second : "";
    };
    auto getInt = [&](const std::string& key, int def = 0) -> int {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stoi(it->second); } catch (...) { return def; }
    };
    auto getFloat = [&](const std::string& key, float def = 0.0f) -> float {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stof(it->second); } catch (...) { return def; }
    };
    auto getUint64 = [&](const std::string& key, uint64_t def = 0) -> uint64_t {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stoull(it->second); } catch (...) { return def; }
    };

    int typeVal = getInt("formationType", 0);
    constexpr int kMaxFormationType = static_cast<int>(FormationType::Column);
    if (typeVal >= 0 && typeVal <= kMaxFormationType) {
        formationType = static_cast<FormationType>(typeVal);
    } else {
        formationType = FormationType::Line;
    }

    spacing  = getFloat("spacing", 10.0f);
    leaderId = static_cast<EntityId>(getUint64("leaderId", 0));
    slotIndex = getInt("slotIndex", -1);
    isLeader = getStr("isLeader") != "0";
    targetOffset.x = getFloat("targetOffset_x", 0.0f);
    targetOffset.y = getFloat("targetOffset_y", 0.0f);
    targetOffset.z = getFloat("targetOffset_z", 0.0f);

    int count = getInt("memberCount", 0);
    members.clear();
    members.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        std::string key = "member_" + std::to_string(i);
        members.push_back(static_cast<EntityId>(getUint64(key, 0)));
    }
}

// ---------------------------------------------------------------------------
// FormationSystem
// ---------------------------------------------------------------------------

FormationSystem::FormationSystem() : SystemBase("FormationSystem") {}

FormationSystem::FormationSystem(EntityManager& entityManager)
    : SystemBase("FormationSystem")
    , _entityManager(&entityManager)
{
}

void FormationSystem::Update(float /*deltaTime*/) {
    // Formation position updates are placeholder for future implementation.
    // This system exists for integration with the ECS update loop.
    if (!_entityManager) return;

    auto components = _entityManager->GetAllComponents<FormationComponent>();
    // Iterate to ensure system runs without crashing.
    for (auto* fc : components) {
        (void)fc;
    }
}

} // namespace subspace
