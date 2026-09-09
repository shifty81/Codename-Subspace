#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class CorporationPermission : std::uint64_t { Pilot=1ull<<0, Industry=1ull<<1, Build=1ull<<2, Logistics=1ull<<3, Market=1ull<<4, FleetCommand=1ull<<5, Storage=1ull<<6, StationManager=1ull<<7, Admin=1ull<<8 };
enum class CrewSpecialty { Pilot, Engineer, Miner, Salvager, Gunner, Logistics, Industry, Explorer, FleetOfficer };
struct CorporationMemberNative { std::uint64_t id=0; std::string name; CrewSpecialty specialty=CrewSpecialty::Pilot; int skill=1; std::uint64_t permissions=0; double salary=0; std::uint64_t assignedShip=0; };
class CorporationSystem {
public:
    std::uint64_t Hire(const std::string& name,CrewSpecialty specialty,int skill,double salary);
    bool SetPermission(std::uint64_t memberId,CorporationPermission permission,bool enabled);
    bool HasPermission(std::uint64_t memberId,CorporationPermission permission) const;
    bool AssignShip(std::uint64_t memberId,std::uint64_t shipId);
    const CorporationMemberNative* Get(std::uint64_t id) const;
    double Payroll() const;
private:
    std::uint64_t nextId_=1; std::unordered_map<std::uint64_t,CorporationMemberNative> members_;
};

} // namespace subspace
