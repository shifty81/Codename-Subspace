#include "integration/PlayerFacingIntegrationSystem.h"

#include "navigation/GalaxyMapSystem.h"
#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
constexpr float kPi = 3.14159265358979323846f;
float WrapAngle(float v) {
    while (v > kPi) v -= 2.0f * kPi;
    while (v < -kPi) v += 2.0f * kPi;
    return v;
}
}

CommandRailRuntimeModel PlayerFacingIntegrationSystem::BuildCommandRail(SandboxWorkspaceMode current) const {
    CommandRailRuntimeModel r;
    const struct Entry{const char*id;const char*label;const char*shortLabel;SandboxWorkspaceMode mode;} entries[]={
        {"flight","FLIGHT","FLT",SandboxWorkspaceMode::Flight},{"galaxy","GALAXY","GAL",SandboxWorkspaceMode::GalaxyMap},
        {"system","SYSTEM","SYS",SandboxWorkspaceMode::SystemMap},{"fleet","FLEET","FLE",SandboxWorkspaceMode::FleetCorporation},
        {"fitting","FITTING","FIT",SandboxWorkspaceMode::HangarFitting},{"industry","INDUSTRY","IND",SandboxWorkspaceMode::PlanetaryManufacturing},
        {"station","STATION","STA",SandboxWorkspaceMode::StationBuilder},{"market","MARKET","MKT",SandboxWorkspaceMode::MarketContracts},
        {"explore","EXPLORE","EXP",SandboxWorkspaceMode::Exploration},{"ship","SHIP","SHP",SandboxWorkspaceMode::ShipBuilder}};
    for(const auto&e:entries)r.items.push_back({e.id,e.label,e.shortLabel,e.mode,e.mode==current});return r;
}
int PlayerFacingIntegrationSystem::HitTestCommandRail(const CommandRailRuntimeModel& r,float x,float y) const {
    if(x<0||x>r.width||y<r.top)return -1;const int i=static_cast<int>((y-r.top)/r.rowHeight);return i>=0&&i<static_cast<int>(r.items.size())?i:-1;
}

FlightHudRuntimeModel PlayerFacingIntegrationSystem::BuildFlightHud(
    FlightControlMode mode, const std::vector<std::string>& fittedModules) const {
    FlightHudRuntimeModel model;
    model.modeLabel = mode == FlightControlMode::Strategic ? "STRATEGIC FLIGHT" : "MANUAL FLIGHT";
    model.moduleSlots = fittedModules;
    if (model.moduleSlots.size() > 8) model.moduleSlots.resize(8);
    return model;
}

ContextMenuRuntimeModel PlayerFacingIntegrationSystem::OpenContext(
    const InteractionContext& context, float screenX, float screenY) const {
    ContextActionSystem resolver;
    ContextMenuRuntimeModel model;
    model.screenX = screenX;
    model.screenY = screenY;
    model.actions = resolver.Resolve(context);
    model.open = !model.actions.empty();
    return model;
}

void PlayerFacingIntegrationSystem::MoveContextSelection(ContextMenuRuntimeModel& menu, int direction) const {
    if (!menu.open || menu.actions.empty() || direction == 0) return;
    const auto count = menu.actions.size();
    if (direction > 0) menu.selected = (menu.selected + 1) % count;
    else menu.selected = (menu.selected + count - 1) % count;
}

const ContextAction* PlayerFacingIntegrationSystem::ActiveContextAction(const ContextMenuRuntimeModel& menu) const {
    if (!menu.open || menu.actions.empty() || menu.selected >= menu.actions.size()) return nullptr;
    return &menu.actions[menu.selected];
}

