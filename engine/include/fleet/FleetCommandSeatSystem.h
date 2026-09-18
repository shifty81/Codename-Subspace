#pragma once

#include <cstdint>
#include <string>

namespace subspace {

enum class FleetSeatKind { StationTerminal, ShipCommandSeat, PilotSeat };
enum class FleetSeatDenial { None, MissingIdentity, Unoccupied, Unauthorized, Unpowered, NoCommandSystem, LinkOffline, Busy, SessionMismatch };
enum class FleetSeatReturnView { OnFoot, Cockpit };

// Supplied from authoritative world/seat state. A request cannot authorize itself:
// occupiedBy, accessAllowed and connectivity must be resolved by the world authority.
struct FleetCommandSeatSnapshot {
    std::uint64_t seatId=0;
    std::uint64_t occupiedBy=0;
    std::uint64_t hostEntityId=0;
    FleetSeatKind kind=FleetSeatKind::StationTerminal;
    bool accessAllowed=false;
    bool powered=false;
    bool commandSystemInstalled=false;
    bool commandLinkOnline=false;
};
struct FleetCommandSession {
    bool active=false;
    std::uint64_t actorId=0;
    std::uint64_t seatId=0;
    std::uint64_t hostEntityId=0;
    FleetSeatReturnView returnView=FleetSeatReturnView::OnFoot;
};
struct FleetSeatResult {
    bool allowed=false;
    FleetSeatDenial denial=FleetSeatDenial::None;
};
class FleetCommandSeatSystem final {
public:
    static FleetSeatResult Validate(std::uint64_t actorId,const FleetCommandSeatSnapshot& seat){
        if(!actorId || !seat.seatId || !seat.hostEntityId)return {false,FleetSeatDenial::MissingIdentity};
        if(seat.occupiedBy!=actorId)return {false,FleetSeatDenial::Unoccupied};
        if(!seat.accessAllowed)return {false,FleetSeatDenial::Unauthorized};
        if(!seat.powered)return {false,FleetSeatDenial::Unpowered};
        if(!seat.commandSystemInstalled)return {false,FleetSeatDenial::NoCommandSystem};
        if(!seat.commandLinkOnline)return {false,FleetSeatDenial::LinkOffline};
        return {true,FleetSeatDenial::None};
    }
    static FleetSeatResult Enter(FleetCommandSession& session,std::uint64_t actorId,
                                const FleetCommandSeatSnapshot& seat){
        const auto result=Validate(actorId,seat);
        if(!result.allowed)return result;
        if(session.active && (session.actorId!=actorId || session.seatId!=seat.seatId ||
                              session.hostEntityId!=seat.hostEntityId))
            return {false,FleetSeatDenial::Busy};
        session.active=true;
        session.actorId=actorId;
        session.seatId=seat.seatId;
        session.hostEntityId=seat.hostEntityId;
        session.returnView=seat.kind==FleetSeatKind::StationTerminal?
            FleetSeatReturnView::OnFoot:FleetSeatReturnView::Cockpit;
        return {true,FleetSeatDenial::None};
    }
    // Revalidate on every order/control use; powering down or losing comms revokes capabilities.
    static FleetSeatResult CanCommand(const FleetCommandSession& session,std::uint64_t actorId,
                                     const FleetCommandSeatSnapshot& seat){
        if(!session.active || session.actorId!=actorId || session.seatId!=seat.seatId ||
           session.hostEntityId!=seat.hostEntityId)return {false,FleetSeatDenial::SessionMismatch};
        return Validate(actorId,seat);
    }
    static FleetSeatResult Exit(FleetCommandSession& session,std::uint64_t actorId){
        if(!session.active || !actorId || session.actorId!=actorId)
            return {false,FleetSeatDenial::SessionMismatch};
        session=FleetCommandSession{};
        return {true,FleetSeatDenial::None};
    }
};
} // namespace subspace
