#include "ui/FrontendFlowSystem.h"

namespace subspace {
bool FrontendFlowSystem::BeginNewSandbox(const NewSandboxConfig& c){if(c.commanderName.empty()||c.corporationName.empty())return false;config_=c;screen_=FrontendScreen::StartingShip;return true;}
StarterSelection FrontendFlowSystem::BuildStarter(StarterCareer career) const {ShipConstructionSystem ships;StarterSelection s;s.career=career;s.design=(career==StarterCareer::Scrapper)?ships.StarterScrapper():ships.StarterProspector();switch(career){case StarterCareer::Prospector:s.startingEquipment={"mining_laser_s","fracture_missile_rack","ore_scanner"};break;case StarterCareer::Scrapper:s.startingEquipment={"salvage_beam_s","tractor_s","salvage_drone"};break;case StarterCareer::Pathfinder:s.startingEquipment={"survey_scanner","light_autocannon"};break;case StarterCareer::Defender:s.startingEquipment={"light_autocannon","point_defense_s"};break;case StarterCareer::Custom:s.startingEquipment={};break;}return s;}
bool FrontendFlowSystem::ConfirmStarter(const StarterSelection& s){ShipConstructionSystem ships;if(!ships.Validate(s.design).valid)return false;selection_=s;screen_=FrontendScreen::StationHangar;return true;}
} // namespace subspace
