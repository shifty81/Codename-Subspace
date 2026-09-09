#include "trade_route/TradeRouteSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// TradeWaypoint
// ---------------------------------------------------------------------------

std::string TradeWaypoint::GetStateName(TradeRouteState state) {
    switch (state) {
        case TradeRouteState::Idle:            return "Idle";
        case TradeRouteState::Traveling:       return "Traveling";
        case TradeRouteState::Buying:          return "Buying";
        case TradeRouteState::Selling:         return "Selling";
        case TradeRouteState::WaitingForCargo: return "Waiting for Cargo";
        case TradeRouteState::Completed:       return "Completed";
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// TradeRoute
// ---------------------------------------------------------------------------

float TradeRoute::CalculateDistance() const {
    float dist = 0.0f;
    for (size_t i = 1; i < waypoints.size(); ++i) {
        float dx = waypoints[i].x - waypoints[i - 1].x;
        float dy = waypoints[i].y - waypoints[i - 1].y;
        float dz = waypoints[i].z - waypoints[i - 1].z;
        dist += std::sqrt(dx * dx + dy * dy + dz * dz);
    }
    return dist;
}

bool TradeRoute::IsValid() const {
    return waypoints.size() >= 2;
}

// ---------------------------------------------------------------------------
// TradeRouteComponent
// ---------------------------------------------------------------------------

void TradeRouteComponent::SetRoute(const TradeRoute& route) {
    _route = route;
    _route.totalDistance = _route.CalculateDistance();
}

const TradeRoute& TradeRouteComponent::GetRoute() const {
    return _route;
}

TradeRouteState TradeRouteComponent::GetState() const {
    return _state;
}

int TradeRouteComponent::GetCurrentWaypointIndex() const {
    return _currentWaypointIndex;
}

const TradeWaypoint* TradeRouteComponent::GetCurrentWaypoint() const {
    if (_route.waypoints.empty()) return nullptr;
    if (_currentWaypointIndex < 0 ||
        _currentWaypointIndex >= static_cast<int>(_route.waypoints.size())) {
        return nullptr;
    }
    return &_route.waypoints[_currentWaypointIndex];
}

const TradeWaypoint* TradeRouteComponent::GetNextWaypoint() const {
    if (_route.waypoints.empty()) return nullptr;
    int nextIndex = _currentWaypointIndex + 1;
    if (nextIndex >= static_cast<int>(_route.waypoints.size())) {
        if (_route.isLoop) {
            nextIndex = 0;
        } else {
            return nullptr;
        }
    }
    return &_route.waypoints[nextIndex];
}

float TradeRouteComponent::GetTravelProgress() const {
    return _travelProgress;
}

float TradeRouteComponent::GetTravelSpeed() const {
    return _travelSpeed;
}

void TradeRouteComponent::SetTravelSpeed(float speed) {
    _travelSpeed = speed;
}

bool TradeRouteComponent::IsActive() const {
    return _isActive;
}

void TradeRouteComponent::StartRoute() {
    if (!_route.IsValid()) return;
    _isActive = true;
    _state = TradeRouteState::Traveling;
    _currentWaypointIndex = 0;
    _travelProgress = 0.0f;
    _currentRunProfit = 0.0f;
}

void TradeRouteComponent::StopRoute() {
    _isActive = false;
    _state = TradeRouteState::Idle;
    _currentWaypointIndex = 0;
    _travelProgress = 0.0f;
    _currentRunProfit = 0.0f;
    _waitTimer = 0.0f;
}

void TradeRouteComponent::PauseRoute() {
    _isActive = false;
    _state = TradeRouteState::Idle;
}

bool TradeRouteComponent::AdvanceToNextWaypoint() {
    int nextIndex = _currentWaypointIndex + 1;
    if (nextIndex >= static_cast<int>(_route.waypoints.size())) {
        if (_route.isLoop) {
            _currentWaypointIndex = 0;
            _route.completedRuns++;
            _route.totalProfit += _currentRunProfit;
            _currentRunProfit = 0.0f;
            _travelProgress = 0.0f;
            return true;
        }
        _route.completedRuns++;
        _route.totalProfit += _currentRunProfit;
        _currentRunProfit = 0.0f;
        return false;
    }
    _currentWaypointIndex = nextIndex;
    _travelProgress = 0.0f;
    return true;
}

float TradeRouteComponent::GetCurrentRunProfit() const {
    return _currentRunProfit;
}

void TradeRouteComponent::AddProfit(float amount) {
    _currentRunProfit += amount;
}

const std::vector<std::string>& TradeRouteComponent::GetCargoManifest() const {
    return _cargoManifest;
}

void TradeRouteComponent::AddCargo(const std::string& goodId) {
    _cargoManifest.push_back(goodId);
}

void TradeRouteComponent::RemoveCargo(const std::string& goodId) {
    auto it = std::find(_cargoManifest.begin(), _cargoManifest.end(), goodId);
    if (it != _cargoManifest.end()) {
        _cargoManifest.erase(it);
    }
}

void TradeRouteComponent::ClearCargo() {
    _cargoManifest.clear();
}

int TradeRouteComponent::GetTotalCompletedRuns() const {
    return _route.completedRuns;
}

float TradeRouteComponent::GetTotalProfit() const {
    return _route.totalProfit;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData TradeRouteComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "TradeRouteComponent";

    // Route metadata
    cd.data["routeId"]        = _route.routeId;
    cd.data["routeName"]      = _route.routeName;
    cd.data["isLoop"]         = _route.isLoop ? "1" : "0";
    cd.data["totalDistance"]   = std::to_string(_route.totalDistance);
    cd.data["completedRuns"]  = std::to_string(_route.completedRuns);
    cd.data["totalProfit"]    = std::to_string(_route.totalProfit);

    // Waypoints
    cd.data["waypointCount"] = std::to_string(_route.waypoints.size());
    for (size_t i = 0; i < _route.waypoints.size(); ++i) {
        std::string prefix = "wp" + std::to_string(i) + "_";
        const auto& wp = _route.waypoints[i];
        cd.data[prefix + "stationId"]   = wp.stationId;
        cd.data[prefix + "stationName"] = wp.stationName;
        cd.data[prefix + "x"]           = std::to_string(wp.x);
        cd.data[prefix + "y"]           = std::to_string(wp.y);
        cd.data[prefix + "z"]           = std::to_string(wp.z);
        cd.data[prefix + "waitTime"]    = std::to_string(wp.waitTime);

        cd.data[prefix + "buyCount"] = std::to_string(wp.buyGoods.size());
        for (size_t b = 0; b < wp.buyGoods.size(); ++b) {
            cd.data[prefix + "buy" + std::to_string(b)] = wp.buyGoods[b];
        }

        cd.data[prefix + "sellCount"] = std::to_string(wp.sellGoods.size());
        for (size_t s = 0; s < wp.sellGoods.size(); ++s) {
            cd.data[prefix + "sell" + std::to_string(s)] = wp.sellGoods[s];
        }
    }

    // Component state
    cd.data["state"]                = std::to_string(static_cast<int>(_state));
    cd.data["currentWaypointIndex"] = std::to_string(_currentWaypointIndex);
    cd.data["waitTimer"]            = std::to_string(_waitTimer);
    cd.data["travelProgress"]       = std::to_string(_travelProgress);
    cd.data["travelSpeed"]          = std::to_string(_travelSpeed);
    cd.data["isActive"]             = _isActive ? "1" : "0";
    cd.data["currentRunProfit"]     = std::to_string(_currentRunProfit);

    // Cargo manifest
    cd.data["cargoCount"] = std::to_string(_cargoManifest.size());
    for (size_t i = 0; i < _cargoManifest.size(); ++i) {
        cd.data["cargo" + std::to_string(i)] = _cargoManifest[i];
    }

    return cd;
}

void TradeRouteComponent::Deserialize(const ComponentData& data) {
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

    // Route metadata
    _route.routeId       = getStr("routeId");
    _route.routeName     = getStr("routeName");
    _route.isLoop        = getStr("isLoop") != "0";
    _route.totalDistance  = getFloat("totalDistance", 0.0f);
    _route.completedRuns = getInt("completedRuns", 0);
    _route.totalProfit   = getFloat("totalProfit", 0.0f);

    // Waypoints
    int wpCount = getInt("waypointCount", 0);
    _route.waypoints.clear();
    for (int i = 0; i < wpCount; ++i) {
        std::string prefix = "wp" + std::to_string(i) + "_";
        TradeWaypoint wp;
        wp.stationId   = getStr(prefix + "stationId");
        wp.stationName = getStr(prefix + "stationName");
        wp.x           = getFloat(prefix + "x", 0.0f);
        wp.y           = getFloat(prefix + "y", 0.0f);
        wp.z           = getFloat(prefix + "z", 0.0f);
        wp.waitTime    = getFloat(prefix + "waitTime", 5.0f);

        int buyCount = getInt(prefix + "buyCount", 0);
        for (int b = 0; b < buyCount; ++b) {
            std::string good = getStr(prefix + "buy" + std::to_string(b));
            if (!good.empty()) wp.buyGoods.push_back(good);
        }

        int sellCount = getInt(prefix + "sellCount", 0);
        for (int s = 0; s < sellCount; ++s) {
            std::string good = getStr(prefix + "sell" + std::to_string(s));
            if (!good.empty()) wp.sellGoods.push_back(good);
        }

        _route.waypoints.push_back(wp);
    }

    // Component state
    constexpr int kMaxState = static_cast<int>(TradeRouteState::Completed);
    int stateVal = getInt("state", 0);
    if (stateVal >= 0 && stateVal <= kMaxState) {
        _state = static_cast<TradeRouteState>(stateVal);
    } else {
        _state = TradeRouteState::Idle;
    }

    _currentWaypointIndex = getInt("currentWaypointIndex", 0);
    _waitTimer            = getFloat("waitTimer", 0.0f);
    _travelProgress       = getFloat("travelProgress", 0.0f);
    _travelSpeed          = getFloat("travelSpeed", 10.0f);
    _isActive             = getStr("isActive") != "0";
    _currentRunProfit     = getFloat("currentRunProfit", 0.0f);

    // Cargo manifest
    int cargoCount = getInt("cargoCount", 0);
    _cargoManifest.clear();
    for (int i = 0; i < cargoCount; ++i) {
        std::string good = getStr("cargo" + std::to_string(i));
        if (!good.empty()) _cargoManifest.push_back(good);
    }
}

// ---------------------------------------------------------------------------
// TradeRouteSystem
// ---------------------------------------------------------------------------

TradeRouteSystem::TradeRouteSystem() : SystemBase("TradeRouteSystem") {}

TradeRouteSystem::TradeRouteSystem(EntityManager& entityManager)
    : SystemBase("TradeRouteSystem")
    , _entityManager(&entityManager)
{
}

void TradeRouteSystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

void TradeRouteSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto components = _entityManager->GetAllComponents<TradeRouteComponent>();
    for (auto* comp : components) {
        if (!comp->IsActive()) continue;

        switch (comp->GetState()) {
            case TradeRouteState::Traveling: {
                const TradeWaypoint* current = comp->GetCurrentWaypoint();
                const TradeWaypoint* next = comp->GetNextWaypoint();
                if (!current || !next) {
                    comp->StopRoute();
                    break;
                }

                float dx = next->x - current->x;
                float dy = next->y - current->y;
                float dz = next->z - current->z;
                float segmentDist = std::sqrt(dx * dx + dy * dy + dz * dz);

                if (segmentDist > 0.0f) {
                    float progressDelta = (comp->GetTravelSpeed() * deltaTime) / segmentDist;
                    comp->_travelProgress += progressDelta;
                }

                if (comp->_travelProgress >= 1.0f) {
                    comp->_travelProgress = 0.0f;
                    comp->_state = TradeRouteState::Buying;
                    comp->_waitTimer = next->waitTime;
                }
                break;
            }

            case TradeRouteState::Buying: {
                comp->_waitTimer -= deltaTime;
                if (comp->_waitTimer <= 0.0f) {
                    comp->_waitTimer = 0.0f;
                    const TradeWaypoint* next = comp->GetNextWaypoint();
                    comp->_state = TradeRouteState::Selling;
                    comp->_waitTimer = next ? next->waitTime : 0.0f;
                }
                break;
            }

            case TradeRouteState::Selling: {
                comp->_waitTimer -= deltaTime;
                if (comp->_waitTimer <= 0.0f) {
                    comp->_waitTimer = 0.0f;
                    comp->_state = TradeRouteState::WaitingForCargo;
                }
                break;
            }

            case TradeRouteState::WaitingForCargo: {
                if (comp->AdvanceToNextWaypoint()) {
                    comp->_state = TradeRouteState::Traveling;
                } else {
                    comp->_state = TradeRouteState::Completed;
                    comp->_isActive = false;
                }
                break;
            }

            case TradeRouteState::Idle:
            case TradeRouteState::Completed:
                break;
        }
    }
}

} // namespace subspace
