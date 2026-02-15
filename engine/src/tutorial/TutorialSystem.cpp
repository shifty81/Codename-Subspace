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
// TutorialComponent serialization
// ---------------------------------------------------------------------------

static std::string TutorialStatusToString(TutorialStatus s) {
    switch (s) {
    case TutorialStatus::NotStarted: return "NotStarted";
    case TutorialStatus::Active:     return "Active";
    case TutorialStatus::Completed:  return "Completed";
    case TutorialStatus::Skipped:    return "Skipped";
    }
    return "NotStarted";
}

static TutorialStatus TutorialStatusFromString(const std::string& s) {
    if (s == "Active")    return TutorialStatus::Active;
    if (s == "Completed") return TutorialStatus::Completed;
    if (s == "Skipped")   return TutorialStatus::Skipped;
    return TutorialStatus::NotStarted;
}

static std::string StepTypeToString(TutorialStepType t) {
    switch (t) {
    case TutorialStepType::Message:       return "Message";
    case TutorialStepType::WaitForKey:    return "WaitForKey";
    case TutorialStepType::WaitForAction: return "WaitForAction";
    case TutorialStepType::HighlightUI:   return "HighlightUI";
    case TutorialStepType::WaitForTime:   return "WaitForTime";
    }
    return "Message";
}

static TutorialStepType StepTypeFromString(const std::string& s) {
    if (s == "WaitForKey")    return TutorialStepType::WaitForKey;
    if (s == "WaitForAction") return TutorialStepType::WaitForAction;
    if (s == "HighlightUI")   return TutorialStepType::HighlightUI;
    if (s == "WaitForTime")   return TutorialStepType::WaitForTime;
    return TutorialStepType::Message;
}

static std::string StepStatusToString(TutorialStepStatus s) {
    switch (s) {
    case TutorialStepStatus::NotStarted: return "NotStarted";
    case TutorialStepStatus::Active:     return "Active";
    case TutorialStepStatus::Completed:  return "Completed";
    case TutorialStepStatus::Skipped:    return "Skipped";
    }
    return "NotStarted";
}

static TutorialStepStatus StepStatusFromString(const std::string& s) {
    if (s == "Active")    return TutorialStepStatus::Active;
    if (s == "Completed") return TutorialStepStatus::Completed;
    if (s == "Skipped")   return TutorialStepStatus::Skipped;
    return TutorialStepStatus::NotStarted;
}

ComponentData TutorialComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "TutorialComponent";

    // Active tutorials
    cd.data["activeTutorialCount"] = std::to_string(activeTutorials.size());
    for (size_t i = 0; i < activeTutorials.size(); ++i) {
        const auto& tut = activeTutorials[i];
        std::string prefix = "tut_" + std::to_string(i) + "_";
        cd.data[prefix + "id"] = tut.id;
        cd.data[prefix + "title"] = tut.title;
        cd.data[prefix + "status"] = TutorialStatusToString(tut.status);
        cd.data[prefix + "currentStep"] = std::to_string(tut.currentStepIndex);
        cd.data[prefix + "autoStart"] = tut.autoStart ? "true" : "false";

        cd.data[prefix + "stepCount"] = std::to_string(tut.steps.size());
        for (size_t j = 0; j < tut.steps.size(); ++j) {
            const auto& step = tut.steps[j];
            std::string sp = prefix + "step_" + std::to_string(j) + "_";
            cd.data[sp + "id"] = step.id;
            cd.data[sp + "type"] = StepTypeToString(step.type);
            cd.data[sp + "status"] = StepStatusToString(step.status);
            cd.data[sp + "requiredAction"] = step.requiredAction;
            cd.data[sp + "canSkip"] = step.canSkip ? "true" : "false";
        }
    }

    // Completed tutorial ids
    cd.data["completedCount"] = std::to_string(completedTutorialIds.size());
    int idx = 0;
    for (const auto& cid : completedTutorialIds) {
        cd.data["completed_" + std::to_string(idx)] = cid;
        ++idx;
    }

    return cd;
}

void TutorialComponent::Deserialize(const ComponentData& data) {
    activeTutorials.clear();
    completedTutorialIds.clear();

    auto getStr = [&](const std::string& key) -> std::string {
        auto it = data.data.find(key);
        return it != data.data.end() ? it->second : "";
    };
    auto getInt = [&](const std::string& key, int def = 0) -> int {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stoi(it->second); } catch (...) { return def; }
    };

    // Active tutorials
    int tutCount = getInt("activeTutorialCount", 0);
    for (int i = 0; i < tutCount; ++i) {
        std::string prefix = "tut_" + std::to_string(i) + "_";
        Tutorial tut;
        tut.id = getStr(prefix + "id");
        tut.title = getStr(prefix + "title");
        tut.status = TutorialStatusFromString(getStr(prefix + "status"));
        tut.currentStepIndex = getInt(prefix + "currentStep", 0);
        tut.autoStart = getStr(prefix + "autoStart") == "true";

        int stepCount = getInt(prefix + "stepCount", 0);
        for (int j = 0; j < stepCount; ++j) {
            std::string sp = prefix + "step_" + std::to_string(j) + "_";
            TutorialStep step;
            step.id = getStr(sp + "id");
            step.type = StepTypeFromString(getStr(sp + "type"));
            step.status = StepStatusFromString(getStr(sp + "status"));
            step.requiredAction = getStr(sp + "requiredAction");
            step.canSkip = getStr(sp + "canSkip") != "false";
            tut.steps.push_back(std::move(step));
        }

        activeTutorials.push_back(std::move(tut));
    }

    // Completed tutorials
    int completedCount = getInt("completedCount", 0);
    for (int i = 0; i < completedCount; ++i) {
        std::string cid = getStr("completed_" + std::to_string(i));
        if (!cid.empty()) {
            completedTutorialIds.insert(cid);
        }
    }
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
