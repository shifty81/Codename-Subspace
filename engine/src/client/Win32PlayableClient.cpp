#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "runtime/GameRuntime.h"
#include "factions/FactionEncounterModel.h"
#include "mining/MiningSalvageModel.h"
#include "procedural/ProceduralEncounterGenerator.h"
#include "scanning/SectorScanner.h"
#include "celestial/CelestialSystemGenerator.h"
#include "celestial/CelestialVisualGenerator.h"
#include "celestial/SectorResourceModel.h"
#include "rendering/RuntimeVisualProfile.h"
#include "ships/ModularShipFactory.h"
#include "ships/ShipModuleLibrary.h"
#include "ships/ShipPartCatalog.h"
#include "client/ClientParticleFx.h"
#include "trading/StationEconomy.h"
#include "home/HomeClientViewModel.h"
#include "home/HomeSurfaceBuilder.h"
#include "roguelite/RogueliteDirector.h"
#include "expedition/ExpeditionRun.h"
#include "travel/InterstellarRailTravel.h"
#include "flight/ShipFlightControl.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <map>

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kZoom = 0.72f;
constexpr int kDefaultWidth = 1280;
constexpr int kDefaultHeight = 720;
constexpr float kSectorOrbitWorldScale = 7.25f;
constexpr float kSectorMinimumOrbitRadius = 1450.0f;
constexpr float kCelestialScanRange = 2600.0f;
constexpr float kSolarLightFalloffRange = 5600.0f;

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
Vec2 operator*(Vec2 a, float s) { return {a.x * s, a.y * s}; }

float Length(Vec2 v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

Vec2 Normalize(Vec2 v)
{
    const float len = Length(v);
    if (len <= 0.0001f) {
        return {};
    }
    return {v.x / len, v.y / len};
}

float Distance(Vec2 a, Vec2 b)
{
    return Length(a - b);
}

Vec2 Rotate(Vec2 point, float radians)
{
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    return {point.x * c - point.y * s, point.x * s + point.y * c};
}

POINT ToPoint(Vec2 value)
{
    return {static_cast<LONG>(std::round(value.x)), static_cast<LONG>(std::round(value.y))};
}

float SeededUnit(std::uint32_t seed, int index)
{
    std::uint32_t value = seed + static_cast<std::uint32_t>(index) * 747796405u + 2891336453u;
    value = ((value >> ((value >> 28u) + 4u)) ^ value) * 277803737u;
    value = (value >> 22u) ^ value;
    return static_cast<float>(value & 0xffffu) / 65535.0f;
}

std::wstring ToWide(const std::string& value)
{
    return std::wstring(value.begin(), value.end());
}

std::string ToNarrow(const std::wstring& value)
{
    return std::string(value.begin(), value.end());
}

std::wstring FormatFloat(float value, int precision = 1)
{
    std::wstringstream stream;
    stream << std::fixed << std::setprecision(precision) << value;
    return stream.str();
}

struct Asteroid {
    Vec2 position;
    float radius = 38.0f;
    float hp = 100.0f;
    float rotation = 0.0f;
    std::uint32_t visualSeed = 1;
    std::string resourceTag = "ore";
    int richness = 1;
    subspace::RuntimeVisualProfile visual;
    bool alive = true;
};

struct CargoPod {
    Vec2 position;
    float value = 10.0f;
    std::string resourceTag = "ore";
    int units = 1;
    int creditValue = 10;
    subspace::CargoYieldItem cargo;
    subspace::RuntimeVisualProfile visual;
    bool alive = true;
};

struct Star {
    Vec2 position;
    int brightness = 120;
};

struct SectorBackdropBody {
    subspace::CelestialBodyDefinition definition;
    subspace::RuntimeVisualProfile visual;
    Vec2 position;
    float parallax = 1.0f;
    float rotation = 0.0f;
    float orbitRadius = 0.0f;
    float orbitAngleRadians = 0.0f;
    float orbitSpeedRadians = 0.0f;
    bool interactive = true;
};

struct PlayerShip {
    Vec2 position{-420.0f, 120.0f};
    Vec2 velocity{};
    float angle = 0.0f;
    float hull = 100.0f;
    float fuel = 100.0f;
    bool mainThrustersEnabled = true;
    bool rcsThrustersEnabled = true;
    bool precisionThrusters = false;
    int cargo = 0;
    int cargoValue = 0;
    int credits = 250;
};

struct DrawResources {
    HBRUSH background = CreateSolidBrush(RGB(5, 8, 14));
    HBRUSH panel = CreateSolidBrush(RGB(16, 23, 34));
    HBRUSH panelAlt = CreateSolidBrush(RGB(25, 34, 50));
    HBRUSH player = CreateSolidBrush(RGB(46, 184, 224));
    HBRUSH playerAccent = CreateSolidBrush(RGB(255, 220, 96));
    HBRUSH playerDark = CreateSolidBrush(RGB(17, 75, 104));
    HBRUSH asteroid = CreateSolidBrush(RGB(98, 91, 82));
    HBRUSH asteroidDark = CreateSolidBrush(RGB(62, 58, 53));
    HBRUSH cargo = CreateSolidBrush(RGB(109, 255, 154));
    HBRUSH station = CreateSolidBrush(RGB(65, 86, 142));
    HBRUSH stationDark = CreateSolidBrush(RGB(24, 31, 55));
    HBRUSH danger = CreateSolidBrush(RGB(190, 66, 70));
    HBRUSH homeZone = CreateSolidBrush(RGB(28, 70, 92));
    HBRUSH homeStructure = CreateSolidBrush(RGB(106, 194, 160));
    HBRUSH homePower = CreateSolidBrush(RGB(255, 202, 88));
    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(22, 31, 45));
    HPEN faintPen = CreatePen(PS_SOLID, 1, RGB(39, 53, 75));
    HPEN playerPen = CreatePen(PS_SOLID, 2, RGB(125, 239, 255));
    HPEN playerDarkPen = CreatePen(PS_SOLID, 2, RGB(18, 87, 120));
    HPEN targetPen = CreatePen(PS_SOLID, 2, RGB(255, 216, 88));
    HPEN miningPen = CreatePen(PS_SOLID, 3, RGB(255, 138, 72));
    HPEN shieldPen = CreatePen(PS_DOT, 1, RGB(74, 151, 184));
    HPEN orbitPen = CreatePen(PS_DOT, 1, RGB(31, 45, 68));
    HPEN celestialPen = CreatePen(PS_SOLID, 1, RGB(128, 174, 220));
    HFONT smallFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  DEFAULT_PITCH | FF_DONTCARE, L"Consolas");
    HFONT titleFont = CreateFontW(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    ~DrawResources()
    {
        DeleteObject(background);
        DeleteObject(panel);
        DeleteObject(panelAlt);
        DeleteObject(player);
        DeleteObject(playerAccent);
        DeleteObject(playerDark);
        DeleteObject(asteroid);
        DeleteObject(asteroidDark);
        DeleteObject(cargo);
        DeleteObject(station);
        DeleteObject(stationDark);
        DeleteObject(danger);
        DeleteObject(homeZone);
        DeleteObject(homeStructure);
        DeleteObject(homePower);
        DeleteObject(gridPen);
        DeleteObject(faintPen);
        DeleteObject(playerPen);
        DeleteObject(playerDarkPen);
        DeleteObject(targetPen);
        DeleteObject(miningPen);
        DeleteObject(shieldPen);
        DeleteObject(orbitPen);
        DeleteObject(celestialPen);
        DeleteObject(smallFont);
        DeleteObject(titleFont);
    }
};

class PlayableClientApp {
public:
    void Initialize(HWND hwnd)
    {
        _hwnd = hwnd;
        _runtime.Initialize();
        BuildRuntimeVisualProfiles();
        BuildHomeDirector();
        RefreshRailTravelRoutes();
        ResetWorld();
        AddConsoleLine(L"Subspace playable client ready.");
        AddConsoleLine(L"Press H for persistent home system, B for surface build mode, L to launch a run, X to extract a run.");
        AddConsoleLine(L"Press F1 for console, F2 for dev mode, Tab to target, Space to mine/fire.");
        AddConsoleLine(L"Flight polish: T toggles main drive, Y toggles RCS, Shift precision burn.");
    }

    void Shutdown()
    {
        _runtime.Shutdown();
    }

    void OnResize(int width, int height)
    {
        _width = std::max(320, width);
        _height = std::max(240, height);
    }

    void OnKeyDown(WPARAM key)
    {
        if (key < _keys.size()) {
            _keys[static_cast<std::size_t>(key)] = true;
        }

        if (key == VK_F1) {
            _consoleOpen = !_consoleOpen;
            return;
        }

        if (_consoleOpen) {
            if (key == VK_ESCAPE) {
                _consoleOpen = false;
            }
            else if (key == VK_RETURN) {
                ExecuteConsoleInput();
            }
            else if (key == VK_BACK) {
                if (!_consoleInput.empty()) {
                    _consoleInput.pop_back();
                }
            }
            return;
        }

        if (key == VK_F2) {
            ExecuteRuntimeCommand(L"dev.mode.toggle");
            _devMode = _runtime.GetDeveloperMode().IsEnabled();
        }
        else if (key == VK_F3) {
            _showDebug = !_showDebug;
        }
        else if (key == 'T') {
            ToggleMainThrusters();
        }
        else if (key == 'Y') {
            ToggleRcsThrusters();
        }
        else if (key == VK_SHIFT) {
            _player.precisionThrusters = true;
        }
        else if (key == 'M') {
            CycleShipPart(1);
        }
        else if (key == 'O') {
            InstallSelectedShipPart();
        }
        else if (key == 'H') {
            ToggleHomeView();
        }
        else if (_homeViewOpen && HandleHomeBuildKey(key)) {
            return;
        }
        else if (key == 'J') {
            CycleRunOffer(-1);
        }
        else if (key == 'K') {
            CycleRunOffer(1);
        }
        else if (key == 'L') {
            LaunchSelectedRun();
        }
        else if (key == 'X') {
            ExtractCurrentRun();
        }
        else if (key == VK_TAB) {
            SelectNearestAsteroid();
        }
        else if (key == 'N') {
            SelectNearestCelestial();
        }
        else if (key == 'G') {
            ScanSelectedCelestial();
        }
        else if (key == 'R') {
            AdvanceSectorAndReset();
        }
    }

    void OnKeyUp(WPARAM key)
    {
        if (key < _keys.size()) {
            _keys[static_cast<std::size_t>(key)] = false;
        }
        if (key == VK_SHIFT) {
            _player.precisionThrusters = false;
        }
    }

    void OnChar(WPARAM ch)
    {
        if (!_consoleOpen) {
            if (ch == '`' || ch == '~') {
                _consoleOpen = true;
            }
            return;
        }

        if (ch >= 32 && ch < 127) {
            _consoleInput.push_back(static_cast<wchar_t>(ch));
        }
    }

    void Update(float dt)
    {
        dt = std::min(dt, 0.05f);
        _runtime.Tick(dt);
        _sectorElapsedSeconds += dt;
        _homeElapsedSeconds += dt;
        TickHomeSystems(dt);
        TickRailTravel(dt);
        UpdateCelestialOrbits();
        _laserTimer = std::max(0.0f, _laserTimer - dt);
        _messageTimer = std::max(0.0f, _messageTimer - dt);
        _clientParticles.Update(dt);

        if (!_consoleOpen && !_homeViewOpen) {
            UpdatePlayer(dt);
            HandleActionEdges();
        }

        _previousKeys = _keys;
    }

    void Draw(HDC targetDc)
    {
        RECT clientRect{};
        GetClientRect(_hwnd, &clientRect);
        const int width = std::max(1, static_cast<int>(clientRect.right - clientRect.left));
        const int height = std::max(1, static_cast<int>(clientRect.bottom - clientRect.top));

        HDC backDc = CreateCompatibleDC(targetDc);
        HBITMAP backBitmap = CreateCompatibleBitmap(targetDc, width, height);
        HGDIOBJ oldBitmap = SelectObject(backDc, backBitmap);

        RECT fillRect{0, 0, width, height};
        FillRect(backDc, &fillRect, _draw.background);

        SetBkMode(backDc, TRANSPARENT);
        SelectObject(backDc, _draw.smallFont);

        DrawStarfield(backDc);
        if (_homeViewOpen) {
            DrawHomeView(backDc);
            DrawConsole(backDc);
        }
        else {
            DrawSectorBackdrop(backDc);
            DrawSectorLighting(backDc);
            DrawGrid(backDc);
            DrawWorld(backDc);
            DrawHud(backDc);
            DrawConsole(backDc);
        }

        BitBlt(targetDc, 0, 0, width, height, backDc, 0, 0, SRCCOPY);
        SelectObject(backDc, oldBitmap);
        DeleteObject(backBitmap);
        DeleteDC(backDc);
    }

private:
    bool KeyDown(int vk) const
    {
        if (vk < 0 || vk >= static_cast<int>(_keys.size())) {
            return false;
        }
        return _keys[static_cast<std::size_t>(vk)];
    }

    bool KeyPressed(int vk) const
    {
        if (vk < 0 || vk >= static_cast<int>(_keys.size())) {
            return false;
        }
        const std::size_t index = static_cast<std::size_t>(vk);
        return _keys[index] && !_previousKeys[index];
    }

    void BuildRuntimeVisualProfiles()
    {
        subspace::ShipModuleLibrary library;
        library.InitializeBuiltInModules();
        subspace::ModularShipFactory factory(library, 51051u);
        subspace::ModularShipConfig config;
        config.shipName = "Player Dev Skiff";
        config.size = subspace::ModularShipSize::Fighter;
        config.role = subspace::ModularShipRole::Mining;
        config.material = "Iron";
        config.seed = 51051u;
        config.addWings = true;
        config.addWeapons = true;
        config.addCargo = true;
        config.addHyperdrive = false;
        config.desiredWeaponMounts = 1;
        config.minimumEngines = 1;
        _playerShipDefinition = factory.CreateCustomShip(config);
        // The playable client is still a lightweight top-down renderer, not a GLB/Blender viewer.
        // Use the Ulysses source-model-inspired proxy as the visible starter ship so
        // the player ship reads like the intended corvette instead of a loose module pile.
        _playerVisual = subspace::BuildUlyssesStarterVisualProfile();
        if (_shipPartCatalog.empty()) {
            _shipPartCatalog = subspace::CreateStarterShipPartCatalog();
        }
        if (_shipLoadout.slots.empty()) {
            _shipLoadout = subspace::CreateStarterShipLoadout();
        }
        // Pass153 hard-visible marker: the old Starter Skiff placeholder label should not survive this pass.
        _shipLoadout.displayName = "Ulysses Proxy Starter";
        RecalculateShipPartStats();
        _stationVisual = subspace::BuildDefaultStationVisualProfile(7331u);
        _cargoVisual = subspace::BuildCargoPodVisualProfile(1201u);
    }

