#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class TutorialStatus { NotStarted, Active, Completed, Skipped };
enum class TutorialStepType { Message, WaitForKey, WaitForAction, HighlightUI, WaitForTime };
enum class TutorialStepStatus { NotStarted, Active, Completed, Skipped };

/// A single step within a tutorial sequence.
struct TutorialStep {
    std::string id;
    TutorialStepType type = TutorialStepType::Message;
    std::string title;
    std::string message;
    TutorialStepStatus status = TutorialStepStatus::NotStarted;

    std::string requiredKey;     // For WaitForKey
    std::string requiredAction;  // For WaitForAction
    std::string uiElementId;    // For HighlightUI
    float duration = 0.0f;      // For WaitForTime (seconds)
    float elapsedTime = 0.0f;
    bool canSkip = true;

    void Start();
    void Complete();
    void Skip();
    void Reset();
    bool IsTimeElapsed() const;
};

/// A tutorial composed of ordered steps.
class Tutorial {
public:
    std::string id;
    std::string title;
    std::string description;
    TutorialStatus status = TutorialStatus::NotStarted;
    std::vector<TutorialStep> steps;
    int currentStepIndex = 0;
    bool autoStart = false;
    std::vector<std::string> prerequisites;

    bool Start();
    bool CompleteCurrentStep();
    void SkipCurrentStep();
    void Skip();
    void Complete();
    void Reset();
    void Update(float deltaTime);

    TutorialStep* GetCurrentStep();
    float GetCompletionPercentage() const;
    bool AreAllStepsComplete() const;
};

/// Component that tracks tutorial progress for an entity.
struct TutorialComponent : public IComponent {
    std::vector<Tutorial> activeTutorials;
    std::set<std::string> completedTutorialIds;
};

/// System that manages tutorial templates and drives tutorial progression.
class TutorialSystem : public SystemBase {
public:
    TutorialSystem();

    void Update(float deltaTime) override;

    /// Register a tutorial template for later instantiation.
    void AddTutorialTemplate(const Tutorial& tutorial);

    /// Start a tutorial from a template on the given entity. Returns false if prerequisites not met.
    bool StartTutorial(EntityId entityId, const std::string& templateId,
                       TutorialComponent& comp);

    /// Complete the current step of a specific tutorial (or the first active one).
    void CompleteCurrentStep(TutorialComponent& comp,
                             const std::string& tutorialId = "");

    /// Complete any active WaitForAction step that matches the given action.
    void CompleteActionStep(TutorialComponent& comp, const std::string& action);

    /// Skip a specific tutorial entirely.
    void SkipTutorial(TutorialComponent& comp, const std::string& tutorialId);

    /// Check whether a tutorial has been completed.
    bool HasCompletedTutorial(const TutorialComponent& comp,
                              const std::string& tutorialId) const;

    /// Check whether all listed prerequisites have been completed.
    bool ArePrerequisitesMet(const TutorialComponent& comp,
                             const std::vector<std::string>& prereqs) const;

    /// Auto-start any eligible tutorials whose prerequisites are met.
    void CheckAutoStartTutorials(EntityId entityId, TutorialComponent& comp);

    /// Get all registered tutorial templates.
    const std::unordered_map<std::string, Tutorial>& GetTutorialTemplates() const;

    /// Get the number of registered templates.
    size_t GetTemplateCount() const;

private:
    std::unordered_map<std::string, Tutorial> _templates;
};

} // namespace subspace
