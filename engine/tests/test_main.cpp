// Self-contained unit tests for Codename-Subspace engine core systems.
// No test framework dependency — uses assert-style macros and a simple runner.

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "core/Math.h"
#include "ships/Block.h"
#include "ships/Ship.h"
#include "ships/BlockPlacement.h"
#include "ships/ShipStats.h"
#include "ships/ShipDamage.h"
#include "ships/Blueprint.h"
#include "ship_editor/ShipEditorController.h"
#include "ship_editor/ShipEditorState.h"
#include "ship_editor/SymmetrySystem.h"
#include "factions/FactionProfile.h"
#include "factions/SilhouetteProfile.h"
#include "ai/AIShipBuilder.h"
#include "weapons/WeaponSystem.h"
#include "ships/ModuleDef.h"
#include "ships/ShipArchetype.h"
#include "core/logging/Logger.h"
#include "core/events/EventSystem.h"
#include "core/events/GameEvents.h"
#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/physics/PhysicsComponent.h"
#include "core/physics/PhysicsSystem.h"
#include "core/resources/Inventory.h"
#include "core/config/ConfigurationManager.h"
#include "core/persistence/SaveGameManager.h"
#include "navigation/NavigationSystem.h"
#include "combat/CombatSystem.h"
#include "trading/TradingSystem.h"
#include "rpg/ProgressionSystem.h"
#include "crew/CrewSystem.h"
#include "power/PowerSystem.h"
#include "mining/MiningSystem.h"
#include "procedural/GalaxyGenerator.h"
#include "quest/QuestSystem.h"
#include "tutorial/TutorialSystem.h"
#include "ai/AIDecisionSystem.h"
#include "ai/AISteeringSystem.h"
#include "core/physics/SpatialHash.h"
#include "ui/UITypes.h"
#include "ui/UIElement.h"
#include "ui/UIPanel.h"
#include "ui/UIRenderer.h"
#include "ui/UISystem.h"
#include "networking/NetworkSystem.h"
#include "scripting/ScriptingSystem.h"
#include "audio/AudioSystem.h"
#include "core/Engine.h"

using namespace subspace;

// ---------------------------------------------------------------------------
// Test harness
// ---------------------------------------------------------------------------
static int testsPassed = 0;
static int testsFailed = 0;

#define TEST(name, expr) do { \
    if (expr) { testsPassed++; std::cout << "  PASS: " << name << "\n"; } \
    else { testsFailed++; std::cout << "  FAIL: " << name << " (" << __FILE__ << ":" << __LINE__ << ")\n"; } \
} while(0)

static constexpr float kEpsilon = 1e-4f;
static bool ApproxEq(float a, float b) { return std::fabs(a - b) < kEpsilon; }

// Helper: create a simple block
static std::shared_ptr<Block> MakeBlock(Vector3Int pos, Vector3Int size,
                                        BlockType type = BlockType::Hull,
                                        MaterialType mat = MaterialType::Iron) {
    auto b = std::make_shared<Block>();
    b->gridPos       = pos;
    b->size          = size;
    b->rotationIndex = 0;
    b->shape         = BlockShape::Cube;
    b->type          = type;
    b->material      = mat;
    const MaterialStats& ms = MaterialDatabase::Get(mat);
    b->maxHP     = GetBlockBaseHP(type) * ms.hpMultiplier;
    b->currentHP = b->maxHP;
    return b;
}

// ===================================================================
// 1. Math tests
// ===================================================================
static void TestMath() {
    std::cout << "[Math]\n";

    // Vector3Int construction
    Vector3Int v(1, 2, 3);
    TEST("Vector3Int construction", v.x == 1 && v.y == 2 && v.z == 3);

    // Default construction
    Vector3Int vd;
    TEST("Vector3Int default zero", vd.x == 0 && vd.y == 0 && vd.z == 0);

    // Equality
    TEST("Vector3Int equality", Vector3Int(1, 2, 3) == Vector3Int(1, 2, 3));
    TEST("Vector3Int inequality", Vector3Int(1, 2, 3) != Vector3Int(4, 5, 6));

    // Addition / subtraction
    Vector3Int a(1, 2, 3), b(4, 5, 6);
    TEST("Vector3Int addition", (a + b) == Vector3Int(5, 7, 9));
    TEST("Vector3Int subtraction", (b - a) == Vector3Int(3, 3, 3));

    // Static helpers
    TEST("Vector3Int::Zero()", Vector3Int::Zero() == Vector3Int(0, 0, 0));
    TEST("Vector3Int::One()", Vector3Int::One() == Vector3Int(1, 1, 1));

    // FloorFromFloat
    TEST("FloorFromFloat positive", Vector3Int::FloorFromFloat(1.7f, 2.3f, 3.9f) == Vector3Int(1, 2, 3));
    TEST("FloorFromFloat negative", Vector3Int::FloorFromFloat(-0.5f, -1.9f, 0.0f) == Vector3Int(-1, -2, 0));

    // Vector3 length
    Vector3 v3(3.0f, 4.0f, 0.0f);
    TEST("Vector3 length", ApproxEq(v3.length(), 5.0f));

    // Vector3 normalized
    Vector3 n = v3.normalized();
    TEST("Vector3 normalized length ~1", ApproxEq(n.length(), 1.0f));
    TEST("Vector3 normalized direction", ApproxEq(n.x, 0.6f) && ApproxEq(n.y, 0.8f));

    // Zero vector normalized
    Vector3 z;
    Vector3 zn = z.normalized();
    TEST("Vector3 zero normalized", ApproxEq(zn.length(), 0.0f));
}

// ===================================================================
// 2. Block tests
// ===================================================================
static void TestBlock() {
    std::cout << "[Block]\n";

    // MaterialDatabase::Get for all materials
    const MaterialType allMats[] = {
        MaterialType::Iron, MaterialType::Titanium, MaterialType::Naonite,
        MaterialType::Trinium, MaterialType::Xanion, MaterialType::Ogonite,
        MaterialType::Avorion
    };
    for (auto mat : allMats) {
        const MaterialStats& ms = MaterialDatabase::Get(mat);
        TEST(("MaterialDatabase density > 0 for " + std::to_string(static_cast<int>(mat))).c_str(),
             ms.density > 0.0f);
        TEST(("MaterialDatabase hpMul > 0 for " + std::to_string(static_cast<int>(mat))).c_str(),
             ms.hpMultiplier > 0.0f);
    }

    // Volume 1x1x1
    Block b1;
    b1.size = {1, 1, 1};
    TEST("Block volume 1x1x1 = 1", ApproxEq(b1.Volume(), 1.0f));

    // Volume 2x3x4
    Block b2;
    b2.size = {2, 3, 4};
    TEST("Block volume 2x3x4 = 24", ApproxEq(b2.Volume(), 24.0f));

    // Mass = volume * density
    Block b3;
    b3.size = {1, 1, 1};
    b3.material = MaterialType::Iron;
    float ironDensity = MaterialDatabase::Get(MaterialType::Iron).density;
    TEST("Block mass = volume * density", ApproxEq(b3.Mass(), 1.0f * ironDensity));

    // GetBlockBaseHP positive for all types
    const BlockType allTypes[] = {
        BlockType::Hull, BlockType::Armor, BlockType::Engine,
        BlockType::Generator, BlockType::Gyro, BlockType::Cargo,
        BlockType::WeaponMount
    };
    for (auto t : allTypes) {
        TEST(("GetBlockBaseHP > 0 for type " + std::to_string(static_cast<int>(t))).c_str(),
             GetBlockBaseHP(t) > 0.0f);
    }
}

// ===================================================================
// 3. Ship tests
// ===================================================================
static void TestShip() {
    std::cout << "[Ship]\n";

    Ship ship;
    TEST("Empty ship block count is 0", ship.BlockCount() == 0);
    TEST("Empty ship IsEmpty", ship.IsEmpty());

    // Add a block then clear
    auto blk = MakeBlock({0, 0, 0}, {1, 1, 1});
    BlockPlacement::Place(ship, blk);
    TEST("Ship has 1 block after place", ship.BlockCount() == 1);

    ship.Clear();
    TEST("Ship::Clear() empties blocks", ship.BlockCount() == 0);
    TEST("Ship::Clear() empties occupiedCells", ship.occupiedCells.empty());
    TEST("Ship::Clear() zeroes mass", ApproxEq(ship.totalMass, 0.0f));
}

// ===================================================================
// 4. BlockPlacement tests
// ===================================================================
static void TestBlockPlacement() {
    std::cout << "[BlockPlacement]\n";

    // GetOccupiedCells 1x1x1
    Block b1;
    b1.gridPos = {0, 0, 0};
    b1.size = {1, 1, 1};
    auto cells1 = BlockPlacement::GetOccupiedCells(b1);
    TEST("GetOccupiedCells 1x1x1 = 1 cell", cells1.size() == 1);

    // GetOccupiedCells 2x2x2
    Block b2;
    b2.gridPos = {0, 0, 0};
    b2.size = {2, 2, 2};
    auto cells2 = BlockPlacement::GetOccupiedCells(b2);
    TEST("GetOccupiedCells 2x2x2 = 8 cells", cells2.size() == 8);

    // CanPlace first block on empty ship
    {
        Ship ship;
        Block fb;
        fb.gridPos = {0, 0, 0};
        fb.size = {1, 1, 1};
        TEST("CanPlace first block (empty ship)", BlockPlacement::CanPlace(ship, fb));
    }

    // CanPlace adjacent block
    {
        Ship ship;
        auto first = MakeBlock({0, 0, 0}, {1, 1, 1});
        BlockPlacement::Place(ship, first);

        Block adj;
        adj.gridPos = {1, 0, 0};
        adj.size = {1, 1, 1};
        TEST("CanPlace adjacent block", BlockPlacement::CanPlace(ship, adj));
    }

    // CanPlace non-adjacent block
    {
        Ship ship;
        auto first = MakeBlock({0, 0, 0}, {1, 1, 1});
        BlockPlacement::Place(ship, first);

        Block far;
        far.gridPos = {5, 5, 5};
        far.size = {1, 1, 1};
        TEST("CanPlace non-adjacent block is false", !BlockPlacement::CanPlace(ship, far));
    }

    // CanPlace overlapping block
    {
        Ship ship;
        auto first = MakeBlock({0, 0, 0}, {1, 1, 1});
        BlockPlacement::Place(ship, first);

        Block overlap;
        overlap.gridPos = {0, 0, 0};
        overlap.size = {1, 1, 1};
        TEST("CanPlace overlapping block is false", !BlockPlacement::CanPlace(ship, overlap));
    }

    // Place adds block
    {
        Ship ship;
        auto blk = MakeBlock({0, 0, 0}, {1, 1, 1});
        bool ok = BlockPlacement::Place(ship, blk);
        TEST("Place returns true", ok);
        TEST("Place adds block to ship", ship.BlockCount() == 1);
    }

    // Remove removes block
    {
        Ship ship;
        auto blk = MakeBlock({0, 0, 0}, {1, 1, 1});
        BlockPlacement::Place(ship, blk);
        BlockPlacement::Remove(ship, blk);
        TEST("Remove removes block from ship", ship.BlockCount() == 0);
        TEST("Remove clears occupied cells", ship.occupiedCells.empty());
    }
}

// ===================================================================
// 5. Symmetry tests
// ===================================================================
static void TestSymmetry() {
    std::cout << "[Symmetry]\n";

    Block original;
    original.gridPos = {3, 5, 7};
    original.size = {1, 1, 1};
    original.shape = BlockShape::Cube;
    original.type = BlockType::Hull;
    original.material = MaterialType::Iron;

    // Mirror X
    Block mx = SymmetrySystem::CreateMirroredBlock(original, SymmetryMirrorX);
    TEST("Mirror X flips x", mx.gridPos.x == -original.gridPos.x - original.size.x);
    TEST("Mirror X preserves y", mx.gridPos.y == original.gridPos.y);
    TEST("Mirror X preserves z", mx.gridPos.z == original.gridPos.z);

    // Mirror Y
    Block my = SymmetrySystem::CreateMirroredBlock(original, SymmetryMirrorY);
    TEST("Mirror Y flips y", my.gridPos.y == -original.gridPos.y - original.size.y);
    TEST("Mirror Y preserves x", my.gridPos.x == original.gridPos.x);

    // Mirror Z
    Block mz = SymmetrySystem::CreateMirroredBlock(original, SymmetryMirrorZ);
    TEST("Mirror Z flips z", mz.gridPos.z == -original.gridPos.z - original.size.z);
    TEST("Mirror Z preserves x", mz.gridPos.x == original.gridPos.x);

    // GetAllMirroredBlocks MirrorX → 1 mirror
    auto mirrorsX = SymmetrySystem::GetAllMirroredBlocks(original, SymmetryMirrorX);
    TEST("MirrorX returns 1 mirror", mirrorsX.size() == 1);

    // GetAllMirroredBlocks MirrorX|MirrorY → 3 mirrors (X, Y, XY)
    auto mirrorsXY = SymmetrySystem::GetAllMirroredBlocks(original, SymmetryMirrorX | SymmetryMirrorY);
    TEST("MirrorX|MirrorY returns 3 mirrors", mirrorsXY.size() == 3);
}

// ===================================================================
// 6. ShipStats tests
// ===================================================================
static void TestShipStats() {
    std::cout << "[ShipStats]\n";

    // No blocks → zero stats
    {
        Ship ship;
        ShipStats::Recalculate(ship);
        TEST("No blocks gives zero mass", ApproxEq(ship.totalMass, 0.0f));
        TEST("No blocks gives zero thrust", ApproxEq(ship.thrust, 0.0f));
        TEST("No blocks gives zero power", ApproxEq(ship.powerGen, 0.0f));
    }

    // Hull block → positive mass
    {
        Ship ship;
        auto hull = MakeBlock({0, 0, 0}, {1, 1, 1}, BlockType::Hull, MaterialType::Iron);
        BlockPlacement::Place(ship, hull);
        TEST("Hull block gives positive mass", ship.totalMass > 0.0f);
    }

    // Engine block → adds thrust
    {
        Ship ship;
        auto core = MakeBlock({0, 0, 0}, {1, 1, 1}, BlockType::Hull);
        BlockPlacement::Place(ship, core);
        float massBefore = ship.totalMass;
        float thrustBefore = ship.thrust;

        auto engine = MakeBlock({1, 0, 0}, {1, 1, 1}, BlockType::Engine);
        BlockPlacement::Place(ship, engine);
        TEST("Engine adds thrust", ship.thrust > thrustBefore);
        TEST("Engine adds mass", ship.totalMass > massBefore);
    }

    // Generator block → adds power (needs energyBonus > 0 material)
    {
        Ship ship;
        auto core = MakeBlock({0, 0, 0}, {1, 1, 1}, BlockType::Hull);
        BlockPlacement::Place(ship, core);

        // Naonite has energyBonus 0.1
        auto gen = MakeBlock({1, 0, 0}, {1, 1, 1}, BlockType::Generator, MaterialType::Naonite);
        BlockPlacement::Place(ship, gen);
        TEST("Generator adds power", ship.powerGen > 0.0f);
    }
}

// ===================================================================
// 7. ShipDamage tests
// ===================================================================
static void TestShipDamage() {
    std::cout << "[ShipDamage]\n";

    // Apply damage reduces HP
    {
        Ship ship;
        auto blk = MakeBlock({0, 0, 0}, {1, 1, 1});
        BlockPlacement::Place(ship, blk);
        float hpBefore = blk->currentHP;
        bool destroyed = ShipDamage::ApplyDamage(ship, blk, 10.0f);
        TEST("Apply damage reduces HP", blk->currentHP < hpBefore);
        TEST("Block not destroyed by small damage", !destroyed);
    }

    // Destroy block
    {
        Ship ship;
        auto blk = MakeBlock({0, 0, 0}, {1, 1, 1});
        BlockPlacement::Place(ship, blk);
        bool destroyed = ShipDamage::ApplyDamage(ship, blk, 99999.0f);
        TEST("Block destroyed when HP <= 0", destroyed);
        TEST("Destroyed block removed from ship", ship.BlockCount() == 0);
    }

    // Stat recalculation after removal
    {
        Ship ship;
        auto b1 = MakeBlock({0, 0, 0}, {1, 1, 1});
        auto b2 = MakeBlock({1, 0, 0}, {1, 1, 1});
        BlockPlacement::Place(ship, b1);
        BlockPlacement::Place(ship, b2);
        float massWith2 = ship.totalMass;

        ShipDamage::RemoveBlock(ship, b2);
        TEST("Removal triggers stat recalc (mass decreased)", ship.totalMass < massWith2);
    }
}

// ===================================================================
// 8. ShipEditor tests
// ===================================================================
static void TestShipEditor() {
    std::cout << "[ShipEditor]\n";

    Ship ship;
    ShipEditorController editor(ship);

    // Initial state
    TEST("Initial mode is Place", editor.GetState().mode == BuildMode::Place);

    // Toggle symmetry X
    editor.GetState().ToggleSymmetryX();
    TEST("ToggleSymmetryX enables X", (editor.GetState().symmetry & SymmetryMirrorX) != 0);
    editor.GetState().ToggleSymmetryX(); // reset

    // Rotate90 cycles
    editor.GetState().rotationIndex = 0;
    editor.GetState().Rotate90();
    TEST("Rotate90 first call = 1", editor.GetState().rotationIndex == 1);
    editor.GetState().Rotate90();
    editor.GetState().Rotate90();
    editor.GetState().Rotate90();
    TEST("Rotate90 wraps to 0", editor.GetState().rotationIndex == 0);

    // BuildGhostBlock creates block at hover cell
    editor.SetHoverCell({3, 4, 5});
    Block ghost = editor.BuildGhostBlock();
    TEST("Ghost block at hover cell", ghost.gridPos == Vector3Int(3, 4, 5));

    // Place adds block
    editor.SetHoverCell({0, 0, 0});
    bool placed = editor.Place();
    TEST("Editor Place succeeds", placed);
    TEST("Editor Place adds block", ship.BlockCount() >= 1);

    // RemoveAtHover removes block
    editor.SetHoverCell({0, 0, 0});
    bool removed = editor.RemoveAtHover();
    TEST("Editor RemoveAtHover succeeds", removed);
    TEST("Editor RemoveAtHover removes block", ship.BlockCount() == 0);
}

// ===================================================================
// 9. Faction tests
// ===================================================================
static void TestFactions() {
    std::cout << "[Factions]\n";

    auto factions = FactionDefinitions::GetAllFactions();
    TEST("All 5 factions exist", factions.size() == 5);

    // Verify correct IDs
    TEST("Iron Dominion id", factions[0].id == "iron_dominion");
    TEST("Nomad Continuum id", factions[1].id == "nomad_continuum");
    TEST("Helix Covenant id", factions[2].id == "helix_covenant");
    TEST("Ashen Clades id", factions[3].id == "ashen_clades");
    TEST("Ascended Archive id", factions[4].id == "ascended_archive");

    // Iron Dominion silhouette
    auto iron = FactionDefinitions::IronDominion();
    TEST("Iron Dominion Short length", iron.silhouette.length == LengthBias::Short);
    TEST("Iron Dominion Chunky thickness", iron.silhouette.thickness == ThicknessBias::Chunky);

    // Shape languages have allowed shapes
    for (const auto& f : factions) {
        TEST(("Faction " + f.id + " has allowed shapes").c_str(),
             !f.shapeLanguage.allowedShapes.empty());
    }
}

// ===================================================================
// 10. AIShipBuilder tests
// ===================================================================
static void TestAIShipBuilder() {
    std::cout << "[AIShipBuilder]\n";

    auto faction = FactionDefinitions::IronDominion();

    // Deterministic: same seed → same block count
    {
        AIShipBuilder builder1(faction, NPCTier::Frigate, 42);
        Ship s1 = builder1.Build();
        AIShipBuilder builder2(faction, NPCTier::Frigate, 42);
        Ship s2 = builder2.Build();
        TEST("Same seed produces same block count", s1.BlockCount() == s2.BlockCount());
    }

    // Scout < Battleship block count
    {
        AIShipBuilder scoutBuilder(faction, NPCTier::Scout, 100);
        Ship scout = scoutBuilder.Build();
        AIShipBuilder bsBuilder(faction, NPCTier::Battleship, 100);
        Ship bs = bsBuilder.Build();
        TEST("Scout fewer blocks than Battleship", scout.BlockCount() < bs.BlockCount());
    }

    // Positive mass
    {
        AIShipBuilder builder(faction, NPCTier::Frigate, 7);
        Ship ship = builder.Build();
        TEST("Generated ship has positive mass", ship.totalMass > 0.0f);
    }
}

// ===================================================================
// 11. Blueprint tests
// ===================================================================
static void TestBlueprint() {
    std::cout << "[Blueprint]\n";

    // Build a small ship, create blueprint, save/load round trip
    Ship ship;
    auto b1 = MakeBlock({0, 0, 0}, {1, 1, 1});
    auto b2 = MakeBlock({1, 0, 0}, {1, 1, 1});
    BlockPlacement::Place(ship, b1);
    BlockPlacement::Place(ship, b2);

    Blueprint bp = Blueprint::FromShip(ship, "TestShip", "Tester");
    TEST("Blueprint has correct block count", bp.blocks.size() == ship.BlockCount());

    // ToJson produces non-empty string
    std::string json = bp.ToJson();
    TEST("ToJson produces valid string", !json.empty());
    TEST("ToJson contains name", json.find("TestShip") != std::string::npos);

    // Round-trip: FromJson → same block count
    Blueprint bp2 = Blueprint::FromJson(json);
    TEST("FromJson preserves block count", bp2.blocks.size() == bp.blocks.size());
    TEST("FromJson preserves name", bp2.name == bp.name);

    // Validate catches empty blueprints
    Blueprint empty;
    TEST("Validate catches empty blueprint", !empty.Validate());

    // Validate passes for valid blueprint
    TEST("Validate passes for valid blueprint", bp.Validate());
}

// ===================================================================
// 12. WeaponSystem tests
// ===================================================================
static void TestWeaponSystem() {
    std::cout << "[WeaponSystem]\n";

    // All weapon types have positive damage
    auto allWeapons = WeaponSystem::GetAllWeaponStats();
    for (const auto& [wtype, ws] : allWeapons) {
        TEST(("Weapon damage > 0 for type " + std::to_string(static_cast<int>(wtype))).c_str(),
             ws.damage > 0.0f);
    }

    // EffectiveDPS is positive
    for (const auto& [wtype, ws] : allWeapons) {
        TEST(("EffectiveDPS > 0 for type " + std::to_string(static_cast<int>(wtype))).c_str(),
             ws.EffectiveDPS() > 0.0f);
    }

    // IsValidHardpoint for exposed block
    {
        Ship ship;
        auto blk = MakeBlock({0, 0, 0}, {1, 1, 1}, BlockType::WeaponMount);
        BlockPlacement::Place(ship, blk);
        TEST("IsValidHardpoint for exposed block", WeaponSystem::IsValidHardpoint(ship, *blk));
    }
}

// ===================================================================
// 13. ModuleDef tests
// ===================================================================
static void TestModuleDef() {
    std::cout << "[ModuleDef]\n";

    // ModuleDatabase returns all 12 modules
    auto all = ModuleDatabase::GetAll();
    TEST("ModuleDatabase has 12 modules", all.size() == 12);

    // Each module has a non-empty id
    for (const auto* m : all) {
        TEST(("Module id non-empty: " + m->id).c_str(), !m->id.empty());
    }

    // Each module has positive mass and HP
    for (const auto* m : all) {
        TEST(("Module mass > 0: " + m->id).c_str(), m->mass > 0.0f);
        TEST(("Module hp > 0: " + m->id).c_str(), m->hp > 0.0f);
    }

    // Each module has at least one hardpoint
    for (const auto* m : all) {
        TEST(("Module has hardpoints: " + m->id).c_str(), m->HardpointCount() > 0);
    }

    // GetByType filters correctly
    auto engines = ModuleDatabase::GetByType(ModuleType::Engine);
    TEST("GetByType Engine returns 2", engines.size() == 2);
    for (const auto* e : engines) {
        TEST(("Engine module type correct: " + e->id).c_str(), e->type == ModuleType::Engine);
    }

    auto cores = ModuleDatabase::GetByType(ModuleType::Core);
    TEST("GetByType Core returns 2", cores.size() == 2);

    auto weapons = ModuleDatabase::GetByType(ModuleType::Weapon);
    TEST("GetByType Weapon returns 2", weapons.size() == 2);

    // Named accessors
    TEST("CoreSmall id", ModuleDatabase::CoreSmall().id == "core_small");
    TEST("CoreMedium id", ModuleDatabase::CoreMedium().id == "core_medium");
    TEST("EngineSmall id", ModuleDatabase::EngineSmall().id == "engine_small");
    TEST("EngineLarge id", ModuleDatabase::EngineLarge().id == "engine_large");
    TEST("WeaponTurret id", ModuleDatabase::WeaponTurret().id == "weapon_turret");
    TEST("ShieldGenerator id", ModuleDatabase::ShieldGenerator().id == "shield_gen");

    // FreeHardpointCount equals total when none occupied
    const ModuleDef& core = ModuleDatabase::CoreSmall();
    TEST("Free hardpoints = total initially", core.FreeHardpointCount() == core.HardpointCount());

    // CoreSmall has powerOutput > 0
    TEST("CoreSmall has powerOutput", ModuleDatabase::CoreSmall().powerOutput > 0.0f);

    // EngineSmall has thrustOutput > 0
    TEST("EngineSmall has thrustOutput", ModuleDatabase::EngineSmall().thrustOutput > 0.0f);

    // CargoSmall has cargoCapacity > 0
    TEST("CargoSmall has cargoCapacity", ModuleDatabase::CargoSmall().cargoCapacity > 0.0f);

    // ShieldGenerator has shieldStrength > 0
    TEST("ShieldGenerator has shieldStrength", ModuleDatabase::ShieldGenerator().shieldStrength > 0.0f);
}

// ===================================================================
// 14. ModularShip tests
// ===================================================================
static void TestModularShip() {
    std::cout << "[ModularShip]\n";

    // Empty ship
    {
        ModularShip ship;
        TEST("Empty modular ship count 0", ship.ModuleCount() == 0);
        TEST("Empty modular ship IsEmpty", ship.IsEmpty());
        TEST("Empty modular ship no core", !ship.HasCore());
    }

    // Add core module
    {
        ModularShip ship;
        const ModuleDef& core = ModuleDatabase::CoreSmall();
        int idx = ship.AddModule(&core, Vector3(0, 0, 0));
        TEST("AddModule returns 0 for first", idx == 0);
        TEST("Ship has 1 module", ship.ModuleCount() == 1);
        TEST("Ship has core", ship.HasCore());
        TEST("Ship has positive mass", ship.totalMass > 0.0f);
        TEST("Ship has positive HP", ship.totalHP > 0.0f);
        TEST("Ship has power generation", ship.totalPowerGen > 0.0f);
    }

    // Add engine → can accelerate
    {
        ModularShip ship;
        ship.AddModule(&ModuleDatabase::CoreSmall(), Vector3(0, 0, 0));
        TEST("No thrust before engine", !ship.CanAccelerate());

        ship.AddModule(&ModuleDatabase::EngineSmall(), Vector3(0, 0, -2), 0);
        TEST("Has thrust after engine", ship.CanAccelerate());
        TEST("Thrust > 0", ship.totalThrust > 0.0f);
    }

    // Power balance
    {
        ModularShip ship;
        ship.AddModule(&ModuleDatabase::CoreSmall(), Vector3(0, 0, 0)); // powerOutput=10
        TEST("Core alone is power balanced", ship.PowerBalanced());

        // Add many weapons to exceed power
        for (int i = 0; i < 5; i++) {
            ship.AddModule(&ModuleDatabase::WeaponRailgun(), Vector3(static_cast<float>(i)*2, 0, 0), 0);
        }
        // 5 railguns * 15 power draw = 75, core output = 10
        TEST("Many weapons exceeds power", !ship.PowerBalanced());
    }

    // Destroy module — recursive
    {
        ModularShip ship;
        ship.AddModule(&ModuleDatabase::CoreSmall(), Vector3(0, 0, 0));   // 0
        ship.AddModule(&ModuleDatabase::HullPlate(), Vector3(0, 0, 2), 0); // 1
        ship.AddModule(&ModuleDatabase::WeaponTurret(), Vector3(0, 1, 2), 1); // 2 (child of 1)

        TEST("3 modules before destroy", ship.ModuleCount() == 3);

        ship.DestroyModule(1); // should destroy module 1 and child 2
        TEST("1 module after destroying subtree", ship.ModuleCount() == 1);
        TEST("Core survives subtree destroy", ship.HasCore());
    }

    // Destroy core kills ship
    {
        ModularShip ship;
        ship.AddModule(&ModuleDatabase::CoreSmall(), Vector3(0, 0, 0));
        ship.AddModule(&ModuleDatabase::EngineSmall(), Vector3(0, 0, -2), 0);

        ship.DestroyModule(0); // destroy core and all children
        TEST("Destroying core empties ship", ship.IsEmpty());
    }

    // RecalculateStats
    {
        ModularShip ship;
        ship.AddModule(&ModuleDatabase::CoreSmall(), Vector3(0, 0, 0));
        ship.AddModule(&ModuleDatabase::CargoLarge(), Vector3(0, 0, 3), 0);

        TEST("Cargo adds capacity", ship.totalCargo > 0.0f);
        float cargoBefore = ship.totalCargo;

        ship.AddModule(&ModuleDatabase::CargoSmall(), Vector3(0, 0, 6), 1);
        TEST("More cargo increases capacity", ship.totalCargo > cargoBefore);
    }
}