    void BuildHomeDirector()
    {
        _director = subspace::CreateRogueliteDirector(0x51B5ACEu);
        _homeView = subspace::BuildHomeClientViewModel(_director, _homeElapsedSeconds);
        _selectedRunOffer = _homeView.runOffers.empty() ? -1 : 0;
    }

    void RefreshHomeView()
    {
        _homeView = subspace::BuildHomeClientViewModel(_director, _homeElapsedSeconds);
        if (_homeView.runOffers.empty()) {
            _selectedRunOffer = -1;
        }
        else if (_selectedRunOffer < 0 || _selectedRunOffer >= static_cast<int>(_homeView.runOffers.size())) {
            _selectedRunOffer = 0;
        }
    }

    void TickHomeSystems(float dt)
    {
        _homeFactoryTick += dt;
        if (_homeFactoryTick >= 1.0f) {
            const float elapsed = _homeFactoryTick;
            _homeFactoryTick = 0.0f;
            subspace::TickHomeFactoryNetwork(_director.save.factory, elapsed);
            RefreshHomeView();
        }
        if (_activeRun.state == subspace::ExpeditionRunState::Active ||
            _activeRun.state == subspace::ExpeditionRunState::ExtractionAvailable) {
            subspace::TickExpeditionRun(_activeRun, dt);
        }
    }

    void ToggleHomeView()
    {
        _homeViewOpen = !_homeViewOpen;
        RefreshHomeView();
        ClampHomeBuildCursor();
        AddTimedMessage(_homeViewOpen ? L"Home Solar System view opened." : L"Returned to expedition flight view.");
    }

    bool HandleHomeBuildKey(WPARAM key)
    {
        if (key == 'B') {
            _homeBuildMode = !_homeBuildMode;
            RefreshHomeBuildPalette();
            AddTimedMessage(_homeBuildMode ? L"Surface build mode enabled." : L"Surface build mode disabled.");
            return true;
        }
        if (key == 'U') {
            CycleHomeBuildZone(-1);
            return true;
        }
        if (key == 'I') {
            CycleHomeBuildZone(1);
            return true;
        }
        if (key >= '1' && key <= '9') {
            SelectHomePaletteIndex(static_cast<int>(key - '1'));
            return true;
        }
        if (!_homeBuildMode) {
            return false;
        }
        if (key == VK_LEFT) {
            _homeBuildCursorX -= 1;
            ClampHomeBuildCursor();
            return true;
        }
        if (key == VK_RIGHT) {
            _homeBuildCursorX += 1;
            ClampHomeBuildCursor();
            return true;
        }
        if (key == VK_UP) {
            _homeBuildCursorY -= 1;
            ClampHomeBuildCursor();
            return true;
        }
        if (key == VK_DOWN) {
            _homeBuildCursorY += 1;
            ClampHomeBuildCursor();
            return true;
        }
        if (key == 'P') {
            PlaceSelectedHomeStructure();
            return true;
        }
        if (key == VK_DELETE) {
            RemoveSelectedHomeStructure();
            return true;
        }
        return false;
    }

    const subspace::HomeBuildZone* SelectedHomeBuildZone() const
    {
        const auto& zones = _director.save.home.buildZones;
        if (zones.empty() || _selectedHomeBuildZone < 0 || _selectedHomeBuildZone >= static_cast<int>(zones.size())) {
            return nullptr;
        }
        return &zones[static_cast<std::size_t>(_selectedHomeBuildZone)];
    }

    void RefreshHomeBuildPalette()
    {
        const auto* zone = SelectedHomeBuildZone();
        _homeBuildPalette = zone ? subspace::CreateHomeBuildPalette(zone->type) : std::vector<subspace::HomeStructureType>{};
        if (_homeBuildPalette.empty()) {
            _selectedHomePalette = -1;
        }
        else if (_selectedHomePalette < 0 || _selectedHomePalette >= static_cast<int>(_homeBuildPalette.size())) {
            _selectedHomePalette = 0;
        }
    }

    void ClampHomeBuildCursor()
    {
        const auto& zones = _director.save.home.buildZones;
        if (zones.empty()) {
            _selectedHomeBuildZone = -1;
            _homeBuildCursorX = 0;
            _homeBuildCursorY = 0;
            _homeBuildPalette.clear();
            _selectedHomePalette = -1;
            return;
        }
        if (_selectedHomeBuildZone < 0 || _selectedHomeBuildZone >= static_cast<int>(zones.size())) {
            _selectedHomeBuildZone = 0;
        }
        const auto& zone = zones[static_cast<std::size_t>(_selectedHomeBuildZone)];
        _homeBuildCursorX = std::max(0, std::min(_homeBuildCursorX, zone.gridWidth - 1));
        _homeBuildCursorY = std::max(0, std::min(_homeBuildCursorY, zone.gridHeight - 1));
        RefreshHomeBuildPalette();
    }

    void CycleHomeBuildZone(int delta)
    {
        const int count = static_cast<int>(_director.save.home.buildZones.size());
        if (count <= 0) {
            AddTimedMessage(L"No home build zones available.");
            return;
        }
        _selectedHomeBuildZone = (_selectedHomeBuildZone + delta + count) % count;
        ClampHomeBuildCursor();
        const auto* zone = SelectedHomeBuildZone();
        AddTimedMessage(zone ? L"Selected build zone: " + ToWide(zone->displayName) : L"No build zone selected.");
        RefreshHomeView();
    }

    void SelectHomePaletteIndex(int index)
    {
        RefreshHomeBuildPalette();
        if (index < 0 || index >= static_cast<int>(_homeBuildPalette.size())) {
            AddTimedMessage(L"No home build palette item in that slot.");
            return;
        }
        _selectedHomePalette = index;
        AddTimedMessage(L"Selected structure: " + ToWide(subspace::HomeStructureTypeName(_homeBuildPalette[static_cast<std::size_t>(_selectedHomePalette)])));
    }

    void PlaceSelectedHomeStructure()
    {
        ClampHomeBuildCursor();
        const auto* zone = SelectedHomeBuildZone();
        if (!zone || _selectedHomePalette < 0 || _selectedHomePalette >= static_cast<int>(_homeBuildPalette.size())) {
            AddTimedMessage(L"No home build zone or structure selected.");
            return;
        }
        subspace::HomeBuildPlacementRequest request;
        request.zoneId = zone->id;
        request.type = _homeBuildPalette[static_cast<std::size_t>(_selectedHomePalette)];
        request.x = _homeBuildCursorX;
        request.y = _homeBuildCursorY;
        const auto result = subspace::PlaceHomeStructure(_director.save.home, _director.save.factory, request);
        _lastHomeBuildMessage = result.message;
        RefreshHomeView();
        AddTimedMessage(ToWide(result.message));
    }

    void RemoveSelectedHomeStructure()
    {
        ClampHomeBuildCursor();
        const auto* zone = SelectedHomeBuildZone();
        if (!zone) {
            AddTimedMessage(L"No home build zone selected.");
            return;
        }
        const auto result = subspace::RemoveHomeStructure(_director.save.home, _director.save.factory, zone->id, _homeBuildCursorX, _homeBuildCursorY);
        _lastHomeBuildMessage = result.message;
        RefreshHomeView();
        AddTimedMessage(ToWide(result.message));
    }

    void CycleRunOffer(int delta)
    {
        RefreshHomeView();
        if (_homeView.runOffers.empty()) {
            _selectedRunOffer = -1;
            AddTimedMessage(L"No expedition run offers available.");
            return;
        }
        const int count = static_cast<int>(_homeView.runOffers.size());
        _selectedRunOffer = (_selectedRunOffer + delta + count) % count;
        const auto& offer = _homeView.runOffers[static_cast<std::size_t>(_selectedRunOffer)];
        AddTimedMessage(L"Selected run: " + ToWide(offer.displayName));
    }

    void LaunchSelectedRun()
    {
        if (_director.availableRuns.empty()) {
            AddTimedMessage(L"No expedition run offers available.");
            return;
        }
        if (_selectedRunOffer < 0 || _selectedRunOffer >= static_cast<int>(_director.availableRuns.size())) {
            _selectedRunOffer = 0;
        }
        const auto& offer = _director.availableRuns[static_cast<std::size_t>(_selectedRunOffer)];
        _activeRun = subspace::CreateExpeditionRun(offer.config);
        subspace::StartExpeditionRun(_activeRun);
        _homeViewOpen = false;
        _sectorId = offer.config.runId;
        _sectorSeed = offer.config.seed;
        _sectorIndex += 1;
        BuildSectorSystem();
        ResetWorld();
        AddTimedMessage(L"Launched expedition: " + ToWide(offer.displayName));
    }

    void ExtractCurrentRun()
    {
        if (_activeRun.state != subspace::ExpeditionRunState::Active &&
            _activeRun.state != subspace::ExpeditionRunState::ExtractionAvailable) {
            AddTimedMessage(L"No active expedition to extract.");
            return;
        }
        _activeRun.pendingRewards.credits += std::max(0, _player.cargoValue);
        for (const auto& item : _cargoManifest) {
            subspace::AddExpeditionCargoReward(_activeRun, item);
        }
        subspace::MarkExpeditionExtracted(_activeRun);
        subspace::ApplyExtractedRunRewards(_director, _activeRun);
        _player.cargo = 0;
        _player.cargoValue = 0;
        _cargoManifest.clear();
        _homeViewOpen = true;
        RefreshHomeView();
        AddTimedMessage(L"Extracted run rewards to the Home Solar System.");
    }

    void BuildSectorSystem()
    {
        subspace::CelestialSystemGeneratorOptions options;
        options.minPlanets = 4;
        options.maxPlanets = 7;
        options.includeAsteroidBelt = true;
        options.allowBlackHolePrimary = true;
        subspace::CelestialSystemGenerator generator(options);

        if (_activeRun.state == subspace::ExpeditionRunState::Active || _activeRun.state == subspace::ExpeditionRunState::ExtractionAvailable) {
            _sectorId = _activeRun.config.runId;
            _sectorSeed = _activeRun.config.seed;
        }
        else {
            _sectorId = "DEV-" + std::to_string(100 + _sectorIndex);
            _sectorSeed = subspace::CelestialHashSeed(_sectorId, 0x53504345u + static_cast<std::uint32_t>(_sectorIndex * 977));
        }
        _sectorSystem = generator.GenerateSystem(_sectorId, _sectorSeed);
        _sectorSurvey = subspace::BuildSectorResourceSurvey(_sectorSystem);
        subspace::SectorScanner scanner;
        _sectorScannerReport = scanner.ScanSystem(_sectorSystem, _sectorSurvey, 2);
        _stationEconomy = subspace::BuildStationEconomyFromSurvey(_sectorSurvey);
        _encounterTable = subspace::BuildEncounterSpawnTable(_sectorSurvey, _sectorSeed);
        _sectorBackdrop.clear();

        const Vec2 primaryAnchor{2600.0f, -950.0f};
        SectorBackdropBody primary;
        primary.definition = _sectorSystem.primary;
        primary.visual = subspace::BuildCelestialVisualProfile(primary.definition, {});
        primary.position = primaryAnchor;
        primary.parallax = 1.0f;
        primary.rotation = 0.0f;
        primary.orbitRadius = 0.0f;
        primary.orbitAngleRadians = 0.0f;
        primary.orbitSpeedRadians = 0.0f;
        primary.interactive = true;
        _sectorBackdrop.push_back(primary);

        int bodyIndex = 0;
        for (const auto& body : _sectorSystem.bodies) {
            SectorBackdropBody backdrop;
            backdrop.definition = body;
            backdrop.visual = subspace::BuildCelestialVisualProfile(body, {});
            const float distance = std::max(kSectorMinimumOrbitRadius + static_cast<float>(bodyIndex) * 420.0f,
                                            body.orbitRadius * kSectorOrbitWorldScale);
            backdrop.orbitRadius = distance;
            backdrop.orbitAngleRadians = body.orbitAngleRadians;
            backdrop.orbitSpeedRadians = 0.0024f / std::sqrt(std::max(1.0f, distance / 1200.0f));
            if (body.type == subspace::CelestialBodyType::AsteroidBelt) {
                backdrop.orbitSpeedRadians *= 0.45f;
            }
            backdrop.position = {primaryAnchor.x + std::cos(backdrop.orbitAngleRadians) * distance,
                                 primaryAnchor.y + std::sin(backdrop.orbitAngleRadians) * distance};
            backdrop.parallax = 1.0f;
            backdrop.rotation = body.orbitAngleRadians * 0.2f;
            backdrop.interactive = true;
            _sectorBackdrop.push_back(backdrop);
            ++bodyIndex;
        }
        SelectNearestCelestial();
    }

    void AdvanceSectorAndReset()
    {
        ++_sectorIndex;
        BuildSectorSystem();
        ResetWorld();
        AddConsoleLine(L"Generated " + ToWide(_sectorSystem.displayName) + L".");
    }

    void ResetWorld()
    {
        _player = PlayerShip{};
        _player.fuel = _shipPartStats.fuelCapacity;
        _clientParticles.Clear();
        _station = {0.0f, 0.0f};
        _asteroids.clear();
        _cargo.clear();
        _stars.clear();
        _selectedAsteroid = -1;
        _laserFlash = 0.0f;
        if (_sectorBackdrop.empty()) {
            BuildSectorSystem();
        }
        SelectNearestCelestial();

        std::mt19937 rng(_sectorSeed == 0 ? 7331u : _sectorSeed);
        std::uniform_real_distribution<float> position(-1250.0f, 1250.0f);
        std::uniform_real_distribution<float> radius(26.0f, 64.0f);
        for (int i = 0; i < 26; ++i) {
            Asteroid asteroid;
            asteroid.position = {position(rng), position(rng)};
            if (Distance(asteroid.position, _station) < 240.0f) {
                asteroid.position.x += 360.0f;
            }
            asteroid.radius = radius(rng);
            asteroid.rotation = position(rng) * 0.01f;
            asteroid.visualSeed = static_cast<std::uint32_t>(_sectorSeed + 7331u + static_cast<std::uint32_t>(i * 97));
            asteroid.resourceTag = subspace::PickPrimaryResourceTag(_sectorSurvey, asteroid.visualSeed, i);
            asteroid.richness = std::max(1, _sectorSurvey.asteroidField.resourceRichness + static_cast<int>(subspace::CelestialSeededUnit(asteroid.visualSeed, 6) * 3.0f) - 1);
            asteroid.hp = asteroid.radius * 1.55f + static_cast<float>(asteroid.richness * 7);
            asteroid.visual = subspace::BuildAsteroidVisualProfile(asteroid.radius, asteroid.visualSeed);
            _asteroids.push_back(asteroid);
        }

        std::uniform_real_distribution<float> starPos(-4000.0f, 4000.0f);
        std::uniform_int_distribution<int> brightness(70, 210);
        for (int i = 0; i < 340; ++i) {
            _stars.push_back({{starPos(rng), starPos(rng)}, brightness(rng)});
        }

        SelectNearestAsteroid();
        SelectNearestCelestial();
    }

