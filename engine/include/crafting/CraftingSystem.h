#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace subspace {

/// Crafting station types that determine which recipes can be used.
enum class CraftingStationType { Basic, Forge, Laboratory, Shipyard, Refinery };

/// A single ingredient required by a recipe.
struct CraftingIngredient {
    std::string itemName;
    int quantity = 1;
};

/// A crafting recipe definition.
struct CraftingRecipe {
    std::string recipeId;
    std::string resultItem;
    int resultQuantity = 1;
    std::vector<CraftingIngredient> ingredients;
    CraftingStationType requiredStation = CraftingStationType::Basic;
    float craftTime = 1.0f; // seconds
    int requiredLevel = 1;

    /// Get the display name for a station type.
    static std::string GetStationName(CraftingStationType type);
};

/// An in-progress crafting job.
struct CraftingJob {
    std::string recipeId;
    float timeRemaining = 0.0f;
    float totalTime = 0.0f;
    bool isComplete = false;

    /// Get progress percentage (0-100).
    float GetProgress() const;
};

/// Recipe database that holds all known recipes.
class RecipeDatabase {
public:
    /// Add a recipe to the database.
    void AddRecipe(const CraftingRecipe& recipe);

    /// Find a recipe by ID. Returns nullptr if not found.
    const CraftingRecipe* FindRecipe(const std::string& recipeId) const;

    /// Get all recipes for a given station type.
    std::vector<const CraftingRecipe*> GetRecipesForStation(CraftingStationType type) const;

    /// Get all recipes.
    const std::vector<CraftingRecipe>& GetAllRecipes() const;

    /// Get the number of recipes.
    size_t GetRecipeCount() const;

    /// Create a set of default recipes.
    static RecipeDatabase CreateDefaultDatabase();

private:
    std::vector<CraftingRecipe> _recipes;
    std::unordered_map<std::string, size_t> _recipeIndex; // recipeId -> index
};

/// ECS component that gives an entity crafting capabilities.
struct CraftingComponent : public IComponent {
    CraftingStationType stationType = CraftingStationType::Basic;
    int crafterLevel = 1;
    std::vector<CraftingJob> activeJobs;
    int maxConcurrentJobs = 1;
    float speedMultiplier = 1.0f;

    /// Start a crafting job from a recipe. Returns true if started.
    bool StartCrafting(const CraftingRecipe& recipe);

    /// Check if crafting capacity is available.
    bool CanStartJob() const;

    /// Get the number of active (incomplete) jobs.
    int GetActiveJobCount() const;

    /// Get completed jobs and remove them from the list.
    std::vector<CraftingJob> CollectCompletedJobs();

    /// Check if the crafter meets the level requirement for a recipe.
    bool MeetsLevelRequirement(const CraftingRecipe& recipe) const;

    /// Check if the station type matches the recipe requirement.
    bool HasRequiredStation(const CraftingRecipe& recipe) const;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);
};

/// System that updates crafting jobs each frame.
class CraftingSystem : public SystemBase {
public:
    CraftingSystem();
    explicit CraftingSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
