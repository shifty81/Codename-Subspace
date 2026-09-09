#include "application/NativeGameApplication.h"
#include "ships/ShipyardAuthoredShipSystem.h"

#include "core/config/ConfigurationManager.h"
#include "core/logging/Logger.h"
#include "core/physics/PhysicsComponent.h"
#include "input/PlayerControlSystem.h"
#include "weapons/MissileSystem.h"
#include "rendering/CelestialEnvironmentSystem.h"
#include "rendering/EnvironmentPresentationSystem.h"
#include "ship_editor/ShipyardDesignExchangeSystem.h"
#include "ship_editor/ShipyardAuthoringSampleSystem.h"
#include "ship_editor/ShipyardEquipmentSystem.h"
#include "ship_editor/ShipyardBuildSafetySystem.h"
#include "combat/FireControlSystem.h"
#include "combat/CombatSystem.h"
#include "navigation/ObservableWarpSystem.h"
#include "procedural/SolarSystemPlacementSystem.h"
#include "procedural/SolarSystemEcologySystem.h"
#include "interior/ShipInteriorLayoutSystem.h"
#include "ui/RuntimeControlContextSystem.h"

#include <algorithm>
#include <memory>
#include <cmath>
#include <limits>
#include <utility>
#include <sstream>
#include <iomanip>
#include <functional>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <ctime>
#include <cctype>

namespace subspace {
namespace {

float Distance2D(const Vector3& a,const Vector3& b)
{
    const float dx=a.x-b.x,dy=a.y-b.y;
    return std::sqrt(dx*dx+dy*dy);
}

bool SameContact(const NativeContactSelection& a,const NativeContactSelection& b)
{
    return a.kind==b.kind && a.kind!=NativeContactKind::None && a.index==b.index && a.id==b.id;
}

Vector3 WorldDeltaToShipLocal(const Vector3& worldDelta,float shipYaw)
{
    const float c=std::cos(shipYaw),s=std::sin(shipYaw);
    return {worldDelta.x*c+worldDelta.y*s,-worldDelta.x*s+worldDelta.y*c,worldDelta.z};
}

Vector3 ShipDeltaToModuleLocal(const Vector3& shipDelta,const VisualModulePlacement& p)
{
    constexpr float kDegToRad=3.14159265358979323846f/180.0f;
    Vector3 v=shipDelta;
    // Inverse of renderer Rz(yaw) * Rx(pitch) * Ry(roll).
    const float yaw=-p.yawDegrees*kDegToRad,cy=std::cos(yaw),sy=std::sin(yaw);
    v={v.x*cy-v.y*sy,v.x*sy+v.y*cy,v.z};
    const float pitch=-p.pitchDegrees*kDegToRad,cp=std::cos(pitch),sp=std::sin(pitch);
    v={v.x,v.y*cp-v.z*sp,v.y*sp+v.z*cp};
    const float roll=-p.rollDegrees*kDegToRad,cr=std::cos(roll),sr=std::sin(roll);
    v={v.x*cr+v.z*sr,v.y,-v.x*sr+v.z*cr};
    const float sx=std::max(.05f,p.scaleX),syScale=std::max(.05f,p.scaleY),sz=std::max(.05f,p.scaleZ);
    v.x/=sx;v.y/=syScale;v.z/=sz;if(p.mirrorX)v.x=-v.x;if(p.mirrorY)v.y=-v.y;if(p.mirrorZ)v.z=-v.z;
    return v;
}

Vector3 ShipLocalToWorld(const Vector3& local,float shipYaw)
{
    const float c=std::cos(shipYaw),s=std::sin(shipYaw);
    return {local.x*c-local.y*s,local.x*s+local.y*c,local.z};
}

TacticalTargetReference ToTargetReference(const NativeContactSelection& selection)
{
    TacticalTargetReference ref;
    ref.kind=static_cast<int>(selection.kind);
    ref.index=selection.index;
    ref.id=selection.id;
    return ref;
}

NativeContactSelection ToNativeContactSelection(const TacticalContactRow& row)
{
    NativeContactSelection selection;
    selection.kind=static_cast<NativeContactKind>(row.sourceKind);
    selection.index=row.sourceIndex;
    selection.id=row.sourceId;
    return selection;
}

TacticalContactKind ToTacticalContactKind(const NativeContactSelection& selection,const GalaxySector& sector)
{
    switch(selection.kind){
        case NativeContactKind::Ship:
            if(selection.index<sector.ships.size()) return sector.ships[selection.index].hostile?TacticalContactKind::HostileShip:TacticalContactKind::NeutralShip;
            return TacticalContactKind::NeutralShip;
        case NativeContactKind::Planet:return TacticalContactKind::Planet;
        case NativeContactKind::Station:return TacticalContactKind::Station;
        case NativeContactKind::Derelict:return TacticalContactKind::Wreck;
        case NativeContactKind::Asteroid:return TacticalContactKind::Asteroid;
        case NativeContactKind::OrbitalHub:return TacticalContactKind::Station;
        case NativeContactKind::Site:return TacticalContactKind::Site;
        default:return TacticalContactKind::NeutralShip;
    }
}

float ShortestAngleRadians(float from,float to) {
    constexpr float pi=3.14159265358979323846f;
    float d=std::fmod(to-from+pi,2.0f*pi);
    if(d<0.0f)d+=2.0f*pi;
    return d-pi;
}

float ShortestAngleDegrees(float from,float to) {
    float d=std::fmod(to-from+180.0f,360.0f);
    if(d<0.0f)d+=360.0f;
    return d-180.0f;
}

Vector3 ProjectOutsidePlanetEnvelope(const Vector3& candidate,const Vector3& planetWorld,
                                    const PlanetData& planet,float additionalClearance) {
    return CelestialEnvironmentSystem{}.ProjectOutsideLocalOrbit(candidate,planetWorld,planet,additionalClearance);
}

Vector3 ContactWorldPosition(const GalaxySector& sector,const NativeContactSelection& selection,
                             const std::vector<RuntimeStationContact>* stationContacts=nullptr)
{
    switch(selection.kind){
        case NativeContactKind::Ship: if(selection.index<sector.ships.size()) return NativeBattlefieldRenderer::SectorToWorld(sector.ships[selection.index].position); break;
        case NativeContactKind::Planet: if(selection.index<sector.planets.size()) return NativeBattlefieldRenderer::SectorToWorld(sector.planets[selection.index].position); break;
        case NativeContactKind::Station:
            if(selection.index==0 && sector.hasStation) return NativeBattlefieldRenderer::SectorToWorld(sector.station.position);
            if(stationContacts && selection.index>0 && selection.index-1<stationContacts->size()) return (*stationContacts)[selection.index-1].position;
            break;
        case NativeContactKind::Derelict: if(selection.index<sector.derelicts.size()) return NativeBattlefieldRenderer::SectorToWorld(sector.derelicts[selection.index].position); break;
        case NativeContactKind::OrbitalHub: if(selection.index<sector.orbitalHubs.size()) return NativeBattlefieldRenderer::SectorToWorld(sector.orbitalHubs[selection.index].position); break;
        case NativeContactKind::Asteroid: if(selection.index<sector.asteroids.size()) return NativeBattlefieldRenderer::SectorToWorld(sector.asteroids[selection.index].position); break;
        case NativeContactKind::Site: if(selection.index<sector.pointsOfInterest.size()) return NativeBattlefieldRenderer::SectorToWorld(sector.pointsOfInterest[selection.index].position); break;
        case NativeContactKind::None: break;
    }
    return {};
}

GeneratedStationProfile StationProfileForSelection(const GeneratedStationProfile& primary,
                                                    const std::vector<RuntimeStationContact>& contacts,
                                                    const NativeContactSelection& selection)
{
    if(selection.kind!=NativeContactKind::Station || selection.index==0 || selection.index-1>=contacts.size()) return primary;
    const auto& c=contacts[selection.index-1];
    GeneratedStationProfile p; p.id=c.id; p.name=c.name; p.archetype=c.archetype; p.orbitClass=c.orbitClass;
    p.dockable=c.dockable; p.asteroidEmbedded=c.asteroidEmbedded; p.services=c.services; p.serviceCount=static_cast<int>(p.services.size());
    p.defenseTier=c.asteroidEmbedded?2:1; return p;
}

std::filesystem::path FindProjectRootForBlueprints(){
    std::error_code ec;auto probe=std::filesystem::current_path(ec);if(ec)return {};
    for(int i=0;i<8&&!probe.empty();++i){
        if(std::filesystem::exists(probe/"SubspaceTools.ps1",ec)&&std::filesystem::exists(probe/"engine",ec))return probe;
        const auto parent=probe.parent_path();if(parent==probe)break;probe=parent;
    }
    return {};
}

std::string BlueprintTimestamp(){
    const auto now=std::chrono::system_clock::now();const std::time_t t=std::chrono::system_clock::to_time_t(now);std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm,&t);
#else
    localtime_r(&t,&tm);
#endif
    std::ostringstream o;o<<std::put_time(&tm,"%Y%m%d-%H%M%S");return o.str();
}

std::string SafeBlueprintStem(std::string value){
    if(value.empty())value="SubspaceShip";
    for(char& c:value){if(!(std::isalnum(static_cast<unsigned char>(c))||c=='_'||c=='-'))c='_';}
    while(value.find("__")!=std::string::npos)value.erase(value.find("__"),1);
    return value;
}

}

NativeGameApplication::NativeGameApplication()
    : _window(_engine.GetInputState())
{
}

int NativeGameApplication::Run(const NativeGameRunOptions& options)
{
    _runOptions = options;
    _engine.Initialize();
    if (options.runtimeSmoke) {
        GoToFrontendScreen(FrontendScreen::InGame);
        BootstrapPlayableSlice();
    } else {
        GoToFrontendScreen(FrontendScreen::MainMenu);
    }

    const auto& graphics = ConfigurationManager::Instance().GetGraphics();
    NativeWindowConfig windowConfig;
    windowConfig.width = options.runtimeSmoke ? 640 : graphics.resolutionWidth;
    windowConfig.height = options.runtimeSmoke ? 360 : graphics.resolutionHeight;
    windowConfig.vsync = graphics.vsync;
    windowConfig.title = options.runtimeSmoke ? "Codename: Subspace [native smoke]" : "Codename: Subspace";

    if (!_window.Initialize(windowConfig)) {
        Logger::Instance().Error("Application",
            "Native window backend could not initialize. The shipping runtime no longer falls back to the legacy C# window.");
        _engine.Shutdown();
        return 2;
    }
    if (!_renderer.Initialize()) {
        Logger::Instance().Error("Application", "Native battlefield renderer failed to initialize.");
        _window.Shutdown();
        _engine.Shutdown();
        return 3;
    }

    Logger::Instance().Info("ShipyardR5",
        "SHIPYARD_R5_AUTHORITY active: legacy synthetic hull/rectangle-thruster visuals disabled; authored Shipyard propulsion only.");
    Logger::Instance().Info("CelestialR5",
        "CELESTIAL_R5 active: continuous planet lighting, lit independent clouds, and limb-only atmosphere.");

    // Pass428 regression repair: restore a real native Shipyard editor over
    // the same certified module catalog/recipe authority used by rendering.
    // The C++ conversion previously left only a painted placeholder workspace.
    const auto starterShipyardRecipe = _renderer.DefaultShipyardRecipe("INDUSTRIAL", 1u);
    _shipBuilder.Initialize(_renderer.ShipyardCatalog(), starterShipyardRecipe);
    LoadCurrentShipyardSocketOverrides();
    LoadCurrentShipyardDefinitionOverrides();
    _shipBuilder.SetAppearance(_playerShipAppearance);
    if (!_shipBuilder.Recipe().modules.empty()) {
        _playerShipRecipe = _shipBuilder.Recipe();
        _hasPlayerShipRecipe = true;
        if(_playerEntity!=0 && !_engine.GetRuntimeServices().interiors.GetLayout(_playerEntity)){
            ShipInteriorLayoutSystem interiorLayout;
            interiorLayout.Materialize(_playerEntity,_renderer.ShipyardCatalog(),_playerShipRecipe,_engine.GetRuntimeServices().interiors);
        }
    }

    _engine.SetViewportSize(static_cast<float>(_window.GetWidth()),
                            static_cast<float>(_window.GetHeight()));
    _runtimeWindowLayout = RuntimeWindowLayoutSystem::DefaultFlightLayout(_window.GetWidth(),_window.GetHeight());

    // Root Project Control Center and developers can enter the upgraded Shipyard
    // directly. This is the same in-game Shipyard workspace exposed by the Main
    // Menu, not a separate editor executable.
    if(options.startShipyard)OpenShipyardWorkspace(true);

    while (_window.PumpEvents() &&
           (_engine.GetState() == EngineState::Running || _engine.GetState() == EngineState::Paused)) {
        _engine.SetViewportSize(static_cast<float>(_window.GetWidth()),
                                static_cast<float>(_window.GetHeight()));
        // Frontend owns pointer input for the entire frame.  The previous
        // ordering let HandleGlobalActions consume LMB before the main menu
        // could hit-test it, making painted controls look inert.
        const bool frontendOwnedFrame = _frontend.Screen()!=FrontendScreen::InGame;
        HandleFrontendActions();
        if (!frontendOwnedFrame && _frontend.Screen()==FrontendScreen::InGame) HandleGlobalActions();

        // Pass487: structural build mode owns input before physics updates.
        // Suppression is explicit so held thrust/boost/fire cannot leak through
        // an overlay focus path and cannot light authored engine plumes.
        if (_workspace.Mode()==SandboxWorkspaceMode::ShipBuilder || _standaloneShipyard)
            ShipyardBuildSafetySystem::SuppressFlightAndWeapons(_engine.GetInputState());

        // Pass470: keep the native backbuffer explicitly dark. The standalone
        // Shipyard must never inherit a platform/default white clear even when
        // the 3D preview has no opaque world geometry behind it.
        _window.BeginFrame(0.003f, 0.007f, 0.012f, 1.0f);
        _engine.Tick();
        if (_frontend.Screen()==FrontendScreen::InGame && !_standaloneShipyard) {
            UpdateOrbitalSimulation();
            if (auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity)) {
                if (!_workspace.IsOverlayOpen()) {
                    _miningLoop.Update(_engine.GetLastDeltaTime(),player->position,_sector);
                }
            }
            UpdateVectorTravel();
            UpdateStrategicAutopilot();
            UpdateDocking();
            UpdateEmbodiment();
            UpdateCameraAndSelection();
            RefreshPlayerFacingModels();
            UpdateFleetCaptains();
        }
        _renderer.Render(BuildRenderFrame());
        _window.EndFrame();

        const std::uint64_t limit = options.maxFrames > 0 ? options.maxFrames : (options.runtimeSmoke ? 4u : 0u);
        if (limit > 0 && _engine.GetFrameCount() >= limit) {
            _engine.RequestShutdown();
        }
    }

    _engine.RequestShutdown();
    _renderer.Shutdown();
    _window.Shutdown();
    _engine.Shutdown();
    return 0;
}

void NativeGameApplication::GoToFrontendScreen(FrontendScreen screen)
{
    _frontend.GoTo(screen);
    _frontendSelected = DefaultFrontendCommand(screen);
    _frontendHovered = FrontendCommand::None;
}

void NativeGameApplication::ActivateFrontendCommand(FrontendCommand command)
{
    switch (command) {
        case FrontendCommand::MainNewSandbox:
            _standaloneShipyard=false;
            GoToFrontendScreen(FrontendScreen::NewSandbox);
            break;
        case FrontendCommand::MainShipyard:
            OpenShipyardWorkspace(true);
            break;
        case FrontendCommand::MainLoadSandbox:
            GoToFrontendScreen(FrontendScreen::LoadSandbox);
            break;
        case FrontendCommand::MainSettings:
            GoToFrontendScreen(FrontendScreen::Settings);
            break;
        case FrontendCommand::MainCredits:
            GoToFrontendScreen(FrontendScreen::Credits);
            break;
        case FrontendCommand::MainExit:
            _engine.RequestShutdown();
            break;
        case FrontendCommand::NewToggleStory: {
            auto cfg = _frontend.Config();
            cfg.storyEnabled = !cfg.storyEnabled;
            _frontend.SetConfig(cfg);
            break;
        }
        case FrontendCommand::NewToggleCoop: {
            auto cfg = _frontend.Config();
            cfg.coopEnabled = !cfg.coopEnabled;
            _frontend.SetConfig(cfg);
            break;
        }
        case FrontendCommand::NewSeedPrevious: {
            auto cfg = _frontend.Config();
            cfg.galaxySeed = cfg.galaxySeed > 1 ? cfg.galaxySeed - 1 : 1;
            _frontend.SetConfig(cfg);
            break;
        }
        case FrontendCommand::NewSeedNext: {
            auto cfg = _frontend.Config();
            ++cfg.galaxySeed;
            _frontend.SetConfig(cfg);
            break;
        }
        case FrontendCommand::NewContinue: {
            const auto cfg = _frontend.Config();
            if (_frontend.BeginNewSandbox(cfg)) {
                // The frontend seed is a real sandbox input, not cosmetic UI.
                // Re-seed the authoritative generator before the playable slice
                // is constructed so mouse/keyboard seed changes affect the world.
                const int runtimeSeed = static_cast<int>(cfg.galaxySeed & 0x7fffffffull);
                _engine.GetGalaxyGenerator() = GalaxyGenerator(runtimeSeed);
                _frontendSelected = DefaultFrontendCommand(FrontendScreen::StartingShip);
                _frontendHovered = FrontendCommand::None;
            }
            break;
        }
        case FrontendCommand::StarterPrevious: {
            const int value = (static_cast<int>(_starterCareer) + 4) % 5;
            _starterCareer = static_cast<StarterCareer>(value);
            break;
        }
        case FrontendCommand::StarterNext: {
            const int value = (static_cast<int>(_starterCareer) + 1) % 5;
            _starterCareer = static_cast<StarterCareer>(value);
            break;
        }
        case FrontendCommand::StarterAccept: {
            const auto selection = _frontend.BuildStarter(_starterCareer);
            if (_frontend.ConfirmStarter(selection)) {
                _frontendSelected = DefaultFrontendCommand(FrontendScreen::StationHangar);
                _frontendHovered = FrontendCommand::None;
            }
            break;
        }
        case FrontendCommand::HangarUndock:
            _standaloneShipyard=false;
            GoToFrontendScreen(FrontendScreen::InGame);
            BootstrapPlayableSlice();
            break;
        case FrontendCommand::Back:
            switch (_frontend.Screen()) {
                case FrontendScreen::NewSandbox:
                case FrontendScreen::LoadSandbox:
                case FrontendScreen::Settings:
                case FrontendScreen::Credits:
                    GoToFrontendScreen(FrontendScreen::MainMenu);
                    break;
                case FrontendScreen::StartingShip:
                    GoToFrontendScreen(FrontendScreen::NewSandbox);
                    break;
                case FrontendScreen::StationHangar:
                    GoToFrontendScreen(FrontendScreen::StartingShip);
                    break;
                default:
                    break;
            }
            break;
        case FrontendCommand::None:
            break;
    }
}

