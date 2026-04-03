#include "procedural/StargateGenerator.h"

#include <algorithm>
#include <functional>
#include <sstream>

namespace subspace {

// ---------------------------------------------------------------------------
// StargateNetwork
// ---------------------------------------------------------------------------

const StargateData* StargateNetwork::FindGate(const std::string& id) const {
    for (const auto& g : gates)
        if (g.gateId == id) return &g;
    return nullptr;
}

std::vector<const StargateData*>
StargateNetwork::GatesInSystem(const std::string& system) const {
    std::vector<const StargateData*> result;
    for (const auto& g : gates)
        if (g.systemName == system) result.push_back(&g);
    return result;
}

void StargateNetwork::LinkGates(const std::string& idA, const std::string& idB) {
    auto findMut = [&](const std::string& id) -> StargateData* {
        for (auto& g : gates)
            if (g.gateId == id) return &g;
        return nullptr;
    };
    auto* a = findMut(idA);
    auto* b = findMut(idB);
    if (a && b) {
        a->destinationGateId  = idB;
        a->destinationSystem  = b->systemName;
        b->destinationGateId  = idA;
        b->destinationSystem  = a->systemName;
    }
}

// ---------------------------------------------------------------------------
// StargateGenerator
// ---------------------------------------------------------------------------

StargateGenerator::StargateGenerator(int seed)
    : _galaxySeed(seed == 0 ? 42 : seed) {}

void StargateGenerator::GenerateForSystem(const std::string& sysName,
                                           float posX, float posY,
                                           StargateNetwork& net) const {
    int h = HashSystem(sysName);
    std::mt19937 rng(static_cast<uint32_t>(h));
    std::uniform_real_distribution<float> prob(0.0f, 1.0f);

    if (prob(rng) > jumpGateProbability) return; // no gate this system

    StargateData gate;
    gate.gateId     = MakeGateId(sysName, _nextGateSerial++);
    gate.systemName = sysName;
    gate.posX       = posX;
    gate.posY       = posY;
    gate.posZ       = 0.0f;
    gate.orbitRadius = 1.0f + prob(rng) * 2.0f;

    if (prob(rng) < hyperspaceRelayProb)
        gate.type = StargateType::HyperspaceRelay;
    else if (prob(rng) < warpGateProbability)
        gate.type = StargateType::WarpGate;
    else
        gate.type = StargateType::JumpGate;

    gate.maxShipMassKg = 1.0e9f;
    gate.transitTimeSec = 30.0f + prob(rng) * 60.0f;
    gate.isActive   = true;

    net.gates.push_back(gate);
}

void StargateGenerator::BuildTradeSpine(const std::vector<std::string>& systems,
                                         StargateNetwork& net) const {
    if (systems.size() < 2) return;

    // Ensure each system in the spine has a gate
    float posX = 0.0f, posY = 0.0f;
    for (const auto& sys : systems) {
        // Check if already generated
        auto existing = net.GatesInSystem(sys);
        if (existing.empty()) {
            GenerateForSystem(sys, posX, posY, net);
        }
        posX += 1.0f; // Offset for next system
    }

    // Link consecutive systems
    for (size_t i = 0; i + 1 < systems.size(); ++i) {
        auto gatesA = net.GatesInSystem(systems[i]);
        auto gatesB = net.GatesInSystem(systems[i + 1]);
        if (!gatesA.empty() && !gatesB.empty())
            net.LinkGates(gatesA[0]->gateId, gatesB[0]->gateId);
    }
}

int StargateGenerator::HashSystem(const std::string& name) const {
    std::hash<std::string> h;
    return static_cast<int>(h(name)) ^ _galaxySeed;
}

std::string StargateGenerator::MakeGateId(const std::string& system,
                                            int serial) const {
    return system + "_gate_" + std::to_string(serial);
}

} // namespace subspace
