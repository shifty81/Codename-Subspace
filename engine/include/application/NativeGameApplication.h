#pragma once

#include "application/NativeBattlefieldRenderer.h"
#include "core/Engine.h"
#include "platform/NativeWindow.h"
#include "procedural/GalaxyGenerator.h"
#include "mining/MiningSalvageLoop.h"
#include "ui/FrontendFlowSystem.h"
#include "ui/FrontendInteraction.h"
#include "ui/SandboxWorkspaceSystem.h"
#include "navigation/SystemMapSystem.h"
#include "navigation/SystemNavigationSystem.h"
#include "navigation/VectorTravelSystem.h"
#include "scanning/PlanetSurveySystem.h"
#include "economy/PlanetaryIndustrializationSystem.h"
#include "navigation/TravelArrivalSystem.h"
#include "interior/ShipEmbodimentSystem.h"
#include "hangar/DockingExperienceSystem.h"
#include "ui/ProductionInterfaceSystem.h"
#include "rendering/ForwardSpacePresentationSystem.h"
#include "rendering/CinematicFlightPresentationSystem.h"
#include "rendering/ShipInspectionReviewSystem.h"
#include "celestial/OrbitalDynamicsSystem.h"
#include "navigation/UniverseSystemMapSystem.h"
#include "flight/StrategicFlightSystem.h"
#include "integration/PlayerFacingIntegrationSystem.h"
#include "station/StationEcologySystem.h"
#include "fleet/FleetCaptainAiSystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipyardRefitSystem.h"
#include "ship_editor/ShipBlueprintLibrarySystem.h"
#include "ship_editor/ShipyardBuildSafetySystem.h"
#include "navigation/ObservableWarpSystem.h"
#include "ui/TacticalContactsSystem.h"
#include "ui/RuntimeWindowLayoutSystem.h"
#include "combat/TacticalTargetingSystem.h"
#include "editor/ConstructionEditorCameraSystem.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace subspace {

struct NativeGameRunOptions {
    bool runtimeSmoke = false;
    bool startShipyard = false;
    std::uint64_t maxFrames = 0; // 0 = unlimited interactive run
};

/// Native executable shell. Owns the OS window and frame presentation while
/// Engine remains the simulation/gameplay authority.
class NativeGameApplication {
public:
    NativeGameApplication();

    int Run(const NativeGameRunOptions& options = {});

private:
    void HandleGlobalActions();
    void HandleFrontendActions();
    void OpenShipyardWorkspace(bool standalone);
    void FrameShipyardView(bool selectedModule);
    void ApplyConstructionCameraView();
    void UpdateConstructionCameraKeyboard();
    bool ActivateShipyardControl(ShipyardBuilderCommand command, int value = 0);
    void HandleShipyardTransformHotkeys();
    void RestoreGameplayCameraLimits();
    void ProcessShipyardRequests();
    void SaveCurrentShipyardBlueprint();
    void SaveCurrentShipyardSocketOverrides();
    void LoadCurrentShipyardSocketOverrides();
    void SaveCurrentShipyardDefinitionOverrides();
    void LoadCurrentShipyardDefinitionOverrides();
    void ActivateFrontendCommand(FrontendCommand command);
    void GoToFrontendScreen(FrontendScreen screen);
    void BootstrapPlayableSlice();
    void UpdateCameraAndSelection();
    void RebuildSystemMap();
    void UpdateVectorTravel();
    double CurrentVectorTopSpeedMetersPerSecond() const;
    void UpdateEmbodiment();
    void UpdateDocking();
    void UpdateVectorCamera();
    void UpdateOrbitalSimulation();
    void UpdateStrategicAutopilot();
    void RefreshPlayerFacingModels();
    void UpdateFleetCaptains();
    InteractionContext BuildInteractionContext(const NativeContactSelection& selection) const;
    std::size_t ActivePlanetIndex() const;
    void EnsurePlanetSurvey(std::size_t planetIndex);
    std::vector<std::string> BuildWorkspaceLines() const;
    NativeBattlefieldFrame BuildRenderFrame() const;