void NativeGameApplication::OpenShipyardWorkspace(bool standalone)
{
    _standaloneShipyard=standalone;
    const auto starter=(_hasPlayerShipRecipe&&!_playerShipRecipe.modules.empty())
        ?_playerShipRecipe:_renderer.DefaultShipyardRecipe("INDUSTRIAL",1u);

    if(!standalone&&_docking.stage==DockingExperienceStage::Docked){
        std::vector<ShipyardRefitPart> stationParts;stationParts.reserve(_renderer.ShipyardCatalog().size());
        for(const auto& rec:_renderer.ShipyardCatalog())stationParts.push_back({rec.source.moduleId,99,1.0f,ShipyardRefitPartSource::StationMarket});
        _shipyardRefit=ShipyardRefitSystem::Begin(starter,std::move(stationParts));
        _shipBuilder.Initialize(_renderer.ShipyardCatalog(),starter);
        LoadCurrentShipyardSocketOverrides();
        LoadCurrentShipyardDefinitionOverrides();
        _shipBuilder.SetAppearance(_playerShipAppearance);
        _shipBuilder.SetAvailableModuleIds(ShipyardRefitSystem::AvailableModuleIds(_shipyardRefit));
        _shipBuilder.SetLiveApplyEnabled(true,false);
    }else{
        _shipyardRefit={};
        _shipBuilder.Initialize(_renderer.ShipyardCatalog(),starter);
        LoadCurrentShipyardSocketOverrides();
        LoadCurrentShipyardDefinitionOverrides();
        _shipBuilder.SetAppearance(_playerShipAppearance);
        _shipBuilder.SetAvailableModuleIds({});
        _shipBuilder.SetLiveApplyEnabled(false,standalone);
    }

    _workspace.Open(SandboxWorkspaceMode::ShipBuilder);
    auto& camera=_engine.GetStrategicCamera();
    if(!_shipyardCameraCaptured){
        _preShipyardZoom=camera.GetZoom();
        _preShipyardTargetZoom=camera.GetTargetZoom();
        _preShipyardMinZoom=camera.GetMinZoom();
        _preShipyardMaxZoom=camera.GetMaxZoom();
        _preShipyardYawDegrees=camera.GetVisualYawDegrees();
        _preShipyardHadElevationOverride=camera.HasElevationOverride();
        _preShipyardElevationDegrees=camera.GetElevationOverrideDegrees();
        _shipyardCameraCaptured=true;
    }
    // Shipyard is an inspection/authoring workspace. Its zoom ceiling must be
    // high enough to fill the viewport with one socket, wing root or material
    // boundary; gameplay limits are restored when the workspace closes.
    camera.SetZoomLimits(0.12f,96.0f);
    if(standalone)GoToFrontendScreen(FrontendScreen::InGame);
    // Pass655-674: every construction workspace uses the same explicit 6DOF
    // inspection camera. Opening Shipyard always frames the authored assembly;
    // module selection itself never steals the persistent camera target.
    FrameShipyardView(false);
}

void NativeGameApplication::RestoreGameplayCameraLimits()
{
    auto& camera=_engine.GetStrategicCamera();
    camera.ClearEditorView();
    if(_shipyardCameraCaptured){
        camera.SetZoomLimits(_preShipyardMinZoom,_preShipyardMaxZoom);
        camera.SetZoom(_preShipyardZoom);
        camera.SetTargetZoom(_preShipyardTargetZoom);
        camera.SetVisualYawDegrees(_preShipyardYawDegrees);
        if(_preShipyardHadElevationOverride)camera.SetElevationOverrideDegrees(_preShipyardElevationDegrees);
        else camera.ClearElevationOverride();
        _shipyardCameraCaptured=false;
    }else{
        camera.SetZoomLimits(0.12f,28.0f);
    }
}

void NativeGameApplication::FrameShipyardView(bool selectedModule)
{
    if(!_shipBuilder.IsInitialized())return;
    auto& camera=_engine.GetStrategicCamera();
    camera.SetZoomLimits(0.12f,96.0f);
    camera.ClearPanOffset();

    Vector3 shipOrigin{};
    float shipYaw=0.0f;
    if(!_standaloneShipyard){
        if(const auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity)){
            shipOrigin=player->position;
            shipYaw=player->rotation.z;
        }
    }

    const auto& modules=_shipBuilder.Recipe().modules;
    Vector3 localCenter{};
    if(selectedModule){
        if(const auto* placed=_shipBuilder.SelectedPlacedModule())localCenter={placed->x,placed->y,placed->z};
    }else if(!modules.empty()){
        for(const auto& placed:modules){localCenter.x+=placed.x;localCenter.y+=placed.y;localCenter.z+=placed.z;}
        const float inv=1.0f/static_cast<float>(modules.size());
        localCenter=localCenter*inv;
    }
    const Vector3 worldCenter=shipOrigin+ShipLocalToWorld(localCenter,shipYaw);

    float radius=selectedModule?1.5f:1.0f;
    if(!selectedModule){
        for(const auto& placed:modules){
            const float dx=placed.x-localCenter.x,dy=placed.y-localCenter.y,dz=placed.z-localCenter.z;
            radius=std::max(radius,std::sqrt(dx*dx+dy*dy+dz*dz));
        }
    }
    ConstructionEditorCameraSystem::Reset(_constructionCamera,worldCenter,std::max(1.5f,radius));
    ApplyConstructionCameraView();
}

void NativeGameApplication::ApplyConstructionCameraView()
{
    auto& camera=_engine.GetStrategicCamera();
    camera.SetEditorView(_constructionCamera.eye,ConstructionEditorCameraSystem::Target(_constructionCamera),_constructionCamera.rollDegrees);
}

void NativeGameApplication::UpdateConstructionCameraKeyboard()
{
    if(!_shipBuilder.IsInitialized())return;
    const auto& input=_engine.GetInputState();
    const float dt=std::max(0.0f,_engine.GetLastDeltaTime());
    if(_window.IsAltDown()){
        ConstructionEditorCameraSystem::BeginFreeFly(_constructionCamera);
        const float forward=(input.IsDown(InputAction::ThrustForward)?1.0f:0.0f)-(input.IsDown(InputAction::ThrustReverse)?1.0f:0.0f);
        const float right=(input.IsDown(InputAction::StrafeRight)?1.0f:0.0f)-(input.IsDown(InputAction::StrafeLeft)?1.0f:0.0f);
        const float up=(input.IsDown(InputAction::FirePrimary)?1.0f:0.0f)-(_window.IsControlDown()?1.0f:0.0f);
        ConstructionEditorCameraSystem::MoveFree(_constructionCamera,forward,right,up,dt);
        const float roll=(input.IsDown(InputAction::TurnRight)?1.0f:0.0f)-(input.IsDown(InputAction::TurnLeft)?1.0f:0.0f);
        if(std::fabs(roll)>0.001f)ConstructionEditorCameraSystem::Roll(_constructionCamera,roll*70.0f*dt);
    }else if(_constructionCamera.mode==ConstructionCameraMode::FreeFly){
        ConstructionEditorCameraSystem::EndFreeFly(_constructionCamera);
    }
    ApplyConstructionCameraView();
}

bool NativeGameApplication::ActivateShipyardControl(ShipyardBuilderCommand command,int value)
{
    if(command==ShipyardBuilderCommand::None)return false;
    const bool handled=_shipBuilder.Activate(command,value);
    if(!handled)return false;
    if(command==ShipyardBuilderCommand::FrameSelected)FrameShipyardView(true);
    else if(command==ShipyardBuilderCommand::FrameShip)FrameShipyardView(false);
    return true;
}

void NativeGameApplication::HandleShipyardTransformHotkeys()
{
    if(!_shipBuilder.IsInitialized())return;
    // Alt belongs exclusively to the construction free-camera. Do not let
    // Alt+W/Q/E/R simultaneously switch editor tools.
    if(_window.IsAltDown())return;
    const auto& input=_engine.GetInputState();
    if(input.WasPressed(InputAction::EditorToolSelect))_shipBuilder.Activate(ShipyardBuilderCommand::ToolSelect);
    if(input.WasPressed(InputAction::EditorToolMove))_shipBuilder.Activate(ShipyardBuilderCommand::ToolMove);
    if(input.WasPressed(InputAction::EditorToolRotate))_shipBuilder.Activate(ShipyardBuilderCommand::ToolRotate);
    if(input.WasPressed(InputAction::EditorToolScale)&&_shipBuilder.Model().inspectorTab!=ShipyardInspectorTab::Sockets)_shipBuilder.Activate(ShipyardBuilderCommand::ToolScale);
    if(input.WasPressed(InputAction::EditorDeleteModule))_shipBuilder.Activate(_shipBuilder.Model().inspectorTab==ShipyardInspectorTab::Sockets?ShipyardBuilderCommand::RemoveSocket:ShipyardBuilderCommand::RemoveModule);
    if(input.WasPressed(InputAction::EditorFrameSelected))FrameShipyardView(true);
    if(input.WasPressed(InputAction::EditorFrameShip))FrameShipyardView(false);

    const auto tool=_shipBuilder.Model().transformTool;
    if(tool==ShipyardTransformTool::Select){
        if(input.WasPressed(InputAction::MenuNext))_shipBuilder.Activate(ShipyardBuilderCommand::NextModule);
        if(input.WasPressed(InputAction::MenuPrevious))_shipBuilder.Activate(ShipyardBuilderCommand::PreviousModule);
        return;
    }

    const bool fine=_window.IsShiftDown();
    if(tool==ShipyardTransformTool::Move){
        float x=0.0f,y=0.0f,z=0.0f;
        if(input.WasPressed(InputAction::EditorNudgeLeft))x-=.10f;
        if(input.WasPressed(InputAction::EditorNudgeRight))x+=.10f;
        if(input.WasPressed(InputAction::EditorNudgeForward))y+=.10f;
        if(input.WasPressed(InputAction::EditorNudgeAft))y-=.10f;
        if(input.WasPressed(InputAction::EditorNudgeUp))z+=.10f;
        if(input.WasPressed(InputAction::EditorNudgeDown))z-=.10f;
        if(std::fabs(x)+std::fabs(y)+std::fabs(z)<=0.0001f)return;

        Vector3 local{x,y,z};
        const auto space=_shipBuilder.Model().transformSpace;
        const bool socketEdit=_shipBuilder.Model().inspectorTab==ShipyardInspectorTab::Sockets;
        if(space==ShipyardTransformSpace::View){
            const auto& camera=_engine.GetStrategicCamera();
            const Vector3 world=camera.ViewRightPlanar()*x+camera.ViewUpPlanar()*y+Vector3{0,0,z};
            float shipYaw=0.0f;
            if(!_standaloneShipyard){if(const auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity))shipYaw=player->rotation.z;}
            local=WorldDeltaToShipLocal(world,shipYaw);
            if(socketEdit)if(const auto* placed=_shipBuilder.SelectedPlacedModule())local=ShipDeltaToModuleLocal(local,*placed);
        }else if(space==ShipyardTransformSpace::Ship&&socketEdit){
            if(const auto* placed=_shipBuilder.SelectedPlacedModule())local=ShipDeltaToModuleLocal(local,*placed);
        }else if(space==ShipyardTransformSpace::Local&&!socketEdit){
            if(const auto* placed=_shipBuilder.SelectedPlacedModule()){
                constexpr float kDegToRad=3.14159265358979323846f/180.0f;
                const float a=placed->yawDegrees*kDegToRad,c=std::cos(a),sn=std::sin(a);
                local={x*c-y*sn,x*sn+y*c,z};
            }
        }
        if(socketEdit){if(_shipBuilder.TranslateSelectedSocket(local,fine))_shipBuilder.CommitSocketTransform();}
        else if(_shipBuilder.TranslateSelected(local,fine))_shipBuilder.CommitTransform();
        return;
    }

    if(tool==ShipyardTransformTool::Rotate){
        Vector3 delta{};
        const float step=_shipBuilder.Model().rotationStepDegrees;
        if(input.WasPressed(InputAction::EditorNudgeLeft))delta.y-=step;
        if(input.WasPressed(InputAction::EditorNudgeRight))delta.y+=step;
        if(input.WasPressed(InputAction::EditorNudgeForward))delta.x+=step;
        if(input.WasPressed(InputAction::EditorNudgeAft))delta.x-=step;
        if(input.WasPressed(InputAction::EditorNudgeUp))delta.z+=step;
        if(input.WasPressed(InputAction::EditorNudgeDown))delta.z-=step;
        if(std::fabs(delta.x)+std::fabs(delta.y)+std::fabs(delta.z)>0.0001f){
            if(_shipBuilder.Model().inspectorTab==ShipyardInspectorTab::Sockets){if(_shipBuilder.RotateSelectedSocket(delta,fine))_shipBuilder.CommitSocketTransform();}
            else if(_shipBuilder.RotateSelected(delta,fine))_shipBuilder.CommitTransform();
        }
        return;
    }

    if(tool==ShipyardTransformTool::Scale){
        float amount=0.0f;
        if(input.WasPressed(InputAction::EditorNudgeRight)||input.WasPressed(InputAction::EditorNudgeForward)||input.WasPressed(InputAction::EditorNudgeUp))amount+=.05f;
        if(input.WasPressed(InputAction::EditorNudgeLeft)||input.WasPressed(InputAction::EditorNudgeAft)||input.WasPressed(InputAction::EditorNudgeDown))amount-=.05f;
        if(std::fabs(amount)>0.0001f&&_shipBuilder.ScaleSelected({amount,amount,amount},fine))_shipBuilder.CommitTransform();
    }
}

void NativeGameApplication::SaveCurrentShipyardBlueprint()
{
    if(!_shipBuilder.IsInitialized()||_shipBuilder.Recipe().modules.empty())return;
    const auto root=FindProjectRootForBlueprints();
    if(root.empty()){_shipBuilder.MarkSaved("ERROR project root not found");return;}
    std::error_code ec;const auto dir=root/"dist"/"blueprints";std::filesystem::create_directories(dir,ec);
    if(ec){_shipBuilder.MarkSaved("ERROR could not create dist/blueprints");return;}

    ShipBlueprintDocument doc;doc.recipe=_shipBuilder.Recipe();doc.appearance=_shipBuilder.Appearance();doc.name="Subspace "+(doc.recipe.role.empty()?std::string("Ship"):doc.recipe.role)+" "+std::to_string(doc.recipe.seed);doc.author="PLAYER";
    doc.equipmentSlots=ShipyardEquipmentSystem::BuildSlots(doc.recipe,_shipBuilder.Model().catalog);
    doc.tags={"PLAYER_AUTHORED","NATIVE_SHIPYARD","REVIEWABLE"};doc.blueprintId=ShipBlueprintLibrarySystem::CanonicalId(doc);doc.recipe.recipeId=doc.blueprintId;
    const std::string stem=SafeBlueprintStem(doc.name)+"_"+BlueprintTimestamp();
    const auto nativePath=dir/(stem+".subspace_ship");
    const auto exchangePath=dir/(stem+".subspace_shipyard.json");
    const auto samplePath=dir/(stem+".subspace_shipyard_example.json");
    std::string nativeError,exchangeError,sampleError;
    const bool nativeOk=ShipBlueprintLibrarySystem::Save(doc,nativePath.string(),&nativeError);
    const bool exchangeOk=ShipyardDesignExchangeSystem::Save(doc,_shipBuilder.Model().catalog,exchangePath.string(),&exchangeError,"Pass533B");
    const auto validation=_shipBuilder.Validate();
    const bool sampleOk=ShipyardAuthoringSampleSystem::Save(_shipBuilder.BaselineRecipe(),doc.recipe,doc.appearance,_shipBuilder.Model().catalog,validation.errors,validation.warnings,samplePath.string(),&sampleError,doc.author);
    if(nativeOk&&exchangeOk&&sampleOk){
        std::ofstream pointer(dir/"LATEST_SHIP_BLUEPRINT.txt",std::ios::trunc);
        pointer<<"Native blueprint: "<<nativePath.string()<<"\n"
               <<"Blender interchange: "<<exchangePath.string()<<"\n"
               <<"Generator refinement sample: "<<samplePath.string()<<"\n"
               <<"Certification: "<<(validation.valid?"PASS":"DRAFT - REVIEW REQUIRED")<<"\n";
        std::ofstream samplePointer(dir/"LATEST_SHIPYARD_AUTHORING_SAMPLE.txt",std::ios::trunc);
        samplePointer<<samplePath.string()<<"\n"
                     <<"Upload this .subspace_shipyard_example.json with screenshots/debug evidence for generator refinement review.\n";
        _shipBuilder.MarkSaved(samplePath.filename().string());
        Logger::Instance().Info("Shipyard","Saved blueprint + generator refinement sample: "+samplePath.string());
    }else{
        std::string error=nativeOk?(exchangeOk?sampleError:exchangeError):nativeError;
        _shipBuilder.MarkSaved("ERROR "+error);
    }
}

