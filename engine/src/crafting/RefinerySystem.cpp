#include "crafting/RefinerySystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// RefineryRecipe
// ---------------------------------------------------------------------------

std::string RefineryRecipe::GetTierName(RefineryTier tier) {
    switch (tier) {
        case RefineryTier::Basic:        return "Basic";
        case RefineryTier::Advanced:     return "Advanced";
        case RefineryTier::Industrial:   return "Industrial";
        case RefineryTier::Military:     return "Military";
        case RefineryTier::Experimental: return "Experimental";
    }
    return "Unknown";
}

std::string RefineryRecipe::GetStateName(RefiningState state) {
    switch (state) {
        case RefiningState::Idle:       return "Idle";
        case RefiningState::Loading:    return "Loading";
        case RefiningState::Processing: return "Processing";
        case RefiningState::Completed:  return "Completed";
        case RefiningState::Failed:     return "Failed";
    }
    return "Unknown";
}

std::vector<RefineryRecipe> RefineryRecipe::GetDefaultRecipes() {
    return {
        {1, "Iron Ore",     10, "Iron Ingot",      8, 5.0f,  RefineryTier::Basic,        1.0f},
        {2, "Titanium Ore", 10, "Titanium Ingot",   6, 8.0f,  RefineryTier::Basic,        1.0f},
        {3, "Naonite Ore",  10, "Naonite Crystal",   4, 12.0f, RefineryTier::Advanced,     1.0f},
        {4, "Trinium Ore",  10, "Trinium Bar",       3, 15.0f, RefineryTier::Advanced,     1.0f},
        {5, "Xanion Ore",   10, "Xanion Alloy",      2, 20.0f, RefineryTier::Industrial,   1.0f},
        {6, "Ogonite Ore",  10, "Ogonite Plate",     2, 25.0f, RefineryTier::Military,     1.0f},
        {7, "Avorion Ore",  10, "Avorion Crystal",    1, 30.0f, RefineryTier::Experimental, 1.0f},
        {8, "Scrap Metal",  20, "Iron Ingot",         5, 10.0f, RefineryTier::Basic,        0.8f},
    };
}

// ---------------------------------------------------------------------------
// RefineryComponent
// ---------------------------------------------------------------------------

RefineryComponent::RefineryComponent(RefineryTier tier, int maxJobs)
    : _tier(tier)
    , _maxJobs(maxJobs)
{
}

RefineryTier RefineryComponent::GetTier() const {
    return _tier;
}

void RefineryComponent::SetTier(RefineryTier tier) {
    _tier = tier;
}

int RefineryComponent::GetMaxJobs() const {
    return _maxJobs;
}

int RefineryComponent::GetActiveJobCount() const {
    int count = 0;
    for (const auto& job : _jobs) {
        if (job.state == RefiningState::Loading ||
            job.state == RefiningState::Processing) {
            ++count;
        }
    }
    return count;
}

int RefineryComponent::GetCompletedJobCount() const {
    int count = 0;
    for (const auto& job : _jobs) {
        if (job.state == RefiningState::Completed) ++count;
    }
    return count;
}

bool RefineryComponent::StartJob(const RefineryRecipe& recipe, int batchSize) {
    // Refinery tier must meet or exceed the recipe requirement
    if (static_cast<int>(_tier) < static_cast<int>(recipe.requiredTier)) {
        return false;
    }

    // Check job capacity (only active jobs count against the limit)
    if (GetActiveJobCount() >= _maxJobs) {
        return false;
    }

    RefiningJob job;
    job.jobId = _nextJobId++;
    job.recipe = recipe;
    job.state = RefiningState::Loading;
    job.progress = 0.0f;
    job.elapsedTime = 0.0f;
    job.batchSize = batchSize;
    job.efficiencyBonus = 0.0f;

    _jobs.push_back(job);
    return true;
}

bool RefineryComponent::CancelJob(int jobId) {
    auto it = std::find_if(_jobs.begin(), _jobs.end(),
        [jobId](const RefiningJob& j) { return j.jobId == jobId; });

    if (it == _jobs.end()) return false;

    _jobs.erase(it);
    return true;
}

std::pair<std::string, int> RefineryComponent::CollectJob(int jobId) {
    auto it = std::find_if(_jobs.begin(), _jobs.end(),
        [jobId](const RefiningJob& j) { return j.jobId == jobId; });

    if (it == _jobs.end() || it->state != RefiningState::Completed) {
        return {"", 0};
    }

    float totalYield = static_cast<float>(it->recipe.outputAmount)
                     * static_cast<float>(it->batchSize)
                     * it->recipe.yieldMultiplier
                     * (GetEfficiencyMultiplier() + it->efficiencyBonus);
    int outputAmount = static_cast<int>(totalYield);
    std::string outputMaterial = it->recipe.outputMaterial;

    _jobs.erase(it);
    return {outputMaterial, outputAmount};
}

const RefiningJob* RefineryComponent::GetJob(int jobId) const {
    for (const auto& job : _jobs) {
        if (job.jobId == jobId) return &job;
    }
    return nullptr;
}

const std::vector<RefiningJob>& RefineryComponent::GetAllJobs() const {
    return _jobs;
}

float RefineryComponent::GetEfficiencyMultiplier() const {
    return 1.0f + 0.1f * static_cast<float>(static_cast<int>(_tier));
}

