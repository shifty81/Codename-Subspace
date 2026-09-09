#pragma once
#include <string>
#include <unordered_map>
namespace subspace {
enum class AdvancedWeaponFamily { Kinetic, Rail, Coil, Beam, Plasma, Missile, Torpedo, Flak, PointDefense };
struct CombatWeaponSpec { std::string id; AdvancedWeaponFamily family=AdvancedWeaponFamily::Kinetic; double damage=10; double heatPerShot=1; double powerPerShot=1; int ammoPerShot=1; double cooldownSeconds=0.25; int magazineCapacity=100; };
struct CombatWeaponRuntime { CombatWeaponSpec spec; int ammo=0; double cooldownRemaining=0; double accumulatedHeat=0; bool enabled=true; };
struct CombatFireResult { bool fired=false; std::string reason; double damage=0; double heatGenerated=0; double powerConsumed=0; int ammoConsumed=0; };
class AdvancedCombatSystem {
public:
 bool Register(const CombatWeaponSpec& spec,int initialAmmo=-1);
 CombatFireResult Fire(const std::string& id,double availablePower);
 void Tick(double seconds);
 bool Reload(const std::string& id,int rounds);
 bool SetEnabled(const std::string& id,bool enabled);
 const CombatWeaponRuntime* Get(const std::string& id) const;
private: std::unordered_map<std::string,CombatWeaponRuntime> weapons_;
};
} // namespace subspace
