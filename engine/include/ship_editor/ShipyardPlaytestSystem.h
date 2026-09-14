#pragma once

#include "interior/ShipEmbodimentSystem.h"
#include "rendering/FirstPersonViewSystem.h"
#include <cstdint>
#include <string>

namespace subspace {

enum class ShipyardPlaytestMode { Editing, InteriorWalk, CockpitFlight, RemoteFleetCommand };

struct ShipyardPlaytestState {
    ShipyardPlaytestMode mode=ShipyardPlaytestMode::Editing;
    std::uint64_t shipId=0;
    bool active=false;
    bool authoredStateDirty=false;
    std::string status="EDITING";
};

/// Non-destructive PIE bridge. Shipyard keeps editing the same canonical ship
/// definition while Play temporarily grants embodied FPS/cockpit/fleet-control
/// views over a runtime instance derived from that definition.
class ShipyardPlaytestSystem {
public:
    bool BeginInterior(std::uint64_t persistentShipId,const InteriorTraversalBounds& bounds={});
    bool TakeCockpit();
    bool OpenRemoteFleetCommand();
    void ReturnToEditing();
    void MoveOnFoot(float forward,float strafe,double seconds);
    void LookOnFoot(float yawDeltaRadians,float pitchDeltaRadians);
    FirstPersonViewPose CurrentOnFootView() const;
    const ShipyardPlaytestState& State() const{return state_;}
    const ShipEmbodimentSystem& Embodiment() const{return embodiment_;}
private:
    ShipyardPlaytestState state_{};
    ShipEmbodimentSystem embodiment_{};
};

} // namespace subspace