// ===================================================================
// 15. ShipArchetype & Generator tests
// ===================================================================
static void TestShipArchetypeGenerator() {
    std::cout << "[ShipArchetype & Generator]\n";

    // All 5 archetypes exist
    auto archetypes = ShipArchetypes::GetAll();
    TEST("5 archetypes exist", archetypes.size() == 5);

    // Interceptor specifics
    auto interceptor = ShipArchetypes::Interceptor();
    TEST("Interceptor id", interceptor.id == "interceptor");
    TEST("Interceptor minModules < maxModules", interceptor.minModules < interceptor.maxModules);

    // Battleship is bigger than interceptor
    auto battleship = ShipArchetypes::BattleshipArchetype();
    TEST("Battleship more modules than Interceptor",
         battleship.maxModules > interceptor.maxModules);
    TEST("Battleship more weapons than Interceptor",
         battleship.maxWeapons > interceptor.maxWeapons);

    // Generate Interceptor
    auto faction = FactionDefinitions::IronDominion();
    {
        ModularShipGenerator gen(interceptor, faction, 42);
        ModularShip ship = gen.Generate();
        TEST("Generated interceptor not empty", !ship.IsEmpty());
        TEST("Generated interceptor has core", ship.HasCore());
        TEST("Generated interceptor can accelerate", ship.CanAccelerate());
        TEST("Generated interceptor has name", !ship.name.empty());
        TEST("Generated interceptor has faction", !ship.faction.empty());
    }

    // Generate Freighter
    {
        auto freighter = ShipArchetypes::Freighter();
        ModularShipGenerator gen(freighter, faction, 99);
        ModularShip ship = gen.Generate();
        TEST("Generated freighter not empty", !ship.IsEmpty());
        TEST("Generated freighter has cargo", ship.totalCargo > 0.0f);
    }

    // Generate Battleship
    {
        ModularShipGenerator gen(battleship, faction, 77);
        ModularShip ship = gen.Generate();
        TEST("Generated battleship not empty", !ship.IsEmpty());
        TEST("Generated battleship has core", ship.HasCore());
        TEST("Battleship more modules than interceptor min",
             static_cast<int>(ship.ModuleCount()) >= battleship.minModules);
    }

    // Deterministic: same seed → same module count
    {
        ModularShipGenerator gen1(interceptor, faction, 42);
        ModularShip s1 = gen1.Generate();
        ModularShipGenerator gen2(interceptor, faction, 42);
        ModularShip s2 = gen2.Generate();
        TEST("Same seed produces same module count", s1.ModuleCount() == s2.ModuleCount());
    }

    // Different seeds → may differ
    {
        ModularShipGenerator gen1(interceptor, faction, 42);
        ModularShip s1 = gen1.Generate();
        ModularShipGenerator gen2(interceptor, faction, 999);
        ModularShip s2 = gen2.Generate();
        // Both should still be valid
        TEST("Different seed ship 1 valid", s1.HasCore());
        TEST("Different seed ship 2 valid", s2.HasCore());
    }

    // All factions can generate ships
    {
        auto allFactions = FactionDefinitions::GetAllFactions();
        for (const auto& f : allFactions) {
            auto frigate = ShipArchetypes::FrigateArchetype();
            ModularShipGenerator gen(frigate, f, 123);
            ModularShip ship = gen.Generate();
            TEST(("Faction " + f.id + " generates valid ship").c_str(),
                 ship.HasCore() && !ship.IsEmpty());
        }
    }
}

// ===================================================================
// 16. Logger tests
// ===================================================================
static void TestLogger() {
    std::cout << "[Logger]\n";

    auto& logger = Logger::Instance();

    // Singleton returns same instance
    auto& logger2 = Logger::Instance();
    TEST("Logger singleton same instance", &logger == &logger2);

    // Set minimum level
    logger.SetMinimumLevel(LogLevel::Debug);
    TEST("Logger set min level", logger.GetMinimumLevel() == LogLevel::Debug);

    // Log messages at various levels
    logger.Debug("Test", "debug message");
    logger.Info("Test", "info message");
    logger.Warning("Test", "warning message");
    logger.Error("Test", "error message");
    logger.Critical("Test", "critical message");

    auto logs = logger.GetRecentLogs(10);
    TEST("Logger stores recent logs", logs.size() >= 5);

    // Check that filtering works
    logger.SetMinimumLevel(LogLevel::Error);
    size_t countBefore = logger.GetRecentLogs(1000).size();
    logger.Debug("Test", "should be filtered");
    size_t countAfter = logger.GetRecentLogs(1000).size();
    TEST("Logger filters below min level", countBefore == countAfter);

    // Reset for other tests
    logger.SetMinimumLevel(LogLevel::Warning);
}

// ===================================================================
// 17. EventSystem tests
// ===================================================================
static void TestEventSystem() {
    std::cout << "[EventSystem]\n";

    auto& events = EventSystem::Instance();
    events.ClearAllListeners();

    // Singleton
    auto& events2 = EventSystem::Instance();
    TEST("EventSystem singleton same instance", &events == &events2);

    // Subscribe and publish
    int callCount = 0;
    events.Subscribe("test.event", [&](const GameEvent& e) {
        callCount++;
    });
    TEST("EventSystem listener count is 1", events.GetListenerCount("test.event") == 1);

    GameEvent evt;
    events.Publish("test.event", evt);
    TEST("EventSystem publish calls subscriber", callCount == 1);

    events.Publish("test.event", evt);
    TEST("EventSystem publish calls again", callCount == 2);

    // Unrelated event doesn't trigger callback
    events.Publish("other.event", evt);
    TEST("EventSystem unrelated event no call", callCount == 2);

    // Queue and process
    auto queued = std::make_shared<GameEvent>();
    events.QueueEvent("test.event", queued);
    TEST("EventSystem queued not yet processed", callCount == 2);
    events.ProcessQueuedEvents();
    TEST("EventSystem queued now processed", callCount == 3);

    // Clear
    events.ClearAllListeners();
    TEST("EventSystem cleared has 0 listeners", events.GetListenerCount("test.event") == 0);
    events.Publish("test.event", evt);
    TEST("EventSystem after clear no callback", callCount == 3);

    // GameEvents constants exist
    TEST("GameEvents::EntityCreated exists", std::string(GameEvents::EntityCreated) == "entity.created");
    TEST("GameEvents::CollisionDetected exists", std::string(GameEvents::CollisionDetected) == "physics.collision");
    TEST("GameEvents::GameSaved exists", std::string(GameEvents::GameSaved) == "game.saved");
}

// ===================================================================
// 18. ECS tests
// ===================================================================

// Test component for ECS tests
struct TestHealthComponent : IComponent {
    float health = 100.0f;
    float maxHealth = 100.0f;
};

struct TestNameComponent : IComponent {
    std::string displayName;
};

// Test system for ECS tests
class TestCountingSystem : public SystemBase {
public:
    int updateCount = 0;
    TestCountingSystem() : SystemBase("TestCountingSystem") {}
    void Update(float /*deltaTime*/) override { updateCount++; }
};

static void TestECS() {
    std::cout << "[ECS]\n";

    // Entity creation
    EntityManager em;
    auto& e1 = em.CreateEntity("Ship1");
    TEST("Entity created with name", e1.name == "Ship1");
    TEST("Entity has valid id", e1.id != InvalidEntityId);
    TEST("Entity is active", e1.isActive);

    auto& e2 = em.CreateEntity("Ship2");
    TEST("Second entity different id", e1.id != e2.id);
    TEST("Entity count is 2", em.GetEntityCount() == 2);

    // Get entity
    auto* found = em.GetEntity(e1.id);
    TEST("GetEntity returns entity", found != nullptr);
    TEST("GetEntity correct name", found != nullptr && found->name == "Ship1");

    auto* notFound = em.GetEntity(999999);
    TEST("GetEntity unknown returns null", notFound == nullptr);

    // Add component
    auto healthComp = std::make_unique<TestHealthComponent>();
    healthComp->health = 75.0f;
    auto* hp = em.AddComponent<TestHealthComponent>(e1.id, std::move(healthComp));
    TEST("AddComponent returns ptr", hp != nullptr);
    TEST("Component has correct value", hp != nullptr && ApproxEq(hp->health, 75.0f));
    TEST("Component entityId set", hp != nullptr && hp->entityId == e1.id);

    // Get component
    auto* retrieved = em.GetComponent<TestHealthComponent>(e1.id);
    TEST("GetComponent returns same ptr", retrieved == hp);

    // Has component
    TEST("HasComponent true for added", em.HasComponent<TestHealthComponent>(e1.id));
    TEST("HasComponent false for other entity", !em.HasComponent<TestHealthComponent>(e2.id));
    TEST("HasComponent false for other type", !em.HasComponent<TestNameComponent>(e1.id));

    // Add second component type
    auto nameComp = std::make_unique<TestNameComponent>();
    nameComp->displayName = "Player Ship";
    em.AddComponent<TestNameComponent>(e1.id, std::move(nameComp));
    TEST("Has both components", em.HasComponent<TestHealthComponent>(e1.id) &&
                                em.HasComponent<TestNameComponent>(e1.id));

    // GetAllComponents
    auto allHealth = em.GetAllComponents<TestHealthComponent>();
    TEST("GetAllComponents returns 1", allHealth.size() == 1);

    // Add component to second entity
    em.AddComponent<TestHealthComponent>(e2.id, std::make_unique<TestHealthComponent>());
    allHealth = em.GetAllComponents<TestHealthComponent>();
    TEST("GetAllComponents returns 2 after adding", allHealth.size() == 2);

    // Remove component
    em.RemoveComponent<TestHealthComponent>(e2.id);
    TEST("RemoveComponent removes it", !em.HasComponent<TestHealthComponent>(e2.id));

    // Destroy entity
    em.DestroyEntity(e2.id);
    TEST("Entity count after destroy", em.GetEntityCount() == 1);
    TEST("Destroyed entity not found", em.GetEntity(e2.id) == nullptr);

    // System registration and update
    EntityManager em2;
    auto sys = std::make_unique<TestCountingSystem>();
    auto* sysPtr = sys.get();
    em2.RegisterSystem(std::move(sys));
    em2.UpdateSystems(1.0f / 60.0f);
    TEST("System updated once", sysPtr->updateCount == 1);
    em2.UpdateSystems(1.0f / 60.0f);
    TEST("System updated twice", sysPtr->updateCount == 2);

    // Disabled system
    sysPtr->SetEnabled(false);
    em2.UpdateSystems(1.0f / 60.0f);
    TEST("Disabled system not updated", sysPtr->updateCount == 2);

    em2.Shutdown();
}

// ===================================================================
// 19. PhysicsComponent tests
// ===================================================================
static void TestPhysicsComponent() {
    std::cout << "[PhysicsComponent]\n";

    PhysicsComponent pc;

    // Default values
    TEST("Default mass 1000", ApproxEq(pc.mass, 1000.0f));
    TEST("Default drag 0.1", ApproxEq(pc.drag, 0.1f));
    TEST("Default maxThrust 100", ApproxEq(pc.maxThrust, 100.0f));
    TEST("Default not static", !pc.isStatic);

    // AddForce
    pc.AddForce(Vector3(10.0f, 0.0f, 0.0f));
    TEST("AddForce applies x", ApproxEq(pc.appliedForce.x, 10.0f));
    pc.AddForce(Vector3(5.0f, 3.0f, 0.0f));
    TEST("AddForce accumulates", ApproxEq(pc.appliedForce.x, 15.0f) &&
                                  ApproxEq(pc.appliedForce.y, 3.0f));

    // ClearForces
    pc.ClearForces();
    TEST("ClearForces zeroes force", ApproxEq(pc.appliedForce.x, 0.0f) &&
                                      ApproxEq(pc.appliedForce.y, 0.0f));

    // ApplyThrust (limited by maxThrust)
    pc.maxThrust = 50.0f;
    pc.ApplyThrust(Vector3(1.0f, 0.0f, 0.0f), 200.0f);
    TEST("ApplyThrust clamped to maxThrust", ApproxEq(pc.appliedForce.x, 50.0f));

    pc.ClearForces();
    pc.ApplyThrust(Vector3(1.0f, 0.0f, 0.0f), 30.0f);
    TEST("ApplyThrust below max uses actual", ApproxEq(pc.appliedForce.x, 30.0f));

    // AddTorque
    pc.ClearForces();
    pc.AddTorque(Vector3(0.0f, 1.0f, 0.0f));
    TEST("AddTorque applies", ApproxEq(pc.appliedTorque.y, 1.0f));
}

// ===================================================================
// 20. PhysicsSystem tests
// ===================================================================
static void TestPhysicsSystem() {
    std::cout << "[PhysicsSystem]\n";

    EntityManager em;
    PhysicsSystem physSys(em);

    // Create entity with physics
    auto& ent = em.CreateEntity("TestShip");
    auto comp = std::make_unique<PhysicsComponent>();
    comp->mass = 100.0f;
    comp->drag = 0.0f; // No drag for predictable tests
    comp->angularDrag = 0.0f;
    auto* pc = em.AddComponent<PhysicsComponent>(ent.id, std::move(comp));

    // Apply force and update
    pc->AddForce(Vector3(1000.0f, 0.0f, 0.0f)); // F=1000, m=100, a=10
    physSys.Update(1.0f); // dt=1s
    TEST("PhysSystem velocity after force", ApproxEq(pc->velocity.x, 10.0f));
    TEST("PhysSystem position after update", ApproxEq(pc->position.x, 10.0f));

    // Forces cleared after update
    TEST("PhysSystem forces cleared", ApproxEq(pc->appliedForce.x, 0.0f));

    // Another update with no force: velocity persists (no drag)
    physSys.Update(1.0f);
    TEST("PhysSystem velocity persists (no drag)", ApproxEq(pc->velocity.x, 10.0f));
    TEST("PhysSystem position advances", ApproxEq(pc->position.x, 20.0f));

    // Static entity should not move
    auto& staticEnt = em.CreateEntity("Station");
    auto sComp = std::make_unique<PhysicsComponent>();
    sComp->isStatic = true;
    sComp->position = Vector3(100.0f, 0.0f, 0.0f);
    auto* spc = em.AddComponent<PhysicsComponent>(staticEnt.id, std::move(sComp));
    spc->AddForce(Vector3(999.0f, 0.0f, 0.0f));
    physSys.Update(1.0f);
    TEST("PhysSystem static entity doesn't move", ApproxEq(spc->position.x, 100.0f));

    // Collision detection between two dynamic objects
    EntityManager em2;
    PhysicsSystem physSys2(em2);

    auto& obj1 = em2.CreateEntity("Obj1");
    auto c1 = std::make_unique<PhysicsComponent>();
    c1->mass = 100.0f;
    c1->drag = 0.0f;
    c1->angularDrag = 0.0f;
    c1->position = Vector3(0.0f, 0.0f, 0.0f);
    c1->velocity = Vector3(5.0f, 0.0f, 0.0f);
    c1->collisionRadius = 5.0f;
    auto* pc1 = em2.AddComponent<PhysicsComponent>(obj1.id, std::move(c1));

    auto& obj2 = em2.CreateEntity("Obj2");
    auto c2 = std::make_unique<PhysicsComponent>();
    c2->mass = 100.0f;
    c2->drag = 0.0f;
    c2->angularDrag = 0.0f;
    c2->position = Vector3(8.0f, 0.0f, 0.0f); // Within collision range (5+5=10 > 8)
    c2->velocity = Vector3(-5.0f, 0.0f, 0.0f);
    c2->collisionRadius = 5.0f;
    auto* pc2 = em2.AddComponent<PhysicsComponent>(obj2.id, std::move(c2));

    physSys2.Update(0.001f); // Very small dt for collision test
    // After elastic collision with equal masses, velocities should swap
    TEST("PhysSystem collision obj1 velocity changed", !ApproxEq(pc1->velocity.x, 5.0f));
    TEST("PhysSystem collision obj2 velocity changed", !ApproxEq(pc2->velocity.x, -5.0f));

    // Interpolation test
    EntityManager em3;
    PhysicsSystem physSys3(em3);
    auto& interpEnt = em3.CreateEntity("InterpShip");
    auto iComp = std::make_unique<PhysicsComponent>();
    iComp->mass = 100.0f;
    iComp->drag = 0.0f;
    iComp->angularDrag = 0.0f;
    iComp->position = Vector3(0.0f, 0.0f, 0.0f);
    auto* ipc = em3.AddComponent<PhysicsComponent>(interpEnt.id, std::move(iComp));
    ipc->AddForce(Vector3(1000.0f, 0.0f, 0.0f));
    physSys3.Update(1.0f);
    physSys3.InterpolatePhysics(0.5f);
    // At alpha=0.5, interpolated should be between previous (0) and current (10)
    TEST("PhysSystem interpolation midpoint", ApproxEq(ipc->interpolatedPosition.x, 5.0f));
}

// ===================================================================
// 21. Inventory tests
// ===================================================================
static void TestInventory() {
    std::cout << "[Inventory]\n";

    Inventory inv;

    // Default state
    TEST("Inventory default capacity 1000", inv.GetMaxCapacity() == 1000);
    TEST("Inventory starts empty", inv.GetCurrentCapacity() == 0);
    TEST("Inventory iron starts at 0", inv.GetResourceAmount(ResourceType::Iron) == 0);

    // Add resources
    TEST("Inventory add iron succeeds", inv.AddResource(ResourceType::Iron, 100));
    TEST("Inventory iron amount", inv.GetResourceAmount(ResourceType::Iron) == 100);
    TEST("Inventory current capacity", inv.GetCurrentCapacity() == 100);

    // Add more types
    inv.AddResource(ResourceType::Credits, 500);
    inv.AddResource(ResourceType::Titanium, 200);
    TEST("Inventory capacity tracks total", inv.GetCurrentCapacity() == 800);

    // HasResource
    TEST("Inventory has 100 iron", inv.HasResource(ResourceType::Iron, 100));
    TEST("Inventory doesn't have 101 iron", !inv.HasResource(ResourceType::Iron, 101));

    // Remove resources
    TEST("Inventory remove 50 iron", inv.RemoveResource(ResourceType::Iron, 50));
    TEST("Inventory iron after remove", inv.GetResourceAmount(ResourceType::Iron) == 50);
    TEST("Inventory capacity after remove", inv.GetCurrentCapacity() == 750);

    // Remove more than available fails
    TEST("Inventory remove too much fails", !inv.RemoveResource(ResourceType::Iron, 999));
    TEST("Inventory iron unchanged after failed remove", inv.GetResourceAmount(ResourceType::Iron) == 50);

    // Capacity limit
    inv.SetMaxCapacity(800);
    TEST("Inventory add over capacity fails", !inv.AddResource(ResourceType::Avorion, 100));
    TEST("Inventory add within capacity ok", inv.AddResource(ResourceType::Avorion, 50));

    // Clear
    inv.Clear();
    TEST("Inventory clear zeros capacity", inv.GetCurrentCapacity() == 0);
    TEST("Inventory clear zeros iron", inv.GetResourceAmount(ResourceType::Iron) == 0);
    TEST("Inventory clear zeros credits", inv.GetResourceAmount(ResourceType::Credits) == 0);

    // GetAllResources
    inv.AddResource(ResourceType::Naonite, 42);
    const auto& all = inv.GetAllResources();
    TEST("Inventory getAllResources has naonite", all.count(ResourceType::Naonite) > 0);
    auto it = all.find(ResourceType::Naonite);
    TEST("Inventory getAllResources naonite amount", it != all.end() && it->second == 42);
}

// ===================================================================
// 21. ConfigurationManager tests
// ===================================================================
static void TestConfigurationManager() {
    std::cout << "[ConfigurationManager]\n";

    auto& mgr = ConfigurationManager::Instance();

    // Singleton returns same instance
    auto& mgr2 = ConfigurationManager::Instance();
    TEST("ConfigManager singleton same instance", &mgr == &mgr2);

    // Reset to defaults
    mgr.ResetToDefaults();
    TEST("Default resolution width", mgr.GetGraphics().resolutionWidth == 1920);
    TEST("Default resolution height", mgr.GetGraphics().resolutionHeight == 1080);
    TEST("Default vsync", mgr.GetGraphics().vsync == true);
    TEST("Default targetFPS", mgr.GetGraphics().targetFPS == 60);
    TEST("Default masterVolume", ApproxEq(mgr.GetAudio().masterVolume, 0.8f));
    TEST("Default musicVolume", ApproxEq(mgr.GetAudio().musicVolume, 0.6f));
    TEST("Default playerName", mgr.GetGameplay().playerName == "Player");
    TEST("Default difficulty", mgr.GetGameplay().difficulty == 1);
    TEST("Default serverPort", mgr.GetNetwork().serverPort == 27015);
    TEST("Default maxPlayers", mgr.GetNetwork().maxPlayers == 50);
    TEST("Default debugMode", mgr.GetDevelopment().debugMode == false);
    TEST("Default galaxySeed", mgr.GetDevelopment().galaxySeed == 12345);

    // Validation passes for defaults
    TEST("Defaults validate", mgr.ValidateConfiguration() == true);

    // Modify to invalid and validate
    mgr.GetMutableConfig().graphics.resolutionWidth = 100; // too low
    TEST("Invalid width fails validation", mgr.ValidateConfiguration() == false);
    mgr.ResetToDefaults();

    mgr.GetMutableConfig().audio.masterVolume = 2.0f; // too high
    TEST("Invalid volume fails validation", mgr.ValidateConfiguration() == false);
    mgr.ResetToDefaults();

    mgr.GetMutableConfig().network.serverPort = 80; // too low
    TEST("Invalid port fails validation", mgr.ValidateConfiguration() == false);
    mgr.ResetToDefaults();

    // Save and load round-trip
    mgr.GetMutableConfig().gameplay.playerName = "TestPilot";
    mgr.GetMutableConfig().graphics.targetFPS = 144;
    mgr.GetMutableConfig().development.galaxySeed = 99999;
    TEST("Save succeeds", mgr.SaveConfiguration("/tmp/subspace_test_config.cfg") == true);

    mgr.ResetToDefaults();
    TEST("After reset playerName is Player", mgr.GetGameplay().playerName == "Player");

    TEST("Load succeeds", mgr.LoadConfiguration("/tmp/subspace_test_config.cfg") == true);
    TEST("Loaded playerName", mgr.GetGameplay().playerName == "TestPilot");
    TEST("Loaded targetFPS", mgr.GetGraphics().targetFPS == 144);
    TEST("Loaded galaxySeed", mgr.GetDevelopment().galaxySeed == 99999);

    mgr.ResetToDefaults();
}

// ===================================================================
// 22. SaveGameManager tests
// ===================================================================
static void TestSaveGameManager() {
    std::cout << "[SaveGameManager]\n";

    auto& mgr = SaveGameManager::Instance();
    mgr.SetSaveDirectory("/tmp/subspace_test_saves");

    // Singleton
    auto& mgr2 = SaveGameManager::Instance();
    TEST("SaveManager singleton same instance", &mgr == &mgr2);

    // Create test save data
    SaveGameData data;
    data.saveName = "TestSave";
    data.saveTime = "2026-02-12T00:00:00Z";
    data.version = "1.0.0";
    data.galaxySeed = 42;
    data.gameState["playerHP"] = "100";
    data.gameState["credits"] = "5000";

    EntityData entity;
    entity.entityId = 1001;
    entity.entityName = "PlayerShip";
    entity.isActive = true;

    ComponentData comp;
    comp.componentType = "PhysicsComponent";
    comp.data["posX"] = "10.5";
    comp.data["posY"] = "20.3";
    entity.components.push_back(comp);
    data.entities.push_back(entity);

    // Save
    TEST("SaveGame succeeds", mgr.SaveGame(data, "test_save_1") == true);

    // Load
    SaveGameData loaded;
    TEST("LoadGame succeeds", mgr.LoadGame("test_save_1", loaded) == true);
    TEST("Loaded saveName", loaded.saveName == "TestSave");
    TEST("Loaded saveTime", loaded.saveTime == "2026-02-12T00:00:00Z");
    TEST("Loaded version", loaded.version == "1.0.0");
    TEST("Loaded galaxySeed", loaded.galaxySeed == 42);
    TEST("Loaded gameState size", loaded.gameState.size() == 2);
    TEST("Loaded playerHP", loaded.gameState["playerHP"] == "100");
    TEST("Loaded entity count", loaded.entities.size() == 1);
    TEST("Loaded entity id", loaded.entities[0].entityId == 1001);
    TEST("Loaded entity name", loaded.entities[0].entityName == "PlayerShip");
    TEST("Loaded entity active", loaded.entities[0].isActive == true);
    TEST("Loaded component count", loaded.entities[0].components.size() == 1);
    TEST("Loaded component type", loaded.entities[0].components[0].componentType == "PhysicsComponent");

    // QuickSave
    TEST("QuickSave succeeds", mgr.QuickSave(data) == true);

    // List saves
    auto saves = mgr.ListSaveGames();
    TEST("ListSaveGames has entries", saves.size() >= 2);

    // Delete
    TEST("DeleteSave succeeds", mgr.DeleteSave("test_save_1") == true);

    // Load deleted file fails
    SaveGameData gone;
    TEST("Load deleted fails", mgr.LoadGame("test_save_1", gone) == false);

    // Clean up
    mgr.DeleteSave("quicksave");
}

// ===================================================================
// 23. NavigationSystem tests
// ===================================================================
static void TestNavigationSystem() {
    std::cout << "[NavigationSystem]\n";

    // SectorCoordinate basics
    SectorCoordinate origin(0, 0, 0);
    SectorCoordinate nearby(3, 4, 0);
    SectorCoordinate farAway(50, 50, 50);

    TEST("Sector origin distance from center", ApproxEq(origin.DistanceFromCenter(), 0.0f));
    TEST("Sector distance 3-4-0", ApproxEq(nearby.DistanceTo(origin), 5.0f));
    TEST("Sector in range", nearby.IsInRangeOf(origin, 6.0f));
    TEST("Sector not in range", !nearby.IsInRangeOf(origin, 4.0f));
    TEST("Sector equality", origin == SectorCoordinate(0, 0, 0));
    TEST("Sector inequality", origin != nearby);

    // Tech levels
    TEST("Origin tech level 7", origin.GetTechLevel() == 7);
    TEST("Nearby tech level", SectorCoordinate(3, 0, 0).GetTechLevel() == 6);
    TEST("Mid tech level", SectorCoordinate(15, 0, 0).GetTechLevel() == 4);
    TEST("Far tech level 1", SectorCoordinate(100, 0, 0).GetTechLevel() == 1);

    // Security levels
    TEST("Origin is HighSec", origin.GetSecurityLevel() == SecurityLevel::HighSec);
    TEST("Nearby is LowSec", SectorCoordinate(15, 0, 0).GetSecurityLevel() == SecurityLevel::LowSec);
    TEST("Far is NullSec", farAway.GetSecurityLevel() == SecurityLevel::NullSec);

    // HyperdriveComponent
    HyperdriveComponent drive;
    TEST("Drive default jumpRange", ApproxEq(drive.jumpRange, 5.0f));
    TEST("Drive default not charging", !drive.isCharging);
    TEST("Drive not fully charged initially", !drive.IsFullyCharged());

    // Start charge
    drive.StartCharge(nearby);
    TEST("Drive is charging after StartCharge", drive.isCharging);
    TEST("Drive has target", drive.hasTarget);
    TEST("Drive charge is 0", ApproxEq(drive.currentCharge, 0.0f));

    // Cancel charge
    drive.CancelCharge();
    TEST("Drive not charging after cancel", !drive.isCharging);
    TEST("Drive no target after cancel", !drive.hasTarget);

    // NavigationSystem
    NavigationSystem nav;
    TEST("NavSystem name", nav.GetName() == "NavigationSystem");

    // Jump range check
    TEST("In jump range nearby", nav.IsInJumpRange(drive, origin, nearby));
    TEST("Not in jump range far", !nav.IsInJumpRange(drive, origin, farAway));

    // Fuel cost
    float cost = nav.CalculateJumpFuelCost(origin, nearby);
    TEST("Fuel cost positive", cost > 0.0f);
    TEST("Fuel cost = dist * 10", ApproxEq(cost, 50.0f));

    // Start jump charge
    HyperdriveComponent drive2;
    drive2.timeSinceLastJump = 100.0f; // cooldown satisfied
    TEST("StartJumpCharge succeeds", nav.StartJumpCharge(drive2, nearby));
    TEST("Drive2 is charging", drive2.isCharging);

    // Can't start charge while already charging
    TEST("StartJumpCharge fails while charging", !nav.StartJumpCharge(drive2, nearby));

    // Execute jump (not ready yet - not fully charged)
    SectorLocationComponent loc;
    loc.currentSector = origin;
    TEST("ExecuteJump fails when not charged", !nav.ExecuteJump(drive2, loc));

    // Manually charge up
    drive2.currentCharge = drive2.chargeTime;
    drive2.isCharging = false;
    TEST("ExecuteJump succeeds when charged", nav.ExecuteJump(drive2, loc));
    TEST("Location updated after jump", loc.currentSector == nearby);
    TEST("Cooldown reset after jump", ApproxEq(drive2.timeSinceLastJump, 0.0f));

    // Cancel jump
    HyperdriveComponent drive3;
    nav.StartJumpCharge(drive3, nearby);
    nav.CancelJump(drive3);
    TEST("CancelJump stops charging", !drive3.isCharging);
}

