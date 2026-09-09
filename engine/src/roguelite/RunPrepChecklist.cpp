#include "roguelite/RunPrepChecklist.h"

namespace subspace {

std::vector<RunPrepChecklistItem> BuildRunPrepChecklist(const RunPrepInputs& inputs) {
    return {
        {"fuel", "Fuel reserve meets route need", inputs.fuel >= inputs.fuelNeed, true},
        {"hull", "Hull integrity above launch threshold", inputs.hull >= 35.0f, true},
        {"cargo", "Cargo space available for haul", inputs.cargoFree >= 10.0f, false},
        {"drive", "Main drive installed", inputs.mainDriveInstalled, true},
        {"rcs", "RCS thrusters installed", inputs.rcsInstalled, true},
        {"defense", "Defense rating acceptable", inputs.defense >= 1.0f, false},
        {"scanner", "Scanner rating acceptable", inputs.scanner >= 1.0f, false},
        {"insurance", "Insurance active", inputs.insuranceActive, false},
    };
}

bool IsRunPrepLaunchAllowed(const std::vector<RunPrepChecklistItem>& checklist) {
    for (const auto& item : checklist) if (item.required && !item.pass) return false;
    return true;
}

} // namespace subspace
