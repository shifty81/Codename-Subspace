#pragma once

#include "ship_editor/ShipyardBuilderMigrationSystem.h"
#include "ship_editor/ShipyardCommandSystem.h"
#include "ship_editor/ShipyardProfessionalShellSystem.h"

#include <string_view>

namespace subspace {

/// Strangler boundary used by new Shipyard UI/Cortex/Forge integrations.
/// Stable command IDs are public; ShipyardBuilderCommand stays an internal
/// compatibility detail until parity migration finishes.
class ShipyardProfessionalController {
public:
    void Bind(ShipyardBuilderSystem& builder,std::string persistentShipId={});
    bool IsBound()const{return builder_!=nullptr&&migration_.bound;}
    ShipyardCommandResult Execute(std::string_view commandId,int value=0);
    void Pull();

    const ShipyardDocument& Document()const{return migration_.document;}
    const ShipyardSession& Session()const{return migration_.session;}
    ShipyardSession& Session(){return migration_.session;}
    ShipyardHistorySystem& History(){return migration_.history;}
    const ShipyardCommandSystem& Commands()const{return commands_;}
    const ShipyardProfessionalShellState& Shell()const{return shell_;}
    ShipyardProfessionalShellState& Shell(){return shell_;}
    void RebuildShell();

private:
    ShipyardCommandResult ExecuteRuntime(std::string_view commandId,int value);
    bool ActivateLegacy(ShipyardBuilderCommand command,int value=0);
    bool ApplySessionToLegacy(std::string_view commandId);

    ShipyardBuilderSystem* builder_=nullptr;
    ShipyardBuilderMigrationState migration_{};
    ShipyardCommandSystem commands_=ShipyardCommandSystem::BuildProfessionalDefaults();
    ShipyardProfessionalShellState shell_=ShipyardProfessionalShellSystem::Create();
};

} // namespace subspace
