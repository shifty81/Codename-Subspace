#include "trading/EconomyAgentSystem.h"

#include <algorithm>
#include <numeric>

namespace subspace {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

const char* NPCAgentTypeName(NPCAgentType type) {
    switch (type) {
        case NPCAgentType::Miner:    return "Miner";
        case NPCAgentType::Trader:   return "Trader";
        case NPCAgentType::Hauler:   return "Hauler";
        case NPCAgentType::Producer: return "Producer";
        default:                     return "Unknown";
    }
}

// ---------------------------------------------------------------------------
// NPCEconomicAgentComponent
// ---------------------------------------------------------------------------

int NPCEconomicAgentComponent::CurrentCargoTotal() const {
    int total = 0;
    for (const auto& [res, qty] : cargo) total += qty;
    return total;
}

bool NPCEconomicAgentComponent::CanAddCargo(int amount) const {
    return (CurrentCargoTotal() + amount) <= maxCargo;
}

void NPCEconomicAgentComponent::AddCargo(const std::string& resource, int amount) {
    cargo[resource] += amount;
}

bool NPCEconomicAgentComponent::RemoveCargo(const std::string& resource, int amount) {
    auto it = cargo.find(resource);
    if (it == cargo.end() || it->second < amount) return false;
    it->second -= amount;
    if (it->second == 0) cargo.erase(it);
    return true;
}

ComponentData NPCEconomicAgentComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "NPCEconomicAgentComponent";
    cd.data["type"]     = std::to_string(static_cast<int>(agentType));
    cd.data["credits"]  = std::to_string(credits);
    cd.data["maxCargo"] = std::to_string(maxCargo);
    cd.data["eff"]      = std::to_string(efficiency);
    cd.data["interval"] = std::to_string(actionIntervalSec);
    cd.data["timer"]    = std::to_string(timeUntilNextAction);
    cd.data["res"]      = targetResource;
    cd.data["cargoN"]   = std::to_string(cargo.size());
    size_t i = 0;
    for (const auto& [res, qty] : cargo) {
        cd.data["cr" + std::to_string(i)] = res;
        cd.data["cq" + std::to_string(i)] = std::to_string(qty);
        ++i;
    }
    return cd;
}

void NPCEconomicAgentComponent::Deserialize(const ComponentData& data) {
    auto f = [&](const std::string& k, const std::string& def = "") {
        auto it = data.data.find(k);
        return (it != data.data.end()) ? it->second : def;
    };
    agentType    = static_cast<NPCAgentType>(std::stoi(f("type", "1")));
    credits      = std::stoi(f("credits", "10000"));
    maxCargo     = std::stoi(f("maxCargo", "1000"));
    efficiency   = std::stof(f("eff", "1.0"));
    actionIntervalSec     = std::stof(f("interval", "60"));
    timeUntilNextAction   = std::stof(f("timer", "0"));
    targetResource        = f("res");
    int n = std::stoi(f("cargoN", "0"));
    cargo.clear();
    for (int i = 0; i < n; ++i) {
        std::string res = f("cr" + std::to_string(i));
        int qty = std::stoi(f("cq" + std::to_string(i), "0"));
        if (!res.empty()) cargo[res] = qty;
    }
}

// ---------------------------------------------------------------------------
// NPCEconomicAgentSystem
// ---------------------------------------------------------------------------

NPCEconomicAgentSystem::NPCEconomicAgentSystem()
    : SystemBase("NPCEconomicAgentSystem") {}
NPCEconomicAgentSystem::NPCEconomicAgentSystem(EntityManager& em)
    : SystemBase("NPCEconomicAgentSystem"), _em(&em) {}

void NPCEconomicAgentSystem::SetEntityManager(EntityManager* em) { _em = em; }

void NPCEconomicAgentSystem::Update(float deltaTime) {
    if (!_em) return;
    auto agents = _em->GetAllComponents<NPCEconomicAgentComponent>();
    for (auto* agent : agents) {
        if (!agent) continue;
        agent->timeUntilNextAction -= deltaTime;
        if (agent->timeUntilNextAction > 0.0f) continue;

        agent->timeUntilNextAction = agent->actionIntervalSec;
        switch (agent->agentType) {
            case NPCAgentType::Miner:    TickMiner(*agent, deltaTime);    break;
            case NPCAgentType::Trader:   TickTrader(*agent, deltaTime);   break;
            case NPCAgentType::Hauler:   TickHauler(*agent, deltaTime);   break;
            case NPCAgentType::Producer: TickProducer(*agent, deltaTime); break;
        }
    }
}