    void UpdateCelestialOrbits()
    {
        if (_sectorBackdrop.empty()) {
            return;
        }
        const Vec2 primary = _sectorBackdrop.front().position;
        for (std::size_t i = 1; i < _sectorBackdrop.size(); ++i) {
            auto& body = _sectorBackdrop[i];
            const float angle = body.orbitAngleRadians + _sectorElapsedSeconds * body.orbitSpeedRadians;
            body.position = {primary.x + std::cos(angle) * body.orbitRadius,
                             primary.y + std::sin(angle) * body.orbitRadius};
            body.rotation = angle * 0.16f + _sectorElapsedSeconds * 0.015f;
        }
    }

    void UpdatePlayer(float dt)
    {
        subspace::ShipFlightInputFrame input;
        input.thrust = KeyDown('W') ? 1.0f : 0.0f;
        input.reverse = KeyDown('S') ? 1.0f : 0.0f;
        input.strafe = (KeyDown('E') ? 1.0f : 0.0f) - (KeyDown('Q') ? 1.0f : 0.0f);
        input.turn = (KeyDown('D') ? 1.0f : 0.0f) - (KeyDown('A') ? 1.0f : 0.0f);
        input.mainThrustersEnabled = _player.mainThrustersEnabled && _player.fuel > 0.05f;
        input.rcsThrustersEnabled = _player.rcsThrustersEnabled && _player.fuel > 0.05f;
        input.precisionMode = _player.precisionThrusters || KeyDown(VK_SHIFT);

        subspace::ShipFlightControlProfile profile;
        profile.mainThrust = _shipPartStats.thrust;
        profile.reverseThrust = _shipPartStats.reverseThrust;
        profile.strafeThrust = _shipPartStats.strafeThrust;
        profile.turnRate = _shipPartStats.turnSpeed;
        profile.maxSpeed = _shipPartStats.maxSpeed;
        profile.fuelBurnMain = std::max(0.05f, _shipPartStats.fuelBurnPerSecond);
        profile.fuelBurnRcs = std::max(0.015f, _shipPartStats.fuelBurnPerSecond * 0.33f);
        profile.precisionMultiplier = 0.42f;

        const auto control = subspace::EvaluateShipFlightControl(input, profile);
        _lastFlightInput = input;
        _lastFlightControl = control;

        const Vec2 forward{std::cos(_player.angle), std::sin(_player.angle)};
        const Vec2 right{-forward.y, forward.x};
        if (std::fabs(control.angularForce) > 0.0001f) {
            _player.angle += control.angularForce * dt;
        }
        if (std::fabs(control.forwardForce) > 0.0001f) {
            _player.velocity = _player.velocity + forward * (control.forwardForce * dt);
        }
        if (std::fabs(control.lateralForce) > 0.0001f) {
            _player.velocity = _player.velocity + right * (control.lateralForce * dt);
        }

        if (control.fuelBurnPerSecond > 0.0f) {
            _player.fuel = std::max(0.0f, _player.fuel - control.fuelBurnPerSecond * dt);
            SpawnThrusterParticles(control.mainBurning,
                                   control.retroBurning,
                                   input.strafe < -0.01f,
                                   input.strafe > 0.01f,
                                   input.turn < -0.01f,
                                   input.turn > 0.01f);
        }

        const float damping = (_player.mainThrustersEnabled || _player.rcsThrustersEnabled) ? 0.989f : 0.9975f;
        _player.velocity = _player.velocity * damping;
        if (Length(_player.velocity) > _shipPartStats.maxSpeed) {
            _player.velocity = Normalize(_player.velocity) * _shipPartStats.maxSpeed;
        }
        _player.position = _player.position + _player.velocity * dt;

        if (_selectedAsteroid < 0 || _selectedAsteroid >= static_cast<int>(_asteroids.size()) || !_asteroids[_selectedAsteroid].alive) {
            SelectNearestAsteroid();
        }
    }

    void RecalculateShipPartStats()
    {
        _shipPartStats = subspace::CalculateShipPartStats(_shipLoadout, _shipPartCatalog);
        if (_shipPartStats.fuelCapacity <= 0.0f) {
            _shipPartStats.fuelCapacity = 100.0f;
        }
        _player.fuel = std::min(std::max(0.0f, _player.fuel), _shipPartStats.fuelCapacity);
    }

    bool ExpeditionActive() const
    {
        return _activeRun.state == subspace::ExpeditionRunState::Active ||
               _activeRun.state == subspace::ExpeditionRunState::ExtractionAvailable;
    }

    std::wstring CurrentShipPartText() const
    {
        if (_shipPartCatalog.empty()) {
            return L"no part catalog";
        }
        const auto& part = _shipPartCatalog[static_cast<std::size_t>(_selectedShipPartIndex % static_cast<int>(_shipPartCatalog.size()))];
        return ToWide(std::string(subspace::ShipPartCategoryName(part.category)) + ": " + part.displayName + " (" + std::to_string(part.installCostCredits) + " cr)");
    }

    void CycleShipPart(int delta)
    {
        if (_shipPartCatalog.empty()) {
            AddTimedMessage(L"No ship part catalog loaded.");
            return;
        }
        const int count = static_cast<int>(_shipPartCatalog.size());
        _selectedShipPartIndex = (_selectedShipPartIndex + delta + count) % count;
        AddTimedMessage(L"Selected ship part: " + CurrentShipPartText());
    }

    void InstallSelectedShipPart()
    {
        if (_shipPartCatalog.empty()) {
            AddTimedMessage(L"No ship part catalog loaded.");
            return;
        }
        const auto& part = _shipPartCatalog[static_cast<std::size_t>(_selectedShipPartIndex % static_cast<int>(_shipPartCatalog.size()))];
        auto result = subspace::InstallShipPart(_shipLoadout, part, _player.credits, _homeViewOpen, ExpeditionActive());
        if (result.success) {
            _player.credits -= result.costCredits;
            RecalculateShipPartStats();
            BuildRuntimeVisualProfiles();
            RefreshHomeView();
        }
        AddTimedMessage(ToWide(result.message));
    }

    void ToggleMainThrusters()
    {
        _player.mainThrustersEnabled = !_player.mainThrustersEnabled;
        AddTimedMessage(_player.mainThrustersEnabled ? L"Main drive armed: forward/reverse thrust restored." : L"Main drive cut: ballistic coast, RCS still available for rotation/strafe.");
    }

    void ToggleRcsThrusters()
    {
        _player.rcsThrustersEnabled = !_player.rcsThrustersEnabled;
        AddTimedMessage(_player.rcsThrustersEnabled ? L"RCS armed: directional/turning thrusters restored." : L"RCS cut: no rotation or strafe burn, main drive unaffected.");
    }

    struct ThrusterPortLayout {
        float aftX = -74.0f;
        float foreX = 52.0f;
        float sideY = 28.0f;
        float mainY = 11.0f;
        float retroY = 9.0f;
        float foreRcsX = 34.0f;
        float aftRcsX = -42.0f;
    };

    ThrusterPortLayout BuildThrusterPortLayout() const
    {
        ThrusterPortLayout layout;
        if (_playerVisual.Empty()) {
            return layout;
        }

        bool foundBodyBounds = false;
        float minX = 0.0f;
        float maxX = 0.0f;
        float minY = 0.0f;
        float maxY = 0.0f;

        auto visit = [&](subspace::RuntimeVisualPoint2 point) {
            if (!foundBodyBounds) {
                minX = maxX = point.x;
                minY = maxY = point.y;
                foundBodyBounds = true;
                return;
            }
            minX = std::min(minX, point.x);
            maxX = std::max(maxX, point.x);
            minY = std::min(minY, point.y);
            maxY = std::max(maxY, point.y);
        };

        for (const auto& primitive : _playerVisual.primitives) {
            // Shield/contact rings are intentionally much larger than the hull and
            // must never move engine/RCS ports forward or outward.
            if (primitive.layer == subspace::RuntimeVisualLayer::Shield ||
                primitive.layer == subspace::RuntimeVisualLayer::Label ||
                primitive.semanticRole == "ShieldRadius") {
                continue;
            }

            if (!primitive.points.empty()) {
                for (const auto& point : primitive.points) {
                    visit(point);
                }
                continue;
            }

            const float halfW = std::max(0.0f, primitive.size.x * 0.5f);
            const float halfH = std::max(0.0f, primitive.size.y * 0.5f);
            visit({primitive.center.x - halfW, primitive.center.y - halfH});
            visit({primitive.center.x + halfW, primitive.center.y + halfH});
        }

        if (!foundBodyBounds) {
            return layout;
        }

        const float halfWidth = std::max(12.0f, std::max(std::abs(minY), std::abs(maxY)));
        const float length = std::max(64.0f, maxX - minX);

        // Local +X is the nose/forward direction.  Main-drive particles must
        // come from the actual rear/aft edge of the visible hull.  The previous
        // version used profile bounds that included the shield ring, which made
        // ports drift away from the ship silhouette.
        layout.aftX = minX - 6.0f;
        layout.foreX = maxX + 4.0f;
        layout.sideY = std::clamp(halfWidth + 3.0f, 18.0f, 44.0f);
        layout.mainY = std::clamp(halfWidth * 0.43f, 9.0f, 18.0f);
        layout.retroY = std::clamp(halfWidth * 0.32f, 6.0f, 13.0f);
        layout.foreRcsX = maxX - length * 0.28f;
        layout.aftRcsX = minX + length * 0.25f;

        if (_playerVisual.id.find("ulysses") != std::string::npos) {
            // Ulysses proxy has explicit aft engines and side RCS nodes.  Keep
            // the exhaust sockets visually attached to those features.
            layout.aftX = minX - 4.0f;
            layout.foreX = maxX + 3.0f;
            layout.mainY = 12.0f;
            layout.retroY = 8.0f;
            layout.foreRcsX = 30.0f;
            layout.aftRcsX = -58.0f;
            layout.sideY = 24.0f;
        }

        return layout;
    }

    void SpawnParticleAt(Vec2 position, float directionRadians, int count, std::uint32_t startColor, float speed = 120.0f, float spread = 0.34f, float lifetime = 0.42f, float size = 3.0f)
    {
        subspace::client::ParticleBurstRequest request;
        request.x = position.x;
        request.y = position.y;
        request.directionRadians = directionRadians;
        request.spreadRadians = spread;
        request.baseSpeed = speed;
        request.speedVariance = speed * 0.55f;
        request.lifetime = lifetime;
        request.size = size;
        request.count = count;
        request.inheritVx = _player.velocity.x * 0.22f;
        request.inheritVy = _player.velocity.y * 0.22f;
        request.startColor = startColor;
        request.endColor = 0x0a1421u;
        _clientParticles.Burst(request);
    }

    void SpawnParticleAtLocal(Vec2 localPosition, float directionRadians, int count, std::uint32_t startColor, float speed = 120.0f, float spread = 0.34f, float lifetime = 0.42f, float size = 3.0f)
    {
        SpawnParticleAt(_player.position + Rotate(localPosition, _player.angle), directionRadians, count, startColor, speed, spread, lifetime, size);
    }

    void SpawnThrusterParticles(bool main, bool reverse, bool strafeLeft, bool strafeRight, bool turnLeft, bool turnRight)
    {
        const ThrusterPortLayout ports = BuildThrusterPortLayout();
        const float aftDir = _player.angle + kPi;
        const float foreDir = _player.angle;
        const float portDir = _player.angle - kPi * 0.5f;
        const float starboardDir = _player.angle + kPi * 0.5f;

        if (main) {
            SpawnParticleAtLocal({ports.aftX, ports.mainY}, aftDir, 5, 0x66ccffu, 205.0f, 0.22f, 0.56f, 3.8f);
            SpawnParticleAtLocal({ports.aftX, -ports.mainY}, aftDir, 5, 0x66ccffu, 205.0f, 0.22f, 0.56f, 3.8f);
            SpawnParticleAtLocal({ports.aftX - 8.0f, 0.0f}, aftDir, 3, 0xffffffu, 155.0f, 0.18f, 0.34f, 2.4f);
        }
        if (reverse) {
            SpawnParticleAtLocal({ports.foreX, ports.retroY}, foreDir, 3, 0xffcc66u, 118.0f, 0.26f, 0.30f, 2.2f);
            SpawnParticleAtLocal({ports.foreX, -ports.retroY}, foreDir, 3, 0xffcc66u, 118.0f, 0.26f, 0.30f, 2.2f);
        }
        if (strafeLeft) {
            // Accelerating left means starboard-side ports exhaust to starboard.
            SpawnParticleAtLocal({ports.foreRcsX, ports.sideY}, starboardDir, 3, 0x8fffe0u, 124.0f, 0.22f, 0.28f, 2.0f);
            SpawnParticleAtLocal({ports.aftRcsX, ports.sideY}, starboardDir, 3, 0x8fffe0u, 112.0f, 0.22f, 0.25f, 1.8f);
        }
        if (strafeRight) {
            // Accelerating right means port-side ports exhaust to port.
            SpawnParticleAtLocal({ports.foreRcsX, -ports.sideY}, portDir, 3, 0x8fffe0u, 124.0f, 0.22f, 0.28f, 2.0f);
            SpawnParticleAtLocal({ports.aftRcsX, -ports.sideY}, portDir, 3, 0x8fffe0u, 112.0f, 0.22f, 0.25f, 1.8f);
        }
        if (turnLeft) {
            // Nose-left torque: starboard nose pushes left, port aft pushes right.
            SpawnParticleAtLocal({ports.foreRcsX, ports.sideY}, starboardDir, 3, 0xffe08au, 104.0f, 0.20f, 0.25f, 1.9f);
            SpawnParticleAtLocal({ports.aftRcsX, -ports.sideY}, portDir, 3, 0xffe08au, 104.0f, 0.20f, 0.25f, 1.9f);
        }
        if (turnRight) {
            // Nose-right torque: port nose pushes right, starboard aft pushes left.
            SpawnParticleAtLocal({ports.foreRcsX, -ports.sideY}, portDir, 3, 0xffe08au, 104.0f, 0.20f, 0.25f, 1.9f);
            SpawnParticleAtLocal({ports.aftRcsX, ports.sideY}, starboardDir, 3, 0xffe08au, 104.0f, 0.20f, 0.25f, 1.9f);
        }
    }

