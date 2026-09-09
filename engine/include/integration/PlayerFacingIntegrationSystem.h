#pragma once

#include "core/Math.h"
#include "flight/StrategicFlightSystem.h"
#include "ui/ContextActionSystem.h"
#include "navigation/GalaxyCatalogSystem.h"
#include "navigation/GalaxyMapSystem.h"
#include "navigation/GalaxyRoutePlannerSystem.h"
#include "navigation/UniverseSystemMapSystem.h"
#include "navigation/SystemMapSystem.h"
#include "economy/PlanetaryIndustrySystem.h"
#include "fleet/FleetIntentSystem.h"
#include "station/StationEcologySystem.h"
#include "hangar/UniversalDockedStationSystem.h"
#include "station/StationDockingGeometrySystem.h"
#include "ui/SandboxWorkspaceSystem.h"
#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct FlightHudRuntimeModel {
    std::string modeLabel = "MANUAL FLIGHT";
    std::vector<std::string> chatChannels{"LOCAL", "FLEET", "CORPORATION", "DIRECT"};
    std::vector<std::string> moduleSlots;
    bool chatBottomLeft = true;
    bool hotbarBottomCenter = true;
    bool centerReserved = true;
    bool tacticalOverview = true;
};

struct ContextMenuRuntimeModel {
    bool open = false;
    float screenX = 0.0f;
    float screenY = 0.0f;
    Vector3 worldTarget{};
    std::uint64_t targetId = 0;
    bool systemMapTarget = false;
    std::vector<ContextAction> actions;
    std::size_t selected = 0;
};

struct ContextMenuLayout {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float headerHeight = 0.0f;
    float rowHeight = 0.0f;
    float textScale = 1.0f;
};

struct StrategicAutopilotCommand {
    bool valid = false;
    StrategicFlightOrder order{};
    StrategicFlightIntent intent{};
    float desiredYawRadians = 0.0f;
    float throttle = 0.0f;
};

struct RuntimeStationContact {
    std::uint64_t id = 0;
    std::string name;
    StationArchetype archetype = StationArchetype::TradeHub;
    StationOrbitClass orbitClass = StationOrbitClass::LowPlanet;
    Vector3 position{};
    bool dockable = true;
    bool asteroidEmbedded = false;
    std::vector<std::string> services;
};

struct HangarRuntimeModel {
    bool docked = false;
    std::string title;
    std::string hangarProfile;
    std::vector<std::string> services;
    std::vector<std::string> serviceActions;
    bool orbitCamera = true;
    bool showActualShip = true;
    float cameraDistance = 18.0f;
    StationArchetype archetype = StationArchetype::TradeHub;
    StationBerthSize berthSize = StationBerthSize::Standard;
};

struct SystemMapRuntimeModel {
    UniverseSystemMapSnapshot snapshot{};
    std::size_t selected = 0;
    float zoom = 1.0f;
    Vector3 pan{};
    bool showOrbitTracks = true;
};


struct SystemMapScreenLayout {
    float left=64.0f;
    float top=58.0f;
    float right=1536.0f;
    float bottom=842.0f;
    float plotRight=1218.0f;
    float centerX=640.0f;
    float centerY=450.0f;
    float scale=1.0f;
    float infoLeft=1232.0f;
    bool Contains(float x,float y) const {return x>=left&&x<=right&&y>=top&&y<=bottom;}
};

struct GalaxyRuntimeModel {
    std::vector<GalaxySystemRecord> catalog;
    GalaxyOverlay overlay = GalaxyOverlay::None;
    GalaxyMapCamera camera{};
    std::uint32_t selectedSystem = 1;
    GalaxyRouteResult route{};
    bool initialized = false;
};

struct PlanetIndustryRuntimeModel {
    std::string planetName;
    PlanetaryIndustryState industry{};
    HexCoord selectedHex{};
    bool tetherAvailable = false;
    double tetherStored = 0.0;
};

struct FleetRuntimeShip {
    std::uint64_t id = 0;
    FleetShipRole role = FleetShipRole::Combat;
    Vector3 desiredOffset{};
    StrategicOrderKind mirroredOrder = StrategicOrderKind::Follow;
};

struct FleetRuntimeModel {
    std::vector<FleetRuntimeShip> ships;
    FormationType formation = FormationType::V;
    float spacing = 18.0f;
};


struct CommandRailItem {
    std::string id;
    std::string label;
    std::string shortLabel;
    SandboxWorkspaceMode workspace = SandboxWorkspaceMode::Flight;
    bool active = false;
};
struct CommandRailRuntimeModel {
    std::string title = "COMMAND RAIL";
    float width = 88.0f;
    float top = 76.0f;
    float rowHeight = 48.0f;
    std::vector<CommandRailItem> items;
};