void NPCEconomicAgentSystem::PopulateAgents(int targetCount) {
    if (!_em) return;
    // Simple population: create agents up to target count
    auto existing = _em->GetAllComponents<NPCEconomicAgentComponent>();
    int toCreate = targetCount - static_cast<int>(existing.size());
    if (toCreate <= 0) return;

    static const NPCAgentType types[] = {
        NPCAgentType::Miner, NPCAgentType::Trader,
        NPCAgentType::Hauler, NPCAgentType::Producer
    };

    for (int i = 0; i < toCreate; ++i) {
        auto& e = _em->CreateEntity("NPC_Agent_" + std::to_string(i));
        auto comp = std::make_unique<NPCEconomicAgentComponent>();
        comp->agentType = types[i % 4];
        comp->credits   = 5000 + (i % 10) * 1000;
        comp->efficiency = 0.8f + (i % 5) * 0.1f;
        _em->AddComponent<NPCEconomicAgentComponent>(e.id, std::move(comp));
    }
}

void NPCEconomicAgentSystem::TickMiner(NPCEconomicAgentComponent& agent,
                                        float /*dt*/) {
    static const char* ores[] = { "Iron", "Titanium", "Naonite", "Trinium" };
    std::string ore = agent.targetResource.empty()
                    ? ores[agent.credits % 4]
                    : agent.targetResource;
    int mined = static_cast<int>(10.0f * agent.efficiency);
    if (agent.CanAddCargo(mined)) agent.AddCargo(ore, mined);
}

void NPCEconomicAgentSystem::TickTrader(NPCEconomicAgentComponent& agent,
                                         float /*dt*/) {
    // Sell all held cargo for credits
    int sold = 0;
    for (auto& [res, qty] : agent.cargo) sold += qty;
    agent.credits += sold * 10;
    agent.cargo.clear();
}

void NPCEconomicAgentSystem::TickHauler(NPCEconomicAgentComponent& agent,
                                         float /*dt*/) {
    // Simulate hauling: clear cargo and gain credits per unit hauled
    int units = agent.CurrentCargoTotal();
    agent.credits += units * 5;
    agent.cargo.clear();
}

void NPCEconomicAgentSystem::TickProducer(NPCEconomicAgentComponent& agent,
                                           float /*dt*/) {
    // Consume iron, produce components
    if (agent.RemoveCargo("Iron", 5))
        agent.AddCargo("Components", static_cast<int>(2 * agent.efficiency));
}

// ---------------------------------------------------------------------------
// ManufacturingRecipe
// ---------------------------------------------------------------------------

std::vector<ManufacturingRecipe> ManufacturingRecipe::GetDefaultRecipes() {
    return {
        { 1, "Iron",       10, "Steel",       8,  5.0f,  1 },
        { 2, "Titanium",   5,  "Alloy",        4,  8.0f,  2 },
        { 3, "Steel",      6,  "Components",  3,  12.0f, 2 },
        { 4, "Naonite",    4,  "Crystals",    2,  20.0f, 3 },
        { 5, "Components", 4,  "Electronics", 2,  25.0f, 3 },
        { 6, "Trinium",    3,  "Conductor",   2,  30.0f, 4 },
    };
}

// ---------------------------------------------------------------------------
// ManufacturingComponent
// ---------------------------------------------------------------------------

bool ManufacturingComponent::StartJob(const ManufacturingRecipe& recipe,
                                       int batchSize) {
    if (static_cast<int>(jobs.size()) >= maxJobs) return false;
    if (recipe.requiredFacilityLevel > facilityLevel) return false;

    ManufacturingJob job;
    job.recipeId  = _nextJobId++;
    job.recipe    = recipe;
    job.batchSize = batchSize;
    job.state     = ManufacturingState::Loading;
    jobs.push_back(job);
    return true;
}

bool ManufacturingComponent::CancelJob(int recipeId) {
    auto it = std::find_if(jobs.begin(), jobs.end(),
        [&](const ManufacturingJob& j){ return j.recipeId == recipeId; });
    if (it == jobs.end()) return false;
    jobs.erase(it);
    return true;
}