    Engine _engine;
    NativeWindow _window;
    NativeBattlefieldRenderer _renderer;
    GalaxySector _sector{};
    EntityId _playerEntity = InvalidEntityId;
    NativeContactSelection _selection{};
    NativeContactSelection _lockedTarget{};
    TacticalTargetingState _targeting{};
    TacticalContactsModel _tacticalContacts{};
    MiningSalvageLoop _miningLoop;
    bool _pauseKeyConsumed = false;
    bool _inspectionKeyConsumed = false;
    bool _shipInspection = false;
    bool _primaryFireConsumed = false;
    bool _miningFireConsumed = false;
    FrontendFlowSystem _frontend;
    FrontendCommand _frontendSelected = FrontendCommand::MainNewSandbox;
    FrontendCommand _frontendHovered = FrontendCommand::None;
    StarterCareer _starterCareer = StarterCareer::Prospector;
    NativeGameRunOptions _runOptions{};
    SandboxWorkspaceSystem _workspace;
    SystemMapSystem _systemMapSystem;
    SystemMapSnapshot _systemMap;
    SystemNavigationSystem _systemNavigation;
    VectorTravelSystem _vectorTravelSystem;
    VectorTravelSession _vectorTravel;
    VectorTravelStage _previousVectorTravelStage = VectorTravelStage::Idle;
    std::vector<ObservableWarpEvent> _observableWarpEvents;
    AstronomicalPosition _currentAstronomicalPosition{};
    double _vectorFuel = 5000.0;
    std::uint64_t _activeWarpDestination = 0;
    TravelArrivalSystem _travelArrivalSystem;
    TravelArrivalEnvelope _activeArrival{};
    ShipEmbodimentSystem _embodiment;
    DockingExperienceSystem _dockingSystem;
    DockingExperienceState _docking{};
    ProductionInterfaceSystem _productionInterface;
    ForwardSpacePresentationSystem _forwardPresentation;
    std::unordered_map<std::size_t,PlanetSurveyRecord> _planetSurveys;
    std::unordered_map<std::size_t,PlanetaryIndustrializationProject> _planetProjects;
    bool _vectorCameraCaptured = false;
    float _preVectorYawDegrees = 0.0f;
    float _preVectorTilt = 0.32f;
    float _preVectorZoom = 1.0f;
    float _preVectorElevationOverride = -1.0f;
    float _inspectionPreYaw = 0.0f;
    float _inspectionPreTilt = 0.32f;
    float _inspectionPreZoom = 1.0f;
    float _inspectionPreElevation = -1.0f;
    ShipInspectionReviewSystem _inspectionReview;
    ShipInspectionCameraState _inspectionCamera{};
    OrbitalDynamicsSystem _orbitalDynamics;
    UniverseSystemMapSystem _universeSystemMapSystem;
    UniverseSystemMapSnapshot _universeSystemMap;
    std::vector<OrbitalBodyRecord> _orbitalBodies;
    StrategicFlightSystem _strategicFlight;
    PlayerFacingIntegrationSystem _playerFacing;
    FlightHudRuntimeModel _flightHud;
    ContextMenuRuntimeModel _contextMenu;
    GeneratedStationProfile _primaryStationProfile{};
    GeneratedStationProfile _activeStationProfile{};
    std::vector<RuntimeStationContact> _stationContacts;
    HangarRuntimeModel _hangarRuntime{};
    SystemMapRuntimeModel _systemMapRuntime{};
    GalaxyRuntimeModel _galaxyRuntime{};
    PlanetIndustryRuntimeModel _activePiRuntime{};
    std::size_t _activePiPlanet = static_cast<std::size_t>(-1);
    FleetRuntimeModel _fleetRuntime{};
    FleetCaptainAiSystem _fleetCaptainAi;
    FleetCaptainRuntime _fleetCaptainRuntime{};
    double _orbitalSimulationSeconds = 0.0;
    double _orbitalMapRefreshAccumulator = 0.0;
    std::string _arrivalTitle;
    ShipyardBuilderSystem _shipBuilder;
    ShipyardRefitSession _shipyardRefit{};
    ProceduralShipVisualRecipe _playerShipRecipe{};
    ShipAppearanceState _playerShipAppearance{};
    bool _hasPlayerShipRecipe = false;
    bool _standaloneShipyard = false;
    bool _shipyardCameraCaptured = false;
    ConstructionEditorCameraState _constructionCamera{};
    float _preShipyardZoom = 0.56f;
    float _preShipyardTargetZoom = 0.56f;
    float _preShipyardMinZoom = 0.12f;
    float _preShipyardMaxZoom = 28.0f;
    float _preShipyardYawDegrees = 0.0f;
    float _preShipyardElevationDegrees = 34.0f;
    bool _preShipyardHadElevationOverride = false;
    bool _shipyardPointerTransform = false;
    bool _shipyardCatalogPointerDrag = false;
    bool _shipyardCatalogPointerCandidate = false;
    int _shipyardCatalogPointerCandidateIndex = -1;
    float _shipyardCatalogPressX = 0.0f;
    float _shipyardCatalogPressY = 0.0f;
    RuntimeWindowLayout _runtimeWindowLayout{};
};

} // namespace subspace
