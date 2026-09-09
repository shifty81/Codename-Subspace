#include "interior/ModularInteriorKitSystem.h"

#include <algorithm>
#include <cctype>
#include <set>

namespace subspace {

InteriorModuleAssetDef ModularInteriorKitSystem::Normalize(InteriorModuleAssetDef module, const InteriorModuleKit& kit) const
{
    module.widthCells = std::max(1, module.widthCells);
    module.heightCells = std::max(1, module.heightCells);
    module.cellSizeMeters = kit.cellSizeMeters > 0.0 ? kit.cellSizeMeters : 2.0;
    module.deckHeightMeters = kit.deckHeightMeters > 0.0 ? kit.deckHeightMeters : 3.0;
    if (module.kind == InteriorModuleKind::MachineryMount || module.kind == InteriorModuleKind::PropMount ||
        module.kind == InteriorModuleKind::Console) {
        module.gameplayMount = true;
    }
    module.walkableSurface = module.kind == InteriorModuleKind::Floor || module.kind == InteriorModuleKind::Stair ||
                             module.kind == InteriorModuleKind::CorridorStraight || module.kind == InteriorModuleKind::CorridorCorner ||
                             module.kind == InteriorModuleKind::CorridorTJunction || module.kind == InteriorModuleKind::CorridorCross;
    module.portalCapable = module.kind == InteriorModuleKind::Door || module.kind == InteriorModuleKind::DoorFrame ||
                           module.kind == InteriorModuleKind::Airlock || module.kind == InteriorModuleKind::CorridorEnd ||
                           module.kind == InteriorModuleKind::CorridorStraight || module.kind == InteriorModuleKind::CorridorCorner ||
                           module.kind == InteriorModuleKind::CorridorTJunction || module.kind == InteriorModuleKind::CorridorCross;
    for (auto& socket : module.sockets) {
        socket.gridX = std::clamp(socket.gridX, 0, module.widthCells);
        socket.gridY = std::clamp(socket.gridY, 0, module.heightCells);
        if (socket.compatibility.empty()) socket.compatibility = "interior";
    }
    return module;
}

InteriorKitValidation ModularInteriorKitSystem::Validate(const InteriorModuleKit& kit) const
{
    InteriorKitValidation out;
    if (kit.kitId.empty()) out.errors.emplace_back("kitId is required");
    if (kit.cellSizeMeters <= 0.0) out.errors.emplace_back("cellSizeMeters must be positive");
    if (kit.deckHeightMeters <= 0.0) out.errors.emplace_back("deckHeightMeters must be positive");
    if (kit.modules.empty()) out.errors.emplace_back("at least one module is required");

    std::set<std::string> ids;
    bool hasFloor = false;
    bool hasWall = false;
    bool hasDoorOrCorridor = false;
    for (const auto& raw : kit.modules) {
        const auto module = Normalize(raw, kit);
        if (module.assetId.empty()) out.errors.emplace_back("module assetId is required");
        else if (!ids.insert(module.assetId).second) out.errors.emplace_back("duplicate module assetId: " + module.assetId);
        if (module.sourceAssetId.empty()) out.errors.emplace_back("module sourceAssetId is required: " + module.assetId);
        if (module.kind == InteriorModuleKind::Floor) hasFloor = true;
        if (module.kind == InteriorModuleKind::Wall) hasWall = true;
        if (module.kind == InteriorModuleKind::Door || module.kind == InteriorModuleKind::CorridorStraight ||
            module.kind == InteriorModuleKind::CorridorCorner || module.kind == InteriorModuleKind::CorridorTJunction ||
            module.kind == InteriorModuleKind::CorridorCross) hasDoorOrCorridor = true;
    }
    if (!hasFloor) out.errors.emplace_back("kit requires a floor module");
    if (!hasWall) out.errors.emplace_back("kit requires a wall module");
    if (!hasDoorOrCorridor) out.errors.emplace_back("kit requires a traversal connector module");
    out.valid = out.errors.empty();
    return out;
}

const InteriorModuleAssetDef* ModularInteriorKitSystem::FindByKind(const InteriorModuleKit& kit, InteriorModuleKind kind) const
{
    for (const auto& module : kit.modules) if (module.kind == kind) return &module;
    return nullptr;
}

InteriorModuleKind ModularInteriorKitSystem::ClassifySourceName(const std::string& raw) const
{
    std::string s=raw;
    std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    const auto has=[&](const char* t){return s.find(t)!=std::string::npos;};
    if(has("airlock")) return InteriorModuleKind::Airlock;
    if(has("doorframe")||has("door_frame")) return InteriorModuleKind::DoorFrame;
    if(has("door")) return InteriorModuleKind::Door;
    if(has("window")||has("glass")) return InteriorModuleKind::Window;
    if(has("ceiling")||has("roof")) return InteriorModuleKind::Ceiling;
    if(has("stair")) return InteriorModuleKind::Stair;
    if(has("ladder")) return InteriorModuleKind::Ladder;
    if(has("bulkhead")) return InteriorModuleKind::Bulkhead;
    if(has("floor")) return InteriorModuleKind::Floor;
    if(has("corridor")||has("hallway")){if(has("corner"))return InteriorModuleKind::CorridorCorner;if(has("cross"))return InteriorModuleKind::CorridorCross;if(has("junction")||has("t_"))return InteriorModuleKind::CorridorTJunction;if(has("end"))return InteriorModuleKind::CorridorEnd;return InteriorModuleKind::CorridorStraight;}
    if(has("wall")) return InteriorModuleKind::Wall;
    if(has("console")||has("terminal")||has("screen")) return InteriorModuleKind::Console;
    if(has("pipe")||has("tube")) return InteriorModuleKind::Pipe;
    if(has("cable")||has("wire")) return InteriorModuleKind::Cable;
    if(has("light")||has("lamp")) return InteriorModuleKind::Light;
    if(has("chair")||has("table")||has("bench")||has("bed")||has("locker")) return InteriorModuleKind::Furniture;
    if(has("machine")||has("reactor")||has("generator")) return InteriorModuleKind::MachineryMount;
    if(has("crate")||has("prop")||has("barrel")) return InteriorModuleKind::PropMount;
    return InteriorModuleKind::RoomShell;
}

InteriorModuleAssetDef ModularInteriorKitSystem::BuildImportedModule(const std::string& assetId,
                                                                     const std::string& sourcePackId,
                                                                     const std::string& sourceObjectName,
                                                                     const InteriorModuleKit& kit) const
{
    InteriorModuleAssetDef m;
    m.assetId=assetId;m.sourceAssetId=sourcePackId+":"+sourceObjectName;m.sourcePackId=sourcePackId;m.sourceObjectName=sourceObjectName;
    m.kind=ClassifySourceName(sourceObjectName);
    if(m.kind==InteriorModuleKind::Floor||m.kind==InteriorModuleKind::Ceiling||m.kind==InteriorModuleKind::Wall||m.kind==InteriorModuleKind::Bulkhead)m.widthCells=2;
    if(m.kind==InteriorModuleKind::CorridorStraight||m.kind==InteriorModuleKind::CorridorCorner||m.kind==InteriorModuleKind::CorridorTJunction||m.kind==InteriorModuleKind::CorridorCross)m.widthCells=2;
    return Normalize(m,kit);
}

} // namespace subspace