struct PlayerFacingAcceptanceState {
    bool hudIntegrated = false;
    bool contextCommands = false;
    bool strategicAutopilot = false;
    bool stationVisibleAndDockable = false;
    bool hangarIntegrated = false;
    bool liveSystemMap = false;
    bool galaxyRuntime = false;
    bool planetaryIndustryRuntime = false;
    bool fleetRuntime = false;
    bool shipOnly = true;
};

struct PlayerFacingAcceptanceReport {
    bool pass = false;
    int score = 0;
    std::vector<std::string> blockers;
};

class PlayerFacingIntegrationSystem {
public:
    CommandRailRuntimeModel BuildCommandRail(SandboxWorkspaceMode current) const;
    int HitTestCommandRail(const CommandRailRuntimeModel& rail, float x, float y) const;

    FlightHudRuntimeModel BuildFlightHud(FlightControlMode mode,
                                         const std::vector<std::string>& fittedModules) const;

    ContextMenuRuntimeModel OpenContext(const InteractionContext& context,
                                        float screenX, float screenY) const;
    void MoveContextSelection(ContextMenuRuntimeModel& menu, int direction) const;
    const ContextAction* ActiveContextAction(const ContextMenuRuntimeModel& menu) const;
    ContextMenuLayout LayoutContextMenu(const ContextMenuRuntimeModel& menu, int viewportWidth, int viewportHeight) const;
    int HitTestContextMenu(const ContextMenuRuntimeModel& menu, int viewportWidth, int viewportHeight, float x, float y) const;
    StrategicFlightOrder ToStrategicOrder(const ContextAction& action,
                                           std::uint64_t targetId,
                                           const Vector3& targetPosition,
                                           float desiredRange = 20.0f) const;

    StrategicAutopilotCommand EvaluateAutopilot(const StrategicFlightSystem& flight,
                                                const Vector3& shipPosition,
                                                float shipYawRadians,
                                                const Vector3& targetPosition) const;

    std::vector<RuntimeStationContact> BuildStationContacts(std::uint32_t seed,
                                                            const Vector3& planetPosition,
                                                            float planetRadius,
                                                            bool populated = true) const;
    HangarRuntimeModel BuildHangar(const GeneratedStationProfile& station,
                                   bool docked, float requestedCameraDistance = 18.0f) const;

    SystemMapRuntimeModel BuildSystemMapRuntime(const std::vector<OrbitalBodyRecord>& bodies,
                                                double simulationSeconds,
                                                int samples = 64) const;
    void StepSystemMapSelection(SystemMapRuntimeModel& model, int direction) const;
    void ZoomSystemMap(SystemMapRuntimeModel& model, float wheelDelta) const;
    bool ShouldPromoteSystemMapToGalaxy(const SystemMapRuntimeModel& model, float wheelDelta) const;
    void PanSystemMap(SystemMapRuntimeModel& model, float dx, float dy) const;
    SystemMapScreenLayout LayoutSystemMap(const SystemMapSnapshot& map,
                                          const UniverseSystemMapSnapshot* live,
                                          int viewportWidth,int viewportHeight,
                                          float zoom,const Vector3& pan) const;
    int HitTestSystemMapNode(const SystemMapSnapshot& map,
                             const UniverseSystemMapSnapshot* live,
                             int viewportWidth,int viewportHeight,
                             float zoom,const Vector3& pan,
                             float screenX,float screenY) const;

    GalaxyRuntimeModel BuildGalaxyRuntime(std::uint32_t seed, std::size_t count = 10000) const;
    bool SelectGalaxySystem(GalaxyRuntimeModel& model, std::uint32_t id) const;
    void OrbitGalaxy(GalaxyRuntimeModel& model, float dx, float dy) const;
    void ZoomGalaxy(GalaxyRuntimeModel& model, float wheelDelta) const;
    bool PlotGalaxyRoute(GalaxyRuntimeModel& model, std::uint32_t start, std::uint32_t end,
                         float jumpRange, GalaxyRouteMode mode) const;

    PlanetIndustryRuntimeModel BuildPlanetIndustry(const PlanetData& planet,
                                                   int radius, std::uint32_t seed) const;
    bool PlaceIndustry(PlanetIndustryRuntimeModel& model, PiInstallationKind kind,
                       PowerTechnology tech = PowerTechnology::Burner) const;
    double TickIndustryToTether(PlanetIndustryRuntimeModel& model, double produced, double hours) const;

    FleetRuntimeModel BuildFleet(const std::vector<FleetWingShip>& wing,
                                 const FleetFlightConfig& config,
                                 StrategicOrderKind playerOrder) const;

    PlayerFacingAcceptanceReport EvaluateAcceptance(const PlayerFacingAcceptanceState& state) const;
};

} // namespace subspace
