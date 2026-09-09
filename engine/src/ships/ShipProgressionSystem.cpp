#include "ships/ShipProgressionSystem.h"
#include <algorithm>
namespace subspace {
ShipLicenseProfile ShipProgressionSystem::LicenseFor(ShipClass c){
    ShipLicenseProfile p;p.shipClass=c;
    switch(c){
        case ShipClass::Fighter:case ShipClass::Shuttle:p={"LIGHT_CRAFT",c,1,1,0,0};break;
        case ShipClass::Corvette:p={"CORVETTE",c,1,2,0,0};break;
        case ShipClass::Frigate:p={"FRIGATE",c,2,2,0,0};break;
        case ShipClass::Destroyer:p={"DESTROYER",c,3,3,1,0};break;
        case ShipClass::Cruiser:p={"CRUISER",c,4,4,2,0};break;
        case ShipClass::Battlecruiser:p={"BATTLECRUISER",c,5,5,3,0};break;
        case ShipClass::Battleship:p={"BATTLESHIP",c,6,6,4,0};break;
        case ShipClass::Carrier:p={"CARRIER",c,7,7,6,0};break;
        case ShipClass::Dreadnought:p={"DREADNOUGHT",c,8,8,7,0};break;
        case ShipClass::IndustrialCapital:p={"INDUSTRIAL_CAPITAL",c,7,6,4,7};break;
        case ShipClass::Capital:p={"CAPITAL",c,9,9,8,5};break;
        case ShipClass::Freighter:p={"FREIGHTER",c,4,4,1,4};break;
        case ShipClass::Miner:p={"MINING_VESSEL",c,2,2,0,2};break;
        case ShipClass::Explorer:p={"SURVEY_VESSEL",c,2,3,0,0};break;
    }
    return p;
}
ShipProgressionGate ShipProgressionSystem::CanPilot(const CharacterShipTraining& ch,ShipClass c){
    const auto p=LicenseFor(c);ShipProgressionGate g;g.requiredLicense=p.licenseId;
    if(ch.trainingRank<p.trainingRank)g.blockers.push_back("training rank "+std::to_string(p.trainingRank));
    if(ch.navigationRank<p.navigationRank)g.blockers.push_back("navigation rank "+std::to_string(p.navigationRank));
    if(ch.commandRank<p.commandRank)g.blockers.push_back("command rank "+std::to_string(p.commandRank));
    if(ch.industryRank<p.industryRank)g.blockers.push_back("industry rank "+std::to_string(p.industryRank));
    if(std::find(ch.licenses.begin(),ch.licenses.end(),p.licenseId)==ch.licenses.end())g.blockers.push_back("license "+p.licenseId);
    g.allowed=g.blockers.empty();return g;
}
std::vector<ShipClass> ShipProgressionSystem::CoreSizeProgression(){return {ShipClass::Shuttle,ShipClass::Corvette,ShipClass::Frigate,ShipClass::Destroyer,ShipClass::Cruiser,ShipClass::Battlecruiser,ShipClass::Battleship,ShipClass::Carrier,ShipClass::Dreadnought,ShipClass::IndustrialCapital};}
} // namespace subspace
