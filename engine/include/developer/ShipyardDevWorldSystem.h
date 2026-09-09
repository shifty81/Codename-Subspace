#pragma once

#include "content/ShipyardModuleSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "world/WorldScaleAuthoritySystem.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class DevWorldBackdrop {
    Stars,
    Checkerboard,
    NeutralStudio,
    Hangar,
    PlanetSurface,
    Nebula,
    Black,
    White
};

enum class DevWorldZoneKind {
    PlayerScale,
    AnimationViewer,
    KitbashCatalog,
    CertifiedShips,
    InteriorKit,
    PcgProvingGround,
    Terraforming,
    LightingMaterials
};

struct DevWorldZone {
    std::string id;
    std::string displayName;
    DevWorldZoneKind kind = DevWorldZoneKind::KitbashCatalog;
    Vector3 origin{};
    Vector3 sizeMeters{32.0f,32.0f,8.0f};
};

struct DevWorldAssetPedestal {
    std::string assetId;
    std::string label;
    Vector3 position{};
    Vector3 rotationDegrees{};
    float uniformScale = 1.0f;
    bool certified = false;
    bool reviewRequired = false;
};

struct DevWorldShipPad {
    std::string recipeId;
    std::string displayName;
    Vector3 position{};
    float headingDegrees = 0.0f;
    bool certified = false;
};

struct ShipyardDevWorldState {
    bool enabled = false;
    DevWorldBackdrop backdrop = DevWorldBackdrop::Checkerboard;
    float chunkSizeMeters = 4096.0f;
    float checkerCellMeters = 4.0f;
    bool showMeterGrid = true;
    bool showPlayerReference = true;
    bool showCertifiedOnly = true;
    Vector3 playerSpawn{};
    std::vector<DevWorldZone> zones;
    std::vector<DevWorldAssetPedestal> assetPedestals;
    std::vector<DevWorldShipPad> shipPads;
};

class ShipyardDevWorldSystem {
public:
    static const char* BackdropName(DevWorldBackdrop backdrop);
    static const char* ZoneName(DevWorldZoneKind kind);
    static ShipyardDevWorldState CreateDefault(const WorldScaleProfile& scale);
    static void CycleBackdrop(ShipyardDevWorldState& state, int direction = 1);
    static std::vector<DevWorldAssetPedestal> LayoutKitbashCatalog(const std::vector<ShipyardModuleRecord>& catalog,
                                                                   const Vector3& origin,
                                                                   const WorldScaleProfile& scale,
                                                                   bool certifiedOnly = true);
    static std::vector<DevWorldShipPad> LayoutCertifiedShips(const std::vector<ProceduralShipVisualRecipe>& recipes,
                                                              const Vector3& origin,
                                                              const WorldScaleProfile& scale);
};

} // namespace subspace
