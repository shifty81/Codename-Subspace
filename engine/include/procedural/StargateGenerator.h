#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace subspace {

// ---------------------------------------------------------------------------
// StargateType
// ---------------------------------------------------------------------------

enum class StargateType {
    JumpGate,        ///< Local transit gate (sector-to-sector)
    WarpGate,        ///< Medium-range inter-system gate
    HyperspaceRelay  ///< Long-range galaxy-spanning relay
};

// ---------------------------------------------------------------------------
// StargateData
// ---------------------------------------------------------------------------

struct StargateData {
    std::string gateId;
    StargateType type           = StargateType::JumpGate;
    std::string  systemName;      ///< Name of the star system this gate serves
    float        posX = 0, posY = 0, posZ = 0;  ///< Position in-system (AU)
    float        orbitRadius = 1.0f;             ///< Orbital radius (AU)
    std::string  destinationGateId;
    std::string  destinationSystem;
    float        maxShipMassKg = 1.0e9f;
    bool         isActive = true;
    float        transitTimeSec = 30.0f;
    std::string  faction = "Neutral";
    bool         requiresDocking = false;
};

// ---------------------------------------------------------------------------
// StargateNetwork
// ---------------------------------------------------------------------------

/// Lookup table of all stargates in the galaxy.
struct StargateNetwork {
    std::vector<StargateData> gates;

    /// Find a gate by id. Returns nullptr if not found.
    const StargateData* FindGate(const std::string& id) const;

    /// Return all gates connected to a system.
    std::vector<const StargateData*> GatesInSystem(
        const std::string& system) const;

    /// Add a bidirectional link between two gates.
    void LinkGates(const std::string& idA, const std::string& idB);
};

// ---------------------------------------------------------------------------
// StargateGenerator
// ---------------------------------------------------------------------------

/// Procedurally places and connects stargates in a galaxy network.
class StargateGenerator {
public:
    explicit StargateGenerator(int galaxySeed = 0);

    /// Generate stargates for a star system and append to the network.
    void GenerateForSystem(const std::string& systemName,
                           float systemPosX, float systemPosY,
                           StargateNetwork& network) const;

    /// Build a spine of jump gates connecting a list of system names in order.
    /// Useful for trade-route generation.
    void BuildTradeSpine(const std::vector<std::string>& systemNames,
                         StargateNetwork& network) const;

    /// Base probability that a system gets a jump gate (default 0.6).
    float jumpGateProbability    = 0.60f;

    /// Probability that a jump gate is also a warp gate (default 0.2).
    float warpGateProbability    = 0.20f;

    /// Probability of a hyperspace relay at large hubs (default 0.05).
    float hyperspaceRelayProb    = 0.05f;

private:
    int _galaxySeed;
    mutable int _nextGateSerial = 1;

    int HashSystem(const std::string& name) const;
    std::string MakeGateId(const std::string& system, int serial) const;
};

} // namespace subspace