void NativeGameApplication::SaveCurrentShipyardSocketOverrides()
{
    if(!_shipBuilder.IsInitialized())return;
    const auto root=FindProjectRootForBlueprints();
    if(root.empty()){_shipBuilder.MarkSocketOverridesSaved("ERROR project root not found");return;}
    std::error_code ec;const auto dir=root/"content"/"authoring";std::filesystem::create_directories(dir,ec);
    if(ec){_shipBuilder.MarkSocketOverridesSaved("ERROR could not create content/authoring");return;}
    const auto path=dir/"shipyard_socket_overrides.subspace_socket_overrides";
    std::string error;std::size_t changed=0;
    if(_shipBuilder.SaveSocketOverrides(path.string(),&error,&changed)){
        _renderer.ApplyShipyardCatalogAuthoringOverrides(_shipBuilder.Model().catalog);
        _shipBuilder.MarkSocketOverridesSaved(path.filename().string()+" / "+std::to_string(changed)+" module(s)");
        Logger::Instance().Info("Shipyard","Saved persistent socket overrides: "+path.string()+"; changed modules "+std::to_string(changed));
    }else _shipBuilder.MarkSocketOverridesSaved("ERROR "+error);
}

void NativeGameApplication::LoadCurrentShipyardSocketOverrides()
{
    if(!_shipBuilder.IsInitialized())return;
    const auto root=FindProjectRootForBlueprints();if(root.empty())return;
    const auto path=root/"content"/"authoring"/"shipyard_socket_overrides.subspace_socket_overrides";
    std::error_code ec;if(!std::filesystem::exists(path,ec)||ec)return;
    std::string error;std::size_t applied=0;
    if(_shipBuilder.LoadSocketOverrides(path.string(),&error,&applied)){
        _renderer.ApplyShipyardCatalogAuthoringOverrides(_shipBuilder.Model().catalog);
        Logger::Instance().Info("Shipyard","Loaded persistent socket overrides: "+std::to_string(applied)+" module(s)");
    }else Logger::Instance().Warning("Shipyard","Could not load socket overrides: "+error);
}


void NativeGameApplication::SaveCurrentShipyardDefinitionOverrides()
{
    if(!_shipBuilder.IsInitialized())return;
    const auto root=FindProjectRootForBlueprints();
    if(root.empty()){_shipBuilder.MarkDefinitionOverridesSaved("ERROR project root not found");return;}
    std::error_code ec;const auto dir=root/"content"/"authoring";std::filesystem::create_directories(dir,ec);
    if(ec){_shipBuilder.MarkDefinitionOverridesSaved("ERROR could not create content/authoring");return;}
    const auto path=dir/"shipyard_definition_overrides.subspace_definition_overrides";
    std::string error;std::size_t changed=0;
    if(_shipBuilder.SaveDefinitionOverrides(path.string(),&error,&changed)){
        _renderer.ApplyShipyardCatalogAuthoringOverrides(_shipBuilder.Model().catalog);
        _shipBuilder.MarkDefinitionOverridesSaved(path.filename().string()+" / "+std::to_string(changed)+" module(s)");
        Logger::Instance().Info("Shipyard","Saved Teach PCG definition overrides: "+path.string()+"; changed modules "+std::to_string(changed));
    }else _shipBuilder.MarkDefinitionOverridesSaved("ERROR "+error);
}

void NativeGameApplication::LoadCurrentShipyardDefinitionOverrides()
{
    if(!_shipBuilder.IsInitialized())return;
    const auto root=FindProjectRootForBlueprints();if(root.empty())return;
    const auto path=root/"content"/"authoring"/"shipyard_definition_overrides.subspace_definition_overrides";
    std::error_code ec;if(!std::filesystem::exists(path,ec)||ec)return;
    std::string error;std::size_t applied=0;
    if(_shipBuilder.LoadDefinitionOverrides(path.string(),&error,&applied)){
        _renderer.ApplyShipyardCatalogAuthoringOverrides(_shipBuilder.Model().catalog);
        Logger::Instance().Info("Shipyard","Loaded Teach PCG definition overrides: "+std::to_string(applied)+" module(s)");
    }else Logger::Instance().Warning("Shipyard","Could not load definition overrides: "+error);
}

void NativeGameApplication::ProcessShipyardRequests()
{
    if(_shipBuilder.ConsumeSaveRequested())SaveCurrentShipyardBlueprint();
    if(_shipBuilder.ConsumeSocketOverridesSaveRequested())SaveCurrentShipyardSocketOverrides();
    if(_shipBuilder.ConsumeDefinitionOverridesSaveRequested())SaveCurrentShipyardDefinitionOverrides();
    if(_shipBuilder.ConsumeApplyRequested()){
        ShipyardRefitDelta delta;
        if(_docking.stage==DockingExperienceStage::Docked&&ShipyardRefitSystem::Commit(_shipyardRefit,_shipBuilder.Recipe(),&delta)){
            _playerShipRecipe=_shipBuilder.Recipe();_playerShipAppearance=_shipBuilder.Appearance();_hasPlayerShipRecipe=!_playerShipRecipe.modules.empty();_shipBuilder.MarkApplied();
            if(_playerEntity!=0&&_hasPlayerShipRecipe){auto& interiors=_engine.GetRuntimeServices().interiors;interiors.ClearLayout(_playerEntity);ShipInteriorLayoutSystem layout;layout.Materialize(_playerEntity,_shipBuilder.Model().catalog,_playerShipRecipe,interiors);}
        }
    }
}

void NativeGameApplication::HandleFrontendActions()
{
    if (_frontend.Screen() == FrontendScreen::InGame) return;

    const auto& input = _engine.GetInputState();
    const auto controls = BuildFrontendControls(_frontend.Screen(), _window.GetWidth(), _window.GetHeight());

    // Hover is visual selection as well: every visible frontend action can be
    // discovered and activated with the mouse without requiring keyboard focus.
    _frontendHovered = HitTestFrontendControls(controls, _window.GetPointerX(), _window.GetPointerY());

    if (input.WasPressed(InputAction::MenuNext)) {
        _frontendSelected = StepFrontendCommand(controls, _frontendSelected, +1);
    }
    if (input.WasPressed(InputAction::MenuPrevious)) {
        _frontendSelected = StepFrontendCommand(controls, _frontendSelected, -1);
    }

    if (input.WasPressed(InputAction::MenuBack)) {
        ActivateFrontendCommand(FrontendCommand::Back);
        return;
    }

    // Consume LMB only while the frontend owns the screen. This prevents a
    // menu click from leaking into gameplay selection on the transition frame.
    float clickX = 0.0f, clickY = 0.0f;
    if (_window.ConsumePrimaryClick(clickX, clickY)) {
        const auto clicked = HitTestFrontendControls(controls, clickX, clickY);
        if (clicked != FrontendCommand::None) {
            _frontendSelected = clicked;
            ActivateFrontendCommand(clicked);
        }
        return;
    }

    if (input.WasPressed(InputAction::MenuAccept)) {
        ActivateFrontendCommand(_frontendSelected);
    }
}

void NativeGameApplication::BootstrapPlayableSlice()
{
    // Pick the most visually useful nearby deterministic sector instead of
    // hardcoding fake presentation objects. This remains seed-dependent game data.
    int bestScore = -1;
    for (int y=-2; y<=2; ++y) {
        for (int x=-2; x<=2; ++x) {
            auto candidate = _engine.GetGalaxyGenerator().GenerateSector(x,y,0);
            const int score = static_cast<int>(candidate.planets.size())*4 +
                              static_cast<int>(candidate.ships.size()) +
                              (candidate.hasStation?5:0) +
                              static_cast<int>(candidate.derelicts.size())*3 +
                              static_cast<int>(candidate.orbitalHubs.size())*4 +
                              static_cast<int>(candidate.pointsOfInterest.size())*2;
            if (score > bestScore) { bestScore=score; _sector=std::move(candidate); }
        }
    }

    // Pass496: the selected starter system must be spawn-certified before any
    // local flight materialization. The debug bundle can now prove whether the
    // generated macro system passed the same authority used for all systems.
    {
        SolarSystemPlacementSystem placement;
        const auto certification=placement.Validate(_sector);
        Logger::Instance().Info("SolarSystem", "Pass496 solar placement certification: status=" + std::string(certification.certified?"PASS":"FAIL") +
                     " violations=" + std::to_string(certification.violations) +
                     " planets=" + std::to_string(_sector.planets.size()) +
                     " moons=" + std::to_string(_sector.moons.size()) +
                     " belts=" + std::to_string(_sector.asteroidBelts.size()) +
                     " asteroids=" + std::to_string(_sector.asteroids.size()) +
                     " sites=" + std::to_string(_sector.pointsOfInterest.size()));
    }

    // Pass369/365 runtime integration: the starting regional slice must always
    // demonstrate a real dockable station and must never fall back to the
    // stellar origin. If the selected deterministic system has no station,
    // create a deterministic frontier berth around its first planet (or a
    // safe stellar orbit when no planet exists).
    if (!_sector.hasStation) {
        _sector.hasStation = true;
        _sector.station.name = "Axiom Frontier Berth";
        _sector.station.stationType = "Frontier";
        _sector.station.stationId = "station_" + std::to_string(_sector.x) + "_" + std::to_string(_sector.y) + "_" + std::to_string(_sector.z) + "_fallback";
        if (!_sector.planets.empty()) {
            const auto& anchorPlanet = _sector.planets.front();
            const float orbit = anchorPlanet.radius * 4.4f + 2200.0f;
            _sector.station.position = {anchorPlanet.position.x + orbit, anchorPlanet.position.y, 0.0f};
        } else {
            _sector.station.position = {_sector.star.position.x + 180000.0f, _sector.star.position.y, 0.0f};
        }
    }

    // Pass410R3 celestial-safe regional anchoring. The enlarged planets/star
    // are presentation-scale bodies, so legacy generated coordinates can place
    // an otherwise valid station inside their visible surface. Move the entire
    // local gameplay neighborhood to the nearest safe orbital envelope before
    // spawning the player or ambient traffic.
    if(_sector.hasStation){
        Vector3 safeStation=NativeBattlefieldRenderer::SectorToWorld(_sector.station.position);
        if(_sector.hasStar){
            const Vector3 starWorld=NativeBattlefieldRenderer::SectorToWorld(_sector.star.position);
            const auto starEnvelope=_orbitalDynamics.StarSafety(_sector.star);
            const float dx=safeStation.x-starWorld.x,dy=safeStation.y-starWorld.y;
            const float d=std::sqrt(dx*dx+dy*dy);
            const float safe=starEnvelope.spawnRadius+320.0f;
            if(d<safe){const float nx=d>.001f?dx/d:1.0f,ny=d>.001f?dy/d:0.0f;safeStation={starWorld.x+nx*safe,starWorld.y+ny*safe,0.0f};}
        }
        // Multiple passes handle rare overlapping celestial presentation
        // envelopes without allowing the correction for one body to enter another.
        for(int pass=0;pass<3;++pass){
            for(const auto& planet:_sector.planets){
                // Pass429: the old +300 world-unit clearance pushed the
                // orbital berth completely beyond the flight camera. Keep a
                // small safety margin outside the celestial/atmosphere envelope
                // so undocking visibly occurs beside the planet again.
                safeStation=ProjectOutsidePlanetEnvelope(safeStation,NativeBattlefieldRenderer::SectorToWorld(planet.position),planet,10.0f);
            }
        }
        _sector.station.position=NativeBattlefieldRenderer::WorldToSector(safeStation);
    }

    // The playable flight scene is a co-moving regional neighborhood around
    // the selected station. Ambient contacts generated near the raw system
    // origin are re-anchored here so enlarged celestial bodies cannot swallow
    // the player, traffic, salvage, or the first docking demonstration.
    const SectorPosition regionalAnchor = _sector.station.position;
    auto regionalOffset = [](std::size_t i, float ring) {
        const float a = 0.91f + static_cast<float>(i) * 2.39996323f;
        return SectorPosition{std::cos(a)*ring, std::sin(a)*ring, 0.0f};
    };
    for (std::size_t i=0;i<_sector.ships.size();++i) { const auto o=regionalOffset(i,1200.0f+420.0f*float(i%5)); _sector.ships[i].position={regionalAnchor.x+o.x,regionalAnchor.y+o.y,0}; }
    for (std::size_t i=0;i<_sector.derelicts.size();++i) { const auto o=regionalOffset(i+11,2600.0f+500.0f*float(i%3)); _sector.derelicts[i].position={regionalAnchor.x+o.x,regionalAnchor.y+o.y,0}; }
    for (std::size_t i=0;i<_sector.debrisFields.size();++i) { const auto o=regionalOffset(i+19,3300.0f+650.0f*float(i%4)); _sector.debrisFields[i].position={regionalAnchor.x+o.x,regionalAnchor.y+o.y,0}; }

    // R5 authored Shipyard Strikes Back vessels remain real universe ships.
    // They are not procedural templates; their dimensions/silhouette are only
    // design-language references. Scout starts disabled so takeover is directly testable.
    const auto& authoredDefs=ShipyardAuthoredShipSystem::Definitions();
    for(std::size_t i=0;i<authoredDefs.size();++i){
        ShipData ship; ship.shipId="authored_"+authoredDefs[i].id; ship.visualSeed=0x515500u+static_cast<std::uint32_t>(i);
        const auto o=regionalOffset(i+31,1850.0f+430.0f*static_cast<float>(i));
        ship.position={regionalAnchor.x+o.x,regionalAnchor.y+o.y,0.0f}; ship.shipType=authoredDefs[i].role;
        ship.faction=(i==1)?"Independent Defense":"Independent"; ship.heading=.35f*static_cast<float>(i);
        ship.authoredShipId=authoredDefs[i].id; ship.capturable=true; ship.disabled=(i==0);
        _sector.ships.push_back(std::move(ship));
    }

    // Pass514: regional re-anchoring and authored contact injection happen
    // after macro generation, so certify those final materialization positions
    // too. This closes the last path that could visually put traffic/rocks/wrecks
    // back through a star, planet atmosphere, or dense-ring envelope.
    {
        SolarSystemPlacementSystem placement;
        if(_sector.hasStation) _sector.station.position=placement.RepairSpawnPosition(_sector,_sector.station.position,8000.0f,2500.0f);
        for(auto& ship:_sector.ships) ship.position=placement.RepairSpawnPosition(_sector,ship.position,6000.0f,1800.0f);
        for(auto& wreck:_sector.derelicts) wreck.position=placement.RepairSpawnPosition(_sector,wreck.position,6000.0f,1800.0f);
        for(auto& debris:_sector.debrisFields) debris.position=placement.RepairSpawnPosition(_sector,debris.position,6000.0f,1800.0f);
        for(auto& rock:_sector.asteroids) rock.position=placement.RepairSpawnPosition(_sector,rock.position,6000.0f,1800.0f);
    }
    {
        SolarSystemEcologySystem ecology;
        const auto report=ecology.Audit(_sector);
        Logger::Instance().Info("SolarSystem","Pass524 ecology certification: status="+std::string(report.certified?"PASS":"REVIEW")+
            " score="+std::to_string(report.distributionScore)+" bands="+std::to_string(report.occupiedBands)+
            " traffic="+std::to_string(report.localTraffic)+" unsafe="+std::to_string(report.unsafeContacts));
    }

    _orbitalBodies = _orbitalDynamics.DeriveSystemOrbits(_sector);
    _orbitalSimulationSeconds = 0.0;
    _orbitalMapRefreshAccumulator = 0.0;
    _universeSystemMap = _universeSystemMapSystem.Build(_orbitalBodies,_orbitalSimulationSeconds,72);

    auto& player = _engine.GetEntityManager().CreateEntity("Player Industrial Cutter");
    _playerEntity = player.id;
    auto physics = std::make_unique<PhysicsComponent>();
    physics->mass = 1450.0f;
    physics->momentOfInertia = 1100.0f;
    // Pass198: heavy industrial cutter, deliberately Newtonian-ish. These
    // forces yield seconds-long acceleration rather than instant warp speed.
    physics->maxThrust = 5200.0f;
    physics->maxTorque = 1650.0f;
    physics->drag = 0.012f;
    physics->angularDrag = 0.075f;
    physics->collisionRadius = 1.4f;
    if (_sector.hasStation) {
        const Vector3 spawn=NativeBattlefieldRenderer::SectorToWorld(_sector.station.position);
        physics->position={spawn.x-3.0f,spawn.y-2.0f,0.0f};
    } else if (!_sector.pointsOfInterest.empty()) {
        const Vector3 spawn=NativeBattlefieldRenderer::SectorToWorld(_sector.pointsOfInterest.front().position);
        physics->position={spawn.x-3.0f,spawn.y-2.0f,0.0f};
    } else {
        physics->position={0.0f,-14.0f,0.0f};
    }
    if (_sector.hasStar) {
        const Vector3 starWorld = NativeBattlefieldRenderer::SectorToWorld(_sector.star.position);
        const auto envelope = _orbitalDynamics.StarSafety(_sector.star);
        if (!_orbitalDynamics.IsOutsideStarSafety(physics->position, starWorld, envelope)) {
            physics->position = {starWorld.x + envelope.spawnRadius + 320.0f, starWorld.y, 0.0f};
        }
    }
    // The player gets its own final body-surface guard because the small
    // docking offset can otherwise step back across a large planet envelope.
    for(int pass=0;pass<3;++pass){
        for(const auto& planet:_sector.planets){
            physics->position=ProjectOutsidePlanetEnvelope(physics->position,NativeBattlefieldRenderer::SectorToWorld(planet.position),planet,32.0f);
        }
    }
    _engine.GetEntityManager().AddComponent<PhysicsComponent>(_playerEntity,std::move(physics));

    // Pass513: combat durability is authoritative runtime state. HUD/shield
    // presentation must never manufacture 100% values independently.
    auto combat = std::make_unique<CombatComponent>();
    combat->armorRating = 8.0f;
    combat->shields.shieldRegenRate = 9.0f;
    combat->shields.shieldRechargeDelay = 4.0f;
    combat->ConfigureDurability(180.0f, 160.0f, 240.0f, 140.0f, false);
    _engine.GetEntityManager().AddComponent<CombatComponent>(_playerEntity,std::move(combat));

    // Pass527: materialize the player's interior from the same authored
    // Shipyard recipe that defines the exterior. Large/complex ships naturally
    // gain additional crew, cargo, workshop and connector spaces.
    if(_hasPlayerShipRecipe&&!_playerShipRecipe.modules.empty()){
        ShipInteriorLayoutSystem interiorLayout;
        const auto plan=interiorLayout.Materialize(_playerEntity,_renderer.ShipyardCatalog(),_playerShipRecipe,_engine.GetRuntimeServices().interiors);
        Logger::Instance().Info("Interior","Pass527 authored interior materialized: rooms="+std::to_string(plan.rooms)+" decks="+std::to_string(plan.decks)+" airlocks="+std::to_string(plan.airlocks));
    }

    if (auto* controls=_engine.GetPlayerControlSystem()) controls->SetControlledShip(_playerEntity);
    _embodiment = ShipEmbodimentSystem{};
    _docking = DockingExperienceState{};
    _miningLoop.Initialize(_sector);
    _engine.GetStrategicCamera().SetZoomLimits(0.12f,28.0f);
    // Pass191 strategic view: the camera is physically presented above and
    // behind the authoritative 2D gameplay plane.  Tilt/height are render-only
    // parameters consumed by StrategicViewProjection.
    _engine.GetStrategicCamera().SetZoom(0.56f);
    _engine.GetStrategicCamera().SetVisualTilt(0.46f);
    _engine.GetStrategicCamera().SetVisualHeight(0.68f);
    _engine.GetStrategicCamera().SetVelocityLookAhead(0.24f);

    // Pass404/405: the first playable system owns an explicit station ecology
    // profile so the same station identity drives flight-space presence and
    // the universal docked-hangar presentation.
    {
        StationEcologySystem ecology;
        const std::uint64_t seed = static_cast<std::uint64_t>((_sector.x+17)*73856093u ^ (_sector.y+31)*19349663u ^ (_sector.z+7)*83492791u);
        auto stations = ecology.BuildStartingSystem(seed,4,true);
        if(!stations.empty()) { _primaryStationProfile=stations.front(); _primaryStationProfile.name=_sector.station.name; _activeStationProfile=_primaryStationProfile; }

        // Pass404 runtime closure: materialize the rest of the starting-system
        // ecology as visible/dockable local contacts instead of leaving those
        // station profiles trapped in an acceptance model. The primary sector
        // station remains index 0; these contacts begin at selection index 1.
        const Vector3 stationWorld=NativeBattlefieldRenderer::SectorToWorld(_sector.station.position);
        auto contacts=_playerFacing.BuildStationContacts(static_cast<std::uint32_t>(seed),stationWorld,120.0f,true);
        _stationContacts.clear();
        for(std::size_t i=1;i<contacts.size();++i)_stationContacts.push_back(std::move(contacts[i]));

        // Give the same visible contacts macro orbital records so the living
        // System Map and moving-target navigation know about them too.
        std::uint64_t parent=1ull;const PlanetData* stationParent=nullptr;
        for(std::size_t p=0;p<_sector.planets.size();++p){
            if(_sector.planets[p].planetId==_sector.station.parentObjectId){parent=100ull+p;stationParent=&_sector.planets[p];break;}
        }
        SolarSystemPlacementSystem placement;
        const double ecologyOrbitBase=stationParent?placement.PlanetEnvelopeRadiusSector(*stationParent)+65000.0:placement.StellarExclusionRadiusSector(_sector.star)+110000.0;
        for(std::size_t i=0;i<_stationContacts.size();++i){
            const auto& c=_stationContacts[i];
            OrbitalBodyRecord b; b.id=5000u+i; b.parentId=parent; b.name=c.name;
            b.kind=c.asteroidEmbedded?OrbitalBodyKind::AsteroidStation:OrbitalBodyKind::Station;
            b.physicalRadius=c.asteroidEmbedded?180.0f:110.0f; b.dockable=c.dockable;
            // Additional ecology stations now use the same macro celestial
            // envelope authority as the primary station instead of tiny legacy
            // 1.4k-sector orbits that could live inside giant planets/rings.
            b.orbit.semiMajorAxis=ecologyOrbitBase+45000.0*static_cast<double>(i);
            b.orbit.eccentricity=c.asteroidEmbedded?0.035:0.003;
            b.orbit.inclinationDegrees=c.asteroidEmbedded?5.0:(1.0+1.5*static_cast<double>(i));
            b.orbit.meanAnomalyAtEpochDegrees=118.0+97.0*static_cast<double>(i);
            b.orbit.orbitalPeriodSeconds=1500.0+420.0*static_cast<double>(i);
            _orbitalBodies.push_back(b);
        }
        _universeSystemMap=_universeSystemMapSystem.Build(_orbitalBodies,_orbitalSimulationSeconds,72);
    }
    _galaxyRuntime=_playerFacing.BuildGalaxyRuntime(1u,10000);
    RefreshPlayerFacingModels();
    RebuildSystemMap();
}