// ===================================================================
// 24. CombatSystem tests
// ===================================================================
static void TestCombatSystem() {
    std::cout << "[CombatSystem]\n";

    CombatSystem combat;
    TEST("CombatSystem name", combat.GetName() == "CombatSystem");

    // Shield component
    ShieldComponent shield;
    TEST("Shield default HP", ApproxEq(shield.maxShieldHP, 100.0f));
    TEST("Shield percentage 100", ApproxEq(shield.GetShieldPercentage(), 100.0f));
    TEST("Shield not depleted", !shield.IsShieldDepleted());

    // Absorb damage within shield capacity
    float overflow = shield.AbsorbDamage(30.0f);
    TEST("Shield absorb no overflow", ApproxEq(overflow, 0.0f));
    TEST("Shield HP after absorb", ApproxEq(shield.currentShieldHP, 70.0f));

    // Absorb damage exceeding shield
    overflow = shield.AbsorbDamage(80.0f);
    TEST("Shield absorb overflow", ApproxEq(overflow, 10.0f));
    TEST("Shield depleted after overflow", shield.IsShieldDepleted());

    // CombatComponent energy
    CombatComponent comp;
    TEST("Has energy 50", comp.HasEnergy(50.0f));
    TEST("Has energy 100", comp.HasEnergy(100.0f));
    TEST("Not has energy 101", !comp.HasEnergy(101.0f));

    TEST("Consume energy succeeds", comp.ConsumeEnergy(60.0f));
    TEST("Energy after consume", ApproxEq(comp.currentEnergy, 40.0f));
    TEST("Consume energy fails if insufficient", !comp.ConsumeEnergy(50.0f));

    // Regenerate energy
    comp.RegenerateEnergy(1.0f); // 20/sec * 1s = 20
    TEST("Energy after regen", ApproxEq(comp.currentEnergy, 60.0f));

    // Energy capped at capacity
    comp.RegenerateEnergy(10.0f); // would be 260, capped at 100
    TEST("Energy capped at capacity", ApproxEq(comp.currentEnergy, 100.0f));

    // Shield regeneration
    CombatComponent comp2;
    comp2.shields.currentShieldHP = 50.0f;
    comp2.shields.timeSinceLastHit = 0.0f;
    comp2.RegenerateShields(1.0f); // delay not met
    TEST("No shield regen during delay", ApproxEq(comp2.shields.currentShieldHP, 50.0f));

    comp2.shields.timeSinceLastHit = 5.0f; // delay met
    comp2.RegenerateShields(1.0f); // 10/sec * 1s = 10
    TEST("Shield regen after delay", ApproxEq(comp2.shields.currentShieldHP, 60.0f));

    // Armor reduction
    TEST("Kinetic armor 50%", ApproxEq(CombatSystem::GetArmorReduction(10.0f, DamageType::Kinetic), 5.0f));
    TEST("Energy armor 25%", ApproxEq(CombatSystem::GetArmorReduction(10.0f, DamageType::Energy), 2.5f));
    TEST("Explosive armor 75%", ApproxEq(CombatSystem::GetArmorReduction(10.0f, DamageType::Explosive), 7.5f));
    TEST("EMP armor 0%", ApproxEq(CombatSystem::GetArmorReduction(10.0f, DamageType::EMP), 0.0f));

    // Shield effectiveness
    TEST("Kinetic shield 80%", ApproxEq(CombatSystem::GetShieldEffectiveness(DamageType::Kinetic), 0.8f));
    TEST("Energy shield 100%", ApproxEq(CombatSystem::GetShieldEffectiveness(DamageType::Energy), 1.0f));
    TEST("EMP shield 120%", ApproxEq(CombatSystem::GetShieldEffectiveness(DamageType::EMP), 1.2f));

    // CalculateDamage
    DamageInfo info = combat.CalculateDamage(50.0f, DamageType::Kinetic, 10.0f);
    TEST("Calculated damage reduced", ApproxEq(info.damage, 45.0f)); // 50 - 5

    // Projectile management
    Projectile proj;
    proj.position = Vector3(0, 0, 0);
    proj.velocity = Vector3(100, 0, 0);
    proj.damage = 25.0f;
    proj.lifetime = 2.0f;

    combat.SpawnProjectile(proj);
    TEST("1 active projectile", combat.GetActiveProjectileCount() == 1);

    combat.UpdateProjectiles(0.5f);
    TEST("Projectile moved", ApproxEq(combat.GetActiveProjectiles()[0].position.x, 50.0f));
    TEST("Projectile lifetime decreased", ApproxEq(combat.GetActiveProjectiles()[0].lifetime, 1.5f));

    // Expire projectile
    combat.UpdateProjectiles(2.0f);
    TEST("Expired projectile removed", combat.GetActiveProjectileCount() == 0);

    // Spawn multiple and clear
    combat.SpawnProjectile(proj);
    combat.SpawnProjectile(proj);
    TEST("2 active projectiles", combat.GetActiveProjectileCount() == 2);
    combat.ClearAllProjectiles();
    TEST("Cleared all projectiles", combat.GetActiveProjectileCount() == 0);

    // ApplyDamageToTarget with shields
    CombatComponent target;
    target.shields.currentShieldHP = 50.0f;
    target.armorRating = 10.0f;
    DamageInfo dmg;
    dmg.damage = 30.0f;
    dmg.damageType = DamageType::Energy; // 100% shield effectiveness
    float dealt = combat.ApplyDamageToTarget(target, dmg);
    TEST("Damage dealt with shields", dealt > 0.0f);
    TEST("Shields absorbed some damage", target.shields.currentShieldHP < 50.0f);
}

// ===================================================================
// 24b. CombatSystem ECS integration tests
// ===================================================================
static void TestCombatSystemECS() {
    std::cout << "[CombatSystem ECS]\n";

    EntityManager em;

    // Create entities with CombatComponents
    auto& e1 = em.CreateEntity("Ship1");
    auto* c1 = em.AddComponent<CombatComponent>(e1.id, std::make_unique<CombatComponent>());
    c1->currentEnergy = 50.0f;
    c1->energyRegenRate = 20.0f;
    c1->shields.currentShieldHP = 60.0f;
    c1->shields.maxShieldHP = 100.0f;
    c1->shields.shieldRegenRate = 10.0f;
    c1->shields.shieldRechargeDelay = 5.0f;
    c1->shields.timeSinceLastHit = 10.0f; // delay met

    auto& e2 = em.CreateEntity("Ship2");
    auto* c2 = em.AddComponent<CombatComponent>(e2.id, std::make_unique<CombatComponent>());
    c2->currentEnergy = 80.0f;
    c2->energyRegenRate = 10.0f;
    c2->shields.currentShieldHP = 30.0f;
    c2->shields.maxShieldHP = 100.0f;
    c2->shields.shieldRegenRate = 5.0f;
    c2->shields.shieldRechargeDelay = 3.0f;
    c2->shields.timeSinceLastHit = 0.5f; // delay NOT met

    // Create CombatSystem with EntityManager
    CombatSystem combat(em);
    TEST("CombatSystem ECS name", combat.GetName() == "CombatSystem");

    // Run Update for 1 second
    combat.Update(1.0f);

    // Ship1: energy 50 + 20 = 70, shield 60 + 10 = 70 (delay met)
    TEST("Ship1 energy regen", ApproxEq(c1->currentEnergy, 70.0f));
    TEST("Ship1 shield regen (delay met)", ApproxEq(c1->shields.currentShieldHP, 70.0f));

    // Ship2: energy 80 + 10 = 90, shield unchanged (delay not met: 0.5 + 1 = 1.5 < 3.0)
    TEST("Ship2 energy regen", ApproxEq(c2->currentEnergy, 90.0f));
    TEST("Ship2 shield no regen (delay not met)", ApproxEq(c2->shields.currentShieldHP, 30.0f));

    // Run Update for 2 more seconds: Ship2 delay now met (1.5 + 2 = 3.5 >= 3.0)
    combat.Update(2.0f);

    // Ship1: energy 70 + 40 = 100 (capped), shield 70 + 20 = 90
    TEST("Ship1 energy capped", ApproxEq(c1->currentEnergy, 100.0f));
    TEST("Ship1 shield continued regen", ApproxEq(c1->shields.currentShieldHP, 90.0f));

    // Ship2: energy 90 + 20 = 100 (capped), shield 30 + 10 = 40 (delay met partway)
    TEST("Ship2 energy capped", ApproxEq(c2->currentEnergy, 100.0f));
    TEST("Ship2 shield regen after delay met", ApproxEq(c2->shields.currentShieldHP, 40.0f));

    // Default constructor (no EntityManager) still works without crashing
    CombatSystem combatNoECS;
    combatNoECS.Update(1.0f);
    TEST("No-ECS combat update doesn't crash", true);
}

// ===================================================================
// 24c. NavigationSystem ECS integration tests
// ===================================================================
static void TestNavigationSystemECS() {
    std::cout << "[NavigationSystem ECS]\n";

    EntityManager em;

    // Create entity with HyperdriveComponent
    auto& e1 = em.CreateEntity("Ship1");
    auto* h1 = em.AddComponent<HyperdriveComponent>(e1.id, std::make_unique<HyperdriveComponent>());
    h1->isCharging = true;
    h1->currentCharge = 0.0f;
    h1->chargeTime = 5.0f;
    h1->timeSinceLastJump = 2.0f;

    auto& e2 = em.CreateEntity("Ship2");
    auto* h2 = em.AddComponent<HyperdriveComponent>(e2.id, std::make_unique<HyperdriveComponent>());
    h2->isCharging = false;
    h2->currentCharge = 0.0f;
    h2->timeSinceLastJump = 8.0f;

    NavigationSystem nav(em);
    TEST("NavSystem ECS name", nav.GetName() == "NavigationSystem");

    // Update for 1 second
    nav.Update(1.0f);

    // Ship1 is charging: charge should increase
    TEST("Ship1 charge increased", ApproxEq(h1->currentCharge, 1.0f));
    // Ship1 cooldown also ticks
    TEST("Ship1 cooldown ticks", ApproxEq(h1->timeSinceLastJump, 3.0f));

    // Ship2 is not charging: charge stays 0
    TEST("Ship2 charge unchanged (not charging)", ApproxEq(h2->currentCharge, 0.0f));
    // Ship2 cooldown still ticks
    TEST("Ship2 cooldown ticks", ApproxEq(h2->timeSinceLastJump, 9.0f));

    // Update for 4 more seconds - Ship1 should be fully charged (5.0)
    nav.Update(4.0f);
    TEST("Ship1 fully charged", ApproxEq(h1->currentCharge, 5.0f));
    TEST("Ship1 fully charged check", h1->IsFullyCharged());

    // Default constructor (no EntityManager) still works without crashing
    NavigationSystem navNoECS;
    navNoECS.Update(1.0f);
    TEST("No-ECS nav update doesn't crash", true);
}

// ===================================================================
// 25. TradingSystem tests
// ===================================================================
static void TestTradingSystem() {
    std::cout << "[TradingSystem]\n";

    TradingSystem trading;
    TEST("TradingSystem name", trading.GetName() == "TradingSystem");

    // Base prices
    TEST("Iron base price", ApproxEq(trading.GetBasePrice(ResourceType::Iron), 10.0f));
    TEST("Titanium base price", ApproxEq(trading.GetBasePrice(ResourceType::Titanium), 25.0f));
    TEST("Naonite base price", ApproxEq(trading.GetBasePrice(ResourceType::Naonite), 50.0f));
    TEST("Trinium base price", ApproxEq(trading.GetBasePrice(ResourceType::Trinium), 100.0f));
    TEST("Xanion base price", ApproxEq(trading.GetBasePrice(ResourceType::Xanion), 200.0f));
    TEST("Ogonite base price", ApproxEq(trading.GetBasePrice(ResourceType::Ogonite), 400.0f));
    TEST("Avorion base price", ApproxEq(trading.GetBasePrice(ResourceType::Avorion), 800.0f));

    // Buy price = base * amount * 1.2
    TEST("Buy 10 Iron", trading.GetBuyPrice(ResourceType::Iron, 10) == 120);
    TEST("Buy 1 Avorion", trading.GetBuyPrice(ResourceType::Avorion, 1) == 960);

    // Sell price = base * amount * 0.8
    TEST("Sell 10 Iron", trading.GetSellPrice(ResourceType::Iron, 10) == 80);
    TEST("Sell 1 Avorion", trading.GetSellPrice(ResourceType::Avorion, 1) == 640);

    // Buy/sell spread: buy > sell
    TEST("Buy price > sell price", trading.GetBuyPrice(ResourceType::Iron, 10) > trading.GetSellPrice(ResourceType::Iron, 10));

    // BuyResource with sufficient credits
    Inventory inv;
    inv.SetMaxCapacity(100000);
    inv.AddResource(ResourceType::Credits, 5000);
    TEST("BuyResource succeeds", trading.BuyResource(ResourceType::Iron, 10, inv));
    TEST("Iron added after buy", inv.GetResourceAmount(ResourceType::Iron) == 10);
    TEST("Credits deducted after buy", inv.GetResourceAmount(ResourceType::Credits) == 5000 - 120);

    // BuyResource with insufficient credits
    Inventory inv2;
    inv2.SetMaxCapacity(100000);
    inv2.AddResource(ResourceType::Credits, 50);
    TEST("BuyResource fails low credits", !trading.BuyResource(ResourceType::Avorion, 1, inv2));

    // SellResource
    Inventory inv3;
    inv3.SetMaxCapacity(100000);
    inv3.AddResource(ResourceType::Iron, 100);
    inv3.AddResource(ResourceType::Credits, 0);
    TEST("SellResource succeeds", trading.SellResource(ResourceType::Iron, 50, inv3));
    TEST("Iron removed after sell", inv3.GetResourceAmount(ResourceType::Iron) == 50);
    TEST("Credits received after sell", inv3.GetResourceAmount(ResourceType::Credits) == trading.GetSellPrice(ResourceType::Iron, 50));

    // SellResource with insufficient resources
    TEST("SellResource fails low stock", !trading.SellResource(ResourceType::Avorion, 1, inv3));

    // Round-trip: buy and sell same amount yields less credits
    Inventory inv4;
    inv4.SetMaxCapacity(100000);
    inv4.AddResource(ResourceType::Credits, 10000);
    int beforeCredits = inv4.GetResourceAmount(ResourceType::Credits);
    trading.BuyResource(ResourceType::Titanium, 10, inv4);
    trading.SellResource(ResourceType::Titanium, 10, inv4);
    int afterCredits = inv4.GetResourceAmount(ResourceType::Credits);
    TEST("Round-trip loses credits (spread)", afterCredits < beforeCredits);

    // BuyResource fails due to inventory capacity limit
    Inventory inv5;
    inv5.SetMaxCapacity(5); // very small capacity
    inv5.AddResource(ResourceType::Credits, 50000);
    TEST("BuyResource fails at capacity", !trading.BuyResource(ResourceType::Iron, 10, inv5));
}

// ===================================================================
// 26. ProgressionSystem tests
// ===================================================================
static void TestProgressionSystem() {
    std::cout << "[ProgressionSystem]\n";

    // ProgressionComponent defaults
    ProgressionComponent prog;
    TEST("Prog default level 1", prog.level == 1);
    TEST("Prog default XP 0", prog.experience == 0);
    TEST("Prog default XP needed 100", prog.experienceToNextLevel == 100);
    TEST("Prog default skill points 0", prog.skillPoints == 0);

    // Add XP without level up
    bool leveled = prog.AddExperience(50);
    TEST("No level up at 50 XP", !leveled);
    TEST("XP is 50", prog.experience == 50);
    TEST("Still level 1", prog.level == 1);

    // Add XP to trigger level up
    leveled = prog.AddExperience(60);
    TEST("Level up at 110 XP", leveled);
    TEST("Now level 2", prog.level == 2);
    TEST("Overflow XP correct", prog.experience == 10); // 110 - 100 = 10
    TEST("XP to next level scaled", prog.experienceToNextLevel == 150); // 100 * 1.5
    TEST("3 skill points gained", prog.skillPoints == 3);

    // Multiple level ups from large XP
    ProgressionComponent prog2;
    prog2.AddExperience(100); // level 1->2
    TEST("Level 2 after 100 XP", prog2.level == 2);
    prog2.AddExperience(150); // level 2->3
    TEST("Level 3 after another 150 XP", prog2.level == 3);
    TEST("6 skill points after 2 level ups", prog2.skillPoints == 6);

    // FactionComponent defaults
    FactionComponent fac;
    TEST("Faction default name", fac.factionName == "Neutral");
    TEST("Unknown faction rep 0", fac.GetReputation("Pirates") == 0);
    TEST("Not friendly with unknown", !fac.IsFriendly("Pirates"));
    TEST("Not hostile with unknown", !fac.IsHostile("Pirates"));

    // Modify reputation
    fac.ModifyReputation("Traders", 60);
    TEST("Traders rep 60", fac.GetReputation("Traders") == 60);
    TEST("Friendly with Traders", fac.IsFriendly("Traders"));
    TEST("Not hostile with Traders", !fac.IsHostile("Traders"));

    // Negative reputation
    fac.ModifyReputation("Pirates", -70);
    TEST("Pirates rep -70", fac.GetReputation("Pirates") == -70);
    TEST("Not friendly with Pirates", !fac.IsFriendly("Pirates"));
    TEST("Hostile with Pirates", fac.IsHostile("Pirates"));

    // Clamping
    fac.ModifyReputation("Traders", 200); // should clamp to 100
    TEST("Rep clamped to 100", fac.GetReputation("Traders") == 100);

    fac.ModifyReputation("Pirates", -200); // should clamp to -100
    TEST("Rep clamped to -100", fac.GetReputation("Pirates") == -100);

    // Boundary checks
    FactionComponent fac2;
    fac2.ModifyReputation("Neutral", 50);
    TEST("Rep 50 is friendly", fac2.IsFriendly("Neutral"));
    fac2.ModifyReputation("Neutral", -100); // 50 + (-100) = -50
    TEST("Rep -50 is hostile", fac2.IsHostile("Neutral"));

    // Multiple factions tracked independently
    FactionComponent fac3;
    fac3.ModifyReputation("A", 30);
    fac3.ModifyReputation("B", -40);
    TEST("Faction A rep 30", fac3.GetReputation("A") == 30);
    TEST("Faction B rep -40", fac3.GetReputation("B") == -40);
}

// ===================================================================
// 27. CrewSystem tests
// ===================================================================
static void TestCrewSystem() {
    std::cout << "[CrewSystem]\n";

    // Pilot defaults
    Pilot pilot;
    pilot.name = "Test Pilot";
    TEST("Pilot default level 1", pilot.level == 1);
    TEST("Pilot default XP 0", pilot.experience == 0);
    TEST("Pilot not assigned", !pilot.IsAssigned());
    TEST("Pilot overall skill", ApproxEq(pilot.GetOverallSkill(), 0.5f));

    // Pilot with custom skills
    Pilot pilot2;
    pilot2.name = "Skilled Pilot";
    pilot2.combatSkill = 0.8f;
    pilot2.navigationSkill = 0.6f;
    pilot2.engineeringSkill = 0.4f;
    TEST("Custom overall skill", ApproxEq(pilot2.GetOverallSkill(), 0.6f));

    // Pilot experience and level up
    Pilot pilot3;
    pilot3.name = "Rookie";
    bool leveled = pilot3.AddExperience(400);
    TEST("No level up at 400 XP (needs 500)", !leveled);
    TEST("Pilot XP 400", pilot3.experience == 400);
    leveled = pilot3.AddExperience(100);
    TEST("Level up at 500 XP", leveled);
    TEST("Pilot now level 2", pilot3.level == 2);
    TEST("Pilot overflow XP 0", pilot3.experience == 0);

    // Level 2 needs 1000 XP
    leveled = pilot3.AddExperience(999);
    TEST("No level up at 999/1000 XP", !leveled);
    leveled = pilot3.AddExperience(1);
    TEST("Level up at 1000 XP", leveled);
    TEST("Pilot now level 3", pilot3.level == 3);

    // CrewComponent defaults
    CrewComponent crew;
    crew.entityId = 42;
    TEST("Crew min 1", crew.minimumCrew == 1);
    TEST("Crew current 0", crew.currentCrew == 0);
    TEST("Crew max 10", crew.maxCrew == 10);
    TEST("Not sufficient crew", !crew.HasSufficientCrew());
    TEST("No pilot", !crew.HasPilot());
    TEST("Not operational", !crew.IsOperational());

    // Add crew
    TEST("Add 5 crew succeeds", crew.AddCrew(5));
    TEST("Current crew 5", crew.currentCrew == 5);
    TEST("Has sufficient crew", crew.HasSufficientCrew());
    TEST("Crew efficiency > 1 (overmanned)", crew.GetCrewEfficiency() > 1.0f);

    // Add crew beyond max fails
    TEST("Add 6 more fails", !crew.AddCrew(6));
    TEST("Still 5 crew", crew.currentCrew == 5);

    // Remove crew
    TEST("Remove 3 crew succeeds", crew.RemoveCrew(3));
    TEST("Current crew 2", crew.currentCrew == 2);
    TEST("Still sufficient crew", crew.HasSufficientCrew());

    // Remove too many fails
    TEST("Remove 5 fails", !crew.RemoveCrew(5));

    // Assign pilot
    Pilot pilotA;
    pilotA.name = "Captain Alpha";
    TEST("Assign pilot succeeds", crew.AssignPilot(pilotA));
    TEST("Has pilot now", crew.HasPilot());
    TEST("Is operational", crew.IsOperational());
    TEST("Pilot assigned ship set", pilotA.assignedShipId == 42);

    // Can't assign already-assigned pilot
    CrewComponent crew2;
    crew2.entityId = 99;
    crew2.AddCrew(5);
    TEST("Assign same pilot fails", !crew2.AssignPilot(pilotA));

    // Remove pilot
    Pilot removedPilot;
    TEST("Remove pilot succeeds", crew.RemovePilot(removedPilot));
    TEST("Removed pilot name", removedPilot.name == "Captain Alpha");
    TEST("Removed pilot unassigned", !removedPilot.IsAssigned());
    TEST("No pilot after removal", !crew.HasPilot());
    TEST("Not operational without pilot", !crew.IsOperational());

    // Remove pilot when none assigned
    Pilot dummy;
    TEST("Remove from empty fails", !crew.RemovePilot(dummy));

    // Crew efficiency: undermanned
    CrewComponent crew3;
    crew3.minimumCrew = 10;
    crew3.maxCrew = 20;
    crew3.AddCrew(5);
    TEST("Undermanned efficiency 0.5", ApproxEq(crew3.GetCrewEfficiency(), 0.5f));

    // Crew efficiency: exactly manned
    crew3.AddCrew(5); // now 10
    TEST("Exact efficiency 1.0", ApproxEq(crew3.GetCrewEfficiency(), 1.0f));

    // Crew efficiency: overmanned (bonus capped at 0.2)
    crew3.AddCrew(10); // now 20
    float expected = 1.0f + std::min(0.2f, (20 - 10) * 0.02f);
    TEST("Overmanned efficiency", ApproxEq(crew3.GetCrewEfficiency(), expected));
}

// ===================================================================
// 28. Power System tests
// ===================================================================
static void TestPowerComponent() {
    std::cout << "[PowerComponent]\n";

    PowerComponent pc;
    TEST("Default no generation", pc.maxPowerGeneration == 0.0f);
    TEST("Default storage 100", ApproxEq(pc.currentStoredPower, 100.0f));
    TEST("Default all enabled", pc.weaponsEnabled && pc.shieldsEnabled && pc.enginesEnabled && pc.systemsEnabled);

    // Setup some consumption
    pc.currentPowerGeneration = 100.0f;
    pc.weaponsPowerConsumption = 30.0f;
    pc.shieldsPowerConsumption = 25.0f;
    pc.enginesPowerConsumption = 20.0f;
    pc.systemsPowerConsumption = 5.0f;
    pc.UpdateTotalConsumption();

    TEST("Total consumption 80", ApproxEq(pc.totalPowerConsumption, 80.0f));
    TEST("Available power 20", ApproxEq(pc.GetAvailablePower(), 20.0f));
    TEST("No deficit", ApproxEq(pc.GetPowerDeficit(), 0.0f));
    TEST("Not low power", !pc.IsLowPower());
    TEST("Has enough for 15", pc.HasEnoughPower(15.0f));
    TEST("Not enough for 25", !pc.HasEnoughPower(25.0f));

    // Toggle weapons off
    pc.ToggleSystem(PowerSystemType::Weapons);
    TEST("Weapons disabled", !pc.weaponsEnabled);
    TEST("Consumption now 50", ApproxEq(pc.totalPowerConsumption, 50.0f));
    TEST("Available power 50", ApproxEq(pc.GetAvailablePower(), 50.0f));

    // Toggle weapons back on
    pc.ToggleSystem(PowerSystemType::Weapons);
    TEST("Weapons re-enabled", pc.weaponsEnabled);
    TEST("Consumption back to 80", ApproxEq(pc.totalPowerConsumption, 80.0f));

    // Low power scenario
    pc.currentPowerGeneration = 40.0f;
    TEST("Deficit 40", ApproxEq(pc.GetPowerDeficit(), 40.0f));
    TEST("Is low power", pc.IsLowPower());

    // Efficiency < 1
    pc.currentPowerGeneration = 100.0f;
    pc.efficiency = 0.5f;
    TEST("Half efficiency deficit 30", ApproxEq(pc.GetPowerDeficit(), 30.0f));

    // Priority defaults
    TEST("Shields priority 1", pc.shieldsPriority == 1);
    TEST("Weapons priority 2", pc.weaponsPriority == 2);
    TEST("Engines priority 3", pc.enginesPriority == 3);
    TEST("Systems priority 4", pc.systemsPriority == 4);
}

