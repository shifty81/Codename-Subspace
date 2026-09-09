#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
namespace subspace {
enum class PersistentEventType { ResourceDepleted, ShipDestroyed, WreckCreated, StationDamaged, StationRepaired, InfrastructureBuilt, MarketShock, DiscoveryRecorded };
struct PersistentUniverseEvent { std::uint64_t sequence=0; PersistentEventType type=PersistentEventType::DiscoveryRecorded; std::string regionId; std::string entityId; double value=0; };
class PersistentUniverseSystem {
public:
 std::uint64_t Record(PersistentEventType type,const std::string& region,const std::string& entity,double value=0);
 double ResourceRemaining(const std::string& entity,double initial) const;
 bool IsDestroyed(const std::string& entity) const;
 std::vector<PersistentUniverseEvent> EventsForRegion(const std::string& region) const;
 std::size_t EventCount() const{return events_.size();}
 std::string Serialize() const;
 bool Deserialize(const std::string& text);
private:std::uint64_t next_=1;std::vector<PersistentUniverseEvent> events_;
};
}
