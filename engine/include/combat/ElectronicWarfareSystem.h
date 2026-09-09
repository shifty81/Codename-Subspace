#pragma once
#include <string>
#include <unordered_map>
#include <vector>
namespace subspace {
enum class EWarEffectType { SensorJam, TrackingDisrupt, MissileDisrupt, PropulsionSuppress, PowerDrain, CommunicationJam, DroneDisrupt, TargetPaint };
struct EWarEffect { std::string id; EWarEffectType type=EWarEffectType::SensorJam; double strength=0; double remainingSeconds=0; std::string sourceId; };
class ElectronicWarfareSystem {
public:
 bool Apply(const EWarEffect& effect);
 void Tick(double seconds);
 double CombinedStrength(EWarEffectType type) const;
 double CapabilityMultiplier(EWarEffectType type) const;
 bool Remove(const std::string& id);
 std::size_t ActiveCount() const{return effects_.size();}
private: std::unordered_map<std::string,EWarEffect> effects_;
};
}
