#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class CelestialInteractionKind {
    Scan,
    Orbit,
    Harvest,
    Dock,
    SurveyContract
};

struct CelestialInteractionZone {
    std::string bodyId;
    CelestialInteractionKind kind = CelestialInteractionKind::Scan;
    float radius = 100.0f;
    float requiredScanner = 0.0f;
    bool available = true;
};

struct CelestialInteractionReport {
    std::string bodyId;
    std::vector<CelestialInteractionKind> availableInteractions;
    std::vector<std::string> blockedReasons;
};

const char* CelestialInteractionKindName(CelestialInteractionKind kind);
CelestialInteractionReport EvaluateCelestialInteractions(const std::vector<CelestialInteractionZone>& zones, const std::string& bodyId, float distance, float scannerRating);
std::vector<CelestialInteractionZone> CreateDefaultCelestialInteractionZones(const std::string& bodyId, float visualRadius);

} // namespace subspace