void NativeGameApplication::RebuildSystemMap()
{
    // Pass271: System Map and Vector navigation are now populated through the
    // normalized production integration authority instead of duplicating the
    // map-to-navigation translation inside the application shell.
    _systemNavigation = SystemNavigationSystem{};
    _systemMap = _engine.GetRuntimeServices().navigationMiningIntegration
        .BuildMapAndNavigation(_sector,_systemNavigation);

    // Pass404/406: additional ecology stations are first-class System Map
    // destinations. Their positions come from the same orbital snapshot used
    // by the Universe-style map rather than their local co-moving render offset.
    for(const auto& contact:_stationContacts){
        SystemMapNode n; n.id=contact.id; n.kind=SystemMapNodeKind::Station; n.label=contact.name;
        n.known=true; n.warpable=true; n.hazard=contact.asteroidEmbedded?0.28f:0.08f;
        n.strategicRadius=contact.asteroidEmbedded?190.0f:120.0f;
        if(const auto it=std::find_if(_universeSystemMap.nodes.begin(),_universeSystemMap.nodes.end(),[&](const UniverseMapNode& o){return o.name==contact.name;});it!=_universeSystemMap.nodes.end()){
            n.position={it->currentPosition.x,it->currentPosition.y,it->currentPosition.z};
        } else {
            n.position={contact.position.x,contact.position.y,contact.position.z};
        }
        n.distanceFromOrigin=std::sqrt(n.position.x*n.position.x+n.position.y*n.position.y);
        n.regionLabel=contact.asteroidEmbedded?"ASTEROID INSTALLATION":"ORBITAL INFRASTRUCTURE";
        _systemMap.nodes.push_back(n);

        SystemDestination d;d.id=n.id;d.name=n.label;d.type=SystemDestinationType::Station;d.discovered=true;d.warpable=true;d.hazardRating=n.hazard;
        d.position.localX=static_cast<double>(n.position.x)*1000000.0;d.position.localY=static_cast<double>(n.position.y)*1000000.0;
        AstronomicalScaleSystem localScale;d.position=localScale.Normalize(d.position);_systemNavigation.RegisterDestination(d);
    }

    // Pass474: moons were visible in the analytical ephemeris but absent from
    // the selectable SystemMap destination model. Promote them into the same
    // node/navigation authority so mouse selection, labels and Vector travel
    // all agree on what the map is showing.
    for(const auto& orbitNode:_universeSystemMap.nodes){
        if(orbitNode.kind!=OrbitalBodyKind::Moon)continue;
        if(std::any_of(_systemMap.nodes.begin(),_systemMap.nodes.end(),[&](const SystemMapNode& n){return n.label==orbitNode.name;}))continue;
        SystemMapNode n;
        n.id=orbitNode.id;
        n.kind=SystemMapNodeKind::Moon;
        n.label=orbitNode.name;
        n.position={orbitNode.currentPosition.x,orbitNode.currentPosition.y,orbitNode.currentPosition.z};
        n.known=true;n.warpable=true;n.hazard=.04f;n.strategicRadius=120.0f;
        n.distanceFromOrigin=std::sqrt(n.position.x*n.position.x+n.position.y*n.position.y);
        n.regionLabel="LUNAR ORBIT";
        _systemMap.nodes.push_back(n);

        SystemDestination d;d.id=n.id;d.name=n.label;d.type=SystemDestinationType::Moon;d.discovered=true;d.warpable=true;d.hazardRating=n.hazard;
        d.position.localX=static_cast<double>(n.position.x)*1000000.0;d.position.localY=static_cast<double>(n.position.y)*1000000.0;
        AstronomicalScaleSystem localScale;d.position=localScale.Normalize(d.position);_systemNavigation.RegisterDestination(d);
    }

    AstronomicalScaleSystem scale;
    if (auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity)) {
        _currentAstronomicalPosition.localX = static_cast<double>(player->position.x / 0.0120f) * 1000000.0;
        _currentAstronomicalPosition.localY = static_cast<double>(player->position.y / 0.0120f) * 1000000.0;
        _currentAstronomicalPosition = scale.Normalize(_currentAstronomicalPosition);
    }
    for (std::size_t i=0;i<_systemMap.nodes.size();++i) {
        if (_systemMap.nodes[i].known && _systemMap.nodes[i].warpable) { _systemMap.selected=i; break; }
    }
}


void NativeGameApplication::UpdateOrbitalSimulation()
{
    if(_orbitalBodies.empty()) return;
    const double dt=std::max(0.0f,_engine.GetLastDeltaTime());
    if(dt<=0.0) return;
    _orbitalSimulationSeconds += dt;
    _orbitalMapRefreshAccumulator += dt;
    if(_orbitalMapRefreshAccumulator < (1.0/12.0) && !_universeSystemMap.nodes.empty()) return;
    _orbitalMapRefreshAccumulator = 0.0;
    _universeSystemMap = _universeSystemMapSystem.Build(_orbitalBodies,_orbitalSimulationSeconds,72);

    // Keep the ordinary SystemMap/Vector destination registry synchronized
    // with the analytical orbit authority. Local flight remains a co-moving
    // regional frame, so updating the macro destination does not drag the
    // player thousands of units through the local scene every frame.
    AstronomicalScaleSystem scale;
    for(const auto& orbitNode:_universeSystemMap.nodes){
        auto it=std::find_if(_systemMap.nodes.begin(),_systemMap.nodes.end(),[&](const SystemMapNode& node){return node.label==orbitNode.name;});
        if(it==_systemMap.nodes.end()) continue;
        it->position.x=orbitNode.currentPosition.x;it->position.y=orbitNode.currentPosition.y;it->position.z=orbitNode.currentPosition.z;
        it->distanceFromOrigin=std::sqrt(it->position.x*it->position.x+it->position.y*it->position.y);
        SystemDestination d;d.id=it->id;d.name=it->label;d.discovered=it->known;d.warpable=it->warpable;d.hazardRating=it->hazard;
        d.position.localX=static_cast<double>(it->position.x)*1000000.0;d.position.localY=static_cast<double>(it->position.y)*1000000.0;d.position=scale.Normalize(d.position);
        switch(it->kind){
            case SystemMapNodeKind::Planet:d.type=SystemDestinationType::Planet;break;
            case SystemMapNodeKind::Moon:d.type=SystemDestinationType::Moon;break;
            case SystemMapNodeKind::Station:case SystemMapNodeKind::OrbitalHub:d.type=SystemDestinationType::Station;break;
            case SystemMapNodeKind::Belt:d.type=SystemDestinationType::BeltRegion;break;
            case SystemMapNodeKind::Salvage:d.type=SystemDestinationType::SalvageSite;break;
            case SystemMapNodeKind::Signature:d.type=SystemDestinationType::Signature;break;
            default:d.type=SystemDestinationType::DeepSpace;break;
        }
        _systemNavigation.RegisterDestination(d);
    }
}

std::size_t NativeGameApplication::ActivePlanetIndex() const
{
    if (_selection.kind==NativeContactKind::Planet && _selection.index<_sector.planets.size()) return _selection.index;
    auto* player=const_cast<Engine&>(_engine).GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);
    if (!player || _sector.planets.empty()) return static_cast<std::size_t>(-1);
    float best=std::numeric_limits<float>::max();std::size_t result=0;
    for(std::size_t i=0;i<_sector.planets.size();++i){const auto p=NativeBattlefieldRenderer::SectorToWorld(_sector.planets[i].position);const float d=Distance2D(player->position,p);if(d<best){best=d;result=i;}}
    return result;
}

void NativeGameApplication::EnsurePlanetSurvey(std::size_t planetIndex)
{
    if (planetIndex>=_sector.planets.size() || _planetSurveys.count(planetIndex)) return;
    const auto& planet=_sector.planets[planetIndex];
    const std::uint64_t seed=static_cast<std::uint64_t>(std::hash<std::string>{}(planet.planetId));
    const std::uint64_t id=static_cast<std::uint64_t>(planetIndex+1);
    PlanetSurveySystem survey;
    auto record=survey.Generate(seed,id,planet.name);
    survey.Advance(record,PlanetSurveyStage::Preliminary);
    _planetSurveys[planetIndex]=record;
}

double NativeGameApplication::CurrentVectorTopSpeedMetersPerSecond() const
{
    // Vector cruise scales from the same top-speed authority used by flight.
    // The current industrial-cutter baseline is 30 world-units/s boost speed
    // -> 4.0e9 m/s Vector top speed. Faster/slower ship propulsion profiles
    // therefore change Vector ETA proportionally instead of every hull receiving
    // the same hard-coded cruise duration.
    constexpr double baselineFlightTopSpeed = 30.0;
    constexpr double baselineVectorTopSpeed = 4.0e9;
    double shipTopSpeed = baselineFlightTopSpeed;
    if (const auto* controls = _engine.GetPlayerControlSystem())
        shipTopSpeed = std::max(1.0, static_cast<double>(controls->GetTuning().boostSpeed));
    return baselineVectorTopSpeed * (shipTopSpeed / baselineFlightTopSpeed);
}

void NativeGameApplication::UpdateVectorTravel()
{
    const double dt=std::max(0.001f,_engine.GetLastDeltaTime());
    ObservableWarpSystem::Tick(_observableWarpEvents,static_cast<float>(dt));
    if (_vectorTravelSystem.InTransit(_vectorTravel)) {
        const auto stageBefore=_vectorTravel.stage;
        _vectorTravelSystem.Update(_vectorTravel,dt);
        if(stageBefore!=_vectorTravel.stage){
            if(auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity)){
                const float yaw=player->rotation.z;
                const Vector3 forward{-std::sin(yaw),std::cos(yaw),0.0f};
                ObservableWarpSystem::EmitStageTransition(_observableWarpEvents,_playerEntity,stageBefore,_vectorTravel.stage,player->position,forward);
            }
            _previousVectorTravelStage=_vectorTravel.stage;
        }

        // Pass355: the ship physically aligns to the destination before the
        // slipstream forms. Charging also adds a short visible acceleration
        // run so entry reads as motion rather than a UI state change.
        if(_activeWarpDestination!=0){
            if(const auto* destination=_systemNavigation.GetDestination(_activeWarpDestination)){
                if(auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity)){
                    const Vector3 target{static_cast<float>(destination->position.localX/1000000.0)*0.0120f,
                                         static_cast<float>(destination->position.localY/1000000.0)*0.0120f,0.0f};
                    const float targetYaw=TravelArrivalSystem::YawToward(player->position,target);
                    const float turnAlpha=1.0f-std::exp(-static_cast<float>(dt)*4.6f);
                    player->rotation.z += ShortestAngleRadians(player->rotation.z,targetYaw)*turnAlpha;
                    if(_vectorTravel.stage==VectorTravelStage::Charging){
                        const float p=static_cast<float>(_vectorTravel.stageProgress);
                        const Vector3 forward{-std::sin(player->rotation.z),std::cos(player->rotation.z),0.0f};
                        const float targetSpeed=4.0f+22.0f*p*p;
                        player->velocity=player->velocity+(forward*targetSpeed-player->velocity)*(turnAlpha*.34f);
                    }
                }
            }
        }
    } else if (_vectorTravel.stage==VectorTravelStage::Complete && _activeWarpDestination==0) {
        _vectorTravel.stageSeconds += dt;
        if (_vectorTravel.stageSeconds > 4.0) { _vectorTravel = VectorTravelSession{}; _activeArrival={}; _arrivalTitle.clear(); }
        return;
    } else return;

    // Resolve and stream the destination as deceleration starts. The tunnel
    // still masks the coordinate transition; its fade reveals the real POI /
    // orbital local scene underneath instead of a post-warp hard teleport.
    if(_vectorTravel.stage==VectorTravelStage::Decelerating && _activeWarpDestination!=0 && !_activeArrival.valid) {
        if(const auto* destination=_systemNavigation.GetDestination(_activeWarpDestination)) {
            _activeArrival=_travelArrivalSystem.Resolve(*destination,_sector,
                _engine.GetPlayerControlSystem()?_engine.GetPlayerControlSystem()->GetTuning().cruiseSpeed:18.0f);
            if(_activeArrival.valid) {
                if(auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity)) {
                    player->position=_activeArrival.arrivalWorld;player->rotation.z=_activeArrival.arrivalYaw;
                    player->velocity={};player->angularVelocity={};
                }
                _currentAstronomicalPosition=destination->position;
            }
        }
    }

    if (_vectorTravel.stage!=VectorTravelStage::Complete || _activeWarpDestination==0) return;
    if(!_activeArrival.valid) {
        if(const auto* destination=_systemNavigation.GetDestination(_activeWarpDestination))
            _activeArrival=_travelArrivalSystem.Resolve(*destination,_sector,18.0f);
    }
    if(!_activeArrival.valid){_vectorTravel.stage=VectorTravelStage::Failed;_vectorTravel.status="NO SAFE ARRIVAL";return;}
    _vectorFuel=std::max(0.0,_vectorFuel-_vectorTravel.plan.fuelCost);
    _selection.Clear();
    if(const auto* destination=_systemNavigation.GetDestination(_activeWarpDestination)) _arrivalTitle=destination->name;
    _vectorTravel.status="LOCAL APPROACH";
    _vectorTravel.stageSeconds=0.0;
    _activeWarpDestination=0;
}

