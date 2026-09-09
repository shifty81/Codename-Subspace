#pragma once

#include <string>
#include <vector>

namespace subspace {

struct RunPrepInputs {
    float fuel = 0.0f;
    float fuelNeed = 0.0f;
    float hull = 100.0f;
    float cargoFree = 0.0f;
    float defense = 0.0f;
    float scanner = 0.0f;
    bool mainDriveInstalled = true;
    bool rcsInstalled = true;
    bool insuranceActive = true;
};

struct RunPrepChecklistItem {
    std::string id;
    std::string label;
    bool pass = false;
    bool required = true;
};

std::vector<RunPrepChecklistItem> BuildRunPrepChecklist(const RunPrepInputs& inputs);
bool IsRunPrepLaunchAllowed(const std::vector<RunPrepChecklistItem>& checklist);

} // namespace subspace