    void HandleActionEdges()
    {
        if (KeyPressed(VK_SPACE)) {
            MineSelectedAsteroid();
        }
        if (KeyPressed('F')) {
            DockOrSell();
        }
        if (KeyPressed('C')) {
            CollectNearbyCargo();
        }
    }

    void SelectNearestAsteroid()
    {
        float best = 999999.0f;
        int bestIndex = -1;
        for (int i = 0; i < static_cast<int>(_asteroids.size()); ++i) {
            if (!_asteroids[i].alive) {
                continue;
            }
            const float distance = Distance(_player.position, _asteroids[i].position);
            if (distance < best) {
                best = distance;
                bestIndex = i;
            }
        }
        _selectedAsteroid = bestIndex;
    }

    void MineSelectedAsteroid()
    {
        if (_laserTimer > 0.0f) {
            return;
        }
        _laserTimer = 0.24f;
        _laserFlash = 0.16f;

        if (_selectedAsteroid < 0 || _selectedAsteroid >= static_cast<int>(_asteroids.size())) {
            AddTimedMessage(L"No asteroid target selected.");
            return;
        }

        Asteroid& asteroid = _asteroids[_selectedAsteroid];
        if (!asteroid.alive) {
            SelectNearestAsteroid();
            return;
        }

        const float range = Distance(_player.position, asteroid.position);
        if (range > 310.0f) {
            AddTimedMessage(L"Target outside mining beam range.");
            return;
        }

        subspace::MiningToolProfile tool;
        tool.id = "dev-mining-laser";
        tool.baseDamage = 34.0f;
        tool.efficiency = _shipPartStats.miningEfficiency + _sectorSurvey.asteroidField.miningYieldMultiplier * 0.08f;
        tool.canSalvage = true;

        subspace::MiningTargetProfile target;
        target.id = "asteroid-" + std::to_string(_selectedAsteroid);
        target.resourceTag = asteroid.resourceTag;
        target.radius = asteroid.radius;
        target.remainingIntegrity = asteroid.hp;
        target.richness = asteroid.richness;

        const auto yield = subspace::GenerateMiningYield(_sectorSurvey.asteroidField, target, tool, asteroid.visualSeed + static_cast<std::uint32_t>(std::round(asteroid.hp)));
        asteroid.hp -= yield.damageApplied;
        AddTimedMessage(ToWide(yield.message));
        if (asteroid.hp <= 0.0f || yield.fractured) {
            asteroid.alive = false;
            int i = 0;
            for (const auto& item : yield.cargo) {
                CargoPod pod;
                pod.position = asteroid.position + Vec2{static_cast<float>((i - 1) * 22), static_cast<float>((i % 2) * 18)};
                pod.resourceTag = item.commodity;
                pod.units = item.units;
                pod.creditValue = item.creditValue;
                pod.cargo = item;
                pod.value = static_cast<float>(item.creditValue);
                pod.visual = _cargoVisual;
                _cargo.push_back(pod);
                ++i;
            }
            AddTimedMessage(L"Asteroid fractured: cargo pods released.");
            SelectNearestAsteroid();
        }
    }

    void CollectNearbyCargo()
    {
        int collected = 0;
        for (auto& pod : _cargo) {
            if (pod.alive && Distance(_player.position, pod.position) <= 86.0f) {
                pod.alive = false;
                _player.cargo += std::max(1, pod.units);
                _player.cargoValue += std::max(0, pod.creditValue);
                _cargoManifest.push_back(pod.cargo.commodity.empty() ? subspace::CargoYieldItem{pod.resourceTag, pod.units, pod.creditValue} : pod.cargo);
                ++collected;
            }
        }
        if (collected > 0) {
            AddTimedMessage(L"Collected " + std::to_wstring(collected) + L" cargo pod(s), manifest value " + std::to_wstring(_player.cargoValue) + L" credits.");
        }
        else {
            AddTimedMessage(L"No cargo in pickup range. Use C near green pods.");
        }
    }

    void DockOrSell()
    {
        if (Distance(_player.position, _station) > 175.0f) {
            AddTimedMessage(L"Not in docking range of station.");
            return;
        }
        const auto quote = subspace::QuoteCargoSale(_stationEconomy, _cargoManifest);
        const int sale = quote.totalCredits > 0 ? quote.totalCredits : _player.cargoValue;
        _player.credits += sale;
        for (const auto& item : _cargoManifest) {
            subspace::AddHomeInventory(_director.save.factory, item.commodity, std::max(1, item.units));
        }
        _player.cargo = 0;
        _player.cargoValue = 0;
        _cargoManifest.clear();
        _player.hull = 100.0f;
        RefreshHomeView();
        AddTimedMessage(L"Docked: sold cargo, stocked home factory, repaired hull. Credits +" + std::to_wstring(sale));
    }

    void AddTimedMessage(const std::wstring& message)
    {
        _timedMessage = message;
        _messageTimer = 2.4f;
        AddConsoleLine(message);
    }

    void AddConsoleLine(const std::wstring& line)
    {
        _consoleLines.push_back(line);
        if (_consoleLines.size() > 12) {
            _consoleLines.erase(_consoleLines.begin());
        }
    }


    subspace::ShipRailTravelFit BuildShipRailTravelFit() const
    {
        subspace::ShipRailTravelFit fit;
        fit.driveTier = std::max(1, _shipPartStats.powerDraw <= 0 ? 1 : 2);
        fit.defenseRating = static_cast<int>(std::round(_shipPartStats.shieldCapacity / 8.0f));
        fit.scannerRating = _shipPartStats.scannerRange > 1.0f ? 3 : 1;
        fit.cargoCapacity = std::max(1, _shipPartStats.cargoCapacity);
        fit.fuelAvailable = _player.fuel;
        fit.fuelCapacity = _shipPartStats.fuelCapacity;
        fit.mass = std::max(1.0f, static_cast<float>(_shipPartStats.mass));
        fit.thrustRating = std::max(1.0f, _shipPartStats.thrust / 120.0f);
        fit.hasRailDrive = true;
        return fit;
    }

    void RefreshRailTravelRoutes()
    {
        _railRoutes = subspace::CreateStarterRailTravelRoutes("HOME-SOL", "EXP-" + std::to_string(100 + _sectorIndex), _sectorSeed == 0 ? 0x5151u : _sectorSeed);
        if (_railRoutes.empty()) {
            _selectedRailRoute = -1;
        }
        else if (_selectedRailRoute < 0 || _selectedRailRoute >= static_cast<int>(_railRoutes.size())) {
            _selectedRailRoute = 0;
        }
    }

    void CycleRailTravelRoute(int delta)
    {
        RefreshRailTravelRoutes();
        if (_railRoutes.empty()) {
            AddTimedMessage(L"No rail travel routes available.");
            return;
        }
        const int count = static_cast<int>(_railRoutes.size());
        _selectedRailRoute = (_selectedRailRoute + delta + count) % count;
        const auto& route = _railRoutes[static_cast<std::size_t>(_selectedRailRoute)];
        AddTimedMessage(L"Selected travel route: " + ToWide(subspace::RailTravelRouteSummary(route)));
    }

    void StartSelectedRailTravel()
    {
        RefreshRailTravelRoutes();
        if (_railRoutes.empty() || _selectedRailRoute < 0) {
            AddTimedMessage(L"No rail travel route selected.");
            return;
        }
        const auto& route = _railRoutes[static_cast<std::size_t>(_selectedRailRoute)];
        const auto fit = BuildShipRailTravelFit();
        const auto report = subspace::EvaluateRailTravelFit(route, fit);
        if (!report.canLaunch) {
            AddTimedMessage(L"Rail launch blocked: " + ToWide(subspace::RailTravelFitSummary(report)));
            return;
        }
        _railTravel = subspace::StartRailTravel(route, fit);
        _player.fuel = _railTravel.fuelRemaining;
        AddTimedMessage(L"Rail travel started: " + ToWide(route.displayName));
    }

    void TickRailTravel(float dt)
    {
        if (_railTravel.state != subspace::RailTravelState::Traveling) {
            return;
        }
        subspace::TickRailTravel(_railTravel, dt);
        _player.fuel = std::min(_player.fuel, _railTravel.fuelRemaining);
        if (_railTravel.state == subspace::RailTravelState::Completed) {
            _player.fuel = _railTravel.fuelRemaining;
            _player.cargo = std::min(_shipPartStats.cargoCapacity, _player.cargo + static_cast<int>(std::round(_railTravel.cargoCollected)));
            _player.cargoValue += static_cast<int>(std::round(_railTravel.cargoCollected * 8.0f + _railTravel.salvageCollected * 12.0f));
            ++_sectorIndex;
            BuildSectorSystem();
            ResetWorld();
            AddTimedMessage(L"Rail travel complete: arrived at " + ToWide(_sectorId));
        }
        else if (_railTravel.state == subspace::RailTravelState::Failed) {
            AddTimedMessage(L"Rail travel failed. Emergency systems recovered partial data.");
        }
    }

    void ExecuteConsoleInput()
    {
        std::wstring input = _consoleInput;
        _consoleInput.clear();
        if (input.empty()) {
            return;
        }
        AddConsoleLine(L"> " + input);

        const std::string narrow = ToNarrow(input);
        if (narrow == "help") {
            AddConsoleLine(L"Commands: help, reset, sector next, sector info, scanner, resources, economy, encounter, cargo");
            AddConsoleLine(L"Home: home, home build, home place, home remove, runs, run next, run launch, run extract, home inventory, shipyard");
            AddConsoleLine(L"Travel: travel routes, travel next, travel start, travel status, travel abort.");
            AddConsoleLine(L"Ship: ship parts, ship stats, main cutoff, rcs cutoff, M cycle part, O install at home.");
            AddConsoleLine(L"Also: bodies, body nearest, body scan, spawn asteroid, spawn cargo, give credits <n>");
            AddConsoleLine(L"Runtime commands also work: entity.inspect, asset.reload, ship.validate, dev.undo, dev.redo.");
        }
        else if (narrow == "reset") {
            ResetWorld();
            AddConsoleLine(L"World reset without changing sector seed.");
        }
        else if (narrow == "sector next") {
            AdvanceSectorAndReset();
        }
        else if (narrow == "sector info") {
            AddConsoleLine(ToWide(subspace::StarSystemSummary(_sectorSystem)));
            AddConsoleLine(L"Bodies: " + std::to_wstring(_sectorSystem.bodies.size()) + L"  Seed: " + std::to_wstring(_sectorSeed));
        }
        else if (narrow == "scanner") {
            for (const auto& line : subspace::FormatScannerReportLines(_sectorScannerReport, 8)) {
                AddConsoleLine(ToWide(line));
            }
        }
        else if (narrow == "resources") {
            AddConsoleLine(ToWide(subspace::SectorResourceSummary(_sectorSurvey)));
        }
        else if (narrow == "economy") {
            AddConsoleLine(ToWide(subspace::StationEconomySummary(_stationEconomy)));
            for (const auto& price : _stationEconomy.commodityPrices) {
                AddConsoleLine(ToWide(price.commodity + " base=" + std::to_string(price.basePrice) + " mult=" + std::to_string(price.multiplier)));
            }
        }
        else if (narrow == "encounter") {
            subspace::ProceduralEncounterGenerator generator;
            const auto encounter = generator.GenerateEncounter(_encounterTable, 1 + _player.credits / 400, _sectorSeed + static_cast<std::uint32_t>(_consoleLines.size()));
            AddConsoleLine(ToWide(subspace::GeneratedEncounterSummary(encounter)));
        }
        else if (narrow == "cargo") {
            AddConsoleLine(L"Cargo units=" + std::to_wstring(_player.cargo) + L" value=" + std::to_wstring(_player.cargoValue) + L" manifest=" + std::to_wstring(_cargoManifest.size()));
        }
        else if (narrow == "home") {
            _homeViewOpen = true;
            RefreshHomeView();
            AddConsoleLine(ToWide(subspace::HomeClientViewSummary(_homeView)));
        }
        else if (narrow == "home inventory") {
            RefreshHomeView();
            AddConsoleLine(ToWide(_homeView.inventoryText));
        }
        else if (narrow == "home build") {
            _homeViewOpen = true;
            _homeBuildMode = true;
            ClampHomeBuildCursor();
            AddConsoleLine(L"Home build mode enabled. U/I zone, 1-9 palette, arrows cursor, P place, Del remove.");
        }
        else if (narrow == "home place") {
            PlaceSelectedHomeStructure();
            AddConsoleLine(ToWide(_lastHomeBuildMessage));
        }
        else if (narrow == "home remove") {
            RemoveSelectedHomeStructure();
            AddConsoleLine(ToWide(_lastHomeBuildMessage));
        }
        else if (narrow == "shipyard") {
            RefreshHomeView();
            AddConsoleLine(ToWide(_homeView.shipyardText));
        }
        else if (narrow == "runs") {
            RefreshHomeView();
            for (const auto& offer : _homeView.runOffers) {
                AddConsoleLine(std::to_wstring(offer.index + 1) + L": " + ToWide(offer.summary));
            }
        }
        else if (narrow == "run next") {
            CycleRunOffer(1);
        }
        else if (narrow == "run launch") {
            LaunchSelectedRun();
        }
        else if (narrow == "run extract") {
            ExtractCurrentRun();
        }
        else if (narrow == "travel routes") {
            RefreshRailTravelRoutes();
            for (int i = 0; i < static_cast<int>(_railRoutes.size()); ++i) {
                const auto& route = _railRoutes[static_cast<std::size_t>(i)];
                const auto report = subspace::EvaluateRailTravelFit(route, BuildShipRailTravelFit());
                AddConsoleLine(std::to_wstring(i + 1) + L": " + ToWide(subspace::RailTravelRouteSummary(route)));
                AddConsoleLine(L"   " + ToWide(subspace::RailTravelFitSummary(report)));
            }
        }
        else if (narrow == "travel next") {
            CycleRailTravelRoute(1);
        }
        else if (narrow == "travel start") {
            StartSelectedRailTravel();
        }
        else if (narrow == "travel status") {
            AddConsoleLine(ToWide(subspace::RailTravelStateSummary(_railTravel)));
        }
        else if (narrow == "travel abort") {
            subspace::AbortRailTravel(_railTravel);
            AddConsoleLine(ToWide(subspace::RailTravelStateSummary(_railTravel)));
        }
        else if (narrow == "bodies") {
            for (std::size_t i = 0; i < _sectorBackdrop.size() && i < 8; ++i) {
                const auto& body = _sectorBackdrop[i];
                AddConsoleLine(std::to_wstring(i) + L": " + ToWide(body.definition.displayName) + L"  " +
                               ToWide(subspace::CelestialBodyTypeName(body.definition.type)) + L"  dist=" +
                               FormatFloat(Distance(_player.position, body.position), 0));
            }
        }
        else if (narrow == "body nearest") {
            SelectNearestCelestial();
        }
        else if (narrow == "body scan" || narrow == "celestial scan") {
            ScanSelectedCelestial();
        }
        else if (narrow == "spawn asteroid") {
            Asteroid asteroid;
            Vec2 forward{std::cos(_player.angle), std::sin(_player.angle)};
            asteroid.position = _player.position + forward * 260.0f;
            asteroid.radius = 42.0f;
            asteroid.hp = 100.0f;
            asteroid.rotation = _player.angle;
            asteroid.visualSeed = static_cast<std::uint32_t>(_sectorSeed + 9001u + static_cast<std::uint32_t>(_asteroids.size() * 131));
            asteroid.resourceTag = subspace::PickPrimaryResourceTag(_sectorSurvey, asteroid.visualSeed, static_cast<int>(_asteroids.size()));
            asteroid.richness = std::max(1, _sectorSurvey.asteroidField.resourceRichness);
            asteroid.visual = subspace::BuildAsteroidVisualProfile(asteroid.radius, asteroid.visualSeed);
            _asteroids.push_back(asteroid);
            SelectNearestAsteroid();
            AddConsoleLine(L"Spawned asteroid ahead of ship.");
        }
        else if (narrow == "spawn cargo") {
            Vec2 forward{std::cos(_player.angle), std::sin(_player.angle)};
            CargoPod pod;
            pod.position = _player.position + forward * 90.0f;
            pod.resourceTag = _sectorSurvey.asteroidField.dominantResource;
            pod.units = 1;
            pod.creditValue = 25;
            pod.cargo = {pod.resourceTag, pod.units, pod.creditValue};
            pod.value = 25.0f;
            pod.visual = _cargoVisual;
            _cargo.push_back(pod);
            AddConsoleLine(L"Spawned cargo pod.");
        }
        else if (narrow.rfind("give credits ", 0) == 0) {
            const int amount = std::max(0, std::atoi(narrow.substr(13).c_str()));
            _player.credits += amount;
            AddConsoleLine(L"Credits added: " + std::to_wstring(amount));
        }
        else if (narrow == "thrust cutoff" || narrow == "main cutoff") {
            ToggleMainThrusters();
        }
        else if (narrow == "rcs cutoff") {
            ToggleRcsThrusters();
        }
        else if (narrow == "ship parts") {
            AddConsoleLine(L"Selected part: " + CurrentShipPartText());
            AddConsoleLine(ToWide(subspace::ShipLoadoutSummary(_shipLoadout, _shipPartCatalog)));
        }
        else if (narrow == "ship stats") {
            AddConsoleLine(L"Thrust=" + FormatFloat(_shipPartStats.thrust, 0) + L" Turn=" + FormatFloat(_shipPartStats.turnSpeed, 2) + L" FuelBurn=" + FormatFloat(_shipPartStats.fuelBurnPerSecond, 2));
            AddConsoleLine(L"CargoCap=" + std::to_wstring(_shipPartStats.cargoCapacity) + L" Mass=" + std::to_wstring(_shipPartStats.mass) + L" Power=" + std::to_wstring(_shipPartStats.powerDraw));
        }
        else if (narrow == "ship install") {
            InstallSelectedShipPart();
        }
        else {
            ExecuteRuntimeCommand(input);
        }
    }

