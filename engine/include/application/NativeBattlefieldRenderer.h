#pragma once

#include "core/Math.h"
#include "procedural/GalaxyGenerator.h"
#include "mining/AsteroidFractureSystem.h"
#include "weapons/MissileSystem.h"
#include "rendering/StrategicBattlefieldPresentation.h"
#include "rendering/StrategicCamera.h"
#include "rendering/StrategicViewProjection.h"
#include "ui/FrontendFlowSystem.h"
#include "ui/FrontendInteraction.h"
#include "ui/SandboxWorkspaceSystem.h"
#include "navigation/SystemMapSystem.h"
#include "navigation/UniverseSystemMapSystem.h"
#include "navigation/VectorTravelSystem.h"
#include "rendering/ForwardSpacePresentationSystem.h"
#include "interior/ShipEmbodimentSystem.h"
#include "hangar/DockingExperienceSystem.h"
#include "ui/ProductionInterfaceSystem.h"
#include "integration/PlayerFacingIntegrationSystem.h"
#include "fleet/FleetCaptainAiSystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include "navigation/ObservableWarpSystem.h"
#include "combat/TacticalTargetingSystem.h"
#include "ui/TacticalContactsSystem.h"
#include "ui/RuntimeWindowLayoutSystem.h"
#include "ui/ShipCommandHudSystem.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace subspace {

struct PhysicsComponent;
class InputState;
class UIRenderer;
struct MiningSalvageTelemetry;
struct ShipAppearanceState;

/// Render-time contact type used by native selection/HUD. It deliberately
/// references generated gameplay data instead of a parallel C# object model.
enum class NativeContactKind {
    None,
    Ship,
    Planet,
    Station,
    Derelict,
    OrbitalHub,
    Asteroid,
    Site
};

struct NativeContactSelection {
    NativeContactKind kind = NativeContactKind::None;
    std::size_t index = 0;
    std::string id;

    bool IsValid() const { return kind != NativeContactKind::None; }
    void Clear() { kind = NativeContactKind::None; index = 0; id.clear(); }
};

struct NativeBattlefieldFrame {
    const GalaxySector* sector = nullptr;
    const PhysicsComponent* playerPhysics = nullptr;
    const InputState* input = nullptr;
    const UIRenderer* uiRenderer = nullptr;
    const StrategicCamera* camera = nullptr;
    const MissileSystem* missileSystem = nullptr;
    const AsteroidFractureSystem* fractureSystem = nullptr;
    const MiningSalvageTelemetry* miningTelemetry = nullptr;
    IndustrialShipVisualProfile industrialProfile = IndustrialShipVisualProfile::StarterIndustrial();
    NativeContactSelection selection{};
    InteriorCutawayState cutaway{};
    float elapsedSeconds = 0.0f;
    bool inertialDampening = true;
    bool boostActive = false;
    FrontendScreen frontendScreen = FrontendScreen::InGame;
    FrontendCommand frontendSelected = FrontendCommand::None;
    FrontendCommand frontendHovered = FrontendCommand::None;
    NewSandboxConfig frontendConfig{};
    StarterCareer starterCareer = StarterCareer::Prospector;
    SandboxWorkspaceMode workspaceMode = SandboxWorkspaceMode::Flight;
    bool standaloneShipyard = false;
    const SystemMapSnapshot* systemMap = nullptr;
    const UniverseSystemMapSnapshot* universeSystemMap = nullptr;
    std::vector<std::string> workspaceLines;
    VectorTravelStage vectorTravelStage = VectorTravelStage::Idle;
    double vectorTravelProgress = 0.0;
    std::string vectorTravelStatus;
    VectorVisualProfile vectorVisual{};
    std::string arrivalTitle;
    float arrivalTitleAlpha = 0.0f;
    bool shipInspection = false;
    bool strategicFlightMode = false;
    SpaceBackdropProfile backdrop{};
    ShipEmbodimentMode embodimentMode = ShipEmbodimentMode::CockpitControl;
    InteriorAvatarState interiorAvatar{};
    DockingExperienceStage dockingStage = DockingExperienceStage::Undocked;
    const DockingExperienceState* dockingState = nullptr;
    double dockingProgress = 0.0;
    std::string dockingStatus;
    ProductionHudModel productionHud{};
    FlightHudRuntimeModel flightHud{};
    ShipCommandHudModel commandHud{};
    TacticalContactsModel tacticalContacts{};
    TacticalTargetingState targeting{};
    const RuntimeWindowLayout* runtimeWindowLayout = nullptr;
    const std::vector<ObservableWarpEvent>* observableWarpEvents = nullptr;
    CommandRailRuntimeModel commandRail{};
    ContextMenuRuntimeModel contextMenu{};
    HangarRuntimeModel hangarRuntime{};
    const std::vector<RuntimeStationContact>* stationContacts = nullptr;
    const GalaxyRuntimeModel* galaxyRuntime = nullptr;
    const PlanetIndustryRuntimeModel* planetIndustryRuntime = nullptr;
    FleetRuntimeModel fleetRuntime{};
    const FleetCaptainRuntime* fleetCaptainRuntime = nullptr;
    float systemMapZoom = 1.0f;
    int systemMapHoveredNode = -1;
    float pointerX = 0.0f;
    float pointerY = 0.0f;
    const ShipyardBuilderRuntimeModel* shipBuilder = nullptr;
    const ProceduralShipVisualRecipe* shipBuilderRecipe = nullptr;
    const ShipAppearanceState* shipBuilderAppearance = nullptr;
    const ProceduralShipVisualRecipe* playerShipRecipe = nullptr;
    const ShipAppearanceState* playerShipAppearance = nullptr;
    Vector3 systemMapPan{};
    int viewportWidth = 1600;
    int viewportHeight = 900;
};