void NativeGameApplication::UpdateVectorCamera()
{
    auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);
    if(!player)return;
    auto& camera=_engine.GetStrategicCamera();
    const bool active=_vectorTravelSystem.InTransit(_vectorTravel);
    const bool arrival=_vectorTravel.stage==VectorTravelStage::Complete && !_arrivalTitle.empty();
    const double dt=std::max(0.001f,_engine.GetLastDeltaTime());
    if(active && !_vectorCameraCaptured){
        _preVectorYawDegrees=camera.GetVisualYawDegrees();
        _preVectorTilt=camera.GetVisualTilt();
        _preVectorZoom=camera.GetTargetZoom();
        _preVectorElevationOverride=camera.HasElevationOverride()?camera.GetElevationOverrideDegrees():-1.0f;
        _vectorCameraCaptured=true;
        _shipInspection=false;
    }
    if(active || arrival){
        const auto profile=CinematicFlightPresentationSystem::Evaluate(
            _vectorTravel.stage,_vectorTravel.stageProgress,arrival?_vectorTravel.stageSeconds:0.0);
        const float shipYaw=player->rotation.z*57.29577951308232f;
        const float restore=arrival?profile.arrivalBlend:0.0f;
        const float desiredYaw=shipYaw+ShortestAngleDegrees(shipYaw,_preVectorYawDegrees)*restore;
        const float yawAlpha=1.0f-std::exp(-static_cast<float>(dt)*profile.cameraEase);
        camera.SetVisualYawDegrees(camera.GetVisualYawDegrees()+ShortestAngleDegrees(camera.GetVisualYawDegrees(),desiredYaw)*yawAlpha);
        camera.SetElevationOverrideDegrees(profile.elevationDegrees);
        camera.SetTargetZoom(profile.targetZoom*(1.0f-restore)+_preVectorZoom*restore);
        camera.SetVisualTilt(_preVectorTilt*restore+0.04f*(1.0f-restore));
    } else if(_vectorCameraCaptured){
        camera.SetVisualYawDegrees(_preVectorYawDegrees);
        camera.SetVisualTilt(_preVectorTilt);
        camera.SetTargetZoom(_preVectorZoom);
        if(_preVectorElevationOverride>=0.0f)camera.SetElevationOverrideDegrees(_preVectorElevationOverride);
        else camera.ClearElevationOverride();
        _vectorCameraCaptured=false;
    }
}

InteractionContext NativeGameApplication::BuildInteractionContext(const NativeContactSelection& selection) const
{
    InteractionContext c;
    c.fleetAvailable=true;
    switch(selection.kind){
        case NativeContactKind::Ship: {
            c.kind=ContextObjectKind::Ship;
            if(selection.index<_sector.ships.size()){const auto& ship=_sector.ships[selection.index];if(ship.hostile)c.kind=ContextObjectKind::HostileShip;c.capturable=ship.capturable;c.disabled=ship.disabled;}
            c.targetable=true;break;
        }
        case NativeContactKind::Planet:c.kind=ContextObjectKind::Planet;c.discovered=true;break;
        case NativeContactKind::Station:c.kind=ContextObjectKind::Station;c.dockable=true;break;
        case NativeContactKind::OrbitalHub:c.kind=ContextObjectKind::Station;c.dockable=false;break;
        case NativeContactKind::Asteroid:c.kind=ContextObjectKind::Asteroid;c.mineable=true;break;
        case NativeContactKind::Derelict:c.kind=ContextObjectKind::Wreck;c.salvageable=true;break;
        case NativeContactKind::Site:c.kind=ContextObjectKind::Poi;break;
        case NativeContactKind::None:default:c.kind=ContextObjectKind::EmptySpace;break;
    }
    return c;
}

void NativeGameApplication::UpdateStrategicAutopilot()
{
    if(_strategicFlight.Mode()!=FlightControlMode::Strategic || !_strategicFlight.CurrentOrder().valid ||
       _workspace.IsOverlayOpen() || _vectorTravelSystem.InTransit(_vectorTravel) ||
       _docking.stage!=DockingExperienceStage::Undocked) return;
    auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);if(!player)return;
    const auto command=_playerFacing.EvaluateAutopilot(_strategicFlight,player->position,player->rotation.z,_strategicFlight.CurrentOrder().targetPosition);
    if(!command.valid)return;
    const float dt=std::max(0.001f,_engine.GetLastDeltaTime());
    const float turnAlpha=1.0f-std::exp(-dt*4.0f);
    player->rotation.z += ShortestAngleRadians(player->rotation.z,command.desiredYawRadians)*turnAlpha;
    const Vector3 forward{-std::sin(player->rotation.z),std::cos(player->rotation.z),0.0f};
    const float targetSpeed=std::clamp(command.throttle,-1.0f,1.0f)*18.0f;
    player->velocity=player->velocity+(forward*targetSpeed-player->velocity)*(1.0f-std::exp(-dt*1.8f));
    if(command.intent.requestDock && _selection.kind==NativeContactKind::Station){
        const Vector3 station=ContactWorldPosition(_sector,_selection,&_stationContacts);
        if(Distance2D(player->position,station)<=180.0f){
            _activeStationProfile=StationProfileForSelection(_primaryStationProfile,_stationContacts,_selection);
            _dockingSystem.Request(_docking,_activeStationProfile.id?_activeStationProfile.id:static_cast<std::uint64_t>(std::hash<std::string>{}(_activeStationProfile.name)),station,player->position,_activeStationProfile.dockable);
        }
    }
}

void NativeGameApplication::RefreshPlayerFacingModels()
{
    _flightHud=_playerFacing.BuildFlightHud(_strategicFlight.Mode(),{"PRIMARY","MINING","SCAN","DRONES","REPAIR","UTILITY"});
    const float dt=std::max(0.001f,_engine.GetLastDeltaTime());
    TacticalTargetingSystem::Tick(_targeting,dt);
    _tacticalContacts.rows.clear();
    auto* playerPhysics=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);
    auto pushContact=[&](const NativeContactSelection& ref,const std::string& name,const std::string& type,const Vector3& world,float velocity,int threat){
        if(!playerPhysics)return;
        TacticalContactRow row;row.id=static_cast<std::uint64_t>(std::hash<std::string>{}(ref.id));row.kind=ToTacticalContactKind(ref,_sector);row.name=name;row.type=type;row.range=(world-playerPhysics->position).length();row.velocity=velocity;row.threat=threat;row.selected=SameContact(ref,_selection);row.locked=TacticalTargetingSystem::IsLocked(_targeting,ToTargetReference(ref));row.sourceKind=static_cast<int>(ref.kind);row.sourceIndex=ref.index;row.sourceId=ref.id;
        if(TacticalContactsSystem::VisibleInPreset(row.kind,_tacticalContacts.preset))_tacticalContacts.rows.push_back(row);
    };
    for(std::size_t i=0;i<_sector.ships.size();++i){const auto& ship=_sector.ships[i];if(ship.claimed)continue;const NativeContactSelection ref{NativeContactKind::Ship,i,"ship_"+std::to_string(i)};pushContact(ref,ship.shipType+" "+std::to_string(i+1),ship.shipType,NativeBattlefieldRenderer::SectorToWorld(ship.position),0.0f,ship.hostile?3:0);}
    for(std::size_t i=0;i<_sector.planets.size();++i){const auto& planet=_sector.planets[i];const NativeContactSelection ref{NativeContactKind::Planet,i,planet.planetId};pushContact(ref,planet.name,"PLANET",NativeBattlefieldRenderer::SectorToWorld(planet.position),0.0f,0);}
    if(_sector.hasStation){const NativeContactSelection ref{NativeContactKind::Station,0,_sector.station.name};pushContact(ref,_sector.station.name,"STATION",NativeBattlefieldRenderer::SectorToWorld(_sector.station.position),0.0f,0);}
    if(_stationContacts.size())for(std::size_t i=0;i<_stationContacts.size();++i){const auto& st=_stationContacts[i];const NativeContactSelection ref{NativeContactKind::Station,i+1,st.name};pushContact(ref,st.name,"STATION",st.position,0.0f,0);}
    for(std::size_t i=0;i<_sector.derelicts.size();++i){const auto& d=_sector.derelicts[i];const NativeContactSelection ref{NativeContactKind::Derelict,i,d.derelictId};pushContact(ref,d.derelictId,"WRECK",NativeBattlefieldRenderer::SectorToWorld(d.position),0.0f,0);}
    for(std::size_t i=0;i<_sector.pointsOfInterest.size();++i){const auto& site=_sector.pointsOfInterest[i];const NativeContactSelection ref{NativeContactKind::Site,i,site.siteId};pushContact(ref,site.siteId,"SITE",NativeBattlefieldRenderer::SectorToWorld(site.position),0.0f,0);}
    TacticalContactsSystem::Sort(_tacticalContacts);
    if(!_galaxyRuntime.initialized)_galaxyRuntime=_playerFacing.BuildGalaxyRuntime(1u,10000);
    if(!_orbitalBodies.empty()){
        const float keepZoom=_systemMapRuntime.zoom;const Vector3 keepPan=_systemMapRuntime.pan;
        _systemMapRuntime=_playerFacing.BuildSystemMapRuntime(_orbitalBodies,_orbitalSimulationSeconds,72);
        _systemMapRuntime.zoom=keepZoom<=0.0f?1.0f:keepZoom;_systemMapRuntime.pan=keepPan;
    }
    _hangarRuntime=_playerFacing.BuildHangar(_activeStationProfile,_docking.stage==DockingExperienceStage::Docked,18.0f);
    if(_workspace.Mode()==SandboxWorkspaceMode::PlanetaryManufacturing){
        const auto index=ActivePlanetIndex();
        if(index<_sector.planets.size() && index!=_activePiPlanet){
            _activePiRuntime=_playerFacing.BuildPlanetIndustry(_sector.planets[index],3,static_cast<std::uint32_t>(index*97+31));
            _activePiPlanet=index;
        }
    }
    FleetFlightConfig cfg;std::vector<FleetWingShip> wing={{1,FleetShipRole::Combat,true},{2,FleetShipRole::Mining,true},{3,FleetShipRole::Support,true},{4,FleetShipRole::Salvage,true}};
    _fleetRuntime=_playerFacing.BuildFleet(wing,cfg,_strategicFlight.CurrentOrder().valid?_strategicFlight.CurrentOrder().kind:StrategicOrderKind::Follow);
}


void NativeGameApplication::UpdateFleetCaptains()
{
    auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);
    if(!player)return;
    const bool targetValid=_selection.IsValid();
    const Vector3 target=targetValid?ContactWorldPosition(_sector,_selection,&_stationContacts):player->position;
    _fleetCaptainAi.Step(_fleetCaptainRuntime,_fleetRuntime,player->position,player->velocity,player->rotation.z,target,targetValid,
                         _vectorTravelSystem.InTransit(_vectorTravel),_docking.stage==DockingExperienceStage::Docked,
                         std::max(0.001f,_engine.GetLastDeltaTime()));
}

void NativeGameApplication::UpdateEmbodiment()
{
    if(!_embodiment.IsOnFoot())return;
    auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);
    if(!player)return;

    // Pass519: on-foot uses the same platform-independent movement actions but
    // with the ship controller detached. The ship is held inert while the
    // avatar walks its local deck, so WASD can never thrust the vessel through
    // the station/world while the player is physically inside it.
    if(auto* controls=_engine.GetPlayerControlSystem())controls->ClearControlledShip();
    player->velocity={};player->angularVelocity={};
    const auto& input=_engine.GetInputState();
    const float forward=(input.IsDown(InputAction::ThrustForward)?1.0f:0.0f)-(input.IsDown(InputAction::ThrustReverse)?1.0f:0.0f);
    const float strafe=(input.IsDown(InputAction::StrafeRight)?1.0f:0.0f)-(input.IsDown(InputAction::StrafeLeft)?1.0f:0.0f);
    _embodiment.Move(forward,strafe,std::max(0.001f,_engine.GetLastDeltaTime()));
}

void NativeGameApplication::UpdateDocking()
{
    if(_docking.stage==DockingExperienceStage::Undocked||_docking.stage==DockingExperienceStage::Docked)return;
    auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);if(!player)return;
    player->position=_dockingSystem.Update(_docking,player->position,std::max(0.001f,_engine.GetLastDeltaTime()));
    if(_docking.stage==DockingExperienceStage::Approach||_docking.stage==DockingExperienceStage::Capture){
        const Vector3 desired=(_docking.geometry.apertureWorld-player->position).normalized();
        if(desired.length()>.001f){const float targetYaw=std::atan2(-desired.x,desired.y);player->rotation.z+=ShortestAngleRadians(player->rotation.z,targetYaw)*.12f;}
        if(_docking.autoDock)player->velocity={};
    }
    if(_docking.stage==DockingExperienceStage::Docked){
        player->velocity={};player->angularVelocity={};
        if(auto* controls=_engine.GetPlayerControlSystem())controls->ClearControlledShip();
        _embodiment.EnterDockedHangar(_playerEntity);
        _workspace.Open(SandboxWorkspaceMode::HangarFitting);
    }else if(_docking.stage==DockingExperienceStage::Undocked){
        if(auto* controls=_engine.GetPlayerControlSystem())controls->SetControlledShip(_playerEntity);
        _embodiment=ShipEmbodimentSystem{};
    }
}

std::vector<std::string> NativeGameApplication::BuildWorkspaceLines() const
{
    std::vector<std::string> lines;
    std::ostringstream text;
    const auto mode=_workspace.Mode();
    if(mode==SandboxWorkspaceMode::GalaxyMap){
        lines.push_back("Catalog systems: "+std::to_string(_galaxyRuntime.catalog.size()));
        lines.push_back("Selected system: "+std::to_string(_galaxyRuntime.selectedSystem));
        if(_galaxyRuntime.route.valid){
            text<<"Route hops "<<_galaxyRuntime.route.systems.size()<<"   distance "<<std::fixed<<std::setprecision(0)<<_galaxyRuntime.route.distance<<"   fuel "<<std::setprecision(1)<<_galaxyRuntime.route.fuel;
            lines.push_back(text.str());
        } else lines.push_back("Route: none");
        lines.push_back("UP/DOWN select   ENTER plot route   RMB orbit   WHEEL zoom   M close");
        lines.push_back("N opens current solar-system orbital map");
        return lines;
    }
    if(mode==SandboxWorkspaceMode::SystemMap){
        const auto* selected=_systemMapSystem.Selected(_systemMap);
        lines.push_back("Known destinations: "+std::to_string(_systemMapSystem.WarpableKnown(_systemMap).size()));
        lines.push_back("Orbital bodies: "+std::to_string(_universeSystemMap.nodes.size())+"   live ephemeris");
        lines.push_back(std::string("Selected: ")+(selected?selected->label:"none"));
        text<<"System time "<<std::fixed<<std::setprecision(1)<<_orbitalSimulationSeconds<<"s   Vector fuel "<<std::setprecision(0)<<_vectorFuel;lines.push_back(text.str());
        lines.push_back("UP/DOWN select   ENTER vector travel   WHEEL OUT -> GALAXY   M close");
        return lines;
    }
    const auto planetIndex=ActivePlanetIndex();
    if((mode==SandboxWorkspaceMode::PlanetSurvey||mode==SandboxWorkspaceMode::PlanetaryManufacturing)&&planetIndex<_sector.planets.size()){
        const auto& planet=_sector.planets[planetIndex];lines.push_back("Target: "+planet.name);
        auto it=_planetSurveys.find(planetIndex);
        if(it==_planetSurveys.end()){lines.push_back("Survey: not initialized");return lines;}
        const auto& survey=it->second;lines.push_back("Survey stage: "+std::to_string(static_cast<int>(survey.stage)));
        text.str("");text.clear();text<<"Gravity "<<std::fixed<<std::setprecision(2)<<survey.gravityG<<"g   Radiation "<<std::setprecision(0)<<survey.radiation*100<<"%";lines.push_back(text.str());
        PlanetSurveySystem surveySystem;text.str("");text.clear();text<<"Industrial score "<<std::fixed<<std::setprecision(2)<<surveySystem.IndustrialValue(survey);lines.push_back(text.str());
        if(mode==SandboxWorkspaceMode::PlanetSurvey)lines.push_back("ENTER advances orbital survey; no landing path exists");
        else {auto pit=_planetProjects.find(planetIndex);lines.push_back(pit==_planetProjects.end()?"Planetary Manufacturing locked: certify survey then deploy tether":"Tether / elevator project registered");}
        return lines;
    }
    if(mode==SandboxWorkspaceMode::FleetCorporation){
        lines.push_back("FORMATION: "+FormationPattern::GetFormationName(_fleetRuntime.formation)+"   spacing "+std::to_string(static_cast<int>(_fleetRuntime.spacing)));
        lines.push_back("Wing captains fly their ships; formation slots are AI targets, not fixed render positions.");
        for(const auto& ship:_fleetCaptainRuntime.ships){
            std::string role="COMBAT";if(ship.role==FleetShipRole::Mining)role="MINING";else if(ship.role==FleetShipRole::Salvage)role="SALVAGE";else if(ship.role==FleetShipRole::Support)role="SUPPORT";
            std::ostringstream row;row<<ship.captain.name<<"  "<<role<<"  "<<FleetCaptainAiSystem::StateName(ship.state)<<"  NAV "<<static_cast<int>(ship.captain.navigation*100)<<"%";lines.push_back(row.str());
        }
        lines.push_back("Leader actions mirror by role: MOVE / VECTOR / COMBAT / MINE / SALVAGE / DOCK");
        return lines;
    }
    if(mode==SandboxWorkspaceMode::HangarFitting){
        lines.push_back("ACTUAL GENERATED SHIP VIEWPORT - free orbit / pan / hull-close zoom");
        lines.push_back("Turret sockets are structural hardpoints on the generated hull, not floating UI slots.");
        lines.push_back("Fitted module rack: PRIMARY / MINING / SCAN / DRONES / REPAIR / UTILITY");
        lines.push_back("Power, heat, mass and hardpoint-size constraints remain authoritative.");
        lines.push_back("RMB orbit   MMB pan   wheel zoom   F6 QA attachment overlays");
        return lines;
    }
    if(mode==SandboxWorkspaceMode::StationBuilder){lines.push_back("MODULE CATALOG / SOCKET GRAPH / POWER / STORAGE / SERVICES / DEFENSE");lines.push_back("Modules connect through validated station sockets; invalid floating modules cannot commit.");}
    if(mode==SandboxWorkspaceMode::MarketContracts){lines.push_back("MARKET ORDERS / CONTRACTS / PRICE HISTORY / SHORTAGES / LOGISTICS");}
    if(mode==SandboxWorkspaceMode::Exploration){lines.push_back("SCAN / PROBES / SIGNATURES / BOOKMARKS / SUBSPACE ANOMALIES");}
    if(mode==SandboxWorkspaceMode::ShipBuilder){
        lines.push_back("DOCKED LIVE-SHIP REFIT / OWNED + CRAFTED + LOOTED + STATION PARTS / SOCKET VALIDATION");
        if(_shipBuilder.IsInitialized())lines.push_back(_shipBuilder.Model().status);
    }
    for(const auto& action:_workspace.Actions())lines.push_back(action);
    return lines;
}