    void ExecuteRuntimeCommand(const std::wstring& command)
    {
        auto result = _runtime.ExecuteDeveloperCommandLine(ToNarrow(command));
        _devMode = _runtime.GetDeveloperMode().IsEnabled();
        std::wstring message = ToWide(result.message.empty() ? result.result.message : result.message);
        if (message.empty()) {
            message = result.success ? L"Command succeeded." : L"Command failed or was ignored.";
        }
        AddConsoleLine(message);
        _lastRuntimeCommand = command;
        _lastRuntimeCommandResult = message;
    }

    POINT WorldToScreen(Vec2 world) const
    {
        const float sx = (world.x - _player.position.x) * kZoom + static_cast<float>(_width) * 0.5f;
        const float sy = (world.y - _player.position.y) * kZoom + static_cast<float>(_height) * 0.5f;
        return {static_cast<LONG>(std::round(sx)), static_cast<LONG>(std::round(sy))};
    }

    Vec2 ParallaxOrigin(Vec2 world, float parallax) const
    {
        return {world.x + _player.position.x * (1.0f - parallax),
                world.y + _player.position.y * (1.0f - parallax)};
    }

    void DrawTextLine(HDC dc, int x, int y, const std::wstring& text, COLORREF color) const
    {
        SetTextColor(dc, color);
        TextOutW(dc, x, y, text.c_str(), static_cast<int>(text.size()));
    }

    void DrawPolishRing(HDC dc, POINT center, int radius, COLORREF color, int style = PS_DOT)
    {
        if (radius <= 0) {
            return;
        }
        HPEN pen = CreatePen(style, 1, color);
        HGDIOBJ oldPen = SelectObject(dc, pen);
        HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
        Ellipse(dc, center.x - radius, center.y - radius, center.x + radius, center.y + radius);
        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        DeleteObject(pen);
    }

    float CelestialClipRadius(const subspace::RuntimeVisualProfile& profile) const
    {
        float radius = 0.0f;
        for (const auto& primitive : profile.primitives) {
            if (primitive.kind == subspace::RuntimeVisualPrimitiveKind::Circle &&
                primitive.layer == subspace::RuntimeVisualLayer::Body) {
                radius = std::max(radius, primitive.radius);
            }
        }
        if (radius <= 0.0f) {
            radius = std::max(std::fabs(profile.boundsMax.x), std::fabs(profile.boundsMax.y)) * 0.34f;
        }
        return std::max(4.0f, radius);
    }

    void DrawStarfield(HDC dc)
    {
        // Pass121: layered deep-space readability pass.  These remain tiny GDI
        // primitives, but they now read as depth, dust and distant machinery
        // instead of one flat set of white pixels.
        for (const auto& star : _stars) {
            POINT p = WorldToScreen(star.position * 0.35f);
            if (p.x < 0 || p.x >= _width || p.y < 0 || p.y >= _height) {
                continue;
            }
            const int blueBias = std::min(255, star.brightness + 18);
            SetPixel(dc, p.x, p.y, RGB(star.brightness, star.brightness, blueBias));
            if (star.brightness > 205) {
                const int px = static_cast<int>(p.x);
                const int py = static_cast<int>(p.y);
                const int leftPixel = std::max(0, px - 1);
                const int rightPixel = std::min(_width - 1, px + 1);
                SetPixel(dc, leftPixel, py, RGB(96, 118, 150));
                SetPixel(dc, rightPixel, py, RGB(96, 118, 150));
            }
        }

        HPEN dustPen = CreatePen(PS_DOT, 1, RGB(18, 27, 42));
        HGDIOBJ oldPen = SelectObject(dc, dustPen);
        for (int i = 0; i < 18; ++i) {
            const int cx = static_cast<int>(SeededUnit(0xDEAD1100u, i) * static_cast<float>(_width));
            const int cy = static_cast<int>(SeededUnit(0xBEEF4400u, i) * static_cast<float>(_height));
            const int rx = 90 + static_cast<int>(SeededUnit(0xA551u, i) * 190.0f);
            const int ry = 24 + static_cast<int>(SeededUnit(0xA552u, i) * 70.0f);
            Arc(dc, cx - rx, cy - ry, cx + rx, cy + ry, cx - rx, cy, cx + rx, cy);
        }
        SelectObject(dc, oldPen);
        DeleteObject(dustPen);
    }

    void DrawSectorBackdrop(HDC dc)
    {
        if (_sectorBackdrop.empty()) {
            return;
        }

        const Vec2 primary = _sectorBackdrop.front().position;
        const POINT primaryScreen = WorldToScreen(primary);

        HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
        HGDIOBJ oldPen = SelectObject(dc, _draw.orbitPen);
        for (std::size_t i = 1; i < _sectorBackdrop.size(); ++i) {
            const auto& body = _sectorBackdrop[i];
            const int radius = std::max(12, static_cast<int>(std::round(body.orbitRadius * kZoom)));
            Ellipse(dc, primaryScreen.x - radius, primaryScreen.y - radius,
                    primaryScreen.x + radius, primaryScreen.y + radius);
        }
        SelectObject(dc, oldPen);
        SelectObject(dc, oldBrush);

        for (std::size_t i = 0; i < _sectorBackdrop.size(); ++i) {
            const auto& body = _sectorBackdrop[i];
            DrawRuntimeVisualProfile(dc, body.visual, body.position, body.rotation);
            if (static_cast<int>(i) == _selectedCelestial) {
                POINT p = WorldToScreen(body.position);
                const int radius = std::max(12, static_cast<int>(std::round((body.definition.visualRadius + 16.0f) * kZoom)));
                HGDIOBJ ringBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
                HGDIOBJ ringPen = SelectObject(dc, _draw.targetPen);
                Ellipse(dc, p.x - radius, p.y - radius, p.x + radius, p.y + radius);
                SelectObject(dc, ringPen);
                SelectObject(dc, ringBrush);
            }
        }
    }

    void DrawSectorLighting(HDC dc)
    {
        if (_sectorBackdrop.empty()) {
            return;
        }
        const auto& primary = _sectorBackdrop.front();
        if (primary.definition.type != subspace::CelestialBodyType::Star) {
            return;
        }
        const POINT sun = WorldToScreen(primary.position);
        const float intensity = SolarLightIntensity();
        const int warm = std::max(48, std::min(210, static_cast<int>(intensity * 180.0f)));
        DrawPolishRing(dc, sun, 42, RGB(warm, warm - 18, 56), PS_DOT);
        DrawPolishRing(dc, sun, 92, RGB(std::max(32, warm - 54), std::max(24, warm - 70), 46), PS_DOT);
        DrawPolishRing(dc, sun, 156, RGB(54, 43, 32), PS_DOT);

        HPEN lightPen = CreatePen(PS_DOT, 1, RGB(warm, std::max(48, warm - 32), 62));
        HGDIOBJ oldPen = SelectObject(dc, lightPen);
        POINT ship = WorldToScreen(_player.position);
        MoveToEx(dc, sun.x, sun.y, nullptr);
        LineTo(dc, ship.x, ship.y);
        for (int i = 0; i < 8; ++i) {
            const float angle = static_cast<float>(i) * kPi * 0.25f + _sectorElapsedSeconds * 0.04f;
            const int length = 150 + (i % 3) * 55;
            MoveToEx(dc, sun.x, sun.y, nullptr);
            LineTo(dc, sun.x + static_cast<int>(std::cos(angle) * static_cast<float>(length)),
                   sun.y + static_cast<int>(std::sin(angle) * static_cast<float>(length)));
        }
        SelectObject(dc, oldPen);
        DeleteObject(lightPen);
    }

    void DrawGrid(HDC dc)
    {
        SelectObject(dc, _draw.gridPen);
        constexpr float spacing = 100.0f;
        const float worldLeft = _player.position.x - static_cast<float>(_width) / kZoom * 0.5f;
        const float worldRight = _player.position.x + static_cast<float>(_width) / kZoom * 0.5f;
        const float worldTop = _player.position.y - static_cast<float>(_height) / kZoom * 0.5f;
        const float worldBottom = _player.position.y + static_cast<float>(_height) / kZoom * 0.5f;

        for (float x = std::floor(worldLeft / spacing) * spacing; x <= worldRight; x += spacing) {
            POINT a = WorldToScreen({x, worldTop});
            POINT b = WorldToScreen({x, worldBottom});
            MoveToEx(dc, a.x, a.y, nullptr);
            LineTo(dc, b.x, b.y);
        }
        for (float y = std::floor(worldTop / spacing) * spacing; y <= worldBottom; y += spacing) {
            POINT a = WorldToScreen({worldLeft, y});
            POINT b = WorldToScreen({worldRight, y});
            MoveToEx(dc, a.x, a.y, nullptr);
            LineTo(dc, b.x, b.y);
        }
    }

    static COLORREF ToColorRef(std::uint32_t rgb)
    {
        return RGB((rgb >> 16) & 0xffu, (rgb >> 8) & 0xffu, rgb & 0xffu);
    }

    POINT VisualLocalToScreen(Vec2 origin, float radians, subspace::RuntimeVisualPoint2 local) const
    {
        Vec2 rotated = Rotate({local.x, local.y}, radians);
        return WorldToScreen(origin + rotated);
    }

    void DrawRuntimePrimitive(HDC dc, const subspace::RuntimeVisualPrimitive& primitive, Vec2 origin, float radians)
    {
        HPEN pen = CreatePen(PS_SOLID, 1, ToColorRef(primitive.strokeColor));
        HBRUSH brush = primitive.filled ? CreateSolidBrush(ToColorRef(primitive.fillColor)) : static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HGDIOBJ oldPen = SelectObject(dc, pen);
        HGDIOBJ oldBrush = SelectObject(dc, brush);

        if (primitive.kind == subspace::RuntimeVisualPrimitiveKind::Polygon) {
            std::vector<POINT> points;
            points.reserve(primitive.points.size());
            for (const auto& point : primitive.points) {
                points.push_back(VisualLocalToScreen(origin, radians, point));
            }
            if (points.size() >= 3) {
                Polygon(dc, points.data(), static_cast<int>(points.size()));
            }
        }
        else if (primitive.kind == subspace::RuntimeVisualPrimitiveKind::Box) {
            const Vec2 center{primitive.center.x, primitive.center.y};
            const float halfW = primitive.size.x * 0.5f;
            const float halfH = primitive.size.y * 0.5f;
            subspace::RuntimeVisualPoint2 corners[4] = {
                {center.x - halfW, center.y - halfH},
                {center.x + halfW, center.y - halfH},
                {center.x + halfW, center.y + halfH},
                {center.x - halfW, center.y + halfH}};
            POINT points[4] = {
                VisualLocalToScreen(origin, radians, corners[0]),
                VisualLocalToScreen(origin, radians, corners[1]),
                VisualLocalToScreen(origin, radians, corners[2]),
                VisualLocalToScreen(origin, radians, corners[3])};
            Polygon(dc, points, 4);
        }
        else if (primitive.kind == subspace::RuntimeVisualPrimitiveKind::Diamond) {
            const Vec2 center{primitive.center.x, primitive.center.y};
            const float halfW = primitive.size.x * 0.5f;
            const float halfH = primitive.size.y * 0.5f;
            POINT points[4] = {
                VisualLocalToScreen(origin, radians, {center.x, center.y - halfH}),
                VisualLocalToScreen(origin, radians, {center.x + halfW, center.y}),
                VisualLocalToScreen(origin, radians, {center.x, center.y + halfH}),
                VisualLocalToScreen(origin, radians, {center.x - halfW, center.y})};
            Polygon(dc, points, 4);
        }
        else if (primitive.kind == subspace::RuntimeVisualPrimitiveKind::Circle ||
                 primitive.kind == subspace::RuntimeVisualPrimitiveKind::Ring) {
            const POINT center = VisualLocalToScreen(origin, radians, primitive.center);
            int halfWidth = std::max(1, static_cast<int>(std::round(primitive.radius * kZoom)));
            int halfHeight = halfWidth;
            if (primitive.kind == subspace::RuntimeVisualPrimitiveKind::Ring && primitive.size.x > 1.0f && primitive.size.y > 1.0f) {
                halfWidth = std::max(1, static_cast<int>(std::round(primitive.size.x * 0.5f * kZoom)));
                halfHeight = std::max(1, static_cast<int>(std::round(primitive.size.y * 0.5f * kZoom)));
            }
            if (primitive.kind == subspace::RuntimeVisualPrimitiveKind::Ring) {
                SelectObject(dc, GetStockObject(NULL_BRUSH));
            }
            Ellipse(dc, center.x - halfWidth, center.y - halfHeight, center.x + halfWidth, center.y + halfHeight);
        }

        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        if (primitive.filled) {
            DeleteObject(brush);
        }
        DeleteObject(pen);
    }