/// Pass175-179 native OpenGL battlefield/HUD backend.
///
/// The backend is intentionally application/platform owned: engine gameplay
/// remains renderer-independent and OpenGL is not reintroduced into the core
/// engine library. On non-Windows builds this class is a compile-safe no-op.
class NativeBattlefieldRenderer {
public:
    struct VisualAssets; // implementation detail exposed only for translation-unit helpers

    NativeBattlefieldRenderer();
    ~NativeBattlefieldRenderer();

    NativeBattlefieldRenderer(const NativeBattlefieldRenderer&) = delete;
    NativeBattlefieldRenderer& operator=(const NativeBattlefieldRenderer&) = delete;

    bool Initialize();
    void Shutdown();
    void Render(const NativeBattlefieldFrame& frame);

    /// Convert client-area pixel coordinates into authoritative X/Y gameplay
    /// coordinates using the same projection as the renderer.
    static Vector3 ScreenToWorld(float screenX, float screenY,
                                 int viewportWidth, int viewportHeight,
                                 const StrategicCamera& camera);

    /// Project a visual-space point into client pixels for labels/target HUD.
    static StrategicScreenPoint WorldToScreen(const Vector3& worldPoint,
                                               int viewportWidth, int viewportHeight,
                                               const StrategicCamera& camera);

    /// Pick the nearest renderable strategic contact to a gameplay-space point.
    static NativeContactSelection PickContact(const GalaxySector& sector,
                                              const Vector3& worldPoint,
                                              float maxWorldDistance = 2.2f,
                                              const std::vector<RuntimeStationContact>* stationContacts = nullptr);

    /// Shipyard editor pick authority. Tests projected module bounds against the
    /// same camera/recipe/ship transform used by the live renderer so viewport
    /// selection and the assembled-module list share one selected module index.
    static int PickShipyardModule(const std::vector<ShipyardModuleRecord>& catalog,
                                  const ProceduralShipVisualRecipe& recipe,
                                  const StrategicCamera& camera,
                                  int viewportWidth,
                                  int viewportHeight,
                                  float screenX,
                                  float screenY,
                                  float shipX,
                                  float shipY,
                                  float shipYaw,
                                  float shipScale = 0.24f,
                                  float screenFraction = 0.22f,
                                  bool player = true);

    static Vector3 SectorToWorld(const SectorPosition& position);
    static SectorPosition WorldToSector(const Vector3& position);
    static float PlanetRadiusToWorld(float radius);

    /// Shared Shipyard catalog/recipe handoff. The native application uses the
    /// same renderer-loaded certified corpus rather than maintaining a second
    /// module scanner with potentially different bounds or IDs.
    const std::vector<ShipyardModuleRecord>& ShipyardCatalog() const;
    void ApplyShipyardCatalogAuthoringOverrides(const std::vector<ShipyardModuleRecord>& catalog);
    ProceduralShipVisualRecipe DefaultShipyardRecipe(const std::string& role,
                                                      std::uint32_t visualSeed) const;

private:
    std::unique_ptr<VisualAssets> _assets;
    bool _initialized = false;
};

} // namespace subspace