ContextMenuLayout PlayerFacingIntegrationSystem::LayoutContextMenu(
    const ContextMenuRuntimeModel& menu, int viewportWidth, int viewportHeight) const {
    ContextMenuLayout layout;
    const float ui = std::clamp(std::min(viewportWidth / 1600.0f, viewportHeight / 900.0f), 1.0f, 1.55f);
    layout.width = 330.0f * ui;
    layout.headerHeight = 44.0f * ui;
    layout.rowHeight = 38.0f * ui;
    layout.textScale = 1.85f * ui;
    layout.height = layout.headerHeight + layout.rowHeight * static_cast<float>(menu.actions.size()) + 12.0f * ui;
    layout.x = std::clamp(menu.screenX, 10.0f, std::max(10.0f, viewportWidth - layout.width - 10.0f));
    layout.y = std::clamp(menu.screenY, 70.0f, std::max(70.0f, viewportHeight - layout.height - 10.0f));
    return layout;
}

int PlayerFacingIntegrationSystem::HitTestContextMenu(
    const ContextMenuRuntimeModel& menu, int viewportWidth, int viewportHeight, float x, float y) const {
    if (!menu.open || menu.actions.empty()) return -1;
    const auto layout = LayoutContextMenu(menu, viewportWidth, viewportHeight);
    if (x < layout.x || x > layout.x + layout.width || y < layout.y || y > layout.y + layout.height) return -1;
    const float rowStart = layout.y + layout.headerHeight;
    if (y < rowStart) return -1;
    const int row = static_cast<int>((y - rowStart) / layout.rowHeight);
    if (row < 0 || row >= static_cast<int>(menu.actions.size())) return -1;
    return row;
}

StrategicFlightOrder PlayerFacingIntegrationSystem::ToStrategicOrder(
    const ContextAction& action, std::uint64_t targetId, const Vector3& targetPosition, float desiredRange) const {
    StrategicFlightOrder order;
    order.targetId = targetId;
    order.targetPosition = targetPosition;
    order.desiredRange = desiredRange;
    order.valid = action.enabled;
    if (!order.valid) return order;
    if (action.id == "approach" || action.id == "move") order.kind = StrategicOrderKind::Approach;
    else if (action.id == "orbit") order.kind = StrategicOrderKind::Orbit;
    else if (action.id == "range") order.kind = StrategicOrderKind::KeepRange;
    else if (action.id == "align") order.kind = StrategicOrderKind::Align;
    else if (action.id == "engage" || action.id == "fleet_engage") order.kind = StrategicOrderKind::Engage;
    else if (action.id == "mine" || action.id == "fleet_mine") order.kind = StrategicOrderKind::Mine;
    else if (action.id == "salvage") order.kind = StrategicOrderKind::Salvage;
    else if (action.id == "dock") order.kind = StrategicOrderKind::Dock;
    else if (action.id == "vector") order.kind = StrategicOrderKind::VectorTo;
    else if (action.id == "hold") order.kind = StrategicOrderKind::Hold;
    else order.valid = false;
    return order;
}

StrategicAutopilotCommand PlayerFacingIntegrationSystem::EvaluateAutopilot(
    const StrategicFlightSystem& flight, const Vector3& shipPosition, float shipYawRadians,
    const Vector3& targetPosition) const {
    StrategicAutopilotCommand command;
    if (flight.Mode() != FlightControlMode::Strategic || !flight.CurrentOrder().valid) return command;
    const Vector3 delta = targetPosition - shipPosition;
    const float distance = delta.length();
    command.intent = flight.Evaluate(shipPosition, targetPosition, distance);
    command.order = flight.CurrentOrder();
    command.valid = true;
    if (delta.length() > 0.001f) {
        const float desiredYaw = std::atan2(-delta.x, delta.y);
        command.desiredYawRadians = shipYawRadians + WrapAngle(desiredYaw - shipYawRadians);
    } else command.desiredYawRadians = shipYawRadians;
    command.throttle = command.intent.desiredThrottle;
    return command;
}

