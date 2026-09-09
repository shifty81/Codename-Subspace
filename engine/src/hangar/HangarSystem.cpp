#include "hangar/HangarSystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// DockingBay
// ---------------------------------------------------------------------------

std::string DockingBay::GetSizeName(BaySize size) {
    switch (size) {
        case BaySize::Small:   return "Small";
        case BaySize::Medium:  return "Medium";
        case BaySize::Large:   return "Large";
        case BaySize::Capital: return "Capital";
    }
    return "Unknown";
}

std::string DockingBay::GetStateName(DockingState state) {
    switch (state) {
        case DockingState::Undocked:       return "Undocked";
        case DockingState::RequestingDock: return "Requesting Dock";
        case DockingState::Approaching:    return "Approaching";
        case DockingState::Docking:        return "Docking";
        case DockingState::Docked:         return "Docked";
        case DockingState::Undocking:      return "Undocking";
        case DockingState::Launching:      return "Launching";
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// HangarComponent
// ---------------------------------------------------------------------------

HangarComponent::HangarComponent(int maxBays)
    : _maxBays(maxBays)
{
}

int HangarComponent::GetMaxBays() const {
    return _maxBays;
}

int HangarComponent::GetOccupiedBayCount() const {
    int count = 0;
    for (const auto& bay : _bays) {
        if (bay.isOccupied) ++count;
    }
    return count;
}

int HangarComponent::GetFreeBayCount() const {
    int count = 0;
    for (const auto& bay : _bays) {
        if (!bay.isOccupied && bay.isEnabled) ++count;
    }
    return count;
}

void HangarComponent::AddBay(const DockingBay& bay) {
    if (static_cast<int>(_bays.size()) >= _maxBays) return;
    _bays.push_back(bay);
}

const DockingBay* HangarComponent::GetBay(int bayId) const {
    for (const auto& bay : _bays) {
        if (bay.bayId == bayId) return &bay;
    }
    return nullptr;
}

DockingBay* HangarComponent::GetBay(int bayId) {
    for (auto& bay : _bays) {
        if (bay.bayId == bayId) return &bay;
    }
    return nullptr;
}

std::vector<const DockingBay*> HangarComponent::GetFreeBays() const {
    std::vector<const DockingBay*> result;
    for (const auto& bay : _bays) {
        if (!bay.isOccupied && bay.isEnabled) {
            result.push_back(&bay);
        }
    }
    return result;
}

std::vector<const DockingBay*> HangarComponent::GetFreeBaysBySize(BaySize minSize) const {
    std::vector<const DockingBay*> result;
    for (const auto& bay : _bays) {
        if (!bay.isOccupied && bay.isEnabled &&
            static_cast<int>(bay.size) >= static_cast<int>(minSize)) {
            result.push_back(&bay);
        }
    }
    return result;
}

bool HangarComponent::RequestDocking(EntityId shipId, BaySize requiredSize) {
    // Check if ship already has an active request
    for (const auto& req : _activeRequests) {
        if (req.shipId == shipId) return false;
    }

    // Find smallest available bay that fits (Small < Medium < Large < Capital)
    DockingBay* bestBay = nullptr;
    for (auto& bay : _bays) {
        if (!bay.isOccupied && bay.isEnabled &&
            static_cast<int>(bay.size) >= static_cast<int>(requiredSize)) {
            if (!bestBay || static_cast<int>(bay.size) < static_cast<int>(bestBay->size)) {
                bestBay = &bay;
            }
        }
    }

    if (!bestBay) return false;

    DockingRequest request;
    request.shipId = shipId;
    request.assignedBayId = bestBay->bayId;
    request.requestTime = 0.0f;
    request.state = DockingState::Approaching;
    request.approachProgress = 0.0f;
    request.dockingProgress = 0.0f;

    // Mark bay as occupied so no other ship can claim it
    bestBay->isOccupied = true;
    bestBay->dockedShipId = shipId;

    _activeRequests.push_back(request);
    return true;
}

bool HangarComponent::CancelDocking(EntityId shipId) {
    auto it = std::find_if(_activeRequests.begin(), _activeRequests.end(),
        [shipId](const DockingRequest& req) { return req.shipId == shipId; });

    if (it == _activeRequests.end()) return false;

    // Free the assigned bay
    DockingBay* bay = GetBay(it->assignedBayId);
    if (bay) {
        bay->isOccupied = false;
        bay->dockedShipId = 0;
    }

    _activeRequests.erase(it);
    return true;
}

bool HangarComponent::IsShipDocked(EntityId shipId) const {
    for (const auto& req : _activeRequests) {
        if (req.shipId == shipId && req.state == DockingState::Docked) {
            return true;
        }
    }
    return false;
}

DockingState HangarComponent::GetShipDockingState(EntityId shipId) const {
    for (const auto& req : _activeRequests) {
        if (req.shipId == shipId) return req.state;
    }
    return DockingState::Undocked;
}

const DockingRequest* HangarComponent::GetDockingRequest(EntityId shipId) const {
    for (const auto& req : _activeRequests) {
        if (req.shipId == shipId) return &req;
    }
    return nullptr;
}

bool HangarComponent::RequestLaunch(EntityId shipId) {
    auto it = std::find_if(_activeRequests.begin(), _activeRequests.end(),
        [shipId](const DockingRequest& req) { return req.shipId == shipId; });

    if (it == _activeRequests.end()) return false;
    if (it->state != DockingState::Docked) return false;

    it->state = DockingState::Undocking;
    it->dockingProgress = 1.0f;
    it->approachProgress = 1.0f;
    return true;
}

void HangarComponent::StoreShip(EntityId shipId) {
    // Avoid duplicates
    auto it = std::find(_storedShips.begin(), _storedShips.end(), shipId);
    if (it == _storedShips.end()) {
        _storedShips.push_back(shipId);
    }
}

bool HangarComponent::RetrieveShip(EntityId shipId) {
    auto it = std::find(_storedShips.begin(), _storedShips.end(), shipId);
    if (it == _storedShips.end()) return false;
    _storedShips.erase(it);
    return true;
}

const std::vector<EntityId>& HangarComponent::GetStoredShips() const {
    return _storedShips;
}

int HangarComponent::GetStoredShipCount() const {
    return static_cast<int>(_storedShips.size());
}

const std::vector<DockingBay>& HangarComponent::GetAllBays() const {
    return _bays;
}

const std::vector<DockingRequest>& HangarComponent::GetActiveRequests() const {
    return _activeRequests;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData HangarComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "HangarComponent";
    cd.data["maxBays"]       = std::to_string(_maxBays);
    cd.data["approachSpeed"] = std::to_string(_approachSpeed);
    cd.data["dockingSpeed"]  = std::to_string(_dockingSpeed);
    cd.data["launchSpeed"]   = std::to_string(_launchSpeed);

    // Bays
    cd.data["bayCount"] = std::to_string(_bays.size());
    for (size_t i = 0; i < _bays.size(); ++i) {
        std::string prefix = "bay_" + std::to_string(i) + "_";
        const auto& b = _bays[i];
        cd.data[prefix + "bayId"]        = std::to_string(b.bayId);
        cd.data[prefix + "bayName"]      = b.bayName;
        cd.data[prefix + "size"]         = std::to_string(static_cast<int>(b.size));
        cd.data[prefix + "isOccupied"]   = b.isOccupied ? "1" : "0";
        cd.data[prefix + "dockedShipId"] = std::to_string(b.dockedShipId);
        cd.data[prefix + "repairRate"]   = std::to_string(b.repairRate);
        cd.data[prefix + "refuelRate"]   = std::to_string(b.refuelRate);
        cd.data[prefix + "isEnabled"]    = b.isEnabled ? "1" : "0";
    }

    // Active requests
    cd.data["requestCount"] = std::to_string(_activeRequests.size());
    for (size_t i = 0; i < _activeRequests.size(); ++i) {
        std::string prefix = "req_" + std::to_string(i) + "_";
        const auto& r = _activeRequests[i];
        cd.data[prefix + "shipId"]           = std::to_string(r.shipId);
        cd.data[prefix + "assignedBayId"]    = std::to_string(r.assignedBayId);
        cd.data[prefix + "requestTime"]      = std::to_string(r.requestTime);
        cd.data[prefix + "state"]            = std::to_string(static_cast<int>(r.state));
        cd.data[prefix + "approachProgress"] = std::to_string(r.approachProgress);
        cd.data[prefix + "dockingProgress"]  = std::to_string(r.dockingProgress);
    }

    // Stored ships
    cd.data["storedShipCount"] = std::to_string(_storedShips.size());
    for (size_t i = 0; i < _storedShips.size(); ++i) {
        cd.data["stored_" + std::to_string(i)] = std::to_string(_storedShips[i]);
    }

    return cd;
}

void HangarComponent::Deserialize(const ComponentData& data) {
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

    _maxBays       = getInt("maxBays", 4);
    _approachSpeed = getFloat("approachSpeed", 2.0f);
    _dockingSpeed  = getFloat("dockingSpeed", 3.0f);
    _launchSpeed   = getFloat("launchSpeed", 2.0f);

    // Bays
    int bayCount = getInt("bayCount", 0);
    _bays.clear();
    _bays.reserve(static_cast<size_t>(bayCount));
    for (int i = 0; i < bayCount; ++i) {
        std::string prefix = "bay_" + std::to_string(i) + "_";
        DockingBay b;
        b.bayId        = getInt(prefix + "bayId", 0);
        b.bayName      = getStr(prefix + "bayName");
        int sizeVal    = getInt(prefix + "size", 1);
        constexpr int kMaxBaySize = static_cast<int>(BaySize::Capital);
        if (sizeVal >= 0 && sizeVal <= kMaxBaySize) {
            b.size = static_cast<BaySize>(sizeVal);
        } else {
            b.size = BaySize::Medium;
        }
        b.isOccupied   = getStr(prefix + "isOccupied") != "0";
        b.dockedShipId = getUint64(prefix + "dockedShipId", 0);
        b.repairRate   = getFloat(prefix + "repairRate", 5.0f);
        b.refuelRate   = getFloat(prefix + "refuelRate", 10.0f);
        b.isEnabled    = getStr(prefix + "isEnabled") != "0";
        _bays.push_back(b);
    }

    // Active requests
    int requestCount = getInt("requestCount", 0);
    _activeRequests.clear();
    _activeRequests.reserve(static_cast<size_t>(requestCount));
    for (int i = 0; i < requestCount; ++i) {
        std::string prefix = "req_" + std::to_string(i) + "_";
        DockingRequest r;
        r.shipId           = getUint64(prefix + "shipId", 0);
        r.assignedBayId    = getInt(prefix + "assignedBayId", -1);
        r.requestTime      = getFloat(prefix + "requestTime", 0.0f);
        int stateVal       = getInt(prefix + "state", 0);
        constexpr int kMaxState = static_cast<int>(DockingState::Launching);
        if (stateVal >= 0 && stateVal <= kMaxState) {
            r.state = static_cast<DockingState>(stateVal);
        } else {
            r.state = DockingState::Undocked;
        }
        r.approachProgress = getFloat(prefix + "approachProgress", 0.0f);
        r.dockingProgress  = getFloat(prefix + "dockingProgress", 0.0f);
        _activeRequests.push_back(r);
    }

    // Stored ships
    int storedCount = getInt("storedShipCount", 0);
    _storedShips.clear();
    _storedShips.reserve(static_cast<size_t>(storedCount));
    for (int i = 0; i < storedCount; ++i) {
        uint64_t sid = getUint64("stored_" + std::to_string(i), 0);
        _storedShips.push_back(sid);
    }
}

// ---------------------------------------------------------------------------
// HangarSystem
// ---------------------------------------------------------------------------

HangarSystem::HangarSystem() : SystemBase("HangarSystem") {}

HangarSystem::HangarSystem(EntityManager& entityManager)
    : SystemBase("HangarSystem")
    , _entityManager(&entityManager)
{
}

void HangarSystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

void HangarSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto hangars = _entityManager->GetAllComponents<HangarComponent>();
    for (auto* hangar : hangars) {
        auto it = hangar->_activeRequests.begin();
        while (it != hangar->_activeRequests.end()) {
            bool removeRequest = false;

            switch (it->state) {
                case DockingState::Approaching: {
                    it->approachProgress += deltaTime / hangar->_approachSpeed;
                    if (it->approachProgress >= 1.0f) {
                        it->approachProgress = 1.0f;
                        it->state = DockingState::Docking;
                        it->dockingProgress = 0.0f;
                    }
                    break;
                }

                case DockingState::Docking: {
                    it->dockingProgress += deltaTime / hangar->_dockingSpeed;
                    if (it->dockingProgress >= 1.0f) {
                        it->dockingProgress = 1.0f;
                        it->state = DockingState::Docked;
                        DockingBay* bay = hangar->GetBay(it->assignedBayId);
                        if (bay) {
                            bay->isOccupied = true;
                            bay->dockedShipId = it->shipId;
                        }
                    }
                    break;
                }

                case DockingState::Undocking: {
                    it->dockingProgress -= deltaTime / hangar->_dockingSpeed;
                    if (it->dockingProgress <= 0.0f) {
                        it->dockingProgress = 0.0f;
                        it->state = DockingState::Launching;
                    }
                    break;
                }

                case DockingState::Launching: {
                    it->approachProgress -= deltaTime / hangar->_launchSpeed;
                    if (it->approachProgress <= 0.0f) {
                        it->approachProgress = 0.0f;
                        it->state = DockingState::Undocked;
                        DockingBay* bay = hangar->GetBay(it->assignedBayId);
                        if (bay) {
                            bay->isOccupied = false;
                            bay->dockedShipId = 0;
                        }
                        removeRequest = true;
                    }
                    break;
                }

                default:
                    break;
            }

            if (removeRequest) {
                it = hangar->_activeRequests.erase(it);
            } else {
                ++it;
            }
        }
    }
}

} // namespace subspace