    void DrawRuntimeVisualProfile(HDC dc, const subspace::RuntimeVisualProfile& profile, Vec2 origin, float radians)
    {
        const bool celestial = profile.entityType == subspace::RuntimeVisualEntityType::CelestialBody;
        const float celestialClipRadius = celestial ? CelestialClipRadius(profile) : 0.0f;
        for (const auto& layer : {subspace::RuntimeVisualLayer::Shadow,
                                  subspace::RuntimeVisualLayer::Body,
                                  subspace::RuntimeVisualLayer::Accent,
                                  subspace::RuntimeVisualLayer::Engine,
                                  subspace::RuntimeVisualLayer::Hardpoint,
                                  subspace::RuntimeVisualLayer::Detail,
                                  subspace::RuntimeVisualLayer::Shield,
                                  subspace::RuntimeVisualLayer::Label}) {
            for (const auto& primitive : profile.primitives) {
                if (primitive.layer != layer) {
                    continue;
                }

                const bool clipToDisc = celestial &&
                    (layer == subspace::RuntimeVisualLayer::Body ||
                     layer == subspace::RuntimeVisualLayer::Accent ||
                     layer == subspace::RuntimeVisualLayer::Detail) &&
                    primitive.kind != subspace::RuntimeVisualPrimitiveKind::Ring;
                if (!clipToDisc) {
                    DrawRuntimePrimitive(dc, primitive, origin, radians);
                    continue;
                }

                const POINT center = WorldToScreen(origin);
                const int radius = std::max(2, static_cast<int>(std::round(celestialClipRadius * kZoom)));
                HRGN clip = CreateEllipticRgn(center.x - radius, center.y - radius, center.x + radius, center.y + radius);
                const int saved = SaveDC(dc);
                SelectClipRgn(dc, clip);
                DrawRuntimePrimitive(dc, primitive, origin, radians);
                RestoreDC(dc, saved);
                DeleteObject(clip);
            }
        }
    }

    void DrawWorld(HDC dc)
    {
        DrawStation(dc);
        DrawAsteroids(dc);
        DrawCargo(dc);
        DrawLaser(dc);
        DrawClientParticles(dc);
        DrawPlayer(dc);
    }

    void DrawStation(HDC dc)
    {
        POINT p = WorldToScreen(_station);
        DrawRuntimeVisualProfile(dc, _stationVisual, _station, 0.0f);
        DrawTextLine(dc, p.x - 28, p.y - 8, L"DOCK", RGB(206, 225, 255));
    }

    std::vector<POINT> BuildAsteroidPolygon(const Asteroid& asteroid, POINT center, int radius) const
    {
        std::vector<POINT> points;
        points.reserve(12);
        for (int i = 0; i < 12; ++i) {
            const float angle = asteroid.rotation + (static_cast<float>(i) / 12.0f) * kPi * 2.0f;
            const float wobble = 0.78f + SeededUnit(asteroid.visualSeed, i) * 0.38f;
            const float localRadius = static_cast<float>(radius) * wobble;
            points.push_back({
                center.x + static_cast<LONG>(std::round(std::cos(angle) * localRadius)),
                center.y + static_cast<LONG>(std::round(std::sin(angle) * localRadius))});
        }
        return points;
    }

    void DrawAsteroids(HDC dc)
    {
        for (int i = 0; i < static_cast<int>(_asteroids.size()); ++i) {
            const auto& asteroid = _asteroids[i];
            if (!asteroid.alive) {
                continue;
            }
            POINT p = WorldToScreen(asteroid.position);
            const int radius = static_cast<int>(asteroid.radius * kZoom);
            DrawRuntimeVisualProfile(dc, asteroid.visual, asteroid.position, asteroid.rotation);

            HPEN veinPen = CreatePen(PS_DOT, 1, RGB(126, 112, 82));
            HGDIOBJ oldPen = SelectObject(dc, veinPen);
            for (int vein = 0; vein < 3; ++vein) {
                const float angle = asteroid.rotation + SeededUnit(asteroid.visualSeed, vein + 31) * kPi * 2.0f;
                const int ax = p.x + static_cast<int>(std::cos(angle) * radius * 0.20f);
                const int ay = p.y + static_cast<int>(std::sin(angle) * radius * 0.20f);
                const int bx = p.x + static_cast<int>(std::cos(angle + 0.75f) * radius * 0.55f);
                const int by = p.y + static_cast<int>(std::sin(angle + 0.75f) * radius * 0.55f);
                MoveToEx(dc, ax, ay, nullptr);
                LineTo(dc, bx, by);
            }
            SelectObject(dc, oldPen);
            DeleteObject(veinPen);

            if (i == _selectedAsteroid) {
                HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
                SelectObject(dc, _draw.targetPen);
                Arc(dc, p.x - radius - 10, p.y - radius - 10, p.x + radius + 10, p.y + radius + 10,
                    p.x - radius, p.y, p.x + radius, p.y);
                DrawPolishRing(dc, p, radius + 18, RGB(255, 216, 88), PS_DOT);
                SelectObject(dc, oldBrush);
            }
        }
    }

    void DrawCargo(HDC dc)
    {
        for (const auto& pod : _cargo) {
            if (!pod.alive) {
                continue;
            }
            POINT p = WorldToScreen(pod.position);
            DrawPolishRing(dc, p, 10, RGB(70, 180, 112), PS_DOT);
            DrawRuntimeVisualProfile(dc, pod.visual.Empty() ? _cargoVisual : pod.visual, pod.position, 0.0f);
        }
    }

    void DrawLaser(HDC dc)
    {
        if (_laserFlash <= 0.0f) {
            return;
        }
        _laserFlash = std::max(0.0f, _laserFlash - 0.03f);
        if (_selectedAsteroid < 0 || _selectedAsteroid >= static_cast<int>(_asteroids.size())) {
            return;
        }
        const auto& asteroid = _asteroids[_selectedAsteroid];
        if (!asteroid.alive || Distance(_player.position, asteroid.position) > 340.0f) {
            return;
        }
        POINT a = WorldToScreen(_player.position);
        POINT b = WorldToScreen(asteroid.position);
        HPEN beamGlow = CreatePen(PS_SOLID, 5, RGB(118, 54, 34));
        HGDIOBJ oldPen = SelectObject(dc, beamGlow);
        MoveToEx(dc, a.x, a.y, nullptr);
        LineTo(dc, b.x, b.y);
        SelectObject(dc, _draw.miningPen);
        MoveToEx(dc, a.x, a.y, nullptr);
        LineTo(dc, b.x, b.y);
        HPEN beamCore = CreatePen(PS_SOLID, 1, RGB(255, 238, 172));
        SelectObject(dc, beamCore);
        MoveToEx(dc, a.x, a.y, nullptr);
        LineTo(dc, b.x, b.y);
        DrawPolishRing(dc, b, 9 + static_cast<int>(_laserFlash * 8.0f), RGB(255, 172, 92), PS_DOT);
        SelectObject(dc, oldPen);
        DeleteObject(beamCore);
        DeleteObject(beamGlow);
    }

    POINT ShipLocalToScreen(Vec2 local) const
    {
        const Vec2 rotated = Rotate(local, _player.angle);
        return WorldToScreen(_player.position + rotated);
    }

    void DrawClientParticles(HDC dc)
    {
        for (const auto& particle : _clientParticles.Particles()) {
            const float t = particle.lifetime <= 0.0f ? 1.0f : std::min(1.0f, particle.age / particle.lifetime);
            const std::uint32_t rgb = subspace::client::LerpRgb(particle.startColor, particle.endColor, t);
            HBRUSH brush = CreateSolidBrush(ToColorRef(rgb));
            HGDIOBJ oldBrush = SelectObject(dc, brush);
            HGDIOBJ oldPen = SelectObject(dc, GetStockObject(NULL_PEN));
            const POINT p = WorldToScreen({particle.x, particle.y});
            const int radius = std::max(1, static_cast<int>(std::round(particle.size * (1.0f - t * 0.35f) * kZoom)));
            Ellipse(dc, p.x - radius, p.y - radius, p.x + radius, p.y + radius);
            SelectObject(dc, oldPen);
            SelectObject(dc, oldBrush);
            DeleteObject(brush);
        }
    }

    void DrawThrusterGlows(HDC dc)
    {
        const bool any = _lastFlightControl.mainBurning || _lastFlightControl.retroBurning ||
                         _lastFlightControl.leftRcsBurning || _lastFlightControl.rightRcsBurning ||
                         _lastFlightControl.strafeRcsBurning;
        if (!any) {
            return;
        }
        const ThrusterPortLayout ports = BuildThrusterPortLayout();
        auto drawGlow = [&](Vec2 local, int radius, COLORREF color) {
            const POINT p = WorldToScreen(_player.position + Rotate(local, _player.angle));
            HBRUSH brush = CreateSolidBrush(color);
            HGDIOBJ oldBrush = SelectObject(dc, brush);
            HGDIOBJ oldPen = SelectObject(dc, GetStockObject(NULL_PEN));
            Ellipse(dc, p.x - radius, p.y - radius, p.x + radius, p.y + radius);
            SelectObject(dc, oldPen);
            SelectObject(dc, oldBrush);
            DeleteObject(brush);
        };
        if (_lastFlightControl.mainBurning) {
            drawGlow({ports.aftX, ports.mainY}, 5, RGB(122, 220, 255));
            drawGlow({ports.aftX, -ports.mainY}, 5, RGB(122, 220, 255));
        }
        if (_lastFlightControl.retroBurning) {
            drawGlow({ports.foreX, ports.retroY}, 3, RGB(255, 204, 96));
            drawGlow({ports.foreX, -ports.retroY}, 3, RGB(255, 204, 96));
        }
        if (_lastFlightInput.strafe < -0.01f) {
            drawGlow({ports.foreRcsX, ports.sideY}, 3, RGB(143, 255, 224));
            drawGlow({ports.aftRcsX, ports.sideY}, 3, RGB(143, 255, 224));
        }
        if (_lastFlightInput.strafe > 0.01f) {
            drawGlow({ports.foreRcsX, -ports.sideY}, 3, RGB(143, 255, 224));
            drawGlow({ports.aftRcsX, -ports.sideY}, 3, RGB(143, 255, 224));
        }
        if (_lastFlightInput.turn < -0.01f) {
            drawGlow({ports.foreRcsX, ports.sideY}, 3, RGB(255, 224, 138));
            drawGlow({ports.aftRcsX, -ports.sideY}, 3, RGB(255, 224, 138));
        }
        if (_lastFlightInput.turn > 0.01f) {
            drawGlow({ports.foreRcsX, -ports.sideY}, 3, RGB(255, 224, 138));
            drawGlow({ports.aftRcsX, ports.sideY}, 3, RGB(255, 224, 138));
        }
    }

    void DrawPlayer(HDC dc)
    {
        DrawRuntimeVisualProfile(dc, _playerVisual, _player.position, _player.angle);
        DrawThrusterGlows(dc);

        POINT p = WorldToScreen(_player.position);
        if (!_player.mainThrustersEnabled) {
            DrawTextLine(dc, p.x - 54, p.y + 42, L"MAIN CUT / COAST", RGB(255, 180, 96));
        }
        if (!_player.rcsThrustersEnabled) {
            DrawTextLine(dc, p.x - 38, p.y + 60, L"RCS CUT", RGB(255, 140, 140));
        }
    }

    void DrawHomeView(HDC dc)
    {
        RefreshHomeView();
        RECT header{12, 12, _width - 12, 120};
        FillRect(dc, &header, _draw.panel);
        SelectObject(dc, _draw.titleFont);
        DrawTextLine(dc, 24, 24, L"Home Planet Surface - Persistent Builder", RGB(135, 230, 255));
        SelectObject(dc, _draw.smallFont);
        DrawTextLine(dc, 24, 54, L"H expedition view  B build mode  U/I zone  1-9 palette  Arrows cursor  P place  Del remove", RGB(218, 226, 240));
        DrawTextLine(dc, 760, 54, L"M cycle ship part  O install before launch  Solar map is now a compact support overview", RGB(255, 233, 140));
        DrawTextLine(dc, 24, 78, ToWide(_homeView.safetyText + " | " + _homeView.powerText), RGB(255, 233, 140));
        DrawTextLine(dc, 24, 98, ToWide(_homeView.automationText), RGB(187, 244, 214));

        DrawHomeSolarMap(dc);
        DrawHomeFactoryPanel(dc);
        DrawHomeBuildGrid(dc);
        DrawHomeRunPanel(dc);
    }

