#include "celestial/SectorResourceModel.h"

#include "celestial/CelestialSystemGenerator.h"

#include <algorithm>
#include <map>
#include <sstream>

namespace subspace {
namespace {

void AddResource(std::map<std::string, SectorResourceEntry>& entries,
                 const std::string& tag,
                 int amount,
                 bool hazardous)
{
    auto& entry = entries[tag];
    entry.tag = tag;
    entry.abundance += std::max(1, amount);
    entry.sourceBodies += 1;
    entry.hazardousSource = entry.hazardousSource || hazardous;
}

void AddHazard(std::vector<SectorHazardEntry>& hazards, const std::string& tag, int severity, const std::string& source)
{
    auto found = std::find_if(hazards.begin(), hazards.end(), [&](const SectorHazardEntry& h) { return h.tag == tag; });
    if (found == hazards.end()) {
        hazards.push_back({tag, severity, source});
    }
    else {
        found->severity = std::max(found->severity, severity);
        if (found->source.find(source) == std::string::npos) {
            found->source += ", " + source;
        }
    }
}

SectorStationEconomyHint MakeHint(const std::string& commodity, int demand, int supply, float multiplier, const std::string& reason)
{
    SectorStationEconomyHint hint;
    hint.commodity = commodity;
    hint.demand = demand;
    hint.supply = supply;
    hint.priceMultiplier = multiplier;
    hint.reason = reason;
    return hint;
}

} // namespace

SectorResourceSurvey BuildSectorResourceSurvey(const StarSystemDefinition& system)
{
    SectorResourceSurvey survey;
    survey.sectorId = system.id;
    survey.seed = system.seed;
    survey.asteroidField.id = system.id + ":asteroid-field";

    std::map<std::string, SectorResourceEntry> resources;
    auto visitBody = [&](const CelestialBodyDefinition& body) {
        const int amount = std::max(1, body.resourceRichness);
        for (const auto& tag : body.resourceTags) {
            AddResource(resources, tag, amount, body.isHazardous);
        }
        if (body.isHazardous) {
            AddHazard(survey.hazards, CelestialBodyTypeName(body.type), amount, body.displayName);
        }
        if (body.type == CelestialBodyType::AsteroidBelt) {
            survey.asteroidField.resourceRichness = std::max(survey.asteroidField.resourceRichness, body.resourceRichness);
            survey.asteroidField.resourceTags = body.resourceTags;
            survey.asteroidField.pirateRisk += 0.12f;
            survey.asteroidField.salvageChance += 0.18f;
        }
        if (body.type == CelestialBodyType::LavaWorld || body.type == CelestialBodyType::BlackHole) {
            survey.asteroidField.hazardRating += 0.16f;
        }
        if (body.type == CelestialBodyType::GasGiant || body.type == CelestialBodyType::RingedGasGiant) {
            survey.economyHints.push_back(MakeHint("fuel", 6, amount, 0.82f, body.displayName + " supplies volatiles"));
        }
        if (body.type == CelestialBodyType::TerranPlanet || body.type == CelestialBodyType::RiverWorld) {
            survey.economyHints.push_back(MakeHint("food", 7, amount, 0.88f, body.displayName + " supports settlements"));
            survey.economyHints.push_back(MakeHint("ore", 7, 2, 1.18f, body.displayName + " imports industrial feedstock"));
        }
        if (body.type == CelestialBodyType::LavaWorld) {
            survey.economyHints.push_back(MakeHint("rare-metals", 4, amount, 0.94f, body.displayName + " has high-temperature extraction"));
        }
    };

    visitBody(system.primary);
    for (const auto& body : system.bodies) {
        visitBody(body);
    }

    for (const auto& kv : resources) {
        survey.resources.push_back(kv.second);
    }
    std::sort(survey.resources.begin(), survey.resources.end(), [](const auto& a, const auto& b) {
        if (a.abundance != b.abundance) {
            return a.abundance > b.abundance;
        }
        return a.tag < b.tag;
    });

    if (survey.asteroidField.resourceTags.empty()) {
        survey.asteroidField.resourceTags = TopSectorResourceTags(survey, 4);
    }
    if (survey.asteroidField.resourceTags.empty()) {
        survey.asteroidField.resourceTags = {"ore", "salvage"};
    }
    survey.asteroidField.dominantResource = survey.asteroidField.resourceTags.front();
    survey.asteroidField.resourceRichness = std::max(1, survey.asteroidField.resourceRichness + static_cast<int>(survey.resources.size() / 3));
    survey.asteroidField.miningYieldMultiplier = 0.75f + static_cast<float>(survey.asteroidField.resourceRichness) * 0.08f;
    survey.asteroidField.salvageChance = std::min(0.72f, survey.asteroidField.salvageChance + static_cast<float>(survey.asteroidField.resourceRichness) * 0.018f);
    survey.asteroidField.pirateRisk = std::min(0.85f, survey.asteroidField.pirateRisk + survey.asteroidField.hazardRating * 0.6f);

    if (survey.economyHints.empty()) {
        survey.economyHints.push_back(MakeHint("ore", 5, survey.asteroidField.resourceRichness, 1.0f, "baseline frontier station demand"));
        survey.economyHints.push_back(MakeHint("salvage", 3, 1, 1.05f, "frontier repair demand"));
    }

    return survey;
}

std::string PickPrimaryResourceTag(const SectorResourceSurvey& survey, std::uint32_t seed, int index)
{
    const auto tags = survey.asteroidField.resourceTags.empty() ? TopSectorResourceTags(survey, 4) : survey.asteroidField.resourceTags;
    if (tags.empty()) {
        return "ore";
    }
    const std::size_t selected = static_cast<std::size_t>((seed + static_cast<std::uint32_t>(index * 17)) % tags.size());
    return tags[selected];
}

std::string SectorResourceSummary(const SectorResourceSurvey& survey)
{
    std::ostringstream stream;
    stream << survey.sectorId << " resources=";
    const auto tags = TopSectorResourceTags(survey, 5);
    for (std::size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) {
            stream << ",";
        }
        stream << tags[i];
    }
    stream << " field=" << AsteroidFieldSummary(survey.asteroidField);
    return stream.str();
}

std::string AsteroidFieldSummary(const AsteroidFieldProfile& field)
{
    std::ostringstream stream;
    stream << field.dominantResource << " richness=" << field.resourceRichness
           << " yieldx=" << field.miningYieldMultiplier
           << " salvage=" << field.salvageChance
           << " risk=" << field.pirateRisk;
    return stream.str();
}

std::vector<std::string> TopSectorResourceTags(const SectorResourceSurvey& survey, std::size_t maxTags)
{
    std::vector<std::string> tags;
    for (const auto& resource : survey.resources) {
        if (tags.size() >= maxTags) {
            break;
        }
        tags.push_back(resource.tag);
    }
    return tags;
}

} // namespace subspace