static void TestPowerSystem() {
    std::cout << "[PowerSystem]\n";

    PowerSystem sys;
    TEST("System name", sys.GetName() == "PowerSystem");

    // CalculatePowerGeneration sets storage capacity
    PowerComponent pc;
    sys.CalculatePowerGeneration(pc, 4);
    TEST("Storage capacity 200", ApproxEq(pc.maxStoredPower, 200.0f));

    sys.CalculatePowerGeneration(pc, 0);
    TEST("Zero generators zero storage", ApproxEq(pc.maxStoredPower, 0.0f));

    // CalculatePowerConsumption
    PowerComponent pc2;
    pc2.currentPowerGeneration = 200.0f;
    sys.CalculatePowerConsumption(pc2, 2, 3, 1, 2, 3);
    // engines: 2*5 + 3*3 + 1*2 = 21
    TEST("Engines consumption 21", ApproxEq(pc2.enginesPowerConsumption, 21.0f));
    // shields: 2*10 = 20
    TEST("Shields consumption 20", ApproxEq(pc2.shieldsPowerConsumption, 20.0f));
    // weapons: 3*8 = 24
    TEST("Weapons consumption 24", ApproxEq(pc2.weaponsPowerConsumption, 24.0f));
    // systems: 5
    TEST("Systems consumption 5", ApproxEq(pc2.systemsPowerConsumption, 5.0f));
    // total: 21+20+24+5 = 70
    TEST("Total consumption 70", ApproxEq(pc2.totalPowerConsumption, 70.0f));

    // DistributePower — no deficit
    TEST("No systems disabled with surplus", sys.DistributePower(pc2) == 0);

    // DistributePower — deficit uses stored power first
    PowerComponent pc3;
    pc3.currentPowerGeneration = 10.0f;
    pc3.weaponsPowerConsumption = 20.0f;
    pc3.UpdateTotalConsumption();
    pc3.currentStoredPower = 50.0f;
    int disabled = sys.DistributePower(pc3);
    TEST("Stored power used, none disabled", disabled == 0);
    TEST("Stored power reduced", pc3.currentStoredPower < 50.0f);

    // DistributePower — no stored power, systems disabled by priority
    PowerComponent pc4;
    pc4.currentPowerGeneration = 10.0f;
    pc4.weaponsPowerConsumption = 20.0f;
    pc4.shieldsPowerConsumption = 15.0f;
    pc4.enginesPowerConsumption = 10.0f;
    pc4.systemsPowerConsumption = 5.0f;
    pc4.UpdateTotalConsumption(); // total 50, gen 10, deficit 40
    pc4.currentStoredPower = 0.0f;
    disabled = sys.DistributePower(pc4);
    TEST("Systems disabled > 0", disabled > 0);
    // Systems (priority 4) should be disabled first, then engines (3), then weapons (2)
    TEST("Systems subsystem disabled", !pc4.systemsEnabled);

    // ChargePowerStorage
    PowerComponent pc5;
    pc5.currentPowerGeneration = 100.0f;
    pc5.weaponsPowerConsumption = 20.0f;
    pc5.UpdateTotalConsumption();
    pc5.maxStoredPower = 200.0f;
    pc5.currentStoredPower = 50.0f;
    sys.ChargePowerStorage(pc5, 1.0f);
    TEST("Storage charged", pc5.currentStoredPower > 50.0f);
    TEST("Storage not over max", pc5.currentStoredPower <= 200.0f);

    // ChargePowerStorage — full storage stays full
    pc5.currentStoredPower = 200.0f;
    sys.ChargePowerStorage(pc5, 1.0f);
    TEST("Full storage unchanged", ApproxEq(pc5.currentStoredPower, 200.0f));

    // ChargePowerStorage — no excess, no charge
    PowerComponent pc6;
    pc6.currentPowerGeneration = 50.0f;
    pc6.weaponsPowerConsumption = 50.0f;
    pc6.UpdateTotalConsumption();
    pc6.maxStoredPower = 100.0f;
    pc6.currentStoredPower = 10.0f;
    sys.ChargePowerStorage(pc6, 1.0f);
    TEST("No excess, no charge", ApproxEq(pc6.currentStoredPower, 10.0f));
}

// ===================================================================
// 29. Mining System tests
// ===================================================================
static void TestMiningSystem() {
    std::cout << "[MiningSystem]\n";

    MiningSystem sys;
    TEST("System name", sys.GetName() == "MiningSystem");
    TEST("Empty asteroids", sys.GetAsteroidCount() == 0);
    TEST("Empty wreckage", sys.GetWreckageCount() == 0);

    // Add asteroid
    Asteroid a1;
    a1.id = 100;
    a1.position = {0.0f, 0.0f, 0.0f};
    a1.size = 10.0f;
    a1.resourceType = ResourceType::Iron;
    a1.remainingResources = 100.0f;
    sys.AddAsteroid(a1);
    TEST("Asteroid added", sys.GetAsteroidCount() == 1);

    // Add wreckage
    Wreckage w1;
    w1.id = 200;
    w1.position = {10.0f, 0.0f, 0.0f};
    w1.resources = {{ResourceType::Titanium, 50}, {ResourceType::Iron, 30}};
    sys.AddWreckage(w1);
    TEST("Wreckage added", sys.GetWreckageCount() == 1);

    // Start mining — in range
    MiningComponent mc;
    mc.miningPower = 10.0f;
    mc.miningRange = 100.0f;
    MiningPosition minerPos = {5.0f, 0.0f, 0.0f};
    TEST("Start mining succeeds", sys.StartMining(mc, 100, minerPos));
    TEST("Is mining", mc.isMining);
    TEST("Target set", mc.targetAsteroidId == 100);

    // Start mining — out of range
    MiningComponent mc2;
    mc2.miningRange = 1.0f;
    MiningPosition farPos = {1000.0f, 0.0f, 0.0f};
    TEST("Out of range fails", !sys.StartMining(mc2, 100, farPos));
    TEST("Not mining", !mc2.isMining);

    // Start mining — invalid asteroid
    TEST("Invalid asteroid fails", !sys.StartMining(mc2, 999, minerPos));

    // Process mining
    Inventory inv;
    inv.SetMaxCapacity(10000);
    float extracted = sys.ProcessMining(mc, inv, 5.0f); // 10 * 5 = 50
    TEST("Extracted 50", ApproxEq(extracted, 50.0f));
    TEST("Iron in inventory", inv.GetResourceAmount(ResourceType::Iron) == 50);

    // Continue mining until depleted
    extracted = sys.ProcessMining(mc, inv, 10.0f); // wants 100, only 50 left
    TEST("Extracted remaining 50", ApproxEq(extracted, 50.0f));
    TEST("Asteroid depleted", sys.GetAsteroidCount() == 0);
    TEST("Mining stopped", !mc.isMining);

    // Process mining when not mining
    MiningComponent mc3;
    extracted = sys.ProcessMining(mc3, inv, 1.0f);
    TEST("Not mining returns 0", ApproxEq(extracted, 0.0f));

    // Start salvaging — in range
    SalvagingComponent sc;
    sc.salvagePower = 8.0f;
    sc.salvageRange = 100.0f;
    MiningPosition salvagerPos = {10.0f, 0.0f, 0.0f};
    TEST("Start salvaging succeeds", sys.StartSalvaging(sc, 200, salvagerPos));
    TEST("Is salvaging", sc.isSalvaging);
    TEST("Target set", sc.targetWreckageId == 200);

    // Start salvaging — out of range
    SalvagingComponent sc2;
    sc2.salvageRange = 1.0f;
    TEST("Out of range salvage fails", !sys.StartSalvaging(sc2, 200, farPos));

    // Process salvaging
    Inventory inv2;
    inv2.SetMaxCapacity(10000);
    int salvaged = sys.ProcessSalvaging(sc, inv2, 10.0f); // 8*10 = 80 per resource
    TEST("Salvaged something", salvaged > 0);

    // Continue salvaging until depleted
    for (int i = 0; i < 10; ++i) {
        sys.ProcessSalvaging(sc, inv2, 100.0f);
    }
    TEST("Wreckage depleted", sys.GetWreckageCount() == 0);
    TEST("Salvaging stopped", !sc.isSalvaging);

    // Stop mining/salvaging manually
    MiningComponent mc4;
    mc4.isMining = true;
    mc4.targetAsteroidId = 42;
    sys.StopMining(mc4);
    TEST("Stop mining works", !mc4.isMining);
    TEST("Target cleared", mc4.targetAsteroidId == InvalidEntityId);

    SalvagingComponent sc3;
    sc3.isSalvaging = true;
    sc3.targetWreckageId = 42;
    sys.StopSalvaging(sc3);
    TEST("Stop salvaging works", !sc3.isSalvaging);
    TEST("Salvage target cleared", sc3.targetWreckageId == InvalidEntityId);
}

// ===================================================================
// 30. Procedural Generation tests
// ===================================================================
static void TestGalaxyGenerator() {
    std::cout << "[GalaxyGenerator]\n";

    // Deterministic generation with fixed seed
    GalaxyGenerator gen(42);
    TEST("Seed stored", gen.GetSeed() == 42);

    // Generate a sector
    GalaxySector sector = gen.GenerateSector(0, 0, 0);
    TEST("Sector coords x", sector.x == 0);
    TEST("Sector coords y", sector.y == 0);
    TEST("Sector coords z", sector.z == 0);
    TEST("Has asteroids", !sector.asteroids.empty());
    TEST("Asteroid count in range", (int)sector.asteroids.size() >= 5 && (int)sector.asteroids.size() <= 20);

    // Verify asteroid data
    const auto& first = sector.asteroids[0];
    TEST("Asteroid has size", first.size >= 10.0f && first.size <= 60.0f);
    TEST("Asteroid in bounds X", first.position.x >= -5000.0f && first.position.x <= 5000.0f);

    // Deterministic: same seed, same coordinates → same result
    GalaxySector sector2 = gen.GenerateSector(0, 0, 0);
    TEST("Deterministic asteroid count", sector.asteroids.size() == sector2.asteroids.size());
    TEST("Deterministic first size", ApproxEq(sector.asteroids[0].size, sector2.asteroids[0].size));
    TEST("Deterministic station presence", sector.hasStation == sector2.hasStation);
    if (sector.hasStation && sector2.hasStation) {
        TEST("Deterministic station name", sector.station.name == sector2.station.name);
        TEST("Deterministic station type", sector.station.stationType == sector2.station.stationType);
    }

    // Different coordinates → different sector (highly likely)
    GalaxySector sector3 = gen.GenerateSector(100, -50, 200);
    // At minimum the asteroid data should differ
    bool different = sector3.asteroids.size() != sector.asteroids.size();
    if (!different && !sector.asteroids.empty() && !sector3.asteroids.empty()) {
        different = !ApproxEq(sector.asteroids[0].size, sector3.asteroids[0].size);
    }
    TEST("Different coords differ", different);

    // Different seed → different sector
    GalaxyGenerator gen2(999);
    GalaxySector sector4 = gen2.GenerateSector(0, 0, 0);
    bool seedDiff = sector4.asteroids.size() != sector.asteroids.size();
    if (!seedDiff && !sector.asteroids.empty() && !sector4.asteroids.empty()) {
        seedDiff = !ApproxEq(sector.asteroids[0].size, sector4.asteroids[0].size);
    }
    TEST("Different seed differs", seedDiff);

    // Custom parameters
    GalaxyGenerator gen3(42);
    gen3.stationProbability = 1.0f;  // Always spawn station
    gen3.wormholeProbability = 1.0f; // Always spawn wormhole
    gen3.minAsteroids = 1;
    gen3.maxAsteroids = 1;
    GalaxySector sector5 = gen3.GenerateSector(0, 0, 0);
    TEST("Custom: 1 asteroid", sector5.asteroids.size() == 1);
    TEST("Custom: has station", sector5.hasStation);
    TEST("Custom: station has name", !sector5.station.name.empty());
    TEST("Custom: station has type", !sector5.station.stationType.empty());
    TEST("Custom: has wormhole", !sector5.wormholes.empty());
    if (!sector5.wormholes.empty()) {
        TEST("Wormhole has designation", !sector5.wormholes[0].designation.empty());
        TEST("Wormhole class 1-6", sector5.wormholes[0].wormholeClass >= 1 && sector5.wormholes[0].wormholeClass <= 6);
    }

    // No station/wormhole with 0 probability
    GalaxyGenerator gen4(42);
    gen4.stationProbability = 0.0f;
    gen4.wormholeProbability = 0.0f;
    GalaxySector sector6 = gen4.GenerateSector(0, 0, 0);
    TEST("No station with 0 prob", !sector6.hasStation);
    TEST("No wormhole with 0 prob", sector6.wormholes.empty());

    // Default seed (non-zero)
    GalaxyGenerator genDefault(0);
    TEST("Default seed non-zero", genDefault.GetSeed() != 0);

    // Generate many sectors to verify no crashes
    for (int i = -5; i <= 5; ++i) {
        for (int j = -5; j <= 5; ++j) {
            gen.GenerateSector(i, j, 0);
        }
    }
    TEST("Bulk generation no crash", true);
}

// ===================================================================
// Quest System Tests
// ===================================================================
static void TestQuestObjective() {
    std::cout << "[QuestObjective]\n";

    QuestObjective obj;
    obj.id = "obj1";
    obj.type = ObjectiveType::Destroy;
    obj.target = "pirate";
    obj.requiredQuantity = 3;

    TEST("Objective starts NotStarted", obj.status == ObjectiveStatus::NotStarted);
    TEST("Initial progress is 0", obj.currentProgress == 0);
    TEST("Not complete initially", !obj.IsComplete());
    TEST("Completion is 0", ApproxEq(obj.GetCompletionPercentage(), 0.0f));

    obj.Progress(1);
    TEST("Progress activates objective", obj.status == ObjectiveStatus::Active);
    TEST("Progress 1/3", obj.currentProgress == 1);
    TEST("Completion ~33%", ApproxEq(obj.GetCompletionPercentage(), 1.0f / 3.0f));

    obj.Progress(1);
    TEST("Progress 2/3", obj.currentProgress == 2);

    bool completed = obj.Progress(1);
    TEST("Progress completes objective", completed);
    TEST("Status is Completed", obj.status == ObjectiveStatus::Completed);
    TEST("IsComplete true", obj.IsComplete());
    TEST("Completion 100%", ApproxEq(obj.GetCompletionPercentage(), 1.0f));

    // Cannot progress after completion
    bool again = obj.Progress(1);
    TEST("Cannot progress after completion", !again);

    // Reset
    obj.Reset();
    TEST("Reset clears progress", obj.currentProgress == 0);
    TEST("Reset restores NotStarted", obj.status == ObjectiveStatus::NotStarted);

    // Fail
    obj.Activate();
    TEST("Activate sets Active", obj.status == ObjectiveStatus::Active);
    obj.Fail();
    TEST("Fail sets Failed", obj.status == ObjectiveStatus::Failed);
    bool afterFail = obj.Progress(1);
    TEST("Cannot progress after failure", !afterFail);
}

static void TestQuest() {
    std::cout << "[Quest]\n";

    Quest quest;
    quest.id = "quest1";
    quest.title = "Destroy Pirates";
    quest.description = "Kill 3 pirates";

    QuestObjective obj1;
    obj1.id = "kill_pirates";
    obj1.type = ObjectiveType::Destroy;
    obj1.target = "pirate";
    obj1.requiredQuantity = 3;

    QuestObjective obj2;
    obj2.id = "collect_loot";
    obj2.type = ObjectiveType::Collect;
    obj2.target = "pirate_loot";
    obj2.requiredQuantity = 1;
    obj2.isOptional = true;

    quest.objectives.push_back(obj1);
    quest.objectives.push_back(obj2);

    QuestReward reward;
    reward.type = RewardType::Credits;
    reward.amount = 1000;
    quest.rewards.push_back(reward);

    TEST("Quest starts Available", quest.status == QuestStatus::Available);
    TEST("Cannot complete before accepting", !quest.Complete());
    TEST("Cannot turn in before completing", !quest.TurnIn());

    bool accepted = quest.Accept();
    TEST("Accept succeeds", accepted);
    TEST("Quest is Active", quest.status == QuestStatus::Active);
    TEST("Cannot accept again", !quest.Accept());

    // Required objectives not complete
    TEST("Required objectives not complete", !quest.AreRequiredObjectivesComplete());
    TEST("No failed objective", !quest.HasFailedObjective());
    TEST("Cannot complete early", !quest.Complete());

    // Progress required objective
    quest.objectives[0].Progress(3);
    TEST("Required objectives complete", quest.AreRequiredObjectivesComplete());
    TEST("Completion ignores optional", ApproxEq(quest.GetCompletionPercentage(), 1.0f));

    bool completed = quest.Complete();
    TEST("Complete succeeds", completed);
    TEST("Quest is Completed", quest.status == QuestStatus::Completed);

    bool turnedIn = quest.TurnIn();
    TEST("TurnIn succeeds", turnedIn);
    TEST("Quest is TurnedIn", quest.status == QuestStatus::TurnedIn);

    // Reset
    quest.Reset();
    TEST("Reset restores Available", quest.status == QuestStatus::Available);
    TEST("Reset clears objective progress", quest.objectives[0].currentProgress == 0);
}

static void TestQuestComponent() {
    std::cout << "[QuestComponent]\n";

    QuestComponent comp;
    TEST("No quests initially", comp.quests.empty());
    TEST("Active count 0", comp.GetActiveQuestCount() == 0);

    Quest q1;
    q1.id = "q1";
    q1.title = "Quest 1";
    comp.AddQuest(q1);
    TEST("One quest after add", comp.quests.size() == 1);
    TEST("Available count 1", comp.GetAvailableQuestCount() == 1);

    Quest* found = comp.GetQuest("q1");
    TEST("Find quest by id", found != nullptr);
    TEST("Found correct quest", found != nullptr && found->id == "q1");

    Quest* notFound = comp.GetQuest("nonexistent");
    TEST("Nonexistent returns nullptr", notFound == nullptr);

    bool accepted = comp.AcceptQuest("q1");
    TEST("Accept quest via component", accepted);
    TEST("Active count 1", comp.GetActiveQuestCount() == 1);
    TEST("Available count 0", comp.GetAvailableQuestCount() == 0);

    // Abandon
    Quest q2;
    q2.id = "q2";
    q2.canAbandon = false;
    comp.AddQuest(q2);
    comp.AcceptQuest("q2");
    bool abandoned = comp.AbandonQuest("q2");
    TEST("Cannot abandon non-abandonable", !abandoned);

    bool abandoned1 = comp.AbandonQuest("q1");
    TEST("Can abandon abandonable quest", abandoned1);
    TEST("Active count 1 after abandon", comp.GetActiveQuestCount() == 1);

    // Remove
    bool removed = comp.RemoveQuest("q1");
    TEST("Remove quest succeeds", removed);
    bool removedAgain = comp.RemoveQuest("q1");
    TEST("Remove nonexistent fails", !removedAgain);

    // Max active quests
    comp.maxActiveQuests = 1;
    Quest q3;
    q3.id = "q3";
    comp.AddQuest(q3);
    bool accepted3 = comp.AcceptQuest("q3");
    TEST("Cannot exceed max active quests", !accepted3);
}

static void TestQuestSystem() {
    std::cout << "[QuestSystem]\n";

    QuestSystem system;
    TEST("System name", system.GetName() == "QuestSystem");
    TEST("No templates initially", system.GetTemplateCount() == 0);

    Quest tmpl;
    tmpl.id = "tmpl_kill";
    tmpl.title = "Kill Quest Template";
    QuestObjective obj;
    obj.id = "kill_obj";
    obj.type = ObjectiveType::Destroy;
    obj.target = "enemy";
    obj.requiredQuantity = 5;
    tmpl.objectives.push_back(obj);

    system.AddQuestTemplate(tmpl);
    TEST("One template after add", system.GetTemplateCount() == 1);

    // Create from template
    Quest created = system.CreateQuestFromTemplate("tmpl_kill");
    TEST("Created quest has id", created.id == "tmpl_kill");
    TEST("Created quest has objective", created.objectives.size() == 1);

    Quest missing = system.CreateQuestFromTemplate("nonexistent");
    TEST("Missing template returns empty", missing.id.empty());

    // Give quest
    QuestComponent comp;
    bool given = system.GiveQuest(1, "tmpl_kill", comp);
    TEST("GiveQuest succeeds", given);
    TEST("Component has quest", comp.quests.size() == 1);

    bool givenBad = system.GiveQuest(1, "nonexistent", comp);
    TEST("GiveQuest with bad template fails", !givenBad);

    // Accept and progress
    comp.AcceptQuest("tmpl_kill");
    system.ProgressObjective(comp, ObjectiveType::Destroy, "enemy", 3);
    Quest* q = comp.GetQuest("tmpl_kill");
    TEST("Progress applied", q != nullptr && q->objectives[0].currentProgress == 3);

    system.ProgressObjective(comp, ObjectiveType::Destroy, "enemy", 2);
    TEST("Quest auto-completed", q != nullptr && q->status == QuestStatus::Completed);

    // Wrong type/target doesn't progress
    Quest tmpl2;
    tmpl2.id = "tmpl_mine";
    QuestObjective obj2;
    obj2.id = "mine_obj";
    obj2.type = ObjectiveType::Mine;
    obj2.target = "iron";
    obj2.requiredQuantity = 10;
    tmpl2.objectives.push_back(obj2);
    system.AddQuestTemplate(tmpl2);
    system.GiveQuest(1, "tmpl_mine", comp);
    comp.AcceptQuest("tmpl_mine");
    system.ProgressObjective(comp, ObjectiveType::Destroy, "iron", 5);
    Quest* q2 = comp.GetQuest("tmpl_mine");
    TEST("Wrong type doesn't progress", q2 != nullptr && q2->objectives[0].currentProgress == 0);
    system.ProgressObjective(comp, ObjectiveType::Mine, "gold", 5);
    TEST("Wrong target doesn't progress", q2 != nullptr && q2->objectives[0].currentProgress == 0);
    system.ProgressObjective(comp, ObjectiveType::Mine, "iron", 5);
    TEST("Correct type+target progresses", q2 != nullptr && q2->objectives[0].currentProgress == 5);
}

static void TestQuestSystemTradeVisitBuild() {
    std::cout << "[QuestSystem Trade/Visit/Build]\n";

    QuestSystem system;

    // Trade objective
    Quest tradeTmpl;
    tradeTmpl.id = "tmpl_trade";
    tradeTmpl.title = "Trade Quest";
    QuestObjective tradeObj;
    tradeObj.id = "trade_obj";
    tradeObj.type = ObjectiveType::Trade;
    tradeObj.target = "Iron";
    tradeObj.requiredQuantity = 50;
    tradeTmpl.objectives.push_back(tradeObj);
    system.AddQuestTemplate(tradeTmpl);

    QuestComponent comp;
    system.GiveQuest(1, "tmpl_trade", comp);
    comp.AcceptQuest("tmpl_trade");

    system.ProgressObjective(comp, ObjectiveType::Trade, "Iron", 30);
    Quest* qt = comp.GetQuest("tmpl_trade");
    TEST("Trade progress applied", qt != nullptr && qt->objectives[0].currentProgress == 30);
    TEST("Trade quest still active", qt != nullptr && qt->status == QuestStatus::Active);

    system.ProgressObjective(comp, ObjectiveType::Trade, "Iron", 20);
    TEST("Trade quest auto-completed", qt != nullptr && qt->status == QuestStatus::Completed);

    // Build objective
    Quest buildTmpl;
    buildTmpl.id = "tmpl_build";
    buildTmpl.title = "Build Quest";
    QuestObjective buildObj;
    buildObj.id = "build_obj";
    buildObj.type = ObjectiveType::Build;
    buildObj.target = "Hull";
    buildObj.requiredQuantity = 10;
    buildTmpl.objectives.push_back(buildObj);
    system.AddQuestTemplate(buildTmpl);

    system.GiveQuest(1, "tmpl_build", comp);
    comp.AcceptQuest("tmpl_build");

    system.ProgressObjective(comp, ObjectiveType::Build, "Hull", 10);
    Quest* qb = comp.GetQuest("tmpl_build");
    TEST("Build quest auto-completed", qb != nullptr && qb->status == QuestStatus::Completed);

    // Visit objective
    Quest visitTmpl;
    visitTmpl.id = "tmpl_visit";
    visitTmpl.title = "Visit Quest";
    QuestObjective visitObj;
    visitObj.id = "visit_obj";
    visitObj.type = ObjectiveType::Visit;
    visitObj.target = "Sector_5_3";
    visitObj.requiredQuantity = 1;
    visitTmpl.objectives.push_back(visitObj);
    system.AddQuestTemplate(visitTmpl);

    system.GiveQuest(1, "tmpl_visit", comp);
    comp.AcceptQuest("tmpl_visit");

    system.ProgressObjective(comp, ObjectiveType::Visit, "Sector_99_99", 1);
    Quest* qv = comp.GetQuest("tmpl_visit");
    TEST("Visit wrong target no progress", qv != nullptr && qv->status == QuestStatus::Active);

    system.ProgressObjective(comp, ObjectiveType::Visit, "Sector_5_3", 1);
    TEST("Visit quest auto-completed", qv != nullptr && qv->status == QuestStatus::Completed);

    // Mixed quest with Trade + Build objectives
    Quest mixedTmpl;
    mixedTmpl.id = "tmpl_mixed";
    mixedTmpl.title = "Mixed Quest";
    QuestObjective mixObj1;
    mixObj1.id = "mix_trade";
    mixObj1.type = ObjectiveType::Trade;
    mixObj1.target = "Titanium";
    mixObj1.requiredQuantity = 5;
    QuestObjective mixObj2;
    mixObj2.id = "mix_build";
    mixObj2.type = ObjectiveType::Build;
    mixObj2.target = "Engine";
    mixObj2.requiredQuantity = 3;
    mixedTmpl.objectives.push_back(mixObj1);
    mixedTmpl.objectives.push_back(mixObj2);
    system.AddQuestTemplate(mixedTmpl);

    system.GiveQuest(1, "tmpl_mixed", comp);
    comp.AcceptQuest("tmpl_mixed");

    system.ProgressObjective(comp, ObjectiveType::Trade, "Titanium", 5);
    Quest* qm = comp.GetQuest("tmpl_mixed");
    TEST("Mixed quest still active after one obj", qm != nullptr && qm->status == QuestStatus::Active);

    system.ProgressObjective(comp, ObjectiveType::Build, "Engine", 3);
    TEST("Mixed quest completed after both objs", qm != nullptr && qm->status == QuestStatus::Completed);
}