    void DrawHomeSolarMap(HDC dc)
    {
        const int cx = _width - 184;
        const int cy = 82;
        HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
        HGDIOBJ oldPen = SelectObject(dc, _draw.orbitPen);
        for (const auto& body : _homeView.bodies) {
            if (!body.primary) {
                const int r = std::max(14, static_cast<int>(body.orbitRadius * 0.13f));
                Ellipse(dc, cx - r, cy - r, cx + r, cy + r);
            }
        }
        SelectObject(dc, oldPen);
        SelectObject(dc, oldBrush);

        for (const auto& body : _homeView.bodies) {
            const int orbit = static_cast<int>(body.orbitRadius * 0.13f);
            const int px = body.primary ? cx : cx + static_cast<int>(std::cos(body.orbitAngleRadians) * orbit);
            const int py = body.primary ? cy : cy + static_cast<int>(std::sin(body.orbitAngleRadians) * orbit);
            const int radius = std::max(4, static_cast<int>(body.visualRadius * (body.primary ? 0.52f : 0.36f)));
            HBRUSH brush = CreateSolidBrush(ToColorRef(body.color));
            HGDIOBJ oldBodyBrush = SelectObject(dc, brush);
            HGDIOBJ oldBodyPen = SelectObject(dc, _draw.celestialPen);
            Ellipse(dc, px - radius, py - radius, px + radius, py + radius);
            SelectObject(dc, oldBodyPen);
            SelectObject(dc, oldBodyBrush);
            DeleteObject(brush);
            if (body.primary) {
                SelectObject(dc, _draw.homePower);
                HPEN glowPen = CreatePen(PS_DOT, 1, RGB(255, 210, 96));
                HGDIOBJ oldGlowPen = SelectObject(dc, glowPen);
                Ellipse(dc, px - radius * 3, py - radius * 3, px + radius * 3, py + radius * 3);
                SelectObject(dc, oldGlowPen);
                DeleteObject(glowPen);
            }
            DrawTextLine(dc, px + radius + 5, py - 8, ToWide(body.displayName), RGB(210, 218, 235));
        }
    }

    void DrawHomeFactoryPanel(HDC dc)
    {
        RECT panel{18, 138, 338, _height - 24};
        FillRect(dc, &panel, _draw.panel);
        DrawTextLine(dc, 30, 152, L"Home Base / Automation", RGB(135, 230, 255));
        DrawTextLine(dc, 30, 178, ToWide(_homeView.inventoryText), RGB(255, 233, 140));
        DrawTextLine(dc, 30, 200, ToWide(_homeView.shipyardText), RGB(187, 244, 214));
        DrawTextLine(dc, 30, 222, ToWide(_homeView.productionText), RGB(218, 226, 240));
        DrawTextLine(dc, 30, 242, ToWide(_homeView.logisticsText), RGB(178, 194, 215));
        DrawTextLine(dc, 30, 262, ToWide(_homeView.powerGridText), RGB(255, 233, 140));
        DrawTextLine(dc, 30, 282, ToWide(_homeView.shipyardQueueText), RGB(187, 244, 214));
        int y = 314;
        DrawTextLine(dc, 30, y, L"Build Zones", RGB(218, 226, 240));
        y += 22;
        for (const auto& zone : _homeView.buildZones) {
            if (y > _height - 120) {
                break;
            }
            DrawTextLine(dc, 42, y, ToWide(zone.displayName + " [" + zone.typeName + "]"), RGB(218, 226, 240));
            y += 18;
            DrawTextLine(dc, 58, y, ToWide("grid " + std::to_string(zone.gridWidth) + "x" + std::to_string(zone.gridHeight) +
                                           " structures=" + std::to_string(zone.structureCount) + " resources=" + zone.resourceSummary), RGB(178, 194, 215));
            y += 22;
        }
        DrawTextLine(dc, 30, _height - 110, L"Starter chain: Extractor -> Refinery -> Storage -> Shipyard", RGB(255, 233, 140));
        DrawTextLine(dc, 30, _height - 88, L"B toggles the active Factorio-like home build grid.", RGB(187, 244, 214));
        DrawTextLine(dc, 30, _height - 66, ToWide(_lastHomeBuildMessage), RGB(218, 226, 240));
    }

    COLORREF HomeStructureColor(subspace::HomeStructureType type) const
    {
        switch (type) {
            case subspace::HomeStructureType::Extractor: return RGB(126, 210, 142);
            case subspace::HomeStructureType::ConveyorHub: return RGB(186, 178, 118);
            case subspace::HomeStructureType::StorageDepot: return RGB(132, 170, 226);
            case subspace::HomeStructureType::Refinery: return RGB(230, 156, 96);
            case subspace::HomeStructureType::Assembler: return RGB(210, 128, 220);
            case subspace::HomeStructureType::PowerRelay: return RGB(255, 232, 120);
            case subspace::HomeStructureType::SolarCollector: return RGB(255, 210, 82);
            case subspace::HomeStructureType::DroneDepot: return RGB(116, 222, 215);
            case subspace::HomeStructureType::ResearchLab: return RGB(166, 150, 255);
            case subspace::HomeStructureType::ShipyardBay: return RGB(220, 230, 245);
            case subspace::HomeStructureType::DysonSwarmNode: return RGB(255, 178, 80);
            case subspace::HomeStructureType::OrbitalRingSegment: return RGB(198, 208, 230);
            case subspace::HomeStructureType::SubspaceAnchor: return RGB(255, 110, 220);
            case subspace::HomeStructureType::LandingPad: return RGB(190, 220, 255);
            default: return RGB(140, 160, 180);
        }
    }

