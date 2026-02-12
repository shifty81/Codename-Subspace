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

    std::cout << "\n=== Summary: " << testsPassed << " passed, "
              << testsFailed << " failed ===\n";

    return testsFailed > 0 ? 1 : 0;
}