std::vector<RuntimeStationContact> PlayerFacingIntegrationSystem::BuildStationContacts(
    std::uint32_t seed, const Vector3& planetPosition, float planetRadius, bool populated) const {
    StationEcologySystem ecology;
    auto generated = ecology.BuildStartingSystem(seed, populated ? 4 : 1, true);
    if(populated){
        GeneratedStationProfile shipworks;
        shipworks.id=static_cast<std::uint64_t>(seed)*1000003ull+77ull;
        shipworks.name="Axiom Shipworks "+std::to_string(seed%97u+3u);
        shipworks.archetype=StationArchetype::Shipyard;shipworks.orbitClass=StationOrbitClass::LagrangeLike;
        shipworks.services={"Dock","Shipyard","Fitting","Repair","Fuel","Storage"};shipworks.serviceCount=static_cast<int>(shipworks.services.size());
        generated.push_back(std::move(shipworks));
    }
    std::vector<RuntimeStationContact> result;
    result.reserve(generated.size() + 1);
    for (std::size_t i = 0; i < generated.size(); ++i) {
        const auto& s = generated[i];
        const float angle = 0.72f + static_cast<float>(i) * 2.15f;
        const float ring = std::max(planetRadius * (4.0f + static_cast<float>(i) * 0.8f), 550.0f + 280.0f * static_cast<float>(i));
        RuntimeStationContact c;
        c.id = s.id;
        c.name = s.name;
        c.archetype = s.archetype;
        c.orbitClass = s.orbitClass;
        c.dockable = s.dockable;
        c.asteroidEmbedded = s.asteroidEmbedded;
        c.services = s.services;
        c.position = planetPosition + Vector3{std::cos(angle) * ring, std::sin(angle) * ring, 0.0f};
        result.push_back(c);
    }
    if (std::none_of(result.begin(), result.end(), [](const RuntimeStationContact& c){ return c.asteroidEmbedded; })) {
        const auto asteroid = ecology.BuildAsteroidStation(static_cast<std::uint64_t>(seed) + 9001u, 4);
        RuntimeStationContact a;
        a.id = asteroid.id;
        a.name = asteroid.name;
        a.archetype = asteroid.archetype;
        a.orbitClass = asteroid.orbitClass;
        a.dockable = asteroid.dockable;
        a.asteroidEmbedded = true;
        a.services = asteroid.services;
        a.position = planetPosition + Vector3{-std::max(planetRadius * 8.0f, 2600.0f), std::max(planetRadius * 2.0f, 800.0f), 0.0f};
        result.push_back(a);
    }
    return result;
}

HangarRuntimeModel PlayerFacingIntegrationSystem::BuildHangar(
    const GeneratedStationProfile& station, bool docked, float requestedCameraDistance) const {
    UniversalDockedStationSystem dock;
    const auto p = dock.Build(station);
    HangarRuntimeModel model;
    model.docked = docked;
    model.title = station.name;
    model.hangarProfile = p.hangarProfile;
    model.services = p.services;
    auto serviceName=[](StationServiceType t){switch(t){case StationServiceType::Refuel:return std::string("REFUEL");case StationServiceType::RepairHull:return std::string("REPAIR HULL");case StationServiceType::RepairModules:return std::string("REPAIR MODULES");case StationServiceType::Refit:return std::string("REFIT");case StationServiceType::Storage:return std::string("STORAGE");case StationServiceType::Market:return std::string("MARKET");case StationServiceType::Shipyard:return std::string("SHIPYARD");}return std::string("SERVICE");};
    for(auto t:p.availableServiceTypes)model.serviceActions.push_back(serviceName(t));
    model.orbitCamera = p.orbitCamera;
    model.showActualShip = p.showActualFittedShip;
    model.cameraDistance = std::clamp(requestedCameraDistance, p.cameraMinDistance, p.cameraMaxDistance);
    model.archetype = station.archetype;
    model.berthSize = StationBerthSize::Standard;
    return model;
}