    void DrawHomeBuildGrid(HDC dc)
    {
        const int panelLeft = 356;
        const int panelTop = 138;
        const int panelRight = std::max(panelLeft + 520, _width - 366);
        const int panelBottom = _height - 24;
        RECT panel{panelLeft, panelTop, panelRight, panelBottom};
        FillRect(dc, &panel, _draw.panel);

        const auto* zone = SelectedHomeBuildZone();
        DrawTextLine(dc, panel.left + 14, panel.top + 12, L"Surface / Orbital Build Grid", RGB(135, 230, 255));
        if (!zone) {
            DrawTextLine(dc, panel.left + 14, panel.top + 42, L"No home build zone selected.", RGB(218, 226, 240));
            return;
        }

        DrawTextLine(dc, panel.left + 14, panel.top + 38, ToWide(zone->displayName + " [" + subspace::HomeBuildZoneTypeName(zone->type) + "]"), RGB(255, 233, 140));
        DrawTextLine(dc, panel.left + 14, panel.top + 60, _homeBuildMode ? L"BUILD MODE ACTIVE" : L"B to enable build mode", _homeBuildMode ? RGB(187, 244, 214) : RGB(178, 194, 215));

        const int gridLeft = panel.left + 18;
        const int gridTop = panel.top + 90;
        const int panelWidth = static_cast<int>(panel.right - panel.left);
        const int panelHeightRemaining = static_cast<int>(panel.bottom - gridTop - 22);
        const int rawGridWidth = panelWidth - 170;
        const int rawGridHeight = panelHeightRemaining;
        const int gridWidth = rawGridWidth < 300 ? rawGridWidth : 300;
        const int gridHeight = rawGridHeight < 160 ? rawGridHeight : 160;
        if (gridWidth <= 0 || gridHeight <= 0) {
            DrawTextLine(dc, panel.left + 14, gridTop, L"Build grid has no drawable space.", RGB(255, 180, 135));
            return;
        }
        RECT gridRect{gridLeft, gridTop, gridLeft + gridWidth, gridTop + gridHeight};
        FillRect(dc, &gridRect, _draw.homeZone);

        constexpr int visibleCols = 18;
        constexpr int visibleRows = 13;
        for (int ty = 0; ty < visibleRows; ++ty) {
            for (int tx = 0; tx < visibleCols; ++tx) {
                RECT tile{gridLeft + tx * gridWidth / visibleCols,
                          gridTop + ty * gridHeight / visibleRows,
                          gridLeft + (tx + 1) * gridWidth / visibleCols + 1,
                          gridTop + (ty + 1) * gridHeight / visibleRows + 1};
                const float moisture = SeededUnit(0x5100u + static_cast<std::uint32_t>(zone->id.size()), tx + ty * 37);
                const float ore = SeededUnit(0xC0A1u, tx * 13 + ty * 19);
                COLORREF tileColor = moisture > 0.80f ? RGB(30, 82, 86) : (ore > 0.83f ? RGB(78, 68, 48) : RGB(28, 70, 58));
                HBRUSH tileBrush = CreateSolidBrush(tileColor);
                FillRect(dc, &tile, tileBrush);
                DeleteObject(tileBrush);
                if (ore > 0.90f) {
                    SetPixel(dc, tile.left + 3, tile.top + 3, RGB(228, 196, 110));
                    SetPixel(dc, tile.left + 6, tile.top + 5, RGB(170, 140, 84));
                }
            }
        }

        HGDIOBJ oldPen = SelectObject(dc, _draw.gridPen);
        for (int i = 0; i <= visibleCols; ++i) {
            const int x = gridLeft + i * gridWidth / visibleCols;
            MoveToEx(dc, x, gridTop, nullptr);
            LineTo(dc, x, gridTop + gridHeight);
        }
        for (int i = 0; i <= visibleRows; ++i) {
            const int y = gridTop + i * gridHeight / visibleRows;
            MoveToEx(dc, gridLeft, y, nullptr);
            LineTo(dc, gridLeft + gridWidth, y);
        }
        SelectObject(dc, oldPen);

        const auto toGridX = [&](int x) { return gridLeft + static_cast<int>((static_cast<float>(x) / std::max(1, zone->gridWidth - 1)) * gridWidth); };
        const auto toGridY = [&](int y) { return gridTop + static_cast<int>((static_cast<float>(y) / std::max(1, zone->gridHeight - 1)) * gridHeight); };
        for (const auto& structure : _director.save.home.structures) {
            if (structure.zoneId != zone->id) {
                continue;
            }
            const int sx = toGridX(structure.x);
            const int sy = toGridY(structure.y);
            HBRUSH brush = CreateSolidBrush(HomeStructureColor(structure.type));
            HGDIOBJ oldBrush = SelectObject(dc, brush);
            HGDIOBJ oldStructurePen = SelectObject(dc, _draw.celestialPen);
            Rectangle(dc, sx - 7, sy - 7, sx + 8, sy + 8);
            if (structure.type == subspace::HomeStructureType::ConveyorHub ||
                structure.type == subspace::HomeStructureType::StorageDepot ||
                structure.type == subspace::HomeStructureType::ShipyardBay) {
                MoveToEx(dc, sx - 11, sy, nullptr);
                LineTo(dc, sx + 12, sy);
                MoveToEx(dc, sx, sy - 11, nullptr);
                LineTo(dc, sx, sy + 12);
            }
            SelectObject(dc, oldStructurePen);
            SelectObject(dc, oldBrush);
            DeleteObject(brush);
        }

        if (_homeBuildMode) {
            const int cx = toGridX(_homeBuildCursorX);
            const int cy = toGridY(_homeBuildCursorY);
            HPEN cursorPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 180));
            HGDIOBJ oldCursorPen = SelectObject(dc, cursorPen);
            HGDIOBJ oldCursorBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
            Rectangle(dc, cx - 7, cy - 7, cx + 8, cy + 8);
            SelectObject(dc, oldCursorBrush);
            SelectObject(dc, oldCursorPen);
            DeleteObject(cursorPen);
        }

        int y = panel.top + 88;
        const int paletteX = gridLeft + gridWidth + 18;
        DrawTextLine(dc, paletteX, y, L"Palette", RGB(218, 226, 240));
        y += 22;
        RefreshHomeBuildPalette();
        for (int i = 0; i < static_cast<int>(_homeBuildPalette.size()) && i < 9; ++i) {
            const auto type = _homeBuildPalette[static_cast<std::size_t>(i)];
            const auto cost = subspace::GetHomeStructureCost(type);
            const std::wstring prefix = std::wstring(i == _selectedHomePalette ? L"> " : L"  ") + std::to_wstring(i + 1) + L" ";
            DrawTextLine(dc, paletteX, y, prefix + ToWide(subspace::HomeStructureTypeName(type)), i == _selectedHomePalette ? RGB(255, 233, 140) : RGB(218, 226, 240));
            y += 18;
            if (i == _selectedHomePalette) {
                DrawTextLine(dc, paletteX + 14, y, ToWide(subspace::HomeStructureCostSummary(cost)), RGB(178, 194, 215));
                y += 18;
            }
        }
    }

    void DrawHomeRunPanel(HDC dc)
    {
        RECT panel{_width - 348, 138, _width - 18, _height - 24};
        FillRect(dc, &panel, _draw.panel);
        DrawTextLine(dc, panel.left + 14, panel.top + 14, L"Expedition Offers", RGB(135, 230, 255));
        DrawTextLine(dc, panel.left + 14, panel.top + 38, L"Runs feed home inventory, research, blueprints, and shipyard growth.", RGB(218, 226, 240));
        int y = panel.top + 72;
        for (const auto& offer : _homeView.runOffers) {
            RECT row{panel.left + 12, y - 4, panel.right - 12, y + 62};
            FillRect(dc, &row, offer.index == _selectedRunOffer ? _draw.panelAlt : _draw.panel);
            DrawTextLine(dc, panel.left + 22, y, std::to_wstring(offer.index + 1) + L". " + ToWide(offer.displayName), RGB(255, 233, 140));
            DrawTextLine(dc, panel.left + 38, y + 20, ToWide("Risk: " + offer.riskLabel + "   Reward: " + offer.rewardLabel), RGB(218, 226, 240));
            DrawTextLine(dc, panel.left + 38, y + 40, ToWide(offer.summary), RGB(178, 194, 215));
            y += 76;
        }
        if (_activeRun.state == subspace::ExpeditionRunState::Active ||
            _activeRun.state == subspace::ExpeditionRunState::ExtractionAvailable ||
            _activeRun.state == subspace::ExpeditionRunState::Extracted ||
            _activeRun.state == subspace::ExpeditionRunState::Failed) {
            DrawTextLine(dc, panel.left + 14, panel.bottom - 78, L"Last/Active Run", RGB(135, 230, 255));
            DrawTextLine(dc, panel.left + 14, panel.bottom - 54, ToWide(subspace::ExpeditionRunSummary(_activeRun)), RGB(218, 226, 240));
        }
    }

    void DrawHud(HDC dc)
    {
        RECT panel{12, 12, 438, 164};
        FillRect(dc, &panel, _draw.panel);
        SelectObject(dc, _draw.titleFont);
        DrawTextLine(dc, 24, 22, L"Codename Subspace - Playable Client", RGB(135, 230, 255));
        SelectObject(dc, _draw.smallFont);
        DrawTextLine(dc, 24, 52, L"W/S thrust  A/D rotate via RCS  Q/E strafe  T main cutoff  Y RCS cutoff  Shift precision", RGB(218, 226, 240));
        DrawTextLine(dc, 24, 72, L"F1 console  F2 dev  F3 debug  H home  M cycle part  O install at home  L launch  X extract", RGB(218, 226, 240));
        DrawTextLine(dc, 24, 98, L"Cargo: " + std::to_wstring(_player.cargo) + L" (" + std::to_wstring(_player.cargoValue) + L" cr)   Credits: " + std::to_wstring(_player.credits), RGB(255, 233, 140));
        DrawTextLine(dc, 24, 118, L"Hull: " + FormatFloat(_player.hull) + L"   Fuel: " + FormatFloat(_player.fuel) + L"/" + FormatFloat(_shipPartStats.fuelCapacity) + L"   Speed: " + FormatFloat(Length(_player.velocity)), RGB(187, 244, 214));
        DrawTextLine(dc, 24, 138, std::wstring(L"Drive: ") + (_player.mainThrustersEnabled ? L"MAIN ON" : L"MAIN CUT") + L" / " + (_player.rcsThrustersEnabled ? L"RCS ON" : L"RCS CUT") + L"   Flight: " + ToWide(_lastFlightControl.modeLabel), (_player.mainThrustersEnabled || _player.rcsThrustersEnabled) ? RGB(187, 244, 214) : RGB(255, 180, 96));
        DrawTextLine(dc, 24, 158, L"Loadout: " + ToWide(_shipLoadout.displayName) + L"   FX particles: " + std::to_wstring(_clientParticles.AliveCount()), RGB(190, 210, 230));
        DrawTextLine(dc, 24, 178, L"VISUAL: ULYSSES_PROXY_PASS153  sockets: MAIN_AFT/RETRO/RCS", RGB(255, 233, 140));
        DrawTextLine(dc, 24, 158, L"Sector: " + ToWide(_sectorId) + L"   Field: " + ToWide(_sectorSurvey.asteroidField.dominantResource) + L"   Light: " + FormatFloat(SolarLightIntensity() * 100.0f, 0) + L"%", RGB(210, 218, 235));
        if (IsValidCelestialSelection()) {
            const auto& body = _sectorBackdrop[static_cast<std::size_t>(_selectedCelestial)];
            DrawTextLine(dc, 24, 198, L"Body: " + ToWide(body.definition.displayName) + L"   Dist: " + FormatFloat(Distance(_player.position, body.position), 0), RGB(210, 218, 235));
        }
        if (_activeRun.state == subspace::ExpeditionRunState::Active || _activeRun.state == subspace::ExpeditionRunState::ExtractionAvailable) {
            DrawTextLine(dc, 24, 198, L"Run: " + ToWide(subspace::ExpeditionObjectiveTypeName(_activeRun.config.objective)) + L"  State: " + ToWide(subspace::ExpeditionRunStateName(_activeRun.state)), RGB(255, 233, 140));
        }
        if (_railTravel.state == subspace::RailTravelState::Traveling || _railTravel.state == subspace::RailTravelState::Completed || _railTravel.state == subspace::RailTravelState::Failed) {
            DrawTextLine(dc, 24, 218, L"Rail: " + ToWide(subspace::RailTravelStateSummary(_railTravel)), RGB(187, 244, 214));
        }

        if (_messageTimer > 0.0f && !_timedMessage.empty()) {
            const int messageRight = (_width - 12) < 760 ? (_width - 12) : 760;
            RECT msg{12, _height - 86, messageRight, _height - 42};
            FillRect(dc, &msg, _draw.panelAlt);
            DrawTextLine(dc, 24, _height - 74, _timedMessage, RGB(255, 235, 150));
        }

        if (_showDebug) {
            RECT debug{_width - 460, 12, _width - 12, 186};
            FillRect(dc, &debug, _draw.panel);
            DrawTextLine(dc, _width - 344, 24, L"Debug", RGB(135, 230, 255));
            DrawTextLine(dc, _width - 344, 48, L"Position: " + FormatFloat(_player.position.x) + L", " + FormatFloat(_player.position.y), RGB(218, 226, 240));
            DrawTextLine(dc, _width - 344, 68, L"Asteroids: " + std::to_wstring(AliveAsteroids()) + L"   Cargo Pods: " + std::to_wstring(AliveCargo()) + L"   Particles: " + std::to_wstring(_clientParticles.AliveCount()), RGB(218, 226, 240));
            DrawTextLine(dc, _width - 344, 88, L"System: " + ToWide(_sectorSystem.displayName), RGB(218, 226, 240));
            DrawTextLine(dc, _width - 344, 108, L"Bodies: " + std::to_wstring(_sectorSystem.bodies.size()) + L"   Seed: " + std::to_wstring(_sectorSeed), RGB(218, 226, 240));
            DrawTextLine(dc, _width - 344, 128, L"Resources: " + ToWide(subspace::AsteroidFieldSummary(_sectorSurvey.asteroidField)), RGB(218, 226, 240));
            DrawTextLine(dc, _width - 344, 148, L"Encounters: " + ToWide(subspace::EncounterSpawnTableSummary(_encounterTable)), RGB(218, 226, 240));
            DrawTextLine(dc, _width - 344, 168, L"Celestial: " + std::to_wstring(_sectorBackdrop.size()) + L" bodies  Light " + FormatFloat(SolarLightIntensity() * 100.0f, 0) + L"%", RGB(218, 226, 240));
            DrawTextLine(dc, _width - 344, 188, L"Last runtime cmd: " + _lastRuntimeCommand, RGB(218, 226, 240));
            DrawTextLine(dc, _width - 344, 208, L"Result: " + _lastRuntimeCommandResult, RGB(218, 226, 240));
        }
    }

    bool IsValidCelestialSelection() const
    {
        return _selectedCelestial >= 0 && _selectedCelestial < static_cast<int>(_sectorBackdrop.size());
    }

    float SolarLightIntensity() const
    {
        if (_sectorBackdrop.empty() || _sectorBackdrop.front().definition.type != subspace::CelestialBodyType::Star) {
            return 0.18f;
        }
        const float distance = Distance(_player.position, _sectorBackdrop.front().position);
        const float normalized = std::max(0.0f, 1.0f - distance / kSolarLightFalloffRange);
        return std::max(0.22f, std::min(1.0f, 0.22f + normalized * 0.82f));
    }

    void SelectNearestCelestial()
    {
        float best = 99999999.0f;
        int bestIndex = -1;
        for (int i = 0; i < static_cast<int>(_sectorBackdrop.size()); ++i) {
            if (!_sectorBackdrop[static_cast<std::size_t>(i)].interactive) {
                continue;
            }
            const float distance = Distance(_player.position, _sectorBackdrop[static_cast<std::size_t>(i)].position);
            if (distance < best) {
                best = distance;
                bestIndex = i;
            }
        }
        _selectedCelestial = bestIndex;
        if (IsValidCelestialSelection()) {
            const auto& body = _sectorBackdrop[static_cast<std::size_t>(_selectedCelestial)];
            AddTimedMessage(L"Selected body: " + ToWide(body.definition.displayName) + L" at " + FormatFloat(best, 0) + L" units.");
        }
    }

    void ScanSelectedCelestial()
    {
        if (!IsValidCelestialSelection()) {
            SelectNearestCelestial();
        }
        if (!IsValidCelestialSelection()) {
            AddTimedMessage(L"No celestial body selected.");
            return;
        }
        const auto& body = _sectorBackdrop[static_cast<std::size_t>(_selectedCelestial)];
        const float distance = Distance(_player.position, body.position);
        if (distance > kCelestialScanRange) {
            AddTimedMessage(L"Body outside close scan range. Use N to retarget or fly closer.");
        }
        AddConsoleLine(L"Body scan: " + ToWide(body.definition.displayName));
        AddConsoleLine(L"Type=" + ToWide(subspace::CelestialBodyTypeName(body.definition.type)) +
                       L" Role=" + ToWide(subspace::CelestialOrbitRoleName(body.definition.orbitRole)) +
                       L" Distance=" + FormatFloat(distance, 0));
        std::wstring resources = L"Resources:";
        for (const auto& tag : body.definition.resourceTags) {
            resources += L" " + ToWide(tag);
        }
        AddConsoleLine(resources + L" Richness=" + std::to_wstring(body.definition.resourceRichness));
        AddConsoleLine(std::wstring(L"Hazard=") + (body.definition.isHazardous ? L"YES" : L"no") +
                       L" Light=" + FormatFloat(SolarLightIntensity() * 100.0f, 0) + L"%");
    }

    int AliveAsteroids() const
    {
        return static_cast<int>(std::count_if(_asteroids.begin(), _asteroids.end(), [](const Asteroid& asteroid) { return asteroid.alive; }));
    }

    int AliveCargo() const
    {
        return static_cast<int>(std::count_if(_cargo.begin(), _cargo.end(), [](const CargoPod& pod) { return pod.alive; }));
    }

    void DrawConsole(HDC dc)
    {
        if (!_consoleOpen) {
            return;
        }
        RECT console{24, _height - 288, _width - 24, _height - 24};
        FillRect(dc, &console, _draw.panel);
        SelectObject(dc, _draw.smallFont);
        DrawTextLine(dc, console.left + 12, console.top + 10, L"Developer Console - enter 'help' for local commands", RGB(135, 230, 255));

        int y = console.top + 38;
        for (const auto& line : _consoleLines) {
            DrawTextLine(dc, console.left + 12, y, line, RGB(218, 226, 240));
            y += 18;
        }
        DrawTextLine(dc, console.left + 12, console.bottom - 30, L"> " + _consoleInput + L"_", RGB(255, 244, 172));
    }

    HWND _hwnd = nullptr;
    int _width = kDefaultWidth;
    int _height = kDefaultHeight;
    subspace::GameRuntime _runtime;
    PlayerShip _player;
    Vec2 _station{};
    std::vector<Asteroid> _asteroids;
    std::vector<CargoPod> _cargo;
    std::vector<Star> _stars;
    int _selectedAsteroid = -1;
    bool _consoleOpen = false;
    bool _devMode = false;
    bool _showDebug = false;
    std::array<bool, 256> _keys{};
    std::array<bool, 256> _previousKeys{};
    std::wstring _consoleInput;
    std::vector<std::wstring> _consoleLines;
    std::wstring _timedMessage;
    float _messageTimer = 0.0f;
    float _laserTimer = 0.0f;
    float _laserFlash = 0.0f;
    std::wstring _lastRuntimeCommand = L"none";
    std::wstring _lastRuntimeCommandResult = L"none";
    std::string _sectorId = "DEV-100";
    int _sectorIndex = 0;
    std::uint32_t _sectorSeed = 0;
    subspace::StarSystemDefinition _sectorSystem;
    subspace::SectorResourceSurvey _sectorSurvey;
    subspace::SectorScannerReport _sectorScannerReport;
    subspace::StationServiceProfile _stationEconomy;
    subspace::EncounterSpawnTable _encounterTable;
    std::vector<subspace::CargoYieldItem> _cargoManifest;
    std::vector<SectorBackdropBody> _sectorBackdrop;
    int _selectedCelestial = -1;
    float _sectorElapsedSeconds = 0.0f;
    subspace::ModularGeneratedShip _playerShipDefinition;
    subspace::RuntimeVisualProfile _playerVisual;
    subspace::RuntimeVisualProfile _stationVisual;
    subspace::RuntimeVisualProfile _cargoVisual;
    subspace::RogueliteDirectorState _director;
    subspace::HomeClientViewModel _homeView;
    subspace::ExpeditionRunStateSnapshot _activeRun;
    bool _homeViewOpen = false;
    int _selectedRunOffer = -1;
    float _homeElapsedSeconds = 0.0f;
    float _homeFactoryTick = 0.0f;
    bool _homeBuildMode = false;
    int _selectedHomeBuildZone = 0;
    int _homeBuildCursorX = 30;
    int _homeBuildCursorY = 30;
    int _selectedHomePalette = 0;
    std::vector<subspace::HomeStructureType> _homeBuildPalette;
    std::string _lastHomeBuildMessage = "Home build grid ready.";
    std::vector<subspace::ShipPartDefinition> _shipPartCatalog;
    subspace::ShipLoadout _shipLoadout;
    subspace::ShipPartStats _shipPartStats;
    int _selectedShipPartIndex = 0;
    subspace::client::ClientParticleFx _clientParticles;
    subspace::ShipFlightInputFrame _lastFlightInput;
    subspace::ShipFlightControlOutput _lastFlightControl;
    std::vector<subspace::RailTravelRouteOption> _railRoutes;
    int _selectedRailRoute = 0;
    subspace::RailTravelStateSnapshot _railTravel;
    DrawResources _draw;
};

PlayableClientApp* GetApp(HWND hwnd)
{
    return reinterpret_cast<PlayableClientApp*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_SIZE:
        if (auto* app = GetApp(hwnd)) {
            app->OnResize(LOWORD(lParam), HIWORD(lParam));
        }
        return 0;
    case WM_KEYDOWN:
        if (auto* app = GetApp(hwnd)) {
            app->OnKeyDown(wParam);
        }
        return 0;
    case WM_KEYUP:
        if (auto* app = GetApp(hwnd)) {
            app->OnKeyUp(wParam);
        }
        return 0;
    case WM_CHAR:
        if (auto* app = GetApp(hwnd)) {
            app->OnChar(wParam);
        }
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(hwnd, &paint);
        if (auto* app = GetApp(hwnd)) {
            app->Draw(dc);
        }
        EndPaint(hwnd, &paint);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}

} // namespace

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int showCommand)
{
    const wchar_t* className = L"SubspacePlayableClientWindow";

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = className;

    if (!RegisterClassExW(&windowClass)) {
        MessageBoxW(nullptr, L"Failed to register Subspace window class.", L"Subspace", MB_ICONERROR | MB_OK);
        return 1;
    }

    RECT desired{0, 0, kDefaultWidth, kDefaultHeight};
    AdjustWindowRect(&desired, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExW(
        0,
        className,
        L"Codename Subspace - Playable Client",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        desired.right - desired.left,
        desired.bottom - desired.top,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (!hwnd) {
        MessageBoxW(nullptr, L"Failed to create Subspace playable client window.", L"Subspace", MB_ICONERROR | MB_OK);
        return 1;
    }

    PlayableClientApp app;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&app));
    app.Initialize(hwnd);

    ShowWindow(hwnd, showCommand);
    UpdateWindow(hwnd);

    auto previous = std::chrono::steady_clock::now();
    MSG message{};
    bool running = true;
    while (running) {
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - previous).count();
        previous = now;
        app.Update(dt);
        InvalidateRect(hwnd, nullptr, FALSE);
        Sleep(1);
    }

    app.Shutdown();
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    return static_cast<int>(message.wParam);
}
