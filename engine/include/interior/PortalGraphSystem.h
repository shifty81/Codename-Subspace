#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace subspace {
using CompartmentId=std::uint32_t;
constexpr CompartmentId ExteriorCompartmentId=0;
struct CompartmentNode {
    CompartmentId id=ExteriorCompartmentId;
    std::string name;
    double volumeM3=0.0;
    double pressureKpa=0.0;
    double oxygenFraction=0.0;
};
struct CompartmentPortal {
    std::uint32_t id=0;
    CompartmentId a=ExteriorCompartmentId,b=ExteriorCompartmentId;
    bool open=false,sealed=true,damaged=false;
    bool pressureCapable=true,navEnabled=true,visibilityEnabled=true;
};
class PortalGraphSystem {
public:
    static bool IsConnected(CompartmentId from,CompartmentId to,const std::vector<CompartmentPortal>& portals,bool requireOpen=true);
    static bool HasExteriorLeak(CompartmentId from,const std::vector<CompartmentPortal>& portals);
};
} // namespace subspace
