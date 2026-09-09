#include "navigation/WormholeSystem.h"

#include <algorithm>
#include <random>

namespace subspace {

// ---------------------------------------------------------------------------
// WormholeLink
// ---------------------------------------------------------------------------

std::string WormholeLink::GetTypeName(WormholeType type) {
    switch (type) {
        case WormholeType::Natural:    return "Natural";
        case WormholeType::Artificial: return "Artificial";
        case WormholeType::Unstable:   return "Unstable";
        case WormholeType::Persistent: return "Persistent";
    }
    return "Unknown";
}

std::string WormholeLink::GetStateName(WormholeState state) {
    switch (state) {
        case WormholeState::Dormant:        return "Dormant";
        case WormholeState::Activating:     return "Activating";
        case WormholeState::Active:         return "Active";
        case WormholeState::Destabilizing:  return "Destabilizing";
        case WormholeState::Collapsed:      return "Collapsed";
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// WormholeComponent
// ---------------------------------------------------------------------------

WormholeComponent::WormholeComponent(int maxLinks)
    : _maxLinks(maxLinks)
{
}

void WormholeComponent::AddLink(const WormholeLink& link) {
    if (static_cast<int>(_links.size()) >= _maxLinks) return;
    _links.push_back(link);
}

const WormholeLink* WormholeComponent::GetLink(int linkId) const {
    for (const auto& link : _links) {
        if (link.linkId == linkId) return &link;
    }
    return nullptr;
}

WormholeLink* WormholeComponent::GetLink(int linkId) {
    for (auto& link : _links) {
        if (link.linkId == linkId) return &link;
    }
    return nullptr;
}

std::vector<const WormholeLink*> WormholeComponent::GetActiveLinks() const {
    std::vector<const WormholeLink*> result;
    for (const auto& link : _links) {
        if (link.state == WormholeState::Active) {
            result.push_back(&link);
        }
    }
    return result;
}

std::vector<const WormholeLink*> WormholeComponent::GetLinksToSector(int sectorX, int sectorY) const {
    std::vector<const WormholeLink*> result;
    for (const auto& link : _links) {
        if ((link.endpointA.sectorX == sectorX && link.endpointA.sectorY == sectorY) ||
            (link.endpointB.sectorX == sectorX && link.endpointB.sectorY == sectorY)) {
            result.push_back(&link);
        }
    }
    return result;
}

const WormholeLink* WormholeComponent::FindLink(int fromSectorX, int fromSectorY,
                                                  int toSectorX, int toSectorY) const {
    for (const auto& link : _links) {
        bool aMatchesFrom = link.endpointA.sectorX == fromSectorX &&
                            link.endpointA.sectorY == fromSectorY;
        bool bMatchesTo   = link.endpointB.sectorX == toSectorX &&
                            link.endpointB.sectorY == toSectorY;
        bool bMatchesFrom = link.endpointB.sectorX == fromSectorX &&
                            link.endpointB.sectorY == fromSectorY;
        bool aMatchesTo   = link.endpointA.sectorX == toSectorX &&
                            link.endpointA.sectorY == toSectorY;

        if ((aMatchesFrom && bMatchesTo) || (bMatchesFrom && aMatchesTo)) {
            return &link;
        }
    }
    return nullptr;
}

bool WormholeComponent::RequestTraversal(int linkId, EntityId shipId, float shipMass) {
    WormholeLink* link = GetLink(linkId);
    if (!link) return false;
    if (link->state != WormholeState::Active) return false;
    if (link->stability <= 0.1f) return false;
    if (link->currentMass + shipMass > link->maxMass) return false;

    link->currentMass += shipMass;
    _activeTraversals.emplace_back(linkId, shipId);
    return true;
}

bool WormholeComponent::CompleteTraversal(int linkId, EntityId shipId, float shipMass) {
    auto it = std::find_if(_activeTraversals.begin(), _activeTraversals.end(),
        [linkId, shipId](const std::pair<int, EntityId>& t) {
            return t.first == linkId && t.second == shipId;
        });

    if (it == _activeTraversals.end()) return false;

    WormholeLink* link = GetLink(linkId);
    if (link) {
        link->currentMass -= shipMass;
        if (link->currentMass < 0.0f) link->currentMass = 0.0f;
    }

    _activeTraversals.erase(it);
    return true;
}

float WormholeComponent::GetStability(int linkId) const {
    const WormholeLink* link = GetLink(linkId);
    return link ? link->stability : 0.0f;
}

int WormholeComponent::GetLinkCount() const {
    return static_cast<int>(_links.size());
}

int WormholeComponent::GetMaxLinks() const {
    return _maxLinks;
}

const std::vector<WormholeLink>& WormholeComponent::GetAllLinks() const {
    return _links;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData WormholeComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "WormholeComponent";
    cd.data["maxLinks"]           = std::to_string(_maxLinks);
    cd.data["stabilityDecayRate"] = std::to_string(_stabilityDecayRate);
    cd.data["stabilityRegenRate"] = std::to_string(_stabilityRegenRate);

    // Links
    cd.data["linkCount"] = std::to_string(_links.size());
    for (size_t i = 0; i < _links.size(); ++i) {
        std::string prefix = "link_" + std::to_string(i) + "_";
        const auto& l = _links[i];
        cd.data[prefix + "linkId"]       = std::to_string(l.linkId);
        cd.data[prefix + "type"]         = std::to_string(static_cast<int>(l.type));
        cd.data[prefix + "state"]        = std::to_string(static_cast<int>(l.state));
        cd.data[prefix + "stability"]    = std::to_string(l.stability);
        cd.data[prefix + "maxMass"]      = std::to_string(l.maxMass);
        cd.data[prefix + "currentMass"]  = std::to_string(l.currentMass);
        cd.data[prefix + "traversalTime"] = std::to_string(l.traversalTime);
        cd.data[prefix + "bidirectional"] = l.bidirectional ? "1" : "0";

        // Endpoint A
        cd.data[prefix + "endpointA_sectorX"] = std::to_string(l.endpointA.sectorX);
        cd.data[prefix + "endpointA_sectorY"] = std::to_string(l.endpointA.sectorY);
        cd.data[prefix + "endpointA_posX"]    = std::to_string(l.endpointA.posX);
        cd.data[prefix + "endpointA_posY"]    = std::to_string(l.endpointA.posY);
        cd.data[prefix + "endpointA_posZ"]    = std::to_string(l.endpointA.posZ);
        cd.data[prefix + "endpointA_name"]    = l.endpointA.name;

        // Endpoint B
        cd.data[prefix + "endpointB_sectorX"] = std::to_string(l.endpointB.sectorX);
        cd.data[prefix + "endpointB_sectorY"] = std::to_string(l.endpointB.sectorY);
        cd.data[prefix + "endpointB_posX"]    = std::to_string(l.endpointB.posX);
        cd.data[prefix + "endpointB_posY"]    = std::to_string(l.endpointB.posY);
        cd.data[prefix + "endpointB_posZ"]    = std::to_string(l.endpointB.posZ);
        cd.data[prefix + "endpointB_name"]    = l.endpointB.name;
    }

    // Active traversals
    cd.data["traversalCount"] = std::to_string(_activeTraversals.size());
    for (size_t i = 0; i < _activeTraversals.size(); ++i) {
        std::string prefix = "trav_" + std::to_string(i) + "_";
        cd.data[prefix + "linkId"] = std::to_string(_activeTraversals[i].first);
        cd.data[prefix + "shipId"] = std::to_string(_activeTraversals[i].second);
    }

    return cd;
}

void WormholeComponent::Deserialize(const ComponentData& data) {
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

    _maxLinks           = getInt("maxLinks", 8);
    _stabilityDecayRate = getFloat("stabilityDecayRate", 0.01f);
    _stabilityRegenRate = getFloat("stabilityRegenRate", 0.005f);

    // Links
    int linkCount = getInt("linkCount", 0);
    _links.clear();
    _links.reserve(static_cast<size_t>(linkCount));
    for (int i = 0; i < linkCount; ++i) {
        std::string prefix = "link_" + std::to_string(i) + "_";
        WormholeLink l;
        l.linkId        = getInt(prefix + "linkId", 0);

        int typeVal = getInt(prefix + "type", 0);
        constexpr int kMaxType = static_cast<int>(WormholeType::Persistent);
        if (typeVal >= 0 && typeVal <= kMaxType) {
            l.type = static_cast<WormholeType>(typeVal);
        } else {
            l.type = WormholeType::Natural;
        }

        int stateVal = getInt(prefix + "state", 0);
        constexpr int kMaxState = static_cast<int>(WormholeState::Collapsed);
        if (stateVal >= 0 && stateVal <= kMaxState) {
            l.state = static_cast<WormholeState>(stateVal);
        } else {
            l.state = WormholeState::Dormant;
        }

        l.stability     = getFloat(prefix + "stability", 1.0f);
        l.maxMass       = getFloat(prefix + "maxMass", 10000.0f);
        l.currentMass   = getFloat(prefix + "currentMass", 0.0f);
        l.traversalTime = getFloat(prefix + "traversalTime", 5.0f);
        l.bidirectional = getStr(prefix + "bidirectional") != "0";

        // Endpoint A
        l.endpointA.sectorX = getInt(prefix + "endpointA_sectorX", 0);
        l.endpointA.sectorY = getInt(prefix + "endpointA_sectorY", 0);
        l.endpointA.posX    = getFloat(prefix + "endpointA_posX", 0.0f);
        l.endpointA.posY    = getFloat(prefix + "endpointA_posY", 0.0f);
        l.endpointA.posZ    = getFloat(prefix + "endpointA_posZ", 0.0f);
        l.endpointA.name    = getStr(prefix + "endpointA_name");

        // Endpoint B
        l.endpointB.sectorX = getInt(prefix + "endpointB_sectorX", 0);
        l.endpointB.sectorY = getInt(prefix + "endpointB_sectorY", 0);
        l.endpointB.posX    = getFloat(prefix + "endpointB_posX", 0.0f);
        l.endpointB.posY    = getFloat(prefix + "endpointB_posY", 0.0f);
        l.endpointB.posZ    = getFloat(prefix + "endpointB_posZ", 0.0f);
        l.endpointB.name    = getStr(prefix + "endpointB_name");

        _links.push_back(l);
    }

    // Active traversals
    int traversalCount = getInt("traversalCount", 0);
    _activeTraversals.clear();
    _activeTraversals.reserve(static_cast<size_t>(traversalCount));
    for (int i = 0; i < traversalCount; ++i) {
        std::string prefix = "trav_" + std::to_string(i) + "_";
        int lid       = getInt(prefix + "linkId", 0);
        uint64_t sid  = getUint64(prefix + "shipId", 0);
        _activeTraversals.emplace_back(lid, static_cast<EntityId>(sid));
    }
}

// ---------------------------------------------------------------------------
// WormholeSystem
// ---------------------------------------------------------------------------

WormholeSystem::WormholeSystem() : SystemBase("WormholeSystem") {}

WormholeSystem::WormholeSystem(EntityManager& entityManager)
    : SystemBase("WormholeSystem")
    , _entityManager(&entityManager)
{
}

void WormholeSystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

void WormholeSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto components = _entityManager->GetAllComponents<WormholeComponent>();
    for (auto* wormhole : components) {
        for (auto& link : wormhole->_links) {
            switch (link.state) {
                case WormholeState::Active: {
                    // Regenerate stability up to 1.0
                    link.stability += wormhole->_stabilityRegenRate * deltaTime;
                    if (link.stability > 1.0f) link.stability = 1.0f;

                    // Collapse if stability depleted
                    if (link.stability <= 0.0f) {
                        link.stability = 0.0f;
                        link.state = WormholeState::Destabilizing;
                    }
                    break;
                }

                case WormholeState::Activating: {
                    link.stability += wormhole->_stabilityRegenRate * deltaTime;
                    if (link.stability >= 0.5f) {
                        link.state = WormholeState::Active;
                    }
                    break;
                }

                case WormholeState::Destabilizing: {
                    link.stability -= wormhole->_stabilityDecayRate * deltaTime;
                    if (link.stability <= 0.0f) {
                        link.stability = 0.0f;
                        link.state = WormholeState::Collapsed;
                    }
                    break;
                }

                default:
                    break;
            }

            // Unstable wormholes have random small stability fluctuations (±0.001)
            if (link.type == WormholeType::Unstable &&
                link.state != WormholeState::Collapsed &&
                link.state != WormholeState::Dormant) {
                constexpr float kMaxFluctuation = 0.001f;
                thread_local std::mt19937 rng{std::random_device{}()};
                std::uniform_real_distribution<float> dist(-kMaxFluctuation,
                                                           kMaxFluctuation);
                link.stability += dist(rng);
                if (link.stability < 0.0f) link.stability = 0.0f;
                if (link.stability > 1.0f) link.stability = 1.0f;
            }
        }
    }
}

} // namespace subspace
