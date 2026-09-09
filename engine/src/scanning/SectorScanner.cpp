#include "scanning/SectorScanner.h"

#include <algorithm>
#include <sstream>

namespace subspace {

SectorScannerReport SectorScanner::ScanSystem(const StarSystemDefinition& system,
                                               const SectorResourceSurvey& survey,
                                               int scannerTier) const
{
    SectorScannerReport report;
    report.sectorId = system.id;
    report.systemSummary = StarSystemSummary(system);

    auto addBody = [&](const CelestialBodyDefinition& body) {
        SectorScanContact contact;
        contact.kind = ScannerContactKind::CelestialBody;
        contact.id = body.id;
        contact.displayName = body.displayName;
        contact.summary = CelestialBodySummary(body);
        contact.signalStrength = std::max(1, body.resourceRichness) * 10;
        contact.hazardous = body.isHazardous;
        report.contacts.push_back(contact);
    };

    addBody(system.primary);
    for (const auto& body : system.bodies) {
        addBody(body);
    }

    SectorScanContact field;
    field.kind = ScannerContactKind::ResourceField;
    field.id = survey.asteroidField.id;
    field.displayName = "Asteroid Resource Field";
    field.summary = AsteroidFieldSummary(survey.asteroidField);
    field.signalStrength = survey.asteroidField.resourceRichness * 12;
    field.hazardous = survey.asteroidField.hazardRating > 0.25f || survey.asteroidField.pirateRisk > 0.35f;
    report.contacts.push_back(field);

    if (scannerTier >= 1) {
        for (const auto& hazard : survey.hazards) {
            SectorScanContact contact;
            contact.kind = ScannerContactKind::Hazard;
            contact.id = hazard.tag;
            contact.displayName = hazard.tag;
            contact.summary = hazard.source;
            contact.signalStrength = hazard.severity * 10;
            contact.hazardous = true;
            report.contacts.push_back(contact);
        }
    }

    if (scannerTier >= 2) {
        for (const auto& hint : survey.economyHints) {
            SectorScanContact contact;
            contact.kind = ScannerContactKind::StationEconomy;
            contact.id = hint.commodity;
            contact.displayName = hint.commodity;
            std::ostringstream s;
            s << "demand=" << hint.demand << " supply=" << hint.supply << " pricex=" << hint.priceMultiplier << " " << hint.reason;
            contact.summary = s.str();
            contact.signalStrength = std::max(hint.demand, hint.supply) * 8;
            report.contacts.push_back(contact);
        }
    }

    std::sort(report.contacts.begin(), report.contacts.end(), [](const auto& a, const auto& b) {
        return a.signalStrength > b.signalStrength;
    });

    report.recommendedActions.push_back("Survey top resource tags before committing mining time.");
    if (survey.asteroidField.pirateRisk > 0.3f) {
        report.recommendedActions.push_back("Pirate risk detected: keep shields charged before deep-belt mining.");
    }
    if (survey.asteroidField.resourceRichness >= 7) {
        report.recommendedActions.push_back("High-richness asteroid field: prioritize cargo capacity and salvage tools.");
    }
    return report;
}

std::string ScannerContactKindName(ScannerContactKind kind)
{
    switch (kind) {
        case ScannerContactKind::CelestialBody: return "Body";
        case ScannerContactKind::ResourceField: return "Field";
        case ScannerContactKind::Hazard: return "Hazard";
        case ScannerContactKind::StationEconomy: return "Economy";
        default: return "Unknown";
    }
}

std::vector<std::string> FormatScannerReportLines(const SectorScannerReport& report, std::size_t maxLines)
{
    std::vector<std::string> lines;
    lines.push_back(report.systemSummary);
    for (const auto& contact : report.contacts) {
        if (lines.size() >= maxLines) {
            break;
        }
        lines.push_back(ScannerContactKindName(contact.kind) + ": " + contact.displayName + " - " + contact.summary);
    }
    return lines;
}

} // namespace subspace