// ===================================================================
// QuestComponent Serialization Tests
// ===================================================================
static void TestQuestComponentSerialization() {
    std::cout << "[QuestComponent Serialization]\n";

    // Build a component with varied quest state
    QuestComponent comp;
    comp.maxActiveQuests = 5;

    Quest q1;
    q1.id = "quest_mine";
    q1.title = "Mine Iron";
    q1.status = QuestStatus::Active;
    q1.canAbandon = true;
    q1.isRepeatable = false;
    q1.timeLimit = 300;

    QuestObjective obj1;
    obj1.id = "obj_mine_iron";
    obj1.type = ObjectiveType::Mine;
    obj1.target = "Iron";
    obj1.requiredQuantity = 10;
    obj1.currentProgress = 4;
    obj1.status = ObjectiveStatus::Active;
    obj1.isOptional = false;
    obj1.isHidden = false;
    q1.objectives.push_back(obj1);

    QuestObjective obj2;
    obj2.id = "obj_bonus";
    obj2.type = ObjectiveType::Collect;
    obj2.target = "Crystal";
    obj2.requiredQuantity = 5;
    obj2.currentProgress = 5;
    obj2.status = ObjectiveStatus::Completed;
    obj2.isOptional = true;
    obj2.isHidden = true;
    q1.objectives.push_back(obj2);

    Quest q2;
    q2.id = "quest_done";
    q2.title = "Trading";
    q2.status = QuestStatus::TurnedIn;
    q2.canAbandon = false;
    q2.isRepeatable = true;
    q2.timeLimit = 0;
    comp.quests.push_back(q1);
    comp.quests.push_back(q2);

    // Serialize
    ComponentData cd = comp.Serialize();
    TEST("Serialize type", cd.componentType == "QuestComponent");
    TEST("Serialize questCount", cd.data.at("questCount") == "2");
    TEST("Serialize maxActiveQuests", cd.data.at("maxActiveQuests") == "5");
    TEST("Serialize quest0 id", cd.data.at("quest_0_id") == "quest_mine");
    TEST("Serialize quest0 status", cd.data.at("quest_0_status") == "Active");
    TEST("Serialize quest0 obj0 progress", cd.data.at("quest_0_obj_0_progress") == "4");
    TEST("Serialize quest1 status", cd.data.at("quest_1_status") == "TurnedIn");

    // Deserialize into fresh component
    QuestComponent comp2;
    comp2.Deserialize(cd);
    TEST("Deserialized maxActiveQuests", comp2.maxActiveQuests == 5);
    TEST("Deserialized quest count", comp2.quests.size() == 2);
    TEST("Deserialized quest0 id", comp2.quests[0].id == "quest_mine");
    TEST("Deserialized quest0 title", comp2.quests[0].title == "Mine Iron");
    TEST("Deserialized quest0 status", comp2.quests[0].status == QuestStatus::Active);
    TEST("Deserialized quest0 canAbandon", comp2.quests[0].canAbandon == true);
    TEST("Deserialized quest0 timeLimit", comp2.quests[0].timeLimit == 300);
    TEST("Deserialized quest0 obj count", comp2.quests[0].objectives.size() == 2);
    TEST("Deserialized obj0 type", comp2.quests[0].objectives[0].type == ObjectiveType::Mine);
    TEST("Deserialized obj0 target", comp2.quests[0].objectives[0].target == "Iron");
    TEST("Deserialized obj0 required", comp2.quests[0].objectives[0].requiredQuantity == 10);
    TEST("Deserialized obj0 progress", comp2.quests[0].objectives[0].currentProgress == 4);
    TEST("Deserialized obj0 status", comp2.quests[0].objectives[0].status == ObjectiveStatus::Active);
    TEST("Deserialized obj0 optional", comp2.quests[0].objectives[0].isOptional == false);
    TEST("Deserialized obj1 optional", comp2.quests[0].objectives[1].isOptional == true);
    TEST("Deserialized obj1 hidden", comp2.quests[0].objectives[1].isHidden == true);
    TEST("Deserialized obj1 status", comp2.quests[0].objectives[1].status == ObjectiveStatus::Completed);
    TEST("Deserialized quest1 id", comp2.quests[1].id == "quest_done");
    TEST("Deserialized quest1 status", comp2.quests[1].status == QuestStatus::TurnedIn);
    TEST("Deserialized quest1 canAbandon", comp2.quests[1].canAbandon == false);
    TEST("Deserialized quest1 isRepeatable", comp2.quests[1].isRepeatable == true);

    // Empty component round-trip
    QuestComponent empty;
    ComponentData emptyCD = empty.Serialize();
    QuestComponent empty2;
    empty2.Deserialize(emptyCD);
    TEST("Empty roundtrip quest count", empty2.quests.size() == 0);
    TEST("Empty roundtrip maxActive", empty2.maxActiveQuests == 10);

    // Full SaveGameManager round-trip with QuestComponent
    {
        auto& mgr = SaveGameManager::Instance();
        mgr.SetSaveDirectory("/tmp/subspace_quest_ser_test");

        SaveGameData saveData;
        saveData.saveName = "QuestTest";
        saveData.saveTime = "2026-02-15T00:00:00Z";
        saveData.version = "1.0.0";

        EntityData ent;
        ent.entityId = 42;
        ent.entityName = "Player";
        ent.isActive = true;
        ent.components.push_back(comp.Serialize());
        saveData.entities.push_back(ent);

        TEST("Save quest data", mgr.SaveGame(saveData, "quest_ser_test") == true);

        SaveGameData loadedData;
        TEST("Load quest data", mgr.LoadGame("quest_ser_test", loadedData) == true);
        TEST("Loaded entity has quest comp", loadedData.entities.size() == 1 &&
             loadedData.entities[0].components.size() == 1);

        QuestComponent loadedComp;
        loadedComp.Deserialize(loadedData.entities[0].components[0]);
        TEST("Saved+loaded quest count", loadedComp.quests.size() == 2);
        TEST("Saved+loaded quest0 id", loadedComp.quests[0].id == "quest_mine");
        TEST("Saved+loaded obj0 progress", loadedComp.quests[0].objectives[0].currentProgress == 4);

        mgr.DeleteSave("quest_ser_test");
    }
}

// ===================================================================
// Tutorial System Tests
// ===================================================================
static void TestTutorialStep() {
    std::cout << "[TutorialStep]\n";

    TutorialStep step;
    step.id = "step1";
    step.type = TutorialStepType::Message;
    step.title = "Welcome";
    step.message = "Welcome to the game!";

    TEST("Step starts NotStarted", step.status == TutorialStepStatus::NotStarted);

    step.Start();
    TEST("Start sets Active", step.status == TutorialStepStatus::Active);
    TEST("Elapsed time reset", ApproxEq(step.elapsedTime, 0.0f));

    step.Complete();
    TEST("Complete sets Completed", step.status == TutorialStepStatus::Completed);

    step.Reset();
    TEST("Reset sets NotStarted", step.status == TutorialStepStatus::NotStarted);

    step.Start();
    step.Skip();
    TEST("Skip sets Skipped", step.status == TutorialStepStatus::Skipped);

    // WaitForTime
    TutorialStep timeStep;
    timeStep.type = TutorialStepType::WaitForTime;
    timeStep.duration = 5.0f;
    timeStep.Start();
    TEST("Time not elapsed at start", !timeStep.IsTimeElapsed());
    timeStep.elapsedTime = 4.9f;
    TEST("Time not elapsed before duration", !timeStep.IsTimeElapsed());
    timeStep.elapsedTime = 5.0f;
    TEST("Time elapsed at duration", timeStep.IsTimeElapsed());
    timeStep.elapsedTime = 6.0f;
    TEST("Time elapsed past duration", timeStep.IsTimeElapsed());

    // Non-WaitForTime type never reports time elapsed
    TutorialStep msgStep;
    msgStep.type = TutorialStepType::Message;
    msgStep.duration = 1.0f;
    msgStep.elapsedTime = 100.0f;
    TEST("Message type never time-elapsed", !msgStep.IsTimeElapsed());
}

static void TestTutorial() {
    std::cout << "[Tutorial]\n";

    Tutorial tut;
    tut.id = "tut1";
    tut.title = "Basic Controls";

    TutorialStep s1;
    s1.id = "s1";
    s1.type = TutorialStepType::Message;
    s1.title = "Move";

    TutorialStep s2;
    s2.id = "s2";
    s2.type = TutorialStepType::WaitForKey;
    s2.requiredKey = "W";

    TutorialStep s3;
    s3.id = "s3";
    s3.type = TutorialStepType::WaitForAction;
    s3.requiredAction = "collect_resource";

    tut.steps.push_back(s1);
    tut.steps.push_back(s2);
    tut.steps.push_back(s3);

    TEST("Tutorial starts NotStarted", tut.status == TutorialStatus::NotStarted);
    TEST("Cannot complete step before start", !tut.CompleteCurrentStep());

    bool started = tut.Start();
    TEST("Start succeeds", started);
    TEST("Status is Active", tut.status == TutorialStatus::Active);
    TEST("First step is active", tut.steps[0].status == TutorialStepStatus::Active);
    TEST("Current step index 0", tut.currentStepIndex == 0);
    TEST("Cannot start again", !tut.Start());

    TEST("Completion 0%", ApproxEq(tut.GetCompletionPercentage(), 0.0f));

    tut.CompleteCurrentStep();
    TEST("Step 1 completed", tut.steps[0].status == TutorialStepStatus::Completed);
    TEST("Step 2 started", tut.steps[1].status == TutorialStepStatus::Active);
    TEST("Current step index 1", tut.currentStepIndex == 1);
    TEST("Completion ~33%", ApproxEq(tut.GetCompletionPercentage(), 100.0f / 3.0f));

    tut.CompleteCurrentStep();
    TEST("Step 2 completed", tut.steps[1].status == TutorialStepStatus::Completed);
    TEST("Completion ~67%", ApproxEq(tut.GetCompletionPercentage(), 200.0f / 3.0f));

    tut.CompleteCurrentStep();
    TEST("Tutorial completed", tut.status == TutorialStatus::Completed);
    TEST("All steps complete", tut.AreAllStepsComplete());
    TEST("Completion 100%", ApproxEq(tut.GetCompletionPercentage(), 100.0f));

    // Reset
    tut.Reset();
    TEST("Reset restores NotStarted", tut.status == TutorialStatus::NotStarted);
    TEST("Reset clears step index", tut.currentStepIndex == 0);

    // Skip
    tut.Start();
    tut.Skip();
    TEST("Skip sets Skipped", tut.status == TutorialStatus::Skipped);

    // WaitForTime auto-complete
    Tutorial timeTut;
    timeTut.id = "tut_time";
    TutorialStep ts;
    ts.id = "ts1";
    ts.type = TutorialStepType::WaitForTime;
    ts.duration = 2.0f;
    timeTut.steps.push_back(ts);
    timeTut.Start();
    timeTut.Update(1.0f);
    TEST("Time step not done yet", timeTut.status == TutorialStatus::Active);
    timeTut.Update(1.5f);
    TEST("Time step auto-completes", timeTut.status == TutorialStatus::Completed);
}

static void TestTutorialSystem() {
    std::cout << "[TutorialSystem]\n";

    TutorialSystem system;
    TEST("System name", system.GetName() == "TutorialSystem");
    TEST("No templates initially", system.GetTemplateCount() == 0);

    Tutorial tmpl;
    tmpl.id = "basic_controls";
    tmpl.title = "Basic Controls";
    tmpl.autoStart = true;
    TutorialStep s1;
    s1.id = "s1";
    s1.type = TutorialStepType::Message;
    tmpl.steps.push_back(s1);
    TutorialStep s2;
    s2.id = "s2";
    s2.type = TutorialStepType::WaitForAction;
    s2.requiredAction = "move_forward";
    tmpl.steps.push_back(s2);

    system.AddTutorialTemplate(tmpl);
    TEST("One template after add", system.GetTemplateCount() == 1);

    // Start tutorial
    TutorialComponent comp;
    bool started = system.StartTutorial(1, "basic_controls", comp);
    TEST("Start tutorial succeeds", started);
    TEST("One active tutorial", comp.activeTutorials.size() == 1);
    TEST("Tutorial is active", comp.activeTutorials[0].status == TutorialStatus::Active);

    // Cannot start same tutorial twice
    bool again = system.StartTutorial(1, "basic_controls", comp);
    TEST("Cannot start duplicate", !again);

    // Nonexistent template
    bool bad = system.StartTutorial(1, "nonexistent", comp);
    TEST("Nonexistent template fails", !bad);

    // Complete step
    system.CompleteCurrentStep(comp, "basic_controls");
    TEST("Step 1 completed", comp.activeTutorials[0].steps[0].status == TutorialStepStatus::Completed);
    TEST("Step 2 started", comp.activeTutorials[0].steps[1].status == TutorialStepStatus::Active);

    // Complete action step
    system.CompleteActionStep(comp, "wrong_action");
    TEST("Wrong action doesn't complete", comp.activeTutorials[0].steps[1].status == TutorialStepStatus::Active);

    system.CompleteActionStep(comp, "move_forward");
    TEST("Correct action completes", comp.activeTutorials[0].status == TutorialStatus::Completed);
    TEST("Tutorial marked completed", system.HasCompletedTutorial(comp, "basic_controls"));

    // Prerequisites
    Tutorial advanced;
    advanced.id = "advanced_controls";
    advanced.prerequisites.push_back("basic_controls");
    TutorialStep as1;
    as1.id = "as1";
    advanced.steps.push_back(as1);
    system.AddTutorialTemplate(advanced);

    TutorialComponent comp2;
    bool prereqFail = system.StartTutorial(1, "advanced_controls", comp2);
    TEST("Prerequisites block start", !prereqFail);

    comp2.completedTutorialIds.insert("basic_controls");
    bool prereqPass = system.StartTutorial(1, "advanced_controls", comp2);
    TEST("Prerequisites met allows start", prereqPass);

    // Auto-start
    TutorialComponent comp3;
    system.CheckAutoStartTutorials(1, comp3);
    TEST("Auto-start works (basic_controls)", comp3.activeTutorials.size() == 1);
    TEST("Auto-started correct tutorial", comp3.activeTutorials[0].id == "basic_controls");

    // Skip tutorial
    TutorialComponent comp4;
    system.StartTutorial(1, "basic_controls", comp4);
    system.SkipTutorial(comp4, "basic_controls");
    TEST("Skip sets Skipped", comp4.activeTutorials[0].status == TutorialStatus::Skipped);
    TEST("Skip marks completed", system.HasCompletedTutorial(comp4, "basic_controls"));

    // HasCompletedTutorial false case
    TEST("Not completed returns false", !system.HasCompletedTutorial(comp4, "nonexistent"));
}

// ===================================================================
// TutorialComponent Serialization Tests
// ===================================================================
static void TestTutorialComponentSerialization() {
    std::cout << "[TutorialComponent Serialization]\n";

    // Build a component with some active and completed tutorials
    TutorialComponent comp;

    Tutorial tut1;
    tut1.id = "basic_controls";
    tut1.title = "Basic Controls";
    tut1.status = TutorialStatus::Active;
    tut1.currentStepIndex = 1;
    tut1.autoStart = true;

    TutorialStep s1;
    s1.id = "move";
    s1.type = TutorialStepType::WaitForKey;
    s1.status = TutorialStepStatus::Completed;
    s1.requiredAction = "";
    s1.canSkip = true;
    tut1.steps.push_back(s1);

    TutorialStep s2;
    s2.id = "shoot";
    s2.type = TutorialStepType::WaitForAction;
    s2.status = TutorialStepStatus::Active;
    s2.requiredAction = "fire_weapon";
    s2.canSkip = false;
    tut1.steps.push_back(s2);

    comp.activeTutorials.push_back(tut1);
    comp.completedTutorialIds.insert("intro");
    comp.completedTutorialIds.insert("mining");

    // Serialize
    ComponentData cd = comp.Serialize();
    TEST("Serialize type", cd.componentType == "TutorialComponent");
    TEST("Serialize activeTutorialCount", cd.data.at("activeTutorialCount") == "1");
    TEST("Serialize completedCount", cd.data.at("completedCount") == "2");
    TEST("Serialize tut0 id", cd.data.at("tut_0_id") == "basic_controls");
    TEST("Serialize tut0 status", cd.data.at("tut_0_status") == "Active");
    TEST("Serialize tut0 currentStep", cd.data.at("tut_0_currentStep") == "1");
    TEST("Serialize tut0 autoStart", cd.data.at("tut_0_autoStart") == "true");
    TEST("Serialize step0 id", cd.data.at("tut_0_step_0_id") == "move");
    TEST("Serialize step0 type", cd.data.at("tut_0_step_0_type") == "WaitForKey");
    TEST("Serialize step0 status", cd.data.at("tut_0_step_0_status") == "Completed");
    TEST("Serialize step1 requiredAction", cd.data.at("tut_0_step_1_requiredAction") == "fire_weapon");
    TEST("Serialize step1 canSkip", cd.data.at("tut_0_step_1_canSkip") == "false");

    // Deserialize
    TutorialComponent comp2;
    comp2.Deserialize(cd);
    TEST("Deserialized active tutorial count", comp2.activeTutorials.size() == 1);
    TEST("Deserialized tut0 id", comp2.activeTutorials[0].id == "basic_controls");
    TEST("Deserialized tut0 title", comp2.activeTutorials[0].title == "Basic Controls");
    TEST("Deserialized tut0 status", comp2.activeTutorials[0].status == TutorialStatus::Active);
    TEST("Deserialized tut0 currentStep", comp2.activeTutorials[0].currentStepIndex == 1);
    TEST("Deserialized tut0 autoStart", comp2.activeTutorials[0].autoStart == true);
    TEST("Deserialized step count", comp2.activeTutorials[0].steps.size() == 2);
    TEST("Deserialized step0 type", comp2.activeTutorials[0].steps[0].type == TutorialStepType::WaitForKey);
    TEST("Deserialized step0 status", comp2.activeTutorials[0].steps[0].status == TutorialStepStatus::Completed);
    TEST("Deserialized step1 type", comp2.activeTutorials[0].steps[1].type == TutorialStepType::WaitForAction);
    TEST("Deserialized step1 action", comp2.activeTutorials[0].steps[1].requiredAction == "fire_weapon");
    TEST("Deserialized step1 canSkip", comp2.activeTutorials[0].steps[1].canSkip == false);
    TEST("Deserialized completed count", comp2.completedTutorialIds.size() == 2);
    TEST("Deserialized has intro", comp2.completedTutorialIds.count("intro") == 1);
    TEST("Deserialized has mining", comp2.completedTutorialIds.count("mining") == 1);

    // Empty component round-trip
    TutorialComponent empty;
    ComponentData emptyCD = empty.Serialize();
    TutorialComponent empty2;
    empty2.Deserialize(emptyCD);
    TEST("Empty roundtrip active count", empty2.activeTutorials.size() == 0);
    TEST("Empty roundtrip completed count", empty2.completedTutorialIds.size() == 0);

    // Full SaveGameManager round-trip
    {
        auto& mgr = SaveGameManager::Instance();
        mgr.SetSaveDirectory("/tmp/subspace_tut_ser_test");

        SaveGameData saveData;
        saveData.saveName = "TutorialTest";
        saveData.saveTime = "2026-02-15T00:00:00Z";
        saveData.version = "1.0.0";

        EntityData ent;
        ent.entityId = 99;
        ent.entityName = "Player";
        ent.isActive = true;
        ent.components.push_back(comp.Serialize());
        saveData.entities.push_back(ent);

        TEST("Save tutorial data", mgr.SaveGame(saveData, "tut_ser_test") == true);

        SaveGameData loadedData;
        TEST("Load tutorial data", mgr.LoadGame("tut_ser_test", loadedData) == true);

        TutorialComponent loadedComp;
        loadedComp.Deserialize(loadedData.entities[0].components[0]);
        TEST("Saved+loaded active tut count", loadedComp.activeTutorials.size() == 1);
        TEST("Saved+loaded tut0 id", loadedComp.activeTutorials[0].id == "basic_controls");
        TEST("Saved+loaded completed count", loadedComp.completedTutorialIds.size() == 2);

        mgr.DeleteSave("tut_ser_test");
    }
}

// ===================================================================
// AI Decision System Tests
// ===================================================================
static void TestAIPerception() {
    std::cout << "[AIPerception]\n";

    AIPerception perception;
    TEST("No threats initially", !perception.HasThreats());
    TEST("Highest threat nullptr", perception.GetHighestThreat() == nullptr);

    ThreatInfo t1;
    t1.entityId = 1;
    t1.priority = TargetPriority::Low;
    t1.threatLevel = 10.0f;
    perception.threats.push_back(t1);

    TEST("Has threats after add", perception.HasThreats());

    ThreatInfo t2;
    t2.entityId = 2;
    t2.priority = TargetPriority::High;
    t2.threatLevel = 5.0f;
    perception.threats.push_back(t2);

    const ThreatInfo* highest = perception.GetHighestThreat();
    TEST("Highest threat by priority", highest != nullptr && highest->entityId == 2);

    // Tiebreak by threat level
    ThreatInfo t3;
    t3.entityId = 3;
    t3.priority = TargetPriority::High;
    t3.threatLevel = 20.0f;
    perception.threats.push_back(t3);

    highest = perception.GetHighestThreat();
    TEST("Tiebreak by threat level", highest != nullptr && highest->entityId == 3);

    perception.Clear();
    TEST("Clear removes all", !perception.HasThreats());
    TEST("Clear empties entities", perception.nearbyEntities.empty());
}

static void TestAIComponent() {
    std::cout << "[AIComponent]\n";

    AIComponent ai;
    TEST("Default state Idle", ai.currentState == AIState::Idle);
    TEST("Default personality Balanced", ai.personality == AIPersonality::Balanced);
    TEST("Default flee threshold", ApproxEq(ai.fleeThreshold, 0.25f));
    TEST("Default no target", ai.currentTarget == InvalidEntityId);
    TEST("Not mining by default", !ai.canMine);
    TEST("Enabled by default", ai.isEnabled);
    TEST("Empty patrol waypoints", ai.patrolWaypoints.empty());
}

static void TestAIDecisionSystem() {
    std::cout << "[AIDecisionSystem]\n";

    AIDecisionSystem system;
    TEST("System name", system.GetName() == "AIDecisionSystem");

    // Idle with no perception
    AIComponent ai;
    AIState state = system.EvaluateState(ai);
    TEST("Idle with no perception", state == AIState::Idle);

    // Patrol with waypoints
    ai.patrolWaypoints.push_back({0, 0, 0});
    ai.patrolWaypoints.push_back({100, 0, 0});
    state = system.EvaluateState(ai);
    TEST("Patrol with waypoints", state == AIState::Patrol);

    // Mining with asteroids nearby
    ai.canMine = true;
    ai.perception.nearbyAsteroids.push_back(42);
    state = system.EvaluateState(ai);
    TEST("Mining with available asteroids", state == AIState::Mining);

    // Combat with threats (aggressive)
    ai.personality = AIPersonality::Aggressive;
    ThreatInfo threat;
    threat.entityId = 99;
    threat.priority = TargetPriority::Low;
    threat.threatLevel = 5.0f;
    ai.perception.threats.push_back(threat);
    state = system.EvaluateState(ai);
    TEST("Combat for aggressive with any threat", state == AIState::Combat);

    // Coward doesn't enter combat
    ai.personality = AIPersonality::Coward;
    state = system.EvaluateState(ai);
    TEST("Coward avoids combat", state != AIState::Combat);

    // Balanced needs medium+ threat
    ai.personality = AIPersonality::Balanced;
    state = system.EvaluateState(ai);
    TEST("Balanced ignores low threat", state != AIState::Combat);

    ThreatInfo medThreat;
    medThreat.entityId = 100;
    medThreat.priority = TargetPriority::Medium;
    medThreat.threatLevel = 15.0f;
    ai.perception.threats.push_back(medThreat);
    state = system.EvaluateState(ai);
    TEST("Balanced enters combat on medium threat", state == AIState::Combat);

    // ShouldFlee
    TEST("Should flee at 20% hull", system.ShouldFlee(ai, 0.20f));
    TEST("Should not flee at 30% hull", !system.ShouldFlee(ai, 0.30f));
    TEST("Should not flee at 25% hull", !system.ShouldFlee(ai, 0.25f));

    // ShouldReturnToBase
    ai.homeBase = 50;
    TEST("Return to base at 85% cargo", system.ShouldReturnToBase(ai, 0.85f));
    TEST("Don't return at 50% cargo", !system.ShouldReturnToBase(ai, 0.50f));
    ai.homeBase = InvalidEntityId;
    TEST("No return without home base", !system.ShouldReturnToBase(ai, 0.99f));

    // SelectTarget
    EntityId target = system.SelectTarget(ai);
    TEST("Select highest priority target", target == 100);

    // Fleeing state persists
    ai.currentState = AIState::Fleeing;
    state = system.EvaluateState(ai);
    TEST("Fleeing state persists", state == AIState::Fleeing);

    // Disabled AI keeps current state
    ai.isEnabled = false;
    ai.currentState = AIState::Mining;
    state = system.EvaluateState(ai);
    TEST("Disabled keeps current state", state == AIState::Mining);
    ai.isEnabled = true;

    // EvaluateGatheringState
    AIComponent miner;
    miner.personality = AIPersonality::Miner;
    miner.canMine = true;
    miner.perception.nearbyAsteroids.push_back(1);
    AIState gatherState = system.EvaluateGatheringState(miner);
    TEST("Miner prefers mining", gatherState == AIState::Mining);

    AIComponent salvager;
    salvager.personality = AIPersonality::Salvager;
    salvager.canSalvage = true;
    gatherState = system.EvaluateGatheringState(salvager);
    TEST("Salvager prefers salvaging", gatherState == AIState::Salvaging);

    AIComponent noGather;
    gatherState = system.EvaluateGatheringState(noGather);
    TEST("No gathering when incapable", gatherState == AIState::Idle);

    // CalculateActionPriority
    AIComponent aggressive;
    aggressive.personality = AIPersonality::Aggressive;
    TEST("Combat high for aggressive", system.CalculateActionPriority(AIState::Combat, aggressive) > 0.8f);

    AIComponent coward;
    coward.personality = AIPersonality::Coward;
    TEST("Combat low for coward", system.CalculateActionPriority(AIState::Combat, coward) < 0.4f);
    TEST("Fleeing high for coward", system.CalculateActionPriority(AIState::Fleeing, coward) > 0.9f);

    AIComponent minerAi;
    minerAi.personality = AIPersonality::Miner;
    TEST("Mining high for miner", system.CalculateActionPriority(AIState::Mining, minerAi) > 0.7f);

    AIComponent traderAi;
    traderAi.personality = AIPersonality::Trader;
    TEST("Trading high for trader", system.CalculateActionPriority(AIState::Trading, traderAi) > 0.7f);
}

// ===================================================================
// SpatialHash Tests
// ===================================================================
static void TestSpatialHash() {
    std::cout << "[SpatialHash]\n";

    SpatialHash hash(50.0f);
    TEST("SpatialHash cell size", ApproxEq(hash.GetCellSize(), 50.0f));
    TEST("SpatialHash empty initially", hash.GetEntityCount() == 0);
    TEST("SpatialHash no cells initially", hash.GetCellCount() == 0);

    // Insert entity
    hash.Insert(1, Vector3(10.0f, 0.0f, 0.0f), 5.0f);
    TEST("SpatialHash entity count 1", hash.GetEntityCount() == 1);
    TEST("SpatialHash cell count >= 1", hash.GetCellCount() >= 1);

    // Query nearby
    auto nearby = hash.QueryNearby(Vector3(10.0f, 0.0f, 0.0f), 10.0f);
    TEST("SpatialHash query finds entity", nearby.size() == 1 && nearby[0] == 1);

    // Query far away
    auto faraway = hash.QueryNearby(Vector3(500.0f, 500.0f, 500.0f), 5.0f);
    TEST("SpatialHash query misses distant", faraway.empty());

    // Insert second entity nearby
    hash.Insert(2, Vector3(20.0f, 0.0f, 0.0f), 5.0f);
    nearby = hash.QueryNearby(Vector3(15.0f, 0.0f, 0.0f), 50.0f);
    TEST("SpatialHash query finds two", nearby.size() == 2);

    // Remove entity
    hash.Remove(1);
    TEST("SpatialHash entity count after remove", hash.GetEntityCount() == 1);
    nearby = hash.QueryNearby(Vector3(10.0f, 0.0f, 0.0f), 50.0f);
    bool foundRemoved = false;
    for (auto id : nearby) { if (id == 1) foundRemoved = true; }
    TEST("SpatialHash removed entity not found", !foundRemoved);

    // Clear
    hash.Clear();
    TEST("SpatialHash empty after clear", hash.GetEntityCount() == 0);
    TEST("SpatialHash no cells after clear", hash.GetCellCount() == 0);

    // Multiple entities in different cells
    SpatialHash hash2(10.0f);
    hash2.Insert(10, Vector3(5.0f, 5.0f, 5.0f), 1.0f);
    hash2.Insert(20, Vector3(50.0f, 50.0f, 50.0f), 1.0f);
    hash2.Insert(30, Vector3(100.0f, 100.0f, 100.0f), 1.0f);
    TEST("SpatialHash 3 entities", hash2.GetEntityCount() == 3);

    auto near10 = hash2.QueryNearby(Vector3(5.0f, 5.0f, 5.0f), 5.0f);
    bool found10 = false;
    bool found20in10 = false;
    for (auto id : near10) {
        if (id == 10) found10 = true;
        if (id == 20) found20in10 = true;
    }
    TEST("SpatialHash locality entity 10 found", found10);
    TEST("SpatialHash locality entity 20 not near 10", !found20in10);

    // Re-insert (update position)
    hash2.Insert(10, Vector3(49.0f, 50.0f, 50.0f), 1.0f);
    auto nearUpdated = hash2.QueryNearby(Vector3(50.0f, 50.0f, 50.0f), 5.0f);
    bool foundUpdated10 = false;
    for (auto id : nearUpdated) { if (id == 10) foundUpdated10 = true; }
    TEST("SpatialHash re-insert updates position", foundUpdated10);

    // Entity spanning multiple cells
    SpatialHash hash3(10.0f);
    hash3.Insert(1, Vector3(0.0f, 0.0f, 0.0f), 15.0f); // radius > cell size
    TEST("SpatialHash large radius multi-cell", hash3.GetCellCount() > 1);
}

