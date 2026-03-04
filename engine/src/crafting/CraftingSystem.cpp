#include "crafting/CraftingSystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// CraftingRecipe
// ---------------------------------------------------------------------------

std::string CraftingRecipe::GetStationName(CraftingStationType type) {
    switch (type) {
        case CraftingStationType::Basic:       return "Basic Workshop";
        case CraftingStationType::Forge:       return "Forge";
        case CraftingStationType::Laboratory:  return "Laboratory";
        case CraftingStationType::Shipyard:    return "Shipyard";
        case CraftingStationType::Refinery:    return "Refinery";
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// CraftingJob
// ---------------------------------------------------------------------------

float CraftingJob::GetProgress() const {
    if (totalTime <= 0.0f) return 100.0f;
    float elapsed = totalTime - timeRemaining;
    float pct = (elapsed / totalTime) * 100.0f;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    return pct;
}

// ---------------------------------------------------------------------------
// RecipeDatabase
// ---------------------------------------------------------------------------

void RecipeDatabase::AddRecipe(const CraftingRecipe& recipe) {
    _recipeIndex[recipe.recipeId] = _recipes.size();
    _recipes.push_back(recipe);
}

const CraftingRecipe* RecipeDatabase::FindRecipe(const std::string& recipeId) const {
    auto it = _recipeIndex.find(recipeId);
    if (it == _recipeIndex.end()) return nullptr;
    return &_recipes[it->second];
}

std::vector<const CraftingRecipe*> RecipeDatabase::GetRecipesForStation(CraftingStationType type) const {
    std::vector<const CraftingRecipe*> result;
    for (const auto& recipe : _recipes) {
        if (recipe.requiredStation == type) {
            result.push_back(&recipe);
        }
    }
    return result;
}

const std::vector<CraftingRecipe>& RecipeDatabase::GetAllRecipes() const {
    return _recipes;
}

size_t RecipeDatabase::GetRecipeCount() const {
    return _recipes.size();
}

RecipeDatabase RecipeDatabase::CreateDefaultDatabase() {
    RecipeDatabase db;

    // 1. iron_plate (Basic) - 2x Iron Ore -> 1x Iron Plate, 2s
    {
        CraftingRecipe r;
        r.recipeId = "iron_plate";
        r.resultItem = "Iron Plate";
        r.resultQuantity = 1;
        r.ingredients = {{"Iron Ore", 2}};
        r.requiredStation = CraftingStationType::Basic;
        r.craftTime = 2.0f;
        r.requiredLevel = 1;
        db.AddRecipe(r);
    }

    // 2. steel_beam (Forge) - 3x Iron Plate -> 1x Steel Beam, 5s
    {
        CraftingRecipe r;
        r.recipeId = "steel_beam";
        r.resultItem = "Steel Beam";
        r.resultQuantity = 1;
        r.ingredients = {{"Iron Plate", 3}};
        r.requiredStation = CraftingStationType::Forge;
        r.craftTime = 5.0f;
        r.requiredLevel = 1;
        db.AddRecipe(r);
    }

    // 3. circuit_board (Laboratory) - 1x Copper Wire + 1x Silicon -> 1x Circuit Board, 3s
    {
        CraftingRecipe r;
        r.recipeId = "circuit_board";
        r.resultItem = "Circuit Board";
        r.resultQuantity = 1;
        r.ingredients = {{"Copper Wire", 1}, {"Silicon", 1}};
        r.requiredStation = CraftingStationType::Laboratory;
        r.craftTime = 3.0f;
        r.requiredLevel = 1;
        db.AddRecipe(r);
    }

    // 4. hull_panel (Forge) - 2x Steel Beam + 1x Iron Plate -> 1x Hull Panel, 4s
    {
        CraftingRecipe r;
        r.recipeId = "hull_panel";
        r.resultItem = "Hull Panel";
        r.resultQuantity = 1;
        r.ingredients = {{"Steel Beam", 2}, {"Iron Plate", 1}};
        r.requiredStation = CraftingStationType::Forge;
        r.craftTime = 4.0f;
        r.requiredLevel = 1;
        db.AddRecipe(r);
    }

    // 5. energy_cell (Laboratory) - 2x Circuit Board + 1x Naonite Crystal -> 1x Energy Cell, 6s
    {
        CraftingRecipe r;
        r.recipeId = "energy_cell";
        r.resultItem = "Energy Cell";
        r.resultQuantity = 1;
        r.ingredients = {{"Circuit Board", 2}, {"Naonite Crystal", 1}};
        r.requiredStation = CraftingStationType::Laboratory;
        r.craftTime = 6.0f;
        r.requiredLevel = 1;
        db.AddRecipe(r);
    }

    // 6. engine_component (Shipyard) - 1x Hull Panel + 2x Energy Cell + 1x Trinium -> 1x Engine Component, 10s, level 3
    {
        CraftingRecipe r;
        r.recipeId = "engine_component";
        r.resultItem = "Engine Component";
        r.resultQuantity = 1;
        r.ingredients = {{"Hull Panel", 1}, {"Energy Cell", 2}, {"Trinium", 1}};
        r.requiredStation = CraftingStationType::Shipyard;
        r.craftTime = 10.0f;
        r.requiredLevel = 3;
        db.AddRecipe(r);
    }

    // 7. shield_capacitor (Laboratory) - 2x Energy Cell + 1x Xanion -> 1x Shield Capacitor, 8s, level 2
    {
        CraftingRecipe r;
        r.recipeId = "shield_capacitor";
        r.resultItem = "Shield Capacitor";
        r.resultQuantity = 1;
        r.ingredients = {{"Energy Cell", 2}, {"Xanion", 1}};
        r.requiredStation = CraftingStationType::Laboratory;
        r.craftTime = 8.0f;
        r.requiredLevel = 2;
        db.AddRecipe(r);
    }

    // 8. refined_fuel (Refinery) - 3x Crude Oil + 1x Catalyst -> 2x Refined Fuel, 4s
    {
        CraftingRecipe r;
        r.recipeId = "refined_fuel";
        r.resultItem = "Refined Fuel";
        r.resultQuantity = 2;
        r.ingredients = {{"Crude Oil", 3}, {"Catalyst", 1}};
        r.requiredStation = CraftingStationType::Refinery;
        r.craftTime = 4.0f;
        r.requiredLevel = 1;
        db.AddRecipe(r);
    }

    return db;
}

// ---------------------------------------------------------------------------
// CraftingComponent
// ---------------------------------------------------------------------------

bool CraftingComponent::StartCrafting(const CraftingRecipe& recipe) {
    if (!CanStartJob()) return false;
    if (!MeetsLevelRequirement(recipe)) return false;
    if (!HasRequiredStation(recipe)) return false;

    CraftingJob job;
    job.recipeId = recipe.recipeId;
    job.timeRemaining = recipe.craftTime;
    job.totalTime = recipe.craftTime;
    job.isComplete = false;
    activeJobs.push_back(job);
    return true;
}

bool CraftingComponent::CanStartJob() const {
    return GetActiveJobCount() < maxConcurrentJobs;
}

int CraftingComponent::GetActiveJobCount() const {
    int count = 0;
    for (const auto& job : activeJobs) {
        if (!job.isComplete) ++count;
    }
    return count;
}

std::vector<CraftingJob> CraftingComponent::CollectCompletedJobs() {
    std::vector<CraftingJob> completed;
    auto it = activeJobs.begin();
    while (it != activeJobs.end()) {
        if (it->isComplete) {
            completed.push_back(*it);
            it = activeJobs.erase(it);
        } else {
            ++it;
        }
    }
    return completed;
}

bool CraftingComponent::MeetsLevelRequirement(const CraftingRecipe& recipe) const {
    return crafterLevel >= recipe.requiredLevel;
}

bool CraftingComponent::HasRequiredStation(const CraftingRecipe& recipe) const {
    return stationType == recipe.requiredStation;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData CraftingComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "CraftingComponent";
    cd.data["stationType"]       = std::to_string(static_cast<int>(stationType));
    cd.data["crafterLevel"]      = std::to_string(crafterLevel);
    cd.data["maxConcurrentJobs"] = std::to_string(maxConcurrentJobs);
    cd.data["speedMultiplier"]   = std::to_string(speedMultiplier);

    cd.data["jobCount"] = std::to_string(activeJobs.size());
    for (size_t i = 0; i < activeJobs.size(); ++i) {
        std::string prefix = "job" + std::to_string(i) + "_";
        cd.data[prefix + "recipeId"]      = activeJobs[i].recipeId;
        cd.data[prefix + "timeRemaining"] = std::to_string(activeJobs[i].timeRemaining);
        cd.data[prefix + "totalTime"]     = std::to_string(activeJobs[i].totalTime);
        cd.data[prefix + "isComplete"]    = activeJobs[i].isComplete ? "1" : "0";
    }

    return cd;
}

void CraftingComponent::Deserialize(const ComponentData& data) {
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

    constexpr int kMaxStationType = static_cast<int>(CraftingStationType::Refinery);
    int typeVal = getInt("stationType", 0);
    if (typeVal >= 0 && typeVal <= kMaxStationType) {
        stationType = static_cast<CraftingStationType>(typeVal);
    } else {
        stationType = CraftingStationType::Basic;
    }

    crafterLevel      = getInt("crafterLevel", 1);
    maxConcurrentJobs = getInt("maxConcurrentJobs", 1);
    speedMultiplier   = getFloat("speedMultiplier", 1.0f);

    int jobCount = getInt("jobCount", 0);
    activeJobs.clear();
    for (int i = 0; i < jobCount; ++i) {
        std::string prefix = "job" + std::to_string(i) + "_";
        CraftingJob job;
        job.recipeId      = getStr(prefix + "recipeId");
        job.timeRemaining = getFloat(prefix + "timeRemaining", 0.0f);
        job.totalTime     = getFloat(prefix + "totalTime", 0.0f);
        job.isComplete    = getStr(prefix + "isComplete") != "0";
        activeJobs.push_back(job);
    }
}

// ---------------------------------------------------------------------------
// CraftingSystem
// ---------------------------------------------------------------------------

CraftingSystem::CraftingSystem() : SystemBase("CraftingSystem") {}

CraftingSystem::CraftingSystem(EntityManager& entityManager)
    : SystemBase("CraftingSystem")
    , _entityManager(&entityManager)
{
}

void CraftingSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto crafters = _entityManager->GetAllComponents<CraftingComponent>();
    for (auto* crafter : crafters) {
        for (auto& job : crafter->activeJobs) {
            if (job.isComplete) continue;

            job.timeRemaining -= deltaTime * crafter->speedMultiplier;
            if (job.timeRemaining <= 0.0f) {
                job.timeRemaining = 0.0f;
                job.isComplete = true;
            }
        }
    }
}

} // namespace subspace
