#pragma once

#include "celestial/CelestialTypes.h"
#include "celestial/SectorResourceModel.h"

#include <string>
#include <vector>

namespace subspace {

enum class ScannerContactKind {
    CelestialBody,
    ResourceField,
    Hazard,
    StationEconomy
};

struct SectorScanContact {
    ScannerContactKind kind = ScannerContactKind::CelestialBody;
    std::string id;
    std::string displayName;
    std::string summary;
    int signalStrength = 0;
    bool hazardous = false;
};

struct SectorScannerReport {
    std::string sectorId;
    std::string systemSummary;
    std::vector<SectorScanContact> contacts;
    std::vector<std::string> recommendedActions;
};

class SectorScanner {
public:
    SectorScannerReport ScanSystem(const StarSystemDefinition& system,
                                   const SectorResourceSurvey& survey,
                                   int scannerTier = 1) const;
};

std::string ScannerContactKindName(ScannerContactKind kind);
std::vector<std::string> FormatScannerReportLines(const SectorScannerReport& report, std::size_t maxLines = 8);

} // namespace subspace