SystemMapRuntimeModel PlayerFacingIntegrationSystem::BuildSystemMapRuntime(
    const std::vector<OrbitalBodyRecord>& bodies, double simulationSeconds, int samples) const {
    UniverseSystemMapSystem system;
    SystemMapRuntimeModel model;
    model.snapshot = system.Build(bodies, simulationSeconds, samples);
    for (std::size_t i = 0; i < model.snapshot.nodes.size(); ++i) {
        if (model.snapshot.nodes[i].dockable) { model.selected = i; break; }
    }
    return model;
}

void PlayerFacingIntegrationSystem::StepSystemMapSelection(SystemMapRuntimeModel& model, int direction) const {
    if (model.snapshot.nodes.empty() || direction == 0) return;
    const auto count = model.snapshot.nodes.size();
    model.selected = direction > 0 ? (model.selected + 1) % count : (model.selected + count - 1) % count;
}

void PlayerFacingIntegrationSystem::ZoomSystemMap(SystemMapRuntimeModel& model, float wheelDelta) const {
    model.zoom = std::clamp(model.zoom * std::pow(1.14f, wheelDelta), 0.10f, 12.0f);
}

bool PlayerFacingIntegrationSystem::ShouldPromoteSystemMapToGalaxy(
    const SystemMapRuntimeModel& model, float wheelDelta) const {
    return wheelDelta < 0.0f && model.zoom <= 0.14f;
}

void PlayerFacingIntegrationSystem::PanSystemMap(SystemMapRuntimeModel& model, float dx, float dy) const {
    model.pan.x += dx / std::max(0.1f, model.zoom);
    model.pan.y += dy / std::max(0.1f, model.zoom);
}

SystemMapScreenLayout PlayerFacingIntegrationSystem::LayoutSystemMap(
    const SystemMapSnapshot& map,const UniverseSystemMapSnapshot* live,
    int viewportWidth,int viewportHeight,float zoom,const Vector3& pan) const {
    SystemMapScreenLayout layout;
    layout.left=64.0f;layout.top=58.0f;
    layout.right=std::max(layout.left+420.0f,static_cast<float>(viewportWidth)-64.0f);
    layout.bottom=std::max(layout.top+320.0f,static_cast<float>(viewportHeight)-58.0f);
    const float infoWidth=std::clamp((layout.right-layout.left)*.225f,260.0f,340.0f);
    layout.plotRight=layout.right-infoWidth-18.0f;
    layout.infoLeft=layout.plotRight+18.0f;

    float extent=1.0f;
    if(live){
        for(const auto& n:live->nodes){
            extent=std::max(extent,std::max(std::fabs(n.currentPosition.x),std::fabs(n.currentPosition.y))+32.0f);
            for(const auto& point:n.orbitTrack)
                extent=std::max(extent,std::max(std::fabs(point.x),std::fabs(point.y))+16.0f);
        }
    }
    for(const auto& n:map.nodes)
        extent=std::max(extent,std::max(std::fabs(n.position.x),std::fabs(n.position.y))+std::max(4.0f,n.strategicRadius));

    layout.centerX=(layout.left+layout.plotRight)*.5f+pan.x*.02f;
    layout.centerY=(layout.top+layout.bottom)*.53f+pan.y*.02f;
    const float sx=(layout.plotRight-layout.left-90.0f)/(2.0f*extent);
    const float sy=(layout.bottom-layout.top-180.0f)/(2.0f*extent);
    layout.scale=std::max(.00001f,std::min(sx,sy))*std::max(.10f,zoom);
    return layout;
}