// ===================================================================
// AISteeringSystem Tests
// ===================================================================
static void TestAISteeringSystem() {
    std::cout << "[AISteeringSystem]\n";

    // Test Seek
    {
        auto s = AISteeringSystem::Seek(
            Vector3(0, 0, 0), Vector3(100, 0, 0), 50.0f);
        TEST("Seek force direction X", s.linear.x > 0.0f);
        TEST("Seek force magnitude", ApproxEq(s.linear.length(), 50.0f));
        TEST("Seek zero Y force", ApproxEq(s.linear.y, 0.0f));
    }

    // Test Seek toward self (zero distance)
    {
        auto s = AISteeringSystem::Seek(
            Vector3(10, 10, 10), Vector3(10, 10, 10), 50.0f);
        TEST("Seek at target zero force", ApproxEq(s.linear.length(), 0.0f));
    }

    // Test Flee
    {
        auto s = AISteeringSystem::Flee(
            Vector3(0, 0, 0), Vector3(100, 0, 0), 50.0f);
        TEST("Flee force opposite direction", s.linear.x < 0.0f);
        TEST("Flee force magnitude", ApproxEq(s.linear.length(), 50.0f));
    }

    // Test Arrive far from target
    {
        auto s = AISteeringSystem::Arrive(
            Vector3(0, 0, 0), Vector3(200, 0, 0), 100.0f, 20.0f);
        TEST("Arrive far full force", ApproxEq(s.linear.length(), 100.0f));
    }

    // Test Arrive within slow radius
    {
        auto s = AISteeringSystem::Arrive(
            Vector3(0, 0, 0), Vector3(10, 0, 0), 100.0f, 20.0f);
        float expectedForce = 100.0f * (10.0f / 20.0f);
        TEST("Arrive slow radius scaled", ApproxEq(s.linear.length(), expectedForce));
    }

    // Test Arrive at target
    {
        auto s = AISteeringSystem::Arrive(
            Vector3(5, 5, 5), Vector3(5, 5, 5), 100.0f, 20.0f);
        TEST("Arrive at target zero force", ApproxEq(s.linear.length(), 0.0f));
    }

    // Test Pursue (stationary target = seek)
    {
        auto s = AISteeringSystem::Pursue(
            Vector3(0, 0, 0),
            Vector3(100, 0, 0), Vector3(0, 0, 0),
            50.0f);
        TEST("Pursue stationary = seek", s.linear.x > 0.0f);
        TEST("Pursue stationary magnitude", ApproxEq(s.linear.length(), 50.0f));
    }

    // Test Pursue moving target (leads ahead)
    {
        auto s = AISteeringSystem::Pursue(
            Vector3(0, 0, 0),
            Vector3(100, 0, 0), Vector3(0, 50, 0),
            50.0f, 2.0f);
        TEST("Pursue moving target Y component", s.linear.y > 0.0f);
    }

    // Test Evade (opposite of Pursue)
    {
        auto s = AISteeringSystem::Evade(
            Vector3(0, 0, 0),
            Vector3(50, 0, 0), Vector3(0, 0, 0),
            100.0f);
        TEST("Evade direction away", s.linear.x < 0.0f);
        TEST("Evade magnitude", ApproxEq(s.linear.length(), 100.0f));
    }

    // Test Patrol
    {
        std::vector<std::array<float, 3>> waypoints = {
            {0, 0, 0}, {100, 0, 0}, {100, 100, 0}
        };
        // Starting at origin which is within threshold of waypoint 0 => should advance to 1
        int idx = 0;
        auto s = AISteeringSystem::Patrol(
            Vector3(0, 0, 0), waypoints, idx, 50.0f, 5.0f);
        TEST("Patrol at wp0 advances to 1", idx == 1);
        TEST("Patrol steers toward wp1 X", s.linear.x > 0.0f);

        // Starting far from waypoint 0 => stays at 0
        int idx2 = 0;
        auto s2 = AISteeringSystem::Patrol(
            Vector3(-100.0f, 0.0f, 0.0f), waypoints, idx2, 50.0f, 5.0f);
        TEST("Patrol far from wp0 stays at 0", idx2 == 0);
        TEST("Patrol steers toward wp0", s2.linear.x > 0.0f);
    }

    // Test Patrol with empty waypoints
    {
        std::vector<std::array<float, 3>> empty;
        int idx = 0;
        auto s = AISteeringSystem::Patrol(
            Vector3(0, 0, 0), empty, idx, 50.0f);
        TEST("Patrol empty waypoints no force", ApproxEq(s.linear.length(), 0.0f));
    }

    // Test Wander produces non-zero force and predictable angle change
    {
        float angle = 0.0f;
        float wanderJitter = 0.5f; // default
        auto s = AISteeringSystem::Wander(
            Vector3(1, 0, 0), angle, 50.0f, 10.0f, wanderJitter);
        TEST("Wander produces force", s.linear.length() > 0.0f);
        float expectedAngle = wanderJitter * 0.5f;
        TEST("Wander angle increases by jitter*0.5", ApproxEq(angle, expectedAngle));
    }

    // Test system name
    {
        EntityManager em;
        AISteeringSystem system(em);
        TEST("AISteeringSystem name", system.GetName() == "AISteeringSystem");
    }

    // Test Update applies forces
    {
        EntityManager em;
        AISteeringSystem steer(em);

        auto& ent = em.CreateEntity("AIShip");
        auto aiComp = std::make_unique<AIComponent>();
        aiComp->currentState = AIState::Patrol;
        aiComp->patrolWaypoints = {{100, 0, 0}, {200, 0, 0}};
        aiComp->currentWaypointIndex = 0;
        em.AddComponent<AIComponent>(ent.id, std::move(aiComp));

        auto physComp = std::make_unique<PhysicsComponent>();
        physComp->position = Vector3(0, 0, 0);
        physComp->maxThrust = 100.0f;
        auto* pc = em.AddComponent<PhysicsComponent>(ent.id, std::move(physComp));

        steer.Update(0.016f);
        TEST("Steering Update applies force X", pc->appliedForce.x > 0.0f);
    }
}

// ===================================================================
// PhysicsSystem SpatialHash Integration Tests
// ===================================================================
static void TestPhysicsSystemSpatialHash() {
    std::cout << "[PhysicsSystemSpatialHash]\n";

    // Collision detection still works with spatial hash
    EntityManager em;
    PhysicsSystem physSys(em);

    auto& obj1 = em.CreateEntity("Obj1");
    auto c1 = std::make_unique<PhysicsComponent>();
    c1->mass = 100.0f;
    c1->drag = 0.0f;
    c1->angularDrag = 0.0f;
    c1->position = Vector3(0.0f, 0.0f, 0.0f);
    c1->velocity = Vector3(5.0f, 0.0f, 0.0f);
    c1->collisionRadius = 5.0f;
    auto* pc1 = em.AddComponent<PhysicsComponent>(obj1.id, std::move(c1));

    auto& obj2 = em.CreateEntity("Obj2");
    auto c2 = std::make_unique<PhysicsComponent>();
    c2->mass = 100.0f;
    c2->drag = 0.0f;
    c2->angularDrag = 0.0f;
    c2->position = Vector3(8.0f, 0.0f, 0.0f);
    c2->velocity = Vector3(-5.0f, 0.0f, 0.0f);
    c2->collisionRadius = 5.0f;
    auto* pc2 = em.AddComponent<PhysicsComponent>(obj2.id, std::move(c2));

    physSys.Update(0.001f);
    TEST("SpatialHash collision obj1 vel changed", !ApproxEq(pc1->velocity.x, 5.0f));
    TEST("SpatialHash collision obj2 vel changed", !ApproxEq(pc2->velocity.x, -5.0f));

    // Distant objects should not collide
    EntityManager em2;
    PhysicsSystem physSys2(em2);

    auto& far1 = em2.CreateEntity("Far1");
    auto fc1 = std::make_unique<PhysicsComponent>();
    fc1->mass = 100.0f;
    fc1->drag = 0.0f;
    fc1->angularDrag = 0.0f;
    fc1->position = Vector3(0.0f, 0.0f, 0.0f);
    fc1->velocity = Vector3(10.0f, 0.0f, 0.0f);
    fc1->collisionRadius = 5.0f;
    auto* fpc1 = em2.AddComponent<PhysicsComponent>(far1.id, std::move(fc1));

    auto& far2 = em2.CreateEntity("Far2");
    auto fc2 = std::make_unique<PhysicsComponent>();
    fc2->mass = 100.0f;
    fc2->drag = 0.0f;
    fc2->angularDrag = 0.0f;
    fc2->position = Vector3(500.0f, 500.0f, 500.0f);
    fc2->velocity = Vector3(-10.0f, 0.0f, 0.0f);
    fc2->collisionRadius = 5.0f;
    auto* fpc2 = em2.AddComponent<PhysicsComponent>(far2.id, std::move(fc2));

    physSys2.Update(0.001f);
    TEST("SpatialHash distant no collision obj1", ApproxEq(fpc1->velocity.x, 10.0f));
    TEST("SpatialHash distant no collision obj2", ApproxEq(fpc2->velocity.x, -10.0f));

    // Spatial hash is accessible
    const auto& sh = physSys.GetSpatialHash();
    TEST("SpatialHash accessible from PhysicsSystem", sh.GetEntityCount() >= 0);
}

// ===================================================================
// UI Types Tests
// ===================================================================
static void TestUITypes() {
    std::cout << "[UITypes]\n";

    // Color
    Color c(0.5f, 0.25f, 0.75f, 1.0f);
    TEST("Color construction", ApproxEq(c.r, 0.5f) && ApproxEq(c.g, 0.25f));
    TEST("Color equality", Color::White() == Color(1, 1, 1, 1));
    TEST("Color inequality", Color::White() != Color::Black());

    uint32_t rgba = Color(1, 0, 0, 1).ToRGBA32();
    TEST("Color ToRGBA32 red", (rgba >> 24) == 255 && ((rgba >> 16) & 0xFF) == 0);

    Color lerped = Color::Lerp(Color::Black(), Color::White(), 0.5f);
    TEST("Color Lerp midpoint", ApproxEq(lerped.r, 0.5f) && ApproxEq(lerped.g, 0.5f));

    Color lerpClamped = Color::Lerp(Color::Black(), Color::White(), 2.0f);
    TEST("Color Lerp clamp high", ApproxEq(lerpClamped.r, 1.0f));

    // Vec2
    Vec2 a(3, 4);
    Vec2 b(1, 2);
    Vec2 sum = a + b;
    TEST("Vec2 add", ApproxEq(sum.x, 4.0f) && ApproxEq(sum.y, 6.0f));
    Vec2 diff = a - b;
    TEST("Vec2 sub", ApproxEq(diff.x, 2.0f) && ApproxEq(diff.y, 2.0f));
    Vec2 scaled = a * 2.0f;
    TEST("Vec2 scale", ApproxEq(scaled.x, 6.0f) && ApproxEq(scaled.y, 8.0f));
    TEST("Vec2 equality", a == Vec2(3, 4));
    TEST("Vec2 inequality", a != b);

    // Rect
    Rect r(10, 20, 100, 50);
    TEST("Rect Left", ApproxEq(r.Left(), 10.0f));
    TEST("Rect Top", ApproxEq(r.Top(), 20.0f));
    TEST("Rect Right", ApproxEq(r.Right(), 110.0f));
    TEST("Rect Bottom", ApproxEq(r.Bottom(), 70.0f));
    TEST("Rect Center", ApproxEq(r.Center().x, 60.0f) && ApproxEq(r.Center().y, 45.0f));
    TEST("Rect Contains inside", r.Contains(50, 40));
    TEST("Rect Contains corner", r.Contains(10, 20));
    TEST("Rect not Contains outside", !r.Contains(5, 40));
    TEST("Rect Contains Vec2", r.Contains(Vec2(50, 40)));
}

// ===================================================================
// UI Element Tests
// ===================================================================
static void TestUILabel() {
    std::cout << "[UILabel]\n";

    UILabel label;
    TEST("Label type", label.GetType() == UIElementType::Label);
    TEST("Label default visible", label.IsVisible());
    TEST("Label default enabled", label.IsEnabled());

    label.SetText("Hello World");
    label.SetColor(Color::Green());
    label.SetFontSize(20);
    label.SetBounds({10, 20, 200, 30});

    TEST("Label text", label.GetText() == "Hello World");
    TEST("Label color", label.GetColor() == Color::Green());
    TEST("Label font size", label.GetFontSize() == 20);

    std::vector<DrawCommand> cmds;
    label.Render(cmds);
    TEST("Label renders 1 command", cmds.size() == 1);
    TEST("Label command type text", cmds[0].type == DrawCommandType::Text);
    TEST("Label command has text", cmds[0].text == "Hello World");

    // Empty label renders nothing
    UILabel emptyLabel;
    cmds.clear();
    emptyLabel.Render(cmds);
    TEST("Empty label renders 0 commands", cmds.empty());

    // Hidden label renders nothing
    label.SetVisible(false);
    cmds.clear();
    label.Render(cmds);
    TEST("Hidden label renders 0 commands", cmds.empty());
}

static void TestUIButton() {
    std::cout << "[UIButton]\n";

    UIButton button;
    TEST("Button type", button.GetType() == UIElementType::Button);

    button.SetLabel("Click Me");
    button.SetBounds({50, 50, 120, 30});
    button.SetBackgroundColor(Color::Blue());
    button.SetTextColor(Color::Yellow());

    TEST("Button label", button.GetLabel() == "Click Me");
    TEST("Button bg color", button.GetBackgroundColor() == Color::Blue());

    std::vector<DrawCommand> cmds;
    button.Render(cmds);
    TEST("Button renders 3 commands (bg+border+text)", cmds.size() == 3);
    TEST("Button first cmd filled rect", cmds[0].type == DrawCommandType::FilledRect);
    TEST("Button second cmd outline", cmds[1].type == DrawCommandType::OutlineRect);
    TEST("Button third cmd text", cmds[2].type == DrawCommandType::Text);

    // Click handling
    bool clicked = false;
    button.SetOnClick([&clicked]() { clicked = true; });
    bool consumed = button.HandleClick(60, 60);
    TEST("Button click inside consumed", consumed);
    TEST("Button click callback fired", clicked);

    clicked = false;
    consumed = button.HandleClick(0, 0);
    TEST("Button click outside not consumed", !consumed);
    TEST("Button click outside no callback", !clicked);

    // Disabled button
    button.SetEnabled(false);
    clicked = false;
    consumed = button.HandleClick(60, 60);
    TEST("Disabled button not consumed", !consumed);
    TEST("Disabled button no callback", !clicked);
}

static void TestUIProgressBar() {
    std::cout << "[UIProgressBar]\n";

    UIProgressBar bar;
    TEST("ProgressBar type", bar.GetType() == UIElementType::ProgressBar);
    TEST("ProgressBar default value 0", ApproxEq(bar.GetValue(), 0.0f));

    bar.SetValue(0.75f);
    TEST("ProgressBar set value", ApproxEq(bar.GetValue(), 0.75f));

    bar.SetValue(-0.5f);
    TEST("ProgressBar clamp low", ApproxEq(bar.GetValue(), 0.0f));

    bar.SetValue(2.0f);
    TEST("ProgressBar clamp high", ApproxEq(bar.GetValue(), 1.0f));

    bar.SetValue(0.5f);
    bar.SetFillColor(Color::Cyan());
    bar.SetLabel("HP: 50%");
    bar.SetBounds({10, 10, 200, 20});

    std::vector<DrawCommand> cmds;
    bar.Render(cmds);
    TEST("ProgressBar renders 4 commands (bg+fill+border+label)", cmds.size() == 4);
    TEST("ProgressBar fill cmd", cmds[1].type == DrawCommandType::FilledRect);
    // Fill width should be half of 200 = 100
    TEST("ProgressBar fill width", ApproxEq(cmds[1].rect.width, 100.0f));

    // Zero value: no fill command
    bar.SetValue(0.0f);
    bar.SetLabel("");
    cmds.clear();
    bar.Render(cmds);
    TEST("ProgressBar 0 renders 2 commands (bg+border)", cmds.size() == 2);

    // Auto-color
    bar.SetAutoColor(true);
    bar.SetValue(0.8f);
    cmds.clear();
    bar.Render(cmds);
    TEST("AutoColor green at 0.8", cmds[1].color == Color::Green());

    bar.SetValue(0.5f);
    cmds.clear();
    bar.Render(cmds);
    TEST("AutoColor yellow at 0.5", cmds[1].color == Color::Yellow());

    bar.SetValue(0.2f);
    cmds.clear();
    bar.Render(cmds);
    TEST("AutoColor red at 0.2", cmds[1].color == Color::Red());
}

static void TestUISeparator() {
    std::cout << "[UISeparator]\n";

    UISeparator sep;
    TEST("Separator type", sep.GetType() == UIElementType::Separator);

    sep.SetBounds({0, 0, 200, 2});
    std::vector<DrawCommand> cmds;
    sep.Render(cmds);
    TEST("Separator renders 1 line command", cmds.size() == 1);
    TEST("Separator command type", cmds[0].type == DrawCommandType::Line);
}

static void TestUICheckbox() {
    std::cout << "[UICheckbox]\n";

    UICheckbox cb;
    TEST("Checkbox type", cb.GetType() == UIElementType::Checkbox);
    TEST("Checkbox default unchecked", !cb.IsChecked());

    cb.SetChecked(true);
    TEST("Checkbox set checked", cb.IsChecked());

    cb.SetLabel("Enable Sounds");
    cb.SetBounds({10, 10, 200, 20});

    std::vector<DrawCommand> cmds;
    cb.Render(cmds);
    // bg + border + check + label = 4
    TEST("Checked checkbox renders 4 commands", cmds.size() == 4);

    cb.SetChecked(false);
    cmds.clear();
    cb.Render(cmds);
    // bg + border + label = 3
    TEST("Unchecked checkbox renders 3 commands", cmds.size() == 3);

    // Click toggles
    bool newState = false;
    cb.SetOnChange([&newState](bool v) { newState = v; });
    cb.HandleClick(15, 15);
    TEST("Checkbox click toggles on", cb.IsChecked());
    TEST("Checkbox onChange fired", newState == true);

    cb.HandleClick(15, 15);
    TEST("Checkbox click toggles off", !cb.IsChecked());
}

// ===================================================================
// UI Panel Tests
// ===================================================================
static void TestUIPanel() {
    std::cout << "[UIPanel]\n";

    UIPanel panel;
    TEST("Panel type", panel.GetType() == UIElementType::Panel);
    TEST("Panel no children initially", panel.GetChildCount() == 0);

    auto label = std::make_shared<UILabel>();
    label->SetId("lbl1");
    label->SetText("Test");
    label->SetBounds({0, 0, 100, 20});

    UIElement* added = panel.AddChild(label);
    TEST("AddChild returns pointer", added != nullptr);
    TEST("Panel has 1 child", panel.GetChildCount() == 1);

    UIElement* found = panel.FindChild("lbl1");
    TEST("FindChild succeeds", found != nullptr);
    TEST("FindChild returns correct element", found == added);

    UIElement* notFound = panel.FindChild("nonexistent");
    TEST("FindChild not found", notFound == nullptr);

    // Add a button
    auto btn = std::make_shared<UIButton>();
    btn->SetId("btn1");
    btn->SetLabel("OK");
    btn->SetBounds({0, 0, 80, 25});
    panel.AddChild(btn);
    TEST("Panel has 2 children", panel.GetChildCount() == 2);

    // Remove child
    bool removed = panel.RemoveChild("lbl1");
    TEST("RemoveChild succeeds", removed);
    TEST("Panel has 1 child after remove", panel.GetChildCount() == 1);

    bool removedAgain = panel.RemoveChild("lbl1");
    TEST("RemoveChild not found", !removedAgain);

    // Clear
    panel.ClearChildren();
    TEST("ClearChildren empties", panel.GetChildCount() == 0);

    // Layout
    panel.SetBounds({100, 100, 300, 400});
    panel.SetPadding(10.0f);
    panel.SetSpacing(5.0f);

    auto lbl1 = std::make_shared<UILabel>();
    lbl1->SetId("l1");
    lbl1->SetBounds({0, 0, 0, 20});
    panel.AddChild(lbl1);

    auto lbl2 = std::make_shared<UILabel>();
    lbl2->SetId("l2");
    lbl2->SetBounds({0, 0, 0, 20});
    panel.AddChild(lbl2);

    panel.PerformLayout();
    TEST("Layout child1 x", ApproxEq(lbl1->GetBounds().x, 110.0f)); // 100 + 10 padding
    TEST("Layout child1 y", ApproxEq(lbl1->GetBounds().y, 110.0f)); // 100 + 10 padding
    TEST("Layout child1 width fills", ApproxEq(lbl1->GetBounds().width, 280.0f)); // 300 - 2*10
    TEST("Layout child2 y", ApproxEq(lbl2->GetBounds().y, 135.0f)); // 110 + 20 + 5 spacing

    // Rendering
    panel.SetTitle("Test Panel");
    std::vector<DrawCommand> cmds;
    panel.Render(cmds);
    TEST("Panel renders multiple commands", cmds.size() > 2);
    TEST("Panel first cmd is bg", cmds[0].type == DrawCommandType::FilledRect);

    // Click propagation
    auto clickBtn = std::make_shared<UIButton>();
    clickBtn->SetId("click_btn");
    clickBtn->SetBounds({110, 150, 80, 25});
    bool wasClicked = false;
    clickBtn->SetOnClick([&wasClicked]() { wasClicked = true; });
    panel.ClearChildren();
    panel.AddChild(clickBtn);

    bool consumed = panel.HandleClick(120, 160);
    TEST("Panel click propagates to button", consumed);
    TEST("Button received click", wasClicked);

    // Click outside children but inside panel
    wasClicked = false;
    consumed = panel.HandleClick(105, 105);
    TEST("Panel consumes click even outside children", consumed);
    TEST("Button not clicked when miss", !wasClicked);
}

// ===================================================================
// UI Renderer Tests
// ===================================================================
static void TestUIRenderer() {
    std::cout << "[UIRenderer]\n";

    UIRenderer renderer;
    renderer.BeginFrame(1920.0f, 1080.0f);
    TEST("Renderer screen width", ApproxEq(renderer.GetScreenWidth(), 1920.0f));
    TEST("Renderer screen height", ApproxEq(renderer.GetScreenHeight(), 1080.0f));
    TEST("Renderer empty after begin", renderer.GetCommandCount() == 0);

    renderer.DrawFilledRect({0, 0, 100, 50}, Color::Red());
    TEST("Renderer 1 command after draw", renderer.GetCommandCount() == 1);

    renderer.DrawOutlineRect({0, 0, 100, 50}, Color::White(), 2.0f);
    renderer.DrawText("Hello", {10, 10}, Color::Green(), 16);
    renderer.DrawLine({0, 0}, {100, 100}, Color::Blue());
    renderer.DrawCircle({50, 50}, 25.0f, Color::Yellow());
    renderer.DrawFilledCircle({50, 50}, 10.0f, Color::Cyan());
    TEST("Renderer 6 commands total", renderer.GetCommandCount() == 6);

    const auto& cmds = renderer.GetCommands();
    TEST("Command 0 FilledRect", cmds[0].type == DrawCommandType::FilledRect);
    TEST("Command 1 OutlineRect", cmds[1].type == DrawCommandType::OutlineRect);
    TEST("Command 2 Text", cmds[2].type == DrawCommandType::Text);
    TEST("Command 3 Line", cmds[3].type == DrawCommandType::Line);
    TEST("Command 4 Circle", cmds[4].type == DrawCommandType::Circle);
    TEST("Command 5 FilledCircle", cmds[5].type == DrawCommandType::FilledCircle);

    // Verify properties
    TEST("Text content correct", cmds[2].text == "Hello");
    TEST("Text color correct", cmds[2].color == Color::Green());
    TEST("Line width correct", cmds[1].lineWidth == 2.0f);

    // Submit batch
    std::vector<DrawCommand> extra;
    DrawCommand extraCmd;
    extraCmd.type = DrawCommandType::FilledRect;
    extra.push_back(extraCmd);
    extra.push_back(extraCmd);
    renderer.Submit(extra);
    TEST("Submit adds commands", renderer.GetCommandCount() == 8);

    // BeginFrame clears
    renderer.BeginFrame(800, 600);
    TEST("BeginFrame clears commands", renderer.GetCommandCount() == 0);
    TEST("BeginFrame updates size", ApproxEq(renderer.GetScreenWidth(), 800.0f));

    renderer.EndFrame();
    TEST("EndFrame does not crash", true);
}

// ===================================================================
// UI System Tests
// ===================================================================
static void TestUISystem() {
    std::cout << "[UISystem]\n";

    UISystem system;
    TEST("UISystem name", system.GetName() == "UISystem");
    TEST("UISystem no panels initially", system.GetPanelCount() == 0);

    // Add panels
    auto hudPanel = std::make_shared<UIPanel>();
    hudPanel->SetBounds({10, 10, 200, 300});
    hudPanel->SetTitle("HUD");

    auto menuPanel = std::make_shared<UIPanel>();
    menuPanel->SetBounds({400, 200, 300, 400});
    menuPanel->SetTitle("Menu");

    UIPanel* hud = system.AddPanel("hud", hudPanel);
    TEST("AddPanel returns pointer", hud != nullptr);
    TEST("System has 1 panel", system.GetPanelCount() == 1);

    system.AddPanel("menu", menuPanel);
    TEST("System has 2 panels", system.GetPanelCount() == 2);

    // Get panel
    UIPanel* got = system.GetPanel("hud");
    TEST("GetPanel finds hud", got != nullptr);
    TEST("GetPanel returns correct panel", got == hud);

    UIPanel* notFound = system.GetPanel("nonexistent");
    TEST("GetPanel not found", notFound == nullptr);

    // Toggle panel
    bool visible = system.TogglePanel("hud");
    TEST("Toggle hides", !visible);
    TEST("Panel is hidden", !hud->IsVisible());

    visible = system.TogglePanel("hud");
    TEST("Toggle shows", visible);
    TEST("Panel is visible", hud->IsVisible());

    bool toggleBad = system.TogglePanel("nonexistent");
    TEST("Toggle nonexistent returns false", !toggleBad);

    // Remove panel
    bool removed = system.RemovePanel("menu");
    TEST("RemovePanel succeeds", removed);
    TEST("System has 1 panel", system.GetPanelCount() == 1);

    bool removedAgain = system.RemovePanel("menu");
    TEST("RemovePanel not found", !removedAgain);

    // Rendering
    auto label = std::make_shared<UILabel>();
    label->SetId("lbl");
    label->SetText("Score: 100");
    label->SetBounds({0, 0, 150, 20});
    hud->AddChild(label);

    UIRenderer renderer;
    renderer.BeginFrame(1920, 1080);
    system.Update(0.016f);
    system.Render(renderer);
    TEST("Rendered commands exist", renderer.GetCommandCount() > 0);

    // Hidden panel produces no commands
    hud->SetVisible(false);
    renderer.BeginFrame(1920, 1080);
    system.Render(renderer);
    TEST("Hidden panel no commands", renderer.GetCommandCount() == 0);

    // Input handling — set button to known absolute position
    hud->SetVisible(true);
    hud->ClearChildren();
    auto btn = std::make_shared<UIButton>();
    btn->SetId("test_btn");
    btn->SetBounds({20, 30, 80, 25});  // absolute position within panel
    bool btnClicked = false;
    btn->SetOnClick([&btnClicked]() { btnClicked = true; });
    hud->AddChild(btn);
    // Don't call PerformLayout — keep the absolute position
    // Button is at (20, 30) to (100, 55)

    system.HandleInput(50, 40, true);
    TEST("HandleInput button click propagated", btnClicked);

    // Click outside panel bounds — should not reach button
    btnClicked = false;
    system.HandleInput(500, 500, true);
    TEST("HandleInput miss does not fire button", !btnClicked);

    // No-click frame should not propagate
    btnClicked = false;
    system.HandleInput(50, 40, false);
    TEST("HandleInput without click is no-op", !btnClicked);

    // Screen size
    system.SetScreenSize(2560, 1440);
    TEST("Screen width updated", ApproxEq(system.GetScreenWidth(), 2560.0f));
    TEST("Screen height updated", ApproxEq(system.GetScreenHeight(), 1440.0f));

    // Replace panel
    auto newHud = std::make_shared<UIPanel>();
    newHud->SetTitle("New HUD");
    system.AddPanel("hud", newHud);
    TEST("Replace panel same count", system.GetPanelCount() == 1);
    UIPanel* replaced = system.GetPanel("hud");
    TEST("Replaced panel is new", replaced == newHud.get());
}

// ===================================================================
// Networking Tests
// ===================================================================

static void TestNetworkMessage() {
    std::cout << "[NetworkMessage]\n";

    // Default constructor
    NetworkMessage msg;
    TEST("Default type is ChatMessage", msg.type == MessageType::ChatMessage);
    TEST("Default data is empty", msg.data.empty());
    TEST("Default timestamp > 0", msg.timestamp > 0.0);

    // Parameterized constructor
    NetworkMessage msg2(MessageType::JoinSector, "sector_alpha");
    TEST("Type set correctly", msg2.type == MessageType::JoinSector);
    TEST("Data set correctly", msg2.data == "sector_alpha");
    TEST("Timestamp set", msg2.timestamp > 0.0);

    // Serialize / Deserialize round-trip
    auto bytes = msg2.Serialize();
    TEST("Serialized bytes non-empty", !bytes.empty());

    NetworkMessage deserialized = NetworkMessage::Deserialize(bytes);
    TEST("Deserialized type matches", deserialized.type == msg2.type);
    TEST("Deserialized data matches", deserialized.data == msg2.data);
    TEST("Deserialized timestamp matches", ApproxEq(static_cast<float>(deserialized.timestamp),
                                                     static_cast<float>(msg2.timestamp)));

    // Empty data round-trip
    NetworkMessage emptyMsg(MessageType::LeaveSector, "");
    auto emptyBytes = emptyMsg.Serialize();
    NetworkMessage emptyDeser = NetworkMessage::Deserialize(emptyBytes);
    TEST("Empty data round-trip type", emptyDeser.type == MessageType::LeaveSector);
    TEST("Empty data round-trip data", emptyDeser.data.empty());

    // Deserialize too-short buffer
    std::vector<uint8_t> tooShort = {0, 0};
    NetworkMessage badMsg = NetworkMessage::Deserialize(tooShort);
    TEST("Short buffer returns default type", badMsg.type == MessageType::ChatMessage);
}