void NativeGameApplication::HandleGlobalActions()
{
    auto& input = _engine.GetInputState();
    const bool inGame = _frontend.Screen()==FrontendScreen::InGame;
    if(!inGame)return; // frontend must be the sole pointer-input owner outside gameplay

    if(_standaloneShipyard){
        if(input.WasPressed(InputAction::MenuBack)){
            _workspace.Close();_standaloneShipyard=false;RestoreGameplayCameraLimits();GoToFrontendScreen(FrontendScreen::MainMenu);return;
        }
        if(input.WasPressed(InputAction::MenuAccept))_shipBuilder.Activate(ShipyardBuilderCommand::AddModule);
        const float wheel=_window.ConsumeWheelDelta();
        if(std::fabs(wheel)>0.0001f){
            const bool wheelOwnedByUi=_shipBuilder.IsInitialized() && _shipBuilder.HandleWheel(
                _window.GetPointerX(),_window.GetPointerY(),wheel,_window.GetWidth(),_window.GetHeight());
            if(!wheelOwnedByUi){
                if(_window.IsAltDown())ConstructionEditorCameraSystem::AdjustSpeed(_constructionCamera,wheel);
                else ConstructionEditorCameraSystem::Dolly(_constructionCamera,wheel*(_window.IsShiftDown()?.25f:1.0f));
                ApplyConstructionCameraView();
            }
        }
        float orbitX=0.0f,orbitY=0.0f;
        if(_window.ConsumeCameraOrbitDelta(orbitX,orbitY)){
            const float precision=_window.IsShiftDown()?.10f:1.0f;
            if(_window.IsAltDown())ConstructionEditorCameraSystem::Look(_constructionCamera,orbitX*.30f*precision,-orbitY*.28f*precision);
            else ConstructionEditorCameraSystem::Orbit(_constructionCamera,orbitX*.30f*precision,-orbitY*.28f*precision);
            ApplyConstructionCameraView();
        }
        float panX=0.0f,panY=0.0f;
        if(_window.ConsumeCameraPanDelta(panX,panY)){
            const float precision=_window.IsShiftDown()?.10f:1.0f;
            const float scale=std::max(.0015f,_constructionCamera.orbitDistance*.0025f);
            ConstructionEditorCameraSystem::TruckPedestal(_constructionCamera,panX*scale*precision,-panY*scale*precision);
            ApplyConstructionCameraView();
        }
        UpdateConstructionCameraKeyboard();
        HandleShipyardTransformHotkeys();
        // Standalone and in-game Shipyards share the same direct-manipulation
        // contract. Main-menu blueprint authoring must not degrade to buttons
        // merely because the normal gameplay camera/update path is bypassed.
        float pressX=0.0f,pressY=0.0f;
        if(_window.ConsumePrimaryPress(pressX,pressY)){
            const auto control=ShipyardBuilderSystem::HitTest(_shipBuilder.Model(),_window.GetWidth(),_window.GetHeight(),pressX,pressY);
            if(control.command==ShipyardBuilderCommand::SelectModule){
                // Click selects/previews a catalog part. A real drag begins only
                // after the pointer crosses the threshold below; zero-distance
                // click/release must never add a module to the ship.
                _shipBuilder.Activate(ShipyardBuilderCommand::SelectModule,control.value);
                _shipyardCatalogPointerCandidate=true;
                _shipyardCatalogPointerCandidateIndex=control.value;
                _shipyardCatalogPressX=pressX;_shipyardCatalogPressY=pressY;
            }
            else if(control.command==ShipyardBuilderCommand::None){
                const int picked=NativeBattlefieldRenderer::PickShipyardModule(_shipBuilder.Model().catalog,_shipBuilder.Recipe(),_engine.GetStrategicCamera(),_window.GetWidth(),_window.GetHeight(),pressX,pressY,0,0,0,.24f,.22f,true);
                if(picked>=0){_shipBuilder.Activate(ShipyardBuilderCommand::SelectPlaced,picked);if(_shipBuilder.Model().transformTool!=ShipyardTransformTool::Select)_shipyardPointerTransform=_shipBuilder.Model().inspectorTab==ShipyardInspectorTab::Sockets?_shipBuilder.BeginSelectedSocketTransform():_shipBuilder.BeginSelectedTransform();}
            }
        }
        float dragX=0.0f,dragY=0.0f;
        if(_window.ConsumePrimaryDragDelta(dragX,dragY)){
            if(_shipyardCatalogPointerCandidate&&!_shipyardCatalogPointerDrag){
                const float ddx=_window.GetPointerX()-_shipyardCatalogPressX,ddy=_window.GetPointerY()-_shipyardCatalogPressY;
                if(ddx*ddx+ddy*ddy>=64.0f){
                    _shipyardCatalogPointerDrag=_shipBuilder.BeginCatalogDrag(_shipyardCatalogPointerCandidateIndex);
                    _shipyardCatalogPointerCandidate=false;
                }
            }
            if(_shipyardCatalogPointerDrag){
                const auto world=NativeBattlefieldRenderer::ScreenToWorld(_window.GetPointerX(),_window.GetPointerY(),_window.GetWidth(),_window.GetHeight(),_engine.GetStrategicCamera());
                _shipBuilder.UpdateCatalogDrag(world);
            }else if(_shipyardPointerTransform){
                const bool fine=_window.IsShiftDown();auto& camera=_engine.GetStrategicCamera();
                const bool socketEdit=_shipBuilder.Model().inspectorTab==ShipyardInspectorTab::Sockets;
                if(_shipBuilder.Model().transformTool==ShipyardTransformTool::Move){const float scale=.012f/std::max(.35f,camera.GetZoom());Vector3 delta=camera.ViewRightPlanar()*(dragX*scale)+camera.ViewUpPlanar()*(-dragY*scale);if(socketEdit)if(const auto* placed=_shipBuilder.SelectedPlacedModule())delta=ShipDeltaToModuleLocal(delta,*placed);if(socketEdit)_shipBuilder.TranslateSelectedSocket(delta,fine);else _shipBuilder.TranslateSelected(delta,fine);}
                else if(_shipBuilder.Model().transformTool==ShipyardTransformTool::Rotate){
                    const Vector3 delta=_window.IsControlDown()?Vector3{0.0f,0.0f,dragX*.35f}:Vector3{-dragY*.35f,dragX*.35f,0.0f};
                    if(socketEdit)_shipBuilder.RotateSelectedSocket(delta,fine);else _shipBuilder.RotateSelected(delta,fine);
                }
                else if(!socketEdit&&_shipBuilder.Model().transformTool==ShipyardTransformTool::Scale){const float amount=(dragX-dragY)*.0030f;_shipBuilder.ScaleSelected({amount,amount,amount},fine);}
            }
        }
        float releaseX=0.0f,releaseY=0.0f;
        if(_window.ConsumePrimaryRelease(releaseX,releaseY)){
            if(_shipyardCatalogPointerDrag){_shipBuilder.CommitCatalogDrag();_shipyardCatalogPointerDrag=false;}
            _shipyardCatalogPointerCandidate=false;_shipyardCatalogPointerCandidateIndex=-1;
            if(_shipyardPointerTransform){if(_shipBuilder.Model().inspectorTab==ShipyardInspectorTab::Sockets)_shipBuilder.CommitSocketTransform();else _shipBuilder.CommitTransform();_shipyardPointerTransform=false;}
        }
        float sx=0.0f,sy=0.0f;if(_window.ConsumePrimaryClick(sx,sy)){const auto c=ShipyardBuilderSystem::HitTest(_shipBuilder.Model(),_window.GetWidth(),_window.GetHeight(),sx,sy);if(c.command!=ShipyardBuilderCommand::None)ActivateShipyardControl(c.command,c.value);else {const int picked=NativeBattlefieldRenderer::PickShipyardModule(_shipBuilder.Model().catalog,_shipBuilder.Recipe(),_engine.GetStrategicCamera(),_window.GetWidth(),_window.GetHeight(),sx,sy,0,0,0,.24f,.22f,true);if(picked>=0)_shipBuilder.Activate(ShipyardBuilderCommand::SelectPlaced,picked);}}
        ProcessShipyardRequests();return;
    }

    bool closedWorkspace = false;
    if (inGame && _workspace.IsOverlayOpen() && input.WasPressed(InputAction::MenuBack)) {
        const bool closingShipyard=_workspace.Mode()==SandboxWorkspaceMode::ShipBuilder;
        _workspace.Close();
        if(closingShipyard)RestoreGameplayCameraLimits();
        closedWorkspace = true;
    }

    if (inGame) {
        bool contextCommandConsumed = false;
        // Pass402: RMB short-click is a universal context request in flight.
        // RMB drag remains inspection orbit; normal flight no longer consumes
        // a right drag as an implicit camera command.
        float contextX=0.0f,contextY=0.0f;
        if(_workspace.Mode()==SandboxWorkspaceMode::SystemMap && _window.ConsumeSecondaryClick(contextX,contextY)) {
            const int hit=_playerFacing.HitTestSystemMapNode(_systemMap,&_universeSystemMap,
                _window.GetWidth(),_window.GetHeight(),_systemMapRuntime.zoom,_systemMapRuntime.pan,contextX,contextY);
            if(hit>=0){
                _systemMap.selected=static_cast<std::size_t>(hit);
                const auto& node=_systemMap.nodes[_systemMap.selected];
                InteractionContext mapContext;mapContext.kind=ContextObjectKind::MapDestination;mapContext.discovered=node.known;
                _contextMenu=_playerFacing.OpenContext(mapContext,contextX,contextY);
                _contextMenu.targetId=node.id;_contextMenu.systemMapTarget=true;
                _contextMenu.worldTarget={node.position.x,node.position.y,node.position.z};
            }else _contextMenu.open=false;
        } else if(!_workspace.IsOverlayOpen() && _window.ConsumeSecondaryClick(contextX,contextY)) {
            // Contacts is a first-class tactical control surface: RMB on a row
            // selects that exact underlying object and opens the same contextual
            // command menu used by right-clicking the object in space.
            const int contactRow=TacticalContactsSystem::HitTestRow(_tacticalContacts,_window.GetWidth(),_window.GetHeight(),contextX,contextY);
            const int visibleContacts=std::min<int>(_tacticalContacts.maxVisibleRows,static_cast<int>(_tacticalContacts.rows.size()));
            const auto contactLayout=TacticalContactsSystem::Layout(_window.GetWidth(),_window.GetHeight(),visibleContacts);
            NativeContactSelection picked;
            Vector3 world=NativeBattlefieldRenderer::ScreenToWorld(contextX,contextY,_window.GetWidth(),_window.GetHeight(),_engine.GetStrategicCamera());
            if(contactRow>=0){picked=ToNativeContactSelection(_tacticalContacts.rows[static_cast<std::size_t>(contactRow)]);_selection=picked;}
            else if(!contactLayout.Contains(contextX,contextY,visibleContacts)){picked=NativeBattlefieldRenderer::PickContact(_sector,world,4.0f,&_stationContacts);if(picked.IsValid())_selection=picked;}
            if(picked.IsValid()){
                _contextMenu=_playerFacing.OpenContext(BuildInteractionContext(picked),contextX,contextY);
                _contextMenu.worldTarget=ContactWorldPosition(_sector,picked,&_stationContacts);
            }else _contextMenu.open=false;
        }
        if(_contextMenu.open) {
            const auto executeContext=[&](std::size_t index){
                if(index>=_contextMenu.actions.size())return;
                _contextMenu.selected=index;
                contextCommandConsumed=true;
                if(const auto* action=_playerFacing.ActiveContextAction(_contextMenu)) {
                    if(!action->enabled)return;
                    if(_contextMenu.systemMapTarget){
                        if(action->id=="vector"&&!_vectorTravelSystem.InTransit(_vectorTravel)){
                            auto plan=_systemNavigation.PlanWarp(_currentAstronomicalPosition,_contextMenu.targetId,CurrentVectorTopSpeedMetersPerSecond(),_vectorFuel);
                            if(plan.valid&&_vectorTravelSystem.Begin(_vectorTravel,plan)){
                                _activeWarpDestination=_contextMenu.targetId;_activeArrival={};_workspace.Close();
                            }
                        }
                        _contextMenu.open=false;return;
                    }
                    if(action->id=="takeover" && _selection.kind==NativeContactKind::Ship && _selection.index<_sector.ships.size()){
                        auto& captured=_sector.ships[_selection.index];
                        if(captured.capturable&&captured.disabled&&!captured.claimed){
                            if(const auto* def=ShipyardAuthoredShipSystem::Find(captured.authoredShipId)){_playerShipRecipe=ShipyardAuthoredShipSystem::BuildRecipe(*def);_playerShipAppearance=ShipAppearanceState{};_hasPlayerShipRecipe=true;captured.claimed=true;captured.hostile=false;auto& interiors=_engine.GetRuntimeServices().interiors;interiors.ClearLayout(_playerEntity);ShipInteriorLayoutSystem layout;layout.Materialize(_playerEntity,_shipBuilder.Model().catalog,_playerShipRecipe,interiors);}
                        }
                        _contextMenu.open=false;return;
                    }
                    const std::uint64_t targetId=_selection.id.empty()?0ull:static_cast<std::uint64_t>(std::hash<std::string>{}(_selection.id));
                    auto order=_playerFacing.ToStrategicOrder(*action,targetId,_contextMenu.worldTarget,24.0f);
                    if(order.valid){_strategicFlight.SetMode(FlightControlMode::Strategic);_strategicFlight.Issue(order);}
                    if(action->id=="survey")_workspace.Open(SandboxWorkspaceMode::PlanetSurvey);
                    _contextMenu.open=false;
                }
            };

            // Pass410R3: the context surface is a real mouse menu. Hovering a
            // row updates selection and LMB activates that row; clicking away
            // dismisses it. Keyboard Up/Down/Enter remains an equivalent path.
            const int hovered=_playerFacing.HitTestContextMenu(_contextMenu,_window.GetWidth(),_window.GetHeight(),_window.GetPointerX(),_window.GetPointerY());
            if(hovered>=0)_contextMenu.selected=static_cast<std::size_t>(hovered);
            float menuClickX=0.0f,menuClickY=0.0f;
            if(_window.ConsumePrimaryClick(menuClickX,menuClickY)){
                const int clicked=_playerFacing.HitTestContextMenu(_contextMenu,_window.GetWidth(),_window.GetHeight(),menuClickX,menuClickY);
                if(clicked>=0)executeContext(static_cast<std::size_t>(clicked));
                else _contextMenu.open=false;
            }
            if(_contextMenu.open&&input.WasPressed(InputAction::MenuNext)) _playerFacing.MoveContextSelection(_contextMenu,+1);
            if(_contextMenu.open&&input.WasPressed(InputAction::MenuPrevious)) _playerFacing.MoveContextSelection(_contextMenu,-1);
            if(_contextMenu.open&&input.WasPressed(InputAction::MenuBack)) _contextMenu.open=false;
            if(_contextMenu.open&&input.WasPressed(InputAction::MenuAccept)) executeContext(_contextMenu.selected);
        }

        // Pass383 runtime refinement: Tab is the explicit Manual/Strategic
        // flight-mode switch. Boost moved to Shift at the native window layer.
        // Strategic mode currently preserves direct keys as an immediate
        // override while the contextual/autopilot order layer remains active.
        if(input.WasPressed(InputAction::ToggleFlightMode) && _docking.stage==DockingExperienceStage::Undocked && !_vectorTravelSystem.InTransit(_vectorTravel)){
            _strategicFlight.ToggleMode();
            auto& camera=_engine.GetStrategicCamera();
            if(_strategicFlight.Mode()==FlightControlMode::Strategic){camera.SetTargetZoom(.43f);camera.SetVisualTilt(.50f);}
            else {camera.SetTargetZoom(.56f);camera.SetVisualTilt(.46f);}
        }

        const auto toggle=[&](InputAction action,SandboxWorkspaceMode mode){
            if(input.WasPressed(action))_workspace.Toggle(mode);
        };
        toggle(InputAction::OpenGalaxyMap,SandboxWorkspaceMode::GalaxyMap);
        toggle(InputAction::OpenSystemMap,SandboxWorkspaceMode::SystemMap);
        toggle(InputAction::OpenPlanetSurvey,SandboxWorkspaceMode::PlanetSurvey);
        toggle(InputAction::OpenPlanetaryManufacturing,SandboxWorkspaceMode::PlanetaryManufacturing);
        toggle(InputAction::OpenStationBuilder,SandboxWorkspaceMode::StationBuilder);
        // Shipyard is always a real blueprint-design workspace. Docking adds
        // transactional APPLY authority; undocked use remains save/design-only.
        if(input.WasPressed(InputAction::OpenShipBuilder)){
            if(_workspace.Mode()==SandboxWorkspaceMode::ShipBuilder){_workspace.Close();ShipyardRefitSystem::Rollback(_shipyardRefit);RestoreGameplayCameraLimits();}
            else OpenShipyardWorkspace(false);
        }
        toggle(InputAction::OpenHangarFitting,SandboxWorkspaceMode::HangarFitting);
        toggle(InputAction::OpenMarketContracts,SandboxWorkspaceMode::MarketContracts);
        toggle(InputAction::OpenExploration,SandboxWorkspaceMode::Exploration);
        toggle(InputAction::OpenFleetCorporation,SandboxWorkspaceMode::FleetCorporation);

        if (_workspace.Mode()==SandboxWorkspaceMode::PlanetSurvey ||
            _workspace.Mode()==SandboxWorkspaceMode::PlanetaryManufacturing) {
            const auto index=ActivePlanetIndex();
            if(index<_sector.planets.size())EnsurePlanetSurvey(index);
        }

        if (_workspace.Mode()==SandboxWorkspaceMode::GalaxyMap && _galaxyRuntime.initialized) {
            const std::uint32_t count=static_cast<std::uint32_t>(_galaxyRuntime.catalog.size());
            if(input.WasPressed(InputAction::MenuNext)) _playerFacing.SelectGalaxySystem(_galaxyRuntime,(_galaxyRuntime.selectedSystem%count)+1);
            if(input.WasPressed(InputAction::MenuPrevious)) _playerFacing.SelectGalaxySystem(_galaxyRuntime,_galaxyRuntime.selectedSystem<=1?count:_galaxyRuntime.selectedSystem-1);
            if(!contextCommandConsumed && input.WasPressed(InputAction::MenuAccept)){
                const std::uint32_t end=std::min<std::uint32_t>(count,_galaxyRuntime.selectedSystem+12);
                _playerFacing.PlotGalaxyRoute(_galaxyRuntime,_galaxyRuntime.selectedSystem,end,1500.0f,GalaxyRouteMode::Safest);
            }
        } else if (_workspace.Mode()==SandboxWorkspaceMode::SystemMap && !_systemMap.nodes.empty()) {
            auto stepSelection=[&](int direction){
                std::size_t index=_systemMap.selected<_systemMap.nodes.size()?_systemMap.selected:0;
                for(std::size_t tries=0;tries<_systemMap.nodes.size();++tries){
                    if(direction>0)index=(index+1)%_systemMap.nodes.size();
                    else index=(index+_systemMap.nodes.size()-1)%_systemMap.nodes.size();
                    const auto& node=_systemMap.nodes[index];
                    if(node.known&&node.warpable){_systemMap.selected=index;break;}
                }
            };
            if(input.WasPressed(InputAction::MenuNext))stepSelection(+1);
            if(input.WasPressed(InputAction::MenuPrevious))stepSelection(-1);
            if(!contextCommandConsumed && input.WasPressed(InputAction::MenuAccept) && !_vectorTravelSystem.InTransit(_vectorTravel)) {
                if(const auto* node=_systemMapSystem.Selected(_systemMap)){
                    auto plan=_systemNavigation.PlanWarp(_currentAstronomicalPosition,node->id,CurrentVectorTopSpeedMetersPerSecond(),_vectorFuel);
                    if(plan.valid&&_vectorTravelSystem.Begin(_vectorTravel,plan)){
                        _activeWarpDestination=node->id;
                        _activeArrival={};
                        _workspace.Close();
                    }
                }
            }
        } else if (_workspace.Mode()==SandboxWorkspaceMode::PlanetSurvey && !contextCommandConsumed && input.WasPressed(InputAction::MenuAccept)) {
            const auto index=ActivePlanetIndex();
            if(index<_sector.planets.size()){
                EnsurePlanetSurvey(index);
                auto& record=_planetSurveys[index];
                PlanetSurveySystem survey;
                if(record.stage==PlanetSurveyStage::Preliminary)survey.Advance(record,PlanetSurveyStage::Detailed);
                else if(record.stage==PlanetSurveyStage::Detailed)survey.Advance(record,PlanetSurveyStage::IndustrialCertified);
            }
        } else if (_workspace.Mode()==SandboxWorkspaceMode::PlanetaryManufacturing && !contextCommandConsumed && input.WasPressed(InputAction::MenuAccept)) {
            const auto index=ActivePlanetIndex();
            if(index<_sector.planets.size()){
                EnsurePlanetSurvey(index);
                const auto& record=_planetSurveys[index];
                if(record.stage>=PlanetSurveyStage::Detailed){
                    PlanetaryIndustrializationSystem industrial;
                    auto& project=_planetProjects[index];
                    if(project.stage==PlanetaryDevelopmentStage::Unsurveyed)industrial.BindSurvey(project,record);
                    else if(project.stage==PlanetaryDevelopmentStage::Surveyed)industrial.SelectAnchor(project,record.elevatorAnchorScore);
                    else if(project.stage==PlanetaryDevelopmentStage::SiteSelected)industrial.DeployTether(project);
                    else if(project.stage==PlanetaryDevelopmentStage::ElevatorOnline)industrial.UnlockManufacturing(project);
                }
            }
        }

        if(_workspace.Mode()==SandboxWorkspaceMode::ShipBuilder&&!contextCommandConsumed){
            HandleShipyardTransformHotkeys();
            if(input.WasPressed(InputAction::MenuAccept))_shipBuilder.Activate(ShipyardBuilderCommand::AddModule);
            ProcessShipyardRequests();
        }

        // Pass301/311 docking context: Enter requests an assigned berth when
        // the selected station is within local approach range. Routine station
        // travel still uses normal engines after Vector arrival.
        if(!contextCommandConsumed && (input.WasPressed(InputAction::MenuAccept)||input.WasPressed(InputAction::RequestDock)) && (_workspace.Mode()==SandboxWorkspaceMode::Flight || (_workspace.Mode()==SandboxWorkspaceMode::HangarFitting && _docking.stage==DockingExperienceStage::Docked))) {
            if(_docking.stage==DockingExperienceStage::Docked) {
                if(_dockingSystem.RequestUndock(_docking)) {
                    _workspace.Close();
                    _embodiment=ShipEmbodimentSystem{};
                }
            } else if(_selection.kind==NativeContactKind::Station &&
                      _docking.stage==DockingExperienceStage::Undocked) {
                if(auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity)) {
                    const Vector3 station=ContactWorldPosition(_sector,_selection,&_stationContacts);
                    if(Distance2D(player->position,station)<=180.0f){
                        _activeStationProfile=StationProfileForSelection(_primaryStationProfile,_stationContacts,_selection);
                        _dockingSystem.Request(_docking,_activeStationProfile.id?_activeStationProfile.id:static_cast<std::uint64_t>(std::hash<std::string>{}(_activeStationProfile.name)),station,player->position,_activeStationProfile.dockable);
                    }
                }
            }
        }
    }

    // Pass518: cockpit/interior transition is explicit. Camera zoom and F6 QA
    // never enter the ship. I leaves/returns to the cockpit; while docked it
    // boards/leaves the ship interior from the hangar instead.
    if(input.WasPressed(InputAction::ToggleInterior) && !_vectorTravelSystem.InTransit(_vectorTravel) && !_workspace.IsOverlayOpen()){
        if(_docking.stage==DockingExperienceStage::Docked){
            if(_embodiment.IsOnFoot()){
                _embodiment.EnterDockedHangar(_playerEntity);
                _workspace.Open(SandboxWorkspaceMode::HangarFitting);
            }else if(_embodiment.Mode()==ShipEmbodimentMode::DockedHangar){
                if(_embodiment.BoardInterior(_playerEntity))_workspace.Close();
            }
        }else if(_docking.stage==DockingExperienceStage::Undocked){
            if(_embodiment.IsPiloting()){
                if(_embodiment.ExitCockpit(_playerEntity)){
                    if(auto* controls=_engine.GetPlayerControlSystem())controls->ClearControlledShip();
                    if(auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity)){player->velocity={};player->angularVelocity={};}
                    auto& camera=_engine.GetStrategicCamera();camera.ClearPanOffset();camera.SetTargetZoom(1.12f);camera.SetVisualTilt(.72f);camera.SetVisualYawDegrees(0.0f);
                }
            }else if(_embodiment.IsOnFoot()&&_embodiment.TakeControls()){
                if(auto* controls=_engine.GetPlayerControlSystem())controls->SetControlledShip(_playerEntity);
                RestoreGameplayCameraLimits();
                auto& camera=_engine.GetStrategicCamera();camera.SetTargetZoom(.56f);camera.SetVisualTilt(.46f);
            }
        }
    }

    if(input.IsDown(InputAction::ToggleShipInspection) && !_inspectionKeyConsumed && !_vectorTravelSystem.InTransit(_vectorTravel)) {
        // Pass416: F6 now toggles QA overlays only. Camera behavior is no longer
        // split between tactical and inspection modes; normal flight already has
        // the same unrestricted orbit/pan/close-zoom camera.
        _shipInspection=!_shipInspection;
        _inspectionKeyConsumed=true;
    } else if(!input.IsDown(InputAction::ToggleShipInspection)) _inspectionKeyConsumed=false;

    if (!closedWorkspace && input.IsDown(InputAction::Pause) && !_pauseKeyConsumed) {
        if (_engine.IsPaused()) _engine.Resume();
        else if (_engine.IsRunning()) _engine.Pause();
        _pauseKeyConsumed = true;
    } else if (!input.IsDown(InputAction::Pause)) {
        _pauseKeyConsumed = false;
    }

    // Ship-only production direction: interior/cutaway hotkeys are intentionally
    // not exposed. Engineering, crew and cargo remain GUI/data systems.

    auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);
    if (player && _embodiment.IsPiloting() && _docking.stage==DockingExperienceStage::Undocked && !_workspace.IsOverlayOpen() && !_vectorTravelSystem.InTransit(_vectorTravel)) {
        const float yaw=player->rotation.z;
        const Vector3 forward{-std::sin(yaw),std::cos(yaw),0.0f};
        Vector3 target=player->position+forward*24.0f;
        if (_selection.IsValid()) target=ContactWorldPosition(_sector,_selection,&_stationContacts);

        if (input.IsDown(InputAction::FireMiningMissile) && !_miningFireConsumed) {
            if (_selection.kind==NativeContactKind::Site && _selection.index<_sector.pointsOfInterest.size()) {
                const Vector3 site=ContactWorldPosition(_sector,_selection,&_stationContacts);
                float best=std::numeric_limits<float>::max();
                for(std::size_t i=0;i<_sector.asteroids.size();++i){
                    if(_miningLoop.GetFractureSystem().IsFractured(i)) continue;
                    const Vector3 candidate=NativeBattlefieldRenderer::SectorToWorld(_sector.asteroids[i].position);
                    const float d=Distance2D(candidate,site);
                    if(d<best){best=d;target=candidate;}
                }
            }
            _miningLoop.LaunchMiningMissile(player->position+forward*0.8f,player->velocity,forward,target);
            _miningFireConsumed=true;
        } else if (!input.IsDown(InputAction::FireMiningMissile)) {
            _miningFireConsumed=false;
        }

        if (input.IsDown(InputAction::FirePrimary) && !_primaryFireConsumed) {
            const bool locked=_selection.IsValid()&&TacticalTargetingSystem::IsLocked(_targeting,ToTargetReference(_selection));
            const Vector3 pointerWorld=NativeBattlefieldRenderer::ScreenToWorld(_window.GetPointerX(),_window.GetPointerY(),_window.GetWidth(),_window.GetHeight(),_engine.GetStrategicCamera());
            FireControlRequest req;req.mode=FireControlMode::LockedManualAim;req.targetLocked=locked;req.triggerHeld=true;req.muzzleWorld=player->position+forward*.8f;req.lockedTargetWorld=target;req.pointerWorld=pointerWorld;req.projectileSpeed=220.0f;
            const auto solution=FireControlSystem::Solve(WeaponType::SpinalRailgun,req);
            if(solution.mayFire)_miningLoop.GetMissileSystem().Launch(req.muzzleWorld,player->velocity,solution.aimDirection,solution.aimPoint,MissilePayloadType::HighExplosive,true);
            _primaryFireConsumed=true;
        } else if (!input.IsDown(InputAction::FirePrimary)) {
            _primaryFireConsumed=false;
        }
    } else {
        if(!input.IsDown(InputAction::FireMiningMissile))_miningFireConsumed=false;
        if(!input.IsDown(InputAction::FirePrimary))_primaryFireConsumed=false;
    }

    // Camera orbit remains available in every runtime workspace. The gameplay
    // plane never rotates with the camera; only presentation yaw/pitch does.
    const float wheel = _window.ConsumeWheelDelta();
    auto& activeCamera=_engine.GetStrategicCamera();
    bool wheelOwnedByUi=false;
    if(std::fabs(wheel)>0.0001f && _workspace.Mode()==SandboxWorkspaceMode::ShipBuilder && _shipBuilder.IsInitialized()){
        wheelOwnedByUi=_shipBuilder.HandleWheel(_window.GetPointerX(),_window.GetPointerY(),wheel,_window.GetWidth(),_window.GetHeight());
    }
    if (wheel != 0.0f && !wheelOwnedByUi) {
        if(_workspace.Mode()==SandboxWorkspaceMode::GalaxyMap)_playerFacing.ZoomGalaxy(_galaxyRuntime,wheel);
        else if(_workspace.Mode()==SandboxWorkspaceMode::SystemMap){
            _playerFacing.ZoomSystemMap(_systemMapRuntime,wheel);
            // Pass410R3 hierarchical navigation: continuing to zoom outward at
            // the solar-system floor promotes the same workspace into the 3D
            // galaxy view instead of trapping the wheel inside one system.
            if(_playerFacing.ShouldPromoteSystemMapToGalaxy(_systemMapRuntime,wheel)&&_galaxyRuntime.initialized){
                if(const auto it=std::find_if(_galaxyRuntime.catalog.begin(),_galaxyRuntime.catalog.end(),[&](const GalaxySystemRecord&r){return r.id==_galaxyRuntime.selectedSystem;});it!=_galaxyRuntime.catalog.end()){
                    _galaxyRuntime.camera.focusX=it->x;_galaxyRuntime.camera.focusY=it->y;_galaxyRuntime.camera.focusZ=it->z;
                }
                _galaxyRuntime.camera.distance=850.0f;
                _workspace.Open(SandboxWorkspaceMode::GalaxyMap);
            }
        }
        else if(_workspace.Mode()==SandboxWorkspaceMode::ShipBuilder){
            if(_window.IsAltDown())ConstructionEditorCameraSystem::AdjustSpeed(_constructionCamera,wheel);
            else ConstructionEditorCameraSystem::Dolly(_constructionCamera,wheel*(_window.IsShiftDown()?.25f:1.0f));
            ApplyConstructionCameraView();
        } else activeCamera.ZoomBy(wheel);
    }
    float orbitX=0.0f,orbitY=0.0f;
    if (_window.ConsumeCameraOrbitDelta(orbitX,orbitY)) {
        if(_workspace.Mode()==SandboxWorkspaceMode::GalaxyMap)_playerFacing.OrbitGalaxy(_galaxyRuntime,orbitX*0.18f,-orbitY*0.12f);
        else if(_workspace.Mode()==SandboxWorkspaceMode::ShipBuilder){
            const float precision=_window.IsShiftDown()?.10f:1.0f;
            if(_window.IsAltDown())ConstructionEditorCameraSystem::Look(_constructionCamera,orbitX*.30f*precision,-orbitY*.28f*precision);
            else ConstructionEditorCameraSystem::Orbit(_constructionCamera,orbitX*.30f*precision,-orbitY*.28f*precision);
            ApplyConstructionCameraView();
        } else {
            const float currentPitch=activeCamera.HasElevationOverride()?activeCamera.GetElevationOverrideDegrees():34.0f;
            activeCamera.SetVisualYawDegrees(activeCamera.GetVisualYawDegrees()+orbitX*0.30f);
            activeCamera.SetElevationOverrideDegrees(std::clamp(currentPitch-orbitY*0.28f,-89.0f,89.0f));
        }
    }
    float panX=0.0f,panY=0.0f;
    if (_window.ConsumeCameraPanDelta(panX,panY)) {
        if(_workspace.Mode()==SandboxWorkspaceMode::SystemMap)_playerFacing.PanSystemMap(_systemMapRuntime,panX*3.0f,-panY*3.0f);
        else if(_workspace.Mode()==SandboxWorkspaceMode::ShipBuilder){
            const float precision=_window.IsShiftDown()?.10f:1.0f;
            const float scale=std::max(.0015f,_constructionCamera.orbitDistance*.0025f);
            ConstructionEditorCameraSystem::TruckPedestal(_constructionCamera,panX*scale*precision,-panY*scale*precision);
            ApplyConstructionCameraView();
        } else if(_workspace.Mode()!=SandboxWorkspaceMode::GalaxyMap){
            const float panScale=0.022f/std::max(.35f,activeCamera.GetZoom());
            activeCamera.PanViewRelative(panX*panScale,panY*panScale);
        }
    }
    if(_workspace.Mode()==SandboxWorkspaceMode::ShipBuilder)UpdateConstructionCameraKeyboard();

    // Pass481-484: editor-grade LMB drag lifecycle. Catalog rows drag ghost modules
    // to compatible sockets; selected viewport modules translate/rotate through
    // one transaction authority. Camera motion remains RMB/MMB.
    if(_workspace.Mode()==SandboxWorkspaceMode::ShipBuilder&&_shipBuilder.IsInitialized()){
        float px=0.0f,py=0.0f;
        if(_window.ConsumePrimaryPress(px,py)){
            const auto control=ShipyardBuilderSystem::HitTest(_shipBuilder.Model(),_window.GetWidth(),_window.GetHeight(),px,py);
            if(control.command==ShipyardBuilderCommand::SelectModule){
                _shipBuilder.Activate(ShipyardBuilderCommand::SelectModule,control.value);
                _shipyardCatalogPointerCandidate=true;
                _shipyardCatalogPointerCandidateIndex=control.value;
                _shipyardCatalogPressX=px;_shipyardCatalogPressY=py;
            }else if(control.command==ShipyardBuilderCommand::None){
                float shipX=0,shipY=0,shipYaw=0;if(!_standaloneShipyard&&player){shipX=player->position.x;shipY=player->position.y;shipYaw=player->rotation.z;}
                const int picked=NativeBattlefieldRenderer::PickShipyardModule(_shipBuilder.Model().catalog,_shipBuilder.Recipe(),activeCamera,_window.GetWidth(),_window.GetHeight(),px,py,shipX,shipY,shipYaw,.24f,.22f,true);
                if(picked>=0){_shipBuilder.Activate(ShipyardBuilderCommand::SelectPlaced,picked);if(_shipBuilder.Model().transformTool!=ShipyardTransformTool::Select)_shipyardPointerTransform=_shipBuilder.Model().inspectorTab==ShipyardInspectorTab::Sockets?_shipBuilder.BeginSelectedSocketTransform():_shipBuilder.BeginSelectedTransform();}
            }
        }
        float dx=0,dy=0;if(_window.ConsumePrimaryDragDelta(dx,dy)){
            if(_shipyardCatalogPointerCandidate&&!_shipyardCatalogPointerDrag){
                const float ddx=_window.GetPointerX()-_shipyardCatalogPressX,ddy=_window.GetPointerY()-_shipyardCatalogPressY;
                if(ddx*ddx+ddy*ddy>=64.0f){_shipyardCatalogPointerDrag=_shipBuilder.BeginCatalogDrag(_shipyardCatalogPointerCandidateIndex);_shipyardCatalogPointerCandidate=false;}
            }
            if(_shipyardCatalogPointerDrag){
                const auto world=NativeBattlefieldRenderer::ScreenToWorld(_window.GetPointerX(),_window.GetPointerY(),_window.GetWidth(),_window.GetHeight(),activeCamera);float shipX=0,shipY=0,shipYaw=0;if(!_standaloneShipyard&&player){shipX=player->position.x;shipY=player->position.y;shipYaw=player->rotation.z;}const Vector3 local=WorldDeltaToShipLocal(world-Vector3{shipX,shipY,0},shipYaw);_shipBuilder.UpdateCatalogDrag(local);
            }else if(_shipyardPointerTransform){
                const bool fine=_window.IsShiftDown();
                const bool socketEdit=_shipBuilder.Model().inspectorTab==ShipyardInspectorTab::Sockets;
                if(_shipBuilder.Model().transformTool==ShipyardTransformTool::Move){const float scale=.012f/std::max(.35f,activeCamera.GetZoom());const Vector3 worldDelta=activeCamera.ViewRightPlanar()*(dx*scale)+activeCamera.ViewUpPlanar()*(-dy*scale);Vector3 delta=WorldDeltaToShipLocal(worldDelta,player?player->rotation.z:0.0f);if(socketEdit)if(const auto* placed=_shipBuilder.SelectedPlacedModule())delta=ShipDeltaToModuleLocal(delta,*placed);if(socketEdit)_shipBuilder.TranslateSelectedSocket(delta,fine);else _shipBuilder.TranslateSelected(delta,fine);}
                else if(_shipBuilder.Model().transformTool==ShipyardTransformTool::Rotate){
                    const Vector3 delta=_window.IsControlDown()?Vector3{0.0f,0.0f,dx*.35f}:Vector3{-dy*.35f,dx*.35f,0.0f};
                    if(socketEdit)_shipBuilder.RotateSelectedSocket(delta,fine);else _shipBuilder.RotateSelected(delta,fine);
                }
                else if(!socketEdit&&_shipBuilder.Model().transformTool==ShipyardTransformTool::Scale){const float amount=(dx-dy)*.0030f;_shipBuilder.ScaleSelected({amount,amount,amount},fine);}
            }
        }
        float rx=0,ry=0;if(_window.ConsumePrimaryRelease(rx,ry)){if(_shipyardCatalogPointerDrag){_shipBuilder.CommitCatalogDrag();_shipyardCatalogPointerDrag=false;}_shipyardCatalogPointerCandidate=false;_shipyardCatalogPointerCandidateIndex=-1;if(_shipyardPointerTransform){if(_shipBuilder.Model().inspectorTab==ShipyardInspectorTab::Sockets)_shipBuilder.CommitSocketTransform();else _shipBuilder.CommitTransform();_shipyardPointerTransform=false;}ProcessShipyardRequests();}
    }

    float sx=0.0f,sy=0.0f;
    if (_window.ConsumePrimaryClick(sx,sy)) {
        bool uiConsumed=false;
        if(_workspace.Mode()==SandboxWorkspaceMode::ShipBuilder&&_shipBuilder.IsInitialized()){
            const auto control=ShipyardBuilderSystem::HitTest(_shipBuilder.Model(),_window.GetWidth(),_window.GetHeight(),sx,sy);
            if(control.command!=ShipyardBuilderCommand::None){
                ActivateShipyardControl(control.command,control.value);uiConsumed=true;
                ProcessShipyardRequests();
            }
        }
        if(!uiConsumed&&_workspace.Mode()==SandboxWorkspaceMode::ShipBuilder&&_shipBuilder.IsInitialized()){
            float shipX=0.0f,shipY=0.0f,shipYaw=0.0f;
            if(!_standaloneShipyard&&player){
                shipX=player->position.x;shipY=player->position.y;shipYaw=player->rotation.z;
            }
            const int picked=NativeBattlefieldRenderer::PickShipyardModule(
                _shipBuilder.Model().catalog,_shipBuilder.Recipe(),activeCamera,
                _window.GetWidth(),_window.GetHeight(),sx,sy,
                shipX,shipY,shipYaw,0.24f,0.22f,true);
            if(picked>=0){
                _shipBuilder.Activate(ShipyardBuilderCommand::SelectPlaced,picked);
                uiConsumed=true;
            }
        }
        if(!uiConsumed&&_workspace.Mode()==SandboxWorkspaceMode::Flight){
            const int preset=TacticalContactsSystem::HitTestPreset(_tacticalContacts,_window.GetWidth(),_window.GetHeight(),sx,sy);
            const int row=TacticalContactsSystem::HitTestRow(_tacticalContacts,_window.GetWidth(),_window.GetHeight(),sx,sy);
            const int visible=std::min<int>(_tacticalContacts.maxVisibleRows,static_cast<int>(_tacticalContacts.rows.size()));
            const auto layout=TacticalContactsSystem::Layout(_window.GetWidth(),_window.GetHeight(),visible);
            if(preset>=0){_tacticalContacts.preset=TacticalContactsSystem::PresetFromIndex(preset);uiConsumed=true;}
            else if(row>=0){_selection=ToNativeContactSelection(_tacticalContacts.rows[static_cast<std::size_t>(row)]);if(_window.IsControlDown()&&_selection.IsValid())TacticalTargetingSystem::Request(_targeting,ToTargetReference(_selection));uiConsumed=true;}
            else if(layout.Contains(sx,sy,visible))uiConsumed=true;
        }
        if(!uiConsumed){
            const auto rail=_playerFacing.BuildCommandRail(_workspace.Mode());
            const int railIndex=_playerFacing.HitTestCommandRail(rail,sx,sy);
            if(railIndex>=0){
                const auto& item=rail.items[static_cast<std::size_t>(railIndex)];
                const bool leavingShipyard=_workspace.Mode()==SandboxWorkspaceMode::ShipBuilder && item.workspace!=SandboxWorkspaceMode::ShipBuilder;
                if(item.workspace==SandboxWorkspaceMode::Flight)_workspace.Close();
                else if(item.workspace==SandboxWorkspaceMode::ShipBuilder){
                    OpenShipyardWorkspace(false);
                } else _workspace.Open(item.workspace);
                if(leavingShipyard)RestoreGameplayCameraLimits();
            } else if(_workspace.Mode()==SandboxWorkspaceMode::SystemMap){
                const int hit=_playerFacing.HitTestSystemMapNode(_systemMap,&_universeSystemMap,
                    _window.GetWidth(),_window.GetHeight(),_systemMapRuntime.zoom,_systemMapRuntime.pan,sx,sy);
                if(hit>=0){_systemMap.selected=static_cast<std::size_t>(hit);_contextMenu.open=false;}
            } else if(_embodiment.IsPiloting()&&!_workspace.IsOverlayOpen()) {
                const Vector3 world = NativeBattlefieldRenderer::ScreenToWorld(
                    sx,sy,_window.GetWidth(),_window.GetHeight(),_engine.GetStrategicCamera());
                _selection = NativeBattlefieldRenderer::PickContact(_sector,world,2.2f,&_stationContacts);
                if(_window.IsControlDown()&&_selection.IsValid())TacticalTargetingSystem::Request(_targeting,ToTargetReference(_selection));
            }
        }
    }
}

