#pragma once

#include <cstdint>
#include <string>

namespace subspace {

enum class FleetSeatKind { StationTerminal, ShipCommandSeat, PilotSeat };
enum class FleetSeatDenial { None, MissingIdentity, Unoccupied, Unauthorized, Unpowered, NoCommandSystem, LinkOffline, Busy, SessionMismatch, InvalidReturnView };
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
    // Capture the actual originating embodiment. A walk-up command console
    // on a ship is not a pilot chair and must return to on-foot first person.
    static FleetSeatResult Enter(FleetCommandSession& session,std::uint64_t actorId,
                                const FleetCommandSeatSnapshot& seat,FleetSeatReturnView origin){
        if(session.active){
            if(session.actorId!=actorId || session.seatId!=seat.seatId ||
               session.hostEntityId!=seat.hostEntityId)return {false,FleetSeatDenial::Busy};
            // Idempotent entry is still required to validate current seat state.
            // Never rewrite the captured return view after a session begins.
            return Validate(actorId,seat);
        }
        const auto result=Validate(actorId,seat);
        if(!result.allowed)return result;
        if(seat.kind==FleetSeatKind::PilotSeat && origin!=FleetSeatReturnView::Cockpit)
            return {false,FleetSeatDenial::InvalidReturnView};
        session.active=true;
        session.actorId=actorId;
        session.seatId=seat.seatId;
        session.hostEntityId=seat.hostEntityId;
        session.returnView=origin;
        return {true,FleetSeatDenial::None};
    }
    // Compatibility for existing callers: non-pilot command seats are walk-up
    // terminals unless the authoritative caller explicitly passes Cockpit.
    static FleetSeatResult Enter(FleetCommandSession& session,std::uint64_t actorId,
                                const FleetCommandSeatSnapshot& seat){
        return Enter(session,actorId,seat,seat.kind==FleetSeatKind::PilotSeat?
            FleetSeatReturnView::Cockpit:FleetSeatReturnView::OnFoot);
    }
    // Revalidate on every order/control use; powering down or losing comms revokes capabilities.
    static FleetSeatResult CanCommand(const FleetCommandSession& session,std::uint64_t actorId,
                                     const FleetCommandSeatSnapshot& seat){
        if(!session.active || session.actorId!=actorId || session.seatId!=seat.seatId ||
           session.hostEntityId!=seat.hostEntityId)return {false,FleetSeatDenial::SessionMismatch};
        return Validate(actorId,seat);
    }
    static FleetSeatResult Exit(FleetCommandSession& session,std::uint64_t actorId,
                                FleetSeatReturnView* restoredView=nullptr){
        if(!session.active || !actorId || session.actorId!=actorId)
            return {false,FleetSeatDenial::SessionMismatch};
        if(restoredView)*restoredView=session.returnView;
        session=FleetCommandSession{};
        return {true,FleetSeatDenial::None};
    }
};
} // namespace subspace