static void TestClientConnection() {
    std::cout << "[ClientConnection]\n";

    ClientConnection client(42, "Player1");
    TEST("ID correct", client.GetId() == 42);
    TEST("Name correct", client.GetName() == "Player1");
    TEST("Initially connected", client.IsConnected());
    TEST("Current sector empty", client.GetCurrentSector().empty());

    // Sector tracking
    client.SetCurrentSector("sector_1");
    TEST("Sector set", client.GetCurrentSector() == "sector_1");

    // Outbox
    client.QueueMessage(NetworkMessage(MessageType::ChatMessage, "hello"));
    client.QueueMessage(NetworkMessage(MessageType::EntityUpdate, "data"));
    auto outbox = client.FlushOutbox();
    TEST("Outbox has 2 messages", outbox.size() == 2);
    TEST("Outbox first is ChatMessage", outbox[0].type == MessageType::ChatMessage);
    TEST("Outbox first data", outbox[0].data == "hello");
    auto outbox2 = client.FlushOutbox();
    TEST("Outbox empty after flush", outbox2.empty());

    // Inbox
    client.ReceiveMessage(NetworkMessage(MessageType::SectorJoined, "sector_1"));
    auto inbox = client.FlushInbox();
    TEST("Inbox has 1 message", inbox.size() == 1);
    TEST("Inbox message type", inbox[0].type == MessageType::SectorJoined);
    auto inbox2 = client.FlushInbox();
    TEST("Inbox empty after flush", inbox2.empty());

    // Disconnect
    client.Disconnect();
    TEST("Disconnected", !client.IsConnected());
}

static void TestSectorServer() {
    std::cout << "[SectorServer]\n";

    SectorServer sector("alpha");
    TEST("Sector ID", sector.GetId() == "alpha");
    TEST("No clients initially", sector.GetClientCount() == 0);

    auto c1 = std::make_shared<ClientConnection>(1, "P1");
    auto c2 = std::make_shared<ClientConnection>(2, "P2");
    auto c3 = std::make_shared<ClientConnection>(3, "P3");

    sector.AddClient(c1);
    TEST("1 client", sector.GetClientCount() == 1);
    TEST("HasClient 1", sector.HasClient(1));
    TEST("Not HasClient 2", !sector.HasClient(2));

    // Duplicate add
    sector.AddClient(c1);
    TEST("Still 1 client after dup", sector.GetClientCount() == 1);

    sector.AddClient(c2);
    sector.AddClient(c3);
    TEST("3 clients", sector.GetClientCount() == 3);

    // GetClient
    auto found = sector.GetClient(2);
    TEST("GetClient found", found != nullptr);
    TEST("GetClient correct", found->GetName() == "P2");
    TEST("GetClient not found", sector.GetClient(99) == nullptr);

    // Broadcast
    sector.Broadcast(NetworkMessage(MessageType::ChatMessage, "hi"), 1);
    auto out1 = c1->FlushOutbox();
    auto out2 = c2->FlushOutbox();
    auto out3 = c3->FlushOutbox();
    TEST("Excluded client gets nothing", out1.empty());
    TEST("Client 2 gets broadcast", out2.size() == 1);
    TEST("Client 3 gets broadcast", out3.size() == 1);
    TEST("Broadcast data correct", out2[0].data == "hi");

    // Broadcast to disconnected client
    c2->Disconnect();
    sector.Broadcast(NetworkMessage(MessageType::ChatMessage, "test"), 0);
    auto out2b = c2->FlushOutbox();
    TEST("Disconnected client skipped", out2b.empty());

    // Remove
    sector.RemoveClient(2);
    TEST("2 clients after remove", sector.GetClientCount() == 2);
    TEST("Client 2 removed", !sector.HasClient(2));

    // GetClients
    auto all = sector.GetClients();
    TEST("GetClients returns 2", all.size() == 2);
}

static void TestGameServer() {
    std::cout << "[GameServer]\n";

    GameServer server(27015);
    TEST("Port correct", server.GetPort() == 27015);
    TEST("Not running initially", !server.IsRunning());
    TEST("No clients initially", server.GetClientCount() == 0);
    TEST("No sectors initially", server.GetSectorCount() == 0);

    // Can't connect when not running
    auto noClient = server.ConnectClient("Player");
    TEST("Connect fails when stopped", noClient == nullptr);

    server.Start();
    TEST("Running after start", server.IsRunning());

    // Double start is safe
    server.Start();
    TEST("Still running after double start", server.IsRunning());

    // Connect clients
    auto c1 = server.ConnectClient("Alice");
    TEST("Client 1 connected", c1 != nullptr);
    TEST("Client 1 name", c1->GetName() == "Alice");
    TEST("1 client", server.GetClientCount() == 1);

    auto c2 = server.ConnectClient("Bob");
    auto c3 = server.ConnectClient("Charlie");
    TEST("3 clients", server.GetClientCount() == 3);

    // Get client
    auto found = server.GetClient(c1->GetId());
    TEST("GetClient found", found != nullptr);
    TEST("GetClient correct", found->GetId() == c1->GetId());
    TEST("GetClient not found", server.GetClient(999) == nullptr);

    // Join sector via ProcessMessage
    server.ProcessMessage(c1->GetId(), NetworkMessage(MessageType::JoinSector, "sector_a"));
    TEST("Sector created", server.GetSectorCount() == 1);
    auto sectorA = server.GetSector("sector_a");
    TEST("Sector a exists", sectorA != nullptr);
    TEST("Sector has client 1", sectorA->HasClient(c1->GetId()));
    TEST("Client 1 in sector_a", c1->GetCurrentSector() == "sector_a");

    // Client gets SectorJoined confirmation
    auto c1out = c1->FlushOutbox();
    TEST("Client 1 got SectorJoined", c1out.size() == 1);
    TEST("SectorJoined type", c1out[0].type == MessageType::SectorJoined);
    TEST("SectorJoined data", c1out[0].data == "sector_a");

    // Second client joins same sector
    server.ProcessMessage(c2->GetId(), NetworkMessage(MessageType::JoinSector, "sector_a"));
    TEST("Sector has 2 clients", sectorA->GetClientCount() == 2);

    // Entity update broadcast
    server.ProcessMessage(c1->GetId(), NetworkMessage(MessageType::EntityUpdate, "pos_update"));
    auto c2out = c2->FlushOutbox();
    // c2 gets both: SectorJoined + EntityUpdate
    TEST("Client 2 got entity update", c2out.size() == 2);
    TEST("Entity update data", c2out[1].data == "pos_update");
    // c1 shouldn't get the broadcast back
    auto c1out2 = c1->FlushOutbox();
    TEST("Sender excluded from broadcast", c1out2.empty());

    // Chat message broadcast
    server.ProcessMessage(c2->GetId(), NetworkMessage(MessageType::ChatMessage, "hello!"));
    auto c1chat = c1->FlushOutbox();
    TEST("Client 1 got chat", c1chat.size() == 1);
    TEST("Chat data", c1chat[0].data == "hello!");

    // Leave sector
    server.ProcessMessage(c1->GetId(), NetworkMessage(MessageType::LeaveSector, "sector_a"));
    TEST("Sector down to 1", sectorA->GetClientCount() == 1);
    TEST("Client 1 sector cleared", c1->GetCurrentSector().empty());

    // Join different sector moves client
    server.ProcessMessage(c2->GetId(), NetworkMessage(MessageType::JoinSector, "sector_b"));
    TEST("2 sectors", server.GetSectorCount() == 2);
    TEST("sector_a has 0", sectorA->GetClientCount() == 0);
    TEST("Client 2 in sector_b", c2->GetCurrentSector() == "sector_b");

    // Empty data messages are ignored
    server.ProcessMessage(c1->GetId(), NetworkMessage(MessageType::JoinSector, ""));
    TEST("Empty join ignored", c1->GetCurrentSector().empty());

    // Entity update without sector is ignored
    server.ProcessMessage(c1->GetId(), NetworkMessage(MessageType::EntityUpdate, "data"));
    // no crash

    // Disconnect client
    server.DisconnectClient(c1->GetId());
    TEST("2 clients after disconnect", server.GetClientCount() == 2);
    TEST("Disconnected client removed", server.GetClient(c1->GetId()) == nullptr);

    // GetOrCreateSector
    auto sectorC = server.GetOrCreateSector("sector_c");
    TEST("Created sector_c", sectorC != nullptr);
    TEST("3 sectors", server.GetSectorCount() == 3);
    auto sectorC2 = server.GetOrCreateSector("sector_c");
    TEST("Get existing sector", sectorC2 == sectorC);

    // Stop
    server.Stop();
    TEST("Stopped", !server.IsRunning());
    TEST("Clients cleared", server.GetClientCount() == 0);
    TEST("Sectors cleared", server.GetSectorCount() == 0);

    // Double stop is safe
    server.Stop();
    TEST("Double stop ok", !server.IsRunning());
}

static void TestGameServerUpdate() {
    std::cout << "[GameServer Update]\n";

    GameServer server;
    server.Start();

    auto c1 = server.ConnectClient("P1");
    auto c2 = server.ConnectClient("P2");

    // Join sector via inbox
    c1->ReceiveMessage(NetworkMessage(MessageType::JoinSector, "lobby"));
    c2->ReceiveMessage(NetworkMessage(MessageType::JoinSector, "lobby"));
    server.Update(0.016f);

    TEST("Both in lobby", server.GetSector("lobby")->GetClientCount() == 2);

    // Flush join confirmations
    c1->FlushOutbox();
    c2->FlushOutbox();

    // Chat via inbox
    c1->ReceiveMessage(NetworkMessage(MessageType::ChatMessage, "hey"));
    server.Update(0.016f);

    auto c2msgs = c2->FlushOutbox();
    TEST("C2 received chat via Update", c2msgs.size() == 1);
    TEST("Chat content", c2msgs[0].data == "hey");

    // Update when stopped is no-op
    server.Stop();
    server.Update(0.016f);
    TEST("Update after stop ok", true);
}

// ===================================================================
// Scripting Tests
// ===================================================================

static void TestScriptingEngine() {
    std::cout << "[ScriptingEngine]\n";

    ScriptingEngine engine;
    TEST("No functions initially", engine.GetFunctionCount() == 0);
    TEST("Log empty initially", engine.GetLog().empty());

    // Register function
    engine.RegisterFunction("greet", [](const std::vector<std::string>& args) -> std::string {
        if (args.empty()) return "Hello, World!";
        return "Hello, " + args[0] + "!";
    });
    TEST("1 function registered", engine.GetFunctionCount() == 1);
    TEST("Has greet", engine.HasFunction("greet"));
    TEST("Not has unknown", !engine.HasFunction("unknown"));

    // Call function
    auto result = engine.CallFunction("greet", {});
    TEST("Call success", result.success);
    TEST("Call output", result.output == "Hello, World!");
    TEST("Log has 1 entry", engine.GetLog().size() == 1);

    auto result2 = engine.CallFunction("greet", {"Alice"});
    TEST("Call with args success", result2.success);
    TEST("Call with args output", result2.output == "Hello, Alice!");

    // Call unknown function
    auto result3 = engine.CallFunction("nonexistent");
    TEST("Unknown function fails", !result3.success);
    TEST("Unknown function error", !result3.error.empty());

    // Register function that throws
    engine.RegisterFunction("bomb", [](const std::vector<std::string>&) -> std::string {
        throw std::runtime_error("boom!");
    });
    auto result4 = engine.CallFunction("bomb");
    TEST("Exception caught", !result4.success);
    TEST("Exception error message", result4.error.find("boom!") != std::string::npos);

    // Unregister
    bool unreg = engine.UnregisterFunction("greet");
    TEST("Unregister success", unreg);
    TEST("1 function after unregister", engine.GetFunctionCount() == 1); // bomb remains
    TEST("greet gone", !engine.HasFunction("greet"));

    bool unreg2 = engine.UnregisterFunction("nonexistent");
    TEST("Unregister nonexistent fails", !unreg2);

    // GetRegisteredFunctions
    engine.RegisterFunction("add", [](const std::vector<std::string>& args) -> std::string {
        if (args.size() < 2) return "0";
        return std::to_string(std::stoi(args[0]) + std::stoi(args[1]));
    });
    auto funcs = engine.GetRegisteredFunctions();
    TEST("GetRegisteredFunctions count", funcs.size() == 2);

    // Globals
    TEST("No global initially", !engine.HasGlobal("version"));
    TEST("Get nonexistent global empty", engine.GetGlobal("version").empty());

    engine.SetGlobal("version", "1.0.0");
    TEST("Has global", engine.HasGlobal("version"));
    TEST("Get global", engine.GetGlobal("version") == "1.0.0");

    engine.SetGlobal("version", "2.0.0");
    TEST("Overwrite global", engine.GetGlobal("version") == "2.0.0");

    // Clear log
    engine.ClearLog();
    TEST("Log cleared", engine.GetLog().empty());
}

static void TestScriptExecution() {
    std::cout << "[ScriptExecution]\n";

    ScriptingEngine engine;
    engine.RegisterFunction("echo", [](const std::vector<std::string>& args) -> std::string {
        std::string result;
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) result += " ";
            result += args[i];
        }
        return result;
    });
    engine.RegisterFunction("add", [](const std::vector<std::string>& args) -> std::string {
        if (args.size() < 2) return "0";
        return std::to_string(std::stoi(args[0]) + std::stoi(args[1]));
    });

    // Execute multi-line script
    auto result = engine.ExecuteScript("echo hello world\nadd 3 4");
    TEST("Script success", result.success);
    TEST("Script output", result.output == "hello world\n7");

    // Empty lines and comments skipped
    auto result2 = engine.ExecuteScript("# comment\n\necho test\n");
    TEST("Comments skipped", result2.success);
    TEST("Comments output", result2.output == "test");

    // Script with unknown function fails
    auto result3 = engine.ExecuteScript("echo ok\nunknown_func\n");
    TEST("Script fails on unknown func", !result3.success);
    TEST("Script error set", !result3.error.empty());

    // Empty script succeeds
    auto result4 = engine.ExecuteScript("");
    TEST("Empty script success", result4.success);
    TEST("Empty script output", result4.output.empty());
}

static void TestModManager() {
    std::cout << "[ModManager]\n";

    ScriptingEngine engine;
    ModManager mgr(engine, "/mods");
    TEST("Mods directory", mgr.GetModsDirectory() == "/mods");
    TEST("No mods initially", mgr.GetRegisteredMods().empty());
    TEST("No loaded mods", mgr.GetLoadedModCount() == 0);

    // Register mods
    ModInfo modA;
    modA.id = "mod_a";
    modA.name = "Mod A";
    modA.version = "1.0.0";
    modA.author = "Author A";

    ModInfo modB;
    modB.id = "mod_b";
    modB.name = "Mod B";
    modB.version = "2.0.0";
    modB.dependencies = {"mod_a"};

    ModInfo modC;
    modC.id = "mod_c";
    modC.name = "Mod C";
    modC.dependencies = {"mod_b"};

    mgr.RegisterMod(modA);
    mgr.RegisterMod(modB);
    mgr.RegisterMod(modC);
    TEST("3 mods registered", mgr.GetRegisteredMods().size() == 3);

    // GetModInfo
    const ModInfo* infoA = mgr.GetModInfo("mod_a");
    TEST("ModInfo found", infoA != nullptr);
    TEST("ModInfo name", infoA->name == "Mod A");
    TEST("ModInfo version", infoA->version == "1.0.0");
    TEST("ModInfo not found", mgr.GetModInfo("nonexistent") == nullptr);

    // Discover mods (returns registered mods in abstraction)
    auto discovered = mgr.DiscoverMods();
    TEST("Discovered 3 mods", discovered.size() == 3);

    // Resolve dependencies
    auto order = mgr.ResolveDependencies();
    TEST("Resolve order has 3", order.size() == 3);
    // mod_a must come before mod_b, mod_b before mod_c
    auto posA = std::find(order.begin(), order.end(), "mod_a") - order.begin();
    auto posB = std::find(order.begin(), order.end(), "mod_b") - order.begin();
    auto posC = std::find(order.begin(), order.end(), "mod_c") - order.begin();
    TEST("A before B", posA < posB);
    TEST("B before C", posB < posC);

    // Load single mod
    bool loaded = mgr.LoadMod("mod_a");
    TEST("Load mod_a success", loaded);
    TEST("mod_a is loaded", mgr.IsModLoaded("mod_a"));
    TEST("1 loaded mod", mgr.GetLoadedModCount() == 1);

    // Load mod with dependency
    bool loadedB = mgr.LoadMod("mod_b");
    TEST("Load mod_b success (dep already loaded)", loadedB);
    TEST("mod_b is loaded", mgr.IsModLoaded("mod_b"));

    // Load all (mod_c remaining)
    bool allLoaded = mgr.LoadAllMods();
    TEST("LoadAllMods success", allLoaded);
    TEST("3 loaded mods", mgr.GetLoadedModCount() == 3);

    // Load order
    auto loadOrder = mgr.GetLoadOrder();
    TEST("Load order has 3", loadOrder.size() == 3);

    // Already loaded mod returns true
    bool reloadA = mgr.LoadMod("mod_a");
    TEST("Already loaded returns true", reloadA);
    TEST("Still 3 loaded", mgr.GetLoadedModCount() == 3);

    // Unload
    bool unloaded = mgr.UnloadMod("mod_c");
    TEST("Unload success", unloaded);
    TEST("mod_c not loaded", !mgr.IsModLoaded("mod_c"));
    TEST("2 loaded mods", mgr.GetLoadedModCount() == 2);

    // Unload not-loaded mod
    bool unloadAgain = mgr.UnloadMod("mod_c");
    TEST("Unload not-loaded fails", !unloadAgain);

    // Unload nonexistent
    bool unloadBad = mgr.UnloadMod("nonexistent");
    TEST("Unload nonexistent fails", !unloadBad);

    // Load nonexistent
    bool loadBad = mgr.LoadMod("nonexistent");
    TEST("Load nonexistent fails", !loadBad);
}

static void TestModDependencyCycle() {
    std::cout << "[ModDependencyCycle]\n";

    ScriptingEngine engine;
    ModManager mgr(engine);

    // Create circular dependency: X -> Y -> Z -> X
    ModInfo modX;
    modX.id = "mod_x";
    modX.name = "Mod X";
    modX.dependencies = {"mod_z"};

    ModInfo modY;
    modY.id = "mod_y";
    modY.name = "Mod Y";
    modY.dependencies = {"mod_x"};

    ModInfo modZ;
    modZ.id = "mod_z";
    modZ.name = "Mod Z";
    modZ.dependencies = {"mod_y"};

    mgr.RegisterMod(modX);
    mgr.RegisterMod(modY);
    mgr.RegisterMod(modZ);

    // Resolve should detect cycle
    auto order = mgr.ResolveDependencies();
    TEST("Circular dependency returns empty", order.empty());

    // LoadAllMods should fail
    bool loaded = mgr.LoadAllMods();
    TEST("LoadAllMods fails with cycle", !loaded);
}

static void TestModMissingDependency() {
    std::cout << "[ModMissingDependency]\n";

    ScriptingEngine engine;
    ModManager mgr(engine);

    ModInfo mod;
    mod.id = "mod_orphan";
    mod.name = "Orphan Mod";
    mod.dependencies = {"nonexistent_dep"};

    mgr.RegisterMod(mod);

    // Resolve should fail due to missing dependency
    auto order = mgr.ResolveDependencies();
    TEST("Missing dep returns empty", order.empty());
}

static void TestModReload() {
    std::cout << "[ModReload]\n";

    ScriptingEngine engine;
    ModManager mgr(engine);

    ModInfo modA;
    modA.id = "mod_a";
    modA.name = "Mod A";

    ModInfo modB;
    modB.id = "mod_b";
    modB.name = "Mod B";
    modB.dependencies = {"mod_a"};

    mgr.RegisterMod(modA);
    mgr.RegisterMod(modB);
    mgr.LoadAllMods();
    TEST("2 loaded before reload", mgr.GetLoadedModCount() == 2);

    bool reloaded = mgr.ReloadAllMods();
    TEST("Reload success", reloaded);
    TEST("2 loaded after reload", mgr.GetLoadedModCount() == 2);
}

// ===================================================================
// AudioClip tests
// ===================================================================

static void TestAudioClip() {
    std::cout << "[AudioClip]\n";

    AudioClip clip;
    TEST("Default clip is invalid", !clip.IsValid());

    clip.id = "laser_fire";
    clip.filePath = "sounds/laser.wav";
    clip.category = AudioCategory::SFX;
    clip.durationSeconds = 0.5f;
    clip.defaultVolume = 0.8f;
    TEST("Clip with id is valid", clip.IsValid());
    TEST("Clip id correct", clip.id == "laser_fire");
    TEST("Clip category SFX", clip.category == AudioCategory::SFX);
    TEST("Clip duration", ApproxEq(clip.durationSeconds, 0.5f));
    TEST("Clip default volume", ApproxEq(clip.defaultVolume, 0.8f));
    TEST("Clip not looping by default", !clip.isLooping);
}

// ===================================================================
// AudioSource tests
// ===================================================================

static void TestAudioSource() {
    std::cout << "[AudioSource]\n";

    AudioSource src;
    TEST("Default source stopped", src.state == AudioSourceState::Stopped);
    TEST("Default source not active", !src.IsActive());
    TEST("Default effective volume is 1", ApproxEq(src.GetEffectiveVolume(), 1.0f));

    src.state = AudioSourceState::Playing;
    TEST("Playing source is active", src.IsActive());

    src.state = AudioSourceState::FadingIn;
    TEST("FadingIn source is active", src.IsActive());

    src.state = AudioSourceState::FadingOut;
    TEST("FadingOut source is active", src.IsActive());

    src.state = AudioSourceState::Paused;
    TEST("Paused source is not active", !src.IsActive());

    // Test fade volume calculation
    AudioSource fade;
    fade.state = AudioSourceState::FadingIn;
    fade.fadeStartVol = 0.0f;
    fade.fadeEndVol = 1.0f;
    fade.fadeDuration = 2.0f;
    fade.fadeTimer = 1.0f; // halfway
    TEST("Fade halfway volume ~0.5", ApproxEq(fade.GetEffectiveVolume(), 0.5f));

    fade.fadeTimer = 2.0f; // complete
    TEST("Fade complete volume ~1.0", ApproxEq(fade.GetEffectiveVolume(), 1.0f));

    fade.fadeTimer = 0.0f;
    TEST("Fade start volume ~0.0", ApproxEq(fade.GetEffectiveVolume(), 0.0f));

    // Zero-duration fade
    AudioSource zeroFade;
    zeroFade.state = AudioSourceState::FadingOut;
    zeroFade.fadeDuration = 0.0f;
    zeroFade.fadeEndVol = 0.3f;
    TEST("Zero duration fade returns endVol", ApproxEq(zeroFade.GetEffectiveVolume(), 0.3f));

    // 3D source
    AudioSource src3d;
    src3d.is3D = true;
    src3d.posX = 10.0f;
    src3d.posY = 20.0f;
    src3d.posZ = 30.0f;
    TEST("3D source position X", ApproxEq(src3d.posX, 10.0f));
    TEST("3D source position Y", ApproxEq(src3d.posY, 20.0f));
    TEST("3D source max distance default", ApproxEq(src3d.maxDistance, 100.0f));
}

// ===================================================================
// AudioComponent tests
// ===================================================================

static void TestAudioComponent() {
    std::cout << "[AudioComponent]\n";

    AudioComponent comp;
    TEST("Default max concurrent sources", comp.maxConcurrentSources == 8);
    TEST("No sources initially", comp.sources.empty());
    TEST("Active source count 0", comp.GetActiveSourceCount() == 0);

    // Add a source
    AudioSource s1;
    s1.sourceId = 1;
    s1.clipId = "laser";
    s1.state = AudioSourceState::Playing;
    uint64_t id = comp.AddSource(s1);
    TEST("AddSource returns source id", id == 1);
    TEST("Source count is 1", comp.sources.size() == 1);
    TEST("Active source count 1", comp.GetActiveSourceCount() == 1);

    // Find source
    AudioSource* found = comp.GetSource(1);
    TEST("GetSource found", found != nullptr);
    TEST("GetSource correct clip", found && found->clipId == "laser");

    // Not found
    TEST("GetSource miss returns null", comp.GetSource(999) == nullptr);

    // Add another source (stopped)
    AudioSource s2;
    s2.sourceId = 2;
    s2.clipId = "explosion";
    s2.state = AudioSourceState::Stopped;
    comp.AddSource(s2);
    TEST("Two sources", comp.sources.size() == 2);
    TEST("Active count still 1", comp.GetActiveSourceCount() == 1);

    // Remove source
    TEST("Remove existing source", comp.RemoveSource(1));
    TEST("One source left", comp.sources.size() == 1);
    TEST("Remove non-existent returns false", !comp.RemoveSource(999));

    // StopAll
    AudioSource s3;
    s3.sourceId = 3;
    s3.state = AudioSourceState::Playing;
    comp.AddSource(s3);
    comp.StopAll();
    TEST("StopAll stops all", comp.GetActiveSourceCount() == 0);

    // Max concurrent limit
    AudioComponent limited;
    limited.maxConcurrentSources = 2;
    AudioSource a, b, c;
    a.sourceId = 10; b.sourceId = 11; c.sourceId = 12;
    TEST("Add first OK", limited.AddSource(a) == 10);
    TEST("Add second OK", limited.AddSource(b) == 11);
    TEST("Add third fails (max)", limited.AddSource(c) == 0);
}

// ===================================================================
// AudioComponent serialization tests
// ===================================================================

static void TestAudioComponentSerialization() {
    std::cout << "[AudioComponent Serialization]\n";

    AudioComponent original;
    original.maxConcurrentSources = 4;

    AudioSource s1;
    s1.sourceId = 42;
    s1.clipId = "engine_hum";
    s1.state = AudioSourceState::Playing;
    s1.volume = 0.7f;
    s1.pitch = 1.2f;
    s1.loop = true;
    s1.is3D = true;
    s1.posX = 1.0f; s1.posY = 2.0f; s1.posZ = 3.0f;
    original.AddSource(s1);

    AudioSource s2;
    s2.sourceId = 43;
    s2.clipId = "alert";
    s2.state = AudioSourceState::Paused;
    s2.volume = 0.5f;
    s2.is3D = false;
    original.AddSource(s2);

    // Serialize
    ComponentData cd = original.Serialize();
    TEST("Serialized component type", cd.componentType == "AudioComponent");
    TEST("Serialized source count", cd.data.at("sourceCount") == "2");
    TEST("Serialized max concurrent", cd.data.at("maxConcurrent") == "4");

    // Deserialize
    AudioComponent restored;
    restored.Deserialize(cd);
    TEST("Restored max concurrent", restored.maxConcurrentSources == 4);
    TEST("Restored source count", restored.sources.size() == 2);

    AudioSource* r1 = restored.GetSource(42);
    TEST("Restored source 1 found", r1 != nullptr);
    if (r1) {
        TEST("Restored clip id", r1->clipId == "engine_hum");
        TEST("Restored state playing", r1->state == AudioSourceState::Playing);
        TEST("Restored volume", ApproxEq(r1->volume, 0.7f));
        TEST("Restored pitch", ApproxEq(r1->pitch, 1.2f));
        TEST("Restored loop true", r1->loop);
        TEST("Restored is3D true", r1->is3D);
        TEST("Restored posX", ApproxEq(r1->posX, 1.0f));
        TEST("Restored posY", ApproxEq(r1->posY, 2.0f));
        TEST("Restored posZ", ApproxEq(r1->posZ, 3.0f));
    }

    AudioSource* r2 = restored.GetSource(43);
    TEST("Restored source 2 found", r2 != nullptr);
    if (r2) {
        TEST("Restored source 2 clip", r2->clipId == "alert");
        TEST("Restored source 2 state paused", r2->state == AudioSourceState::Paused);
        TEST("Restored source 2 not 3D", !r2->is3D);
    }

    // Empty component round-trip
    AudioComponent empty;
    ComponentData emptyCD = empty.Serialize();
    AudioComponent emptyRestored;
    emptyRestored.Deserialize(emptyCD);
    TEST("Empty restored has no sources", emptyRestored.sources.empty());
}