std::pair<std::string, int>
ManufacturingComponent::CollectJob(int recipeId) {
    for (auto it = jobs.begin(); it != jobs.end(); ++it) {
        if (it->recipeId == recipeId &&
            it->state == ManufacturingState::Completed) {
            auto result = std::make_pair(
                it->recipe.outputResource,
                it->recipe.outputAmount * it->batchSize);
            jobs.erase(it);
            return result;
        }
    }
    return { "", 0 };
}

int ManufacturingComponent::ActiveJobCount() const {
    int n = 0;
    for (const auto& j : jobs)
        if (j.state == ManufacturingState::Processing ||
            j.state == ManufacturingState::Loading) ++n;
    return n;
}

int ManufacturingComponent::CompletedJobCount() const {
    int n = 0;
    for (const auto& j : jobs)
        if (j.state == ManufacturingState::Completed) ++n;
    return n;
}

ComponentData ManufacturingComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "ManufacturingComponent";
    cd.data["level"] = std::to_string(facilityLevel);
    cd.data["maxJobs"] = std::to_string(maxJobs);
    cd.data["speed"]   = std::to_string(speedMultiplier);
    cd.data["jobs"]    = std::to_string(jobs.size());
    for (size_t i = 0; i < jobs.size(); ++i) {
        std::string p = "j" + std::to_string(i) + "_";
        cd.data[p + "rid"]     = std::to_string(jobs[i].recipeId);
        cd.data[p + "state"]   = std::to_string(static_cast<int>(jobs[i].state));
        cd.data[p + "prog"]    = std::to_string(jobs[i].progress);
        cd.data[p + "batch"]   = std::to_string(jobs[i].batchSize);
        cd.data[p + "out"]     = jobs[i].recipe.outputResource;
        cd.data[p + "outAmt"]  = std::to_string(jobs[i].recipe.outputAmount);
    }
    return cd;
}

void ManufacturingComponent::Deserialize(const ComponentData& data) {
    auto f = [&](const std::string& k, const std::string& def = "") {
        auto it = data.data.find(k);
        return (it != data.data.end()) ? it->second : def;
    };
    facilityLevel    = std::stoi(f("level", "1"));
    maxJobs          = std::stoi(f("maxJobs", "3"));
    speedMultiplier  = std::stof(f("speed", "1.0"));
    int n = std::stoi(f("jobs", "0"));
    jobs.clear();
    for (int i = 0; i < n; ++i) {
        std::string p = "j" + std::to_string(i) + "_";
        ManufacturingJob job;
        job.recipeId  = std::stoi(f(p + "rid", "0"));
        job.state     = static_cast<ManufacturingState>(
                            std::stoi(f(p + "state", "0")));
        job.progress  = std::stof(f(p + "prog", "0"));
        job.batchSize = std::stoi(f(p + "batch", "1"));
        job.recipe.outputResource = f(p + "out");
        job.recipe.outputAmount   = std::stoi(f(p + "outAmt", "1"));
        jobs.push_back(job);
    }
}

// ---------------------------------------------------------------------------
// ManufacturingSystem
// ---------------------------------------------------------------------------

ManufacturingSystem::ManufacturingSystem()
    : SystemBase("ManufacturingSystem") {}
ManufacturingSystem::ManufacturingSystem(EntityManager& em)
    : SystemBase("ManufacturingSystem"), _em(&em) {}

void ManufacturingSystem::SetEntityManager(EntityManager* em) { _em = em; }

void ManufacturingSystem::Update(float deltaTime) {
    if (!_em) return;
    auto comps = _em->GetAllComponents<ManufacturingComponent>();
    for (auto* comp : comps) {
        if (!comp) continue;
        for (auto& job : comp->jobs)
            AdvanceJob(job, deltaTime, comp->speedMultiplier);
    }
}

void ManufacturingSystem::AdvanceJob(ManufacturingJob& job, float dt,
                                      float speedMult) const {
    if (job.state == ManufacturingState::Loading)
        job.state = ManufacturingState::Processing;

    if (job.state != ManufacturingState::Processing) return;

    float tick = dt * speedMult / (job.recipe.productionTimeSec * job.batchSize);
    job.progress += tick;
    job.elapsed  += dt;

    if (job.progress >= 1.0f) {
        job.progress = 1.0f;
        job.state    = ManufacturingState::Completed;
    }
}

} // namespace subspace
