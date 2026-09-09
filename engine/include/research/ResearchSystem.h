#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>

namespace subspace {

/// Research categories for organizing the tech tree.
enum class ResearchCategory { Engineering, Weapons, Shields, Navigation, Economy, Special };

/// A single research node in the tech tree.
struct ResearchNode {
    std::string nodeId;
    std::string displayName;
    std::string description;
    ResearchCategory category = ResearchCategory::Engineering;
    float researchCost = 100.0f;      // total research points needed
    int requiredLevel = 1;
    std::vector<std::string> prerequisites;   // nodeIds that must be completed first

    /// Get the display name for a category.
    static std::string GetCategoryName(ResearchCategory cat);
};

/// An in-progress research job.
struct ResearchJob {
    std::string nodeId;
    float progress = 0.0f;     // accumulated research points
    float totalCost = 0.0f;
    bool isComplete = false;

    /// Get completion percentage (0-100).
    float GetPercentage() const;
};

/// Database of all available research nodes (the tech tree).
class ResearchTree {
public:
    /// Add a research node.
    void AddNode(const ResearchNode& node);

    /// Find a node by ID. Returns nullptr if not found.
    const ResearchNode* FindNode(const std::string& nodeId) const;

    /// Get all nodes in a category.
    std::vector<const ResearchNode*> GetNodesByCategory(ResearchCategory cat) const;

    /// Get all nodes.
    const std::vector<ResearchNode>& GetAllNodes() const;

    /// Get the number of nodes.
    size_t GetNodeCount() const;

    /// Check if all prerequisites of a node are in the completed set.
    bool ArePrerequisitesMet(const std::string& nodeId,
                             const std::unordered_set<std::string>& completed) const;

    /// Create a default tech tree for testing.
    static ResearchTree CreateDefaultTree();

private:
    std::vector<ResearchNode> _nodes;
    std::unordered_map<std::string, size_t> _nodeIndex;
};

/// ECS component that gives an entity research capabilities.
struct ResearchComponent : public IComponent {
    std::unordered_set<std::string> completedResearch;
    ResearchJob currentJob;
    bool hasActiveJob = false;
    float researchRate = 1.0f;     // research points generated per second
    int researcherLevel = 1;

    /// Start researching a node. Returns true if started.
    bool StartResearch(const ResearchNode& node, const ResearchTree& tree);

    /// Check if a specific node has been completed.
    bool HasCompleted(const std::string& nodeId) const;

    /// Get the number of completed research nodes.
    size_t GetCompletedCount() const;

    /// Cancel current research. Returns true if there was an active job.
    bool CancelResearch();

    /// Check if currently researching.
    bool IsResearching() const;

    /// Get all available (unlocked) nodes from a tree.
    std::vector<const ResearchNode*> GetAvailableResearch(const ResearchTree& tree) const;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);
};

/// System that ticks research progress each frame.
class ResearchSystem : public SystemBase {
public:
    ResearchSystem();
    explicit ResearchSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
