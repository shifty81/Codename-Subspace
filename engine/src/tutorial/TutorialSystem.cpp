#include "tutorial/TutorialSystem.h"

namespace subspace {

// ---------------------------------------------------------------------------
// TutorialStep
// ---------------------------------------------------------------------------

void TutorialStep::Start() {
    status = TutorialStepStatus::Active;
    elapsedTime = 0.0f;
}

void TutorialStep::Complete() {
    status = TutorialStepStatus::Completed;
}

void TutorialStep::Skip() {
    status = TutorialStepStatus::Skipped;
}

void TutorialStep::Reset() {
    status = TutorialStepStatus::NotStarted;
    elapsedTime = 0.0f;
}

bool TutorialStep::IsTimeElapsed() const {
    return type == TutorialStepType::WaitForTime && elapsedTime >= duration;
}

// ---------------------------------------------------------------------------
// Tutorial
// ---------------------------------------------------------------------------

bool Tutorial::Start() {
    if (status != TutorialStatus::NotStarted) return false;
    status = TutorialStatus::Active;
    currentStepIndex = 0;
    if (!steps.empty()) {
        steps[0].Start();
    }
    return true;
}

bool Tutorial::CompleteCurrentStep() {
    if (status != TutorialStatus::Active) return false;
    if (currentStepIndex < 0 || currentStepIndex >= static_cast<int>(steps.size()))
        return false;

    steps[currentStepIndex].Complete();

    int nextIndex = currentStepIndex + 1;
    if (nextIndex >= static_cast<int>(steps.size())) {
        Complete();
    } else {
        currentStepIndex = nextIndex;
        steps[currentStepIndex].Start();
    }
    return true;
}

void Tutorial::SkipCurrentStep() {
    if (status != TutorialStatus::Active) return;
    if (currentStepIndex < 0 || currentStepIndex >= static_cast<int>(steps.size()))
        return;

    TutorialStep& step = steps[currentStepIndex];
    if (!step.canSkip) return;

    step.Skip();

    int nextIndex = currentStepIndex + 1;
    if (nextIndex >= static_cast<int>(steps.size())) {
        Complete();
    } else {
        currentStepIndex = nextIndex;
        steps[currentStepIndex].Start();
    }
}

void Tutorial::Skip() {
    status = TutorialStatus::Skipped;
    for (auto& step : steps) {
        if (step.status == TutorialStepStatus::NotStarted ||
            step.status == TutorialStepStatus::Active) {
            step.Skip();
        }
    }
}

void Tutorial::Complete() {
    status = TutorialStatus::Completed;
}

void Tutorial::Reset() {
    status = TutorialStatus::NotStarted;
    currentStepIndex = 0;
    for (auto& step : steps) {
        step.Reset();
    }
}

void Tutorial::Update(float deltaTime) {
    if (status != TutorialStatus::Active) return;
    if (currentStepIndex < 0 || currentStepIndex >= static_cast<int>(steps.size()))
        return;

    TutorialStep& step = steps[currentStepIndex];
    if (step.status == TutorialStepStatus::Active &&
        step.type == TutorialStepType::WaitForTime) {
        step.elapsedTime += deltaTime;
        if (step.IsTimeElapsed()) {
            CompleteCurrentStep();
        }
    }
}

TutorialStep* Tutorial::GetCurrentStep() {
    if (currentStepIndex < 0 || currentStepIndex >= static_cast<int>(steps.size()))
        return nullptr;
    return &steps[currentStepIndex];
}

float Tutorial::GetCompletionPercentage() const {
    if (steps.empty()) return 0.0f;
    int completed = 0;
    for (const auto& step : steps) {
        if (step.status == TutorialStepStatus::Completed ||
            step.status == TutorialStepStatus::Skipped) {
            ++completed;
        }
    }
    return static_cast<float>(completed) / static_cast<float>(steps.size()) * 100.0f;
}

bool Tutorial::AreAllStepsComplete() const {
    for (const auto& step : steps) {
        if (step.status != TutorialStepStatus::Completed &&
            step.status != TutorialStepStatus::Skipped) {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// TutorialSystem
// ---------------------------------------------------------------------------

TutorialSystem::TutorialSystem() : SystemBase("TutorialSystem") {}

void TutorialSystem::Update(float /*deltaTime*/) {
    // Standalone: callers drive per-entity tutorials via the public API.
}

void TutorialSystem::AddTutorialTemplate(const Tutorial& tutorial) {
    _templates[tutorial.id] = tutorial;
}

bool TutorialSystem::StartTutorial(EntityId /*entityId*/,
                                   const std::string& templateId,
                                   TutorialComponent& comp) {
    auto it = _templates.find(templateId);
    if (it == _templates.end()) return false;

    if (!ArePrerequisitesMet(comp, it->second.prerequisites)) return false;

    // Don't start if already active or completed
    if (comp.completedTutorialIds.count(templateId)) return false;
    for (const auto& tut : comp.activeTutorials) {
        if (tut.id == templateId) return false;
    }

    Tutorial instance = it->second;
    instance.Start();
    comp.activeTutorials.push_back(std::move(instance));
    return true;
}

void TutorialSystem::CompleteCurrentStep(TutorialComponent& comp,
                                         const std::string& tutorialId) {
    // When tutorialId is empty, complete the first active tutorial's current step only.
    for (auto& tut : comp.activeTutorials) {
        if (tutorialId.empty() || tut.id == tutorialId) {
            if (tut.status == TutorialStatus::Active) {
                tut.CompleteCurrentStep();
                if (tut.status == TutorialStatus::Completed) {
                    comp.completedTutorialIds.insert(tut.id);
                }
                return;
            }
        }
    }
}

void TutorialSystem::CompleteActionStep(TutorialComponent& comp,
                                        const std::string& action) {
    // Intentionally completes all active tutorials with a matching WaitForAction step.
    for (auto& tut : comp.activeTutorials) {
        if (tut.status != TutorialStatus::Active) continue;
        TutorialStep* step = tut.GetCurrentStep();
        if (step && step->status == TutorialStepStatus::Active &&
            step->type == TutorialStepType::WaitForAction &&
            step->requiredAction == action) {
            tut.CompleteCurrentStep();
            if (tut.status == TutorialStatus::Completed) {
                comp.completedTutorialIds.insert(tut.id);
            }
        }
    }
}

void TutorialSystem::SkipTutorial(TutorialComponent& comp,
                                  const std::string& tutorialId) {
    for (auto& tut : comp.activeTutorials) {
        if (tut.id == tutorialId) {
            tut.Skip();
            comp.completedTutorialIds.insert(tut.id);
            return;
        }
    }
}

bool TutorialSystem::HasCompletedTutorial(const TutorialComponent& comp,
                                          const std::string& tutorialId) const {
    return comp.completedTutorialIds.count(tutorialId) > 0;
}

bool TutorialSystem::ArePrerequisitesMet(
    const TutorialComponent& comp,
    const std::vector<std::string>& prereqs) const {
    for (const auto& prereq : prereqs) {
        if (comp.completedTutorialIds.count(prereq) == 0) return false;
    }
    return true;
}

void TutorialSystem::CheckAutoStartTutorials(EntityId entityId,
                                             TutorialComponent& comp) {
    for (const auto& [templateId, tmpl] : _templates) {
        if (!tmpl.autoStart) continue;
        if (comp.completedTutorialIds.count(templateId)) continue;

        // Skip if already active
        bool alreadyActive = false;
        for (const auto& tut : comp.activeTutorials) {
            if (tut.id == templateId) {
                alreadyActive = true;
                break;
            }
        }
        if (alreadyActive) continue;

        if (ArePrerequisitesMet(comp, tmpl.prerequisites)) {
            StartTutorial(entityId, templateId, comp);
        }
    }
}

const std::unordered_map<std::string, Tutorial>&
TutorialSystem::GetTutorialTemplates() const {
    return _templates;
}

size_t TutorialSystem::GetTemplateCount() const { return _templates.size(); }

} // namespace subspace