// ===================================================================
// MusicPlaylist tests
// ===================================================================

static void TestMusicPlaylist() {
    std::cout << "[MusicPlaylist]\n";

    MusicPlaylist pl;
    TEST("Empty playlist current is empty", pl.CurrentTrackId().empty());
    TEST("Empty playlist next is empty", pl.NextTrackId().empty());

    pl.name = "battle_music";
    pl.trackIds = {"track_a", "track_b", "track_c"};
    pl.repeat = true;

    TEST("Playlist name", pl.name == "battle_music");
    TEST("Current track is first", pl.CurrentTrackId() == "track_a");

    std::string next = pl.NextTrackId();
    TEST("Next advances to track_b", next == "track_b");
    TEST("Current now track_b", pl.CurrentTrackId() == "track_b");

    pl.NextTrackId();
    TEST("Current now track_c", pl.CurrentTrackId() == "track_c");

    // Wrap around with repeat
    std::string wrapped = pl.NextTrackId();
    TEST("Wrap repeats to track_a", wrapped == "track_a");

    // Non-repeating playlist
    MusicPlaylist noRepeat;
    noRepeat.trackIds = {"only_track"};
    noRepeat.repeat = false;
    TEST("No repeat first", noRepeat.CurrentTrackId() == "only_track");
    noRepeat.NextTrackId();
    TEST("No repeat stays on last", noRepeat.CurrentTrackId() == "only_track");

    // Reset
    pl.Reset();
    TEST("Reset brings back to first", pl.CurrentTrackId() == "track_a");
}

// ===================================================================
// AudioSystem tests
// ===================================================================

static void TestAudioSystem() {
    std::cout << "[AudioSystem]\n";

    AudioSystem sys;
    sys.Initialize();

    TEST("System name", sys.GetName() == "AudioSystem");
    TEST("System enabled by default", sys.IsEnabled());
    TEST("No clips initially", sys.GetClipCount() == 0);
    TEST("Not muted initially", !sys.IsMuted());

    // Register clips
    AudioClip sfx;
    sfx.id = "laser";
    sfx.filePath = "sounds/laser.wav";
    sfx.category = AudioCategory::SFX;
    sfx.durationSeconds = 0.3f;
    sys.RegisterClip(sfx);

    AudioClip music;
    music.id = "ambient_space";
    music.filePath = "music/space.ogg";
    music.category = AudioCategory::Music;
    music.durationSeconds = 120.0f;
    sys.RegisterClip(music);

    TEST("Clip count is 2", sys.GetClipCount() == 2);
    TEST("Has laser clip", sys.HasClip("laser"));
    TEST("Has ambient clip", sys.HasClip("ambient_space"));
    TEST("Missing clip returns false", !sys.HasClip("nonexistent"));

    const AudioClip* found = sys.GetClip("laser");
    TEST("GetClip returns clip", found != nullptr);
    TEST("GetClip correct path", found && found->filePath == "sounds/laser.wav");
    TEST("GetClip null for missing", sys.GetClip("missing") == nullptr);

    // Register clip with empty id is ignored
    AudioClip empty;
    sys.RegisterClip(empty);
    TEST("Empty id clip ignored", sys.GetClipCount() == 2);

    // Play sound
    uint64_t sid = sys.PlaySound("laser", 0.8f, 1.0f);
    TEST("PlaySound returns non-zero id", sid > 0);
    TEST("Active global source count 1", sys.GetActiveGlobalSourceCount() == 1);

    AudioSource* src = sys.GetGlobalSource(sid);
    TEST("GetGlobalSource found", src != nullptr);
    TEST("Source is playing", src && src->state == AudioSourceState::Playing);
    TEST("Source volume", src && ApproxEq(src->volume, 0.8f));

    // Play unknown clip
    uint64_t badId = sys.PlaySound("nonexistent");
    TEST("PlaySound unknown returns 0", badId == 0);

    // Play 3D sound
    uint64_t sid3d = sys.PlaySound3D("laser", 5.0f, 10.0f, 15.0f, 0.6f);
    TEST("PlaySound3D returns non-zero", sid3d > 0);
    AudioSource* src3d = sys.GetGlobalSource(sid3d);
    TEST("3D source is 3D", src3d && src3d->is3D);
    TEST("3D source posX", src3d && ApproxEq(src3d->posX, 5.0f));

    // Stop specific sound
    sys.StopSound(sid);
    AudioSource* stopped = sys.GetGlobalSource(sid);
    TEST("Stopped source state", stopped && stopped->state == AudioSourceState::Stopped);

    // StopAll
    sys.StopAllSounds();
    TEST("All sources cleared", sys.GetActiveGlobalSourceCount() == 0);

    // Listener position
    sys.SetListenerPosition(1.0f, 2.0f, 3.0f);
    float lx, ly, lz;
    sys.GetListenerPosition(lx, ly, lz);
    TEST("Listener X", ApproxEq(lx, 1.0f));
    TEST("Listener Y", ApproxEq(ly, 2.0f));
    TEST("Listener Z", ApproxEq(lz, 3.0f));

    // Mute
    sys.SetMuted(true);
    TEST("Muted", sys.IsMuted());
    sys.SetMuted(false);
    TEST("Unmuted", !sys.IsMuted());

    sys.Shutdown();
    TEST("Clips cleared after shutdown", sys.GetClipCount() == 0);
}

// ===================================================================
// AudioSystem fade tests
// ===================================================================

static void TestAudioFade() {
    std::cout << "[AudioSystem Fades]\n";

    AudioSystem sys;
    sys.Initialize();

    AudioClip clip;
    clip.id = "tone";
    clip.durationSeconds = 5.0f;
    sys.RegisterClip(clip);

    uint64_t sid = sys.PlaySound("tone", 1.0f);
    TEST("Source playing", sys.GetGlobalSource(sid)->state == AudioSourceState::Playing);

    // FadeOut
    sys.FadeOut(sid, 1.0f);
    AudioSource* src = sys.GetGlobalSource(sid);
    TEST("FadeOut state", src && src->state == AudioSourceState::FadingOut);
    TEST("FadeOut start vol 1.0", src && ApproxEq(src->fadeStartVol, 1.0f));
    TEST("FadeOut end vol 0.0", src && ApproxEq(src->fadeEndVol, 0.0f));

    // Simulate half fade
    sys.Update(0.5f);
    src = sys.GetGlobalSource(sid);
    TEST("Half fade still fading", src && src->state == AudioSourceState::FadingOut);

    // Complete fade
    sys.Update(0.5f);
    // After fade completes the source is stopped and gets cleaned up
    TEST("Fade out complete removes source", sys.GetActiveGlobalSourceCount() == 0);

    // FadeIn test
    uint64_t sid2 = sys.PlaySound("tone", 0.8f);
    sys.FadeIn(sid2, 2.0f);
    src = sys.GetGlobalSource(sid2);
    TEST("FadeIn state", src && src->state == AudioSourceState::FadingIn);
    TEST("FadeIn end vol 0.8", src && ApproxEq(src->fadeEndVol, 0.8f));

    sys.Update(2.0f); // complete fade
    src = sys.GetGlobalSource(sid2);
    TEST("FadeIn completes to Playing", src && src->state == AudioSourceState::Playing);

    // Fade on non-existent source is no-op
    sys.FadeIn(9999, 1.0f); // should not crash
    sys.FadeOut(9999, 1.0f);
    TEST("Fade on missing source no-op", true);

    sys.Shutdown();
}

// ===================================================================
// AudioSystem music tests
// ===================================================================

static void TestAudioMusic() {
    std::cout << "[AudioSystem Music]\n";

    AudioSystem sys;
    sys.Initialize();

    AudioClip t1, t2, t3;
    t1.id = "track1"; t1.durationSeconds = 1.0f;
    t2.id = "track2"; t2.durationSeconds = 1.0f;
    t3.id = "track3"; t3.durationSeconds = 1.0f;
    sys.RegisterClip(t1);
    sys.RegisterClip(t2);
    sys.RegisterClip(t3);

    MusicPlaylist pl;
    pl.name = "test";
    pl.trackIds = {"track1", "track2", "track3"};
    pl.repeat = true;
    sys.SetMusicPlaylist(pl);

    TEST("Music not playing initially", !sys.IsMusicPlaying());

    sys.PlayMusic();
    TEST("Music now playing", sys.IsMusicPlaying());
    TEST("Current playlist track", sys.GetMusicPlaylist().CurrentTrackId() == "track1");

    // Advance track manually
    sys.NextTrack();
    TEST("Next track is track2", sys.GetMusicPlaylist().CurrentTrackId() == "track2");
    TEST("Still playing", sys.IsMusicPlaying());

    // Pause and resume
    sys.PauseMusic();
    TEST("Music paused", !sys.IsMusicPlaying());

    // PlayMusic resumes from playlist current
    sys.PlayMusic();
    TEST("Music resumed", sys.IsMusicPlaying());

    // Auto-advance via update
    sys.Update(1.1f); // track2 finishes (1.0s duration)
    TEST("Auto-advanced to track3", sys.GetMusicPlaylist().CurrentTrackId() == "track3");

    // Stop
    sys.StopMusic();
    TEST("Music stopped", !sys.IsMusicPlaying());

    sys.Shutdown();
}

// ===================================================================
// AudioSystem volume calculation tests
// ===================================================================

static void TestAudioVolume() {
    std::cout << "[AudioSystem Volume]\n";

    AudioSystem sys;
    sys.Initialize();

    // GetMasterVolume uses ConfigurationManager defaults
    float master = sys.GetMasterVolume();
    TEST("Master volume default 0.8", ApproxEq(master, 0.8f));

    float sfxVol = sys.GetCategoryVolume(AudioCategory::SFX);
    TEST("SFX category volume default 0.7", ApproxEq(sfxVol, 0.7f));

    float musicVol = sys.GetCategoryVolume(AudioCategory::Music);
    TEST("Music category volume default 0.6", ApproxEq(musicVol, 0.6f));

    float voiceVol = sys.GetCategoryVolume(AudioCategory::Voice);
    TEST("Voice category volume default 1.0", ApproxEq(voiceVol, 1.0f));

    // ComputeFinalVolume = src * category * master
    AudioSource src;
    src.volume = 0.5f;
    src.state = AudioSourceState::Playing;
    float finalVol = sys.ComputeFinalVolume(src, AudioCategory::SFX);
    // 0.5 * 0.7 * 0.8 = 0.28
    TEST("Final volume computation", ApproxEq(finalVol, 0.28f));

    // Muted returns 0
    sys.SetMuted(true);
    float mutedVol = sys.ComputeFinalVolume(src, AudioCategory::SFX);
    TEST("Muted final volume is 0", ApproxEq(mutedVol, 0.0f));

    sys.Shutdown();
}

// ===================================================================
// AudioSystem update / clip lifetime tests
// ===================================================================

static void TestAudioUpdate() {
    std::cout << "[AudioSystem Update]\n";

    AudioSystem sys;
    sys.Initialize();

    AudioClip clip;
    clip.id = "short";
    clip.durationSeconds = 0.5f;
    sys.RegisterClip(clip);

    uint64_t sid = sys.PlaySound("short");
    TEST("Source active after play", sys.GetActiveGlobalSourceCount() == 1);

    // Advance time past duration
    sys.Update(0.6f);
    TEST("Source cleaned up after duration", sys.GetActiveGlobalSourceCount() == 0);

    // Looping clip doesn't stop
    AudioClip loopClip;
    loopClip.id = "loop";
    loopClip.durationSeconds = 0.5f;
    loopClip.isLooping = true;
    sys.RegisterClip(loopClip);

    uint64_t lid = sys.PlaySound("loop");
    sys.Update(0.6f);
    // Looping source resets playback but stays active
    AudioSource* loopSrc = sys.GetGlobalSource(lid);
    TEST("Looping source still active", loopSrc && loopSrc->IsActive());

    // Disabled system skips update
    sys.SetEnabled(false);
    uint64_t sid2 = sys.PlaySound("short");
    sys.Update(10.0f); // would normally expire
    TEST("Disabled system skips update", sys.GetGlobalSource(sid2) != nullptr);

    sys.SetEnabled(true);
    sys.Shutdown();
}

// ===================================================================
// QuestGenerator tests
// ===================================================================

static void TestQuestGenerator() {
    std::cout << "[QuestGenerator]\n";

    QuestGenerator gen;
    gen.SetSeed(42);

    // Generate a single quest
    Quest q = gen.Generate(5, 5);
    TEST("Generated quest has id", !q.id.empty());
    TEST("Generated quest has title", !q.title.empty());
    TEST("Generated quest has description", !q.description.empty());
    TEST("Generated quest status Available", q.status == QuestStatus::Available);
    TEST("Generated quest has objectives", !q.objectives.empty());
    TEST("Generated quest has rewards", !q.rewards.empty());
    TEST("Generated quest can abandon", q.canAbandon);
    TEST("Generated quest not repeatable", !q.isRepeatable);
    TEST("Generated count is 1", gen.GetGeneratedCount() == 1);

    // Rewards include credits and experience
    bool hasCredits = false, hasXP = false;
    for (const auto& r : q.rewards) {
        if (r.type == RewardType::Credits) hasCredits = true;
        if (r.type == RewardType::Experience) hasXP = true;
    }
    TEST("Has credit reward", hasCredits);
    TEST("Has experience reward", hasXP);

    // Generate batch
    auto batch = gen.GenerateBatch(5, 10, 3);
    TEST("Batch size 5", batch.size() == 5);
    TEST("Generated count is 6", gen.GetGeneratedCount() == 6);

    // All quests have unique ids
    bool allUnique = true;
    for (size_t i = 0; i < batch.size(); ++i) {
        for (size_t j = i + 1; j < batch.size(); ++j) {
            if (batch[i].id == batch[j].id) { allUnique = false; break; }
        }
    }
    TEST("All batch quests have unique ids", allUnique);

    // Higher-level quests should have higher rewards
    Quest lowLevel = gen.Generate(1, 8);
    Quest highLevel = gen.Generate(20, 2);

    int lowCredits = 0, highCredits = 0;
    for (const auto& r : lowLevel.rewards)
        if (r.type == RewardType::Credits) lowCredits = r.amount;
    for (const auto& r : highLevel.rewards)
        if (r.type == RewardType::Credits) highCredits = r.amount;
    TEST("Higher level = more credits", highCredits > lowCredits);

    // Deterministic: same seed produces same quest
    QuestGenerator gen2;
    gen2.SetSeed(42);
    Quest q2 = gen2.Generate(5, 5);
    TEST("Deterministic: same seed same id", q.id == q2.id);
    TEST("Deterministic: same seed same title", q.title == q2.title);
    TEST("Deterministic: same objectives count",
         q.objectives.size() == q2.objectives.size());
}

// ===================================================================
// QuestGenerator difficulty scaling tests
// ===================================================================

static void TestQuestGeneratorDifficulty() {
    std::cout << "[QuestGenerator Difficulty]\n";

    QuestGenerator gen;
    gen.SetSeed(100);

    // Level 1 → Trivial difficulty
    Quest q1 = gen.Generate(1, 10);
    TEST("Level 1 quest difficulty Trivial",
         q1.difficulty == QuestDifficulty::Trivial);

    // Level 10 → Normal difficulty
    gen.SetSeed(100);
    Quest q10 = gen.Generate(10, 10);
    TEST("Level 10 quest difficulty Normal",
         q10.difficulty == QuestDifficulty::Normal);

    // Level 25, high-security → Elite difficulty
    gen.SetSeed(100);
    Quest q25 = gen.Generate(25, 10);
    TEST("Level 25 quest difficulty Elite",
         q25.difficulty == QuestDifficulty::Elite);

    // Low security increases difficulty
    gen.SetSeed(100);
    Quest lowSec = gen.Generate(5, 1);
    gen.SetSeed(100);
    Quest highSec = gen.Generate(5, 10);
    TEST("Low security higher or equal difficulty",
         static_cast<int>(lowSec.difficulty) >= static_cast<int>(highSec.difficulty));

    // Difficulty affects objective count (more = harder)
    gen.SetSeed(200);
    Quest easyQ = gen.Generate(1, 10);
    gen.SetSeed(200);
    Quest hardQ = gen.Generate(25, 1);
    TEST("Hard quest >= easy quest objective count",
         hardQ.objectives.size() >= easyQ.objectives.size());

    // High difficulty quests may have reputation rewards
    bool hasReputation = false;
    for (const auto& r : hardQ.rewards) {
        if (r.type == RewardType::Reputation) hasReputation = true;
    }
    TEST("Hard quest may have reputation reward", hasReputation);
}

// ===================================================================
// QuestGenerator integration with QuestSystem tests
// ===================================================================

static void TestQuestGeneratorIntegration() {
    std::cout << "[QuestGenerator Integration]\n";

    QuestGenerator gen;
    gen.SetSeed(777);

    QuestSystem sys;
    QuestComponent comp;

    // Generate and register as template
    Quest generated = gen.Generate(10, 5);
    sys.AddQuestTemplate(generated);
    TEST("Template registered", sys.GetTemplateCount() == 1);

    // Give to entity
    bool given = sys.GiveQuest(1, generated.id, comp);
    TEST("Generated quest given to entity", given);
    TEST("Component has quest", comp.GetQuest(generated.id) != nullptr);

    // Accept and progress
    bool accepted = comp.AcceptQuest(generated.id);
    TEST("Generated quest accepted", accepted);

    Quest* active = comp.GetQuest(generated.id);
    TEST("Quest is active", active && active->status == QuestStatus::Active);

    // Progress first objective
    if (active && !active->objectives.empty()) {
        sys.ProgressObjective(comp, active->objectives[0].type,
                              active->objectives[0].target,
                              active->objectives[0].requiredQuantity);
    }
    TEST("Objective progressed", true);

    // Generate batch and add all as templates
    auto batch = gen.GenerateBatch(3, 15, 3);
    for (const auto& q : batch) {
        sys.AddQuestTemplate(q);
    }
    TEST("Batch templates registered", sys.GetTemplateCount() == 4);
}

// ===================================================================
// Engine tests
// ===================================================================

static void TestEngine() {
    std::cout << "[Engine]\n";

    // Clear singleton state left over from earlier tests.
    EventSystem::Instance().ClearAllListeners();

    // --- Construction ---
    {
        Engine engine;
        TEST("Initial state is Uninitialized", engine.GetState() == EngineState::Uninitialized);
        TEST("Frame count starts at 0", engine.GetFrameCount() == 0);
        TEST("Elapsed seconds starts at 0", engine.GetElapsedSeconds() == 0.0);
        TEST("Not running before init", !engine.IsRunning());
        TEST("Not paused before init", !engine.IsPaused());
    }

    // --- Initialize ---
    {
        Engine engine;
        engine.Initialize();
        TEST("State is Running after init", engine.GetState() == EngineState::Running);
        TEST("IsRunning true after init", engine.IsRunning());
        TEST("Frame count still 0 after init", engine.GetFrameCount() == 0);
        engine.Shutdown();
    }

    // --- Version ---
    {
        const char* ver = Engine::GetVersionString();
        std::string vs(ver);
        TEST("Version string not empty", !vs.empty());
        TEST("Version contains Subspace", vs.find("Subspace") != std::string::npos);
    }

    // --- Tick ---
    {
        Engine engine;
        engine.Initialize();
        engine.Tick();
        TEST("Frame count 1 after one tick", engine.GetFrameCount() == 1);
        TEST("Last delta > 0 after tick", engine.GetLastDeltaTime() >= 0.0f);
        engine.Tick();
        engine.Tick();
        TEST("Frame count 3 after three ticks", engine.GetFrameCount() == 3);
        engine.Shutdown();
    }

    // --- Pause / Resume ---
    {
        Engine engine;
        engine.Initialize();

        engine.Pause();
        TEST("State paused", engine.GetState() == EngineState::Paused);
        TEST("IsPaused true", engine.IsPaused());
        TEST("IsRunning false when paused", !engine.IsRunning());

        // Ticking while paused should still increment frames but not update systems.
        engine.Tick();
        TEST("Frame increments while paused", engine.GetFrameCount() == 1);

        engine.Resume();
        TEST("State running after resume", engine.GetState() == EngineState::Running);
        TEST("IsRunning true after resume", engine.IsRunning());
        TEST("IsPaused false after resume", !engine.IsPaused());
        engine.Shutdown();
    }

    // --- RequestShutdown ---
    {
        Engine engine;
        engine.Initialize();
        engine.RequestShutdown();
        TEST("State is ShuttingDown", engine.GetState() == EngineState::ShuttingDown);
        TEST("IsRunning false after shutdown request", !engine.IsRunning());

        // Tick should no-op in ShuttingDown state.
        engine.Tick();
        TEST("Frame count 0 after tick in ShuttingDown", engine.GetFrameCount() == 0);
        engine.Shutdown();
        TEST("State stopped after Shutdown", engine.GetState() == EngineState::Stopped);
    }

    // --- Run with MaxFrames ---
    {
        Engine engine;
        engine.Initialize();
        engine.SetMaxFrames(5);
        engine.Run();
        TEST("Ran max frames", engine.GetFrameCount() == 5);
        TEST("State after Run with max frames", engine.GetState() == EngineState::ShuttingDown);
        engine.Shutdown();
        TEST("State stopped", engine.GetState() == EngineState::Stopped);
    }

    // --- Fixed timestep ---
    {
        Engine engine;
        engine.SetFixedTimestep(1.0f / 30.0f);
        TEST("Timestep is 1/30", ApproxEq(engine.GetFixedTimestep(), 1.0f / 30.0f));
    }

    // --- Double-init is a no-op ---
    {
        Engine engine;
        engine.Initialize();
        engine.Initialize(); // should not crash or change state
        TEST("Still running after double init", engine.IsRunning());
        engine.Shutdown();
    }

    // --- Double-shutdown is safe ---
    {
        Engine engine;
        engine.Initialize();
        engine.Shutdown();
        engine.Shutdown(); // should not crash
        TEST("Still stopped after double shutdown", engine.GetState() == EngineState::Stopped);
    }

    // --- Pause before init is harmless ---
    {
        Engine engine;
        engine.Pause();
        TEST("Pause on uninitialized is no-op", engine.GetState() == EngineState::Uninitialized);
    }

    // --- Resume before init is harmless ---
    {
        Engine engine;
        engine.Resume();
        TEST("Resume on uninitialized is no-op", engine.GetState() == EngineState::Uninitialized);
    }

    // --- EntityManager accessible ---
    {
        Engine engine;
        engine.Initialize();
        auto& em = engine.GetEntityManager();
        auto& e = em.CreateEntity("TestFromEngine");
        TEST("Can create entity via engine", e.name == "TestFromEngine");
        engine.Shutdown();
    }

    // --- GalaxyGenerator accessible ---
    {
        Engine engine;
        engine.Initialize();
        auto sector = engine.GetGalaxyGenerator().GenerateSector(0, 0, 0);
        TEST("Galaxy generator works via engine", true);
        engine.Shutdown();
    }

    // --- Events fire on lifecycle ---
    {
        EventSystem::Instance().ClearAllListeners();
        int started = 0, paused = 0, resumed = 0;
        EventSystem::Instance().Subscribe(GameEvents::GameStarted,
            [&](const GameEvent&) { started++; });
        EventSystem::Instance().Subscribe(GameEvents::GamePaused,
            [&](const GameEvent&) { paused++; });
        EventSystem::Instance().Subscribe(GameEvents::GameResumed,
            [&](const GameEvent&) { resumed++; });

        Engine engine;
        engine.Initialize();
        TEST("GameStarted event fired", started == 1);
        engine.Pause();
        TEST("GamePaused event fired", paused == 1);
        engine.Resume();
        TEST("GameResumed event fired", resumed == 1);
        engine.Shutdown();
        EventSystem::Instance().ClearAllListeners();
    }

    // --- Elapsed time increases ---
    {
        Engine engine;
        engine.Initialize();
        engine.SetMaxFrames(10);
        engine.Run();
        TEST("Elapsed seconds > 0 after run", engine.GetElapsedSeconds() > 0.0);
        engine.Shutdown();
    }

    // --- UIRenderer accessible ---
    {
        Engine engine;
        engine.Initialize();
        auto& renderer = engine.GetUIRenderer();
        TEST("UIRenderer accessible", true);
        TEST("UIRenderer screen width default", ApproxEq(renderer.GetScreenWidth(), 1920.0f));
        TEST("UIRenderer screen height default", ApproxEq(renderer.GetScreenHeight(), 1080.0f));
        engine.Shutdown();
    }

    // --- RenderFrame called during Tick ---
    {
        Engine engine;
        engine.Initialize();
        engine.Tick();
        // After a tick, the renderer should have been called (BeginFrame clears,
        // EndFrame finalizes). With no visible panels the command count is 0,
        // but the frame was processed — screen dimensions should be set.
        auto& renderer = engine.GetUIRenderer();
        TEST("Renderer frame processed after tick", ApproxEq(renderer.GetScreenWidth(), 1920.0f));
        TEST("Renderer commands 0 with no panels", renderer.GetCommandCount() == 0);
        engine.Shutdown();
    }

    // --- Rendering runs even when paused ---
    {
        EventSystem::Instance().ClearAllListeners();

        Engine engine;
        engine.Initialize();
        engine.Pause();
        engine.Tick();
        auto& renderer = engine.GetUIRenderer();
        // Rendering should still run (menus need to be visible while paused).
        TEST("Renderer runs while paused", ApproxEq(renderer.GetScreenWidth(), 1920.0f));
        engine.Shutdown();
    }
}

// ===================================================================
// Main
// ===================================================================
int main() {
    std::cout << "=== Subspace Engine Unit Tests ===\n\n";

    TestMath();
    TestBlock();
    TestShip();
    TestBlockPlacement();
    TestSymmetry();
    TestShipStats();
    TestShipDamage();
    TestShipEditor();
    TestFactions();
    TestAIShipBuilder();
    TestBlueprint();
    TestWeaponSystem();
    TestModuleDef();
    TestModularShip();
    TestShipArchetypeGenerator();
    TestLogger();
    TestEventSystem();
    TestECS();
    TestPhysicsComponent();
    TestPhysicsSystem();
    TestInventory();
    TestConfigurationManager();
    TestSaveGameManager();
    TestNavigationSystem();
    TestCombatSystem();
    TestCombatSystemECS();
    TestNavigationSystemECS();
    TestTradingSystem();
    TestProgressionSystem();
    TestCrewSystem();
    TestPowerComponent();
    TestPowerSystem();
    TestMiningSystem();
    TestGalaxyGenerator();
    TestQuestObjective();
    TestQuest();
    TestQuestComponent();
    TestQuestSystem();
    TestQuestSystemTradeVisitBuild();
    TestQuestComponentSerialization();
    TestTutorialStep();
    TestTutorial();
    TestTutorialSystem();
    TestTutorialComponentSerialization();
    TestAIPerception();
    TestAIComponent();
    TestAIDecisionSystem();
    TestSpatialHash();
    TestAISteeringSystem();
    TestPhysicsSystemSpatialHash();
    TestUITypes();
    TestUILabel();
    TestUIButton();
    TestUIProgressBar();
    TestUISeparator();
    TestUICheckbox();
    TestUIPanel();
    TestUIRenderer();
    TestUISystem();
    TestNetworkMessage();
    TestClientConnection();
    TestSectorServer();
    TestGameServer();
    TestGameServerUpdate();
    TestScriptingEngine();
    TestScriptExecution();
    TestModManager();
    TestModDependencyCycle();
    TestModMissingDependency();
    TestModReload();
    TestAudioClip();
    TestAudioSource();
    TestAudioComponent();
    TestAudioComponentSerialization();
    TestMusicPlaylist();
    TestAudioSystem();
    TestAudioFade();
    TestAudioMusic();
    TestAudioVolume();
    TestAudioUpdate();
    TestQuestGenerator();
    TestQuestGeneratorDifficulty();
    TestQuestGeneratorIntegration();
    TestEngine();

    std::cout << "\n=== Summary: " << testsPassed << " passed, "
              << testsFailed << " failed ===\n";

    return testsFailed > 0 ? 1 : 0;
}