void NativeGameApplication::UpdateCameraAndSelection()
{
    UpdateVectorCamera();
    auto* player = _engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);
    if (!player) return;
    auto& camera=_engine.GetStrategicCamera();
    if(_workspace.Mode()==SandboxWorkspaceMode::ShipBuilder){
        // Shipyard owns its inspection pivot. Following the player every frame
        // would immediately undo Frame Part, Frame Ship and MMB authoring pans.
        camera.Update(_engine.GetLastDeltaTime());
        return;
    }
    EnvironmentPresentationSystem environment;
    RuntimeControlContextSystem controls;
    const auto controlContext=controls.Build(_workspace.Mode(),_embodiment.Mode(),_docking.stage,
        _vectorTravelSystem.InTransit(_vectorTravel),_strategicFlight.Mode()==FlightControlMode::Strategic);
    const auto motion=environment.MotionFor(controlContext.cameraMode,player->velocity.length());
    camera.SetFollowSmoothness(motion.followSmoothness);
    camera.SetVelocityLookAhead(motion.velocityLookAhead);
    if(_embodiment.IsOnFoot()){
        const auto& avatar=_embodiment.Avatar();
        const Vector3 avatarWorld=player->position+Vector3{avatar.localPosition.x*.72f,avatar.localPosition.y*.72f,0.0f};
        camera.FollowTarget(avatarWorld,{},_engine.GetLastDeltaTime());
    }else{
        camera.FollowTarget(player->position,player->velocity,_engine.GetLastDeltaTime());
    }
}

