#include "celestial/CelestialInteractionModel.h"

namespace subspace {

const char* CelestialInteractionKindName(CelestialInteractionKind kind) {
    switch (kind) {
    case CelestialInteractionKind::Scan: return "Scan";
    case CelestialInteractionKind::Orbit: return "Orbit";
    case CelestialInteractionKind::Harvest: return "Harvest";
    case CelestialInteractionKind::Dock: return "Dock";
    case CelestialInteractionKind::SurveyContract: return "Survey Contract";
    }
    return "Unknown";
}

CelestialInteractionReport EvaluateCelestialInteractions(const std::vector<CelestialInteractionZone>& zones, const std::string& bodyId, float distance, float scannerRating) {
    CelestialInteractionReport report;
    report.bodyId = bodyId;
    for (const auto& zone : zones) {
        if (zone.bodyId != bodyId) continue;
        if (!zone.available) { report.blockedReasons.push_back(std::string(CelestialInteractionKindName(zone.kind)) + " unavailable"); continue; }
        if (distance > zone.radius) { report.blockedReasons.push_back(std::string(CelestialInteractionKindName(zone.kind)) + " out of range"); continue; }
        if (scannerRating + 0.01f < zone.requiredScanner) { report.blockedReasons.push_back(std::string(CelestialInteractionKindName(zone.kind)) + " needs better scanner"); continue; }
        report.availableInteractions.push_back(zone.kind);
    }
    return report;
}

std::vector<CelestialInteractionZone> CreateDefaultCelestialInteractionZones(const std::string& bodyId, float visualRadius) {
    const float base = visualRadius > 1.0f ? visualRadius : 64.0f;
    return {
        {bodyId, CelestialInteractionKind::Scan, base * 9.0f, 0.0f, true},
        {bodyId, CelestialInteractionKind::Orbit, base * 4.0f, 0.0f, true},
        {bodyId, CelestialInteractionKind::Harvest, base * 2.5f, 2.0f, true},
        {bodyId, CelestialInteractionKind::SurveyContract, base * 6.0f, 1.0f, true},
    };
}

} // namespace subspace