int PlayerFacingIntegrationSystem::HitTestSystemMapNode(
    const SystemMapSnapshot& map,const UniverseSystemMapSnapshot* live,
    int viewportWidth,int viewportHeight,float zoom,const Vector3& pan,
    float screenX,float screenY) const {
    const auto layout=LayoutSystemMap(map,live,viewportWidth,viewportHeight,zoom,pan);
    if(!layout.Contains(screenX,screenY)||screenX>layout.plotRight)return -1;
    int best=-1;float bestDistance=1e30f;
    for(std::size_t i=0;i<map.nodes.size();++i){
        const auto& n=map.nodes[i];if(!n.known)continue;
        const float x=layout.centerX+n.position.x*layout.scale;
        const float y=layout.centerY-n.position.y*layout.scale;
        float radius=9.0f;
        switch(n.kind){
            case SystemMapNodeKind::Star:radius=16.0f;break;
            case SystemMapNodeKind::Planet:radius=13.0f;break;
            case SystemMapNodeKind::Moon:radius=10.0f;break;
            case SystemMapNodeKind::Station:case SystemMapNodeKind::OrbitalHub:radius=12.0f;break;
            case SystemMapNodeKind::Belt:radius=15.0f;break;
            default:break;
        }
        const float dx=screenX-x,dy=screenY-y,d=std::sqrt(dx*dx+dy*dy);
        if(d<=radius&&d<bestDistance){best=static_cast<int>(i);bestDistance=d;}
    }
    return best;
}

GalaxyRuntimeModel PlayerFacingIntegrationSystem::BuildGalaxyRuntime(std::uint32_t seed, std::size_t count) const {
    GalaxyCatalogSystem catalog;
    GalaxyRuntimeModel model;
    model.catalog = catalog.Generate(seed, count);
    model.initialized = !model.catalog.empty();
    if (model.initialized) model.selectedSystem = model.catalog.front().id;
    return model;
}

bool PlayerFacingIntegrationSystem::SelectGalaxySystem(GalaxyRuntimeModel& model, std::uint32_t id) const {
    const auto it = std::find_if(model.catalog.begin(), model.catalog.end(), [id](const GalaxySystemRecord& r){ return r.id == id; });
    if (it == model.catalog.end()) return false;
    model.selectedSystem = id;
    return true;
}

void PlayerFacingIntegrationSystem::OrbitGalaxy(GalaxyRuntimeModel& model, float dx, float dy) const {
    GalaxyMapSystem map;
    map.Orbit(model.camera, dx, dy);
}

void PlayerFacingIntegrationSystem::ZoomGalaxy(GalaxyRuntimeModel& model, float wheelDelta) const {
    GalaxyMapSystem map;
    map.Zoom(model.camera, wheelDelta * 35.0f);
}

bool PlayerFacingIntegrationSystem::PlotGalaxyRoute(GalaxyRuntimeModel& model, std::uint32_t start,
    std::uint32_t end, float jumpRange, GalaxyRouteMode mode) const {
    GalaxyRoutePlannerSystem planner;
    GalaxyRouteRequest request;
    request.start = start;
    request.end = end;
    request.jumpRange = jumpRange;
    request.mode = mode;
    model.route = planner.Plan(model.catalog, request);
    return model.route.valid;
}

PlanetIndustryRuntimeModel PlayerFacingIntegrationSystem::BuildPlanetIndustry(
    const PlanetData& planet, int radius, std::uint32_t seed) const {
    PlanetaryIndustrySystem pi;
    PlanetIndustryRuntimeModel model;
    model.planetName = planet.name;
    model.industry = pi.Generate(planet, radius, seed);
    model.selectedHex = {0,0};
    auto it = model.industry.hexes.find(model.selectedHex);
    if (it != model.industry.hexes.end()) it->second.surveyed = true;
    return model;
}