NativeBattlefieldFrame NativeGameApplication::BuildRenderFrame() const
{
    NativeBattlefieldFrame f;
    f.frontendScreen=_frontend.Screen();
    f.frontendSelected=_frontendSelected;
    f.frontendHovered=_frontendHovered;
    f.frontendConfig=_frontend.Config();
    f.starterCareer=_starterCareer;
    f.workspaceMode=_workspace.Mode();
    f.standaloneShipyard=_standaloneShipyard;
    f.systemMap=&_systemMap;
    f.universeSystemMap=&_universeSystemMap;
    f.workspaceLines=BuildWorkspaceLines();
    f.vectorTravelStage=_vectorTravel.stage;
    f.vectorTravelProgress=_vectorTravel.progress;
    f.vectorTravelStatus=_vectorTravel.status;
    f.vectorVisual=_forwardPresentation.VectorVisual(_vectorTravel.stage,_vectorTravel.stageProgress);
    f.arrivalTitle=_arrivalTitle;
    f.arrivalTitleAlpha=CinematicFlightPresentationSystem::Evaluate(_vectorTravel.stage,_vectorTravel.stageProgress,_vectorTravel.stageSeconds).arrivalTitleAlpha;
    f.shipInspection=_shipInspection;
    f.strategicFlightMode=_strategicFlight.Mode()==FlightControlMode::Strategic;
    f.backdrop=_forwardPresentation.ForSector(_sector);
    f.embodimentMode=_embodiment.Mode();
    f.interiorAvatar=_embodiment.Avatar();
    f.dockingStage=_docking.stage;
    f.dockingState=&_docking;
    f.dockingProgress=_docking.progress;
    f.dockingStatus=_docking.status;
    ProductionContextKind context=ProductionContextKind::None;
    switch(_selection.kind){
        case NativeContactKind::Ship:context=ProductionContextKind::Ship;break;
        case NativeContactKind::Planet:context=ProductionContextKind::Planet;break;
        case NativeContactKind::Station:context=ProductionContextKind::Station;break;
        case NativeContactKind::Derelict:context=ProductionContextKind::Derelict;break;
        case NativeContactKind::OrbitalHub:context=ProductionContextKind::OrbitalHub;break;
        case NativeContactKind::Asteroid:context=ProductionContextKind::Asteroid;break;
        case NativeContactKind::Site:context=ProductionContextKind::Site;break;
        default:break;
    }
    f.productionHud=_productionInterface.Build(context,_selection.id,_embodiment.Mode(),_docking.stage,_vectorTravelSystem.InTransit(_vectorTravel));
    f.flightHud=_flightHud;
    const auto* hudPlayer=const_cast<Engine&>(_engine).GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);
    const float hudSpeed=hudPlayer?hudPlayer->velocity.length():0.0f;
    bool hudDamp=true,hudBoost=false;if(const auto* controls=_engine.GetPlayerControlSystem()){hudDamp=controls->IsInertialDampeningEnabled();hudBoost=controls->IsBoostActive();}
    const auto* hudCombat=const_cast<Engine&>(_engine).GetEntityManager().GetComponent<CombatComponent>(_playerEntity);
    f.commandHud=ShipCommandHudSystem::Build(hudSpeed,hudDamp,hudBoost,{"PRIMARY","MINING","SCAN","DRONES","REPAIR","UTILITY"},hudCombat);
    f.tacticalContacts=_tacticalContacts;
    f.targeting=_targeting;
    f.runtimeWindowLayout=&_runtimeWindowLayout;
    f.observableWarpEvents=&_observableWarpEvents;
    f.commandRail=_standaloneShipyard?CommandRailRuntimeModel{}:_playerFacing.BuildCommandRail(_workspace.Mode());
    f.contextMenu=_contextMenu;
    f.hangarRuntime=_hangarRuntime;
    f.stationContacts=&_stationContacts;
    f.galaxyRuntime=&_galaxyRuntime;
    f.planetIndustryRuntime=(_activePiPlanet!=static_cast<std::size_t>(-1))?&_activePiRuntime:nullptr;
    f.fleetRuntime=_fleetRuntime;
    f.fleetCaptainRuntime=&_fleetCaptainRuntime;
    f.systemMapZoom=_systemMapRuntime.zoom;
    f.systemMapPan=_systemMapRuntime.pan;
    f.pointerX=_window.GetPointerX();f.pointerY=_window.GetPointerY();
    if(_workspace.Mode()==SandboxWorkspaceMode::SystemMap&&f.systemMap){
        f.systemMapHoveredNode=_playerFacing.HitTestSystemMapNode(*f.systemMap,f.universeSystemMap,
            _window.GetWidth(),_window.GetHeight(),f.systemMapZoom,f.systemMapPan,
            _window.GetPointerX(),_window.GetPointerY());
    }
    f.shipBuilder=_shipBuilder.IsInitialized()?&_shipBuilder.Model():nullptr;
    f.shipBuilderRecipe=_shipBuilder.IsInitialized()?&_shipBuilder.Recipe():nullptr;
    f.shipBuilderAppearance=_shipBuilder.IsInitialized()?&_shipBuilder.Appearance():nullptr;
    f.playerShipRecipe=_hasPlayerShipRecipe?&_playerShipRecipe:nullptr;
    f.playerShipAppearance=_hasPlayerShipRecipe?&_playerShipAppearance:nullptr;
    f.sector=(_frontend.Screen()==FrontendScreen::InGame)?&_sector:nullptr;
    f.playerPhysics=const_cast<Engine&>(_engine).GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);
    f.input=&_engine.GetInputState();
    f.uiRenderer=&_engine.GetUIRenderer();
    f.camera=&_engine.GetStrategicCamera();
    f.missileSystem=&_miningLoop.GetMissileSystem();
    f.fractureSystem=&_miningLoop.GetFractureSystem();
    f.miningTelemetry=&_miningLoop.GetTelemetry();
    f.selection=_selection;
    f.elapsedSeconds=static_cast<float>(_engine.GetElapsedSeconds());
    f.viewportWidth=_window.GetWidth();
    f.viewportHeight=_window.GetHeight();
    if (const auto* controls=_engine.GetPlayerControlSystem()) {
        f.inertialDampening=controls->IsInertialDampeningEnabled();
        f.boostActive=controls->IsBoostActive();
    }
    // Ship-only production direction: zoom never reveals an interior/cutaway.
    f.cutaway={};
    return f;
}

} // namespace subspace
