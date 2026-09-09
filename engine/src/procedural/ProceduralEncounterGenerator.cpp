#include "procedural/ProceduralEncounterGenerator.h"

#include "celestial/CelestialSystemGenerator.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace subspace {
namespace {
bool HasTag(const std::vector<std::string>& tags,const std::string& needle){return std::find(tags.begin(),tags.end(),needle)!=tags.end();}
bool ThreatLike(const EncounterArchetype& a){return a.disposition==EncounterDisposition::Hostile||a.disposition==EncounterDisposition::Pirate||HasTag(a.tags,"hostile")||HasTag(a.tags,"raid")||HasTag(a.tags,"combat");}
bool OpportunityLike(const EncounterArchetype& a){return a.disposition==EncounterDisposition::Friendly||HasTag(a.tags,"trader")||HasTag(a.tags,"salvage")||HasTag(a.tags,"rescue")||HasTag(a.tags,"opportunity");}
int EffectivePower(const EncounterDirectorState&s){return std::max(1,s.fleetPower+s.crewCount/3+s.stations*2+s.colonies*3+s.economicWealth/500);}
}

GeneratedEncounter ProceduralEncounterGenerator::GenerateEncounter(const EncounterSpawnTable& table,
                                                                    int playerPower,
                                                                    std::uint32_t seed)
const {
    if (table.encounters.empty()) {
        return {"empty", "Quiet Sector", EncounterDisposition::Neutral, 0, {"empty"}};
    }

    int totalWeight = 0;
    for (const auto& archetype : table.encounters) {
        totalWeight += std::max(1, archetype.weight);
    }
    int roll = static_cast<int>(CelestialSeededUnit(seed, 8) * static_cast<float>(std::max(1, totalWeight)));
    const EncounterArchetype* selected = &table.encounters.front();
    for (const auto& archetype : table.encounters) {
        roll -= std::max(1, archetype.weight);
        if (roll <= 0) {
            selected = &archetype;
            break;
        }
    }

    GeneratedEncounter encounter;
    encounter.id = table.sectorId + ":" + selected->id;
    encounter.displayName = selected->displayName;
    encounter.disposition = selected->disposition;
    encounter.threat = std::max(1, selected->threat + playerPower / 3);
    encounter.tags = selected->tags;
    return encounter;
}

DirectedEncounterResult ProceduralEncounterGenerator::GenerateDirectedEncounter(const EncounterSpawnTable& table,
                                                                                 const EncounterDirectorState& state,
                                                                                 const EncounterDirectorContext& context,
                                                                                 std::uint32_t seed) const {
    DirectedEncounterResult out;
    if(table.encounters.empty()){out.encounter={"empty","Quiet Sector",EncounterDisposition::Neutral,0,{"empty"}};out.reasons.push_back("No encounter archetypes are legal in this location");return out;}
    if(context.playerInCombat){out.reasons.push_back("Existing combat suppresses a new storyteller incident");return out;}

    const double threatReadiness=std::clamp(state.daysSinceMajorThreat/3.0,0.0,1.5);
    const double opportunityReadiness=std::clamp(state.daysSinceOpportunity/2.0,0.0,1.5);
    const double tension=std::clamp(state.tension,0.0,1.0);
    const double recovery=std::clamp(state.recoveryNeed,0.0,1.0);
    const double baseSpawn=0.18+0.42*std::max(threatReadiness,opportunityReadiness)+0.25*tension;
    const double spawnRoll=CelestialSeededUnit(seed,31);
    if(spawnRoll>std::clamp(baseSpawn,0.05,0.92)){out.reasons.push_back("Storyteller selected a quiet beat to preserve pacing");return out;}

    std::vector<int> weights;weights.reserve(table.encounters.size());int total=0;
    for(const auto&a:table.encounters){
        double w=std::max(1,a.weight);
        const bool threat=ThreatLike(a),opp=OpportunityLike(a);
        if(threat){w*=0.30+1.35*threatReadiness+1.10*tension;w*=1.0-0.65*recovery;if(state.daysSinceMajorThreat<0.5)w*=0.08;}
        if(opp){w*=0.45+1.15*opportunityReadiness+1.25*recovery;if(state.daysSinceOpportunity<0.35)w*=0.20;}
        if(context.docked&&threat)w*=0.45;
        if(context.interior&&HasTag(a.tags,"space_only"))w=0.0;
        if(context.planetary&&HasTag(a.tags,"space_only"))w=0.0;
        if(context.deepSpace&&HasTag(a.tags,"planet_only"))w=0.0;
        if(state.storyEnabled&&!context.storyTags.empty()){
            for(const auto&t:context.storyTags)if(HasTag(a.tags,t))w*=1.75;
        }
        for(const auto&t:state.recentTags)if(HasTag(a.tags,t))w*=0.62; // variety pressure
        const int wi=std::max(0,static_cast<int>(std::round(w)));weights.push_back(wi);total+=wi;
    }
    if(total<=0){out.reasons.push_back("Pacing/context filters suppressed every legal encounter");return out;}
    int roll=static_cast<int>(CelestialSeededUnit(seed,47)*float(total));std::size_t selected=0;
    for(std::size_t i=0;i<weights.size();++i){roll-=weights[i];if(roll<=0){selected=i;break;}}
    const auto&a=table.encounters[selected];const int power=EffectivePower(state);
    out.encounter.id=(context.locationId.empty()?table.sectorId:context.locationId)+":"+a.id;
    out.encounter.displayName=a.displayName;out.encounter.disposition=a.disposition;out.encounter.tags=a.tags;
    const double wealthPressure=std::clamp(state.economicWealth/5000.0,0.0,3.0);
    const double difficulty=1.0+0.20*tension+0.08*wealthPressure+0.04*std::max(0,state.storyPhase);
    out.adjustedThreat=std::max(1,static_cast<int>(std::round((a.threat+power/3)*difficulty)));
    out.encounter.threat=out.adjustedThreat;out.shouldSpawn=true;
    if(ThreatLike(a)){out.nextThreatCooldownDays=1.25+CelestialSeededUnit(seed,61)*1.75;out.reasons.push_back("Threat selected from tension, recovery state, wealth and time-since-threat budget");}
    else if(OpportunityLike(a)){out.nextOpportunityCooldownDays=.65+CelestialSeededUnit(seed,67)*1.15;out.reasons.push_back("Opportunity selected to vary pressure and support sandbox recovery");}
    else out.reasons.push_back("Neutral/world incident selected by location-aware storyteller weighting");
    if(state.storyEnabled&&!context.storyTags.empty())out.reasons.push_back("Story spine supplied soft tags; sandbox legality remained authoritative");
    return out;
}

std::string GeneratedEncounterSummary(const GeneratedEncounter& encounter)
{
    std::ostringstream stream;
    stream << encounter.displayName << " [" << EncounterDispositionName(encounter.disposition) << "] threat=" << encounter.threat;
    return stream.str();
}

} // namespace subspace
