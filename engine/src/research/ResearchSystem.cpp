#include "research/ResearchSystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// ResearchNode
// ---------------------------------------------------------------------------

std::string ResearchNode::GetCategoryName(ResearchCategory cat) {
    switch (cat) {
        case ResearchCategory::Engineering: return "Engineering";
        case ResearchCategory::Weapons:     return "Weapons";
        case ResearchCategory::Shields:     return "Shields";
        case ResearchCategory::Navigation:  return "Navigation";
        case ResearchCategory::Economy:     return "Economy";
        case ResearchCategory::Special:     return "Special";
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// ResearchJob
// ---------------------------------------------------------------------------

float ResearchJob::GetPercentage() const {
    if (totalCost <= 0.0f) return 100.0f;
    float pct = (progress / totalCost) * 100.0f;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    return pct;
}

// ---------------------------------------------------------------------------
// ResearchTree
// ---------------------------------------------------------------------------

void ResearchTree::AddNode(const ResearchNode& node) {
    _nodeIndex[node.nodeId] = _nodes.size();
    _nodes.push_back(node);
}

const ResearchNode* ResearchTree::FindNode(const std::string& nodeId) const {
    auto it = _nodeIndex.find(nodeId);
    if (it == _nodeIndex.end()) return nullptr;
    return &_nodes[it->second];
}

std::vector<const ResearchNode*> ResearchTree::GetNodesByCategory(ResearchCategory cat) const {
    std::vector<const ResearchNode*> result;
    for (const auto& node : _nodes) {
        if (node.category == cat) {
            result.push_back(&node);
        }
    }
    return result;
}

const std::vector<ResearchNode>& ResearchTree::GetAllNodes() const {
    return _nodes;
}

size_t ResearchTree::GetNodeCount() const {
    return _nodes.size();
}

bool ResearchTree::ArePrerequisitesMet(const std::string& nodeId,
                                       const std::unordered_set<std::string>& completed) const {
    const ResearchNode* node = FindNode(nodeId);
    if (!node) return false;

    for (const auto& prereq : node->prerequisites) {
        if (completed.find(prereq) == completed.end()) return false;
    }
    return true;
}

ResearchTree ResearchTree::CreateDefaultTree() {
    ResearchTree tree;

    // Engineering tier 1
    {
        ResearchNode n;
        n.nodeId = "improved_hull";
        n.displayName = "Improved Hull";
        n.description = "Increases hull integrity by 20%";
        n.category = ResearchCategory::Engineering;
        n.researchCost = 100.0f;
        n.requiredLevel = 1;
        tree.AddNode(n);
    }

    // Engineering tier 2
    {
        ResearchNode n;
        n.nodeId = "advanced_materials";
        n.displayName = "Advanced Materials";
        n.description = "Unlocks stronger building materials";
        n.category = ResearchCategory::Engineering;
        n.researchCost = 200.0f;
        n.requiredLevel = 2;
        n.prerequisites = {"improved_hull"};
        tree.AddNode(n);
    }

    // Weapons tier 1
    {
        ResearchNode n;
        n.nodeId = "laser_efficiency";
        n.displayName = "Laser Efficiency";
        n.description = "Reduces laser energy consumption by 15%";
        n.category = ResearchCategory::Weapons;
        n.researchCost = 120.0f;
        n.requiredLevel = 1;
        tree.AddNode(n);
    }

    // Weapons tier 2
    {
        ResearchNode n;
        n.nodeId = "plasma_weapons";
        n.displayName = "Plasma Weapons";
        n.description = "Unlocks plasma-based weapon systems";
        n.category = ResearchCategory::Weapons;
        n.researchCost = 250.0f;
        n.requiredLevel = 2;
        n.prerequisites = {"laser_efficiency"};
        tree.AddNode(n);
    }

    // Shields tier 1
    {
        ResearchNode n;
        n.nodeId = "shield_harmonics";
        n.displayName = "Shield Harmonics";
        n.description = "Improves shield recharge rate by 10%";
        n.category = ResearchCategory::Shields;
        n.researchCost = 150.0f;
        n.requiredLevel = 1;
        tree.AddNode(n);
    }

    // Navigation tier 1
    {
        ResearchNode n;
        n.nodeId = "hyperdrive_calibration";
        n.displayName = "Hyperdrive Calibration";
        n.description = "Reduces hyperspace travel time by 20%";
        n.category = ResearchCategory::Navigation;
        n.researchCost = 130.0f;
        n.requiredLevel = 1;
        tree.AddNode(n);
    }

    // Economy tier 1
    {
        ResearchNode n;
        n.nodeId = "trade_networks";
        n.displayName = "Trade Networks";
        n.description = "Unlocks additional trade routes and better prices";
        n.category = ResearchCategory::Economy;
        n.researchCost = 100.0f;
        n.requiredLevel = 1;
        tree.AddNode(n);
    }

    // Special (requires multiple prerequisites)
    {
        ResearchNode n;
        n.nodeId = "experimental_reactor";
        n.displayName = "Experimental Reactor";
        n.description = "Doubles power generation capacity";
        n.category = ResearchCategory::Special;
        n.researchCost = 500.0f;
        n.requiredLevel = 3;
        n.prerequisites = {"advanced_materials", "shield_harmonics"};
        tree.AddNode(n);
    }

    return tree;
}

// ---------------------------------------------------------------------------
// ResearchComponent
// ---------------------------------------------------------------------------

bool ResearchComponent::StartResearch(const ResearchNode& node, const ResearchTree& tree) {
    if (hasActiveJob) return false;
    if (researcherLevel < node.requiredLevel) return false;
    if (HasCompleted(node.nodeId)) return false;
    if (!tree.ArePrerequisitesMet(node.nodeId, completedResearch)) return false;

    currentJob.nodeId = node.nodeId;
    currentJob.progress = 0.0f;
    currentJob.totalCost = node.researchCost;
    currentJob.isComplete = false;
    hasActiveJob = true;
    return true;
}

bool ResearchComponent::HasCompleted(const std::string& nodeId) const {
    return completedResearch.find(nodeId) != completedResearch.end();
}

size_t ResearchComponent::GetCompletedCount() const {
    return completedResearch.size();
}

bool ResearchComponent::CancelResearch() {
    if (!hasActiveJob) return false;
    hasActiveJob = false;
    currentJob = ResearchJob();
    return true;
}

bool ResearchComponent::IsResearching() const {
    return hasActiveJob && !currentJob.isComplete;
}

std::vector<const ResearchNode*> ResearchComponent::GetAvailableResearch(const ResearchTree& tree) const {
    std::vector<const ResearchNode*> result;
    for (const auto& node : tree.GetAllNodes()) {
        if (HasCompleted(node.nodeId)) continue;
        if (researcherLevel < node.requiredLevel) continue;
        if (!tree.ArePrerequisitesMet(node.nodeId, completedResearch)) continue;
        result.push_back(&node);
    }
    return result;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData ResearchComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "ResearchComponent";
    cd.data["researchRate"]    = std::to_string(researchRate);
    cd.data["researcherLevel"] = std::to_string(researcherLevel);
    cd.data["hasActiveJob"]    = hasActiveJob ? "1" : "0";

    if (hasActiveJob) {
        cd.data["job_nodeId"]    = currentJob.nodeId;
        cd.data["job_progress"]  = std::to_string(currentJob.progress);
        cd.data["job_totalCost"] = std::to_string(currentJob.totalCost);
        cd.data["job_isComplete"] = currentJob.isComplete ? "1" : "0";
    }

    cd.data["completedCount"] = std::to_string(completedResearch.size());
    int idx = 0;
    for (const auto& id : completedResearch) {
        cd.data["completed_" + std::to_string(idx)] = id;
        ++idx;
    }

    return cd;
}

void ResearchComponent::Deserialize(const ComponentData& data) {
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

    researchRate    = getFloat("researchRate", 1.0f);
    researcherLevel = getInt("researcherLevel", 1);
    hasActiveJob    = getStr("hasActiveJob") == "1";

    if (hasActiveJob) {
        currentJob.nodeId    = getStr("job_nodeId");
        currentJob.progress  = getFloat("job_progress", 0.0f);
        currentJob.totalCost = getFloat("job_totalCost", 0.0f);
        currentJob.isComplete = getStr("job_isComplete") == "1";
    } else {
        currentJob = ResearchJob();
    }

    int completedCount = getInt("completedCount", 0);
    completedResearch.clear();
    for (int i = 0; i < completedCount; ++i) {
        std::string id = getStr("completed_" + std::to_string(i));
        if (!id.empty()) completedResearch.insert(id);
    }
}

// ---------------------------------------------------------------------------
// ResearchSystem
// ---------------------------------------------------------------------------

ResearchSystem::ResearchSystem() : SystemBase("ResearchSystem") {}

ResearchSystem::ResearchSystem(EntityManager& entityManager)
    : SystemBase("ResearchSystem")
    , _entityManager(&entityManager)
{
}

void ResearchSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto researchers = _entityManager->GetAllComponents<ResearchComponent>();
    for (auto* rc : researchers) {
        if (!rc->hasActiveJob || rc->currentJob.isComplete) continue;

        rc->currentJob.progress += rc->researchRate * deltaTime;
        if (rc->currentJob.progress >= rc->currentJob.totalCost) {
            rc->currentJob.progress = rc->currentJob.totalCost;
            rc->currentJob.isComplete = true;
            rc->completedResearch.insert(rc->currentJob.nodeId);
            rc->hasActiveJob = false;
        }
    }
}

} // namespace subspace
