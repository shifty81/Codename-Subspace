#include "waypoint/WaypointSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// Waypoint helpers
// ---------------------------------------------------------------------------

float Waypoint::DistanceTo(float x, float y, float z) const {
    float dx = posX - x;
    float dy = posY - y;
    float dz = posZ - z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool Waypoint::IsExpired() const {
    if (expiryTime < 0.0f) return false; // permanent
    return elapsedTime >= expiryTime;
}

std::string Waypoint::GetTypeName(WaypointType type) {
    switch (type) {
        case WaypointType::Generic:        return "Generic";
        case WaypointType::Navigation:     return "Navigation";
        case WaypointType::PointOfInterest:return "Point of Interest";
        case WaypointType::Danger:         return "Danger";
        case WaypointType::Mining:         return "Mining";
        case WaypointType::Trading:        return "Trading";
        case WaypointType::Rally:          return "Rally";
        case WaypointType::Custom:         return "Custom";
    }
    return "Unknown";
}

std::string Waypoint::GetIconName(WaypointIcon icon) {
    switch (icon) {
        case WaypointIcon::Circle:    return "Circle";
        case WaypointIcon::Diamond:   return "Diamond";
        case WaypointIcon::Triangle:  return "Triangle";
        case WaypointIcon::Star:      return "Star";
        case WaypointIcon::Crosshair: return "Crosshair";
        case WaypointIcon::Flag:      return "Flag";
        case WaypointIcon::Anchor:    return "Anchor";
        case WaypointIcon::Skull:     return "Skull";
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// WaypointComponent
// ---------------------------------------------------------------------------

bool WaypointComponent::AddWaypoint(const Waypoint& wp) {
    if (_waypoints.size() >= kMaxWaypoints) return false;
    Waypoint added = wp;
    added.waypointId = _nextId++;
    _waypoints.push_back(added);
    return true;
}

bool WaypointComponent::RemoveWaypoint(uint64_t waypointId) {
    auto it = std::find_if(_waypoints.begin(), _waypoints.end(),
        [waypointId](const Waypoint& w) { return w.waypointId == waypointId; });
    if (it == _waypoints.end()) return false;
    _waypoints.erase(it);
    return true;
}

const Waypoint* WaypointComponent::GetWaypoint(uint64_t waypointId) const {
    for (const auto& w : _waypoints) {
        if (w.waypointId == waypointId) return &w;
    }
    return nullptr;
}

const std::vector<Waypoint>& WaypointComponent::GetAllWaypoints() const {
    return _waypoints;
}

std::vector<const Waypoint*> WaypointComponent::GetWaypointsByType(WaypointType type) const {
    std::vector<const Waypoint*> result;
    for (const auto& w : _waypoints) {
        if (w.type == type) result.push_back(&w);
    }
    return result;
}

std::vector<const Waypoint*> WaypointComponent::GetWaypointsInSector(uint64_t sectorId) const {
    std::vector<const Waypoint*> result;
    for (const auto& w : _waypoints) {
        if (w.sectorId == sectorId) result.push_back(&w);
    }
    return result;
}

const Waypoint* WaypointComponent::GetNearest(float x, float y, float z) const {
    const Waypoint* nearest = nullptr;
    float bestDist = 1e30f;
    for (const auto& w : _waypoints) {
        float d = w.DistanceTo(x, y, z);
        if (d < bestDist) {
            bestDist = d;
            nearest = &w;
        }
    }
    return nearest;
}

size_t WaypointComponent::GetCount() const {
    return _waypoints.size();
}

void WaypointComponent::ClearExpired() {
    _waypoints.erase(
        std::remove_if(_waypoints.begin(), _waypoints.end(),
            [](const Waypoint& w) { return w.IsExpired(); }),
        _waypoints.end());
}

void WaypointComponent::ClearAll() {
    _waypoints.clear();
}

bool WaypointComponent::ToggleVisibility(uint64_t waypointId) {
    auto* wp = FindMutable(waypointId);
    if (!wp) return false;
    wp->isVisible = !wp->isVisible;
    return true;
}

uint64_t WaypointComponent::GetNextId() const {
    return _nextId;
}

Waypoint* WaypointComponent::FindMutable(uint64_t waypointId) {
    for (auto& w : _waypoints) {
        if (w.waypointId == waypointId) return &w;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData WaypointComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "WaypointComponent";
    cd.data["count"] = std::to_string(_waypoints.size());
    cd.data["nextId"] = std::to_string(_nextId);

    for (size_t i = 0; i < _waypoints.size(); ++i) {
        std::string p = "wp_" + std::to_string(i) + "_";
        const auto& w = _waypoints[i];
        cd.data[p + "id"]         = std::to_string(w.waypointId);
        cd.data[p + "label"]      = w.label;
        cd.data[p + "type"]       = std::to_string(static_cast<int>(w.type));
        cd.data[p + "icon"]       = std::to_string(static_cast<int>(w.icon));
        cd.data[p + "posX"]       = std::to_string(w.posX);
        cd.data[p + "posY"]       = std::to_string(w.posY);
        cd.data[p + "posZ"]       = std::to_string(w.posZ);
        cd.data[p + "sectorId"]   = std::to_string(w.sectorId);
        cd.data[p + "visible"]    = w.isVisible ? "1" : "0";
        cd.data[p + "expiryTime"] = std::to_string(w.expiryTime);
        cd.data[p + "elapsed"]    = std::to_string(w.elapsedTime);
    }
    return cd;
}

void WaypointComponent::Deserialize(const ComponentData& data) {
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

    int count = getInt("count", 0);
    _nextId = getUint64("nextId", 1);
    _waypoints.clear();
    _waypoints.reserve(static_cast<size_t>(count));

    for (int i = 0; i < count; ++i) {
        std::string p = "wp_" + std::to_string(i) + "_";
        Waypoint w;
        w.waypointId = getUint64(p + "id", 0);
        w.label      = getStr(p + "label");

        int typeVal = getInt(p + "type", 0);
        constexpr int kMaxType = static_cast<int>(WaypointType::Custom);
        if (typeVal >= 0 && typeVal <= kMaxType)
            w.type = static_cast<WaypointType>(typeVal);

        int iconVal = getInt(p + "icon", 0);
        constexpr int kMaxIcon = static_cast<int>(WaypointIcon::Skull);
        if (iconVal >= 0 && iconVal <= kMaxIcon)
            w.icon = static_cast<WaypointIcon>(iconVal);

        w.posX       = getFloat(p + "posX", 0.0f);
        w.posY       = getFloat(p + "posY", 0.0f);
        w.posZ       = getFloat(p + "posZ", 0.0f);
        w.sectorId   = getUint64(p + "sectorId", 0);
        w.isVisible  = getInt(p + "visible", 1) != 0;
        w.expiryTime = getFloat(p + "expiryTime", -1.0f);
        w.elapsedTime= getFloat(p + "elapsed", 0.0f);
        _waypoints.push_back(w);
    }
}

// ---------------------------------------------------------------------------
// WaypointSystem
// ---------------------------------------------------------------------------

WaypointSystem::WaypointSystem() : SystemBase("WaypointSystem") {}

WaypointSystem::WaypointSystem(EntityManager& entityManager)
    : SystemBase("WaypointSystem")
    , _entityManager(&entityManager)
{
}

void WaypointSystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

void WaypointSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto components = _entityManager->GetAllComponents<WaypointComponent>();
    for (auto* wc : components) {
        for (auto& wp : wc->_waypoints) {
            if (wp.expiryTime >= 0.0f) {
                wp.elapsedTime += deltaTime;
            }
        }
        // Remove expired waypoints
        wc->ClearExpired();
    }
}

} // namespace subspace