bool PlayerFacingIntegrationSystem::PlaceIndustry(PlanetIndustryRuntimeModel& model,
    PiInstallationKind kind, PowerTechnology tech) const {
    PlanetaryIndustrySystem pi;
    auto it = model.industry.hexes.find(model.selectedHex);
    if (it == model.industry.hexes.end()) return false;
    it->second.surveyed = true;
    PiInstallation installation;
    installation.id = static_cast<std::uint64_t>(model.industry.installations.size() + 1);
    installation.hex = model.selectedHex;
    installation.kind = kind;
    installation.tech = tech;
    installation.power = kind == PiInstallationKind::Power ? 16.0 : 3.0;
    installation.outputPerHour = kind == PiInstallationKind::Extractor || kind == PiInstallationKind::AtmosphericCollector ? 20.0 : 0.0;
    installation.storage = kind == PiInstallationKind::Storage ? 120.0 : 0.0;
    const bool placed = pi.Place(model.industry, installation);
    if (placed && (kind == PiInstallationKind::Logistics || kind == PiInstallationKind::Storage)) model.tetherAvailable = true;
    return placed;
}

double PlayerFacingIntegrationSystem::TickIndustryToTether(PlanetIndustryRuntimeModel& model,
    double produced, double hours) const {
    PlanetaryIndustrySystem pi;
    const double moved = pi.TransferToTether(model.industry, produced, hours);
    model.tetherStored = model.industry.tetherStorage;
    return moved;
}

FleetRuntimeModel PlayerFacingIntegrationSystem::BuildFleet(const std::vector<FleetWingShip>& wing,
    const FleetFlightConfig& config, StrategicOrderKind playerOrder) const {
    FleetIntentSystem fleet;
    FleetRuntimeModel model;
    model.formation = fleet.FormationFor(config, playerOrder);
    model.spacing = config.spacing;
    const auto assignments = fleet.Mirror(wing, playerOrder);
    const std::size_t count = std::min<std::size_t>(4, assignments.size());
    for (std::size_t i = 0; i < count; ++i) {
        FleetRuntimeShip s;
        s.id = assignments[i].shipId;
        s.mirroredOrder = assignments[i].order;
        if (i < wing.size()) s.role = wing[i].role;
        const float side = (i % 2 == 0) ? -1.0f : 1.0f;
        const float rank = static_cast<float>(i / 2 + 1);
        switch (model.formation) {
            case FormationType::Line: s.desiredOffset = {side * rank * config.spacing, -rank * config.spacing * 0.2f, 0}; break;
            case FormationType::Column: s.desiredOffset = {0, -rank * config.spacing, 0}; break;
            case FormationType::Diamond: s.desiredOffset = {side * rank * config.spacing * 0.7f, -rank * config.spacing * 0.65f, 0}; break;
            case FormationType::Circle: {
                const float a = static_cast<float>(i) / static_cast<float>(std::max<std::size_t>(1,count)) * kPi * 2.0f;
                s.desiredOffset = {std::cos(a) * config.spacing, std::sin(a) * config.spacing, 0}; break;
            }
            default: s.desiredOffset = {side * rank * config.spacing * 0.65f, -rank * config.spacing, 0}; break;
        }
        model.ships.push_back(s);
    }
    return model;
}

PlayerFacingAcceptanceReport PlayerFacingIntegrationSystem::EvaluateAcceptance(
    const PlayerFacingAcceptanceState& state) const {
    PlayerFacingAcceptanceReport r;
    const struct Check { bool value; const char* name; } checks[] = {
        {state.hudIntegrated,"HUD"}, {state.contextCommands,"RMB context"},
        {state.strategicAutopilot,"strategic autopilot"}, {state.stationVisibleAndDockable,"station ecology"},
        {state.hangarIntegrated,"docked hangar"}, {state.liveSystemMap,"living System Map"},
        {state.galaxyRuntime,"galaxy runtime"}, {state.planetaryIndustryRuntime,"planetary industry"},
        {state.fleetRuntime,"fleet runtime"}, {state.shipOnly,"ship-only production direction"}
    };
    for (const auto& c : checks) {
        if (c.value) r.score += 10;
        else r.blockers.emplace_back(c.name);
    }
    r.pass = r.score == 100;
    return r;
}

} // namespace subspace