float RefineryComponent::GetProcessingSpeedMultiplier() const {
    return 1.0f + 0.15f * static_cast<float>(static_cast<int>(_tier));
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData RefineryComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "RefineryComponent";
    cd.data["tier"]      = std::to_string(static_cast<int>(_tier));
    cd.data["maxJobs"]   = std::to_string(_maxJobs);
    cd.data["nextJobId"] = std::to_string(_nextJobId);

    // Jobs
    cd.data["jobCount"] = std::to_string(_jobs.size());
    for (size_t i = 0; i < _jobs.size(); ++i) {
        std::string prefix = "job_" + std::to_string(i) + "_";
        const auto& j = _jobs[i];
        cd.data[prefix + "jobId"]           = std::to_string(j.jobId);
        cd.data[prefix + "state"]           = std::to_string(static_cast<int>(j.state));
        cd.data[prefix + "progress"]        = std::to_string(j.progress);
        cd.data[prefix + "elapsedTime"]     = std::to_string(j.elapsedTime);
        cd.data[prefix + "batchSize"]       = std::to_string(j.batchSize);
        cd.data[prefix + "efficiencyBonus"] = std::to_string(j.efficiencyBonus);

        // Nested recipe
        std::string rp = prefix + "recipe_";
        cd.data[rp + "recipeId"]       = std::to_string(j.recipe.recipeId);
        cd.data[rp + "inputMaterial"]  = j.recipe.inputMaterial;
        cd.data[rp + "inputAmount"]    = std::to_string(j.recipe.inputAmount);
        cd.data[rp + "outputMaterial"] = j.recipe.outputMaterial;
        cd.data[rp + "outputAmount"]   = std::to_string(j.recipe.outputAmount);
        cd.data[rp + "processingTime"] = std::to_string(j.recipe.processingTime);
        cd.data[rp + "requiredTier"]   = std::to_string(static_cast<int>(j.recipe.requiredTier));
        cd.data[rp + "yieldMultiplier"]= std::to_string(j.recipe.yieldMultiplier);
    }

    return cd;
}

void RefineryComponent::Deserialize(const ComponentData& data) {
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

    // Tier
    int tierVal = getInt("tier", 0);
    constexpr int kMaxTier = static_cast<int>(RefineryTier::Experimental);
    if (tierVal >= 0 && tierVal <= kMaxTier) {
        _tier = static_cast<RefineryTier>(tierVal);
    } else {
        _tier = RefineryTier::Basic;
    }

    _maxJobs   = getInt("maxJobs", 3);
    _nextJobId = getInt("nextJobId", 1);

    // Jobs
    int jobCount = getInt("jobCount", 0);
    _jobs.clear();
    _jobs.reserve(static_cast<size_t>(jobCount));
    for (int i = 0; i < jobCount; ++i) {
        std::string prefix = "job_" + std::to_string(i) + "_";
        RefiningJob j;
        j.jobId           = getInt(prefix + "jobId", 0);
        int stateVal      = getInt(prefix + "state", 0);
        constexpr int kMaxState = static_cast<int>(RefiningState::Failed);
        if (stateVal >= 0 && stateVal <= kMaxState) {
            j.state = static_cast<RefiningState>(stateVal);
        } else {
            j.state = RefiningState::Idle;
        }
        j.progress        = getFloat(prefix + "progress", 0.0f);
        j.elapsedTime     = getFloat(prefix + "elapsedTime", 0.0f);
        j.batchSize       = getInt(prefix + "batchSize", 1);
        j.efficiencyBonus = getFloat(prefix + "efficiencyBonus", 0.0f);

        // Nested recipe
        std::string rp = prefix + "recipe_";
        j.recipe.recipeId       = getInt(rp + "recipeId", 0);
        j.recipe.inputMaterial  = getStr(rp + "inputMaterial");
        j.recipe.inputAmount    = getInt(rp + "inputAmount", 0);
        j.recipe.outputMaterial = getStr(rp + "outputMaterial");
        j.recipe.outputAmount   = getInt(rp + "outputAmount", 0);
        j.recipe.processingTime = getFloat(rp + "processingTime", 0.0f);
        int recipeTierVal       = getInt(rp + "requiredTier", 0);
        if (recipeTierVal >= 0 && recipeTierVal <= kMaxTier) {
            j.recipe.requiredTier = static_cast<RefineryTier>(recipeTierVal);
        } else {
            j.recipe.requiredTier = RefineryTier::Basic;
        }
        j.recipe.yieldMultiplier = getFloat(rp + "yieldMultiplier", 1.0f);

        _jobs.push_back(j);
    }
}

// ---------------------------------------------------------------------------
// RefinerySystem
// ---------------------------------------------------------------------------

RefinerySystem::RefinerySystem() : SystemBase("RefinerySystem") {}

RefinerySystem::RefinerySystem(EntityManager& entityManager)
    : SystemBase("RefinerySystem")
    , _entityManager(&entityManager)
{
}

void RefinerySystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

void RefinerySystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto refineries = _entityManager->GetAllComponents<RefineryComponent>();
    for (auto* refinery : refineries) {
        float speedMult = refinery->GetProcessingSpeedMultiplier();

        for (auto& job : refinery->_jobs) {
            switch (job.state) {
                case RefiningState::Loading: {
                    job.state = RefiningState::Processing;
                    break;
                }

                case RefiningState::Processing: {
                    job.elapsedTime += deltaTime * speedMult;
                    float totalTime = job.recipe.processingTime
                                    * static_cast<float>(job.batchSize);
                    if (totalTime > 0.0f) {
                        job.progress = job.elapsedTime / totalTime;
                    }
                    if (job.progress >= 1.0f) {
                        job.progress = 1.0f;
                        job.state = RefiningState::Completed;
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }
}

} // namespace subspace
