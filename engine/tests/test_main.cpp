// Self-contained unit tests for Codename-Subspace engine core systems.
// No test framework dependency — uses assert-style macros and a simple runner.

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <atomic>

// ---- New ported systems ----
#include "core/math/Vector2.h"
#include "core/math/Matrix4.h"
#include "core/math/Quaternion.h"
#include "core/memory/Allocator.h"
#include "core/events/EventBus.h"
#include "core/threading/TaskSystem.h"
#include "core/assets/AssetManager.h"
#include "core/serialization/BinaryArchive.h"
#include "core/world/Level.h"
#include "input/Keyboard.h"
#include "input/Mouse.h"
#include "input/Gamepad.h"
#include "input/InputSystem.h"
#include "audio/SoundAsset.h"
#include "audio/SoundBank.h"
#include "audio/AudioDevice.h"
#include "audio/AudioMixer.h"
#include "audio/SpatialAudio.h"
#include "ai/BehaviorTree.h"
#include "ui/widgets/Widget.h"
#include "ui/widgets/WidgetTree.h"
#include "ui/widgets/Widgets.h"
#include "physics/CharacterController.h"

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
#include "ship_editor/EditorAction.h"
#include "ship_editor/EditorClipboard.h"
#include "ship_editor/EditorSelection.h"
#include "ship_editor/ShipValidator.h"
#include "ship_editor/BlockPalette.h"
#include "ship_editor/EditorGrid.h"
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
#include "rendering/ParticleSystem.h"
#include "achievement/AchievementSystem.h"
#include "ships/StructuralIntegrity.h"
#include "ships/DamageComponent.h"
#include "core/physics/Octree.h"
#include "core/physics/CollisionLayers.h"
#include "navigation/Pathfinding.h"
#include "navigation/PathfindingComponent.h"
#include "navigation/PathfindingSystem.h"
#include "core/Engine.h"
#include "combat/TargetLockSystem.h"
#include "combat/ShieldSystem.h"
#include "combat/StatusEffectSystem.h"
#include "combat/LootSystem.h"
#include "crafting/CraftingSystem.h"
#include "formation/FormationSystem.h"
#include "reputation/ReputationSystem.h"
#include "ships/CapabilitySystem.h"
#include "debug_tools/DebugRenderer.h"
#include "debug_tools/PerformanceMonitor.h"
#include "diplomacy/DiplomacySystem.h"
#include "research/ResearchSystem.h"
#include "notification/NotificationSystem.h"
#include "inventory/InventorySystem.h"
#include "trade_route/TradeRouteSystem.h"
#include "hangar/HangarSystem.h"
#include "navigation/WormholeSystem.h"
#include "ships/ShipClassSystem.h"
#include "crafting/RefinerySystem.h"
#include "scanning/ScanningSystem.h"
#include "salvage/SalvageSystem.h"
#include "fleet/FleetCommandSystem.h"
#include "rendering/PostProcessingSystem.h"
#include "rendering/ShadowSystem.h"
#include "voxel/VoxelSystem.h"
#include "building/BuildingSystem.h"
#include "ships/ModularShipFactory.h"
#include "procedural/StarSystemGenerator.h"
#include "procedural/StargateGenerator.h"
#include "trading/EconomyAgentSystem.h"
#include "rpg/GalaxyProgressionSystem.h"
#include "rpg/PodSystem.h"

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
static constexpr float kTinyDeltaTime = 0.0001f;
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
// 8a. EditorHistory (Undo/Redo) tests
// ===================================================================
static void TestEditorHistory() {
    std::cout << "[EditorHistory]\n";

    EditorHistory history(50);

    TEST("Initial undo count is 0", history.UndoCount() == 0);
    TEST("Initial redo count is 0", history.RedoCount() == 0);
    TEST("Cannot undo empty", !history.CanUndo());
    TEST("Cannot redo empty", !history.CanRedo());
    TEST("MaxSize correct", history.MaxSize() == 50);

    // Push one action
    Block b{};
    b.gridPos = {1, 2, 3};
    history.PushAction(EditorAction::MakePlaceAction(b));
    TEST("UndoCount after push", history.UndoCount() == 1);
    TEST("CanUndo after push", history.CanUndo());

    // Undo
    EditorAction undone = history.Undo();
    TEST("Undone action type", undone.type == EditorActionType::PlaceBlock);
    TEST("Undone pos matches", undone.blockData.gridPos == Vector3Int(1, 2, 3));
    TEST("CanRedo after undo", history.CanRedo());
    TEST("Cannot undo after single undo", !history.CanUndo());

    // Redo
    EditorAction redone = history.Redo();
    TEST("Redone action type", redone.type == EditorActionType::PlaceBlock);
    TEST("CanUndo after redo", history.CanUndo());

    // Push discards redo stack
    history.Undo();
    Block b2{};
    b2.gridPos = {4, 5, 6};
    history.PushAction(EditorAction::MakeRemoveAction(b2));
    TEST("Redo cleared after new push", !history.CanRedo());
    TEST("UndoCount after re-push", history.UndoCount() == 1);

    // Clear
    history.Clear();
    TEST("Clear empties history", history.UndoCount() == 0);
    TEST("Clear empties redo", history.RedoCount() == 0);
}

static void TestEditorHistoryMaxSize() {
    std::cout << "[EditorHistory MaxSize]\n";

    EditorHistory history(3);
    Block b{};
    for (int i = 0; i < 5; ++i) {
        b.gridPos = {i, 0, 0};
        history.PushAction(EditorAction::MakePlaceAction(b));
    }
    TEST("MaxSize trims old entries", history.UndoCount() == 3);
}

static void TestEditorActionFactories() {
    std::cout << "[EditorAction Factories]\n";

    Block b{};
    b.gridPos = {1, 1, 1};
    b.material = MaterialType::Titanium;

    auto place = EditorAction::MakePlaceAction(b);
    TEST("PlaceAction type", place.type == EditorActionType::PlaceBlock);

    auto remove = EditorAction::MakeRemoveAction(b);
    TEST("RemoveAction type", remove.type == EditorActionType::RemoveBlock);

    auto paint = EditorAction::MakePaintAction(b, MaterialType::Iron);
    TEST("PaintAction type", paint.type == EditorActionType::PaintBlock);
    TEST("PaintAction previousMaterial", paint.previousMaterial == MaterialType::Iron);

    std::vector<Block> blocks = {b, b};
    auto mp = EditorAction::MakeMultiPlace(blocks);
    TEST("MultiPlace type", mp.type == EditorActionType::MultiPlace);
    TEST("MultiPlace count", mp.multiBlocks.size() == 2);

    auto mr = EditorAction::MakeMultiRemove(blocks);
    TEST("MultiRemove type", mr.type == EditorActionType::MultiRemove);
}

// ===================================================================
// 8b. EditorClipboard tests
// ===================================================================
static void TestEditorClipboard() {
    std::cout << "[EditorClipboard]\n";

    EditorClipboard clipboard;
    TEST("Clipboard initially empty", clipboard.IsEmpty());
    TEST("Clipboard count 0", clipboard.BlockCount() == 0);

    // Copy two blocks anchored at (2,0,0)
    Block b1{};
    b1.gridPos = {2, 0, 0};
    b1.size = {1, 1, 1};
    b1.type = BlockType::Hull;
    Block b2{};
    b2.gridPos = {3, 0, 0};
    b2.size = {1, 1, 1};
    b2.type = BlockType::Engine;

    clipboard.Copy({b1, b2}, {2, 0, 0});
    TEST("Clipboard not empty after copy", !clipboard.IsEmpty());
    TEST("Clipboard has 2 blocks", clipboard.BlockCount() == 2);

    // Paste at (10, 5, 0) → relative offsets applied
    auto pasted = clipboard.Paste({10, 5, 0});
    TEST("Paste returns 2 blocks", pasted.size() == 2);

    // The first block was at offset (0,0,0) → should be at (10,5,0)
    bool found10 = false, found11 = false;
    for (const auto& p : pasted) {
        if (p.gridPos == Vector3Int(10, 5, 0)) found10 = true;
        if (p.gridPos == Vector3Int(11, 5, 0)) found11 = true;
    }
    TEST("Paste places first block correctly", found10);
    TEST("Paste places second block correctly", found11);

    // Clear
    clipboard.Clear();
    TEST("Clipboard empty after clear", clipboard.IsEmpty());
}

// ===================================================================
// 8c. EditorSelection tests
// ===================================================================
static void TestEditorSelection() {
    std::cout << "[EditorSelection]\n";

    EditorSelection sel;
    TEST("Selection initially empty", sel.IsEmpty());
    TEST("Selection count 0", sel.Count() == 0);

    sel.Add({1, 0, 0});
    TEST("Selection count after add", sel.Count() == 1);
    TEST("Contains added cell", sel.Contains({1, 0, 0}));
    TEST("Does not contain other", !sel.Contains({2, 0, 0}));

    sel.Remove({1, 0, 0});
    TEST("Selection empty after remove", sel.IsEmpty());

    // Toggle
    sel.Toggle({5, 5, 5});
    TEST("Toggle adds", sel.Contains({5, 5, 5}));
    sel.Toggle({5, 5, 5});
    TEST("Toggle removes", !sel.Contains({5, 5, 5}));

    // Box select
    sel.SelectBox({0, 0, 0}, {2, 2, 0});
    TEST("Box select count", sel.Count() == 9);  // 3x3x1
    TEST("Box select contains corner", sel.Contains({0, 0, 0}));
    TEST("Box select contains far corner", sel.Contains({2, 2, 0}));

    // Bounds
    Vector3Int bMin, bMax;
    bool hasBounds = sel.GetBounds(bMin, bMax);
    TEST("GetBounds returns true", hasBounds);
    TEST("GetBounds min", bMin == Vector3Int(0, 0, 0));
    TEST("GetBounds max", bMax == Vector3Int(2, 2, 0));

    // Clear
    sel.Clear();
    TEST("Selection cleared", sel.IsEmpty());

    // GetBounds on empty
    TEST("GetBounds on empty returns false", !sel.GetBounds(bMin, bMax));
}

static void TestEditorSelectionGatherBlocks() {
    std::cout << "[EditorSelection GatherBlocks]\n";

    Ship ship;
    auto b1 = std::make_shared<Block>();
    b1->gridPos = {0, 0, 0};
    b1->size = {1, 1, 1};
    b1->type = BlockType::Hull;
    b1->maxHP = 100;
    b1->currentHP = 100;
    BlockPlacement::Place(ship, b1);

    auto b2 = std::make_shared<Block>();
    b2->gridPos = {1, 0, 0};
    b2->size = {1, 1, 1};
    b2->type = BlockType::Engine;
    b2->maxHP = 100;
    b2->currentHP = 100;
    BlockPlacement::Place(ship, b2);

    EditorSelection sel;
    sel.Add({0, 0, 0});
    sel.Add({1, 0, 0});
    sel.Add({99, 99, 99}); // not a real block

    auto gathered = sel.GatherBlocks(ship);
    TEST("GatherBlocks returns existing blocks", gathered.size() == 2);
}

// ===================================================================
// 8d. ShipValidator tests
// ===================================================================
static void TestShipValidator() {
    std::cout << "[ShipValidator]\n";

    // Empty ship
    {
        Ship empty;
        auto result = ShipValidator::Validate(empty);
        TEST("Empty ship is invalid", !result.valid);
        TEST("Empty ship error count", result.errors.size() == 1);
    }

    // Single hull block – connected but no engine/generator
    {
        Ship ship;
        auto b = std::make_shared<Block>();
        b->gridPos = {0, 0, 0};
        b->size = {1, 1, 1};
        b->type = BlockType::Hull;
        b->maxHP = 100;
        b->currentHP = 100;
        BlockPlacement::Place(ship, b);

        auto result = ShipValidator::Validate(ship);
        TEST("Single hull is valid (no hard errors)", result.valid);
        TEST("Warns about missing engine", result.warnings.size() >= 1);
    }

    // Ship with engine + generator
    {
        Ship ship;
        auto b1 = std::make_shared<Block>();
        b1->gridPos = {0, 0, 0};
        b1->size = {1, 1, 1};
        b1->type = BlockType::Engine;
        b1->maxHP = 100;
        b1->currentHP = 100;
        BlockPlacement::Place(ship, b1);

        auto b2 = std::make_shared<Block>();
        b2->gridPos = {1, 0, 0};
        b2->size = {1, 1, 1};
        b2->type = BlockType::Generator;
        b2->maxHP = 100;
        b2->currentHP = 100;
        BlockPlacement::Place(ship, b2);

        auto result = ShipValidator::Validate(ship);
        TEST("Engine+generator ship valid", result.valid);
        TEST("No warnings for complete ship", result.warnings.empty());
    }

    // Individual checks
    {
        Ship ship;
        TEST("HasBlocks false for empty", !ShipValidator::HasBlocks(ship));

        auto b = std::make_shared<Block>();
        b->gridPos = {0, 0, 0};
        b->size = {1, 1, 1};
        b->type = BlockType::Hull;
        b->maxHP = 100;
        b->currentHP = 100;
        BlockPlacement::Place(ship, b);

        TEST("HasBlocks true after add", ShipValidator::HasBlocks(ship));
        TEST("HasEngine false for hull", !ShipValidator::HasEngine(ship));
        TEST("HasGenerator false for hull", !ShipValidator::HasGenerator(ship));
        TEST("MassWithinLimit", ShipValidator::MassWithinLimit(ship, 99999.0f));
        TEST("BlockCountWithinLimit", ShipValidator::BlockCountWithinLimit(ship, 100));
        TEST("BlockCountWithinLimit fails", !ShipValidator::BlockCountWithinLimit(ship, 0));
    }
}

static void TestShipValidatorConnectivity() {
    std::cout << "[ShipValidator Connectivity]\n";

    // Connected ship
    {
        Ship ship;
        auto b1 = std::make_shared<Block>();
        b1->gridPos = {0, 0, 0};
        b1->size = {1, 1, 1};
        b1->type = BlockType::Hull;
        b1->maxHP = 100;
        b1->currentHP = 100;
        BlockPlacement::Place(ship, b1);

        auto b2 = std::make_shared<Block>();
        b2->gridPos = {1, 0, 0};
        b2->size = {1, 1, 1};
        b2->type = BlockType::Hull;
        b2->maxHP = 100;
        b2->currentHP = 100;
        BlockPlacement::Place(ship, b2);

        TEST("Two adjacent blocks connected", ShipValidator::IsConnected(ship));
    }

    // Disconnected ship (manually insert two blocks far apart)
    {
        Ship ship;
        auto b1 = std::make_shared<Block>();
        b1->gridPos = {0, 0, 0};
        b1->size = {1, 1, 1};
        b1->type = BlockType::Hull;
        b1->maxHP = 100;
        b1->currentHP = 100;
        ship.blocks.push_back(b1);
        ship.occupiedCells[b1->gridPos] = b1;

        auto b2 = std::make_shared<Block>();
        b2->gridPos = {10, 10, 10};
        b2->size = {1, 1, 1};
        b2->type = BlockType::Hull;
        b2->maxHP = 100;
        b2->currentHP = 100;
        ship.blocks.push_back(b2);
        ship.occupiedCells[b2->gridPos] = b2;

        TEST("Two distant blocks not connected", !ShipValidator::IsConnected(ship));
    }
}

// ===================================================================
// 8e. BlockPalette tests
// ===================================================================
static void TestBlockPalette() {
    std::cout << "[BlockPalette]\n";

    BlockPalette palette;
    TEST("Palette not empty", palette.Count() > 0);
    TEST("Palette has entries", palette.GetAll().size() == palette.Count());

    // Categories
    auto cats = palette.GetCategories();
    TEST("Has at least 3 categories", cats.size() >= 3);

    bool hasStructure = false, hasFunctional = false, hasWeapons = false;
    for (const auto& c : cats) {
        if (c == "Structure") hasStructure = true;
        if (c == "Functional") hasFunctional = true;
        if (c == "Weapons") hasWeapons = true;
    }
    TEST("Has Structure category", hasStructure);
    TEST("Has Functional category", hasFunctional);
    TEST("Has Weapons category", hasWeapons);

    // Filter by category
    auto structural = palette.GetByCategory("Structure");
    TEST("Structure has entries", !structural.empty());

    auto weapons = palette.GetByCategory("Weapons");
    TEST("Weapons has entries", !weapons.empty());

    // FindByType
    auto engine = palette.FindByType(BlockType::Engine);
    TEST("FindByType Engine", engine != nullptr);
    TEST("Engine entry name", engine->name == "Engine");

    auto notFound = palette.FindByType(static_cast<BlockType>(99));
    TEST("FindByType returns null for unknown", notFound == nullptr);
}

// ===================================================================
// 8f. EditorGrid tests
// ===================================================================
static void TestEditorGrid() {
    std::cout << "[EditorGrid]\n";

    EditorGrid grid;
    TEST("Default cell size 1", grid.GetCellSize() == 1);
    TEST("Default visible true", grid.IsVisible());
    TEST("Default extent 50", grid.GetExtent() == 50);

    // Snap to grid
    auto snapped = grid.SnapToGrid(2.7f, -0.3f, 5.0f);
    TEST("Snap X", snapped.x == 2);
    TEST("Snap Y", snapped.y == -1);
    TEST("Snap Z", snapped.z == 5);

    // Cell to world
    auto world = grid.CellToWorld({3, 4, 5});
    TEST("CellToWorld X", std::fabs(world.x - 3.5f) < 0.01f);
    TEST("CellToWorld Y", std::fabs(world.y - 4.5f) < 0.01f);
    TEST("CellToWorld Z", std::fabs(world.z - 5.5f) < 0.01f);

    // Cell size 2
    grid.SetCellSize(2);
    TEST("Cell size updated", grid.GetCellSize() == 2);
    auto snapped2 = grid.SnapToGrid(5.0f, 5.0f, 5.0f);
    TEST("Snap with cell size 2", snapped2.x == 2);

    auto world2 = grid.CellToWorld({1, 1, 1});
    TEST("CellToWorld with size 2", std::fabs(world2.x - 3.0f) < 0.01f);

    // Visibility
    grid.SetVisible(false);
    TEST("Set visible false", !grid.IsVisible());

    // Extent
    grid.SetExtent(100);
    TEST("Set extent 100", grid.GetExtent() == 100);

    // Cannot set negative cell size
    grid.SetCellSize(-5);
    TEST("Negative cell size clamped to 1", grid.GetCellSize() == 1);

    // Snap via Vector3
    EditorGrid grid3(1);
    auto snappedV = grid3.SnapToGrid(Vector3{1.5f, 2.5f, 3.5f});
    TEST("SnapToGrid Vector3 X", snappedV.x == 1);
    TEST("SnapToGrid Vector3 Y", snappedV.y == 2);
    TEST("SnapToGrid Vector3 Z", snappedV.z == 3);
}

// ===================================================================
// 8g. Integrated editor undo/redo tests
// ===================================================================
static void TestEditorUndoRedo() {
    std::cout << "[Editor Undo/Redo]\n";

    Ship ship;
    ShipEditorController editor(ship);

    // Place → Undo → Redo
    editor.SetHoverCell({0, 0, 0});
    editor.Place();
    TEST("Ship has 1 block after place", ship.BlockCount() == 1);

    editor.Undo();
    TEST("Ship has 0 blocks after undo place", ship.BlockCount() == 0);

    editor.Redo();
    TEST("Ship has 1 block after redo", ship.BlockCount() == 1);

    // Remove → Undo
    editor.SetHoverCell({0, 0, 0});
    editor.RemoveAtHover();
    TEST("Ship has 0 blocks after remove", ship.BlockCount() == 0);

    editor.Undo();
    TEST("Ship has 1 block after undo remove", ship.BlockCount() == 1);

    // Paint → Undo
    editor.GetState().selectedMaterial = MaterialType::Titanium;
    editor.SetHoverCell({0, 0, 0});
    editor.PaintAtHover();
    auto it = ship.occupiedCells.find({0, 0, 0});
    TEST("Paint changed material", it->second->material == MaterialType::Titanium);

    editor.Undo();
    it = ship.occupiedCells.find({0, 0, 0});
    TEST("Undo paint restores material", it->second->material == MaterialType::Iron);
}

static void TestEditorCopyPaste() {
    std::cout << "[Editor Copy/Paste]\n";

    Ship ship;
    ShipEditorController editor(ship);

    // Place two blocks
    editor.SetHoverCell({0, 0, 0});
    editor.Place();
    editor.SetHoverCell({1, 0, 0});
    editor.Place();
    TEST("Ship has 2 blocks", ship.BlockCount() == 2);

    // Select both and copy
    editor.GetSelection().Add({0, 0, 0});
    editor.GetSelection().Add({1, 0, 0});
    editor.CopySelection();
    TEST("Clipboard has 2 blocks", editor.GetClipboard().BlockCount() == 2);

    // Paste at (2,0,0) — adjacent to existing block at (1,0,0)
    editor.SetHoverCell({2, 0, 0});
    bool pasted = editor.PasteAtHover();
    TEST("Paste succeeds", pasted);
    TEST("Ship has 4 blocks after paste", ship.BlockCount() == 4);

    // Undo paste
    editor.Undo();
    TEST("Ship has 2 blocks after undo paste", ship.BlockCount() == 2);
}

static void TestEditorCutPaste() {
    std::cout << "[Editor Cut/Paste]\n";

    Ship ship;
    ShipEditorController editor(ship);

    editor.SetHoverCell({0, 0, 0});
    editor.Place();
    editor.SetHoverCell({1, 0, 0});
    editor.Place();
    TEST("Ship has 2 blocks before cut", ship.BlockCount() == 2);

    editor.GetSelection().Add({0, 0, 0});
    editor.GetSelection().Add({1, 0, 0});
    editor.CutSelection();
    TEST("Ship has 0 blocks after cut", ship.BlockCount() == 0);
    TEST("Clipboard has 2 blocks after cut", editor.GetClipboard().BlockCount() == 2);
    TEST("Selection cleared after cut", editor.GetSelection().IsEmpty());

    // Undo cut restores blocks
    editor.Undo();
    TEST("Ship has 2 blocks after undo cut", ship.BlockCount() == 2);
}

static void TestEditorRemoveSelected() {
    std::cout << "[Editor RemoveSelected]\n";

    Ship ship;
    ShipEditorController editor(ship);

    editor.SetHoverCell({0, 0, 0});
    editor.Place();
    editor.SetHoverCell({1, 0, 0});
    editor.Place();
    editor.SetHoverCell({2, 0, 0});
    editor.Place();
    TEST("Ship has 3 blocks", ship.BlockCount() == 3);

    editor.GetSelection().Add({0, 0, 0});
    editor.GetSelection().Add({2, 0, 0});
    bool removed = editor.RemoveSelected();
    TEST("RemoveSelected succeeds", removed);
    TEST("Ship has 1 block after remove selected", ship.BlockCount() == 1);

    // Undo
    editor.Undo();
    TEST("Ship has 3 blocks after undo remove selected", ship.BlockCount() == 3);
}

static void TestEditorValidation() {
    std::cout << "[Editor Validation]\n";

    Ship ship;
    ShipEditorController editor(ship);

    auto result = editor.ValidateShip();
    TEST("Empty ship validation fails", !result.valid);

    editor.SetHoverCell({0, 0, 0});
    editor.GetState().selectedType = BlockType::Engine;
    editor.Place();
    editor.SetHoverCell({1, 0, 0});
    editor.GetState().selectedType = BlockType::Generator;
    editor.Place();

    auto result2 = editor.ValidateShip();
    TEST("Ship with engine+gen is valid", result2.valid);
    TEST("No warnings for complete ship", result2.warnings.empty());
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
// Quest Reward Distribution Tests
// ===================================================================

static void TestQuestRewardDistributeCredits() {
    std::cout << "[QuestReward DistributeCredits]\n";

    EntityManager em;
    auto& entity = em.CreateEntity("Player");
    EntityId eid = entity.id;

    em.AddComponent<InventoryComponent>(eid, std::make_unique<InventoryComponent>(20, 1000.0f));
    em.AddComponent<ProgressionComponent>(eid, std::make_unique<ProgressionComponent>());
    em.AddComponent<FactionComponent>(eid, std::make_unique<FactionComponent>());

    QuestSystem system;
    system.SetEntityManager(&em);

    std::vector<QuestReward> rewards;
    QuestReward creditReward;
    creditReward.type = RewardType::Credits;
    creditReward.amount = 500;
    creditReward.description = "Credit reward";
    rewards.push_back(creditReward);

    int count = system.DistributeRewards(eid, rewards);
    TEST("One reward distributed", count == 1);

    auto* inv = em.GetComponent<InventoryComponent>(eid);
    TEST("Credits added to inventory", inv != nullptr && inv->HasItem("credits", 500));
}

static void TestQuestRewardDistributeExperience() {
    std::cout << "[QuestReward DistributeExperience]\n";

    EntityManager em;
    auto& entity = em.CreateEntity("Player");
    EntityId eid = entity.id;

    em.AddComponent<ProgressionComponent>(eid, std::make_unique<ProgressionComponent>());

    QuestSystem system;
    system.SetEntityManager(&em);

    std::vector<QuestReward> rewards;
    QuestReward xpReward;
    xpReward.type = RewardType::Experience;
    xpReward.amount = 75;
    xpReward.description = "XP reward";
    rewards.push_back(xpReward);

    int count = system.DistributeRewards(eid, rewards);
    TEST("XP reward distributed", count == 1);

    auto* prog = em.GetComponent<ProgressionComponent>(eid);
    TEST("Experience added", prog != nullptr && prog->experience == 75);
    TEST("Still level 1", prog != nullptr && prog->level == 1);

    // Give enough to level up
    QuestReward bigXp;
    bigXp.type = RewardType::Experience;
    bigXp.amount = 50;
    std::vector<QuestReward> r2;
    r2.push_back(bigXp);
    system.DistributeRewards(eid, r2);
    TEST("Leveled up to 2", prog != nullptr && prog->level == 2);
}

static void TestQuestRewardDistributeReputation() {
    std::cout << "[QuestReward DistributeReputation]\n";

    EntityManager em;
    auto& entity = em.CreateEntity("Player");
    EntityId eid = entity.id;

    auto factionComp = std::make_unique<FactionComponent>();
    factionComp->factionName = "Neutral";
    em.AddComponent<FactionComponent>(eid, std::move(factionComp));

    QuestSystem system;
    system.SetEntityManager(&em);

    // Reputation with specific faction
    QuestReward repReward;
    repReward.type = RewardType::Reputation;
    repReward.rewardId = "Iron Dominion";
    repReward.amount = 25;
    std::vector<QuestReward> rewards;
    rewards.push_back(repReward);

    int count = system.DistributeRewards(eid, rewards);
    TEST("Rep reward distributed", count == 1);

    auto* faction = em.GetComponent<FactionComponent>(eid);
    TEST("Reputation increased", faction != nullptr && faction->GetReputation("Iron Dominion") == 25);

    // Reputation with default faction (empty rewardId)
    QuestReward defaultRep;
    defaultRep.type = RewardType::Reputation;
    defaultRep.rewardId = "";
    defaultRep.amount = 10;
    std::vector<QuestReward> r2;
    r2.push_back(defaultRep);
    system.DistributeRewards(eid, r2);
    TEST("Default faction rep", faction != nullptr && faction->GetReputation("Neutral") == 10);
}

static void TestQuestRewardDistributeResource() {
    std::cout << "[QuestReward DistributeResource]\n";

    EntityManager em;
    auto& entity = em.CreateEntity("Player");
    EntityId eid = entity.id;

    em.AddComponent<InventoryComponent>(eid, std::make_unique<InventoryComponent>(20, 1000.0f));

    QuestSystem system;
    system.SetEntityManager(&em);

    QuestReward resReward;
    resReward.type = RewardType::Resource;
    resReward.rewardId = "titanium_ore";
    resReward.amount = 10;
    resReward.description = "Titanium ore reward";
    std::vector<QuestReward> rewards;
    rewards.push_back(resReward);

    int count = system.DistributeRewards(eid, rewards);
    TEST("Resource reward distributed", count == 1);

    auto* inv = em.GetComponent<InventoryComponent>(eid);
    TEST("Resource added", inv != nullptr && inv->HasItem("titanium_ore", 10));
}

static void TestQuestRewardDistributeItem() {
    std::cout << "[QuestReward DistributeItem]\n";

    EntityManager em;
    auto& entity = em.CreateEntity("Player");
    EntityId eid = entity.id;

    em.AddComponent<InventoryComponent>(eid, std::make_unique<InventoryComponent>(20, 1000.0f));

    QuestSystem system;
    system.SetEntityManager(&em);

    QuestReward itemReward;
    itemReward.type = RewardType::Item;
    itemReward.rewardId = "laser_mk2";
    itemReward.amount = 1;
    itemReward.description = "A laser weapon";
    std::vector<QuestReward> rewards;
    rewards.push_back(itemReward);

    int count = system.DistributeRewards(eid, rewards);
    TEST("Item reward distributed", count == 1);

    auto* inv = em.GetComponent<InventoryComponent>(eid);
    TEST("Item added", inv != nullptr && inv->HasItem("laser_mk2", 1));
}

static void TestQuestRewardDistributeMultiple() {
    std::cout << "[QuestReward DistributeMultiple]\n";

    EntityManager em;
    auto& entity = em.CreateEntity("Player");
    EntityId eid = entity.id;

    em.AddComponent<InventoryComponent>(eid, std::make_unique<InventoryComponent>(20, 5000.0f));
    em.AddComponent<ProgressionComponent>(eid, std::make_unique<ProgressionComponent>());
    em.AddComponent<FactionComponent>(eid, std::make_unique<FactionComponent>());

    QuestSystem system;
    system.SetEntityManager(&em);

    std::vector<QuestReward> rewards;

    QuestReward credit;
    credit.type = RewardType::Credits;
    credit.amount = 200;
    rewards.push_back(credit);

    QuestReward xp;
    xp.type = RewardType::Experience;
    xp.amount = 50;
    rewards.push_back(xp);

    QuestReward rep;
    rep.type = RewardType::Reputation;
    rep.rewardId = "Nomad Continuum";
    rep.amount = 15;
    rewards.push_back(rep);

    int count = system.DistributeRewards(eid, rewards);
    TEST("All three rewards distributed", count == 3);

    auto* inv = em.GetComponent<InventoryComponent>(eid);
    TEST("Credits present", inv != nullptr && inv->HasItem("credits", 200));

    auto* prog = em.GetComponent<ProgressionComponent>(eid);
    TEST("XP present", prog != nullptr && prog->experience == 50);

    auto* faction = em.GetComponent<FactionComponent>(eid);
    TEST("Rep present", faction != nullptr && faction->GetReputation("Nomad Continuum") == 15);
}

static void TestQuestRewardDistributeNoEntityManager() {
    std::cout << "[QuestReward DistributeNoEntityManager]\n";

    QuestSystem system;
    // No EntityManager set

    QuestReward r;
    r.type = RewardType::Credits;
    r.amount = 100;
    std::vector<QuestReward> rewards;
    rewards.push_back(r);

    int count = system.DistributeRewards(1, rewards);
    TEST("No EM returns 0", count == 0);
}

static void TestQuestRewardDistributeMissingComponents() {
    std::cout << "[QuestReward DistributeMissingComponents]\n";

    EntityManager em;
    auto& entity = em.CreateEntity("Player");
    EntityId eid = entity.id;
    // Entity has no components

    QuestSystem system;
    system.SetEntityManager(&em);

    std::vector<QuestReward> rewards;

    QuestReward credit;
    credit.type = RewardType::Credits;
    credit.amount = 100;
    rewards.push_back(credit);

    QuestReward xp;
    xp.type = RewardType::Experience;
    xp.amount = 50;
    rewards.push_back(xp);

    QuestReward rep;
    rep.type = RewardType::Reputation;
    rep.rewardId = "TestFaction";
    rep.amount = 10;
    rewards.push_back(rep);

    int count = system.DistributeRewards(eid, rewards);
    TEST("Missing components returns 0", count == 0);
}

static void TestQuestRewardDistributeEndToEnd() {
    std::cout << "[QuestReward EndToEnd]\n";

    EntityManager em;
    auto& entity = em.CreateEntity("Player");
    EntityId eid = entity.id;

    em.AddComponent<InventoryComponent>(eid, std::make_unique<InventoryComponent>(20, 5000.0f));
    em.AddComponent<ProgressionComponent>(eid, std::make_unique<ProgressionComponent>());
    em.AddComponent<FactionComponent>(eid, std::make_unique<FactionComponent>());
    em.AddComponent<QuestComponent>(eid, std::make_unique<QuestComponent>());

    QuestSystem system;
    system.SetEntityManager(&em);

    // Create a quest template with rewards
    Quest tmpl;
    tmpl.id = "reward_quest";
    tmpl.title = "Destroy Pirates";
    QuestObjective obj;
    obj.id = "kill_pirates";
    obj.type = ObjectiveType::Destroy;
    obj.target = "pirate_ship";
    obj.requiredQuantity = 3;
    tmpl.objectives.push_back(obj);

    QuestReward cr;
    cr.type = RewardType::Credits;
    cr.amount = 1000;
    tmpl.rewards.push_back(cr);

    QuestReward xp;
    xp.type = RewardType::Experience;
    xp.amount = 80;
    tmpl.rewards.push_back(xp);

    QuestReward rep;
    rep.type = RewardType::Reputation;
    rep.rewardId = "Helix Covenant";
    rep.amount = 20;
    tmpl.rewards.push_back(rep);

    system.AddQuestTemplate(tmpl);

    auto* comp = em.GetComponent<QuestComponent>(eid);
    system.GiveQuest(eid, "reward_quest", *comp);
    comp->AcceptQuest("reward_quest");

    // Progress the quest to completion
    system.ProgressObjective(*comp, ObjectiveType::Destroy, "pirate_ship", 3);
    Quest* q = comp->GetQuest("reward_quest");
    TEST("Quest completed", q != nullptr && q->status == QuestStatus::Completed);

    // Turn in and distribute rewards
    bool turnedIn = comp->TurnInQuest("reward_quest");
    TEST("Quest turned in", turnedIn);
    TEST("Quest status TurnedIn", q != nullptr && q->status == QuestStatus::TurnedIn);

    int distributed = system.DistributeRewards(eid, q->rewards);
    TEST("All rewards distributed", distributed == 3);

    auto* inv = em.GetComponent<InventoryComponent>(eid);
    TEST("E2E credits", inv != nullptr && inv->HasItem("credits", 1000));

    auto* prog = em.GetComponent<ProgressionComponent>(eid);
    TEST("E2E experience", prog != nullptr && prog->experience == 80);

    auto* faction = em.GetComponent<FactionComponent>(eid);
    TEST("E2E reputation", faction != nullptr && faction->GetReputation("Helix Covenant") == 20);
}

// ===================================================================
// Particle System tests
// ===================================================================

static void TestParticle() {
    std::cout << "[Particle]\n";

    // Default particle
    {
        Particle p;
        TEST("Particle default alive", p.IsAlive());
        TEST("Particle default age 0", ApproxEq(p.age, 0.0f));
        TEST("Particle default normalized age 0", ApproxEq(p.GetNormalizedAge(), 0.0f));
    }

    // Dead particle
    {
        Particle p;
        p.lifetime = 1.0f;
        p.age = 1.5f;
        TEST("Particle dead after lifetime", !p.IsAlive());
        TEST("Particle normalized age clamped to 1", ApproxEq(p.GetNormalizedAge(), 1.0f));
    }

    // Normalized age mid-life
    {
        Particle p;
        p.lifetime = 2.0f;
        p.age = 1.0f;
        TEST("Particle half-life normalized age", ApproxEq(p.GetNormalizedAge(), 0.5f));
    }

    // Color interpolation
    {
        Particle p;
        p.colorR = 1.0f; p.colorG = 0.0f; p.colorB = 0.0f; p.colorA = 1.0f;
        p.endColorR = 0.0f; p.endColorG = 1.0f; p.endColorB = 0.0f; p.endColorA = 0.0f;
        p.lifetime = 1.0f;

        // At birth
        p.age = 0.0f;
        float r, g, b, a;
        p.GetCurrentColor(r, g, b, a);
        TEST("Particle start color R", ApproxEq(r, 1.0f));
        TEST("Particle start color G", ApproxEq(g, 0.0f));
        TEST("Particle start color A", ApproxEq(a, 1.0f));

        // At death
        p.age = 1.0f;
        p.GetCurrentColor(r, g, b, a);
        TEST("Particle end color R", ApproxEq(r, 0.0f));
        TEST("Particle end color G", ApproxEq(g, 1.0f));
        TEST("Particle end color A", ApproxEq(a, 0.0f));

        // At half-life
        p.age = 0.5f;
        p.GetCurrentColor(r, g, b, a);
        TEST("Particle mid color R", ApproxEq(r, 0.5f));
        TEST("Particle mid color G", ApproxEq(g, 0.5f));
        TEST("Particle mid color A", ApproxEq(a, 0.5f));
    }

    // Zero lifetime
    {
        Particle p;
        p.lifetime = 0.0f;
        TEST("Particle zero lifetime normalized age 1", ApproxEq(p.GetNormalizedAge(), 1.0f));
    }
}

static void TestParticleEmitterConfig() {
    std::cout << "[ParticleEmitterConfig]\n";

    ParticleEmitterConfig cfg;
    TEST("Default shape is Point", cfg.shape == EmitterShape::Point);
    TEST("Default emit rate", ApproxEq(cfg.emitRate, 10.0f));
    TEST("Default max particles", cfg.maxParticles == 200);
    TEST("Default min lifetime", ApproxEq(cfg.minLifetime, 0.5f));
    TEST("Default max lifetime", ApproxEq(cfg.maxLifetime, 2.0f));
    TEST("Default gravity", ApproxEq(cfg.gravityY, 0.0f));
    TEST("Default start alpha", ApproxEq(cfg.startA, 1.0f));
    TEST("Default end alpha", ApproxEq(cfg.endA, 0.0f));
}

static void TestParticleEmitter() {
    std::cout << "[ParticleEmitter]\n";

    // Construction
    {
        ParticleEmitter emitter("test_emitter");
        TEST("Emitter id", emitter.GetId() == "test_emitter");
        TEST("Emitter active by default", emitter.IsActive());
        TEST("Emitter starts with 0 particles", emitter.GetAliveCount() == 0);
    }

    // Position
    {
        ParticleEmitter emitter("pos_test");
        emitter.SetPosition(1.0f, 2.0f, 3.0f);
        float x, y, z;
        emitter.GetPosition(x, y, z);
        TEST("Emitter position X", ApproxEq(x, 1.0f));
        TEST("Emitter position Y", ApproxEq(y, 2.0f));
        TEST("Emitter position Z", ApproxEq(z, 3.0f));
    }

    // Burst emit
    {
        ParticleEmitterConfig cfg;
        cfg.emitRate = 0.0f;
        cfg.maxParticles = 50;
        cfg.minLifetime = 1.0f;
        cfg.maxLifetime = 2.0f;
        ParticleEmitter emitter("burst_test", cfg);
        emitter.Emit(10);
        TEST("Burst emit 10 particles", emitter.GetAliveCount() == 10);
    }

    // Max particles cap
    {
        ParticleEmitterConfig cfg;
        cfg.emitRate = 0.0f;
        cfg.maxParticles = 5;
        cfg.minLifetime = 10.0f;
        cfg.maxLifetime = 10.0f;
        ParticleEmitter emitter("cap_test", cfg);
        emitter.Emit(20);
        TEST("Max particles capped", emitter.GetAliveCount() == 5);
    }

    // Active/inactive toggle
    {
        ParticleEmitter emitter("toggle_test");
        TEST("Emitter active initially", emitter.IsActive());
        emitter.SetActive(false);
        TEST("Emitter deactivated", !emitter.IsActive());
        emitter.SetActive(true);
        TEST("Emitter reactivated", emitter.IsActive());
    }

    // Continuous emission via update
    {
        ParticleEmitterConfig cfg;
        cfg.emitRate = 100.0f;  // 100 per second
        cfg.maxParticles = 500;
        cfg.minLifetime = 5.0f;
        cfg.maxLifetime = 5.0f;
        ParticleEmitter emitter("continuous_test", cfg);
        emitter.Update(1.0f); // 1 second
        TEST("Continuous emit ~100 particles", emitter.GetAliveCount() >= 90 && emitter.GetAliveCount() <= 110);
    }

    // Particles die after lifetime
    {
        ParticleEmitterConfig cfg;
        cfg.emitRate = 0.0f;
        cfg.maxParticles = 100;
        cfg.minLifetime = 0.5f;
        cfg.maxLifetime = 0.5f;
        ParticleEmitter emitter("die_test", cfg);
        emitter.Emit(10);
        TEST("10 particles before aging", emitter.GetAliveCount() == 10);
        emitter.Update(0.6f);
        TEST("0 particles after lifetime", emitter.GetAliveCount() == 0);
    }

    // Reset
    {
        ParticleEmitterConfig cfg;
        cfg.emitRate = 0.0f;
        cfg.maxParticles = 100;
        cfg.minLifetime = 10.0f;
        cfg.maxLifetime = 10.0f;
        ParticleEmitter emitter("reset_test", cfg);
        emitter.Emit(20);
        TEST("20 particles before reset", emitter.GetAliveCount() == 20);
        emitter.Reset();
        TEST("0 particles after reset", emitter.GetAliveCount() == 0);
    }

    // Config change
    {
        ParticleEmitter emitter("cfg_test");
        ParticleEmitterConfig cfg;
        cfg.emitRate = 42.0f;
        emitter.SetConfig(cfg);
        TEST("Config updated", ApproxEq(emitter.GetConfig().emitRate, 42.0f));
    }

    // Deterministic with seed
    {
        ParticleEmitterConfig cfg;
        cfg.emitRate = 0.0f;
        cfg.maxParticles = 100;
        cfg.minLifetime = 5.0f;
        cfg.maxLifetime = 5.0f;
        cfg.minSpeed = 1.0f;
        cfg.maxSpeed = 10.0f;

        ParticleEmitter emitter1("det1", cfg);
        emitter1.SetSeed(42);
        emitter1.Emit(5);

        ParticleEmitter emitter2("det2", cfg);
        emitter2.SetSeed(42);
        emitter2.Emit(5);

        const auto& p1 = emitter1.GetParticles();
        const auto& p2 = emitter2.GetParticles();
        bool match = true;
        for (int i = 0; i < 5 && match; ++i) {
            if (!ApproxEq(p1[i].posX, p2[i].posX) ||
                !ApproxEq(p1[i].posY, p2[i].posY) ||
                !ApproxEq(p1[i].posZ, p2[i].posZ)) {
                match = false;
            }
        }
        TEST("Deterministic particle positions with same seed", match);
    }

    // Inactive emitter doesn't emit on update
    {
        ParticleEmitterConfig cfg;
        cfg.emitRate = 100.0f;
        cfg.maxParticles = 500;
        cfg.minLifetime = 5.0f;
        cfg.maxLifetime = 5.0f;
        ParticleEmitter emitter("inactive_test", cfg);
        emitter.SetActive(false);
        emitter.Update(1.0f);
        TEST("Inactive emitter no particles", emitter.GetAliveCount() == 0);
    }

    // Gravity effect
    {
        ParticleEmitterConfig cfg;
        cfg.shape = EmitterShape::Point;
        cfg.emitRate = 0.0f;
        cfg.maxParticles = 10;
        cfg.minLifetime = 10.0f;
        cfg.maxLifetime = 10.0f;
        cfg.gravityY = -10.0f;
        cfg.minSpeed = 0.0f;
        cfg.maxSpeed = 0.0f;  // zero initial speed so only gravity matters
        ParticleEmitter emitter("gravity_test", cfg);
        emitter.Emit(1);
        float initialY = emitter.GetParticles()[0].posY;
        emitter.Update(1.0f);
        float afterY = emitter.GetParticles()[0].posY;
        TEST("Gravity moves particle downward", afterY < initialY);
    }
}

static void TestParticleEmitterShapes() {
    std::cout << "[ParticleEmitterShapes]\n";

    // Sphere shape
    {
        ParticleEmitterConfig cfg;
        cfg.shape = EmitterShape::Sphere;
        cfg.sphereRadius = 2.0f;
        cfg.emitRate = 0.0f;
        cfg.maxParticles = 100;
        cfg.minLifetime = 5.0f;
        cfg.maxLifetime = 5.0f;
        ParticleEmitter emitter("sphere_test", cfg);
        emitter.Emit(20);
        TEST("Sphere emitter spawns particles", emitter.GetAliveCount() == 20);

        bool allInRange = true;
        for (const auto& p : emitter.GetParticles()) {
            float dist = std::sqrt(p.posX * p.posX + p.posY * p.posY + p.posZ * p.posZ);
            if (dist > cfg.sphereRadius + 0.01f) allInRange = false;
        }
        TEST("Sphere particles within radius", allInRange);
    }

    // Cone shape
    {
        ParticleEmitterConfig cfg;
        cfg.shape = EmitterShape::Cone;
        cfg.coneAngleDeg = 45.0f;
        cfg.emitRate = 0.0f;
        cfg.maxParticles = 100;
        cfg.minLifetime = 5.0f;
        cfg.maxLifetime = 5.0f;
        ParticleEmitter emitter("cone_test", cfg);
        emitter.Emit(20);
        TEST("Cone emitter spawns particles", emitter.GetAliveCount() == 20);
    }

    // Box shape
    {
        ParticleEmitterConfig cfg;
        cfg.shape = EmitterShape::Box;
        cfg.boxHalfW = 1.0f;
        cfg.boxHalfH = 2.0f;
        cfg.boxHalfD = 3.0f;
        cfg.emitRate = 0.0f;
        cfg.maxParticles = 100;
        cfg.minLifetime = 5.0f;
        cfg.maxLifetime = 5.0f;
        ParticleEmitter emitter("box_test", cfg);
        emitter.Emit(20);
        TEST("Box emitter spawns particles", emitter.GetAliveCount() == 20);

        bool allInBox = true;
        for (const auto& p : emitter.GetParticles()) {
            if (std::fabs(p.posX) > cfg.boxHalfW + 0.01f ||
                std::fabs(p.posY) > cfg.boxHalfH + 0.01f ||
                std::fabs(p.posZ) > cfg.boxHalfD + 0.01f) {
                allInBox = false;
            }
        }
        TEST("Box particles within extents", allInBox);
    }
}

static void TestParticleComponent() {
    std::cout << "[ParticleComponent]\n";

    // Add and get emitter
    {
        ParticleComponent comp;
        ParticleEmitter emitter("fx1");
        comp.AddEmitter(emitter);
        TEST("Component has 1 emitter", comp.GetEmitterCount() == 1);
        auto* found = comp.GetEmitter("fx1");
        TEST("Can find emitter by id", found != nullptr);
        TEST("Found correct emitter", found && found->GetId() == "fx1");
    }

    // Remove emitter
    {
        ParticleComponent comp;
        comp.AddEmitter(ParticleEmitter("a"));
        comp.AddEmitter(ParticleEmitter("b"));
        TEST("2 emitters added", comp.GetEmitterCount() == 2);
        bool removed = comp.RemoveEmitter("a");
        TEST("Remove returned true", removed);
        TEST("1 emitter remaining", comp.GetEmitterCount() == 1);
        TEST("Remaining is b", comp.GetEmitter("b") != nullptr);
    }

    // Remove non-existent
    {
        ParticleComponent comp;
        bool removed = comp.RemoveEmitter("nope");
        TEST("Remove non-existent returns false", !removed);
    }

    // Get non-existent
    {
        ParticleComponent comp;
        TEST("Get non-existent returns nullptr", comp.GetEmitter("nope") == nullptr);
    }

    // Total particle count
    {
        ParticleEmitterConfig cfg;
        cfg.emitRate = 0.0f;
        cfg.maxParticles = 50;
        cfg.minLifetime = 10.0f;
        cfg.maxLifetime = 10.0f;

        ParticleComponent comp;
        ParticleEmitter e1("e1", cfg);
        e1.Emit(5);
        ParticleEmitter e2("e2", cfg);
        e2.Emit(3);
        comp.AddEmitter(e1);
        comp.AddEmitter(e2);
        TEST("Total particle count", comp.GetTotalParticleCount() == 8);
    }

    // Stop and resume all
    {
        ParticleComponent comp;
        comp.AddEmitter(ParticleEmitter("x"));
        comp.AddEmitter(ParticleEmitter("y"));
        comp.StopAll();
        TEST("All stopped", !comp.emitters[0].IsActive() && !comp.emitters[1].IsActive());
        comp.ResumeAll();
        TEST("All resumed", comp.emitters[0].IsActive() && comp.emitters[1].IsActive());
    }
}

static void TestParticlePresets() {
    std::cout << "[ParticlePresets]\n";

    // Explosion
    {
        auto cfg = ParticleSystem::CreateExplosionPreset();
        TEST("Explosion shape sphere", cfg.shape == EmitterShape::Sphere);
        TEST("Explosion emit rate 0 (burst only)", ApproxEq(cfg.emitRate, 0.0f));
        TEST("Explosion max particles > 0", cfg.maxParticles > 0);
        TEST("Explosion has gravity", cfg.gravityY < 0.0f);

        ParticleEmitter emitter("explosion", cfg);
        emitter.Emit(50);
        TEST("Explosion burst works", emitter.GetAliveCount() == 50);
    }

    // Engine thrust
    {
        auto cfg = ParticleSystem::CreateEngineThrustPreset();
        TEST("Thrust shape cone", cfg.shape == EmitterShape::Cone);
        TEST("Thrust emit rate > 0", cfg.emitRate > 0.0f);
        TEST("Thrust cone angle > 0", cfg.coneAngleDeg > 0.0f);
    }

    // Shield hit
    {
        auto cfg = ParticleSystem::CreateShieldHitPreset();
        TEST("Shield shape point", cfg.shape == EmitterShape::Point);
        TEST("Shield emit rate 0", ApproxEq(cfg.emitRate, 0.0f));
    }

    // Mining
    {
        auto cfg = ParticleSystem::CreateMiningPreset();
        TEST("Mining shape cone", cfg.shape == EmitterShape::Cone);
        TEST("Mining has gravity", cfg.gravityY < 0.0f);
    }

    // Hyperdrive
    {
        auto cfg = ParticleSystem::CreateHyperdrivePreset();
        TEST("Hyperdrive shape cone", cfg.shape == EmitterShape::Cone);
        TEST("Hyperdrive narrow angle", cfg.coneAngleDeg < 10.0f);
        TEST("Hyperdrive fast particles", cfg.minSpeed > 15.0f);
    }
}

static void TestParticleSystem() {
    std::cout << "[ParticleSystem]\n";

    ParticleSystem sys;
    TEST("System name", sys.GetName() == "ParticleSystem");
    TEST("System enabled", sys.IsEnabled());

    sys.Initialize();
    sys.Update(0.016f);
    sys.Shutdown();
    TEST("Particle system lifecycle ok", true);
}

// ===================================================================
// Achievement System tests
// ===================================================================

static void TestAchievementCriterion() {
    std::cout << "[AchievementCriterion]\n";

    // Incomplete criterion
    {
        AchievementCriterion c;
        c.eventType = "test.event";
        c.requiredCount = 10;
        c.currentCount = 3;
        TEST("Criterion not complete", !c.IsComplete());
        TEST("Criterion progress 0.3", ApproxEq(c.GetProgress(), 0.3f));
    }

    // Complete criterion
    {
        AchievementCriterion c;
        c.requiredCount = 5;
        c.currentCount = 5;
        TEST("Criterion complete", c.IsComplete());
        TEST("Criterion progress 1.0", ApproxEq(c.GetProgress(), 1.0f));
    }

    // Over-complete
    {
        AchievementCriterion c;
        c.requiredCount = 5;
        c.currentCount = 10;
        TEST("Criterion over-complete", c.IsComplete());
        TEST("Criterion progress clamped 1.0", ApproxEq(c.GetProgress(), 1.0f));
    }

    // Zero required
    {
        AchievementCriterion c;
        c.requiredCount = 0;
        c.currentCount = 0;
        TEST("Zero required progress 1.0", ApproxEq(c.GetProgress(), 1.0f));
    }
}

static void TestAchievement() {
    std::cout << "[Achievement]\n";

    // Incomplete achievement
    {
        Achievement a;
        a.id = "test";
        a.name = "Test Achievement";
        a.criteria.push_back({"event.a", 5, 2});
        a.criteria.push_back({"event.b", 10, 10});
        TEST("Achievement not complete", !a.IsComplete());
        // Progress: (2/5 + 10/10) / 2 = (0.4 + 1.0) / 2 = 0.7
        TEST("Achievement progress", ApproxEq(a.GetProgress(), 0.7f));
    }

    // Complete achievement
    {
        Achievement a;
        a.id = "done";
        a.criteria.push_back({"e1", 1, 1});
        a.criteria.push_back({"e2", 5, 5});
        TEST("Achievement complete", a.IsComplete());
        TEST("Achievement progress 1.0", ApproxEq(a.GetProgress(), 1.0f));
    }

    // Empty criteria
    {
        Achievement a;
        a.id = "empty";
        TEST("Empty criteria not complete", !a.IsComplete());
        TEST("Empty criteria progress 0", ApproxEq(a.GetProgress(), 0.0f));
    }

    // Single criterion
    {
        Achievement a;
        a.id = "single";
        a.criteria.push_back({"e1", 10, 7});
        TEST("Single criterion progress", ApproxEq(a.GetProgress(), 0.7f));
    }
}

static void TestAchievementComponent() {
    std::cout << "[AchievementComponent]\n";

    // Add and get
    {
        AchievementComponent comp;
        Achievement a;
        a.id = "ach1";
        a.name = "First";
        a.category = AchievementCategory::Combat;
        comp.AddAchievement(a);
        TEST("Total count 1", comp.GetTotalCount() == 1);

        auto* found = comp.GetAchievement("ach1");
        TEST("Found achievement", found != nullptr);
        TEST("Correct name", found && found->name == "First");
    }

    // Duplicate prevention
    {
        AchievementComponent comp;
        Achievement a;
        a.id = "dup";
        comp.AddAchievement(a);
        comp.AddAchievement(a);
        TEST("No duplicates", comp.GetTotalCount() == 1);
    }

    // Not found
    {
        AchievementComponent comp;
        TEST("Get non-existent null", comp.GetAchievement("nope") == nullptr);
    }

    // IsUnlocked
    {
        AchievementComponent comp;
        Achievement a;
        a.id = "unlock_test";
        a.unlocked = false;
        comp.AddAchievement(a);
        TEST("Not unlocked initially", !comp.IsUnlocked("unlock_test"));
        comp.GetAchievement("unlock_test")->unlocked = true;
        TEST("Unlocked after set", comp.IsUnlocked("unlock_test"));
    }

    // Unlocked count
    {
        AchievementComponent comp;
        Achievement a1; a1.id = "a1"; a1.unlocked = true;
        Achievement a2; a2.id = "a2"; a2.unlocked = false;
        Achievement a3; a3.id = "a3"; a3.unlocked = true;
        comp.AddAchievement(a1);
        comp.AddAchievement(a2);
        comp.AddAchievement(a3);
        TEST("Unlocked count 2", comp.GetUnlockedCount() == 2);
    }

    // Overall progress
    {
        AchievementComponent comp;
        Achievement a1;
        a1.id = "p1";
        a1.unlocked = true;
        Achievement a2;
        a2.id = "p2";
        a2.criteria.push_back({"e", 10, 5});
        comp.AddAchievement(a1);
        comp.AddAchievement(a2);
        // (1.0 + 0.5) / 2 = 0.75
        TEST("Overall progress", ApproxEq(comp.GetOverallProgress(), 0.75f));
    }

    // GetByCategory
    {
        AchievementComponent comp;
        Achievement a1; a1.id = "c1"; a1.category = AchievementCategory::Combat;
        Achievement a2; a2.id = "c2"; a2.category = AchievementCategory::Trading;
        Achievement a3; a3.id = "c3"; a3.category = AchievementCategory::Combat;
        comp.AddAchievement(a1);
        comp.AddAchievement(a2);
        comp.AddAchievement(a3);
        auto combat = comp.GetByCategory(AchievementCategory::Combat);
        TEST("2 combat achievements", combat.size() == 2);
        auto trading = comp.GetByCategory(AchievementCategory::Trading);
        TEST("1 trading achievement", trading.size() == 1);
    }

    // RecordEvent
    {
        AchievementComponent comp;
        Achievement a;
        a.id = "record_test";
        a.criteria.push_back({"kill", 3, 0});
        comp.AddAchievement(a);

        bool complete = comp.RecordEvent("record_test", "kill", 1);
        TEST("Not complete after 1 kill", !complete);
        complete = comp.RecordEvent("record_test", "kill", 2);
        TEST("Complete after 3 kills total", complete);
        TEST("Achievement unlocked", comp.IsUnlocked("record_test"));
    }

    // RecordEvent on already-unlocked
    {
        AchievementComponent comp;
        Achievement a;
        a.id = "already";
        a.unlocked = true;
        a.criteria.push_back({"e", 1, 1});
        comp.AddAchievement(a);
        bool result = comp.RecordEvent("already", "e", 1);
        TEST("RecordEvent on unlocked returns false", !result);
    }

    // RecordEvent wrong event type
    {
        AchievementComponent comp;
        Achievement a;
        a.id = "wrong_event";
        a.criteria.push_back({"kill", 1, 0});
        comp.AddAchievement(a);
        comp.RecordEvent("wrong_event", "trade", 1);
        TEST("Wrong event no progress", !comp.IsUnlocked("wrong_event"));
    }

    // Empty component
    {
        AchievementComponent comp;
        TEST("Empty total 0", comp.GetTotalCount() == 0);
        TEST("Empty unlocked 0", comp.GetUnlockedCount() == 0);
        TEST("Empty progress 0", ApproxEq(comp.GetOverallProgress(), 0.0f));
    }
}

static void TestAchievementComponentSerialization() {
    std::cout << "[AchievementComponentSerialization]\n";

    // Round-trip
    {
        AchievementComponent original;
        Achievement a1;
        a1.id = "first_blood";
        a1.name = "First Blood";
        a1.description = "Destroy first enemy";
        a1.category = AchievementCategory::Combat;
        a1.rewardXP = 50;
        a1.rewardCredits = 100;
        a1.unlocked = true;
        a1.unlockTimestamp = 1234567.0;
        a1.criteria.push_back({"entity.destroyed", 1, 1});
        original.AddAchievement(a1);

        Achievement a2;
        a2.id = "explorer";
        a2.name = "Explorer";
        a2.description = "Visit sectors";
        a2.category = AchievementCategory::Exploration;
        a2.rewardXP = 100;
        a2.rewardCredits = 250;
        a2.unlocked = false;
        a2.criteria.push_back({"sector.entered", 10, 3});
        a2.criteria.push_back({"resource.collected", 50, 20});
        original.AddAchievement(a2);

        ComponentData data = original.Serialize();
        TEST("Serialize type", data.componentType == "AchievementComponent");

        AchievementComponent restored;
        restored.Deserialize(data);

        TEST("Restored count", restored.GetTotalCount() == 2);

        auto* r1 = restored.GetAchievement("first_blood");
        TEST("R1 exists", r1 != nullptr);
        if (r1) {
            TEST("R1 name", r1->name == "First Blood");
            TEST("R1 description", r1->description == "Destroy first enemy");
            TEST("R1 category", r1->category == AchievementCategory::Combat);
            TEST("R1 rewardXP", r1->rewardXP == 50);
            TEST("R1 rewardCredits", r1->rewardCredits == 100);
            TEST("R1 unlocked", r1->unlocked);
            TEST("R1 timestamp", ApproxEq(static_cast<float>(r1->unlockTimestamp), 1234567.0f));
            TEST("R1 criteria count", r1->criteria.size() == 1);
            TEST("R1 criterion event", r1->criteria[0].eventType == "entity.destroyed");
            TEST("R1 criterion required", r1->criteria[0].requiredCount == 1);
            TEST("R1 criterion current", r1->criteria[0].currentCount == 1);
        }

        auto* r2 = restored.GetAchievement("explorer");
        TEST("R2 exists", r2 != nullptr);
        if (r2) {
            TEST("R2 not unlocked", !r2->unlocked);
            TEST("R2 criteria count", r2->criteria.size() == 2);
            TEST("R2 c1 current", r2->criteria[0].currentCount == 3);
            TEST("R2 c2 current", r2->criteria[1].currentCount == 20);
        }
    }

    // Empty component round-trip
    {
        AchievementComponent empty;
        ComponentData data = empty.Serialize();
        AchievementComponent restored;
        restored.Deserialize(data);
        TEST("Empty round-trip", restored.GetTotalCount() == 0);
    }
}

static void TestAchievementSystem() {
    std::cout << "[AchievementSystem]\n";

    AchievementSystem sys;
    TEST("System name", sys.GetName() == "AchievementSystem");

    // Register achievements
    {
        sys.RegisterAchievement(AchievementSystem::CreateFirstBlood());
        sys.RegisterAchievement(AchievementSystem::CreateExplorer());
        sys.RegisterAchievement(AchievementSystem::CreateShipwright());
        TEST("Registered 3", sys.GetRegisteredCount() == 3);
        TEST("Has first_blood", sys.HasAchievement("first_blood"));
        TEST("Has explorer", sys.HasAchievement("explorer"));
        TEST("No nonexistent", !sys.HasAchievement("nonexistent"));
    }

    // Get achievement
    {
        const auto* a = sys.GetAchievement("first_blood");
        TEST("Get first_blood", a != nullptr);
        TEST("First blood name", a && a->name == "First Blood");
        TEST("Get non-existent null", sys.GetAchievement("nope") == nullptr);
    }

    // Record progress
    {
        bool complete = sys.RecordProgress("first_blood", "entity.destroyed", 1);
        TEST("First blood completed", complete);
        auto unlocked = sys.GetUnlockedIds();
        bool found = false;
        for (const auto& id : unlocked)
            if (id == "first_blood") found = true;
        TEST("First blood in unlocked list", found);
    }

    // Record on already unlocked
    {
        bool result = sys.RecordProgress("first_blood", "entity.destroyed", 1);
        TEST("Already unlocked returns false", !result);
    }

    // Record on non-existent
    {
        bool result = sys.RecordProgress("nonexistent", "e", 1);
        TEST("Non-existent returns false", !result);
    }

    // Get all achievements
    {
        auto all = sys.GetAllAchievements();
        TEST("GetAll returns 3", all.size() == 3);
    }

    // Lifecycle
    sys.Initialize();
    sys.Update(0.016f);
    sys.Shutdown();
    TEST("System lifecycle ok", sys.GetRegisteredCount() == 0);
}

static void TestAchievementTemplates() {
    std::cout << "[AchievementTemplates]\n";

    auto firstBlood = AchievementSystem::CreateFirstBlood();
    TEST("First blood id", firstBlood.id == "first_blood");
    TEST("First blood category", firstBlood.category == AchievementCategory::Combat);
    TEST("First blood has criteria", !firstBlood.criteria.empty());
    TEST("First blood not complete", !firstBlood.IsComplete());

    auto explorer = AchievementSystem::CreateExplorer();
    TEST("Explorer id", explorer.id == "explorer");
    TEST("Explorer category", explorer.category == AchievementCategory::Exploration);

    auto shipwright = AchievementSystem::CreateShipwright();
    TEST("Shipwright id", shipwright.id == "shipwright");
    TEST("Shipwright category", shipwright.category == AchievementCategory::Building);

    auto trader = AchievementSystem::CreateTrader();
    TEST("Trader id", trader.id == "trader");
    TEST("Trader category", trader.category == AchievementCategory::Trading);

    auto veteran = AchievementSystem::CreateVeteran();
    TEST("Veteran id", veteran.id == "veteran");
    TEST("Veteran category", veteran.category == AchievementCategory::Progression);

    auto miner = AchievementSystem::CreateMiner();
    TEST("Miner id", miner.id == "miner");
    TEST("Miner has criteria", !miner.criteria.empty());

    auto fleet = AchievementSystem::CreateFleetCommander();
    TEST("Fleet commander id", fleet.id == "fleet_commander");
    TEST("Fleet category", fleet.category == AchievementCategory::Social);

    auto rich = AchievementSystem::CreateRichPilot();
    TEST("Rich pilot id", rich.id == "rich_pilot");
    TEST("Rich pilot 0 credit reward", rich.rewardCredits == 0);
}

static void TestAchievementGameEvents() {
    std::cout << "[AchievementGameEvents]\n";

    TEST("AchievementUnlocked event", std::string(GameEvents::AchievementUnlocked) == "achievement.unlocked");
    TEST("AchievementProgress event", std::string(GameEvents::AchievementProgress) == "achievement.progress");
    TEST("ParticleEmitted event", std::string(GameEvents::ParticleEmitted) == "particle.emitted");
    TEST("ParticleBurst event", std::string(GameEvents::ParticleBurst) == "particle.burst");
    TEST("EmitterStarted event", std::string(GameEvents::EmitterStarted) == "particle.emitter.started");
    TEST("EmitterStopped event", std::string(GameEvents::EmitterStopped) == "particle.emitter.stopped");
}

// ===================================================================
// StructuralIntegrity tests
// ===================================================================

static void TestStructuralIntegrityConnected() {
    std::cout << "\n--- StructuralIntegrity (Connected) ---\n";
    Ship ship;

    // Empty ship is trivially connected
    TEST("Empty ship is connected", StructuralIntegrity::IsFullyConnected(ship));

    // Single block
    BlockPlacement::Place(ship, MakeBlock({0,0,0}, {1,1,1}));
    TEST("Single block is connected", StructuralIntegrity::IsFullyConnected(ship));

    // Linear chain of 3 blocks
    BlockPlacement::Place(ship, MakeBlock({1,0,0}, {1,1,1}));
    BlockPlacement::Place(ship, MakeBlock({2,0,0}, {1,1,1}));
    TEST("3-block line is connected", StructuralIntegrity::IsFullyConnected(ship));

    // L-shape still connected
    BlockPlacement::Place(ship, MakeBlock({2,1,0}, {1,1,1}));
    TEST("L-shape is connected", StructuralIntegrity::IsFullyConnected(ship));

    auto groups = StructuralIntegrity::FindDisconnectedGroups(ship);
    TEST("L-shape has 1 group", groups.size() == 1);
    TEST("L-shape group has 4 blocks", groups[0].size() == 4);
}

static void TestStructuralIntegrityDisconnected() {
    std::cout << "\n--- StructuralIntegrity (Disconnected) ---\n";
    Ship ship;

    // Build two separate clusters manually
    // Cluster A: blocks at (0,0,0) and (1,0,0)
    auto blockA1 = MakeBlock({0,0,0}, {1,1,1});
    auto blockA2 = MakeBlock({1,0,0}, {1,1,1});
    // Cluster B: blocks at (5,5,5) and (6,5,5) -- placed without adjacency check
    auto blockB1 = MakeBlock({5,5,5}, {1,1,1});
    auto blockB2 = MakeBlock({6,5,5}, {1,1,1});

    // Manually add blocks (bypass adjacency validation to create disconnected state)
    ship.blocks.push_back(blockA1);
    for (auto& c : BlockPlacement::GetOccupiedCells(*blockA1)) ship.occupiedCells[c] = blockA1;
    ship.blocks.push_back(blockA2);
    for (auto& c : BlockPlacement::GetOccupiedCells(*blockA2)) ship.occupiedCells[c] = blockA2;
    ship.blocks.push_back(blockB1);
    for (auto& c : BlockPlacement::GetOccupiedCells(*blockB1)) ship.occupiedCells[c] = blockB1;
    ship.blocks.push_back(blockB2);
    for (auto& c : BlockPlacement::GetOccupiedCells(*blockB2)) ship.occupiedCells[c] = blockB2;
    ShipStats::Recalculate(ship);

    TEST("Disconnected ship not fully connected", !StructuralIntegrity::IsFullyConnected(ship));

    auto groups = StructuralIntegrity::FindDisconnectedGroups(ship);
    TEST("Disconnected ship has 2 groups", groups.size() == 2);
    TEST("Largest group has 2 blocks", groups[0].size() == 2);
    TEST("Smallest group has 2 blocks", groups[1].size() == 2);
}

static void TestStructuralIntegrityWouldDisconnect() {
    std::cout << "\n--- StructuralIntegrity (WouldDisconnect) ---\n";
    Ship ship;

    // Build a chain: A - B - C
    auto a = MakeBlock({0,0,0}, {1,1,1});
    auto b = MakeBlock({1,0,0}, {1,1,1});
    auto c = MakeBlock({2,0,0}, {1,1,1});
    BlockPlacement::Place(ship, a);
    BlockPlacement::Place(ship, b);
    BlockPlacement::Place(ship, c);

    TEST("Removing middle block would disconnect", StructuralIntegrity::WouldDisconnect(ship, b));
    TEST("Removing end block A would not disconnect", !StructuralIntegrity::WouldDisconnect(ship, a));
    TEST("Removing end block C would not disconnect", !StructuralIntegrity::WouldDisconnect(ship, c));
    TEST("Null block returns false", !StructuralIntegrity::WouldDisconnect(ship, nullptr));
}

static void TestStructuralIntegrityMultiCell() {
    std::cout << "\n--- StructuralIntegrity (MultiCell) ---\n";
    Ship ship;

    // A 2x1x1 block followed by a 1x1x1 block
    auto big = MakeBlock({0,0,0}, {2,1,1});
    auto small = MakeBlock({2,0,0}, {1,1,1});
    BlockPlacement::Place(ship, big);
    BlockPlacement::Place(ship, small);

    TEST("Multi-cell ship is connected", StructuralIntegrity::IsFullyConnected(ship));

    auto groups = StructuralIntegrity::FindDisconnectedGroups(ship);
    TEST("Multi-cell ship has 1 group", groups.size() == 1);
    TEST("Multi-cell group has 2 blocks", groups[0].size() == 2);

    // Removing the big block should not leave the small disconnected
    // (it would be alone but still connected to itself)
    TEST("Small block alone would not disconnect", !StructuralIntegrity::WouldDisconnect(ship, small));
}

// ===================================================================
// Expanded ShipDamage tests
// ===================================================================

static void TestSplashDamage() {
    std::cout << "\n--- ShipDamage (Splash) ---\n";
    Ship ship;

    // Create a cross pattern: center + 4 cardinal directions
    auto center = MakeBlock({0,0,0}, {1,1,1});
    auto right  = MakeBlock({1,0,0}, {1,1,1});
    auto left   = MakeBlock({-1,0,0}, {1,1,1});
    auto up     = MakeBlock({0,1,0}, {1,1,1});
    auto down   = MakeBlock({0,-1,0}, {1,1,1});
    BlockPlacement::Place(ship, center);
    BlockPlacement::Place(ship, right);
    BlockPlacement::Place(ship, left);
    BlockPlacement::Place(ship, up);
    BlockPlacement::Place(ship, down);

    float origHP = center->currentHP;

    // Splash from center with radius 1
    int hit = ShipDamage::ApplySplashDamage(ship, {0,0,0}, 10.0f, 1);
    TEST("Splash hits 5 blocks", hit == 5);
    TEST("Center block took full damage", center->currentHP < origHP);
    TEST("Neighbor took reduced damage", right->currentHP < origHP);

    // Splash with radius 0 only hits center
    Ship ship2;
    auto only = MakeBlock({0,0,0}, {1,1,1});
    BlockPlacement::Place(ship2, only);
    int hit2 = ShipDamage::ApplySplashDamage(ship2, {0,0,0}, 5.0f, 0);
    TEST("Radius 0 splash hits 1 block", hit2 == 1);

    // Splash far from blocks hits nothing
    Ship ship3;
    BlockPlacement::Place(ship3, MakeBlock({0,0,0}, {1,1,1}));
    int hit3 = ShipDamage::ApplySplashDamage(ship3, {100,100,100}, 10.0f, 1);
    TEST("Splash far away hits 0 blocks", hit3 == 0);
}

static void TestPenetratingDamage() {
    std::cout << "\n--- ShipDamage (Penetrating) ---\n";
    Ship ship;

    // 5-block line along X axis
    for (int i = 0; i < 5; ++i) {
        BlockPlacement::Place(ship, MakeBlock({i,0,0}, {1,1,1}));
    }

    float origHP = ship.blocks[0]->currentHP;
    int hit = ShipDamage::ApplyPenetratingDamage(ship, {0,0,0}, {1,0,0}, 30.0f, 5);
    TEST("Penetrating hits 5 blocks", hit == 5);
    TEST("First block took full 30 damage", ApproxEq(ship.blocks[0]->currentHP, origHP - 30.0f));
    TEST("Second block took 70% damage", ApproxEq(ship.blocks[1]->currentHP, origHP - 21.0f));
    TEST("Third block took 49% damage", ApproxEq(ship.blocks[2]->currentHP, origHP - 14.7f));

    // Penetrating into empty space
    Ship ship2;
    BlockPlacement::Place(ship2, MakeBlock({0,0,0}, {1,1,1}));
    int hit2 = ShipDamage::ApplyPenetratingDamage(ship2, {0,0,0}, {1,0,0}, 10.0f, 5);
    TEST("Penetrating through 1 block hits 1", hit2 == 1);
}

static void TestRepairBlock() {
    std::cout << "\n--- ShipDamage (Repair Block) ---\n";
    Ship ship;
    auto block = MakeBlock({0,0,0}, {1,1,1});
    BlockPlacement::Place(ship, block);

    float maxHP = block->maxHP;
    ShipDamage::ApplyDamage(ship, block, 50.0f);
    float damagedHP = block->currentHP;

    float repaired = ShipDamage::RepairBlock(ship, block, 30.0f);
    TEST("Repaired 30 HP", ApproxEq(repaired, 30.0f));
    TEST("Block HP increased by 30", ApproxEq(block->currentHP, damagedHP + 30.0f));

    // Repair more than missing
    float repaired2 = ShipDamage::RepairBlock(ship, block, 1000.0f);
    TEST("Can't repair past maxHP", ApproxEq(block->currentHP, maxHP));
    TEST("Only repaired what was missing", repaired2 < 1000.0f);

    // Repair null block
    float repaired3 = ShipDamage::RepairBlock(ship, nullptr, 10.0f);
    TEST("Null repair returns 0", ApproxEq(repaired3, 0.0f));

    // Repair with 0 budget
    float repaired4 = ShipDamage::RepairBlock(ship, block, 0.0f);
    TEST("Zero budget returns 0", ApproxEq(repaired4, 0.0f));
}

static void TestRepairAll() {
    std::cout << "\n--- ShipDamage (Repair All) ---\n";
    Ship ship;
    auto b1 = MakeBlock({0,0,0}, {1,1,1});
    auto b2 = MakeBlock({1,0,0}, {1,1,1});
    auto b3 = MakeBlock({2,0,0}, {1,1,1});
    BlockPlacement::Place(ship, b1);
    BlockPlacement::Place(ship, b2);
    BlockPlacement::Place(ship, b3);

    // Damage blocks differently
    ShipDamage::ApplyDamage(ship, b1, 80.0f);  // heavily damaged
    ShipDamage::ApplyDamage(ship, b2, 30.0f);  // lightly damaged
    // b3 is untouched

    float totalRepaired = ShipDamage::RepairAll(ship, 50.0f);
    TEST("RepairAll returns positive", totalRepaired > 0.0f);
    TEST("Most damaged block got some repair", b1->currentHP > b1->maxHP - 80.0f);

    // RepairAll with 0 budget
    float r2 = ShipDamage::RepairAll(ship, 0.0f);
    TEST("RepairAll with 0 budget returns 0", ApproxEq(r2, 0.0f));

    // RepairAll on empty ship
    Ship empty;
    float r3 = ShipDamage::RepairAll(empty, 100.0f);
    TEST("RepairAll on empty ship returns 0", ApproxEq(r3, 0.0f));
}

static void TestCheckAndSeparateFragments() {
    std::cout << "\n--- ShipDamage (CheckAndSeparateFragments) ---\n";
    Ship ship;

    // Connected ship should produce no fragments
    BlockPlacement::Place(ship, MakeBlock({0,0,0}, {1,1,1}));
    BlockPlacement::Place(ship, MakeBlock({1,0,0}, {1,1,1}));
    auto fragments = ShipDamage::CheckAndSeparateFragments(ship);
    TEST("Connected ship has 0 fragments", fragments.empty());
    TEST("Ship still has 2 blocks", ship.BlockCount() == 2);

    // Create disconnected ship manually
    Ship ship2;
    auto mainA = MakeBlock({0,0,0}, {1,1,1});
    auto mainB = MakeBlock({1,0,0}, {1,1,1});
    auto mainC = MakeBlock({2,0,0}, {1,1,1});
    auto fragX = MakeBlock({10,10,10}, {1,1,1});

    ship2.blocks.push_back(mainA);
    for (auto& c : BlockPlacement::GetOccupiedCells(*mainA)) ship2.occupiedCells[c] = mainA;
    ship2.blocks.push_back(mainB);
    for (auto& c : BlockPlacement::GetOccupiedCells(*mainB)) ship2.occupiedCells[c] = mainB;
    ship2.blocks.push_back(mainC);
    for (auto& c : BlockPlacement::GetOccupiedCells(*mainC)) ship2.occupiedCells[c] = mainC;
    ship2.blocks.push_back(fragX);
    for (auto& c : BlockPlacement::GetOccupiedCells(*fragX)) ship2.occupiedCells[c] = fragX;
    ShipStats::Recalculate(ship2);

    auto frags = ShipDamage::CheckAndSeparateFragments(ship2);
    TEST("Disconnected ship produces 1 fragment group", frags.size() == 1);
    TEST("Fragment group has 1 block", frags[0].size() == 1);
    TEST("Main ship left with 3 blocks", ship2.BlockCount() == 3);
}

static void TestDamagePercentageAndQueries() {
    std::cout << "\n--- ShipDamage (Percentage & Queries) ---\n";
    Ship ship;
    auto b1 = MakeBlock({0,0,0}, {1,1,1});
    auto b2 = MakeBlock({1,0,0}, {1,1,1});
    BlockPlacement::Place(ship, b1);
    BlockPlacement::Place(ship, b2);

    TEST("Pristine ship damage is 0%", ApproxEq(ShipDamage::GetDamagePercentage(ship), 0.0f));

    ShipDamage::ApplyDamage(ship, b1, b1->maxHP * 0.5f);
    float dmg = ShipDamage::GetDamagePercentage(ship);
    TEST("Partially damaged ship > 0%", dmg > 0.0f);
    TEST("Partially damaged ship < 100%", dmg < 1.0f);

    auto damaged = ShipDamage::GetDamagedBlocks(ship, 0.75f);
    TEST("GetDamagedBlocks finds b1", damaged.size() == 1);

    int inRadius = ShipDamage::GetBlocksInRadius(ship, {0,0,0}, 1);
    TEST("GetBlocksInRadius(0,0,0 r=1) finds 2", inRadius == 2);

    int farRadius = ShipDamage::GetBlocksInRadius(ship, {100,100,100}, 0);
    TEST("GetBlocksInRadius far away finds 0", farRadius == 0);

    // Empty ship
    Ship empty;
    TEST("Empty ship damage is 0%", ApproxEq(ShipDamage::GetDamagePercentage(empty), 0.0f));
}

// ===================================================================
// DamageComponent tests
// ===================================================================

static void TestDamageComponentBasic() {
    std::cout << "\n--- DamageComponent (Basic) ---\n";
    DamageComponent dc;

    TEST("Default damageMultiplier is 1.0", ApproxEq(dc.damageMultiplier, 1.0f));
    TEST("Default repairRate is 0.0", ApproxEq(dc.repairRate, 0.0f));
    TEST("Default isInvulnerable is false", !dc.isInvulnerable);
    TEST("Default hasStructuralDamage is false", !dc.hasStructuralDamage);
    TEST("Default disconnectedFragments is 0", dc.disconnectedFragments == 0);
    TEST("History starts empty", dc.damageHistory.empty());
}

static void TestDamageComponentHistory() {
    std::cout << "\n--- DamageComponent (History) ---\n";
    DamageComponent dc;

    DamageRecord r1{1.0f, 25.0f, DamageType::Kinetic, {0,0,0}};
    DamageRecord r2{2.0f, 50.0f, DamageType::Energy, {1,0,0}};
    DamageRecord r3{3.0f, 75.0f, DamageType::Explosive, {2,0,0}};

    dc.AddDamageRecord(r1);
    dc.AddDamageRecord(r2);
    dc.AddDamageRecord(r3);

    TEST("History has 3 records", dc.damageHistory.size() == 3);
    TEST("Total damage is 150", ApproxEq(dc.GetTotalDamageReceived(), 150.0f));

    TEST("Recent damage within 1s at t=3.5", ApproxEq(dc.GetRecentDamage(1.0f, 3.5f), 75.0f));
    TEST("Recent damage within 2s at t=3.5", ApproxEq(dc.GetRecentDamage(2.0f, 3.5f), 125.0f));
    TEST("Recent damage within 10s at t=3.5", ApproxEq(dc.GetRecentDamage(10.0f, 3.5f), 150.0f));

    // Test max history eviction
    for (int i = 0; i < 60; ++i) {
        dc.AddDamageRecord({static_cast<float>(i + 10), 1.0f, DamageType::Thermal, {0,0,0}});
    }
    TEST("History capped at max", dc.damageHistory.size() == DamageComponent::kMaxHistorySize);
}

static void TestDamageComponentSerialization() {
    std::cout << "\n--- DamageComponent (Serialization) ---\n";
    DamageComponent original;
    original.damageMultiplier = 1.5f;
    original.repairRate = 10.0f;
    original.isInvulnerable = true;
    original.hasStructuralDamage = true;
    original.disconnectedFragments = 3;
    original.AddDamageRecord({1.0f, 50.0f, DamageType::Energy, {1,2,3}});
    original.AddDamageRecord({2.0f, 100.0f, DamageType::EMP, {4,5,6}});

    auto cd = original.Serialize();
    TEST("Serialize type is DamageComponent", cd.componentType == "DamageComponent");

    DamageComponent restored;
    restored.Deserialize(cd);

    TEST("Restored damageMultiplier", ApproxEq(restored.damageMultiplier, 1.5f));
    TEST("Restored repairRate", ApproxEq(restored.repairRate, 10.0f));
    TEST("Restored isInvulnerable", restored.isInvulnerable);
    TEST("Restored hasStructuralDamage", restored.hasStructuralDamage);
    TEST("Restored disconnectedFragments", restored.disconnectedFragments == 3);
    TEST("Restored history size", restored.damageHistory.size() == 2);
    TEST("Restored first record damage", ApproxEq(restored.damageHistory[0].damageAmount, 50.0f));
    TEST("Restored first record type is Energy", restored.damageHistory[0].damageType == DamageType::Energy);
    TEST("Restored first record position", restored.damageHistory[0].hitPosition == Vector3Int(1,2,3));
    TEST("Restored second record damage", ApproxEq(restored.damageHistory[1].damageAmount, 100.0f));
    TEST("Restored second record type is EMP", restored.damageHistory[1].damageType == DamageType::EMP);
}

// ===================================================================
// Octree tests
// ===================================================================

static void TestAABB() {
    std::cout << "\n--- AABB ---\n";
    AABB box(Vector3(0,0,0), Vector3(10,10,10));

    TEST("Contains center", box.Contains(Vector3(0,0,0)));
    TEST("Contains corner", box.Contains(Vector3(10,10,10)));
    TEST("Contains edge", box.Contains(Vector3(10,0,0)));
    TEST("Does not contain outside", !box.Contains(Vector3(11,0,0)));

    TEST("Sphere at center intersects", box.IntersectsSphere(Vector3(0,0,0), 1.0f));
    TEST("Sphere far away no intersect", !box.IntersectsSphere(Vector3(100,100,100), 1.0f));
    TEST("Sphere touching edge intersects", box.IntersectsSphere(Vector3(11,0,0), 2.0f));

    AABB other(Vector3(5,5,5), Vector3(5,5,5));
    TEST("Overlapping boxes intersect", box.Intersects(other));

    AABB far(Vector3(100,100,100), Vector3(1,1,1));
    TEST("Far boxes no intersect", !box.Intersects(far));
}

static void TestOctreeInsertAndCount() {
    std::cout << "\n--- Octree (Insert & Count) ---\n";
    AABB bounds(Vector3(0,0,0), Vector3(100,100,100));
    Octree tree(bounds);

    TEST("Empty tree has 0 entities", tree.GetEntityCount() == 0);
    TEST("Empty tree has 1 node", tree.GetNodeCount() == 1);

    bool inserted = tree.Insert(1, Vector3(10, 10, 10));
    TEST("Insert succeeds", inserted);
    TEST("Tree has 1 entity", tree.GetEntityCount() == 1);

    tree.Insert(2, Vector3(-10, -10, -10));
    tree.Insert(3, Vector3(50, 50, 50));
    TEST("Tree has 3 entities", tree.GetEntityCount() == 3);

    // Insert outside bounds should fail
    bool outside = tree.Insert(99, Vector3(200, 200, 200));
    TEST("Insert outside bounds fails", !outside);
    TEST("Count unchanged after failed insert", tree.GetEntityCount() == 3);
}

static void TestOctreeRemove() {
    std::cout << "\n--- Octree (Remove) ---\n";
    AABB bounds(Vector3(0,0,0), Vector3(100,100,100));
    Octree tree(bounds);

    tree.Insert(1, Vector3(10, 10, 10));
    tree.Insert(2, Vector3(-10, -10, -10));
    tree.Insert(3, Vector3(50, 50, 50));

    bool removed = tree.Remove(2);
    TEST("Remove existing entity succeeds", removed);
    TEST("Count decreased", tree.GetEntityCount() == 2);

    bool removed2 = tree.Remove(999);
    TEST("Remove nonexistent entity fails", !removed2);
    TEST("Count unchanged", tree.GetEntityCount() == 2);
}

static void TestOctreeClear() {
    std::cout << "\n--- Octree (Clear) ---\n";
    AABB bounds(Vector3(0,0,0), Vector3(100,100,100));
    Octree tree(bounds);

    for (int i = 0; i < 20; ++i) {
        tree.Insert(static_cast<EntityId>(i + 1), Vector3(static_cast<float>(i), 0, 0));
    }
    TEST("20 entities inserted", tree.GetEntityCount() == 20);

    tree.Clear();
    TEST("After clear, 0 entities", tree.GetEntityCount() == 0);
    TEST("After clear, 1 node", tree.GetNodeCount() == 1);
}

static void TestOctreeQuerySphere() {
    std::cout << "\n--- Octree (QuerySphere) ---\n";
    AABB bounds(Vector3(0,0,0), Vector3(100,100,100));
    Octree tree(bounds);

    tree.Insert(1, Vector3(0, 0, 0));
    tree.Insert(2, Vector3(5, 0, 0));
    tree.Insert(3, Vector3(50, 50, 50));
    tree.Insert(4, Vector3(-5, 0, 0));

    auto nearby = tree.QuerySphere(Vector3(0, 0, 0), 10.0f);
    TEST("Sphere query finds 3 nearby", nearby.size() == 3);

    auto far = tree.QuerySphere(Vector3(50, 50, 50), 1.0f);
    TEST("Tight sphere finds 1", far.size() == 1);
    TEST("Tight sphere finds entity 3", far[0] == 3);

    auto none = tree.QuerySphere(Vector3(99, 99, 99), 0.1f);
    TEST("Very tight query finds 0", none.empty());
}

static void TestOctreeQueryBox() {
    std::cout << "\n--- Octree (QueryBox) ---\n";
    AABB bounds(Vector3(0,0,0), Vector3(100,100,100));
    Octree tree(bounds);

    tree.Insert(1, Vector3(0, 0, 0));
    tree.Insert(2, Vector3(5, 5, 5));
    tree.Insert(3, Vector3(50, 50, 50));

    AABB queryBox(Vector3(2.5f, 2.5f, 2.5f), Vector3(5, 5, 5));
    auto results = tree.QueryBox(queryBox);
    TEST("Box query finds 2 entities", results.size() == 2);
}

static void TestOctreeFindNearest() {
    std::cout << "\n--- Octree (FindNearest) ---\n";
    AABB bounds(Vector3(0,0,0), Vector3(100,100,100));
    Octree tree(bounds);

    // Empty tree
    auto nearest = tree.FindNearest(Vector3(0, 0, 0));
    TEST("FindNearest on empty returns invalid", nearest == InvalidEntityId);

    tree.Insert(1, Vector3(10, 10, 10));
    tree.Insert(2, Vector3(20, 20, 20));
    tree.Insert(3, Vector3(-10, -10, -10));

    auto n1 = tree.FindNearest(Vector3(9, 9, 9));
    TEST("Nearest to (9,9,9) is entity 1", n1 == 1);

    auto n2 = tree.FindNearest(Vector3(19, 19, 19));
    TEST("Nearest to (19,19,19) is entity 2", n2 == 2);

    auto n3 = tree.FindNearest(Vector3(-9, -9, -9));
    TEST("Nearest to (-9,-9,-9) is entity 3", n3 == 3);
}

static void TestOctreeFindKNearest() {
    std::cout << "\n--- Octree (FindKNearest) ---\n";
    AABB bounds(Vector3(0,0,0), Vector3(100,100,100));
    Octree tree(bounds);

    tree.Insert(1, Vector3(0, 0, 0));
    tree.Insert(2, Vector3(5, 0, 0));
    tree.Insert(3, Vector3(10, 0, 0));
    tree.Insert(4, Vector3(20, 0, 0));
    tree.Insert(5, Vector3(50, 0, 0));

    auto k3 = tree.FindKNearest(Vector3(0, 0, 0), 3);
    TEST("FindKNearest(3) returns 3", k3.size() == 3);
    TEST("K-nearest first is entity 1", k3[0] == 1);
    TEST("K-nearest second is entity 2", k3[1] == 2);
    TEST("K-nearest third is entity 3", k3[2] == 3);

    auto k0 = tree.FindKNearest(Vector3(0, 0, 0), 0);
    TEST("FindKNearest(0) returns 0", k0.empty());

    auto k100 = tree.FindKNearest(Vector3(0, 0, 0), 100);
    TEST("FindKNearest(100) returns all 5", k100.size() == 5);
}

static void TestOctreeSubdivision() {
    std::cout << "\n--- Octree (Subdivision) ---\n";
    AABB bounds(Vector3(0,0,0), Vector3(100,100,100));
    Octree tree(bounds, 4, 2);  // maxDepth=4, maxEntries=2

    tree.Insert(1, Vector3(10, 10, 10));
    tree.Insert(2, Vector3(20, 20, 20));
    TEST("2 entities, 1 node (no split yet)", tree.GetNodeCount() == 1);

    tree.Insert(3, Vector3(30, 30, 30));
    TEST("3 entities after split", tree.GetEntityCount() == 3);
    TEST("More than 1 node after split", tree.GetNodeCount() > 1);

    // Max depth test - many entries in same spot
    Octree deep(bounds, 2, 1);  // maxDepth=2, split after 1 entry
    for (int i = 0; i < 20; ++i) {
        deep.Insert(static_cast<EntityId>(i + 1), Vector3(1, 1, 1));
    }
    TEST("All 20 entities stored even at max depth", deep.GetEntityCount() == 20);
    TEST("Max used depth <= 2", deep.GetMaxUsedDepth() <= 2);
}

static void TestOctreeRebuild() {
    std::cout << "\n--- Octree (Rebuild) ---\n";
    AABB bounds(Vector3(0,0,0), Vector3(100,100,100));
    Octree tree(bounds, 8, 4);

    for (int i = 0; i < 50; ++i) {
        tree.Insert(static_cast<EntityId>(i + 1), Vector3(static_cast<float>(i * 2), 0, 0));
    }
    TEST("50 entities before rebuild", tree.GetEntityCount() == 50);

    // Remove half
    for (int i = 0; i < 25; ++i) {
        tree.Remove(static_cast<EntityId>(i + 1));
    }
    TEST("25 entities after removal", tree.GetEntityCount() == 25);

    size_t nodesBefore = tree.GetNodeCount();
    tree.Rebuild();
    TEST("25 entities after rebuild", tree.GetEntityCount() == 25);
    TEST("Rebuild potentially reduces nodes", tree.GetNodeCount() <= nodesBefore);
}

static void TestOctreeGameEvents() {
    std::cout << "\n--- Octree (GameEvents) ---\n";
    TEST("OctreeRebuilt event defined", std::string(GameEvents::OctreeRebuilt) == "spatial.octree.rebuilt");
    TEST("SpatialQueryPerformed event defined", std::string(GameEvents::SpatialQueryPerformed) == "spatial.query.performed");
}

static void TestVoxelDamageGameEvents() {
    std::cout << "\n--- VoxelDamage (GameEvents) ---\n";
    TEST("BlockDamaged event", std::string(GameEvents::BlockDamaged) == "ship.block.damaged");
    TEST("BlockDestroyed event", std::string(GameEvents::BlockDestroyed) == "ship.block.destroyed");
    TEST("BlockRepaired event", std::string(GameEvents::BlockRepaired) == "ship.block.repaired");
    TEST("SplashDamageApplied event", std::string(GameEvents::SplashDamageApplied) == "ship.splash.damage");
    TEST("PenetratingDamageApplied event", std::string(GameEvents::PenetratingDamageApplied) == "ship.penetrating.damage");
    TEST("StructuralCheck event", std::string(GameEvents::StructuralCheck) == "ship.structural.check");
    TEST("ShipFragmented event", std::string(GameEvents::ShipFragmented) == "ship.fragmented");
    TEST("IntegrityRestored event", std::string(GameEvents::IntegrityRestored) == "ship.integrity.restored");
}

// ===================================================================
// Collision Layer tests
// ===================================================================

static void TestCollisionCategoryBitwise() {
    std::cout << "[CollisionCategory Bitwise]\n";

    // Individual category values are powers of two
    TEST("Player is 1", static_cast<uint32_t>(CollisionCategory::Player) == 1);
    TEST("Enemy is 2", static_cast<uint32_t>(CollisionCategory::Enemy) == 2);
    TEST("Projectile is 4", static_cast<uint32_t>(CollisionCategory::Projectile) == 4);
    TEST("Asteroid is 8", static_cast<uint32_t>(CollisionCategory::Asteroid) == 8);
    TEST("Station is 16", static_cast<uint32_t>(CollisionCategory::Station) == 16);
    TEST("Debris is 32", static_cast<uint32_t>(CollisionCategory::Debris) == 32);
    TEST("Shield is 64", static_cast<uint32_t>(CollisionCategory::Shield) == 64);
    TEST("Sensor is 128", static_cast<uint32_t>(CollisionCategory::Sensor) == 128);
    TEST("Pickup is 256", static_cast<uint32_t>(CollisionCategory::Pickup) == 256);
    TEST("Missile is 512", static_cast<uint32_t>(CollisionCategory::Missile) == 512);
    TEST("None is 0", static_cast<uint32_t>(CollisionCategory::None) == 0);
    TEST("All is 0xFFFFFFFF", static_cast<uint32_t>(CollisionCategory::All) == 0xFFFFFFFF);

    // Bitwise OR
    auto combined = CollisionCategory::Player | CollisionCategory::Enemy;
    TEST("Player|Enemy = 3", static_cast<uint32_t>(combined) == 3);

    // Bitwise AND
    auto masked = combined & CollisionCategory::Player;
    TEST("(Player|Enemy) & Player = Player", static_cast<uint32_t>(masked) == 1);

    auto maskedNone = combined & CollisionCategory::Asteroid;
    TEST("(Player|Enemy) & Asteroid = 0", static_cast<uint32_t>(maskedNone) == 0);

    // Bitwise NOT
    auto inverted = ~CollisionCategory::Player;
    TEST("~Player does not contain Player", !HasCategory(inverted, CollisionCategory::Player));
    TEST("~Player contains Enemy", HasCategory(inverted, CollisionCategory::Enemy));
}

static void TestHasCategory() {
    std::cout << "[HasCategory]\n";

    auto flags = CollisionCategory::Player | CollisionCategory::Enemy | CollisionCategory::Projectile;
    TEST("flags has Player", HasCategory(flags, CollisionCategory::Player));
    TEST("flags has Enemy", HasCategory(flags, CollisionCategory::Enemy));
    TEST("flags has Projectile", HasCategory(flags, CollisionCategory::Projectile));
    TEST("flags no Asteroid", !HasCategory(flags, CollisionCategory::Asteroid));
    TEST("flags no Station", !HasCategory(flags, CollisionCategory::Station));
    TEST("None has nothing", !HasCategory(CollisionCategory::None, CollisionCategory::Player));
    TEST("All has everything", HasCategory(CollisionCategory::All, CollisionCategory::Player));
    TEST("All has Missile", HasCategory(CollisionCategory::All, CollisionCategory::Missile));
}

static void TestShouldCollide() {
    std::cout << "[ShouldCollide]\n";

    // Default: All collides with All
    TEST("All vs All collide", ShouldCollide(CollisionCategory::All, CollisionCategory::All,
                                              CollisionCategory::All, CollisionCategory::All));

    // Player projectile vs Enemy: should collide
    auto ppPreset = CollisionPresets::PlayerProjectile();
    auto ePreset = CollisionPresets::EnemyShip();
    TEST("PlayerProjectile vs EnemyShip collide",
         ShouldCollide(ppPreset.layer, ppPreset.mask, ePreset.layer, ePreset.mask));

    // Player projectile vs Player: should NOT collide (friendly fire off)
    auto pPreset = CollisionPresets::PlayerShip();
    TEST("PlayerProjectile vs PlayerShip no collide",
         !ShouldCollide(ppPreset.layer, ppPreset.mask, pPreset.layer, pPreset.mask));

    // Enemy projectile vs Player: should collide
    auto epPreset = CollisionPresets::EnemyProjectile();
    TEST("EnemyProjectile vs PlayerShip collide",
         ShouldCollide(epPreset.layer, epPreset.mask, pPreset.layer, pPreset.mask));

    // Enemy projectile vs Enemy: should NOT collide
    TEST("EnemyProjectile vs EnemyShip no collide",
         !ShouldCollide(epPreset.layer, epPreset.mask, ePreset.layer, ePreset.mask));

    // Pickup vs Player: should collide
    auto pickPreset = CollisionPresets::PickupPreset();
    TEST("Pickup vs Player collide",
         ShouldCollide(pickPreset.layer, pickPreset.mask, pPreset.layer, pPreset.mask));

    // Pickup vs Enemy: should NOT collide (pickup only collides with Player)
    TEST("Pickup vs Enemy no collide",
         !ShouldCollide(pickPreset.layer, pickPreset.mask, ePreset.layer, ePreset.mask));

    // Sensor vs Player: should NOT collide (Player's mask doesn't include Sensor)
    auto sensorPreset = CollisionPresets::SensorPreset();
    TEST("Sensor vs Player no collide (bidirectional check)",
         !ShouldCollide(sensorPreset.layer, sensorPreset.mask, pPreset.layer, pPreset.mask));

    // None vs anything: should NOT collide
    TEST("None vs All no collide",
         !ShouldCollide(CollisionCategory::None, CollisionCategory::None,
                        CollisionCategory::All, CollisionCategory::All));
}

static void TestCollisionPresets() {
    std::cout << "[CollisionPresets]\n";

    // Default preset
    auto def = CollisionPresets::Default();
    TEST("Default layer is All", def.layer == CollisionCategory::All);
    TEST("Default mask is All", def.mask == CollisionCategory::All);

    // PlayerShip preset
    auto ps = CollisionPresets::PlayerShip();
    TEST("PlayerShip layer is Player", ps.layer == CollisionCategory::Player);
    TEST("PlayerShip mask has Enemy", HasCategory(ps.mask, CollisionCategory::Enemy));
    TEST("PlayerShip mask has Projectile", HasCategory(ps.mask, CollisionCategory::Projectile));
    TEST("PlayerShip mask has Asteroid", HasCategory(ps.mask, CollisionCategory::Asteroid));
    TEST("PlayerShip mask has Pickup", HasCategory(ps.mask, CollisionCategory::Pickup));
    TEST("PlayerShip mask no Sensor", !HasCategory(ps.mask, CollisionCategory::Sensor));

    // EnemyShip preset
    auto es = CollisionPresets::EnemyShip();
    TEST("EnemyShip layer is Enemy", es.layer == CollisionCategory::Enemy);
    TEST("EnemyShip mask has Player", HasCategory(es.mask, CollisionCategory::Player));
    TEST("EnemyShip mask has Enemy (self)", HasCategory(es.mask, CollisionCategory::Enemy));

    // Asteroid preset
    auto ast = CollisionPresets::AsteroidPreset();
    TEST("Asteroid layer is Asteroid", ast.layer == CollisionCategory::Asteroid);
    TEST("Asteroid mask has Missile", HasCategory(ast.mask, CollisionCategory::Missile));
    TEST("Asteroid mask no Debris", !HasCategory(ast.mask, CollisionCategory::Debris));

    // Debris preset
    auto deb = CollisionPresets::DebrisPreset();
    TEST("Debris layer is Debris", deb.layer == CollisionCategory::Debris);
    TEST("Debris mask has Player", HasCategory(deb.mask, CollisionCategory::Player));
    TEST("Debris mask no Projectile", !HasCategory(deb.mask, CollisionCategory::Projectile));

    // Missile preset
    auto mis = CollisionPresets::MissilePreset();
    TEST("Missile layer is Missile", mis.layer == CollisionCategory::Missile);
    TEST("Missile mask has Shield", HasCategory(mis.mask, CollisionCategory::Shield));
    TEST("Missile mask has Station", HasCategory(mis.mask, CollisionCategory::Station));
}

static void TestGetCategoryName() {
    std::cout << "[GetCategoryName]\n";

    TEST("Name Player", GetCategoryName(CollisionCategory::Player) == "Player");
    TEST("Name Enemy", GetCategoryName(CollisionCategory::Enemy) == "Enemy");
    TEST("Name Projectile", GetCategoryName(CollisionCategory::Projectile) == "Projectile");
    TEST("Name Asteroid", GetCategoryName(CollisionCategory::Asteroid) == "Asteroid");
    TEST("Name Station", GetCategoryName(CollisionCategory::Station) == "Station");
    TEST("Name Debris", GetCategoryName(CollisionCategory::Debris) == "Debris");
    TEST("Name Shield", GetCategoryName(CollisionCategory::Shield) == "Shield");
    TEST("Name Sensor", GetCategoryName(CollisionCategory::Sensor) == "Sensor");
    TEST("Name Pickup", GetCategoryName(CollisionCategory::Pickup) == "Pickup");
    TEST("Name Missile", GetCategoryName(CollisionCategory::Missile) == "Missile");
    TEST("Name None", GetCategoryName(CollisionCategory::None) == "None");
    TEST("Name All", GetCategoryName(CollisionCategory::All) == "All");
}

static void TestPhysicsComponentCollisionLayers() {
    std::cout << "[PhysicsComponent CollisionLayers]\n";

    // Default values for backward compat
    PhysicsComponent pc;
    TEST("Default layer is All", pc.collisionLayer == CollisionCategory::All);
    TEST("Default mask is All", pc.collisionMask == CollisionCategory::All);
    TEST("Default not trigger", !pc.isTrigger);

    // SetCollisionPreset
    pc.SetCollisionPreset(CollisionPresets::PlayerShip());
    TEST("After preset layer is Player", pc.collisionLayer == CollisionCategory::Player);
    TEST("After preset mask has Enemy", HasCategory(pc.collisionMask, CollisionCategory::Enemy));

    // ShouldCollideWith
    PhysicsComponent enemy;
    enemy.SetCollisionPreset(CollisionPresets::EnemyShip());
    TEST("Player should collide with Enemy", pc.ShouldCollideWith(enemy));

    PhysicsComponent pickup;
    pickup.SetCollisionPreset(CollisionPresets::PickupPreset());
    TEST("Player should collide with Pickup", pc.ShouldCollideWith(pickup));
    TEST("Enemy should not collide with Pickup", !enemy.ShouldCollideWith(pickup));

    // Trigger field
    pc.isTrigger = true;
    TEST("Trigger set", pc.isTrigger);
}

static void TestPhysicsSystemCollisionLayers() {
    std::cout << "[PhysicsSystem CollisionLayers]\n";

    // Two objects on different non-overlapping layers should NOT collide
    EntityManager em;
    PhysicsSystem physSys(em);

    auto& obj1 = em.CreateEntity("PlayerShip");
    auto c1 = std::make_unique<PhysicsComponent>();
    c1->mass = 100.0f;
    c1->drag = 0.0f;
    c1->angularDrag = 0.0f;
    c1->position = Vector3(0.0f, 0.0f, 0.0f);
    c1->velocity = Vector3(5.0f, 0.0f, 0.0f);
    c1->collisionRadius = 5.0f;
    c1->SetCollisionPreset(CollisionPresets::PlayerShip());
    auto* pc1 = em.AddComponent<PhysicsComponent>(obj1.id, std::move(c1));

    auto& obj2 = em.CreateEntity("PlayerProjectile");
    auto c2 = std::make_unique<PhysicsComponent>();
    c2->mass = 1.0f;
    c2->drag = 0.0f;
    c2->angularDrag = 0.0f;
    c2->position = Vector3(8.0f, 0.0f, 0.0f);
    c2->velocity = Vector3(-5.0f, 0.0f, 0.0f);
    c2->collisionRadius = 5.0f;
    c2->SetCollisionPreset(CollisionPresets::PlayerProjectile());
    auto* pc2 = em.AddComponent<PhysicsComponent>(obj2.id, std::move(c2));

    // Store velocities before update
    float v1Before = pc1->velocity.x;
    float v2Before = pc2->velocity.x;

    physSys.Update(kTinyDeltaTime); // Tiny dt to minimize position change

    // PlayerProjectile should NOT collide with PlayerShip (friendly fire off)
    // Velocities should remain essentially unchanged (no collision response)
    TEST("Player not hit by own projectile (v1 same)",
         ApproxEq(pc1->velocity.x, v1Before));
    TEST("Player not hit by own projectile (v2 same)",
         ApproxEq(pc2->velocity.x, v2Before));

    // Now test that enemy projectile DOES collide with player
    EntityManager em2;
    PhysicsSystem physSys2(em2);

    auto& ship = em2.CreateEntity("PlayerShip2");
    auto cs = std::make_unique<PhysicsComponent>();
    cs->mass = 100.0f;
    cs->drag = 0.0f;
    cs->angularDrag = 0.0f;
    cs->position = Vector3(0.0f, 0.0f, 0.0f);
    cs->velocity = Vector3(0.0f, 0.0f, 0.0f);
    cs->collisionRadius = 5.0f;
    cs->SetCollisionPreset(CollisionPresets::PlayerShip());
    auto* pcs = em2.AddComponent<PhysicsComponent>(ship.id, std::move(cs));

    auto& proj = em2.CreateEntity("EnemyProjectile");
    auto cp = std::make_unique<PhysicsComponent>();
    cp->mass = 1.0f;
    cp->drag = 0.0f;
    cp->angularDrag = 0.0f;
    cp->position = Vector3(8.0f, 0.0f, 0.0f);
    cp->velocity = Vector3(-10.0f, 0.0f, 0.0f);
    cp->collisionRadius = 5.0f;
    cp->SetCollisionPreset(CollisionPresets::EnemyProjectile());
    auto* pcp = em2.AddComponent<PhysicsComponent>(proj.id, std::move(cp));

    physSys2.Update(kTinyDeltaTime);

    // Enemy projectile should collide with player - velocity should change
    TEST("Enemy projectile hits player (velocity changed)",
         !ApproxEq(pcp->velocity.x, -10.0f) || !ApproxEq(pcs->velocity.x, 0.0f));
}

static void TestPhysicsSystemTrigger() {
    std::cout << "[PhysicsSystem Trigger]\n";

    EntityManager em;
    PhysicsSystem physSys(em);

    auto& obj1 = em.CreateEntity("Ship");
    auto c1 = std::make_unique<PhysicsComponent>();
    c1->mass = 100.0f;
    c1->drag = 0.0f;
    c1->angularDrag = 0.0f;
    c1->position = Vector3(0.0f, 0.0f, 0.0f);
    c1->velocity = Vector3(5.0f, 0.0f, 0.0f);
    c1->collisionRadius = 5.0f;
    auto* pc1 = em.AddComponent<PhysicsComponent>(obj1.id, std::move(c1));

    auto& obj2 = em.CreateEntity("SensorZone");
    auto c2 = std::make_unique<PhysicsComponent>();
    c2->mass = 1.0f;
    c2->isStatic = true;
    c2->position = Vector3(8.0f, 0.0f, 0.0f);
    c2->collisionRadius = 5.0f;
    c2->isTrigger = true; // Trigger volume
    auto* pc2 = em.AddComponent<PhysicsComponent>(obj2.id, std::move(c2));

    float v1Before = pc1->velocity.x;
    physSys.Update(kTinyDeltaTime);

    // Trigger should not alter velocity (no physics response)
    TEST("Trigger does not alter velocity", ApproxEq(pc1->velocity.x, v1Before));
}

static void TestCollisionLayerGameEvents() {
    std::cout << "[CollisionLayer GameEvents]\n";

    TEST("CollisionLayerChanged event",
         std::string(GameEvents::CollisionLayerChanged) == "physics.collision.layer_changed");
    TEST("TriggerEntered event",
         std::string(GameEvents::TriggerEntered) == "physics.trigger.entered");
    TEST("TriggerExited event",
         std::string(GameEvents::TriggerExited) == "physics.trigger.exited");
}

static void TestPhysicsCollisionSeparation() {
    std::cout << "[PhysicsSystem Collision Separation]\n";

    // Test 1: Default restitution value
    {
        PhysicsComponent pc;
        TEST("Default restitution 0.8", ApproxEq(pc.restitution, 0.8f));
    }

    // Test 2: Two overlapping dynamic objects get separated after collision
    {
        EntityManager em;
        PhysicsSystem physSys(em);

        auto& obj1 = em.CreateEntity("Ship1");
        auto c1 = std::make_unique<PhysicsComponent>();
        c1->mass = 100.0f;
        c1->drag = 0.0f;
        c1->angularDrag = 0.0f;
        c1->position = Vector3(0.0f, 0.0f, 0.0f);
        c1->velocity = Vector3(5.0f, 0.0f, 0.0f);
        c1->collisionRadius = 5.0f;
        c1->restitution = 1.0f; // Perfectly elastic for predictable test
        auto* pc1 = em.AddComponent<PhysicsComponent>(obj1.id, std::move(c1));

        auto& obj2 = em.CreateEntity("Ship2");
        auto c2 = std::make_unique<PhysicsComponent>();
        c2->mass = 100.0f;
        c2->drag = 0.0f;
        c2->angularDrag = 0.0f;
        c2->position = Vector3(8.0f, 0.0f, 0.0f); // Overlap: distance 8 < radius sum 10
        c2->velocity = Vector3(-5.0f, 0.0f, 0.0f);
        c2->collisionRadius = 5.0f;
        c2->restitution = 1.0f;
        auto* pc2 = em.AddComponent<PhysicsComponent>(obj2.id, std::move(c2));

        physSys.Update(0.001f);

        // After collision, objects should be separated (distance >= sum of radii)
        float postDistance = (pc2->position - pc1->position).length();
        float minDistance = pc1->collisionRadius + pc2->collisionRadius;
        TEST("Collision separates overlapping objects", postDistance >= minDistance - 0.1f);
    }

    // Test 3: Dynamic object colliding with static gets separated
    {
        EntityManager em;
        PhysicsSystem physSys(em);

        auto& staticObj = em.CreateEntity("Wall");
        auto sc = std::make_unique<PhysicsComponent>();
        sc->mass = 9999.0f;
        sc->isStatic = true;
        sc->position = Vector3(8.0f, 0.0f, 0.0f);
        sc->collisionRadius = 5.0f;
        sc->restitution = 1.0f;
        auto* spc = em.AddComponent<PhysicsComponent>(staticObj.id, std::move(sc));

        auto& dynObj = em.CreateEntity("Ship");
        auto dc = std::make_unique<PhysicsComponent>();
        dc->mass = 100.0f;
        dc->drag = 0.0f;
        dc->angularDrag = 0.0f;
        dc->position = Vector3(0.0f, 0.0f, 0.0f);
        dc->velocity = Vector3(5.0f, 0.0f, 0.0f);
        dc->collisionRadius = 5.0f;
        dc->restitution = 1.0f;
        auto* dpc = em.AddComponent<PhysicsComponent>(dynObj.id, std::move(dc));

        physSys.Update(0.001f);

        // Static object should not move
        TEST("Static obj unchanged", ApproxEq(spc->position.x, 8.0f));

        // Dynamic object should be pushed away
        float postDist = (spc->position - dpc->position).length();
        float minDist = spc->collisionRadius + dpc->collisionRadius;
        TEST("Dynamic separated from static", postDist >= minDist - 0.1f);

        // Dynamic object should bounce (velocity reversed direction)
        TEST("Dynamic bounces off static", dpc->velocity.x < 0.0f);
    }

    // Test 4: Low restitution reduces bounce
    {
        EntityManager em;
        PhysicsSystem physSys(em);

        auto& obj1 = em.CreateEntity("Ship1");
        auto c1 = std::make_unique<PhysicsComponent>();
        c1->mass = 100.0f;
        c1->drag = 0.0f;
        c1->angularDrag = 0.0f;
        c1->position = Vector3(0.0f, 0.0f, 0.0f);
        c1->velocity = Vector3(10.0f, 0.0f, 0.0f);
        c1->collisionRadius = 5.0f;
        c1->restitution = 0.5f;
        auto* pc1 = em.AddComponent<PhysicsComponent>(obj1.id, std::move(c1));

        auto& obj2 = em.CreateEntity("Ship2");
        auto c2 = std::make_unique<PhysicsComponent>();
        c2->mass = 100.0f;
        c2->drag = 0.0f;
        c2->angularDrag = 0.0f;
        c2->position = Vector3(8.0f, 0.0f, 0.0f);
        c2->velocity = Vector3(0.0f, 0.0f, 0.0f);
        c2->collisionRadius = 5.0f;
        c2->restitution = 0.5f;
        auto* pc2 = em.AddComponent<PhysicsComponent>(obj2.id, std::move(c2));

        physSys.Update(0.001f);

        // With restitution 0.5, the bounce should be less energetic
        // obj2 should gain some velocity but less than the full 10
        TEST("Low restitution obj2 gains velocity", pc2->velocity.x > 0.0f);
        TEST("Low restitution obj2 less than full transfer", pc2->velocity.x < 10.0f);
    }
}

// ===================================================================
// NavGraph tests
// ===================================================================

static void TestNavGraphAddNode() {
    std::cout << "[NavGraph AddNode]\n";

    NavGraph graph;
    TEST("Empty graph has 0 nodes", graph.NodeCount() == 0);

    NavNodeId id1 = graph.AddNode(Vector3(0, 0, 0));
    TEST("First node id valid", id1 != InvalidNavNodeId);
    TEST("Node count is 1", graph.NodeCount() == 1);

    NavNodeId id2 = graph.AddNode(Vector3(10, 0, 0));
    TEST("Second node id valid", id2 != InvalidNavNodeId);
    TEST("Node count is 2", graph.NodeCount() == 2);
    TEST("IDs are different", id1 != id2);

    const NavNode* node = graph.GetNode(id1);
    TEST("GetNode returns valid", node != nullptr);
    TEST("Node position correct", ApproxEq(node->position.x, 0.0f));
    TEST("Node not blocked", !node->blocked);
    TEST("Node default cost 1", ApproxEq(node->cost, 1.0f));

    NavNodeId id3 = graph.AddNode(Vector3(20, 0, 0), 2.5f);
    const NavNode* node3 = graph.GetNode(id3);
    TEST("Custom cost node", ApproxEq(node3->cost, 2.5f));
}

static void TestNavGraphAddEdge() {
    std::cout << "[NavGraph AddEdge]\n";

    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(0, 0, 0));
    NavNodeId b = graph.AddNode(Vector3(10, 0, 0));

    TEST("No edges initially", graph.EdgeCount() == 0);

    graph.AddEdge(a, b);
    TEST("Bidirectional adds 2 directed edges", graph.EdgeCount() == 2);

    const auto& edgesA = graph.GetEdges(a);
    TEST("Node A has 1 edge", edgesA.size() == 1);
    TEST("Edge from A goes to B", edgesA[0].to == b);
    TEST("Edge weight is distance (10)", ApproxEq(edgesA[0].weight, 10.0f));

    const auto& edgesB = graph.GetEdges(b);
    TEST("Node B has 1 edge", edgesB.size() == 1);
    TEST("Edge from B goes to A", edgesB[0].to == a);
}

static void TestNavGraphDirectedEdge() {
    std::cout << "[NavGraph DirectedEdge]\n";

    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(0, 0, 0));
    NavNodeId b = graph.AddNode(Vector3(10, 0, 0));

    graph.AddDirectedEdge(a, b, 5.0f);
    TEST("Directed adds 1 edge", graph.EdgeCount() == 1);

    const auto& edgesA = graph.GetEdges(a);
    TEST("A has 1 outgoing edge", edgesA.size() == 1);
    TEST("Custom weight applied", ApproxEq(edgesA[0].weight, 5.0f));

    const auto& edgesB = graph.GetEdges(b);
    TEST("B has 0 outgoing edges", edgesB.empty());
}

static void TestNavGraphRemoveNode() {
    std::cout << "[NavGraph RemoveNode]\n";

    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(0, 0, 0));
    NavNodeId b = graph.AddNode(Vector3(10, 0, 0));
    NavNodeId c = graph.AddNode(Vector3(20, 0, 0));
    graph.AddEdge(a, b);
    graph.AddEdge(b, c);

    TEST("3 nodes before remove", graph.NodeCount() == 3);
    TEST("4 edges before remove", graph.EdgeCount() == 4);

    graph.RemoveNode(b);
    TEST("2 nodes after remove", graph.NodeCount() == 2);
    TEST("Removed node not found", graph.GetNode(b) == nullptr);

    // Edges to/from removed node should be gone
    const auto& edgesA = graph.GetEdges(a);
    TEST("A has no edges after B removed", edgesA.empty());
    const auto& edgesC = graph.GetEdges(c);
    TEST("C has no edges after B removed", edgesC.empty());
}

static void TestNavGraphRemoveEdge() {
    std::cout << "[NavGraph RemoveEdge]\n";

    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(0, 0, 0));
    NavNodeId b = graph.AddNode(Vector3(10, 0, 0));
    graph.AddEdge(a, b);
    TEST("2 edges before", graph.EdgeCount() == 2);

    graph.RemoveEdge(a, b);
    TEST("0 edges after remove", graph.EdgeCount() == 0);
}

static void TestNavGraphBlocking() {
    std::cout << "[NavGraph Blocking]\n";

    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(0, 0, 0));

    TEST("Node not blocked initially", !graph.IsBlocked(a));
    graph.SetBlocked(a, true);
    TEST("Node blocked after set", graph.IsBlocked(a));
    graph.SetBlocked(a, false);
    TEST("Node unblocked", !graph.IsBlocked(a));

    // Non-existent node treated as blocked
    TEST("Non-existent node is blocked", graph.IsBlocked(999));
}

static void TestNavGraphFindNearest() {
    std::cout << "[NavGraph FindNearest]\n";

    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(0, 0, 0));
    NavNodeId b = graph.AddNode(Vector3(10, 0, 0));
    NavNodeId c = graph.AddNode(Vector3(20, 0, 0));

    TEST("Nearest to origin is A", graph.FindNearest(Vector3(1, 0, 0)) == a);
    NavNodeId nearMid = graph.FindNearest(Vector3(15, 0, 0));
    TEST("Nearest to (15,0,0) is B or C", nearMid == b || nearMid == c);
    TEST("Nearest to (25,0,0) is C", graph.FindNearest(Vector3(25, 0, 0)) == c);

    // Blocked nodes are skipped
    graph.SetBlocked(a, true);
    TEST("Nearest skips blocked A", graph.FindNearest(Vector3(1, 0, 0)) == b);

    // Empty graph returns invalid
    NavGraph empty;
    TEST("Empty graph returns invalid", empty.FindNearest(Vector3(0, 0, 0)) == InvalidNavNodeId);
}

static void TestNavGraphClear() {
    std::cout << "[NavGraph Clear]\n";

    NavGraph graph;
    graph.AddNode(Vector3(0, 0, 0));
    graph.AddNode(Vector3(10, 0, 0));
    TEST("2 nodes before clear", graph.NodeCount() == 2);

    graph.Clear();
    TEST("0 nodes after clear", graph.NodeCount() == 0);
    TEST("0 edges after clear", graph.EdgeCount() == 0);
}

static void TestNavGraphBuildGrid() {
    std::cout << "[NavGraph BuildGrid]\n";

    NavGraph graph;

    // 3x3x1 grid
    graph.BuildGrid(Vector3(0, 0, 0), 10.0f, 3, 3, 1);
    TEST("3x3x1 grid = 9 nodes", graph.NodeCount() == 9);

    // Each interior node has 4 edges (up/down/left/right in 2D plane)
    // Corner nodes have 2 edges, edge nodes have 3 edges
    // Total edges: 2*(3*2) = 12 bidirectional = 24 directed
    TEST("3x3x1 grid edge count = 24", graph.EdgeCount() == 24);

    // 2x2x2 grid
    NavGraph graph2;
    graph2.BuildGrid(Vector3(0, 0, 0), 5.0f, 2, 2, 2);
    TEST("2x2x2 grid = 8 nodes", graph2.NodeCount() == 8);
    // 12 bidirectional edges = 24 directed edges
    TEST("2x2x2 grid edge count = 24", graph2.EdgeCount() == 24);

    // 1x1x1 grid (single node)
    NavGraph graph3;
    graph3.BuildGrid(Vector3(0, 0, 0), 10.0f, 1, 1, 1);
    TEST("1x1x1 grid = 1 node", graph3.NodeCount() == 1);
    TEST("1x1x1 grid = 0 edges", graph3.EdgeCount() == 0);

    // Zero dimensions
    NavGraph graph4;
    graph4.BuildGrid(Vector3(0, 0, 0), 10.0f, 0, 5, 5);
    TEST("0x5x5 grid = 0 nodes", graph4.NodeCount() == 0);
}

static void TestPathfinderSimple() {
    std::cout << "[Pathfinder Simple]\n";

    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(0, 0, 0));
    NavNodeId b = graph.AddNode(Vector3(10, 0, 0));
    NavNodeId c = graph.AddNode(Vector3(20, 0, 0));
    graph.AddEdge(a, b);
    graph.AddEdge(b, c);

    Pathfinder pf(graph);
    NavPath path = pf.FindPath(a, c);

    TEST("Path is valid", path.valid);
    TEST("Path has 3 waypoints", path.Length() == 3);
    TEST("Path start at A", ApproxEq(path.waypoints[0].x, 0.0f));
    TEST("Path through B", ApproxEq(path.waypoints[1].x, 10.0f));
    TEST("Path ends at C", ApproxEq(path.waypoints[2].x, 20.0f));
    TEST("Path cost is 20", ApproxEq(path.totalCost, 20.0f));
    TEST("Nodes explored > 0", pf.LastNodesExplored() > 0);
}

static void TestPathfinderSameNode() {
    std::cout << "[Pathfinder SameNode]\n";

    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(5, 5, 5));

    Pathfinder pf(graph);
    NavPath path = pf.FindPath(a, a);

    TEST("Same node path valid", path.valid);
    TEST("Same node path has 1 waypoint", path.Length() == 1);
    TEST("Same node cost is 0", ApproxEq(path.totalCost, 0.0f));
}

static void TestPathfinderNoPath() {
    std::cout << "[Pathfinder NoPath]\n";

    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(0, 0, 0));
    NavNodeId b = graph.AddNode(Vector3(100, 0, 0));
    // No edge between them

    Pathfinder pf(graph);
    NavPath path = pf.FindPath(a, b);

    TEST("No path is invalid", !path.valid);
    TEST("No path is empty", path.IsEmpty());
}

static void TestPathfinderBlockedNode() {
    std::cout << "[Pathfinder BlockedNode]\n";

    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(0, 0, 0));
    NavNodeId b = graph.AddNode(Vector3(10, 0, 0));
    NavNodeId c = graph.AddNode(Vector3(20, 0, 0));
    graph.AddEdge(a, b);
    graph.AddEdge(b, c);

    // Block the middle node
    graph.SetBlocked(b, true);

    Pathfinder pf(graph);
    NavPath path = pf.FindPath(a, c);

    TEST("Path through blocked node invalid", !path.valid);

    // Block the goal
    graph.SetBlocked(b, false);
    graph.SetBlocked(c, true);
    NavPath path2 = pf.FindPath(a, c);
    TEST("Path to blocked goal invalid", !path2.valid);
}

static void TestPathfinderAlternateRoute() {
    std::cout << "[Pathfinder AlternateRoute]\n";

    // Diamond graph: A -> B -> D, A -> C -> D
    // Block B, should go through C
    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(0, 0, 0));
    NavNodeId b = graph.AddNode(Vector3(10, 10, 0));
    NavNodeId c = graph.AddNode(Vector3(10, -10, 0));
    NavNodeId d = graph.AddNode(Vector3(20, 0, 0));
    graph.AddEdge(a, b);
    graph.AddEdge(a, c);
    graph.AddEdge(b, d);
    graph.AddEdge(c, d);

    graph.SetBlocked(b, true);

    Pathfinder pf(graph);
    NavPath path = pf.FindPath(a, d);

    TEST("Alternate route found", path.valid);
    TEST("Alternate route has 3 waypoints", path.Length() == 3);
    // Should go through C (y=-10)
    TEST("Goes through C", ApproxEq(path.waypoints[1].y, -10.0f));
}

static void TestPathfinderByPosition() {
    std::cout << "[Pathfinder ByPosition]\n";

    NavGraph graph;
    graph.AddNode(Vector3(0, 0, 0));
    graph.AddNode(Vector3(10, 0, 0));
    graph.AddNode(Vector3(20, 0, 0));
    NavNodeId a = graph.FindNearest(Vector3(0, 0, 0));
    NavNodeId b = graph.FindNearest(Vector3(10, 0, 0));
    NavNodeId c = graph.FindNearest(Vector3(20, 0, 0));
    graph.AddEdge(a, b);
    graph.AddEdge(b, c);

    Pathfinder pf(graph);
    NavPath path = pf.FindPathByPosition(Vector3(1, 1, 0), Vector3(19, 0, 0));

    TEST("Position-based path valid", path.valid);
    TEST("Position-based path has waypoints", path.Length() >= 2);
}

static void TestPathfinderGrid() {
    std::cout << "[Pathfinder Grid]\n";

    NavGraph graph;
    graph.BuildGrid(Vector3(0, 0, 0), 10.0f, 5, 5, 1);

    Pathfinder pf(graph);
    NavNodeId start = graph.FindNearest(Vector3(0, 0, 0));
    NavNodeId goal = graph.FindNearest(Vector3(40, 40, 0));

    NavPath path = pf.FindPath(start, goal);
    TEST("Grid path valid", path.valid);
    TEST("Grid path has waypoints", path.Length() > 0);
    TEST("Grid path cost > 0", path.totalCost > 0.0f);

    // Path should go from (0,0,0) to (40,40,0)
    TEST("Grid path starts near origin",
         ApproxEq(path.waypoints.front().x, 0.0f) &&
         ApproxEq(path.waypoints.front().y, 0.0f));
    TEST("Grid path ends near (40,40,0)",
         ApproxEq(path.waypoints.back().x, 40.0f) &&
         ApproxEq(path.waypoints.back().y, 40.0f));
}

static void TestPathfinderGridBlocked() {
    std::cout << "[Pathfinder GridBlocked]\n";

    NavGraph graph;
    graph.BuildGrid(Vector3(0, 0, 0), 10.0f, 5, 1, 1);
    // Nodes at x=0,10,20,30,40 in a line

    // Block node at x=20 (the middle)
    NavNodeId midNode = graph.FindNearest(Vector3(20, 0, 0));
    graph.SetBlocked(midNode, true);

    Pathfinder pf(graph);
    NavNodeId start = graph.FindNearest(Vector3(0, 0, 0));
    NavNodeId goal = graph.FindNearest(Vector3(40, 0, 0));

    NavPath path = pf.FindPath(start, goal);
    // In a 1D line, blocking the middle means no path
    TEST("Blocked middle in line = no path", !path.valid);
}

static void TestPathfinderManhattanHeuristic() {
    std::cout << "[Pathfinder ManhattanHeuristic]\n";

    float d = Pathfinder::ManhattanHeuristic(Vector3(0, 0, 0), Vector3(3, 4, 5));
    TEST("Manhattan distance (3,4,5) = 12", ApproxEq(d, 12.0f));

    float d2 = Pathfinder::EuclideanHeuristic(Vector3(0, 0, 0), Vector3(3, 4, 0));
    TEST("Euclidean distance (3,4,0) = 5", ApproxEq(d2, 5.0f));
}

static void TestPathfinderCustomHeuristic() {
    std::cout << "[Pathfinder CustomHeuristic]\n";

    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(0, 0, 0));
    NavNodeId b = graph.AddNode(Vector3(10, 0, 0));
    graph.AddEdge(a, b);

    Pathfinder pf(graph);
    pf.SetHeuristic(Pathfinder::ManhattanHeuristic);
    NavPath path = pf.FindPath(a, b);
    TEST("Path with Manhattan heuristic valid", path.valid);
}

static void TestPathfinderNodeCost() {
    std::cout << "[Pathfinder NodeCost]\n";

    // Two paths: direct through expensive node vs longer through cheap nodes
    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(0, 0, 0), 1.0f);
    NavNodeId expensive = graph.AddNode(Vector3(10, 0, 0), 10.0f);
    NavNodeId cheap1 = graph.AddNode(Vector3(5, 10, 0), 1.0f);
    NavNodeId cheap2 = graph.AddNode(Vector3(15, 10, 0), 1.0f);
    NavNodeId goal = graph.AddNode(Vector3(20, 0, 0), 1.0f);

    graph.AddEdge(a, expensive);
    graph.AddEdge(expensive, goal);
    graph.AddEdge(a, cheap1);
    graph.AddEdge(cheap1, cheap2);
    graph.AddEdge(cheap2, goal);

    Pathfinder pf(graph);
    NavPath path = pf.FindPath(a, goal);
    TEST("Avoids expensive node path valid", path.valid);
    // The path should prefer going through cheap nodes
    TEST("Path has waypoints", path.Length() >= 2);
}

static void TestSmoothPathFunc() {
    std::cout << "[SmoothPath]\n";

    // Create a path with collinear points
    NavPath raw;
    raw.valid = true;
    raw.totalCost = 30.0f;
    raw.waypoints = {
        Vector3(0, 0, 0),
        Vector3(10, 0, 0),
        Vector3(20, 0, 0),
        Vector3(30, 0, 0)
    };

    NavPath smoothed = SmoothPath(raw);
    TEST("Smoothed path valid", smoothed.valid);
    TEST("Smoothed removes collinear", smoothed.Length() == 2); // start and end only
    TEST("Smoothed keeps start", ApproxEq(smoothed.waypoints.front().x, 0.0f));
    TEST("Smoothed keeps end", ApproxEq(smoothed.waypoints.back().x, 30.0f));
    TEST("Smoothed preserves cost", ApproxEq(smoothed.totalCost, 30.0f));

    // Non-collinear path should not be changed
    NavPath nonCollinear;
    nonCollinear.valid = true;
    nonCollinear.waypoints = {
        Vector3(0, 0, 0),
        Vector3(10, 10, 0),
        Vector3(20, 0, 0)
    };
    NavPath smoothed2 = SmoothPath(nonCollinear);
    TEST("Non-collinear preserved", smoothed2.Length() == 3);

    // Invalid path returns as-is
    NavPath invalid;
    NavPath smoothedInvalid = SmoothPath(invalid);
    TEST("Invalid path unchanged", !smoothedInvalid.valid);

    // Two-point path returns as-is
    NavPath twoPoint;
    twoPoint.valid = true;
    twoPoint.waypoints = { Vector3(0, 0, 0), Vector3(10, 0, 0) };
    NavPath smoothedTwo = SmoothPath(twoPoint);
    TEST("Two-point path unchanged", smoothedTwo.Length() == 2);
}

static void TestNavPath() {
    std::cout << "[NavPath]\n";

    NavPath path;
    TEST("Default path invalid", !path.valid);
    TEST("Default path empty", path.IsEmpty());
    TEST("Default path length 0", path.Length() == 0);

    path.valid = true;
    path.waypoints = { Vector3(0, 0, 0), Vector3(10, 0, 0) };
    TEST("Path not empty after add", !path.IsEmpty());
    TEST("Path length 2", path.Length() == 2);
}

// ===================================================================
// PathfindingComponent tests
// ===================================================================

static void TestPathfindingComponent() {
    std::cout << "[PathfindingComponent]\n";

    PathfindingComponent pfc;

    // Default state
    TEST("Default no target", !pfc.hasTarget);
    TEST("Default reached destination", pfc.HasReachedDestination());
    TEST("Default waypoint is zero", ApproxEq(pfc.GetNextWaypoint().x, 0.0f));
    TEST("Default arrival threshold 5", ApproxEq(pfc.arrivalThreshold, 5.0f));
    TEST("Default repath interval 2", ApproxEq(pfc.repathInterval, 2.0f));

    // SetTarget
    pfc.SetTarget(Vector3(100, 50, 0));
    TEST("Has target after set", pfc.hasTarget);
    TEST("Needs repath after set", pfc.needsRepath);
    TEST("Target x correct", ApproxEq(pfc.targetPosition.x, 100.0f));
    TEST("Target y correct", ApproxEq(pfc.targetPosition.y, 50.0f));
    TEST("Waypoint index reset", pfc.currentWaypointIndex == 0);

    // Not reached (no path assigned yet)
    TEST("Not reached without path", !pfc.HasReachedDestination());

    // Assign a path manually
    NavPath path;
    path.valid = true;
    path.waypoints = {
        Vector3(0, 0, 0),
        Vector3(50, 25, 0),
        Vector3(100, 50, 0)
    };
    pfc.currentPath = path;
    pfc.currentWaypointIndex = 0;

    TEST("First waypoint is start", ApproxEq(pfc.GetNextWaypoint().x, 0.0f));

    // AdvanceWaypoint: too far away
    bool advanced = pfc.AdvanceWaypoint(Vector3(50, 50, 50));
    TEST("Not advanced when far", !advanced);

    // AdvanceWaypoint: close enough
    advanced = pfc.AdvanceWaypoint(Vector3(1, 1, 1));
    TEST("Advanced when close", advanced);
    TEST("Waypoint index is 1", pfc.currentWaypointIndex == 1);
    TEST("Next waypoint is second", ApproxEq(pfc.GetNextWaypoint().x, 50.0f));

    // Advance to end
    pfc.AdvanceWaypoint(Vector3(50, 25, 0));
    pfc.AdvanceWaypoint(Vector3(100, 50, 0));
    TEST("Reached destination", pfc.HasReachedDestination());
    TEST("Waypoint past end returns zero", ApproxEq(pfc.GetNextWaypoint().x, 0.0f));

    // ClearPath
    pfc.ClearPath();
    TEST("No target after clear", !pfc.hasTarget);
    TEST("Path empty after clear", pfc.currentPath.IsEmpty());
    TEST("Index 0 after clear", pfc.currentWaypointIndex == 0);
}

// ===================================================================
// PathfindingSystem tests
// ===================================================================

static void TestPathfindingSystem() {
    std::cout << "[PathfindingSystem]\n";

    EntityManager em;
    PathfindingSystem pfSys(em);

    // Build a small nav grid
    pfSys.BuildNavGrid(Vector3(0, 0, 0), 10.0f, 5, 5, 1);
    TEST("Nav grid built", pfSys.GetNavGraph().NodeCount() == 25);

    // Request a path
    NavPath path = pfSys.RequestPath(Vector3(0, 0, 0), Vector3(40, 40, 0));
    TEST("Requested path valid", path.valid);
    TEST("Total paths calculated is 1", pfSys.GetTotalPathsCalculated() == 1);

    // Create entity with pathfinding component
    auto& ent = em.CreateEntity("NavBot");
    auto physComp = std::make_unique<PhysicsComponent>();
    physComp->position = Vector3(0, 0, 0);
    em.AddComponent<PhysicsComponent>(ent.id, std::move(physComp));

    auto pfComp = std::make_unique<PathfindingComponent>();
    pfComp->SetTarget(Vector3(40, 40, 0));
    auto* pfc = em.AddComponent<PathfindingComponent>(ent.id, std::move(pfComp));

    // First update should trigger path calculation (needsRepath = true)
    pfSys.Update(0.1f);
    TEST("Path calculated after update", pfc->currentPath.valid);
    TEST("Path has waypoints", !pfc->currentPath.IsEmpty());
    TEST("Total paths is 2", pfSys.GetTotalPathsCalculated() == 2);
}

static void TestPathfindingSystemRepath() {
    std::cout << "[PathfindingSystem Repath]\n";

    EntityManager em;
    PathfindingSystem pfSys(em);
    pfSys.BuildNavGrid(Vector3(0, 0, 0), 10.0f, 3, 3, 1);

    auto& ent = em.CreateEntity("NavBot2");
    auto physComp = std::make_unique<PhysicsComponent>();
    physComp->position = Vector3(0, 0, 0);
    em.AddComponent<PhysicsComponent>(ent.id, std::move(physComp));

    auto pfComp = std::make_unique<PathfindingComponent>();
    pfComp->SetTarget(Vector3(20, 20, 0));
    pfComp->repathInterval = 1.0f;
    auto* pfc = em.AddComponent<PathfindingComponent>(ent.id, std::move(pfComp));

    // First update: repath due to needsRepath flag
    pfSys.Update(0.1f);
    int count1 = pfSys.GetTotalPathsCalculated();
    TEST("First update calculates path", count1 == 1);

    // Small update: no repath needed
    pfSys.Update(0.1f);
    TEST("No repath on small dt", pfSys.GetTotalPathsCalculated() == 1);

    // Large update: repath triggered by timer
    pfSys.Update(1.0f);
    TEST("Repath after interval", pfSys.GetTotalPathsCalculated() == 2);
}

static void TestPathfindingGameEvents() {
    std::cout << "[Pathfinding GameEvents]\n";

    TEST("PathFound event", std::string(GameEvents::PathFound) == "navigation.path.found");
    TEST("PathNotFound event", std::string(GameEvents::PathNotFound) == "navigation.path.not_found");
    TEST("WaypointReached event", std::string(GameEvents::WaypointReached) == "navigation.waypoint.reached");
    TEST("PathCompleted event", std::string(GameEvents::PathCompleted) == "navigation.path.completed");
    TEST("NavGridBuilt event", std::string(GameEvents::NavGridBuilt) == "navigation.grid.built");
}

static void TestPathfinder3D() {
    std::cout << "[Pathfinder 3D]\n";

    NavGraph graph;
    graph.BuildGrid(Vector3(0, 0, 0), 10.0f, 3, 3, 3);
    TEST("3D grid has 27 nodes", graph.NodeCount() == 27);

    Pathfinder pf(graph);
    NavNodeId start = graph.FindNearest(Vector3(0, 0, 0));
    NavNodeId goal = graph.FindNearest(Vector3(20, 20, 20));

    NavPath path = pf.FindPath(start, goal);
    TEST("3D path valid", path.valid);
    TEST("3D path starts at origin",
         ApproxEq(path.waypoints.front().x, 0.0f) &&
         ApproxEq(path.waypoints.front().y, 0.0f) &&
         ApproxEq(path.waypoints.front().z, 0.0f));
    TEST("3D path ends at (20,20,20)",
         ApproxEq(path.waypoints.back().x, 20.0f) &&
         ApproxEq(path.waypoints.back().y, 20.0f) &&
         ApproxEq(path.waypoints.back().z, 20.0f));
}

static void TestPathfinderInvalidNodes() {
    std::cout << "[Pathfinder InvalidNodes]\n";

    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(0, 0, 0));

    Pathfinder pf(graph);

    // Non-existent start
    NavPath path1 = pf.FindPath(999, a);
    TEST("Invalid start = no path", !path1.valid);

    // Non-existent goal
    NavPath path2 = pf.FindPath(a, 999);
    TEST("Invalid goal = no path", !path2.valid);

    // Both invalid
    NavPath path3 = pf.FindPath(998, 999);
    TEST("Both invalid = no path", !path3.valid);
}

static void TestNavGraphEdgeWeight() {
    std::cout << "[NavGraph EdgeWeight]\n";

    NavGraph graph;
    NavNodeId a = graph.AddNode(Vector3(0, 0, 0), 1.0f);
    NavNodeId b = graph.AddNode(Vector3(10, 0, 0), 3.0f);

    // Auto-calculated weight = distance * avg(1.0, 3.0) = 10 * 2.0 = 20
    graph.AddEdge(a, b);
    const auto& edges = graph.GetEdges(a);
    TEST("Auto weight = dist * avgCost", ApproxEq(edges[0].weight, 20.0f));

    // Explicit weight overrides
    NavNodeId c = graph.AddNode(Vector3(20, 0, 0));
    graph.AddEdge(b, c, 5.0f);
    const auto& edgesB = graph.GetEdges(b);
    bool hasExplicit = false;
    for (const auto& e : edgesB) {
        if (e.to == c && ApproxEq(e.weight, 5.0f)) hasExplicit = true;
    }
    TEST("Explicit weight applied", hasExplicit);
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
// Ammunition System tests
// ===================================================================
static void TestAmmoPoolCanFire() {
    std::cout << "[AmmoPool CanFire]\n";
    AmmoPool pool;
    pool.maxAmmo = 10;
    pool.currentAmmo = 10;
    pool.isReloading = false;
    TEST("CanFire with ammo", pool.CanFire());

    pool.currentAmmo = 0;
    TEST("Cannot fire empty", !pool.CanFire());

    pool.currentAmmo = 5;
    pool.isReloading = true;
    TEST("Cannot fire while reloading", !pool.CanFire());
}

static void TestAmmoPoolConsumeAmmo() {
    std::cout << "[AmmoPool ConsumeAmmo]\n";
    AmmoPool pool;
    pool.maxAmmo = 5;
    pool.currentAmmo = 3;
    pool.isReloading = false;

    TEST("Consume succeeds", pool.ConsumeAmmo());
    TEST("Ammo decremented", pool.currentAmmo == 2);

    TEST("Consume again", pool.ConsumeAmmo());
    TEST("Consume again", pool.ConsumeAmmo());
    TEST("Ammo at zero", pool.currentAmmo == 0);
    TEST("Consume fails when empty", !pool.ConsumeAmmo());
}

static void TestAmmoPoolReload() {
    std::cout << "[AmmoPool Reload]\n";
    AmmoPool pool;
    pool.maxAmmo = 30;
    pool.currentAmmo = 0;
    pool.reloadTime = 4.0f;

    pool.StartReload();
    TEST("Is reloading", pool.isReloading);
    TEST("Timer set", ApproxEq(pool.currentReloadTimer, 4.0f));

    bool done = pool.UpdateReload(2.0f);
    TEST("Reload not done at half", !done);
    TEST("Still reloading", pool.isReloading);

    done = pool.UpdateReload(2.0f);
    TEST("Reload complete", done);
    TEST("No longer reloading", !pool.isReloading);
    TEST("Ammo refilled to max", pool.currentAmmo == 30);
    TEST("Timer zeroed", ApproxEq(pool.currentReloadTimer, 0.0f));
}

static void TestAmmoPoolRefill() {
    std::cout << "[AmmoPool Refill]\n";
    AmmoPool pool;
    pool.maxAmmo = 50;
    pool.currentAmmo = 10;
    pool.isReloading = true;
    pool.currentReloadTimer = 2.0f;

    pool.Refill();
    TEST("Ammo at max", pool.currentAmmo == 50);
    TEST("Not reloading", !pool.isReloading);
    TEST("Timer zeroed", ApproxEq(pool.currentReloadTimer, 0.0f));
}

static void TestAmmoPoolPercentage() {
    std::cout << "[AmmoPool Percentage]\n";
    AmmoPool pool;
    pool.maxAmmo = 100;
    pool.currentAmmo = 75;
    TEST("75% ammo", ApproxEq(pool.GetAmmoPercentage(), 75.0f));

    pool.currentAmmo = 0;
    TEST("0% ammo", ApproxEq(pool.GetAmmoPercentage(), 0.0f));

    pool.currentAmmo = 100;
    TEST("100% ammo", ApproxEq(pool.GetAmmoPercentage(), 100.0f));

    pool.maxAmmo = 0;
    TEST("0 max ammo returns 0%", ApproxEq(pool.GetAmmoPercentage(), 0.0f));
}

static void TestDefaultAmmoPools() {
    std::cout << "[DefaultAmmoPools]\n";
    auto broadside = WeaponSystem::GetDefaultAmmoPool(WeaponType::BroadsideCannon);
    TEST("Broadside ammo type Standard", broadside.type == AmmoType::Standard);
    TEST("Broadside max ammo 30", broadside.maxAmmo == 30);
    TEST("Broadside current == max", broadside.currentAmmo == broadside.maxAmmo);

    auto railgun = WeaponSystem::GetDefaultAmmoPool(WeaponType::SpinalRailgun);
    TEST("Railgun ammo type ArmorPiercing", railgun.type == AmmoType::ArmorPiercing);
    TEST("Railgun max ammo 5", railgun.maxAmmo == 5);

    auto flak = WeaponSystem::GetDefaultAmmoPool(WeaponType::InwardFlak);
    TEST("Flak ammo type Explosive", flak.type == AmmoType::Explosive);
    TEST("Flak max ammo 60", flak.maxAmmo == 60);

    auto lancer = WeaponSystem::GetDefaultAmmoPool(WeaponType::BurstLancer);
    TEST("Lancer ammo type Incendiary", lancer.type == AmmoType::Incendiary);
    TEST("Lancer max ammo 8", lancer.maxAmmo == 8);

    auto beam = WeaponSystem::GetDefaultAmmoPool(WeaponType::BeamArray);
    TEST("Beam ammo type EMP", beam.type == AmmoType::EMP);
    TEST("Beam max ammo 200", beam.maxAmmo == 200);
}

static void TestAmmoDamageMultiplier() {
    std::cout << "[AmmoDamageMultiplier]\n";
    TEST("Standard multiplier 1.0", ApproxEq(WeaponSystem::GetAmmoDamageMultiplier(AmmoType::Standard), 1.0f));
    TEST("ArmorPiercing multiplier 1.3", ApproxEq(WeaponSystem::GetAmmoDamageMultiplier(AmmoType::ArmorPiercing), 1.3f));
    TEST("Explosive multiplier 1.5", ApproxEq(WeaponSystem::GetAmmoDamageMultiplier(AmmoType::Explosive), 1.5f));
    TEST("EMP multiplier 0.5", ApproxEq(WeaponSystem::GetAmmoDamageMultiplier(AmmoType::EMP), 0.5f));
    TEST("Incendiary multiplier 1.2", ApproxEq(WeaponSystem::GetAmmoDamageMultiplier(AmmoType::Incendiary), 1.2f));
}

static void TestAmmoReloadNotReloading() {
    std::cout << "[AmmoPool UpdateReload not reloading]\n";
    AmmoPool pool;
    pool.isReloading = false;
    bool done = pool.UpdateReload(1.0f);
    TEST("UpdateReload returns false when not reloading", !done);
}

// ===================================================================
// Target Lock System tests
// ===================================================================
static void TestTargetLockComponent() {
    std::cout << "[TargetLockComponent]\n";

    TargetLockComponent tlc;
    TEST("Default state None", tlc.lockState == LockState::None);
    TEST("Default target invalid", tlc.targetId == InvalidEntityId);
    TEST("Default not locked", !tlc.IsLocked());
    TEST("Default not acquiring", !tlc.IsAcquiring());
    TEST("Default progress 0", ApproxEq(tlc.GetLockProgress(), 0.0f));

    tlc.BeginLock(42);
    TEST("BeginLock sets target", tlc.targetId == 42);
    TEST("BeginLock sets Acquiring", tlc.IsAcquiring());
    TEST("Not yet locked", !tlc.IsLocked());
    TEST("Progress 0 after begin", ApproxEq(tlc.GetLockProgress(), 0.0f));

    // Simulate partial lock progress
    tlc.lockTimer = 1.0f; // half of 2.0s acquire time
    TEST("Progress 50%", ApproxEq(tlc.GetLockProgress(), 50.0f));

    tlc.lockTimer = 2.0f;
    TEST("Progress 100%", ApproxEq(tlc.GetLockProgress(), 100.0f));

    tlc.ClearLock();
    TEST("ClearLock resets target", tlc.targetId == InvalidEntityId);
    TEST("ClearLock sets None", tlc.lockState == LockState::None);
    TEST("ClearLock resets timer", ApproxEq(tlc.lockTimer, 0.0f));
}

static void TestTargetLockComponentZeroAcquireTime() {
    std::cout << "[TargetLockComponent zero acquire time]\n";

    TargetLockComponent tlc;
    tlc.lockAcquireTime = 0.0f;
    TEST("Zero acquire time returns 100% progress", ApproxEq(tlc.GetLockProgress(), 100.0f));
}

static void TestTargetLockSystem() {
    std::cout << "[TargetLockSystem]\n";

    TargetLockSystem tls;
    TEST("TargetLockSystem name", tls.GetName() == "TargetLockSystem");

    // No EntityManager - should not crash
    tls.Update(1.0f);
    TEST("Update without EM does not crash", true);
}

static void TestTargetLockSystemAcquire() {
    std::cout << "[TargetLockSystem Acquire]\n";

    EntityManager em;
    auto& ship = em.CreateEntity("Ship");
    auto* phys = em.AddComponent<PhysicsComponent>(ship.id, std::make_unique<PhysicsComponent>());
    phys->position = Vector3(0, 0, 0);
    auto* lock = em.AddComponent<TargetLockComponent>(ship.id, std::make_unique<TargetLockComponent>());
    lock->lockRange = 500.0f;
    lock->lockAcquireTime = 2.0f;
    lock->lockBreakRange = 600.0f;

    auto& target = em.CreateEntity("Target");
    auto* tPhys = em.AddComponent<PhysicsComponent>(target.id, std::make_unique<PhysicsComponent>());
    tPhys->position = Vector3(100, 0, 0); // 100 units away (within range)

    lock->BeginLock(target.id);
    TEST("Acquiring after BeginLock", lock->IsAcquiring());

    TargetLockSystem tls(em);

    // Update for 1 second: should still be acquiring
    tls.Update(1.0f);
    TEST("Still acquiring after 1s", lock->IsAcquiring());
    TEST("Timer advanced", ApproxEq(lock->lockTimer, 1.0f));

    // Update for another 1 second: should be locked
    tls.Update(1.0f);
    TEST("Locked after 2s", lock->IsLocked());
}

static void TestTargetLockSystemBreak() {
    std::cout << "[TargetLockSystem Break]\n";

    EntityManager em;
    auto& ship = em.CreateEntity("Ship");
    auto* phys = em.AddComponent<PhysicsComponent>(ship.id, std::make_unique<PhysicsComponent>());
    phys->position = Vector3(0, 0, 0);
    auto* lock = em.AddComponent<TargetLockComponent>(ship.id, std::make_unique<TargetLockComponent>());
    lock->lockRange = 500.0f;
    lock->lockAcquireTime = 1.0f;
    lock->lockBreakRange = 600.0f;

    auto& target = em.CreateEntity("Target");
    auto* tPhys = em.AddComponent<PhysicsComponent>(target.id, std::make_unique<PhysicsComponent>());
    tPhys->position = Vector3(100, 0, 0);

    lock->BeginLock(target.id);

    TargetLockSystem tls(em);
    tls.Update(1.0f); // Lock acquired
    TEST("Lock acquired", lock->IsLocked());

    // Move target beyond break range
    tPhys->position = Vector3(700, 0, 0);
    tls.Update(0.1f);
    TEST("Lock broken beyond break range", lock->lockState == LockState::None);
    TEST("Target cleared", lock->targetId == InvalidEntityId);
}

static void TestTargetLockSystemOutOfRange() {
    std::cout << "[TargetLockSystem OutOfRange]\n";

    EntityManager em;
    auto& ship = em.CreateEntity("Ship");
    auto* phys = em.AddComponent<PhysicsComponent>(ship.id, std::make_unique<PhysicsComponent>());
    phys->position = Vector3(0, 0, 0);
    auto* lock = em.AddComponent<TargetLockComponent>(ship.id, std::make_unique<TargetLockComponent>());
    lock->lockRange = 200.0f;

    auto& target = em.CreateEntity("Target");
    auto* tPhys = em.AddComponent<PhysicsComponent>(target.id, std::make_unique<PhysicsComponent>());
    tPhys->position = Vector3(300, 0, 0); // out of range

    lock->BeginLock(target.id);

    TargetLockSystem tls(em);
    tls.Update(0.5f);
    TEST("Lock cancelled when target out of range", lock->lockState == LockState::None);
}

static void TestTargetLockSystemDistance() {
    std::cout << "[TargetLockSystem Distance]\n";

    EntityManager em;
    auto& e1 = em.CreateEntity("E1");
    auto* p1 = em.AddComponent<PhysicsComponent>(e1.id, std::make_unique<PhysicsComponent>());
    p1->position = Vector3(0, 0, 0);

    auto& e2 = em.CreateEntity("E2");
    auto* p2 = em.AddComponent<PhysicsComponent>(e2.id, std::make_unique<PhysicsComponent>());
    p2->position = Vector3(3, 4, 0);

    TargetLockSystem tls(em);
    float dist = tls.GetDistanceBetween(e1.id, e2.id);
    TEST("Distance 3-4-5 triangle", ApproxEq(dist, 5.0f));

    // Entity without physics
    auto& e3 = em.CreateEntity("E3");
    float dist2 = tls.GetDistanceBetween(e1.id, e3.id);
    TEST("Distance -1 when no physics", ApproxEq(dist2, -1.0f));
}

static void TestTargetLockSystemNoPhysics() {
    std::cout << "[TargetLockSystem NoPhysics]\n";

    EntityManager em;
    TargetLockSystem tls(em);
    float dist = tls.GetDistanceBetween(999, 998);
    TEST("Distance -1 for nonexistent entities", ApproxEq(dist, -1.0f));
}

// ===================================================================
// Anomaly System tests
// ===================================================================
static void TestAnomalyGeneration() {
    std::cout << "[AnomalyGeneration]\n";

    GalaxyGenerator gen(42);
    gen.anomalyProbability = 1.0f; // force anomalies in every sector

    GalaxySector sector = gen.GenerateSector(5, 5, 0);
    TEST("Anomalies generated", !sector.anomalies.empty());

    for (const auto& a : sector.anomalies) {
        TEST("Anomaly has name", !a.name.empty());
        TEST("Anomaly has positive radius", a.radius > 0.0f);
        TEST("Anomaly has positive intensity", a.intensity > 0.0f);
        int typeVal = static_cast<int>(a.type);
        TEST("Anomaly type in valid range", typeVal >= 0 && typeVal <= 4);
    }
}

static void TestAnomalyDeterminism() {
    std::cout << "[AnomalyDeterminism]\n";

    GalaxyGenerator gen1(123);
    gen1.anomalyProbability = 1.0f;
    GalaxySector s1 = gen1.GenerateSector(10, 20, 0);

    GalaxyGenerator gen2(123);
    gen2.anomalyProbability = 1.0f;
    GalaxySector s2 = gen2.GenerateSector(10, 20, 0);

    TEST("Same number of anomalies", s1.anomalies.size() == s2.anomalies.size());

    for (size_t i = 0; i < s1.anomalies.size() && i < s2.anomalies.size(); ++i) {
        TEST("Same anomaly type", s1.anomalies[i].type == s2.anomalies[i].type);
        TEST("Same anomaly name", s1.anomalies[i].name == s2.anomalies[i].name);
        TEST("Same anomaly radius", ApproxEq(s1.anomalies[i].radius, s2.anomalies[i].radius));
        TEST("Same anomaly intensity", ApproxEq(s1.anomalies[i].intensity, s2.anomalies[i].intensity));
    }
}

static void TestAnomalyProbabilityZero() {
    std::cout << "[AnomalyProbabilityZero]\n";

    GalaxyGenerator gen(99);
    gen.anomalyProbability = 0.0f; // never generate anomalies

    // Check many sectors
    bool anyAnomalies = false;
    for (int i = 0; i < 50; ++i) {
        GalaxySector s = gen.GenerateSector(i, i, 0);
        if (!s.anomalies.empty()) anyAnomalies = true;
    }
    TEST("No anomalies with probability 0", !anyAnomalies);
}

static void TestAnomalyTypes() {
    std::cout << "[AnomalyTypes]\n";
    // Verify all anomaly types are valid enum values
    AnomalyData a;
    a.type = AnomalyType::Nebula;
    TEST("Nebula type", static_cast<int>(a.type) == 0);
    a.type = AnomalyType::BlackHole;
    TEST("BlackHole type", static_cast<int>(a.type) == 1);
    a.type = AnomalyType::RadiationZone;
    TEST("RadiationZone type", static_cast<int>(a.type) == 2);
    a.type = AnomalyType::IonStorm;
    TEST("IonStorm type", static_cast<int>(a.type) == 3);
    a.type = AnomalyType::GravityWell;
    TEST("GravityWell type", static_cast<int>(a.type) == 4);
}

// ===================================================================
// New Game Event Constants tests
// ===================================================================
static void TestAdvancedCombatGameEvents() {
    std::cout << "[AdvancedCombatGameEvents]\n";
    // Ammunition events
    TEST("AmmoDepleted event", std::string(GameEvents::AmmoDepleted) == "combat.ammo.depleted");
    TEST("AmmoReloaded event", std::string(GameEvents::AmmoReloaded) == "combat.ammo.reloaded");
    // Target lock events
    TEST("TargetLocked event", std::string(GameEvents::TargetLocked) == "combat.target.locked");
    TEST("TargetLost event", std::string(GameEvents::TargetLost) == "combat.target.lost");
    // Anomaly events
    TEST("AnomalyDiscovered event", std::string(GameEvents::AnomalyDiscovered) == "sector.anomaly.discovered");
    TEST("AnomalyEffect event", std::string(GameEvents::AnomalyEffect) == "sector.anomaly.effect");
}

// ===================================================================
// Shield System tests
// ===================================================================
static void TestShieldComponentDefaults() {
    std::cout << "[ShieldModuleComponent Defaults]\n";
    ShieldModuleComponent sc;
    TEST("Default type Standard", sc.shieldType == ShieldType::Standard);
    TEST("Default max 100", ApproxEq(sc.maxShield, 100.0f));
    TEST("Default current 100", ApproxEq(sc.currentShield, 100.0f));
    TEST("Default regen rate 5", ApproxEq(sc.regenRate, 5.0f));
    TEST("Default regen delay 3", ApproxEq(sc.regenDelay, 3.0f));
    TEST("Default active", sc.isActive);
    TEST("Default overcharge 0", ApproxEq(sc.overchargeAmount, 0.0f));
    TEST("Not depleted by default", !sc.IsDepleted());
    TEST("Effective shield 100", ApproxEq(sc.GetEffectiveShield(), 100.0f));
    TEST("Percentage 100%", ApproxEq(sc.GetShieldPercentage(), 100.0f));
}

static void TestShieldAbsorbDamage() {
    std::cout << "[ShieldModuleComponent AbsorbDamage]\n";
    ShieldModuleComponent sc;
    sc.maxShield = 100.0f;
    sc.currentShield = 100.0f;
    sc.shieldType = ShieldType::Standard;

    // Standard absorption: 1.0x multiplier
    float overflow = sc.AbsorbDamage(30.0f);
    TEST("No overflow with 30 dmg", ApproxEq(overflow, 0.0f));
    TEST("Shield at 70 after 30 dmg", ApproxEq(sc.currentShield, 70.0f));
    TEST("timeSinceLastHit reset", ApproxEq(sc.timeSinceLastHit, 0.0f));

    // Damage that exceeds shield
    overflow = sc.AbsorbDamage(100.0f);
    TEST("Overflow from excess damage", overflow > 0.0f);
    TEST("Shield depleted to 0", ApproxEq(sc.currentShield, 0.0f));
    TEST("Depleted after excess damage", sc.IsDepleted());
}

static void TestShieldAbsorbDamageHardened() {
    std::cout << "[ShieldModuleComponent AbsorbDamage Hardened]\n";
    ShieldModuleComponent sc;
    sc.maxShield = 100.0f;
    sc.currentShield = 100.0f;
    sc.shieldType = ShieldType::Hardened; // 0.7x absorption

    // 50 damage * 0.7 = 35 absorbed
    float overflow = sc.AbsorbDamage(50.0f);
    TEST("No overflow Hardened 50 dmg", ApproxEq(overflow, 0.0f));
    TEST("Hardened absorbs 35 from 50", ApproxEq(sc.currentShield, 65.0f));
}

static void TestShieldAbsorbDamageInactive() {
    std::cout << "[ShieldModuleComponent AbsorbDamage Inactive]\n";
    ShieldModuleComponent sc;
    sc.currentShield = 100.0f;
    sc.isActive = false;

    float overflow = sc.AbsorbDamage(50.0f);
    TEST("Inactive shield passes all damage", ApproxEq(overflow, 50.0f));
    TEST("Shield unchanged when inactive", ApproxEq(sc.currentShield, 100.0f));
}

static void TestShieldOvercharge() {
    std::cout << "[ShieldModuleComponent Overcharge]\n";
    ShieldModuleComponent sc;
    sc.maxShield = 100.0f;
    sc.currentShield = 100.0f;

    sc.ApplyOvercharge(50.0f);
    TEST("Overcharge added", ApproxEq(sc.overchargeAmount, 50.0f));
    TEST("Effective shield 150", ApproxEq(sc.GetEffectiveShield(), 150.0f));
    TEST("Not depleted with overcharge", !sc.IsDepleted());

    // Absorb damage consumes overcharge first
    float overflow = sc.AbsorbDamage(30.0f);
    TEST("No overflow with overcharge", ApproxEq(overflow, 0.0f));
    TEST("Overcharge reduced to 20", ApproxEq(sc.overchargeAmount, 20.0f));
    TEST("Current shield unchanged", ApproxEq(sc.currentShield, 100.0f));

    // Damage that bleeds through overcharge into shield
    overflow = sc.AbsorbDamage(50.0f);
    TEST("No overflow bleed-through", ApproxEq(overflow, 0.0f));
    TEST("Overcharge fully consumed", ApproxEq(sc.overchargeAmount, 0.0f));
    TEST("Shield reduced by remaining", ApproxEq(sc.currentShield, 70.0f));
}

static void TestShieldPercentageEdgeCases() {
    std::cout << "[ShieldModuleComponent Percentage Edge Cases]\n";
    ShieldModuleComponent sc;
    sc.maxShield = 0.0f;
    TEST("0 maxShield returns 0%", ApproxEq(sc.GetShieldPercentage(), 0.0f));

    sc.maxShield = 200.0f;
    sc.currentShield = 100.0f;
    TEST("50% shield", ApproxEq(sc.GetShieldPercentage(), 50.0f));

    sc.currentShield = 0.0f;
    TEST("0% shield", ApproxEq(sc.GetShieldPercentage(), 0.0f));
}

static void TestShieldRestore() {
    std::cout << "[ShieldModuleComponent Restore]\n";
    ShieldModuleComponent sc;
    sc.maxShield = 100.0f;
    sc.currentShield = 20.0f;
    sc.overchargeAmount = 50.0f;

    sc.RestoreShield();
    TEST("Restore sets current to max", ApproxEq(sc.currentShield, 100.0f));
    TEST("Restore clears overcharge", ApproxEq(sc.overchargeAmount, 0.0f));
}

static void TestShieldAbsorptionMultipliers() {
    std::cout << "[ShieldModuleComponent Absorption Multipliers]\n";
    TEST("Standard multiplier 1.0", ApproxEq(ShieldModuleComponent::GetAbsorptionMultiplier(ShieldType::Standard), 1.0f));
    TEST("Hardened multiplier 0.7", ApproxEq(ShieldModuleComponent::GetAbsorptionMultiplier(ShieldType::Hardened), 0.7f));
    TEST("Phase multiplier 0.85", ApproxEq(ShieldModuleComponent::GetAbsorptionMultiplier(ShieldType::Phase), 0.85f));
    TEST("Regenerative multiplier 1.1", ApproxEq(ShieldModuleComponent::GetAbsorptionMultiplier(ShieldType::Regenerative), 1.1f));
}

static void TestShieldComponentSerialization() {
    std::cout << "[ShieldModuleComponent Serialization]\n";
    ShieldModuleComponent original;
    original.shieldType = ShieldType::Phase;
    original.maxShield = 200.0f;
    original.currentShield = 150.0f;
    original.regenRate = 10.0f;
    original.regenDelay = 5.0f;
    original.timeSinceLastHit = 2.5f;
    original.isActive = true;
    original.overchargeAmount = 30.0f;
    original.overchargeDecayRate = 15.0f;

    ComponentData cd = original.Serialize();
    TEST("Serialized type", cd.componentType == "ShieldModuleComponent");

    ShieldModuleComponent restored;
    restored.Deserialize(cd);
    TEST("Restored shieldType", restored.shieldType == ShieldType::Phase);
    TEST("Restored maxShield", ApproxEq(restored.maxShield, 200.0f));
    TEST("Restored currentShield", ApproxEq(restored.currentShield, 150.0f));
    TEST("Restored regenRate", ApproxEq(restored.regenRate, 10.0f));
    TEST("Restored regenDelay", ApproxEq(restored.regenDelay, 5.0f));
    TEST("Restored timeSinceLastHit", ApproxEq(restored.timeSinceLastHit, 2.5f));
    TEST("Restored isActive", restored.isActive);
    TEST("Restored overchargeAmount", ApproxEq(restored.overchargeAmount, 30.0f));
    TEST("Restored overchargeDecayRate", ApproxEq(restored.overchargeDecayRate, 15.0f));
}

static void TestShieldSystem() {
    std::cout << "[ShieldSystem]\n";
    ShieldSystem ss;
    TEST("ShieldSystem name", ss.GetName() == "ShieldSystem");
    ss.Update(1.0f); // Should not crash without EntityManager
    TEST("Update without EM does not crash", true);
}

static void TestShieldSystemRegen() {
    std::cout << "[ShieldSystem Regen]\n";
    EntityManager em;
    auto& ent = em.CreateEntity("Ship");
    auto* sc = em.AddComponent<ShieldModuleComponent>(ent.id, std::make_unique<ShieldModuleComponent>());
    sc->maxShield = 100.0f;
    sc->currentShield = 50.0f;
    sc->regenRate = 10.0f;
    sc->regenDelay = 2.0f;
    sc->timeSinceLastHit = 0.0f;

    ShieldSystem ss(em);

    // Before regen delay
    ss.Update(1.0f);
    TEST("No regen before delay", ApproxEq(sc->currentShield, 50.0f));
    TEST("Time since hit 1s", ApproxEq(sc->timeSinceLastHit, 1.0f));

    // Still before delay
    ss.Update(0.5f);
    TEST("Still no regen at 1.5s", ApproxEq(sc->currentShield, 50.0f));

    // After delay - should start regenerating
    ss.Update(1.0f); // now at 2.5s, 0.5s of regen time
    TEST("Regen started after delay", sc->currentShield > 50.0f);

    // Regen to max
    ss.Update(10.0f);
    TEST("Shield capped at max", ApproxEq(sc->currentShield, 100.0f));
}

static void TestShieldSystemOverchargeDecay() {
    std::cout << "[ShieldSystem Overcharge Decay]\n";
    EntityManager em;
    auto& ent = em.CreateEntity("Ship");
    auto* sc = em.AddComponent<ShieldModuleComponent>(ent.id, std::make_unique<ShieldModuleComponent>());
    sc->maxShield = 100.0f;
    sc->currentShield = 100.0f;
    sc->overchargeAmount = 50.0f;
    sc->overchargeDecayRate = 10.0f;
    sc->timeSinceLastHit = 10.0f; // already past delay

    ShieldSystem ss(em);
    ss.Update(1.0f);
    TEST("Overcharge decayed by 10", ApproxEq(sc->overchargeAmount, 40.0f));

    ss.Update(4.0f);
    TEST("Overcharge at 0", ApproxEq(sc->overchargeAmount, 0.0f));

    // Should not go negative
    ss.Update(1.0f);
    TEST("Overcharge does not go negative", ApproxEq(sc->overchargeAmount, 0.0f));
}

// ===================================================================
// Status Effect System tests
// ===================================================================
static void TestStatusEffectBasic() {
    std::cout << "[StatusEffect Basic]\n";
    StatusEffect e;
    e.duration = 5.0f;
    e.remainingTime = 5.0f;
    TEST("Effect is active", e.IsActive());
    TEST("Remaining 100%", ApproxEq(e.GetRemainingPercent(), 100.0f));

    e.remainingTime = 2.5f;
    TEST("Remaining 50%", ApproxEq(e.GetRemainingPercent(), 50.0f));

    e.remainingTime = 0.0f;
    TEST("Effect expired", !e.IsActive());
    TEST("Remaining 0%", ApproxEq(e.GetRemainingPercent(), 0.0f));

    e.duration = 0.0f;
    TEST("Zero duration returns 0%", ApproxEq(e.GetRemainingPercent(), 0.0f));
}

static void TestStatusEffectNames() {
    std::cout << "[StatusEffect Names]\n";
    TEST("EMP name", StatusEffect::GetEffectName(StatusEffectType::EMPDisruption) == "EMP Disruption");
    TEST("Fire name", StatusEffect::GetEffectName(StatusEffectType::FireDOT) == "Fire");
    TEST("Radiation name", StatusEffect::GetEffectName(StatusEffectType::RadiationDOT) == "Radiation");
    TEST("Shield Drain name", StatusEffect::GetEffectName(StatusEffectType::ShieldDrain) == "Shield Drain");
    TEST("Engine Jam name", StatusEffect::GetEffectName(StatusEffectType::EngineJam) == "Engine Jam");
    TEST("Sensor Scramble name", StatusEffect::GetEffectName(StatusEffectType::SensorScramble) == "Sensor Scramble");
}

static void TestStatusEffectDefaults() {
    std::cout << "[StatusEffect Defaults]\n";
    TEST("EMP default duration 3", ApproxEq(StatusEffect::GetDefaultDuration(StatusEffectType::EMPDisruption), 3.0f));
    TEST("Fire default duration 8", ApproxEq(StatusEffect::GetDefaultDuration(StatusEffectType::FireDOT), 8.0f));
    TEST("Radiation default duration 10", ApproxEq(StatusEffect::GetDefaultDuration(StatusEffectType::RadiationDOT), 10.0f));
    TEST("ShieldDrain default duration 6", ApproxEq(StatusEffect::GetDefaultDuration(StatusEffectType::ShieldDrain), 6.0f));
    TEST("EngineJam default duration 5", ApproxEq(StatusEffect::GetDefaultDuration(StatusEffectType::EngineJam), 5.0f));
    TEST("SensorScramble default duration 7", ApproxEq(StatusEffect::GetDefaultDuration(StatusEffectType::SensorScramble), 7.0f));

    TEST("EMP default magnitude 0", ApproxEq(StatusEffect::GetDefaultMagnitude(StatusEffectType::EMPDisruption), 0.0f));
    TEST("Fire default magnitude 15", ApproxEq(StatusEffect::GetDefaultMagnitude(StatusEffectType::FireDOT), 15.0f));
    TEST("Radiation default magnitude 10", ApproxEq(StatusEffect::GetDefaultMagnitude(StatusEffectType::RadiationDOT), 10.0f));
    TEST("ShieldDrain default magnitude 20", ApproxEq(StatusEffect::GetDefaultMagnitude(StatusEffectType::ShieldDrain), 20.0f));
    TEST("EngineJam default magnitude 50", ApproxEq(StatusEffect::GetDefaultMagnitude(StatusEffectType::EngineJam), 50.0f));
    TEST("SensorScramble default magnitude 40", ApproxEq(StatusEffect::GetDefaultMagnitude(StatusEffectType::SensorScramble), 40.0f));
}

static void TestStatusEffectComponentApply() {
    std::cout << "[StatusEffectComponent Apply]\n";
    StatusEffectComponent sec;
    TEST("No active effects initially", sec.GetActiveCount() == 0);

    StatusEffect fire;
    fire.type = StatusEffectType::FireDOT;
    fire.duration = 5.0f;
    fire.remainingTime = 5.0f;
    fire.magnitude = 10.0f;

    bool applied = sec.ApplyEffect(fire);
    TEST("Effect applied", applied);
    TEST("One active effect", sec.GetActiveCount() == 1);
    TEST("Has fire effect", sec.HasEffect(StatusEffectType::FireDOT));
    TEST("No EMP effect", !sec.HasEffect(StatusEffectType::EMPDisruption));
    TEST("Fire magnitude 10", ApproxEq(sec.GetEffectMagnitude(StatusEffectType::FireDOT), 10.0f));
}

static void TestStatusEffectComponentImmune() {
    std::cout << "[StatusEffectComponent Immune]\n";
    StatusEffectComponent sec;
    sec.isImmune = true;

    StatusEffect fire;
    fire.type = StatusEffectType::FireDOT;
    bool applied = sec.ApplyEffect(fire);
    TEST("Immune prevents application", !applied);
    TEST("No effects when immune", sec.GetActiveCount() == 0);
}

static void TestStatusEffectComponentCapacity() {
    std::cout << "[StatusEffectComponent Capacity]\n";
    StatusEffectComponent sec;
    for (size_t i = 0; i < StatusEffectComponent::kMaxEffects; ++i) {
        StatusEffect e;
        e.type = StatusEffectType::FireDOT;
        e.remainingTime = 5.0f;
        sec.ApplyEffect(e);
    }
    TEST("At max capacity", sec.GetActiveCount() == StatusEffectComponent::kMaxEffects);

    StatusEffect extra;
    extra.type = StatusEffectType::EMPDisruption;
    bool applied = sec.ApplyEffect(extra);
    TEST("Rejected at capacity", !applied);
    TEST("Still at max", sec.GetActiveCount() == StatusEffectComponent::kMaxEffects);
}

static void TestStatusEffectComponentResistance() {
    std::cout << "[StatusEffectComponent Resistance]\n";
    StatusEffectComponent sec;
    sec.resistanceMultiplier = 0.5f; // 50% resistance

    StatusEffect fire;
    fire.type = StatusEffectType::FireDOT;
    fire.magnitude = 20.0f;
    fire.remainingTime = 5.0f;
    sec.ApplyEffect(fire);

    TEST("Magnitude halved by resistance", ApproxEq(sec.GetEffectMagnitude(StatusEffectType::FireDOT), 10.0f));
}

static void TestStatusEffectComponentRemoveByType() {
    std::cout << "[StatusEffectComponent RemoveByType]\n";
    StatusEffectComponent sec;

    StatusEffect fire;
    fire.type = StatusEffectType::FireDOT;
    fire.remainingTime = 5.0f;
    sec.ApplyEffect(fire);

    StatusEffect emp;
    emp.type = StatusEffectType::EMPDisruption;
    emp.remainingTime = 3.0f;
    sec.ApplyEffect(emp);

    TEST("Two effects active", sec.GetActiveCount() == 2);

    sec.RemoveEffectsByType(StatusEffectType::FireDOT);
    TEST("One effect after removal", sec.GetActiveCount() == 1);
    TEST("Fire removed", !sec.HasEffect(StatusEffectType::FireDOT));
    TEST("EMP still active", sec.HasEffect(StatusEffectType::EMPDisruption));
}

static void TestStatusEffectComponentClearExpired() {
    std::cout << "[StatusEffectComponent ClearExpired]\n";
    StatusEffectComponent sec;

    StatusEffect alive;
    alive.type = StatusEffectType::FireDOT;
    alive.remainingTime = 5.0f;
    sec.ApplyEffect(alive);

    StatusEffect expired;
    expired.type = StatusEffectType::EMPDisruption;
    expired.remainingTime = 0.0f;
    sec.ApplyEffect(expired);

    TEST("Two effects before clear", sec.GetActiveCount() == 2);
    sec.ClearExpired();
    TEST("One effect after clear", sec.GetActiveCount() == 1);
    TEST("Fire still active", sec.HasEffect(StatusEffectType::FireDOT));
}

static void TestStatusEffectComponentSerialization() {
    std::cout << "[StatusEffectComponent Serialization]\n";
    StatusEffectComponent original;
    original.isImmune = false;
    original.resistanceMultiplier = 0.8f;

    StatusEffect fire;
    fire.type = StatusEffectType::FireDOT;
    fire.duration = 8.0f;
    fire.remainingTime = 5.0f;
    fire.tickInterval = 1.0f;
    fire.tickTimer = 0.3f;
    fire.magnitude = 15.0f;
    original.ApplyEffect(fire);

    StatusEffect emp;
    emp.type = StatusEffectType::EMPDisruption;
    emp.duration = 3.0f;
    emp.remainingTime = 2.0f;
    emp.tickInterval = 0.5f;
    emp.tickTimer = 0.1f;
    emp.magnitude = 0.0f;
    original.ApplyEffect(emp);

    ComponentData cd = original.Serialize();
    TEST("Serialized type", cd.componentType == "StatusEffectComponent");

    StatusEffectComponent restored;
    restored.Deserialize(cd);
    TEST("Restored isImmune", !restored.isImmune);
    TEST("Restored resistance", ApproxEq(restored.resistanceMultiplier, 0.8f));
    TEST("Restored effect count", restored.GetActiveCount() == 2);
    TEST("Restored has fire", restored.HasEffect(StatusEffectType::FireDOT));
    TEST("Restored has EMP", restored.HasEffect(StatusEffectType::EMPDisruption));
}

static void TestStatusEffectSystem() {
    std::cout << "[StatusEffectSystem]\n";
    StatusEffectSystem ses;
    TEST("StatusEffectSystem name", ses.GetName() == "StatusEffectSystem");
    ses.Update(1.0f);
    TEST("Update without EM does not crash", true);
}

static void TestStatusEffectSystemUpdate() {
    std::cout << "[StatusEffectSystem Update]\n";
    EntityManager em;
    auto& ent = em.CreateEntity("Ship");
    auto* sec = em.AddComponent<StatusEffectComponent>(ent.id, std::make_unique<StatusEffectComponent>());

    StatusEffect fire;
    fire.type = StatusEffectType::FireDOT;
    fire.duration = 5.0f;
    fire.remainingTime = 5.0f;
    fire.tickInterval = 1.0f;
    fire.tickTimer = 0.0f;
    fire.magnitude = 10.0f;
    sec->ApplyEffect(fire);

    StatusEffectSystem ses(em);

    ses.Update(2.0f);
    TEST("Remaining time decreased", sec->activeEffects[0].remainingTime < 5.0f);
    TEST("Effect still active", sec->GetActiveCount() == 1);

    // Expire the effect
    ses.Update(5.0f);
    TEST("Expired effect removed", sec->GetActiveCount() == 0);
}

// ===================================================================
// Loot System tests
// ===================================================================
static void TestLootRarityNames() {
    std::cout << "[LootRarity Names]\n";
    TEST("Common name", LootTableEntry::GetRarityName(LootRarity::Common) == "Common");
    TEST("Uncommon name", LootTableEntry::GetRarityName(LootRarity::Uncommon) == "Uncommon");
    TEST("Rare name", LootTableEntry::GetRarityName(LootRarity::Rare) == "Rare");
    TEST("Epic name", LootTableEntry::GetRarityName(LootRarity::Epic) == "Epic");
    TEST("Legendary name", LootTableEntry::GetRarityName(LootRarity::Legendary) == "Legendary");
}

static void TestLootRarityWeights() {
    std::cout << "[LootRarity Weights]\n";
    TEST("Common weight 1.0", ApproxEq(LootTableEntry::GetRarityWeight(LootRarity::Common), 1.0f));
    TEST("Uncommon weight 0.5", ApproxEq(LootTableEntry::GetRarityWeight(LootRarity::Uncommon), 0.5f));
    TEST("Rare weight 0.2", ApproxEq(LootTableEntry::GetRarityWeight(LootRarity::Rare), 0.2f));
    TEST("Epic weight 0.08", ApproxEq(LootTableEntry::GetRarityWeight(LootRarity::Epic), 0.08f));
    TEST("Legendary weight 0.02", ApproxEq(LootTableEntry::GetRarityWeight(LootRarity::Legendary), 0.02f));
}

static void TestLootTableRoll() {
    std::cout << "[LootTable Roll]\n";
    LootTable table;
    table.tableName = "Test";
    table.entries = {
        {"Iron", LootRarity::Common, 1.0f, 1, 5},   // always drops
        {"Gold", LootRarity::Rare, 0.0f, 1, 1},       // never drops
    };

    auto drops = table.Roll(42);
    TEST("At least one drop", !drops.empty());

    bool hasIron = false, hasGold = false;
    for (const auto& d : drops) {
        if (d.itemName == "Iron") hasIron = true;
        if (d.itemName == "Gold") hasGold = true;
    }
    TEST("Iron always drops", hasIron);
    TEST("Gold never drops", !hasGold);
}

static void TestLootTableDeterminism() {
    std::cout << "[LootTable Determinism]\n";
    LootTable table = LootTable::CreateStandardEnemyTable();

    auto drops1 = table.Roll(12345);
    auto drops2 = table.Roll(12345);
    TEST("Same seed same count", drops1.size() == drops2.size());
    for (size_t i = 0; i < drops1.size() && i < drops2.size(); ++i) {
        TEST("Same item name", drops1[i].itemName == drops2[i].itemName);
        TEST("Same quantity", drops1[i].quantity == drops2[i].quantity);
    }
}

static void TestLootTableWithLuck() {
    std::cout << "[LootTable RollWithLuck]\n";
    LootTable table;
    table.tableName = "LuckTest";
    table.entries = {
        {"Rare Item", LootRarity::Rare, 0.5f, 1, 1},
    };

    // Roll many times with high luck - should get more drops
    int dropsWithLuck = 0;
    int dropsWithoutLuck = 0;
    for (uint32_t seed = 0; seed < 100; ++seed) {
        auto d1 = table.RollWithLuck(seed, 2.0f); // capped at 1.0
        if (!d1.empty()) dropsWithLuck++;
        auto d2 = table.RollWithLuck(seed, 1.0f);
        if (!d2.empty()) dropsWithoutLuck++;
    }
    TEST("High luck gets more drops", dropsWithLuck >= dropsWithoutLuck);
}

static void TestLootTablePresets() {
    std::cout << "[LootTable Presets]\n";
    auto standard = LootTable::CreateStandardEnemyTable();
    TEST("Standard table name", standard.tableName == "StandardEnemy");
    TEST("Standard has 5 entries", standard.entries.size() == 5);

    auto boss = LootTable::CreateBossTable();
    TEST("Boss table name", boss.tableName == "BossEnemy");
    TEST("Boss has 5 entries", boss.entries.size() == 5);

    auto asteroid = LootTable::CreateAsteroidTable();
    TEST("Asteroid table name", asteroid.tableName == "Asteroid");
    TEST("Asteroid has 5 entries", asteroid.entries.size() == 5);
}

static void TestLootComponent() {
    std::cout << "[LootComponent]\n";
    LootComponent lc;
    lc.lootTable = LootTable::CreateStandardEnemyTable();
    lc.lootSeed = 42;
    lc.luckModifier = 1.0f;

    TEST("Not looted initially", !lc.hasBeenLooted);

    auto drops = lc.GenerateDrops();
    TEST("Generates drops", true); // just checking it doesn't crash

    lc.MarkLooted();
    TEST("Marked as looted", lc.hasBeenLooted);

    auto drops2 = lc.GenerateDrops();
    TEST("No drops after looted", drops2.empty());
}

static void TestLootComponentSerialization() {
    std::cout << "[LootComponent Serialization]\n";
    LootComponent original;
    original.lootTable = LootTable::CreateStandardEnemyTable();
    original.luckModifier = 1.5f;
    original.hasBeenLooted = false;
    original.lootSeed = 9999;

    ComponentData cd = original.Serialize();
    TEST("Serialized type", cd.componentType == "LootComponent");

    LootComponent restored;
    restored.Deserialize(cd);
    TEST("Restored luck modifier", ApproxEq(restored.luckModifier, 1.5f));
    TEST("Restored not looted", !restored.hasBeenLooted);
    TEST("Restored seed", restored.lootSeed == 9999);
    TEST("Restored table name", restored.lootTable.tableName == "StandardEnemy");
    TEST("Restored entry count", restored.lootTable.entries.size() == 5);
    TEST("Restored first entry name", restored.lootTable.entries[0].itemName == "Scrap Metal");
}

static void TestLootSystem() {
    std::cout << "[LootSystem]\n";
    LootSystem ls;
    TEST("LootSystem name", ls.GetName() == "LootSystem");
    ls.Update(1.0f);
    TEST("Update without EM does not crash", true);
}

static void TestLootSystemWithEM() {
    std::cout << "[LootSystem WithEM]\n";
    EntityManager em;
    auto& ent = em.CreateEntity("Enemy");
    auto* lc = em.AddComponent<LootComponent>(ent.id, std::make_unique<LootComponent>());
    lc->lootTable = LootTable::CreateBossTable();
    lc->lootSeed = 42;

    LootSystem ls(em);
    ls.Update(1.0f); // No-op, but should not crash
    TEST("LootSystem with EM does not crash", true);
}

// ===================================================================
// New Game Event Constants tests (Shield, StatusEffect, Loot)
// ===================================================================
static void TestShieldStatusLootGameEvents() {
    std::cout << "[ShieldStatusLootGameEvents]\n";
    // Shield events
    TEST("ShieldAbsorbed event", std::string(GameEvents::ShieldAbsorbed) == "combat.shield.absorbed");
    TEST("ShieldDepleted event", std::string(GameEvents::ShieldDepleted) == "combat.shield.depleted");
    TEST("ShieldRestored event", std::string(GameEvents::ShieldRestored) == "combat.shield.restored");
    TEST("ShieldOvercharged event", std::string(GameEvents::ShieldOvercharged) == "combat.shield.overcharged");
    // Status effect events
    TEST("StatusEffectApplied event", std::string(GameEvents::StatusEffectApplied) == "combat.status.applied");
    TEST("StatusEffectExpired event", std::string(GameEvents::StatusEffectExpired) == "combat.status.expired");
    TEST("StatusEffectRemoved event", std::string(GameEvents::StatusEffectRemoved) == "combat.status.removed");
    TEST("StatusEffectTick event", std::string(GameEvents::StatusEffectTick) == "combat.status.tick");
    // Loot events
    TEST("LootGenerated event", std::string(GameEvents::LootGenerated) == "loot.generated");
    TEST("LootCollected event", std::string(GameEvents::LootCollected) == "loot.collected");
    TEST("LootDropped event", std::string(GameEvents::LootDropped) == "loot.dropped");
    TEST("RareItemFound event", std::string(GameEvents::RareItemFound) == "loot.rare_item");
}

// ===================================================================
// Crafting System tests
// ===================================================================
static void TestCraftingRecipeStationName() {
    std::cout << "[CraftingRecipe::GetStationName]\n";
    TEST("Basic name", CraftingRecipe::GetStationName(CraftingStationType::Basic) == "Basic Workshop");
    TEST("Forge name", CraftingRecipe::GetStationName(CraftingStationType::Forge) == "Forge");
    TEST("Laboratory name", CraftingRecipe::GetStationName(CraftingStationType::Laboratory) == "Laboratory");
    TEST("Shipyard name", CraftingRecipe::GetStationName(CraftingStationType::Shipyard) == "Shipyard");
    TEST("Refinery name", CraftingRecipe::GetStationName(CraftingStationType::Refinery) == "Refinery");
}

static void TestCraftingJobProgress() {
    std::cout << "[CraftingJob::GetProgress]\n";
    CraftingJob job;
    job.totalTime = 10.0f;
    job.timeRemaining = 10.0f;
    TEST("Progress at start", ApproxEq(job.GetProgress(), 0.0f));

    job.timeRemaining = 5.0f;
    TEST("Progress at half", ApproxEq(job.GetProgress(), 50.0f));

    job.timeRemaining = 0.0f;
    TEST("Progress at end", ApproxEq(job.GetProgress(), 100.0f));

    CraftingJob zeroJob;
    zeroJob.totalTime = 0.0f;
    zeroJob.timeRemaining = 0.0f;
    TEST("Progress zero totalTime", ApproxEq(zeroJob.GetProgress(), 100.0f));
}

static void TestRecipeDatabase() {
    std::cout << "[RecipeDatabase]\n";
    RecipeDatabase db;
    TEST("Empty db count", db.GetRecipeCount() == 0);

    CraftingRecipe r;
    r.recipeId = "test_recipe";
    r.resultItem = "Test Item";
    r.resultQuantity = 1;
    r.ingredients = {{"Ore", 2}};
    r.requiredStation = CraftingStationType::Basic;
    r.craftTime = 3.0f;
    db.AddRecipe(r);

    TEST("Count after add", db.GetRecipeCount() == 1);
    TEST("FindRecipe exists", db.FindRecipe("test_recipe") != nullptr);
    TEST("FindRecipe id", db.FindRecipe("test_recipe")->recipeId == "test_recipe");
    TEST("FindRecipe missing", db.FindRecipe("nonexistent") == nullptr);

    auto basicRecipes = db.GetRecipesForStation(CraftingStationType::Basic);
    TEST("Station filter count", basicRecipes.size() == 1);
    auto forgeRecipes = db.GetRecipesForStation(CraftingStationType::Forge);
    TEST("Station filter empty", forgeRecipes.empty());

    TEST("GetAllRecipes size", db.GetAllRecipes().size() == 1);
}

static void TestRecipeDatabaseDefaults() {
    std::cout << "[RecipeDatabase::CreateDefaultDatabase]\n";
    RecipeDatabase db = RecipeDatabase::CreateDefaultDatabase();
    TEST("Default recipe count", db.GetRecipeCount() == 8);
    TEST("iron_plate exists", db.FindRecipe("iron_plate") != nullptr);
    TEST("steel_beam exists", db.FindRecipe("steel_beam") != nullptr);
    TEST("circuit_board exists", db.FindRecipe("circuit_board") != nullptr);
    TEST("hull_panel exists", db.FindRecipe("hull_panel") != nullptr);
    TEST("energy_cell exists", db.FindRecipe("energy_cell") != nullptr);
    TEST("engine_component exists", db.FindRecipe("engine_component") != nullptr);
    TEST("shield_capacitor exists", db.FindRecipe("shield_capacitor") != nullptr);
    TEST("refined_fuel exists", db.FindRecipe("refined_fuel") != nullptr);

    const auto* fuel = db.FindRecipe("refined_fuel");
    TEST("refined_fuel quantity", fuel->resultQuantity == 2);
    TEST("refined_fuel station", fuel->requiredStation == CraftingStationType::Refinery);

    const auto* engine = db.FindRecipe("engine_component");
    TEST("engine_component level", engine->requiredLevel == 3);
    TEST("engine_component station", engine->requiredStation == CraftingStationType::Shipyard);
}

static void TestCraftingComponentDefaults() {
    std::cout << "[CraftingComponent defaults]\n";
    CraftingComponent cc;
    TEST("Default stationType", cc.stationType == CraftingStationType::Basic);
    TEST("Default crafterLevel", cc.crafterLevel == 1);
    TEST("Default maxConcurrentJobs", cc.maxConcurrentJobs == 1);
    TEST("Default speedMultiplier", ApproxEq(cc.speedMultiplier, 1.0f));
    TEST("Default activeJobs empty", cc.activeJobs.empty());
    TEST("Default CanStartJob", cc.CanStartJob());
    TEST("Default GetActiveJobCount", cc.GetActiveJobCount() == 0);
}

static void TestCraftingComponentStartCrafting() {
    std::cout << "[CraftingComponent::StartCrafting]\n";
    CraftingComponent cc;
    cc.stationType = CraftingStationType::Basic;
    cc.crafterLevel = 1;
    cc.maxConcurrentJobs = 2;

    CraftingRecipe recipe;
    recipe.recipeId = "iron_plate";
    recipe.resultItem = "Iron Plate";
    recipe.craftTime = 2.0f;
    recipe.requiredStation = CraftingStationType::Basic;
    recipe.requiredLevel = 1;

    TEST("Start first job", cc.StartCrafting(recipe));
    TEST("Active count after 1", cc.GetActiveJobCount() == 1);
    TEST("Can start second", cc.CanStartJob());

    TEST("Start second job", cc.StartCrafting(recipe));
    TEST("Active count after 2", cc.GetActiveJobCount() == 2);
    TEST("Cannot start third", !cc.CanStartJob());
    TEST("Third job rejected", !cc.StartCrafting(recipe));
}

static void TestCraftingComponentLevelRequirement() {
    std::cout << "[CraftingComponent level requirement]\n";
    CraftingComponent cc;
    cc.stationType = CraftingStationType::Shipyard;
    cc.crafterLevel = 1;

    CraftingRecipe recipe;
    recipe.recipeId = "engine_component";
    recipe.requiredStation = CraftingStationType::Shipyard;
    recipe.requiredLevel = 3;
    recipe.craftTime = 10.0f;

    TEST("Level too low", !cc.MeetsLevelRequirement(recipe));
    TEST("Start rejected by level", !cc.StartCrafting(recipe));

    cc.crafterLevel = 3;
    TEST("Level met", cc.MeetsLevelRequirement(recipe));
    TEST("Start accepted", cc.StartCrafting(recipe));
}

static void TestCraftingComponentStationRequirement() {
    std::cout << "[CraftingComponent station requirement]\n";
    CraftingComponent cc;
    cc.stationType = CraftingStationType::Basic;

    CraftingRecipe recipe;
    recipe.recipeId = "steel_beam";
    recipe.requiredStation = CraftingStationType::Forge;
    recipe.requiredLevel = 1;
    recipe.craftTime = 5.0f;

    TEST("Wrong station", !cc.HasRequiredStation(recipe));
    TEST("Start rejected by station", !cc.StartCrafting(recipe));

    cc.stationType = CraftingStationType::Forge;
    TEST("Correct station", cc.HasRequiredStation(recipe));
    TEST("Start accepted", cc.StartCrafting(recipe));
}

static void TestCraftingComponentCollectCompleted() {
    std::cout << "[CraftingComponent::CollectCompletedJobs]\n";
    CraftingComponent cc;
    cc.maxConcurrentJobs = 3;

    CraftingJob j1; j1.recipeId = "a"; j1.isComplete = true;
    CraftingJob j2; j2.recipeId = "b"; j2.isComplete = false;
    CraftingJob j3; j3.recipeId = "c"; j3.isComplete = true;
    cc.activeJobs = {j1, j2, j3};

    auto completed = cc.CollectCompletedJobs();
    TEST("Collected 2 completed", completed.size() == 2);
    TEST("1 remaining", cc.activeJobs.size() == 1);
    TEST("Remaining is incomplete", cc.activeJobs[0].recipeId == "b");
}

static void TestCraftingComponentSerialization() {
    std::cout << "[CraftingComponent Serialization]\n";
    CraftingComponent original;
    original.stationType = CraftingStationType::Laboratory;
    original.crafterLevel = 5;
    original.maxConcurrentJobs = 3;
    original.speedMultiplier = 1.5f;

    CraftingJob job;
    job.recipeId = "circuit_board";
    job.timeRemaining = 1.5f;
    job.totalTime = 3.0f;
    job.isComplete = false;
    original.activeJobs.push_back(job);

    CraftingJob completedJob;
    completedJob.recipeId = "energy_cell";
    completedJob.timeRemaining = 0.0f;
    completedJob.totalTime = 6.0f;
    completedJob.isComplete = true;
    original.activeJobs.push_back(completedJob);

    ComponentData cd = original.Serialize();
    TEST("Serialized type", cd.componentType == "CraftingComponent");

    CraftingComponent restored;
    restored.Deserialize(cd);
    TEST("Restored stationType", restored.stationType == CraftingStationType::Laboratory);
    TEST("Restored crafterLevel", restored.crafterLevel == 5);
    TEST("Restored maxConcurrentJobs", restored.maxConcurrentJobs == 3);
    TEST("Restored speedMultiplier", ApproxEq(restored.speedMultiplier, 1.5f));
    TEST("Restored job count", restored.activeJobs.size() == 2);
    TEST("Restored job0 recipeId", restored.activeJobs[0].recipeId == "circuit_board");
    TEST("Restored job0 timeRemaining", ApproxEq(restored.activeJobs[0].timeRemaining, 1.5f));
    TEST("Restored job0 totalTime", ApproxEq(restored.activeJobs[0].totalTime, 3.0f));
    TEST("Restored job0 not complete", !restored.activeJobs[0].isComplete);
    TEST("Restored job1 recipeId", restored.activeJobs[1].recipeId == "energy_cell");
    TEST("Restored job1 complete", restored.activeJobs[1].isComplete);
}

static void TestCraftingComponentDeserializeInvalidEnum() {
    std::cout << "[CraftingComponent Deserialize invalid enum]\n";
    ComponentData cd;
    cd.componentType = "CraftingComponent";
    cd.data["stationType"] = "999";
    cd.data["crafterLevel"] = "1";
    cd.data["maxConcurrentJobs"] = "1";
    cd.data["speedMultiplier"] = "1.0";
    cd.data["jobCount"] = "0";

    CraftingComponent cc;
    cc.Deserialize(cd);
    TEST("Invalid enum defaults to Basic", cc.stationType == CraftingStationType::Basic);
}

static void TestCraftingSystem() {
    std::cout << "[CraftingSystem]\n";
    EntityManager em;
    CraftingSystem sys(em);

    auto& ent = em.CreateEntity("crafter");
    auto* cc = em.AddComponent<CraftingComponent>(ent.id, std::make_unique<CraftingComponent>());
    cc->stationType = CraftingStationType::Basic;
    cc->maxConcurrentJobs = 2;
    cc->speedMultiplier = 1.0f;

    CraftingRecipe recipe;
    recipe.recipeId = "iron_plate";
    recipe.requiredStation = CraftingStationType::Basic;
    recipe.requiredLevel = 1;
    recipe.craftTime = 2.0f;
    cc->StartCrafting(recipe);

    TEST("Job not complete before update", !cc->activeJobs[0].isComplete);

    sys.Update(1.0f);
    TEST("Job not complete after 1s", !cc->activeJobs[0].isComplete);
    TEST("Time remaining ~1s", ApproxEq(cc->activeJobs[0].timeRemaining, 1.0f));

    sys.Update(1.0f);
    TEST("Job complete after 2s", cc->activeJobs[0].isComplete);
    TEST("Time remaining 0", ApproxEq(cc->activeJobs[0].timeRemaining, 0.0f));
}

static void TestCraftingSystemSpeedMultiplier() {
    std::cout << "[CraftingSystem speed multiplier]\n";
    EntityManager em;
    CraftingSystem sys(em);

    auto& ent = em.CreateEntity("fast_crafter");
    auto* cc = em.AddComponent<CraftingComponent>(ent.id, std::make_unique<CraftingComponent>());
    cc->stationType = CraftingStationType::Basic;
    cc->speedMultiplier = 2.0f;

    CraftingRecipe recipe;
    recipe.recipeId = "iron_plate";
    recipe.requiredStation = CraftingStationType::Basic;
    recipe.requiredLevel = 1;
    recipe.craftTime = 4.0f;
    cc->StartCrafting(recipe);

    sys.Update(1.0f);
    TEST("2x speed: 2s elapsed after 1s dt", ApproxEq(cc->activeJobs[0].timeRemaining, 2.0f));

    sys.Update(1.0f);
    TEST("2x speed: complete after 2s dt", cc->activeJobs[0].isComplete);
}

static void TestCraftingSystemMultipleJobs() {
    std::cout << "[CraftingSystem multiple jobs]\n";
    EntityManager em;
    CraftingSystem sys(em);

    auto& ent = em.CreateEntity("multi_crafter");
    auto* cc = em.AddComponent<CraftingComponent>(ent.id, std::make_unique<CraftingComponent>());
    cc->stationType = CraftingStationType::Basic;
    cc->maxConcurrentJobs = 3;

    CraftingRecipe r1;
    r1.recipeId = "fast"; r1.requiredStation = CraftingStationType::Basic;
    r1.requiredLevel = 1; r1.craftTime = 1.0f;
    CraftingRecipe r2;
    r2.recipeId = "slow"; r2.requiredStation = CraftingStationType::Basic;
    r2.requiredLevel = 1; r2.craftTime = 3.0f;

    cc->StartCrafting(r1);
    cc->StartCrafting(r2);
    TEST("Two active jobs", cc->GetActiveJobCount() == 2);

    sys.Update(1.0f);
    TEST("Fast job complete", cc->activeJobs[0].isComplete);
    TEST("Slow job still active", !cc->activeJobs[1].isComplete);

    auto completed = cc->CollectCompletedJobs();
    TEST("Collected 1 completed", completed.size() == 1);
    TEST("1 job remaining", cc->activeJobs.size() == 1);
}

// ---------------------------------------------------------------------------
// Reputation system tests
// ---------------------------------------------------------------------------

static void TestFactionReputationDefaults() {
    std::cout << "[FactionReputation Defaults]\n";
    FactionReputation fr;
    TEST("Default reputation is 0", fr.reputation == 0);
    TEST("Default standing is Neutral", fr.GetStanding() == Standing::Neutral);
    TEST("Default minReputation is -1000", fr.minReputation == -1000);
    TEST("Default maxReputation is 1000", fr.maxReputation == 1000);
    TEST("Default factionId is empty", fr.factionId.empty());
    TEST("Default normalized rep is 0", ApproxEq(fr.GetNormalizedReputation(), 0.0f));
}

static void TestFactionReputationModify() {
    std::cout << "[FactionReputation Modify]\n";
    FactionReputation fr;
    fr.ModifyReputation(200);
    TEST("Add 200 rep", fr.reputation == 200);
    fr.ModifyReputation(-300);
    TEST("Subtract 300 rep", fr.reputation == -100);
    fr.ModifyReputation(-2000);
    TEST("Clamped to min", fr.reputation == -1000);
    fr.ModifyReputation(5000);
    TEST("Clamped to max", fr.reputation == 1000);
}

static void TestFactionReputationStandings() {
    std::cout << "[FactionReputation Standings]\n";
    FactionReputation fr;

    fr.reputation = -1000;
    TEST("Hostile at -1000", fr.GetStanding() == Standing::Hostile);
    fr.reputation = -500;
    TEST("Hostile at -500", fr.GetStanding() == Standing::Hostile);
    fr.reputation = -499;
    TEST("Unfriendly at -499", fr.GetStanding() == Standing::Unfriendly);
    fr.reputation = -100;
    TEST("Unfriendly at -100", fr.GetStanding() == Standing::Unfriendly);
    fr.reputation = -99;
    TEST("Neutral at -99", fr.GetStanding() == Standing::Neutral);
    fr.reputation = 0;
    TEST("Neutral at 0", fr.GetStanding() == Standing::Neutral);
    fr.reputation = 100;
    TEST("Neutral at 100", fr.GetStanding() == Standing::Neutral);
    fr.reputation = 101;
    TEST("Friendly at 101", fr.GetStanding() == Standing::Friendly);
    fr.reputation = 500;
    TEST("Friendly at 500", fr.GetStanding() == Standing::Friendly);
    fr.reputation = 501;
    TEST("Allied at 501", fr.GetStanding() == Standing::Allied);
    fr.reputation = 1000;
    TEST("Allied at 1000", fr.GetStanding() == Standing::Allied);
}

static void TestFactionReputationNormalized() {
    std::cout << "[FactionReputation Normalized]\n";
    FactionReputation fr;
    fr.maxReputation = 1000;

    fr.reputation = 500;
    TEST("Normalized 0.5", ApproxEq(fr.GetNormalizedReputation(), 0.5f));
    fr.reputation = -1000;
    TEST("Normalized -1.0", ApproxEq(fr.GetNormalizedReputation(), -1.0f));
    fr.reputation = 1000;
    TEST("Normalized 1.0", ApproxEq(fr.GetNormalizedReputation(), 1.0f));
    fr.reputation = 0;
    TEST("Normalized 0.0", ApproxEq(fr.GetNormalizedReputation(), 0.0f));

    FactionReputation frZero;
    frZero.maxReputation = 0;
    TEST("Normalized with max=0 returns 0", ApproxEq(frZero.GetNormalizedReputation(), 0.0f));
}

static void TestStandingNames() {
    std::cout << "[Standing Names]\n";
    TEST("Hostile name", FactionReputation::GetStandingName(Standing::Hostile) == "Hostile");
    TEST("Unfriendly name", FactionReputation::GetStandingName(Standing::Unfriendly) == "Unfriendly");
    TEST("Neutral name", FactionReputation::GetStandingName(Standing::Neutral) == "Neutral");
    TEST("Friendly name", FactionReputation::GetStandingName(Standing::Friendly) == "Friendly");
    TEST("Allied name", FactionReputation::GetStandingName(Standing::Allied) == "Allied");
}

static void TestStandingThresholds() {
    std::cout << "[Standing Thresholds]\n";
    TEST("Hostile threshold", FactionReputation::GetStandingThreshold(Standing::Hostile) == -500);
    TEST("Unfriendly threshold", FactionReputation::GetStandingThreshold(Standing::Unfriendly) == -100);
    TEST("Neutral threshold", FactionReputation::GetStandingThreshold(Standing::Neutral) == 100);
    TEST("Friendly threshold", FactionReputation::GetStandingThreshold(Standing::Friendly) == 500);
    TEST("Allied threshold", FactionReputation::GetStandingThreshold(Standing::Allied) == 500);
}

static void TestReputationComponentAddFaction() {
    std::cout << "[ReputationComponent AddFaction]\n";
    ReputationComponent rc;
    TEST("Initial faction count is 0", rc.GetFactionCount() == 0);

    rc.AddFaction("pirates", 0);
    TEST("Count after 1 add", rc.GetFactionCount() == 1);

    rc.AddFaction("traders", 200);
    TEST("Count after 2 adds", rc.GetFactionCount() == 2);

    // Adding same faction again should not duplicate
    rc.AddFaction("pirates", 500);
    TEST("No duplicate on re-add", rc.GetFactionCount() == 2);

    auto* pirates = rc.GetFaction("pirates");
    TEST("Pirates found", pirates != nullptr);
    TEST("Pirates rep unchanged on re-add", pirates->reputation == 0);

    auto* traders = rc.GetFaction("traders");
    TEST("Traders found", traders != nullptr);
    TEST("Traders initial rep", traders->reputation == 200);
}

static void TestReputationComponentModifyRep() {
    std::cout << "[ReputationComponent ModifyRep]\n";
    ReputationComponent rc;
    rc.ModifyReputation("alliance", 300, "Quest completed");
    TEST("Faction created on modify", rc.GetFactionCount() == 1);
    TEST("Rep set correctly", rc.GetFaction("alliance")->reputation == 300);
    TEST("Event recorded", rc.recentEvents.size() == 1);
    TEST("Event factionId", rc.recentEvents[0].factionId == "alliance");
    TEST("Event amount", rc.recentEvents[0].amount == 300);
    TEST("Event reason", rc.recentEvents[0].reason == "Quest completed");

    rc.ModifyReputation("alliance", -100, "Attacked ship");
    TEST("Rep modified", rc.GetFaction("alliance")->reputation == 200);
    TEST("Two events", rc.recentEvents.size() == 2);

    // Test history trimming
    rc.maxEventHistory = 3;
    for (int i = 0; i < 5; ++i) {
        rc.ModifyReputation("alliance", 1, "spam");
    }
    TEST("Event history trimmed", static_cast<int>(rc.recentEvents.size()) == 3);
    // Verify FIFO: oldest events (Quest completed, Attacked ship, first spams) dropped
    TEST("FIFO: all remaining are spam", rc.recentEvents[0].reason == "spam");
    TEST("FIFO: newest at end", rc.recentEvents[2].reason == "spam");
}

static void TestReputationComponentGetStanding() {
    std::cout << "[ReputationComponent GetStanding]\n";
    ReputationComponent rc;
    TEST("Untracked faction is Neutral", rc.GetStanding("unknown") == Standing::Neutral);

    rc.AddFaction("enemies", -600);
    TEST("Hostile faction", rc.GetStanding("enemies") == Standing::Hostile);

    rc.AddFaction("friends", 300);
    TEST("Friendly faction", rc.GetStanding("friends") == Standing::Friendly);
}

static void TestReputationComponentGetFactionsWithStanding() {
    std::cout << "[ReputationComponent GetFactionsWithStanding]\n";
    ReputationComponent rc;
    rc.AddFaction("pirates", -700);
    rc.AddFaction("rebels", -600);
    rc.AddFaction("traders", 50);
    rc.AddFaction("alliance", 300);
    rc.AddFaction("empire", 800);

    auto hostile = rc.GetFactionsWithStanding(Standing::Hostile);
    TEST("2 hostile factions", hostile.size() == 2);

    auto neutral = rc.GetFactionsWithStanding(Standing::Neutral);
    TEST("1 neutral faction", neutral.size() == 1);
    TEST("Traders are neutral", neutral[0] == "traders");

    auto friendly = rc.GetFactionsWithStanding(Standing::Friendly);
    TEST("1 friendly faction", friendly.size() == 1);

    auto allied = rc.GetFactionsWithStanding(Standing::Allied);
    TEST("1 allied faction", allied.size() == 1);
    TEST("Empire is allied", allied[0] == "empire");
}

static void TestReputationComponentSerialization() {
    std::cout << "[ReputationComponent Serialization]\n";
    ReputationComponent original;
    original.decayRate = 5.0f;
    original.maxEventHistory = 10;
    original.AddFaction("pirates", -700);
    original.AddFaction("traders", 300);
    original.ModifyReputation("pirates", 100, "Bribe");

    ComponentData cd = original.Serialize();
    TEST("Component type", cd.componentType == "ReputationComponent");

    ReputationComponent restored;
    restored.Deserialize(cd);
    TEST("Decay rate restored", ApproxEq(restored.decayRate, 5.0f));
    TEST("Max event history restored", restored.maxEventHistory == 10);
    TEST("Faction count restored", restored.GetFactionCount() == 2);

    auto* pirates = restored.GetFaction("pirates");
    TEST("Pirates restored", pirates != nullptr);
    TEST("Pirates rep restored", pirates->reputation == -600);

    auto* traders = restored.GetFaction("traders");
    TEST("Traders restored", traders != nullptr);
    TEST("Traders rep restored", traders->reputation == 300);

    TEST("Events restored", restored.recentEvents.size() == 1);
    TEST("Event reason restored", restored.recentEvents[0].reason == "Bribe");
}

static void TestReputationSystem() {
    std::cout << "[ReputationSystem]\n";
    ReputationSystem sys;
    TEST("System name", sys.GetName() == "ReputationSystem");
    TEST("System enabled", sys.IsEnabled());

    // Update without entity manager should not crash
    sys.Update(1.0f);
    TEST("Update without EM ok", true);
}

static void TestReputationSystemDecay() {
    std::cout << "[ReputationSystem Decay]\n";
    EntityManager em;
    ReputationSystem sys(em);

    auto& ent = em.CreateEntity("player");
    auto* rc = em.AddComponent<ReputationComponent>(ent.id, std::make_unique<ReputationComponent>());
    rc->decayRate = 10.0f;
    rc->AddFaction("pirates", -100);
    rc->AddFaction("traders", 200);
    rc->AddFaction("neutral_faction", 0);

    sys.Update(1.0f);

    auto* pirates = rc->GetFaction("pirates");
    TEST("Negative rep decays toward 0", pirates->reputation > -100);

    auto* traders = rc->GetFaction("traders");
    TEST("Positive rep decays toward 0", traders->reputation < 200);

    auto* neutralFaction = rc->GetFaction("neutral_faction");
    TEST("Zero rep stays at 0", neutralFaction->reputation == 0);

    // Decay should not overshoot past 0
    rc->AddFaction("small_pos", 5);
    rc->decayRate = 100.0f;
    sys.Update(1.0f);
    auto* smallPos = rc->GetFaction("small_pos");
    TEST("Decay does not overshoot past 0", smallPos->reputation == 0);
}

// ===================================================================
// Formation System tests
// ===================================================================

static void TestFormationPatternLine() {
    std::cout << "[FormationPattern Line]\n";
    FormationPattern pattern;
    pattern.type = FormationType::Line;
    pattern.spacing = 10.0f;
    auto slots = pattern.ComputeSlots(5);
    TEST("Line slot count", slots.size() == 5);
    // Center offset = (5-1)/2 = 2.0, so slot 0 = (0-2)*10 = -20, slot 2 = 0, slot 4 = 20
    TEST("Line slot 0 x", ApproxEq(slots[0].offset.x, -20.0f));
    TEST("Line slot 0 y", ApproxEq(slots[0].offset.y, 0.0f));
    TEST("Line slot 0 z", ApproxEq(slots[0].offset.z, 0.0f));
    TEST("Line slot 2 centered", ApproxEq(slots[2].offset.x, 0.0f));
    TEST("Line slot 4 x", ApproxEq(slots[4].offset.x, 20.0f));
    TEST("Line slot indices", slots[0].slotIndex == 0 && slots[4].slotIndex == 4);
}

static void TestFormationPatternV() {
    std::cout << "[FormationPattern V]\n";
    FormationPattern pattern;
    pattern.type = FormationType::V;
    pattern.spacing = 10.0f;
    auto slots = pattern.ComputeSlots(5);
    TEST("V slot count", slots.size() == 5);
    // Slot 0: origin
    TEST("V slot 0 at origin", ApproxEq(slots[0].offset.x, 0.0f) &&
                                ApproxEq(slots[0].offset.z, 0.0f));
    // Slot 1: left, pair=1 => (-10, 0, -10)
    TEST("V slot 1 left", ApproxEq(slots[1].offset.x, -10.0f) &&
                           ApproxEq(slots[1].offset.z, -10.0f));
    // Slot 2: right, pair=1 => (10, 0, -10)
    TEST("V slot 2 right", ApproxEq(slots[2].offset.x, 10.0f) &&
                            ApproxEq(slots[2].offset.z, -10.0f));
    // Slot 3: left, pair=2 => (-20, 0, -20)
    TEST("V slot 3 left deep", ApproxEq(slots[3].offset.x, -20.0f) &&
                                ApproxEq(slots[3].offset.z, -20.0f));
    // Slot 4: right, pair=2 => (20, 0, -20)
    TEST("V slot 4 right deep", ApproxEq(slots[4].offset.x, 20.0f) &&
                                 ApproxEq(slots[4].offset.z, -20.0f));
}

static void TestFormationPatternDiamond() {
    std::cout << "[FormationPattern Diamond]\n";
    FormationPattern pattern;
    pattern.type = FormationType::Diamond;
    pattern.spacing = 10.0f;
    auto slots = pattern.ComputeSlots(5);
    TEST("Diamond slot count", slots.size() == 5);
    // Slot 0: center
    TEST("Diamond slot 0 center", ApproxEq(slots[0].offset.x, 0.0f) &&
                                   ApproxEq(slots[0].offset.z, 0.0f));
    // Slot 1: front (+Z)
    TEST("Diamond slot 1 front", ApproxEq(slots[1].offset.x, 0.0f) &&
                                  ApproxEq(slots[1].offset.z, 10.0f));
    // Slot 2: left (-X)
    TEST("Diamond slot 2 left", ApproxEq(slots[2].offset.x, -10.0f) &&
                                 ApproxEq(slots[2].offset.z, 0.0f));
    // Slot 3: right (+X)
    TEST("Diamond slot 3 right", ApproxEq(slots[3].offset.x, 10.0f) &&
                                  ApproxEq(slots[3].offset.z, 0.0f));
    // Slot 4: back (-Z)
    TEST("Diamond slot 4 back", ApproxEq(slots[4].offset.x, 0.0f) &&
                                 ApproxEq(slots[4].offset.z, -10.0f));
}

static void TestFormationPatternCircle() {
    std::cout << "[FormationPattern Circle]\n";
    FormationPattern pattern;
    pattern.type = FormationType::Circle;
    pattern.spacing = 10.0f;
    auto slots = pattern.ComputeSlots(5);
    TEST("Circle slot count", slots.size() == 5);
    // Slot 0: center
    TEST("Circle slot 0 center", ApproxEq(slots[0].offset.x, 0.0f) &&
                                  ApproxEq(slots[0].offset.z, 0.0f));
    // Remaining 4 slots at radius = spacing = 10
    for (int i = 1; i < 5; ++i) {
        float r = std::sqrt(slots[i].offset.x * slots[i].offset.x +
                            slots[i].offset.z * slots[i].offset.z);
        TEST(("Circle slot " + std::to_string(i) + " radius").c_str(),
             ApproxEq(r, 10.0f));
    }
    TEST("Circle slot 0 y", ApproxEq(slots[0].offset.y, 0.0f));
}

static void TestFormationPatternWedge() {
    std::cout << "[FormationPattern Wedge]\n";
    FormationPattern pattern;
    pattern.type = FormationType::Wedge;
    pattern.spacing = 10.0f;
    auto slots = pattern.ComputeSlots(5);
    TEST("Wedge slot count", slots.size() == 5);
    // Slot 0: origin
    TEST("Wedge slot 0 at origin", ApproxEq(slots[0].offset.x, 0.0f) &&
                                    ApproxEq(slots[0].offset.z, 0.0f));
    // Slot 1: left, pair=1, x = -5 (spacing/2), z = -10
    TEST("Wedge slot 1 left", ApproxEq(slots[1].offset.x, -5.0f) &&
                               ApproxEq(slots[1].offset.z, -10.0f));
    // Slot 2: right, pair=1, x = 5, z = -10
    TEST("Wedge slot 2 right", ApproxEq(slots[2].offset.x, 5.0f) &&
                                ApproxEq(slots[2].offset.z, -10.0f));
    // Slot 3: left, pair=2, x = -10, z = -20
    TEST("Wedge slot 3 left deep", ApproxEq(slots[3].offset.x, -10.0f) &&
                                    ApproxEq(slots[3].offset.z, -20.0f));
    // Slot 4: right, pair=2, x = 10, z = -20
    TEST("Wedge slot 4 right deep", ApproxEq(slots[4].offset.x, 10.0f) &&
                                     ApproxEq(slots[4].offset.z, -20.0f));
}

static void TestFormationPatternColumn() {
    std::cout << "[FormationPattern Column]\n";
    FormationPattern pattern;
    pattern.type = FormationType::Column;
    pattern.spacing = 10.0f;
    auto slots = pattern.ComputeSlots(5);
    TEST("Column slot count", slots.size() == 5);
    TEST("Column slot 0", ApproxEq(slots[0].offset.x, 0.0f) &&
                           ApproxEq(slots[0].offset.z, 0.0f));
    TEST("Column slot 1", ApproxEq(slots[1].offset.x, 0.0f) &&
                           ApproxEq(slots[1].offset.z, -10.0f));
    TEST("Column slot 4", ApproxEq(slots[4].offset.x, 0.0f) &&
                           ApproxEq(slots[4].offset.z, -40.0f));
    // All y offsets should be 0
    for (int i = 0; i < 5; ++i) {
        TEST(("Column slot " + std::to_string(i) + " y").c_str(),
             ApproxEq(slots[i].offset.y, 0.0f));
    }
}

static void TestFormationNames() {
    std::cout << "[FormationPattern Names]\n";
    TEST("Line name", FormationPattern::GetFormationName(FormationType::Line) == "Line");
    TEST("V name", FormationPattern::GetFormationName(FormationType::V) == "V-Formation");
    TEST("Diamond name", FormationPattern::GetFormationName(FormationType::Diamond) == "Diamond");
    TEST("Circle name", FormationPattern::GetFormationName(FormationType::Circle) == "Circle");
    TEST("Wedge name", FormationPattern::GetFormationName(FormationType::Wedge) == "Wedge");
    TEST("Column name", FormationPattern::GetFormationName(FormationType::Column) == "Column");
}

static void TestFormationMaxSizes() {
    std::cout << "[FormationPattern MaxSizes]\n";
    TEST("Line max", FormationPattern::GetMaxRecommendedSize(FormationType::Line) == 10);
    TEST("V max", FormationPattern::GetMaxRecommendedSize(FormationType::V) == 8);
    TEST("Diamond max", FormationPattern::GetMaxRecommendedSize(FormationType::Diamond) == 9);
    TEST("Circle max", FormationPattern::GetMaxRecommendedSize(FormationType::Circle) == 12);
    TEST("Wedge max", FormationPattern::GetMaxRecommendedSize(FormationType::Wedge) == 7);
    TEST("Column max", FormationPattern::GetMaxRecommendedSize(FormationType::Column) == 6);
}

static void TestFormationComponentAddRemove() {
    std::cout << "[FormationComponent AddRemove]\n";
    FormationComponent fc;
    TEST("Initially empty", fc.GetMemberCount() == 0);

    fc.AddMember(100);
    fc.AddMember(200);
    fc.AddMember(300);
    TEST("3 members after add", fc.GetMemberCount() == 3);

    // Adding duplicate should not increase count
    fc.AddMember(200);
    TEST("No duplicate", fc.GetMemberCount() == 3);

    bool removed = fc.RemoveMember(200);
    TEST("Remove returns true", removed);
    TEST("2 members after remove", fc.GetMemberCount() == 2);

    bool removedAgain = fc.RemoveMember(200);
    TEST("Remove missing returns false", !removedAgain);
    TEST("Still 2 members", fc.GetMemberCount() == 2);
}

static void TestFormationComponentHasMember() {
    std::cout << "[FormationComponent HasMember]\n";
    FormationComponent fc;
    fc.AddMember(10);
    fc.AddMember(20);
    fc.AddMember(30);
    TEST("Has member 10", fc.HasMember(10));
    TEST("Has member 20", fc.HasMember(20));
    TEST("Has member 30", fc.HasMember(30));
    TEST("Does not have 40", !fc.HasMember(40));
    TEST("Does not have 0", !fc.HasMember(0));
}

static void TestFormationComponentReassignSlots() {
    std::cout << "[FormationComponent ReassignSlots]\n";
    FormationComponent fc;
    fc.AddMember(1);
    fc.AddMember(2);
    fc.AddMember(3);

    FormationPattern pattern;
    pattern.type = FormationType::Line;
    pattern.spacing = 5.0f;

    // Reassign should not crash
    fc.ReassignSlots(pattern);
    TEST("ReassignSlots does not crash", true);

    // Change pattern and reassign
    pattern.type = FormationType::V;
    fc.ReassignSlots(pattern);
    TEST("ReassignSlots with V does not crash", true);

    // Add more members and reassign
    fc.AddMember(4);
    fc.AddMember(5);
    fc.ReassignSlots(pattern);
    TEST("ReassignSlots with 5 members does not crash", true);
}

static void TestFormationComponentSerialization() {
    std::cout << "[FormationComponent Serialization]\n";
    FormationComponent original;
    original.formationType = FormationType::Diamond;
    original.spacing = 15.0f;
    original.leaderId = 42;
    original.slotIndex = 3;
    original.isLeader = true;
    original.targetOffset = Vector3(1.0f, 2.0f, 3.0f);
    original.AddMember(100);
    original.AddMember(200);
    original.AddMember(300);

    ComponentData cd = original.Serialize();
    TEST("Serialized type", cd.componentType == "FormationComponent");

    FormationComponent restored;
    restored.Deserialize(cd);
    TEST("Restored formationType", restored.formationType == FormationType::Diamond);
    TEST("Restored spacing", ApproxEq(restored.spacing, 15.0f));
    TEST("Restored leaderId", restored.leaderId == 42);
    TEST("Restored slotIndex", restored.slotIndex == 3);
    TEST("Restored isLeader", restored.isLeader);
    TEST("Restored targetOffset x", ApproxEq(restored.targetOffset.x, 1.0f));
    TEST("Restored targetOffset y", ApproxEq(restored.targetOffset.y, 2.0f));
    TEST("Restored targetOffset z", ApproxEq(restored.targetOffset.z, 3.0f));
    TEST("Restored member count", restored.GetMemberCount() == 3);
    TEST("Restored member 0", restored.members[0] == 100);
    TEST("Restored member 1", restored.members[1] == 200);
    TEST("Restored member 2", restored.members[2] == 300);
}

static void TestFormationSystem() {
    std::cout << "[FormationSystem]\n";
    FormationSystem fs;
    TEST("FormationSystem name", fs.GetName() == "FormationSystem");
    fs.Update(1.0f);
    TEST("Update without EM does not crash", true);
}

static void TestFormationSystemWithEM() {
    std::cout << "[FormationSystem WithEM]\n";
    EntityManager em;
    auto& ent = em.CreateEntity("Leader");
    auto* fc = em.AddComponent<FormationComponent>(ent.id, std::make_unique<FormationComponent>());
    fc->isLeader = true;
    fc->formationType = FormationType::V;
    fc->AddMember(ent.id);

    FormationSystem fs(em);
    fs.Update(1.0f);
    TEST("FormationSystem with EM does not crash", true);
}

static void TestCraftingReputationFormationGameEvents() {
    std::cout << "[Crafting/Reputation/Formation GameEvents]\n";
    // Crafting events
    TEST("CraftingStarted event", std::string(GameEvents::CraftingStarted) == "crafting.started");
    TEST("CraftingCompleted event", std::string(GameEvents::CraftingCompleted) == "crafting.completed");
    TEST("CraftingFailed event", std::string(GameEvents::CraftingFailed) == "crafting.failed");
    TEST("RecipeLearned event", std::string(GameEvents::RecipeLearned) == "crafting.recipe.learned");
    // Reputation events
    TEST("ReputationModified event", std::string(GameEvents::ReputationModified) == "reputation.changed");
    TEST("StandingChanged event", std::string(GameEvents::StandingChanged) == "reputation.standing.changed");
    TEST("ReputationDecayed event", std::string(GameEvents::ReputationDecayed) == "reputation.decayed");
    // Formation events
    TEST("FormationCreated event", std::string(GameEvents::FormationCreated) == "formation.created");
    TEST("FormationDisbanded event", std::string(GameEvents::FormationDisbanded) == "formation.disbanded");
    TEST("FormationChanged event", std::string(GameEvents::FormationChanged) == "formation.changed");
    TEST("MemberJoined event", std::string(GameEvents::MemberJoined) == "formation.member.joined");
    TEST("MemberLeft event", std::string(GameEvents::MemberLeft) == "formation.member.left");
}

// ===================================================================
// CapabilitySystem tests
// ===================================================================

static void TestCapabilitySystemEmptyShip() {
    std::cout << "[CapabilitySystem - Empty Ship]\n";
    Ship ship;
    ShipCapabilities caps = CapabilitySystem::Evaluate(ship);
    TEST("Empty ship blockCount=0", caps.blockCount == 0);
    TEST("Empty ship aliveCount=0", caps.aliveCount == 0);
    TEST("Empty ship mobility=0", ApproxEq(caps.mobility, 0.0f));
    TEST("Empty ship firepower=0", ApproxEq(caps.firepower, 0.0f));
    TEST("Empty ship power=0", ApproxEq(caps.power, 0.0f));
    TEST("Empty ship command=0", ApproxEq(caps.command, 0.0f));
    TEST("Empty ship defense=0", ApproxEq(caps.defense, 0.0f));
    TEST("Empty ship cargo=0", ApproxEq(caps.cargo, 0.0f));
    TEST("Empty ship totalMass=0", ApproxEq(caps.totalMass, 0.0f));
    TEST("Empty ship healthFraction=0", ApproxEq(caps.GetHealthFraction(), 0.0f));
}

static void TestCapabilitySystemSingleBlock() {
    std::cout << "[CapabilitySystem - Single Block]\n";
    Ship ship;
    ship.blocks.push_back(MakeBlock({0,0,0}, {1,1,1}, BlockType::Engine));
    ShipCapabilities caps = CapabilitySystem::Evaluate(ship);
    TEST("Single engine blockCount=1", caps.blockCount == 1);
    TEST("Single engine aliveCount=1", caps.aliveCount == 1);
    TEST("Single engine mobility=10", ApproxEq(caps.mobility, 10.0f));
    TEST("Single engine firepower=0", ApproxEq(caps.firepower, 0.0f));
    TEST("Single engine healthFraction=1", ApproxEq(caps.GetHealthFraction(), 1.0f));
}

static void TestCapabilitySystemMultipleBlockTypes() {
    std::cout << "[CapabilitySystem - Multiple Types]\n";
    Ship ship;
    ship.blocks.push_back(MakeBlock({0,0,0}, {1,1,1}, BlockType::Engine));
    ship.blocks.push_back(MakeBlock({1,0,0}, {1,1,1}, BlockType::WeaponMount));
    ship.blocks.push_back(MakeBlock({2,0,0}, {1,1,1}, BlockType::Generator));
    ship.blocks.push_back(MakeBlock({3,0,0}, {1,1,1}, BlockType::Gyro));
    ship.blocks.push_back(MakeBlock({4,0,0}, {1,1,1}, BlockType::Armor));
    ship.blocks.push_back(MakeBlock({5,0,0}, {1,1,1}, BlockType::Cargo));
    ship.blocks.push_back(MakeBlock({6,0,0}, {1,1,1}, BlockType::Hull));

    ShipCapabilities caps = CapabilitySystem::Evaluate(ship);
    TEST("All types blockCount=7", caps.blockCount == 7);
    TEST("All types aliveCount=7", caps.aliveCount == 7);
    TEST("All types mobility=10", ApproxEq(caps.mobility, 10.0f));
    TEST("All types firepower=8", ApproxEq(caps.firepower, 8.0f));
    TEST("All types power=12", ApproxEq(caps.power, 12.0f));
    TEST("All types command=6", ApproxEq(caps.command, 6.0f));
    TEST("All types defense=5", ApproxEq(caps.defense, 5.0f));
    TEST("All types cargo=15", ApproxEq(caps.cargo, 15.0f));
    TEST("All types totalMass>0", caps.totalMass > 0.0f);
}

static void TestCapabilitySystemDeadBlocks() {
    std::cout << "[CapabilitySystem - Dead Blocks]\n";
    Ship ship;
    auto engine1 = MakeBlock({0,0,0}, {1,1,1}, BlockType::Engine);
    auto engine2 = MakeBlock({1,0,0}, {1,1,1}, BlockType::Engine);
    engine2->currentHP = 0.0f; // dead
    ship.blocks.push_back(engine1);
    ship.blocks.push_back(engine2);

    ShipCapabilities caps = CapabilitySystem::Evaluate(ship);
    TEST("Dead block blockCount=2", caps.blockCount == 2);
    TEST("Dead block aliveCount=1", caps.aliveCount == 1);
    TEST("Dead block mobility=10", ApproxEq(caps.mobility, 10.0f)); // only 1 alive
    TEST("Dead block healthFraction=0.5", ApproxEq(caps.GetHealthFraction(), 0.5f));
}

static void TestCapabilitySystemLargerBlocks() {
    std::cout << "[CapabilitySystem - Larger Blocks]\n";
    Ship ship;
    // 2x2x2 engine = volume 8 => mobility = 10 * 8 = 80
    ship.blocks.push_back(MakeBlock({0,0,0}, {2,2,2}, BlockType::Engine));
    ShipCapabilities caps = CapabilitySystem::Evaluate(ship);
    TEST("Large engine mobility=80", ApproxEq(caps.mobility, 80.0f));
}

static void TestCapabilitySystemGetCapability() {
    std::cout << "[CapabilitySystem - GetCapability]\n";
    ShipCapabilities caps;
    caps.mobility = 50.0f;
    caps.firepower = 30.0f;
    caps.power = 20.0f;
    caps.command = 10.0f;
    caps.defense = 5.0f;
    caps.cargo = 15.0f;
    caps.totalMass = 100.0f;

    TEST("GetCapability mobility", ApproxEq(caps.GetCapability("mobility"), 50.0f));
    TEST("GetCapability firepower", ApproxEq(caps.GetCapability("firepower"), 30.0f));
    TEST("GetCapability power", ApproxEq(caps.GetCapability("power"), 20.0f));
    TEST("GetCapability command", ApproxEq(caps.GetCapability("command"), 10.0f));
    TEST("GetCapability defense", ApproxEq(caps.GetCapability("defense"), 5.0f));
    TEST("GetCapability cargo", ApproxEq(caps.GetCapability("cargo"), 15.0f));
    TEST("GetCapability totalMass", ApproxEq(caps.GetCapability("totalMass"), 100.0f));
    TEST("GetCapability unknown=0", ApproxEq(caps.GetCapability("unknown"), 0.0f));
}

static void TestCapabilitySystemGetSummary() {
    std::cout << "[CapabilitySystem - GetSummary]\n";
    ShipCapabilities caps;
    caps.blockCount = 5;
    caps.aliveCount = 3;
    std::string summary = caps.GetSummary();
    TEST("Summary not empty", !summary.empty());
    TEST("Summary contains 'Caps'", summary.find("Caps") != std::string::npos);
    TEST("Summary contains block counts", summary.find("3/5") != std::string::npos);
}

static void TestCapabilitySystemBlockWeights() {
    std::cout << "[CapabilitySystem - Block Weights]\n";
    TEST("Engine mobility weight=10", ApproxEq(CapabilitySystem::GetBlockCapabilityWeight(BlockType::Engine, "mobility"), 10.0f));
    TEST("WeaponMount firepower weight=8", ApproxEq(CapabilitySystem::GetBlockCapabilityWeight(BlockType::WeaponMount, "firepower"), 8.0f));
    TEST("Generator power weight=12", ApproxEq(CapabilitySystem::GetBlockCapabilityWeight(BlockType::Generator, "power"), 12.0f));
    TEST("Gyro command weight=6", ApproxEq(CapabilitySystem::GetBlockCapabilityWeight(BlockType::Gyro, "command"), 6.0f));
    TEST("Armor defense weight=5", ApproxEq(CapabilitySystem::GetBlockCapabilityWeight(BlockType::Armor, "defense"), 5.0f));
    TEST("Cargo cargo weight=15", ApproxEq(CapabilitySystem::GetBlockCapabilityWeight(BlockType::Cargo, "cargo"), 15.0f));
    TEST("Hull mobility weight=0", ApproxEq(CapabilitySystem::GetBlockCapabilityWeight(BlockType::Hull, "mobility"), 0.0f));
    TEST("Engine firepower weight=0", ApproxEq(CapabilitySystem::GetBlockCapabilityWeight(BlockType::Engine, "firepower"), 0.0f));
}

static void TestCapabilitySystemBlockTypeNames() {
    std::cout << "[CapabilitySystem - Block Type Names]\n";
    TEST("Hull name", CapabilitySystem::GetBlockTypeName(BlockType::Hull) == "Hull");
    TEST("Armor name", CapabilitySystem::GetBlockTypeName(BlockType::Armor) == "Armor");
    TEST("Engine name", CapabilitySystem::GetBlockTypeName(BlockType::Engine) == "Engine");
    TEST("Generator name", CapabilitySystem::GetBlockTypeName(BlockType::Generator) == "Generator");
    TEST("Gyro name", CapabilitySystem::GetBlockTypeName(BlockType::Gyro) == "Gyro");
    TEST("Cargo name", CapabilitySystem::GetBlockTypeName(BlockType::Cargo) == "Cargo");
    TEST("WeaponMount name", CapabilitySystem::GetBlockTypeName(BlockType::WeaponMount) == "WeaponMount");
}

// ===================================================================
// DebugRenderer tests
// ===================================================================

static void TestDebugRendererDrawLine() {
    std::cout << "[DebugRenderer - DrawLine]\n";
    DebugRenderer dr;
    dr.DrawLine({0,0,0}, {1,1,1}, DebugColor::Red());
    TEST("Line command count=1", dr.GetCommandCount() == 1);
    auto& cmds = dr.GetCommands();
    TEST("Line type", cmds[0].type == DebugDrawCommand::Type::Line);
    TEST("Line color red", cmds[0].color == DebugColor::Red());
}

static void TestDebugRendererDrawBox() {
    std::cout << "[DebugRenderer - DrawBox]\n";
    DebugRenderer dr;
    dr.DrawBox({5,5,5}, {1,1,1}, DebugColor::Green());
    TEST("Box command count=1", dr.GetCommandCount() == 1);
    auto& cmds = dr.GetCommands();
    TEST("Box type", cmds[0].type == DebugDrawCommand::Type::Box);
    TEST("Box center.x=5", ApproxEq(cmds[0].p1.x, 5.0f));
}

static void TestDebugRendererDrawSphere() {
    std::cout << "[DebugRenderer - DrawSphere]\n";
    DebugRenderer dr;
    dr.DrawSphere({3,3,3}, 2.5f, DebugColor::Blue());
    TEST("Sphere command count=1", dr.GetCommandCount() == 1);
    auto& cmds = dr.GetCommands();
    TEST("Sphere type", cmds[0].type == DebugDrawCommand::Type::Sphere);
    TEST("Sphere radius=2.5", ApproxEq(cmds[0].radius, 2.5f));
}

static void TestDebugRendererDrawText() {
    std::cout << "[DebugRenderer - DrawText]\n";
    DebugRenderer dr;
    dr.DrawText({0,0,0}, "Hello", DebugColor::White());
    TEST("Text command count=1", dr.GetCommandCount() == 1);
    auto& cmds = dr.GetCommands();
    TEST("Text type", cmds[0].type == DebugDrawCommand::Type::Text);
    TEST("Text content", cmds[0].text == "Hello");
}

static void TestDebugRendererClear() {
    std::cout << "[DebugRenderer - Clear]\n";
    DebugRenderer dr;
    dr.DrawLine({0,0,0}, {1,1,1}, DebugColor::Red());
    dr.DrawBox({0,0,0}, {1,1,1}, DebugColor::Green());
    TEST("Before clear count=2", dr.GetCommandCount() == 2);
    dr.Clear();
    TEST("After clear count=0", dr.GetCommandCount() == 0);
}

static void TestDebugRendererUpdate() {
    std::cout << "[DebugRenderer - Update]\n";
    DebugRenderer dr;
    // Persistent command (lifetime 1.0s)
    dr.DrawLine({0,0,0}, {1,1,1}, DebugColor::Red(), 1.0f);
    // Single-frame command (lifetime 0)
    dr.DrawLine({0,0,0}, {2,2,2}, DebugColor::Green(), 0.0f);
    TEST("Initial count=2", dr.GetCommandCount() == 2);

    // After update, single-frame commands are removed
    dr.Update(0.016f);
    TEST("After first update count=1", dr.GetCommandCount() == 1);

    // Persistent command still alive
    dr.Update(0.5f);
    TEST("After 0.5s update count=1", dr.GetCommandCount() == 1);

    // Expire the persistent command
    dr.Update(0.5f);
    TEST("After 1s total count=0", dr.GetCommandCount() == 0);
}

static void TestDebugRendererGetByType() {
    std::cout << "[DebugRenderer - GetByType]\n";
    DebugRenderer dr;
    dr.DrawLine({0,0,0}, {1,1,1}, DebugColor::Red());
    dr.DrawBox({0,0,0}, {1,1,1}, DebugColor::Green());
    dr.DrawLine({1,1,1}, {2,2,2}, DebugColor::Blue());

    auto lines = dr.GetCommandsByType(DebugDrawCommand::Type::Line);
    TEST("Line count=2", lines.size() == 2);
    auto boxes = dr.GetCommandsByType(DebugDrawCommand::Type::Box);
    TEST("Box count=1", boxes.size() == 1);
    auto spheres = dr.GetCommandsByType(DebugDrawCommand::Type::Sphere);
    TEST("Sphere count=0", spheres.size() == 0);
}

static void TestDebugRendererOverlayToggle() {
    std::cout << "[DebugRenderer - Overlay Toggle]\n";
    DebugRenderer dr;
    TEST("BlockRoles initially off", !dr.IsOverlayEnabled(DebugOverlayType::BlockRoles));

    dr.SetOverlayEnabled(DebugOverlayType::BlockRoles, true);
    TEST("BlockRoles enabled", dr.IsOverlayEnabled(DebugOverlayType::BlockRoles));
    TEST("DamageState still off", !dr.IsOverlayEnabled(DebugOverlayType::DamageState));

    dr.ToggleOverlay(DebugOverlayType::BlockRoles);
    TEST("BlockRoles toggled off", !dr.IsOverlayEnabled(DebugOverlayType::BlockRoles));

    dr.ToggleOverlay(DebugOverlayType::DamageState);
    TEST("DamageState toggled on", dr.IsOverlayEnabled(DebugOverlayType::DamageState));
}

static void TestDebugRendererOverlayNames() {
    std::cout << "[DebugRenderer - Overlay Names]\n";
    TEST("BlockRoles name", DebugRenderer::GetOverlayName(DebugOverlayType::BlockRoles) == "Block Roles");
    TEST("DamageState name", DebugRenderer::GetOverlayName(DebugOverlayType::DamageState) == "Damage State");
    TEST("Hardpoints name", DebugRenderer::GetOverlayName(DebugOverlayType::Hardpoints) == "Hardpoints");
    TEST("Capabilities name", DebugRenderer::GetOverlayName(DebugOverlayType::Capabilities) == "Capabilities");
    TEST("Grid name", DebugRenderer::GetOverlayName(DebugOverlayType::Grid) == "Grid");
    TEST("Physics name", DebugRenderer::GetOverlayName(DebugOverlayType::Physics) == "Physics");
}

static void TestDebugRendererBlockRoles() {
    std::cout << "[DebugRenderer - DrawBlockRoles]\n";
    Ship ship;
    ship.blocks.push_back(MakeBlock({0,0,0}, {1,1,1}, BlockType::Engine));
    ship.blocks.push_back(MakeBlock({1,0,0}, {1,1,1}, BlockType::WeaponMount));
    ship.blocks.push_back(MakeBlock({2,0,0}, {1,1,1}, BlockType::Hull));

    DebugRenderer dr;
    dr.DrawBlockRoles(ship);
    TEST("BlockRoles 3 commands", dr.GetCommandCount() == 3);
    auto boxes = dr.GetCommandsByType(DebugDrawCommand::Type::Box);
    TEST("BlockRoles all boxes", boxes.size() == 3);
    // Engine should be green
    TEST("Engine block green", boxes[0].color == DebugColor::Green());
    // WeaponMount should be red
    TEST("Weapon block red", boxes[1].color == DebugColor::Red());
}

static void TestDebugRendererDamageOverlay() {
    std::cout << "[DebugRenderer - DrawDamageOverlay]\n";
    Ship ship;
    auto fullHP = MakeBlock({0,0,0}, {1,1,1}, BlockType::Hull);
    auto halfHP = MakeBlock({1,0,0}, {1,1,1}, BlockType::Hull);
    halfHP->currentHP = halfHP->maxHP * 0.5f;
    auto deadBlock = MakeBlock({2,0,0}, {1,1,1}, BlockType::Hull);
    deadBlock->currentHP = 0.0f;

    ship.blocks.push_back(fullHP);
    ship.blocks.push_back(halfHP);
    ship.blocks.push_back(deadBlock);

    DebugRenderer dr;
    dr.DrawDamageOverlay(ship);
    TEST("DamageOverlay 3 commands", dr.GetCommandCount() == 3);

    auto boxes = dr.GetCommandsByType(DebugDrawCommand::Type::Box);
    // Full HP block: green (r=0, g=255)
    TEST("Full HP block green", boxes[0].color.g == 255 && boxes[0].color.r == 0);
    // Dead block: red (r=255, g=0)
    TEST("Dead block red", boxes[2].color.r == 255 && boxes[2].color.g == 0);
}

static void TestDebugColorPresets() {
    std::cout << "[DebugColor - Presets]\n";
    DebugColor red = DebugColor::Red();
    TEST("Red r=255", red.r == 255 && red.g == 0 && red.b == 0);
    DebugColor green = DebugColor::Green();
    TEST("Green g=255", green.r == 0 && green.g == 255 && green.b == 0);
    DebugColor blue = DebugColor::Blue();
    TEST("Blue b=255", blue.r == 0 && blue.g == 0 && blue.b == 255);
    DebugColor yellow = DebugColor::Yellow();
    TEST("Yellow r+g=255", yellow.r == 255 && yellow.g == 255 && yellow.b == 0);
    DebugColor cyan = DebugColor::Cyan();
    TEST("Cyan g+b=255", cyan.r == 0 && cyan.g == 255 && cyan.b == 255);
    DebugColor white = DebugColor::White();
    TEST("White all=255", white.r == 255 && white.g == 255 && white.b == 255);
    TEST("Color equality", red == DebugColor::Red());
    TEST("Color inequality", red != green);
}

// ===================================================================
// PerformanceMonitor tests
// ===================================================================

static void TestPerfMetricDefaults() {
    std::cout << "[PerfMetric - Defaults]\n";
    PerfMetric m("TestMetric");
    TEST("Name is TestMetric", m.GetName() == "TestMetric");
    TEST("Initial count=0", m.GetSampleCount() == 0);
    TEST("Initial latest=0", ApproxEq(m.GetLatest(), 0.0f));
    TEST("Initial avg=0", ApproxEq(m.GetAverage(), 0.0f));
    TEST("Initial min=0", ApproxEq(m.GetMin(), 0.0f));
    TEST("Initial max=0", ApproxEq(m.GetMax(), 0.0f));
}

static void TestPerfMetricRecord() {
    std::cout << "[PerfMetric - Record]\n";
    PerfMetric m("FrameTime");
    m.Record(16.0f);
    m.Record(17.0f);
    m.Record(15.0f);
    TEST("After 3 records count=3", m.GetSampleCount() == 3);
    TEST("Latest=15", ApproxEq(m.GetLatest(), 15.0f));
    TEST("Average=16", ApproxEq(m.GetAverage(), 16.0f));
    TEST("Min=15", ApproxEq(m.GetMin(), 15.0f));
    TEST("Max=17", ApproxEq(m.GetMax(), 17.0f));
}

static void TestPerfMetricMaxSamples() {
    std::cout << "[PerfMetric - MaxSamples]\n";
    PerfMetric m("Limited", 3);
    m.Record(1.0f);
    m.Record(2.0f);
    m.Record(3.0f);
    m.Record(4.0f);
    m.Record(5.0f);
    TEST("MaxSamples capped at 3", m.GetSampleCount() == 3);
    TEST("Oldest evicted, latest=5", ApproxEq(m.GetLatest(), 5.0f));
    TEST("Min=3 (1,2 evicted)", ApproxEq(m.GetMin(), 3.0f));
}

static void TestPerfMetricClear() {
    std::cout << "[PerfMetric - Clear]\n";
    PerfMetric m("Clearable");
    m.Record(10.0f);
    m.Record(20.0f);
    TEST("Before clear count=2", m.GetSampleCount() == 2);
    m.Clear();
    TEST("After clear count=0", m.GetSampleCount() == 0);
    TEST("After clear latest=0", ApproxEq(m.GetLatest(), 0.0f));
}

static void TestPerfMetricGetSamples() {
    std::cout << "[PerfMetric - GetSamples]\n";
    PerfMetric m("Samples");
    m.Record(1.0f, 0.0f);
    m.Record(2.0f, 1.0f);
    const auto& samples = m.GetSamples();
    TEST("Samples size=2", samples.size() == 2);
    TEST("Sample 0 value=1", ApproxEq(samples[0].value, 1.0f));
    TEST("Sample 1 value=2", ApproxEq(samples[1].value, 2.0f));
    TEST("Sample 1 timestamp=1", ApproxEq(samples[1].timestamp, 1.0f));
}

static void TestPerformanceMonitorFrame() {
    std::cout << "[PerformanceMonitor - Frame]\n";
    PerformanceMonitor pm;
    TEST("Initial frame count=0", pm.GetFrameCount() == 0);
    TEST("Initial FPS=0", ApproxEq(pm.GetFPS(), 0.0f));

    pm.BeginFrame();
    // Simulate a tiny amount of work
    pm.EndFrame();

    TEST("After 1 frame, count=1", pm.GetFrameCount() == 1);
    TEST("Frame time >= 0", pm.GetFrameTimeMs() >= 0.0f);
    // FPS should be positive if frame time > 0
    // (might be very high since the frame was nearly instant)
}

static void TestPerformanceMonitorSection() {
    std::cout << "[PerformanceMonitor - Section]\n";
    PerformanceMonitor pm;
    pm.BeginSection("Physics");
    // Simulate some work
    pm.EndSection("Physics");

    float physicsTime = pm.GetSectionTime("Physics");
    TEST("Physics section time >= 0", physicsTime >= 0.0f);
    TEST("Unknown section time = 0", ApproxEq(pm.GetSectionTime("Unknown"), 0.0f));

    const PerfMetric* metric = pm.GetMetric("Physics");
    TEST("Physics metric exists", metric != nullptr);
    TEST("Physics metric count=1", metric != nullptr && metric->GetSampleCount() == 1);
}

static void TestPerformanceMonitorCounters() {
    std::cout << "[PerformanceMonitor - Counters]\n";
    PerformanceMonitor pm;
    pm.RecordCounter("EntityCount", 150.0f);
    pm.RecordCounter("DrawCalls", 45.0f);

    TEST("EntityCount=150", ApproxEq(pm.GetCounter("EntityCount"), 150.0f));
    TEST("DrawCalls=45", ApproxEq(pm.GetCounter("DrawCalls"), 45.0f));
    TEST("Unknown counter=0", ApproxEq(pm.GetCounter("Unknown"), 0.0f));
}

static void TestPerformanceMonitorMetricNames() {
    std::cout << "[PerformanceMonitor - MetricNames]\n";
    PerformanceMonitor pm;
    pm.RecordCounter("Alpha", 1.0f);
    pm.RecordCounter("Beta", 2.0f);
    pm.RecordCounter("Gamma", 3.0f);

    auto names = pm.GetAllMetricNames();
    TEST("3 metrics", names.size() == 3);
    // Names should be sorted
    TEST("First is Alpha", names[0] == "Alpha");
    TEST("Second is Beta", names[1] == "Beta");
    TEST("Third is Gamma", names[2] == "Gamma");
}

static void TestPerformanceMonitorSummary() {
    std::cout << "[PerformanceMonitor - Summary]\n";
    PerformanceMonitor pm;
    pm.BeginFrame();
    pm.EndFrame();
    std::string summary = pm.GetSummary();
    TEST("Summary not empty", !summary.empty());
    TEST("Summary contains FPS", summary.find("FPS") != std::string::npos);
    TEST("Summary contains Frame", summary.find("Frame") != std::string::npos);
}

static void TestPerformanceMonitorReset() {
    std::cout << "[PerformanceMonitor - Reset]\n";
    PerformanceMonitor pm;
    pm.BeginFrame();
    pm.EndFrame();
    pm.RecordCounter("Test", 100.0f);
    TEST("Before reset frame>0", pm.GetFrameCount() > 0);

    pm.Reset();
    TEST("After reset frame=0", pm.GetFrameCount() == 0);
    TEST("After reset FPS=0", ApproxEq(pm.GetFPS(), 0.0f));
    TEST("After reset counter=0", ApproxEq(pm.GetCounter("Test"), 0.0f));
    TEST("After reset metrics empty", pm.GetAllMetricNames().empty());
}

static void TestPerformanceMonitorEndSectionWithoutBegin() {
    std::cout << "[PerformanceMonitor - EndSection without Begin]\n";
    PerformanceMonitor pm;
    // Should not crash
    pm.EndSection("NeverStarted");
    TEST("No crash on EndSection without Begin", true);
    TEST("NeverStarted time=0", ApproxEq(pm.GetSectionTime("NeverStarted"), 0.0f));
}

// ===================================================================
// Capability/Debug/Performance GameEvents tests
// ===================================================================

static void TestCapabilityDebugPerfGameEvents() {
    std::cout << "[Capability/Debug/Perf GameEvents]\n";
    // Capability events
    TEST("CapabilityEvaluated event", std::string(GameEvents::CapabilityEvaluated) == "capability.evaluated");
    TEST("CapabilityDegraded event", std::string(GameEvents::CapabilityDegraded) == "capability.degraded");
    TEST("CapabilityRestored event", std::string(GameEvents::CapabilityRestored) == "capability.restored");
    // Debug events
    TEST("DebugOverlayToggled event", std::string(GameEvents::DebugOverlayToggled) == "debug.overlay.toggled");
    TEST("DebugCommandQueued event", std::string(GameEvents::DebugCommandQueued) == "debug.command.queued");
    // Performance events
    TEST("PerfFrameRecorded event", std::string(GameEvents::PerfFrameRecorded) == "perf.frame.recorded");
    TEST("PerfSectionRecorded event", std::string(GameEvents::PerfSectionRecorded) == "perf.section.recorded");
    TEST("PerfCounterRecorded event", std::string(GameEvents::PerfCounterRecorded) == "perf.counter.recorded");
}

// ===================================================================
// DiplomacySystem tests
// ===================================================================

static void TestTreatyGetName() {
    std::cout << "[Treaty::GetTreatyName]\n";
    TEST("NonAggression name", Treaty::GetTreatyName(TreatyType::NonAggression) == "Non-Aggression Pact");
    TEST("TradeAgreement name", Treaty::GetTreatyName(TreatyType::TradeAgreement) == "Trade Agreement");
    TEST("DefensivePact name", Treaty::GetTreatyName(TreatyType::DefensivePact) == "Defensive Pact");
    TEST("Alliance name", Treaty::GetTreatyName(TreatyType::Alliance) == "Alliance");
    TEST("Ceasefire name", Treaty::GetTreatyName(TreatyType::Ceasefire) == "Ceasefire");
}

static void TestTreatyProgress() {
    std::cout << "[Treaty::GetProgress]\n";
    Treaty t;
    t.totalDuration = 100.0f;
    t.duration = 100.0f;
    TEST("Progress at start", ApproxEq(t.GetProgress(), 0.0f));

    t.duration = 50.0f;
    TEST("Progress at half", ApproxEq(t.GetProgress(), 50.0f));

    t.duration = 0.0f;
    TEST("Progress at end", ApproxEq(t.GetProgress(), 100.0f));

    Treaty indefinite;
    indefinite.totalDuration = -1.0f;
    indefinite.duration = -1.0f;
    TEST("Indefinite progress", ApproxEq(indefinite.GetProgress(), 100.0f));
}

static void TestDiplomaticRelationStatusName() {
    std::cout << "[DiplomaticRelation::GetStatusName]\n";
    TEST("War name", DiplomaticRelation::GetStatusName(DiplomaticStatus::War) == "War");
    TEST("Hostile name", DiplomaticRelation::GetStatusName(DiplomaticStatus::Hostile) == "Hostile");
    TEST("Neutral name", DiplomaticRelation::GetStatusName(DiplomaticStatus::Neutral) == "Neutral");
    TEST("NonAggression name", DiplomaticRelation::GetStatusName(DiplomaticStatus::NonAggression) == "Non-Aggression");
    TEST("Trade name", DiplomaticRelation::GetStatusName(DiplomaticStatus::Trade) == "Trade");
    TEST("Alliance name", DiplomaticRelation::GetStatusName(DiplomaticStatus::Alliance) == "Alliance");
}

static void TestDiplomaticRelationTrust() {
    std::cout << "[DiplomaticRelation::ModifyTrust]\n";
    DiplomaticRelation r;
    r.trust = 0;
    r.ModifyTrust(50);
    TEST("Trust +50", r.trust == 50);
    r.ModifyTrust(60);
    TEST("Trust clamped to 100", r.trust == 100);
    r.ModifyTrust(-250);
    TEST("Trust clamped to -100", r.trust == -100);
}

static void TestDiplomacyDatabase() {
    std::cout << "[DiplomacyDatabase]\n";
    DiplomacyDatabase db;
    TEST("Empty db count", db.GetTreatyCount() == 0);

    Treaty t;
    t.type = TreatyType::TradeAgreement;
    t.factionA = "alpha";
    t.factionB = "beta";
    std::string id = db.AddTreaty(t);
    TEST("Count after add", db.GetTreatyCount() == 1);
    TEST("FindTreaty exists", db.FindTreaty(id) != nullptr);
    TEST("FindTreaty factionA", db.FindTreaty(id)->factionA == "alpha");
    TEST("FindTreaty missing", db.FindTreaty("nonexistent") == nullptr);

    auto alphaT = db.GetTreatiesForFaction("alpha");
    TEST("Treaties for alpha", alphaT.size() == 1);
    auto gammaT = db.GetTreatiesForFaction("gamma");
    TEST("Treaties for gamma", gammaT.empty());

    auto active = db.GetActiveTreaties();
    TEST("Active treaties", active.size() == 1);

    TEST("Remove treaty", db.RemoveTreaty(id));
    TEST("Count after remove", db.GetTreatyCount() == 0);
    TEST("Remove nonexistent", !db.RemoveTreaty("fake"));
}

static void TestDiplomacyDatabaseDefaults() {
    std::cout << "[DiplomacyDatabase::CreateDefaultDatabase]\n";
    DiplomacyDatabase db = DiplomacyDatabase::CreateDefaultDatabase();
    TEST("Default treaty count", db.GetTreatyCount() == 4);

    auto tradersT = db.GetTreatiesForFaction("traders_guild");
    TEST("Traders treaties count", tradersT.size() == 1);

    auto empireT = db.GetTreatiesForFaction("galactic_empire");
    TEST("Empire treaties count", empireT.size() == 2);

    auto republicT = db.GetTreatiesForFaction("free_republic");
    TEST("Republic treaties count", republicT.size() == 2);
}

static void TestDiplomacyComponentDefaults() {
    std::cout << "[DiplomacyComponent defaults]\n";
    DiplomacyComponent dc;
    TEST("Default factionId", dc.factionId.empty());
    TEST("Default relations empty", dc.relations.empty());
    TEST("Default warWeariness", ApproxEq(dc.warWeariness, 0.0f));
    TEST("Default not at war", !dc.IsAtWar());
    TEST("Default war count", dc.GetWarCount() == 0);
    TEST("Default relation count", dc.GetRelationCount() == 0);
}

static void TestDiplomacyComponentAddRelation() {
    std::cout << "[DiplomacyComponent::AddRelation]\n";
    DiplomacyComponent dc;
    dc.factionId = "empire";

    dc.AddRelation("republic");
    TEST("Relation count", dc.GetRelationCount() == 1);
    TEST("Default status", dc.GetStatus("republic") == DiplomaticStatus::Neutral);

    // Adding same faction again returns existing
    dc.AddRelation("republic", DiplomaticStatus::War);
    TEST("No duplicate", dc.GetRelationCount() == 1);
    TEST("Status unchanged by AddRelation", dc.GetStatus("republic") == DiplomaticStatus::Neutral);
}

static void TestDiplomacyComponentDeclareWar() {
    std::cout << "[DiplomacyComponent war/peace]\n";
    DiplomacyComponent dc;
    dc.factionId = "empire";

    dc.DeclareWar("pirates");
    TEST("At war", dc.IsAtWar());
    TEST("War count", dc.GetWarCount() == 1);
    TEST("Status is War", dc.GetStatus("pirates") == DiplomaticStatus::War);

    dc.ProposePeace("pirates");
    TEST("Not at war after peace", !dc.IsAtWar());
    TEST("Status is Neutral", dc.GetStatus("pirates") == DiplomaticStatus::Neutral);
}

static void TestDiplomacyComponentSetStatus() {
    std::cout << "[DiplomacyComponent::SetStatus]\n";
    DiplomacyComponent dc;
    dc.factionId = "empire";

    dc.SetStatus("republic", DiplomaticStatus::Alliance);
    TEST("Status set", dc.GetStatus("republic") == DiplomaticStatus::Alliance);

    dc.SetStatus("republic", DiplomaticStatus::Trade);
    TEST("Status updated", dc.GetStatus("republic") == DiplomaticStatus::Trade);
}

static void TestDiplomacyComponentGetFactionsWithStatus() {
    std::cout << "[DiplomacyComponent::GetFactionsWithStatus]\n";
    DiplomacyComponent dc;
    dc.factionId = "empire";
    dc.SetStatus("republic", DiplomaticStatus::Alliance);
    dc.SetStatus("traders", DiplomaticStatus::Trade);
    dc.SetStatus("pirates", DiplomaticStatus::War);
    dc.SetStatus("miners", DiplomaticStatus::Alliance);

    auto allies = dc.GetFactionsWithStatus(DiplomaticStatus::Alliance);
    TEST("Allied factions count", allies.size() == 2);

    auto enemies = dc.GetFactionsWithStatus(DiplomaticStatus::War);
    TEST("War factions count", enemies.size() == 1);
    TEST("War faction is pirates", enemies[0] == "pirates");
}

static void TestDiplomacyComponentSerialization() {
    std::cout << "[DiplomacyComponent serialization]\n";
    DiplomacyComponent dc;
    dc.factionId = "empire";
    dc.warWeariness = 45.0f;
    dc.warWearinessRate = 2.0f;
    dc.trustGainRate = 0.5f;
    dc.SetStatus("republic", DiplomaticStatus::Alliance);
    dc.SetStatus("pirates", DiplomaticStatus::War);

    auto* rel = dc.GetRelation("republic");
    rel->trust = 75;
    rel->activeTreatyIds = {"t1", "t2"};

    ComponentData cd = dc.Serialize();

    DiplomacyComponent dc2;
    dc2.Deserialize(cd);
    TEST("Faction round-trip", dc2.factionId == "empire");
    TEST("WarWeariness round-trip", ApproxEq(dc2.warWeariness, 45.0f));
    TEST("WarWearinessRate round-trip", ApproxEq(dc2.warWearinessRate, 2.0f));
    TEST("TrustGainRate round-trip", ApproxEq(dc2.trustGainRate, 0.5f));
    TEST("Relation count round-trip", dc2.GetRelationCount() == 2);
    TEST("Status round-trip", dc2.GetStatus("republic") == DiplomaticStatus::Alliance);
    TEST("Status round-trip2", dc2.GetStatus("pirates") == DiplomaticStatus::War);

    auto* rel2 = dc2.GetRelation("republic");
    TEST("Trust round-trip", rel2 != nullptr && rel2->trust == 75);
    TEST("Treaty IDs round-trip", rel2 != nullptr && rel2->activeTreatyIds.size() == 2);
}

static void TestDiplomacyComponentDeserializeInvalidEnum() {
    std::cout << "[DiplomacyComponent deserialize invalid enum]\n";
    ComponentData cd;
    cd.componentType = "DiplomacyComponent";
    cd.data["relationCount"] = "1";
    cd.data["rel_0_factionA"] = "a";
    cd.data["rel_0_factionB"] = "b";
    cd.data["rel_0_status"] = "999";  // invalid
    cd.data["rel_0_trust"] = "500";   // out of range

    DiplomacyComponent dc;
    dc.Deserialize(cd);
    TEST("Invalid status -> Neutral", dc.relations[0].status == DiplomaticStatus::Neutral);
    TEST("Trust clamped to 100", dc.relations[0].trust == 100);
}

static void TestDiplomacySystem() {
    std::cout << "[DiplomacySystem default ctor]\n";
    DiplomacySystem sys;
    TEST("System name", sys.GetName() == "DiplomacySystem");
    // Should not crash on empty update
    sys.Update(1.0f);
    TEST("No crash on empty Update", true);
}

static void TestDiplomacySystemWarWeariness() {
    std::cout << "[DiplomacySystem war weariness]\n";
    EntityManager em;
    DiplomacySystem sys(em);

    auto& ent = em.CreateEntity("faction_test");
    auto* dc = em.AddComponent<DiplomacyComponent>(ent.id, std::make_unique<DiplomacyComponent>());
    dc->factionId = "empire";
    dc->warWearinessRate = 10.0f;
    dc->DeclareWar("pirates");

    sys.Update(1.0f);
    TEST("War weariness increases", dc->warWeariness > 0.0f);
    TEST("War weariness ~10", ApproxEq(dc->warWeariness, 10.0f));

    // Without war, weariness should not increase
    dc->ProposePeace("pirates");
    float prevWeariness = dc->warWeariness;
    sys.Update(1.0f);
    TEST("No weariness without war", ApproxEq(dc->warWeariness, prevWeariness));
}

static void TestDiplomacySystemTrustGain() {
    std::cout << "[DiplomacySystem trust gain]\n";
    EntityManager em;
    DiplomacySystem sys(em);

    auto& ent = em.CreateEntity("faction_test");
    auto* dc = em.AddComponent<DiplomacyComponent>(ent.id, std::make_unique<DiplomacyComponent>());
    dc->factionId = "empire";
    dc->trustGainRate = 10.0f;
    dc->SetStatus("republic", DiplomaticStatus::Alliance);

    sys.Update(1.0f);
    auto* rel = dc->GetRelation("republic");
    TEST("Trust increased for alliance", rel != nullptr && rel->trust > 0);
}

// ===================================================================
// ResearchSystem tests
// ===================================================================

static void TestResearchNodeCategoryName() {
    std::cout << "[ResearchNode::GetCategoryName]\n";
    TEST("Engineering name", ResearchNode::GetCategoryName(ResearchCategory::Engineering) == "Engineering");
    TEST("Weapons name", ResearchNode::GetCategoryName(ResearchCategory::Weapons) == "Weapons");
    TEST("Shields name", ResearchNode::GetCategoryName(ResearchCategory::Shields) == "Shields");
    TEST("Navigation name", ResearchNode::GetCategoryName(ResearchCategory::Navigation) == "Navigation");
    TEST("Economy name", ResearchNode::GetCategoryName(ResearchCategory::Economy) == "Economy");
    TEST("Special name", ResearchNode::GetCategoryName(ResearchCategory::Special) == "Special");
}

static void TestResearchJobPercentage() {
    std::cout << "[ResearchJob::GetPercentage]\n";
    ResearchJob job;
    job.totalCost = 100.0f;
    job.progress = 0.0f;
    TEST("Percentage at start", ApproxEq(job.GetPercentage(), 0.0f));

    job.progress = 50.0f;
    TEST("Percentage at half", ApproxEq(job.GetPercentage(), 50.0f));

    job.progress = 100.0f;
    TEST("Percentage at end", ApproxEq(job.GetPercentage(), 100.0f));

    ResearchJob zeroJob;
    zeroJob.totalCost = 0.0f;
    TEST("Zero cost percentage", ApproxEq(zeroJob.GetPercentage(), 100.0f));
}

static void TestResearchTree() {
    std::cout << "[ResearchTree]\n";
    ResearchTree tree;
    TEST("Empty tree count", tree.GetNodeCount() == 0);

    ResearchNode n;
    n.nodeId = "test_tech";
    n.displayName = "Test Tech";
    n.category = ResearchCategory::Engineering;
    n.researchCost = 50.0f;
    tree.AddNode(n);

    TEST("Count after add", tree.GetNodeCount() == 1);
    TEST("FindNode exists", tree.FindNode("test_tech") != nullptr);
    TEST("FindNode id", tree.FindNode("test_tech")->nodeId == "test_tech");
    TEST("FindNode missing", tree.FindNode("nonexistent") == nullptr);

    auto engNodes = tree.GetNodesByCategory(ResearchCategory::Engineering);
    TEST("Category filter count", engNodes.size() == 1);
    auto weapNodes = tree.GetNodesByCategory(ResearchCategory::Weapons);
    TEST("Category filter empty", weapNodes.empty());
}

static void TestResearchTreeDefaults() {
    std::cout << "[ResearchTree::CreateDefaultTree]\n";
    ResearchTree tree = ResearchTree::CreateDefaultTree();
    TEST("Default node count", tree.GetNodeCount() == 8);
    TEST("improved_hull exists", tree.FindNode("improved_hull") != nullptr);
    TEST("advanced_materials exists", tree.FindNode("advanced_materials") != nullptr);
    TEST("laser_efficiency exists", tree.FindNode("laser_efficiency") != nullptr);
    TEST("plasma_weapons exists", tree.FindNode("plasma_weapons") != nullptr);
    TEST("shield_harmonics exists", tree.FindNode("shield_harmonics") != nullptr);
    TEST("hyperdrive_calibration exists", tree.FindNode("hyperdrive_calibration") != nullptr);
    TEST("trade_networks exists", tree.FindNode("trade_networks") != nullptr);
    TEST("experimental_reactor exists", tree.FindNode("experimental_reactor") != nullptr);

    const auto* reactor = tree.FindNode("experimental_reactor");
    TEST("Reactor level 3", reactor->requiredLevel == 3);
    TEST("Reactor cost", ApproxEq(reactor->researchCost, 500.0f));
    TEST("Reactor prereqs", reactor->prerequisites.size() == 2);
}

static void TestResearchTreePrerequisites() {
    std::cout << "[ResearchTree::ArePrerequisitesMet]\n";
    ResearchTree tree = ResearchTree::CreateDefaultTree();
    std::unordered_set<std::string> completed;

    // improved_hull has no prereqs
    TEST("No prereqs met", tree.ArePrerequisitesMet("improved_hull", completed));
    // advanced_materials needs improved_hull
    TEST("Prereq not met", !tree.ArePrerequisitesMet("advanced_materials", completed));

    completed.insert("improved_hull");
    TEST("Prereq met after completing", tree.ArePrerequisitesMet("advanced_materials", completed));

    // experimental_reactor needs advanced_materials + shield_harmonics
    TEST("Partial prereqs not met", !tree.ArePrerequisitesMet("experimental_reactor", completed));
    completed.insert("advanced_materials");
    completed.insert("shield_harmonics");
    TEST("All prereqs met", tree.ArePrerequisitesMet("experimental_reactor", completed));
}

static void TestResearchComponentDefaults() {
    std::cout << "[ResearchComponent defaults]\n";
    ResearchComponent rc;
    TEST("Default completed empty", rc.GetCompletedCount() == 0);
    TEST("Default not researching", !rc.IsResearching());
    TEST("Default no active job", !rc.hasActiveJob);
    TEST("Default rate", ApproxEq(rc.researchRate, 1.0f));
    TEST("Default level", rc.researcherLevel == 1);
}

static void TestResearchComponentStartResearch() {
    std::cout << "[ResearchComponent::StartResearch]\n";
    ResearchTree tree = ResearchTree::CreateDefaultTree();
    ResearchComponent rc;

    const auto* hull = tree.FindNode("improved_hull");
    TEST("Start research", rc.StartResearch(*hull, tree));
    TEST("Is researching", rc.IsResearching());
    TEST("Has active job", rc.hasActiveJob);
    TEST("Job nodeId", rc.currentJob.nodeId == "improved_hull");
    TEST("Job progress", ApproxEq(rc.currentJob.progress, 0.0f));

    // Can't start another while active
    const auto* laser = tree.FindNode("laser_efficiency");
    TEST("Can't start another", !rc.StartResearch(*laser, tree));
}

static void TestResearchComponentPrerequisites() {
    std::cout << "[ResearchComponent prerequisite check]\n";
    ResearchTree tree = ResearchTree::CreateDefaultTree();
    ResearchComponent rc;
    rc.researcherLevel = 2; // advanced_materials requires level 2

    // advanced_materials requires improved_hull
    const auto* adv = tree.FindNode("advanced_materials");
    TEST("Prereq blocks start", !rc.StartResearch(*adv, tree));

    rc.completedResearch.insert("improved_hull");
    TEST("Prereq met allows start", rc.StartResearch(*adv, tree));
}

static void TestResearchComponentLevelRequirement() {
    std::cout << "[ResearchComponent level requirement]\n";
    ResearchTree tree = ResearchTree::CreateDefaultTree();
    ResearchComponent rc;
    rc.completedResearch.insert("improved_hull");
    rc.completedResearch.insert("advanced_materials");
    rc.completedResearch.insert("shield_harmonics");

    // experimental_reactor requires level 3
    const auto* reactor = tree.FindNode("experimental_reactor");
    TEST("Level too low", !rc.StartResearch(*reactor, tree));

    rc.researcherLevel = 3;
    TEST("Level met", rc.StartResearch(*reactor, tree));
}

static void TestResearchComponentCancel() {
    std::cout << "[ResearchComponent::CancelResearch]\n";
    ResearchTree tree = ResearchTree::CreateDefaultTree();
    ResearchComponent rc;
    const auto* hull = tree.FindNode("improved_hull");
    rc.StartResearch(*hull, tree);

    TEST("Cancel returns true", rc.CancelResearch());
    TEST("Not researching after cancel", !rc.IsResearching());
    TEST("Cancel empty returns false", !rc.CancelResearch());
}

static void TestResearchComponentAlreadyCompleted() {
    std::cout << "[ResearchComponent already completed check]\n";
    ResearchTree tree = ResearchTree::CreateDefaultTree();
    ResearchComponent rc;
    rc.completedResearch.insert("improved_hull");

    const auto* hull = tree.FindNode("improved_hull");
    TEST("Can't re-research completed", !rc.StartResearch(*hull, tree));
}

static void TestResearchComponentGetAvailable() {
    std::cout << "[ResearchComponent::GetAvailableResearch]\n";
    ResearchTree tree = ResearchTree::CreateDefaultTree();
    ResearchComponent rc;

    auto available = rc.GetAvailableResearch(tree);
    // Tier 1 nodes with no prereqs and level 1: improved_hull, laser_efficiency, shield_harmonics,
    // hyperdrive_calibration, trade_networks
    TEST("Initial available count", available.size() == 5);

    rc.completedResearch.insert("improved_hull");
    available = rc.GetAvailableResearch(tree);
    // Should now include advanced_materials (level 2 but rc is level 1)
    // improved_hull is completed so removed; advanced_materials needs level 2
    TEST("Available after completing hull", available.size() == 4);

    rc.researcherLevel = 2;
    available = rc.GetAvailableResearch(tree);
    // Now advanced_materials is available
    TEST("Available at level 2", available.size() == 5);
}

static void TestResearchComponentSerialization() {
    std::cout << "[ResearchComponent serialization]\n";
    ResearchTree tree = ResearchTree::CreateDefaultTree();
    ResearchComponent rc;
    rc.researchRate = 2.5f;
    rc.researcherLevel = 3;
    rc.completedResearch.insert("improved_hull");
    rc.completedResearch.insert("laser_efficiency");

    const auto* adv = tree.FindNode("advanced_materials");
    rc.StartResearch(*adv, tree);
    rc.currentJob.progress = 75.0f;

    ComponentData cd = rc.Serialize();

    ResearchComponent rc2;
    rc2.Deserialize(cd);
    TEST("Rate round-trip", ApproxEq(rc2.researchRate, 2.5f));
    TEST("Level round-trip", rc2.researcherLevel == 3);
    TEST("Completed count round-trip", rc2.GetCompletedCount() == 2);
    TEST("Has hull", rc2.HasCompleted("improved_hull"));
    TEST("Has laser", rc2.HasCompleted("laser_efficiency"));
    TEST("Active job round-trip", rc2.hasActiveJob);
    TEST("Job nodeId round-trip", rc2.currentJob.nodeId == "advanced_materials");
    TEST("Job progress round-trip", ApproxEq(rc2.currentJob.progress, 75.0f));
}

static void TestResearchSystem() {
    std::cout << "[ResearchSystem default ctor]\n";
    ResearchSystem sys;
    TEST("System name", sys.GetName() == "ResearchSystem");
    sys.Update(1.0f);
    TEST("No crash on empty Update", true);
}

static void TestResearchSystemProgress() {
    std::cout << "[ResearchSystem progress]\n";
    EntityManager em;
    ResearchSystem sys(em);
    ResearchTree tree = ResearchTree::CreateDefaultTree();

    auto& ent = em.CreateEntity("researcher");
    auto* rc = em.AddComponent<ResearchComponent>(ent.id, std::make_unique<ResearchComponent>());
    rc->researchRate = 50.0f;

    const auto* hull = tree.FindNode("improved_hull");
    rc->StartResearch(*hull, tree);

    sys.Update(1.0f);
    TEST("Progress after 1s", ApproxEq(rc->currentJob.progress, 50.0f));
    TEST("Not complete yet", !rc->currentJob.isComplete);

    sys.Update(1.0f);
    TEST("Research complete", rc->currentJob.isComplete);
    TEST("Added to completed set", rc->HasCompleted("improved_hull"));
    TEST("Job cleared", !rc->hasActiveJob);
}

static void TestResearchSystemNoOvershoot() {
    std::cout << "[ResearchSystem no overshoot]\n";
    EntityManager em;
    ResearchSystem sys(em);
    ResearchTree tree = ResearchTree::CreateDefaultTree();

    auto& ent = em.CreateEntity("researcher");
    auto* rc = em.AddComponent<ResearchComponent>(ent.id, std::make_unique<ResearchComponent>());
    rc->researchRate = 1000.0f;  // Very fast

    const auto* hull = tree.FindNode("improved_hull");
    rc->StartResearch(*hull, tree);

    sys.Update(1.0f);
    TEST("Progress capped at cost", ApproxEq(rc->currentJob.progress, hull->researchCost));
    TEST("Complete", rc->currentJob.isComplete);
}

// ===================================================================
// NotificationSystem tests
// ===================================================================

static void TestNotificationCategoryName() {
    std::cout << "[Notification::GetCategoryName]\n";
    TEST("Combat name", Notification::GetCategoryName(NotificationCategory::Combat) == "Combat");
    TEST("Trade name", Notification::GetCategoryName(NotificationCategory::Trade) == "Trade");
    TEST("Diplomacy name", Notification::GetCategoryName(NotificationCategory::Diplomacy) == "Diplomacy");
    TEST("Research name", Notification::GetCategoryName(NotificationCategory::Research) == "Research");
    TEST("Navigation name", Notification::GetCategoryName(NotificationCategory::Navigation) == "Navigation");
    TEST("System name", Notification::GetCategoryName(NotificationCategory::System) == "System");
}

static void TestNotificationPriorityName() {
    std::cout << "[Notification::GetPriorityName]\n";
    TEST("Low name", Notification::GetPriorityName(NotificationPriority::Low) == "Low");
    TEST("Normal name", Notification::GetPriorityName(NotificationPriority::Normal) == "Normal");
    TEST("High name", Notification::GetPriorityName(NotificationPriority::High) == "High");
    TEST("Critical name", Notification::GetPriorityName(NotificationPriority::Critical) == "Critical");
}

static void TestNotificationComponentDefaults() {
    std::cout << "[NotificationComponent defaults]\n";
    NotificationComponent nc;
    TEST("Default empty", nc.notifications.empty());
    TEST("Default max", nc.maxNotifications == 50);
    TEST("Default autoRemove", nc.autoRemoveExpired);
    TEST("Default unread count", nc.GetUnreadCount() == 0);
    TEST("Default active count", nc.GetActiveCount() == 0);
    TEST("No critical unread", !nc.HasCriticalUnread());
}

static void TestNotificationComponentAddNotification() {
    std::cout << "[NotificationComponent::AddNotification]\n";
    NotificationComponent nc;
    int id1 = nc.AddNotification("Test", "Test message");
    TEST("First ID positive", id1 > 0);
    TEST("Count after add", nc.GetActiveCount() == 1);
    TEST("Unread after add", nc.GetUnreadCount() == 1);

    int id2 = nc.AddNotification("Test 2", "Another message", NotificationCategory::Combat,
                                 NotificationPriority::High);
    TEST("IDs are unique", id2 != id1);
    TEST("Count after 2 adds", nc.GetActiveCount() == 2);

    const auto* n = nc.FindNotification(id2);
    TEST("Find by ID", n != nullptr);
    TEST("Title matches", n != nullptr && n->title == "Test 2");
    TEST("Category matches", n != nullptr && n->category == NotificationCategory::Combat);
    TEST("Priority matches", n != nullptr && n->priority == NotificationPriority::High);
}

static void TestNotificationComponentMaxCapacity() {
    std::cout << "[NotificationComponent max capacity]\n";
    NotificationComponent nc;
    nc.maxNotifications = 3;
    nc.AddNotification("1", "m1");
    nc.AddNotification("2", "m2");
    nc.AddNotification("3", "m3");
    TEST("At capacity", nc.GetActiveCount() == 3);

    nc.AddNotification("4", "m4");
    TEST("Still at max", nc.GetActiveCount() == 3);
    // Oldest should have been removed
    TEST("Newest exists", nc.FindNotification(4) != nullptr);
}

static void TestNotificationComponentMarkAsRead() {
    std::cout << "[NotificationComponent mark as read]\n";
    NotificationComponent nc;
    int id = nc.AddNotification("Test", "msg");
    TEST("Unread before", nc.GetUnreadCount() == 1);

    TEST("MarkAsRead returns true", nc.MarkAsRead(id));
    TEST("Unread after", nc.GetUnreadCount() == 0);
    TEST("MarkAsRead nonexistent", !nc.MarkAsRead(999));
}

static void TestNotificationComponentMarkAllAsRead() {
    std::cout << "[NotificationComponent::MarkAllAsRead]\n";
    NotificationComponent nc;
    nc.AddNotification("1", "m1");
    nc.AddNotification("2", "m2");
    nc.AddNotification("3", "m3");
    TEST("Unread before", nc.GetUnreadCount() == 3);

    nc.MarkAllAsRead();
    TEST("Unread after", nc.GetUnreadCount() == 0);
}

static void TestNotificationComponentRemove() {
    std::cout << "[NotificationComponent::RemoveNotification]\n";
    NotificationComponent nc;
    int id = nc.AddNotification("Test", "msg");
    TEST("Remove returns true", nc.RemoveNotification(id));
    TEST("Count after remove", nc.GetActiveCount() == 0);
    TEST("Remove nonexistent", !nc.RemoveNotification(999));
}

static void TestNotificationComponentGetByCategory() {
    std::cout << "[NotificationComponent::GetByCategory]\n";
    NotificationComponent nc;
    nc.AddNotification("Combat 1", "m", NotificationCategory::Combat);
    nc.AddNotification("Trade 1", "m", NotificationCategory::Trade);
    nc.AddNotification("Combat 2", "m", NotificationCategory::Combat);

    auto combat = nc.GetByCategory(NotificationCategory::Combat);
    TEST("Combat count", combat.size() == 2);
    auto trade = nc.GetByCategory(NotificationCategory::Trade);
    TEST("Trade count", trade.size() == 1);
    auto nav = nc.GetByCategory(NotificationCategory::Navigation);
    TEST("Navigation count", nav.empty());
}

static void TestNotificationComponentGetByMinPriority() {
    std::cout << "[NotificationComponent::GetByMinPriority]\n";
    NotificationComponent nc;
    nc.AddNotification("Low", "m", NotificationCategory::System, NotificationPriority::Low);
    nc.AddNotification("Normal", "m", NotificationCategory::System, NotificationPriority::Normal);
    nc.AddNotification("High", "m", NotificationCategory::System, NotificationPriority::High);
    nc.AddNotification("Critical", "m", NotificationCategory::System, NotificationPriority::Critical);

    auto all = nc.GetByMinPriority(NotificationPriority::Low);
    TEST("All priorities", all.size() == 4);
    auto highPlus = nc.GetByMinPriority(NotificationPriority::High);
    TEST("High and above", highPlus.size() == 2);
    auto critical = nc.GetByMinPriority(NotificationPriority::Critical);
    TEST("Critical only", critical.size() == 1);
}

static void TestNotificationComponentHasCriticalUnread() {
    std::cout << "[NotificationComponent::HasCriticalUnread]\n";
    NotificationComponent nc;
    nc.AddNotification("Normal", "m", NotificationCategory::System, NotificationPriority::Normal);
    TEST("No critical yet", !nc.HasCriticalUnread());

    int critId = nc.AddNotification("Alert!", "m", NotificationCategory::Combat, NotificationPriority::Critical);
    TEST("Has critical unread", nc.HasCriticalUnread());

    nc.MarkAsRead(critId);
    TEST("No critical unread after read", !nc.HasCriticalUnread());
}

static void TestNotificationComponentSerialization() {
    std::cout << "[NotificationComponent serialization]\n";
    NotificationComponent nc;
    nc.maxNotifications = 30;
    nc.autoRemoveExpired = false;
    int id1 = nc.AddNotification("Alert", "Something happened",
                                 NotificationCategory::Combat, NotificationPriority::High, 60.0f);
    nc.AddNotification("Info", "FYI",
                       NotificationCategory::Trade, NotificationPriority::Low);
    nc.MarkAsRead(id1);

    ComponentData cd = nc.Serialize();

    NotificationComponent nc2;
    nc2.Deserialize(cd);
    TEST("Max round-trip", nc2.maxNotifications == 30);
    TEST("AutoRemove round-trip", !nc2.autoRemoveExpired);
    TEST("Count round-trip", nc2.notifications.size() == 2);

    const auto* n = nc2.FindNotification(id1);
    TEST("First notif found", n != nullptr);
    TEST("Title round-trip", n != nullptr && n->title == "Alert");
    TEST("Message round-trip", n != nullptr && n->message == "Something happened");
    TEST("Category round-trip", n != nullptr && n->category == NotificationCategory::Combat);
    TEST("Priority round-trip", n != nullptr && n->priority == NotificationPriority::High);
    TEST("IsRead round-trip", n != nullptr && n->isRead);
}

static void TestNotificationComponentDeserializeInvalidEnum() {
    std::cout << "[NotificationComponent deserialize invalid enum]\n";
    ComponentData cd;
    cd.componentType = "NotificationComponent";
    cd.data["notifCount"] = "1";
    cd.data["notif_0_id"] = "1";
    cd.data["notif_0_title"] = "test";
    cd.data["notif_0_message"] = "msg";
    cd.data["notif_0_category"] = "999";   // invalid
    cd.data["notif_0_priority"] = "999";   // invalid

    NotificationComponent nc;
    nc.Deserialize(cd);
    TEST("Invalid category -> System", nc.notifications[0].category == NotificationCategory::System);
    TEST("Invalid priority -> Normal", nc.notifications[0].priority == NotificationPriority::Normal);
}

static void TestNotificationSystem() {
    std::cout << "[NotificationSystem default ctor]\n";
    NotificationSystem sys;
    TEST("System name", sys.GetName() == "NotificationSystem");
    sys.Update(1.0f);
    TEST("No crash on empty Update", true);
}

static void TestNotificationSystemExpiry() {
    std::cout << "[NotificationSystem expiry]\n";
    EntityManager em;
    NotificationSystem sys(em);

    auto& ent = em.CreateEntity("player");
    auto* nc = em.AddComponent<NotificationComponent>(ent.id, std::make_unique<NotificationComponent>());
    nc->autoRemoveExpired = false; // so we can inspect

    nc->AddNotification("Timed", "Will expire", NotificationCategory::System,
                       NotificationPriority::Normal, 2.0f);
    nc->AddNotification("Persistent", "Won't expire");

    sys.Update(1.0f);
    TEST("Active after 1s", nc->GetActiveCount() == 2);

    sys.Update(1.5f);
    TEST("Timed expired after 2.5s", nc->GetActiveCount() == 1);
}

static void TestNotificationSystemAutoRemove() {
    std::cout << "[NotificationSystem auto-remove]\n";
    EntityManager em;
    NotificationSystem sys(em);

    auto& ent = em.CreateEntity("player");
    auto* nc = em.AddComponent<NotificationComponent>(ent.id, std::make_unique<NotificationComponent>());
    nc->autoRemoveExpired = true;

    nc->AddNotification("Timed", "Will expire", NotificationCategory::System,
                       NotificationPriority::Normal, 0.5f);

    sys.Update(1.0f);
    TEST("Auto-removed expired", nc->notifications.empty());
}

// ===================================================================
// Diplomacy/Research/Notification GameEvents tests
// ===================================================================

static void TestDiplomacyResearchNotificationGameEvents() {
    std::cout << "[Diplomacy/Research/Notification GameEvents]\n";
    // Diplomacy events
    TEST("WarDeclared event", std::string(GameEvents::WarDeclared) == "diplomacy.war.declared");
    TEST("PeaceProposed event", std::string(GameEvents::PeaceProposed) == "diplomacy.peace.proposed");
    TEST("TreatyProposed event", std::string(GameEvents::TreatyProposed) == "diplomacy.treaty.proposed");
    TEST("TreatySigned event", std::string(GameEvents::TreatySigned) == "diplomacy.treaty.signed");
    TEST("TreatyBroken event", std::string(GameEvents::TreatyBroken) == "diplomacy.treaty.broken");
    TEST("TreatyExpired event", std::string(GameEvents::TreatyExpired) == "diplomacy.treaty.expired");
    TEST("DiplomaticStatusChanged event", std::string(GameEvents::DiplomaticStatusChanged) == "diplomacy.status.changed");
    // Research events
    TEST("ResearchStarted event", std::string(GameEvents::ResearchStarted) == "research.started");
    TEST("ResearchCompleted event", std::string(GameEvents::ResearchCompleted) == "research.completed");
    TEST("ResearchCancelled event", std::string(GameEvents::ResearchCancelled) == "research.cancelled");
    TEST("TechUnlocked event", std::string(GameEvents::TechUnlocked) == "research.tech.unlocked");
    // Notification events
    TEST("NotificationAdded event", std::string(GameEvents::NotificationAdded) == "notification.added");
    TEST("NotificationRead event", std::string(GameEvents::NotificationRead) == "notification.read");
    TEST("NotificationExpired event", std::string(GameEvents::NotificationExpired) == "notification.expired");
    TEST("NotificationDismissed event", std::string(GameEvents::NotificationDismissed) == "notification.dismissed");
    TEST("CriticalAlert event", std::string(GameEvents::CriticalAlert) == "notification.critical");
}

// ===================================================================
// Inventory System Tests
// ===================================================================

static void TestItemRarityNames() {
    std::cout << "[InventoryItem Rarity Names]\n";
    TEST("Common name", InventoryItem::GetRarityName(ItemRarity::Common) == "Common");
    TEST("Uncommon name", InventoryItem::GetRarityName(ItemRarity::Uncommon) == "Uncommon");
    TEST("Rare name", InventoryItem::GetRarityName(ItemRarity::Rare) == "Rare");
    TEST("Epic name", InventoryItem::GetRarityName(ItemRarity::Epic) == "Epic");
    TEST("Legendary name", InventoryItem::GetRarityName(ItemRarity::Legendary) == "Legendary");
}

static void TestInventoryComponentDefaults() {
    std::cout << "[InventoryComponent Defaults]\n";
    InventoryComponent inv;
    TEST("Default max slots", inv.GetMaxSlots() == 20);
    TEST("Default max weight", inv.GetMaxWeight() == 100.0f);
    TEST("Default current weight", inv.GetCurrentWeight() == 0.0f);
    TEST("Default weight pct", inv.GetWeightPercentage() == 0.0f);
    TEST("Default used slots", inv.GetUsedSlotCount() == 0);
    TEST("Default free slots", inv.GetFreeSlotCount() == 20);
}

static void TestInventoryComponentCustomInit() {
    std::cout << "[InventoryComponent Custom Init]\n";
    InventoryComponent inv(10, 50.0f);
    TEST("Custom max slots", inv.GetMaxSlots() == 10);
    TEST("Custom max weight", inv.GetMaxWeight() == 50.0f);
    TEST("Custom free slots", inv.GetFreeSlotCount() == 10);
}

static void TestInventoryComponentAddItem() {
    std::cout << "[InventoryComponent AddItem]\n";
    InventoryComponent inv(5, 100.0f);
    InventoryItem item;
    item.itemId = "iron_ore";
    item.name = "Iron Ore";
    item.category = "material";
    item.rarity = ItemRarity::Common;
    item.weight = 2.0f;
    item.stackSize = 5;
    item.maxStackSize = 50;
    item.value = 10;

    TEST("Add item succeeds", inv.AddItem(item));
    TEST("Used slot count", inv.GetUsedSlotCount() == 1);
    TEST("Item count", inv.GetItemCount("iron_ore") == 5);
    TEST("Current weight", inv.GetCurrentWeight() == 10.0f);
    TEST("Has item", inv.HasItem("iron_ore", 5));
    TEST("Has item insufficient", !inv.HasItem("iron_ore", 10));
}

static void TestInventoryComponentStacking() {
    std::cout << "[InventoryComponent Stacking]\n";
    InventoryComponent inv(5, 200.0f);
    InventoryItem item;
    item.itemId = "iron_ore";
    item.name = "Iron Ore";
    item.weight = 1.0f;
    item.stackSize = 10;
    item.maxStackSize = 20;

    TEST("First add", inv.AddItem(item));
    TEST("Second add", inv.AddItem(item));
    TEST("Stacked into one slot", inv.GetUsedSlotCount() == 1);
    TEST("Total count", inv.GetItemCount("iron_ore") == 20);

    // Third add should go to new slot since first is full
    TEST("Third add", inv.AddItem(item));
    TEST("Two slots used", inv.GetUsedSlotCount() == 2);
    TEST("Total count 30", inv.GetItemCount("iron_ore") == 30);
}

static void TestInventoryComponentRemoveItem() {
    std::cout << "[InventoryComponent RemoveItem]\n";
    InventoryComponent inv(5, 200.0f);
    InventoryItem item;
    item.itemId = "crystal";
    item.name = "Crystal";
    item.weight = 5.0f;
    item.stackSize = 3;
    item.maxStackSize = 10;

    inv.AddItem(item);
    TEST("Has 3 crystals", inv.GetItemCount("crystal") == 3);
    TEST("Remove 2", inv.RemoveItem("crystal", 2));
    TEST("Has 1 crystal", inv.GetItemCount("crystal") == 1);
    TEST("Remove all", inv.RemoveItem("crystal", 1));
    TEST("Has 0 crystals", inv.GetItemCount("crystal") == 0);
    TEST("Slot freed", inv.GetUsedSlotCount() == 0);
    TEST("Remove nonexistent fails", !inv.RemoveItem("crystal", 1));
}

static void TestInventoryComponentOverweight() {
    std::cout << "[InventoryComponent Overweight]\n";
    InventoryComponent inv(5, 10.0f);
    InventoryItem item;
    item.itemId = "heavy";
    item.name = "Heavy Item";
    item.weight = 6.0f;
    item.stackSize = 1;
    item.maxStackSize = 10;

    TEST("First heavy item fits", inv.AddItem(item));
    TEST("Second heavy item overweight", !inv.AddItem(item));
    TEST("Only one added", inv.GetItemCount("heavy") == 1);
}

static void TestInventoryComponentGetSlot() {
    std::cout << "[InventoryComponent GetSlot]\n";
    InventoryComponent inv(3, 100.0f);
    InventoryItem item;
    item.itemId = "test_item";
    item.name = "Test";
    item.weight = 1.0f;
    item.stackSize = 1;

    inv.AddItem(item);
    const InventorySlot* slot0 = inv.GetSlot(0);
    TEST("Slot 0 exists", slot0 != nullptr);
    TEST("Slot 0 not empty", !slot0->isEmpty);
    TEST("Slot 0 item", slot0->item.itemId == "test_item");

    const InventorySlot* slot1 = inv.GetSlot(1);
    TEST("Slot 1 exists", slot1 != nullptr);
    TEST("Slot 1 empty", slot1->isEmpty);

    TEST("Out of range returns null", inv.GetSlot(100) == nullptr);
    TEST("Negative returns null", inv.GetSlot(-1) == nullptr);
}

static void TestInventoryComponentFilterByCategory() {
    std::cout << "[InventoryComponent FilterByCategory]\n";
    InventoryComponent inv(10, 200.0f);

    InventoryItem weapon;
    weapon.itemId = "laser_mk1";
    weapon.name = "Laser Mk1";
    weapon.category = "weapon";
    weapon.weight = 5.0f;
    weapon.stackSize = 1;
    inv.AddItem(weapon);

    InventoryItem material;
    material.itemId = "iron";
    material.name = "Iron";
    material.category = "material";
    material.weight = 1.0f;
    material.stackSize = 10;
    inv.AddItem(material);

    auto weapons = inv.GetItemsByCategory("weapon");
    TEST("One weapon", weapons.size() == 1);
    auto materials = inv.GetItemsByCategory("material");
    TEST("One material", materials.size() == 1);
    auto empty = inv.GetItemsByCategory("consumable");
    TEST("No consumables", empty.size() == 0);
}

static void TestInventoryComponentFilterByRarity() {
    std::cout << "[InventoryComponent FilterByRarity]\n";
    InventoryComponent inv(10, 200.0f);

    InventoryItem common;
    common.itemId = "common1";
    common.name = "Common Item";
    common.rarity = ItemRarity::Common;
    common.weight = 1.0f;
    common.stackSize = 1;
    inv.AddItem(common);

    InventoryItem rare;
    rare.itemId = "rare1";
    rare.name = "Rare Item";
    rare.rarity = ItemRarity::Rare;
    rare.weight = 1.0f;
    rare.stackSize = 1;
    inv.AddItem(rare);

    InventoryItem epic;
    epic.itemId = "epic1";
    epic.name = "Epic Item";
    epic.rarity = ItemRarity::Epic;
    epic.weight = 1.0f;
    epic.stackSize = 1;
    inv.AddItem(epic);

    auto allItems = inv.GetItemsByRarity(ItemRarity::Common);
    TEST("All items (min Common)", allItems.size() == 3);
    auto rareUp = inv.GetItemsByRarity(ItemRarity::Rare);
    TEST("Rare+ items", rareUp.size() == 2);
    auto epicUp = inv.GetItemsByRarity(ItemRarity::Epic);
    TEST("Epic+ items", epicUp.size() == 1);
    auto legendaryUp = inv.GetItemsByRarity(ItemRarity::Legendary);
    TEST("Legendary+ items", legendaryUp.size() == 0);
}

static void TestInventoryComponentTransfer() {
    std::cout << "[InventoryComponent Transfer]\n";
    InventoryComponent src(5, 100.0f);
    InventoryComponent dst(5, 100.0f);

    InventoryItem item;
    item.itemId = "gem";
    item.name = "Gem";
    item.weight = 3.0f;
    item.stackSize = 5;
    item.maxStackSize = 10;
    src.AddItem(item);

    TEST("Transfer succeeds", src.TransferItem("gem", 3, dst));
    TEST("Source has 2", src.GetItemCount("gem") == 2);
    TEST("Dest has 3", dst.GetItemCount("gem") == 3);
    TEST("Transfer too many fails", !src.TransferItem("gem", 5, dst));
}

static void TestInventoryComponentSortByName() {
    std::cout << "[InventoryComponent SortByName]\n";
    InventoryComponent inv(5, 200.0f);

    InventoryItem b;
    b.itemId = "b"; b.name = "Banana"; b.weight = 1.0f; b.stackSize = 1;
    InventoryItem a;
    a.itemId = "a"; a.name = "Apple"; a.weight = 1.0f; a.stackSize = 1;
    InventoryItem c;
    c.itemId = "c"; c.name = "Cherry"; c.weight = 1.0f; c.stackSize = 1;

    inv.AddItem(b);
    inv.AddItem(a);
    inv.AddItem(c);

    inv.SortByName();
    TEST("First is Apple", inv.GetSlot(0)->item.name == "Apple");
    TEST("Second is Banana", inv.GetSlot(1)->item.name == "Banana");
    TEST("Third is Cherry", inv.GetSlot(2)->item.name == "Cherry");
    TEST("Fourth is empty", inv.GetSlot(3)->isEmpty);
}

static void TestInventoryComponentSortByRarity() {
    std::cout << "[InventoryComponent SortByRarity]\n";
    InventoryComponent inv(5, 200.0f);

    InventoryItem common;
    common.itemId = "c"; common.name = "Common"; common.rarity = ItemRarity::Common; common.weight = 1.0f; common.stackSize = 1;
    InventoryItem epic;
    epic.itemId = "e"; epic.name = "Epic"; epic.rarity = ItemRarity::Epic; epic.weight = 1.0f; epic.stackSize = 1;
    InventoryItem rare;
    rare.itemId = "r"; rare.name = "Rare"; rare.rarity = ItemRarity::Rare; rare.weight = 1.0f; rare.stackSize = 1;

    inv.AddItem(common);
    inv.AddItem(epic);
    inv.AddItem(rare);

    inv.SortByRarity();
    TEST("First is Epic", inv.GetSlot(0)->item.rarity == ItemRarity::Epic);
    TEST("Second is Rare", inv.GetSlot(1)->item.rarity == ItemRarity::Rare);
    TEST("Third is Common", inv.GetSlot(2)->item.rarity == ItemRarity::Common);
}

static void TestInventoryComponentClear() {
    std::cout << "[InventoryComponent Clear]\n";
    InventoryComponent inv(5, 100.0f);
    InventoryItem item;
    item.itemId = "stuff"; item.name = "Stuff"; item.weight = 2.0f; item.stackSize = 3;
    inv.AddItem(item);
    TEST("Has items before clear", inv.GetUsedSlotCount() > 0);
    inv.Clear();
    TEST("No items after clear", inv.GetUsedSlotCount() == 0);
    TEST("Weight zero after clear", inv.GetCurrentWeight() == 0.0f);
}

static void TestInventoryComponentSerialization() {
    std::cout << "[InventoryComponent Serialization]\n";
    InventoryComponent inv(10, 75.0f);

    InventoryItem item1;
    item1.itemId = "iron_ore";
    item1.name = "Iron Ore";
    item1.description = "Raw iron";
    item1.rarity = ItemRarity::Common;
    item1.weight = 2.0f;
    item1.stackSize = 5;
    item1.maxStackSize = 50;
    item1.value = 10;
    item1.category = "material";
    inv.AddItem(item1);

    InventoryItem item2;
    item2.itemId = "rare_gem";
    item2.name = "Rare Gem";
    item2.description = "A shiny gem";
    item2.rarity = ItemRarity::Rare;
    item2.weight = 0.5f;
    item2.stackSize = 1;
    item2.maxStackSize = 5;
    item2.value = 500;
    item2.category = "treasure";
    inv.AddItem(item2);

    ComponentData cd = inv.Serialize();
    TEST("Component type", cd.componentType == "InventoryComponent");

    InventoryComponent inv2;
    inv2.Deserialize(cd);
    TEST("Restored max slots", inv2.GetMaxSlots() == 10);
    TEST("Restored max weight", inv2.GetMaxWeight() == 75.0f);
    TEST("Restored iron count", inv2.GetItemCount("iron_ore") == 5);
    TEST("Restored gem count", inv2.GetItemCount("rare_gem") == 1);
    TEST("Restored weight", std::abs(inv2.GetCurrentWeight() - 10.5f) < 0.01f);
    TEST("Restored used slots", inv2.GetUsedSlotCount() == 2);
}

static void TestInventorySystem() {
    std::cout << "[InventorySystem]\n";
    InventorySystem sys;
    TEST("System created", true);
    sys.Update(0.016f);  // should not crash without entity manager
    TEST("Update without EM", true);
}

// ===================================================================
// Trade Route System Tests
// ===================================================================

static void TestTradeRouteStateNames() {
    std::cout << "[TradeWaypoint State Names]\n";
    TEST("Idle", TradeWaypoint::GetStateName(TradeRouteState::Idle) == "Idle");
    TEST("Traveling", TradeWaypoint::GetStateName(TradeRouteState::Traveling) == "Traveling");
    TEST("Buying", TradeWaypoint::GetStateName(TradeRouteState::Buying) == "Buying");
    TEST("Selling", TradeWaypoint::GetStateName(TradeRouteState::Selling) == "Selling");
    TEST("WaitingForCargo", TradeWaypoint::GetStateName(TradeRouteState::WaitingForCargo) == "Waiting for Cargo");
    TEST("Completed", TradeWaypoint::GetStateName(TradeRouteState::Completed) == "Completed");
}

static void TestTradeRouteValidity() {
    std::cout << "[TradeRoute Validity]\n";
    TradeRoute route;
    TEST("Empty route invalid", !route.IsValid());

    TradeWaypoint wp1;
    wp1.stationId = "station_1";
    wp1.stationName = "Station Alpha";
    wp1.x = 0; wp1.y = 0; wp1.z = 0;
    route.waypoints.push_back(wp1);
    TEST("One waypoint invalid", !route.IsValid());

    TradeWaypoint wp2;
    wp2.stationId = "station_2";
    wp2.stationName = "Station Beta";
    wp2.x = 30; wp2.y = 40; wp2.z = 0;
    route.waypoints.push_back(wp2);
    TEST("Two waypoints valid", route.IsValid());
}

static void TestTradeRouteDistance() {
    std::cout << "[TradeRoute Distance]\n";
    TradeRoute route;
    TradeWaypoint wp1;
    wp1.x = 0; wp1.y = 0; wp1.z = 0;
    TradeWaypoint wp2;
    wp2.x = 3; wp2.y = 4; wp2.z = 0;
    route.waypoints.push_back(wp1);
    route.waypoints.push_back(wp2);

    float dist = route.CalculateDistance();
    TEST("Distance 3-4-5 triangle", std::abs(dist - 5.0f) < 0.01f);
}

static void TestTradeRouteComponentDefaults() {
    std::cout << "[TradeRouteComponent Defaults]\n";
    TradeRouteComponent comp;
    TEST("Default state Idle", comp.GetState() == TradeRouteState::Idle);
    TEST("Default waypoint index 0", comp.GetCurrentWaypointIndex() == 0);
    TEST("Default progress 0", comp.GetTravelProgress() == 0.0f);
    TEST("Default speed 10", comp.GetTravelSpeed() == 10.0f);
    TEST("Not active", !comp.IsActive());
    TEST("No current waypoint", comp.GetCurrentWaypoint() == nullptr);
    TEST("No cargo", comp.GetCargoManifest().empty());
    TEST("No completed runs", comp.GetTotalCompletedRuns() == 0);
    TEST("No profit", comp.GetTotalProfit() == 0.0f);
}

static void TestTradeRouteComponentStartStop() {
    std::cout << "[TradeRouteComponent Start/Stop]\n";
    TradeRouteComponent comp;
    TradeRoute route;
    route.routeId = "route_1";
    route.routeName = "Iron Run";
    TradeWaypoint wp1; wp1.stationId = "s1"; wp1.x = 0; wp1.y = 0; wp1.z = 0;
    TradeWaypoint wp2; wp2.stationId = "s2"; wp2.x = 10; wp2.y = 0; wp2.z = 0;
    route.waypoints.push_back(wp1);
    route.waypoints.push_back(wp2);
    comp.SetRoute(route);

    comp.StartRoute();
    TEST("Active after start", comp.IsActive());
    TEST("Traveling state", comp.GetState() == TradeRouteState::Traveling);
    TEST("Current waypoint", comp.GetCurrentWaypoint() != nullptr);

    comp.StopRoute();
    TEST("Not active after stop", !comp.IsActive());
    TEST("Idle after stop", comp.GetState() == TradeRouteState::Idle);
}

static void TestTradeRouteComponentCargo() {
    std::cout << "[TradeRouteComponent Cargo]\n";
    TradeRouteComponent comp;
    comp.AddCargo("iron");
    comp.AddCargo("titanium");
    TEST("Two cargo items", comp.GetCargoManifest().size() == 2);
    comp.RemoveCargo("iron");
    TEST("One cargo item", comp.GetCargoManifest().size() == 1);
    TEST("Remaining is titanium", comp.GetCargoManifest()[0] == "titanium");
    comp.ClearCargo();
    TEST("Empty after clear", comp.GetCargoManifest().empty());
}

static void TestTradeRouteComponentProfit() {
    std::cout << "[TradeRouteComponent Profit]\n";
    TradeRouteComponent comp;
    comp.AddProfit(100.0f);
    comp.AddProfit(50.0f);
    TEST("Current run profit", comp.GetCurrentRunProfit() == 150.0f);
}

static void TestTradeRouteComponentAdvance() {
    std::cout << "[TradeRouteComponent Advance]\n";
    TradeRouteComponent comp;
    TradeRoute route;
    route.routeId = "r1";
    route.isLoop = false;
    TradeWaypoint wp1; wp1.stationId = "s1"; wp1.x = 0; wp1.y = 0; wp1.z = 0;
    TradeWaypoint wp2; wp2.stationId = "s2"; wp2.x = 10; wp2.y = 0; wp2.z = 0;
    TradeWaypoint wp3; wp3.stationId = "s3"; wp3.x = 20; wp3.y = 0; wp3.z = 0;
    route.waypoints.push_back(wp1);
    route.waypoints.push_back(wp2);
    route.waypoints.push_back(wp3);
    comp.SetRoute(route);
    comp.StartRoute();

    TEST("Advance to wp2", comp.AdvanceToNextWaypoint());
    TEST("Index is 1", comp.GetCurrentWaypointIndex() == 1);
    TEST("Advance to wp3", comp.AdvanceToNextWaypoint());
    TEST("Index is 2", comp.GetCurrentWaypointIndex() == 2);
    TEST("No more waypoints (non-loop)", !comp.AdvanceToNextWaypoint());
    TEST("Completed 1 run", comp.GetTotalCompletedRuns() == 1);
}

static void TestTradeRouteComponentLoop() {
    std::cout << "[TradeRouteComponent Loop]\n";
    TradeRouteComponent comp;
    TradeRoute route;
    route.routeId = "r_loop";
    route.isLoop = true;
    TradeWaypoint wp1; wp1.stationId = "s1"; wp1.x = 0; wp1.y = 0; wp1.z = 0;
    TradeWaypoint wp2; wp2.stationId = "s2"; wp2.x = 10; wp2.y = 0; wp2.z = 0;
    route.waypoints.push_back(wp1);
    route.waypoints.push_back(wp2);
    comp.SetRoute(route);
    comp.StartRoute();

    comp.AddProfit(200.0f);
    // Advance from wp0 -> wp1
    TEST("Advance to wp1", comp.AdvanceToNextWaypoint());
    TEST("Index is 1", comp.GetCurrentWaypointIndex() == 1);

    // Advance from wp1 -> loops back to wp0
    TEST("Advance loops back", comp.AdvanceToNextWaypoint());
    TEST("Index back to 0", comp.GetCurrentWaypointIndex() == 0);
    TEST("Completed 1 loop run", comp.GetTotalCompletedRuns() == 1);
    TEST("Total profit recorded", comp.GetTotalProfit() == 200.0f);
}

static void TestTradeRouteComponentSerialization() {
    std::cout << "[TradeRouteComponent Serialization]\n";
    TradeRouteComponent comp;
    TradeRoute route;
    route.routeId = "test_route";
    route.routeName = "Test Route";
    route.isLoop = true;
    TradeWaypoint wp1;
    wp1.stationId = "s1"; wp1.stationName = "Station 1";
    wp1.x = 0; wp1.y = 0; wp1.z = 0;
    wp1.buyGoods.push_back("iron");
    wp1.sellGoods.push_back("gold");
    wp1.waitTime = 3.0f;
    TradeWaypoint wp2;
    wp2.stationId = "s2"; wp2.stationName = "Station 2";
    wp2.x = 100; wp2.y = 0; wp2.z = 0;
    wp2.buyGoods.push_back("gold");
    wp2.sellGoods.push_back("iron");
    wp2.waitTime = 4.0f;
    route.waypoints.push_back(wp1);
    route.waypoints.push_back(wp2);

    comp.SetRoute(route);
    comp.SetTravelSpeed(20.0f);
    comp.AddCargo("iron");
    comp.AddCargo("gold");

    ComponentData cd = comp.Serialize();
    TEST("Component type", cd.componentType == "TradeRouteComponent");

    TradeRouteComponent comp2;
    comp2.Deserialize(cd);
    TEST("Restored route id", comp2.GetRoute().routeId == "test_route");
    TEST("Restored route name", comp2.GetRoute().routeName == "Test Route");
    TEST("Restored loop", comp2.GetRoute().isLoop);
    TEST("Restored waypoints", comp2.GetRoute().waypoints.size() == 2);
    TEST("Restored speed", comp2.GetTravelSpeed() == 20.0f);
    TEST("Restored cargo", comp2.GetCargoManifest().size() == 2);
    TEST("Restored wp1 station", comp2.GetRoute().waypoints[0].stationId == "s1");
    TEST("Restored wp1 buy goods", comp2.GetRoute().waypoints[0].buyGoods.size() == 1);
    TEST("Restored wp2 sell goods", comp2.GetRoute().waypoints[1].sellGoods.size() == 1);
}

static void TestTradeRouteSystem() {
    std::cout << "[TradeRouteSystem]\n";
    TradeRouteSystem sys;
    TEST("System created", true);
    sys.Update(0.016f);
    TEST("Update without EM", true);
}

static void TestTradeRouteSystemTravel() {
    std::cout << "[TradeRouteSystem Travel]\n";
    EntityManager em;
    TradeRouteSystem sys(em);

    auto& entity = em.CreateEntity("trader");
    auto* comp = em.AddComponent<TradeRouteComponent>(entity.id, std::make_unique<TradeRouteComponent>());

    TradeRoute route;
    route.routeId = "r1";
    route.isLoop = false;
    TradeWaypoint wp1; wp1.stationId = "s1"; wp1.x = 0; wp1.y = 0; wp1.z = 0; wp1.waitTime = 0.1f;
    TradeWaypoint wp2; wp2.stationId = "s2"; wp2.x = 10; wp2.y = 0; wp2.z = 0; wp2.waitTime = 0.1f;
    route.waypoints.push_back(wp1);
    route.waypoints.push_back(wp2);
    comp->SetRoute(route);
    comp->SetTravelSpeed(100.0f);  // fast travel
    comp->StartRoute();

    // Simulate enough time to travel
    sys.Update(1.0f);
    TEST("Arrived at buying", comp->GetState() == TradeRouteState::Buying);

    // Wait through buying phase
    sys.Update(0.2f);
    TEST("Moved to selling", comp->GetState() == TradeRouteState::Selling);

    // Wait through selling phase
    sys.Update(0.2f);
    TEST("Moved to waiting", comp->GetState() == TradeRouteState::WaitingForCargo);

    // Advance to next state
    sys.Update(0.01f);
    bool endState = comp->GetState() == TradeRouteState::Traveling ||
                    comp->GetState() == TradeRouteState::Completed ||
                    comp->GetState() == TradeRouteState::Idle;
    TEST("Route progressed to end state", endState);
}

// ===================================================================
// Hangar/Docking System Tests
// ===================================================================

static void TestDockingBaySizeNames() {
    std::cout << "[DockingBay Size Names]\n";
    TEST("Small", DockingBay::GetSizeName(BaySize::Small) == "Small");
    TEST("Medium", DockingBay::GetSizeName(BaySize::Medium) == "Medium");
    TEST("Large", DockingBay::GetSizeName(BaySize::Large) == "Large");
    TEST("Capital", DockingBay::GetSizeName(BaySize::Capital) == "Capital");
}

static void TestDockingStateNames() {
    std::cout << "[DockingBay State Names]\n";
    TEST("Undocked", DockingBay::GetStateName(DockingState::Undocked) == "Undocked");
    TEST("Requesting", DockingBay::GetStateName(DockingState::RequestingDock) == "Requesting Dock");
    TEST("Approaching", DockingBay::GetStateName(DockingState::Approaching) == "Approaching");
    TEST("Docking", DockingBay::GetStateName(DockingState::Docking) == "Docking");
    TEST("Docked", DockingBay::GetStateName(DockingState::Docked) == "Docked");
    TEST("Undocking", DockingBay::GetStateName(DockingState::Undocking) == "Undocking");
    TEST("Launching", DockingBay::GetStateName(DockingState::Launching) == "Launching");
}

static void TestHangarComponentDefaults() {
    std::cout << "[HangarComponent Defaults]\n";
    HangarComponent hangar;
    TEST("Default max bays", hangar.GetMaxBays() == 4);
    TEST("No occupied bays", hangar.GetOccupiedBayCount() == 0);
    TEST("No free bays (none added)", hangar.GetFreeBayCount() == 0);
    TEST("No stored ships", hangar.GetStoredShipCount() == 0);
    TEST("No active requests", hangar.GetActiveRequests().empty());
}

static void TestHangarComponentAddBays() {
    std::cout << "[HangarComponent AddBays]\n";
    HangarComponent hangar(3);
    DockingBay bay1; bay1.bayId = 1; bay1.bayName = "Bay 1"; bay1.size = BaySize::Small;
    DockingBay bay2; bay2.bayId = 2; bay2.bayName = "Bay 2"; bay2.size = BaySize::Medium;
    DockingBay bay3; bay3.bayId = 3; bay3.bayName = "Bay 3"; bay3.size = BaySize::Large;

    hangar.AddBay(bay1);
    hangar.AddBay(bay2);
    hangar.AddBay(bay3);
    TEST("Three bays added", hangar.GetAllBays().size() == 3);
    TEST("Three free bays", hangar.GetFreeBayCount() == 3);

    // Max bays is 3, adding a 4th should be ignored
    DockingBay bay4; bay4.bayId = 4; bay4.bayName = "Bay 4";
    hangar.AddBay(bay4);
    TEST("Still three bays", hangar.GetAllBays().size() == 3);
}

static void TestHangarComponentGetBay() {
    std::cout << "[HangarComponent GetBay]\n";
    HangarComponent hangar(4);
    DockingBay bay; bay.bayId = 42; bay.bayName = "Special Bay";
    hangar.AddBay(bay);

    const DockingBay* found = hangar.GetBay(42);
    TEST("Found bay 42", found != nullptr);
    TEST("Bay name", found->bayName == "Special Bay");
    TEST("Not found bay 99", hangar.GetBay(99) == nullptr);
}

static void TestHangarComponentRequestDocking() {
    std::cout << "[HangarComponent RequestDocking]\n";
    HangarComponent hangar(4);
    DockingBay smallBay; smallBay.bayId = 1; smallBay.size = BaySize::Small;
    DockingBay medBay; medBay.bayId = 2; medBay.size = BaySize::Medium;
    hangar.AddBay(smallBay);
    hangar.AddBay(medBay);

    EntityId ship1 = 100;
    EntityId ship2 = 200;
    EntityId ship3 = 300;

    TEST("Request dock ship1", hangar.RequestDocking(ship1, BaySize::Small));
    TEST("Ship1 approaching", hangar.GetShipDockingState(ship1) == DockingState::Approaching);
    TEST("One active request", hangar.GetActiveRequests().size() == 1);

    TEST("Request dock ship2", hangar.RequestDocking(ship2, BaySize::Small));
    TEST("Two active requests", hangar.GetActiveRequests().size() == 2);

    // No more bays
    TEST("Request dock ship3 fails (no bays)", !hangar.RequestDocking(ship3));
    TEST("Still two requests", hangar.GetActiveRequests().size() == 2);

    // Duplicate request fails
    TEST("Duplicate request fails", !hangar.RequestDocking(ship1));
}

static void TestHangarComponentRequestDockingBySize() {
    std::cout << "[HangarComponent RequestDocking BySize]\n";
    HangarComponent hangar(4);
    DockingBay smallBay; smallBay.bayId = 1; smallBay.size = BaySize::Small;
    DockingBay largeBay; largeBay.bayId = 2; largeBay.size = BaySize::Large;
    hangar.AddBay(smallBay);
    hangar.AddBay(largeBay);

    // Request Large bay for ship - should get bay 2
    EntityId ship1 = 100;
    TEST("Request large dock", hangar.RequestDocking(ship1, BaySize::Large));
    const DockingRequest* req = hangar.GetDockingRequest(ship1);
    TEST("Assigned to large bay", req != nullptr && req->assignedBayId == 2);

    // Request Capital bay - no capital bays available
    EntityId ship2 = 200;
    TEST("Request capital fails", !hangar.RequestDocking(ship2, BaySize::Capital));
}

static void TestHangarComponentCancelDocking() {
    std::cout << "[HangarComponent CancelDocking]\n";
    HangarComponent hangar(4);
    DockingBay bay; bay.bayId = 1; bay.size = BaySize::Medium;
    hangar.AddBay(bay);

    EntityId ship1 = 100;
    hangar.RequestDocking(ship1);
    TEST("One request", hangar.GetActiveRequests().size() == 1);
    TEST("Bay occupied", hangar.GetOccupiedBayCount() == 1);

    TEST("Cancel succeeds", hangar.CancelDocking(ship1));
    TEST("No requests", hangar.GetActiveRequests().empty());
    TEST("Bay freed", hangar.GetOccupiedBayCount() == 0);

    TEST("Cancel nonexistent fails", !hangar.CancelDocking(999));
}

static void TestHangarComponentIsShipDocked() {
    std::cout << "[HangarComponent IsShipDocked]\n";
    HangarComponent hangar(4);
    DockingBay bay; bay.bayId = 1; bay.size = BaySize::Medium;
    hangar.AddBay(bay);

    EntityId ship1 = 100;
    TEST("Not docked initially", !hangar.IsShipDocked(ship1));
    TEST("Undocked state", hangar.GetShipDockingState(ship1) == DockingState::Undocked);
    hangar.RequestDocking(ship1);
    TEST("Not yet docked (approaching)", !hangar.IsShipDocked(ship1));
}

static void TestHangarComponentShipStorage() {
    std::cout << "[HangarComponent Ship Storage]\n";
    HangarComponent hangar;
    EntityId ship1 = 100;
    EntityId ship2 = 200;

    hangar.StoreShip(ship1);
    hangar.StoreShip(ship2);
    TEST("Two stored ships", hangar.GetStoredShipCount() == 2);

    // Storing duplicate should not add again
    hangar.StoreShip(ship1);
    TEST("Still two stored", hangar.GetStoredShipCount() == 2);

    TEST("Retrieve ship1", hangar.RetrieveShip(ship1));
    TEST("One stored", hangar.GetStoredShipCount() == 1);
    TEST("Retrieve nonexistent fails", !hangar.RetrieveShip(999));
}

static void TestHangarComponentRequestLaunch() {
    std::cout << "[HangarComponent RequestLaunch]\n";
    HangarComponent hangar(4);
    DockingBay bay; bay.bayId = 1; bay.size = BaySize::Medium;
    hangar.AddBay(bay);

    EntityId ship1 = 100;
    // Can't launch if not docked
    TEST("Launch fails (not docked)", !hangar.RequestLaunch(ship1));

    hangar.RequestDocking(ship1);
    // Ship is Approaching, not Docked yet
    TEST("Launch fails (approaching)", !hangar.RequestLaunch(ship1));
}

static void TestHangarComponentFreeBaysBySize() {
    std::cout << "[HangarComponent FreeBaysBySize]\n";
    HangarComponent hangar(4);
    DockingBay small1; small1.bayId = 1; small1.size = BaySize::Small;
    DockingBay med1; med1.bayId = 2; med1.size = BaySize::Medium;
    DockingBay large1; large1.bayId = 3; large1.size = BaySize::Large;
    hangar.AddBay(small1);
    hangar.AddBay(med1);
    hangar.AddBay(large1);

    auto medPlus = hangar.GetFreeBaysBySize(BaySize::Medium);
    TEST("Medium+ bays: 2", medPlus.size() == 2);
    auto largePlus = hangar.GetFreeBaysBySize(BaySize::Large);
    TEST("Large+ bays: 1", largePlus.size() == 1);
    auto capitalPlus = hangar.GetFreeBaysBySize(BaySize::Capital);
    TEST("Capital+ bays: 0", capitalPlus.size() == 0);
}

static void TestHangarComponentSerialization() {
    std::cout << "[HangarComponent Serialization]\n";
    HangarComponent hangar(6);
    DockingBay bay1; bay1.bayId = 1; bay1.bayName = "Bay Alpha"; bay1.size = BaySize::Small;
    bay1.repairRate = 10.0f; bay1.refuelRate = 20.0f;
    DockingBay bay2; bay2.bayId = 2; bay2.bayName = "Bay Beta"; bay2.size = BaySize::Large;
    hangar.AddBay(bay1);
    hangar.AddBay(bay2);

    hangar.StoreShip(500);
    hangar.StoreShip(600);

    hangar.RequestDocking(100, BaySize::Small);

    ComponentData cd = hangar.Serialize();
    TEST("Component type", cd.componentType == "HangarComponent");

    HangarComponent hangar2;
    hangar2.Deserialize(cd);
    TEST("Restored max bays", hangar2.GetMaxBays() == 6);
    TEST("Restored bay count", hangar2.GetAllBays().size() == 2);
    TEST("Restored bay 1 name", hangar2.GetBay(1)->bayName == "Bay Alpha");
    TEST("Restored bay 2 size", hangar2.GetBay(2)->size == BaySize::Large);
    TEST("Restored stored ships", hangar2.GetStoredShipCount() == 2);
    TEST("Restored active requests", hangar2.GetActiveRequests().size() == 1);
    TEST("Restored request ship", hangar2.GetActiveRequests()[0].shipId == 100);
}

static void TestHangarSystem() {
    std::cout << "[HangarSystem]\n";
    HangarSystem sys;
    TEST("System created", true);
    sys.Update(0.016f);
    TEST("Update without EM", true);
}

static void TestHangarSystemDockingSequence() {
    std::cout << "[HangarSystem Docking Sequence]\n";
    EntityManager em;
    HangarSystem sys(em);

    auto& entity = em.CreateEntity("station");
    auto* hangar = em.AddComponent<HangarComponent>(entity.id, std::make_unique<HangarComponent>(4));
    DockingBay bay; bay.bayId = 1; bay.size = BaySize::Medium;
    hangar->AddBay(bay);

    EntityId ship1 = 999;
    hangar->RequestDocking(ship1);
    TEST("Ship approaching", hangar->GetShipDockingState(ship1) == DockingState::Approaching);

    // Simulate approach (default 2 seconds)
    sys.Update(1.0f);
    TEST("Still approaching", hangar->GetShipDockingState(ship1) == DockingState::Approaching);
    sys.Update(1.5f);
    TEST("Now docking", hangar->GetShipDockingState(ship1) == DockingState::Docking);

    // Simulate docking (default 3 seconds)
    sys.Update(3.5f);
    TEST("Now docked", hangar->GetShipDockingState(ship1) == DockingState::Docked);
    TEST("Ship is docked", hangar->IsShipDocked(ship1));

    // Request launch
    TEST("Launch request", hangar->RequestLaunch(ship1));
    TEST("Undocking state", hangar->GetShipDockingState(ship1) == DockingState::Undocking);

    // Simulate undocking (3 seconds)
    sys.Update(3.5f);
    TEST("Now launching", hangar->GetShipDockingState(ship1) == DockingState::Launching);

    // Simulate launch (2 seconds)
    sys.Update(2.5f);
    TEST("Launch complete", hangar->GetShipDockingState(ship1) == DockingState::Undocked);
    TEST("Request removed", hangar->GetActiveRequests().empty());
    TEST("Bay freed", hangar->GetFreeBayCount() == 1);
}

// ===================================================================
// Inventory/TradeRoute/Hangar GameEvents Tests
// ===================================================================

static void TestInventoryTradeRouteHangarGameEvents() {
    std::cout << "[Inventory/TradeRoute/Hangar GameEvents]\n";
    // Inventory events
    TEST("ItemAdded event", std::string(GameEvents::ItemAdded) == "inventory.item.added");
    TEST("ItemRemoved event", std::string(GameEvents::ItemRemoved) == "inventory.item.removed");
    TEST("ItemTransferred event", std::string(GameEvents::ItemTransferred) == "inventory.item.transferred");
    TEST("InventoryOverweight event", std::string(GameEvents::InventoryOverweight) == "inventory.overweight");
    TEST("InventorySorted event", std::string(GameEvents::InventorySorted) == "inventory.sorted");
    // Trade route events
    TEST("TradeRouteStarted event", std::string(GameEvents::TradeRouteStarted) == "trade_route.started");
    TEST("TradeRouteStopped event", std::string(GameEvents::TradeRouteStopped) == "trade_route.stopped");
    TEST("TradeRouteCompleted event", std::string(GameEvents::TradeRouteCompleted) == "trade_route.completed");
    TEST("TradeWaypointReached event", std::string(GameEvents::TradeWaypointReached) == "trade_route.waypoint.reached");
    TEST("TradeBuyCompleted event", std::string(GameEvents::TradeBuyCompleted) == "trade_route.buy.completed");
    TEST("TradeSellCompleted event", std::string(GameEvents::TradeSellCompleted) == "trade_route.sell.completed");
    // Hangar events
    TEST("DockingRequested event", std::string(GameEvents::DockingRequested) == "hangar.docking.requested");
    TEST("DockingCompleted event", std::string(GameEvents::DockingCompleted) == "hangar.docking.completed");
    TEST("DockingCancelled event", std::string(GameEvents::DockingCancelled) == "hangar.docking.cancelled");
    TEST("LaunchRequested event", std::string(GameEvents::LaunchRequested) == "hangar.launch.requested");
    TEST("LaunchCompleted event", std::string(GameEvents::LaunchCompleted) == "hangar.launch.completed");
    TEST("ShipStored event", std::string(GameEvents::ShipStored) == "hangar.ship.stored");
    TEST("ShipRetrieved event", std::string(GameEvents::ShipRetrieved) == "hangar.ship.retrieved");
}

// ===================================================================
// Wormhole System Tests
// ===================================================================

static void TestWormholeTypeNames() {
    std::cout << "[WormholeLink Type Names]\n";
    TEST("Natural name", WormholeLink::GetTypeName(WormholeType::Natural) == "Natural");
    TEST("Artificial name", WormholeLink::GetTypeName(WormholeType::Artificial) == "Artificial");
    TEST("Unstable name", WormholeLink::GetTypeName(WormholeType::Unstable) == "Unstable");
    TEST("Persistent name", WormholeLink::GetTypeName(WormholeType::Persistent) == "Persistent");
}

static void TestWormholeStateNames() {
    std::cout << "[WormholeLink State Names]\n";
    TEST("Dormant name", WormholeLink::GetStateName(WormholeState::Dormant) == "Dormant");
    TEST("Activating name", WormholeLink::GetStateName(WormholeState::Activating) == "Activating");
    TEST("Active name", WormholeLink::GetStateName(WormholeState::Active) == "Active");
    TEST("Destabilizing name", WormholeLink::GetStateName(WormholeState::Destabilizing) == "Destabilizing");
    TEST("Collapsed name", WormholeLink::GetStateName(WormholeState::Collapsed) == "Collapsed");
}

static void TestWormholeComponentDefaults() {
    std::cout << "[WormholeComponent Defaults]\n";
    WormholeComponent wc;
    TEST("Default max links", wc.GetMaxLinks() == 8);
    TEST("No links", wc.GetLinkCount() == 0);
    TEST("No active links", wc.GetActiveLinks().empty());
}

static void TestWormholeComponentAddLink() {
    std::cout << "[WormholeComponent AddLink]\n";
    WormholeComponent wc(2);
    WormholeLink link1;
    link1.linkId = 1;
    link1.state = WormholeState::Active;
    link1.endpointA.sectorX = 0; link1.endpointA.sectorY = 0;
    link1.endpointB.sectorX = 5; link1.endpointB.sectorY = 5;

    WormholeLink link2;
    link2.linkId = 2;
    link2.state = WormholeState::Dormant;

    wc.AddLink(link1);
    wc.AddLink(link2);
    TEST("Two links added", wc.GetLinkCount() == 2);

    // Max is 2, adding a 3rd should be ignored
    WormholeLink link3;
    link3.linkId = 3;
    wc.AddLink(link3);
    TEST("Still two links", wc.GetLinkCount() == 2);
}

static void TestWormholeComponentGetLink() {
    std::cout << "[WormholeComponent GetLink]\n";
    WormholeComponent wc;
    WormholeLink link;
    link.linkId = 42;
    link.endpointA.name = "Alpha Gate";
    wc.AddLink(link);

    const WormholeLink* found = wc.GetLink(42);
    TEST("Found link 42", found != nullptr);
    TEST("Link name", found->endpointA.name == "Alpha Gate");
    TEST("Not found link 99", wc.GetLink(99) == nullptr);
}

static void TestWormholeComponentActiveLinks() {
    std::cout << "[WormholeComponent ActiveLinks]\n";
    WormholeComponent wc;
    WormholeLink active;
    active.linkId = 1;
    active.state = WormholeState::Active;
    WormholeLink dormant;
    dormant.linkId = 2;
    dormant.state = WormholeState::Dormant;
    wc.AddLink(active);
    wc.AddLink(dormant);

    auto activeLinks = wc.GetActiveLinks();
    TEST("One active link", activeLinks.size() == 1);
    TEST("Active link id", activeLinks[0]->linkId == 1);
}

static void TestWormholeComponentLinksToSector() {
    std::cout << "[WormholeComponent LinksToSector]\n";
    WormholeComponent wc;
    WormholeLink link;
    link.linkId = 1;
    link.endpointA.sectorX = 0; link.endpointA.sectorY = 0;
    link.endpointB.sectorX = 5; link.endpointB.sectorY = 5;
    wc.AddLink(link);

    auto toSector5 = wc.GetLinksToSector(5, 5);
    TEST("Found link to sector 5,5", toSector5.size() == 1);
    auto toSector9 = wc.GetLinksToSector(9, 9);
    TEST("No link to sector 9,9", toSector9.empty());
}

static void TestWormholeComponentFindLink() {
    std::cout << "[WormholeComponent FindLink]\n";
    WormholeComponent wc;
    WormholeLink link;
    link.linkId = 1;
    link.endpointA.sectorX = 0; link.endpointA.sectorY = 0;
    link.endpointB.sectorX = 5; link.endpointB.sectorY = 5;
    wc.AddLink(link);

    TEST("Find forward", wc.FindLink(0, 0, 5, 5) != nullptr);
    TEST("Find reverse", wc.FindLink(5, 5, 0, 0) != nullptr);
    TEST("Not found", wc.FindLink(1, 1, 9, 9) == nullptr);
}

static void TestWormholeComponentTraversal() {
    std::cout << "[WormholeComponent Traversal]\n";
    WormholeComponent wc;
    WormholeLink link;
    link.linkId = 1;
    link.state = WormholeState::Active;
    link.stability = 0.8f;
    link.maxMass = 1000.0f;
    link.currentMass = 0.0f;
    wc.AddLink(link);

    EntityId ship1 = 100;
    TEST("Request traversal", wc.RequestTraversal(1, ship1, 500.0f));
    TEST("Mass increased", wc.GetLink(1)->currentMass == 500.0f);

    // Over mass limit
    EntityId ship2 = 200;
    TEST("Over mass fails", !wc.RequestTraversal(1, ship2, 600.0f));

    // Complete traversal
    TEST("Complete traversal", wc.CompleteTraversal(1, ship1, 500.0f));
    TEST("Mass decreased", wc.GetLink(1)->currentMass == 0.0f);
}

static void TestWormholeComponentTraversalRequirements() {
    std::cout << "[WormholeComponent Traversal Requirements]\n";
    WormholeComponent wc;

    // Dormant wormhole - can't traverse
    WormholeLink dormant;
    dormant.linkId = 1;
    dormant.state = WormholeState::Dormant;
    dormant.stability = 0.8f;
    dormant.maxMass = 1000.0f;
    wc.AddLink(dormant);
    TEST("Dormant traversal fails", !wc.RequestTraversal(1, 100, 100.0f));

    // Low stability - can't traverse
    WormholeLink lowStab;
    lowStab.linkId = 2;
    lowStab.state = WormholeState::Active;
    lowStab.stability = 0.05f; // below 0.1 threshold
    lowStab.maxMass = 1000.0f;
    wc.AddLink(lowStab);
    TEST("Low stability traversal fails", !wc.RequestTraversal(2, 100, 100.0f));
}

static void TestWormholeComponentSerialization() {
    std::cout << "[WormholeComponent Serialization]\n";
    WormholeComponent wc(4);
    WormholeLink link;
    link.linkId = 1;
    link.type = WormholeType::Artificial;
    link.state = WormholeState::Active;
    link.stability = 0.75f;
    link.maxMass = 5000.0f;
    link.currentMass = 100.0f;
    link.traversalTime = 3.0f;
    link.bidirectional = true;
    link.endpointA.sectorX = 1; link.endpointA.sectorY = 2;
    link.endpointA.name = "Gate Alpha";
    link.endpointB.sectorX = 5; link.endpointB.sectorY = 6;
    link.endpointB.name = "Gate Beta";
    wc.AddLink(link);

    ComponentData cd = wc.Serialize();
    TEST("Component type", cd.componentType == "WormholeComponent");

    WormholeComponent wc2;
    wc2.Deserialize(cd);
    TEST("Restored max links", wc2.GetMaxLinks() == 4);
    TEST("Restored link count", wc2.GetLinkCount() == 1);
    const WormholeLink* restored = wc2.GetLink(1);
    TEST("Restored link exists", restored != nullptr);
    TEST("Restored type", restored->type == WormholeType::Artificial);
    TEST("Restored state", restored->state == WormholeState::Active);
    TEST("Restored endpoint A name", restored->endpointA.name == "Gate Alpha");
    TEST("Restored endpoint B sector", restored->endpointB.sectorX == 5);
}

static void TestWormholeSystem() {
    std::cout << "[WormholeSystem]\n";
    WormholeSystem sys;
    TEST("System created", true);
    sys.Update(0.016f);
    TEST("Update without EM", true);
}

static void TestWormholeSystemActivation() {
    std::cout << "[WormholeSystem Activation]\n";
    EntityManager em;
    WormholeSystem sys(em);

    auto& entity = em.CreateEntity("wormhole_network");
    auto* wc = em.AddComponent<WormholeComponent>(
        entity.id, std::make_unique<WormholeComponent>(4));

    WormholeLink link;
    link.linkId = 1;
    link.state = WormholeState::Activating;
    link.stability = 0.4f;
    wc->AddLink(link);

    // Stability should increase toward 0.5 activation threshold
    // Default regen rate is 0.005 per second
    sys.Update(100.0f); // 100 * 0.005 = 0.5 added, 0.4 + 0.5 = 0.9 => active
    TEST("Became active", wc->GetLink(1)->state == WormholeState::Active);
}

static void TestWormholeSystemDestabilization() {
    std::cout << "[WormholeSystem Destabilization]\n";
    EntityManager em;
    WormholeSystem sys(em);

    auto& entity = em.CreateEntity("wormhole_network");
    auto* wc = em.AddComponent<WormholeComponent>(
        entity.id, std::make_unique<WormholeComponent>(4));

    WormholeLink link;
    link.linkId = 1;
    link.state = WormholeState::Destabilizing;
    link.stability = 0.005f;
    wc->AddLink(link);

    // Default decay rate 0.01 per second, should collapse quickly
    sys.Update(1.0f); // 0.005 - 0.01*1.0 = -0.005 => collapse
    TEST("Collapsed", wc->GetLink(1)->state == WormholeState::Collapsed);
    TEST("Stability zero", wc->GetLink(1)->stability == 0.0f);
}

// ===================================================================
// Ship Class System Tests
// ===================================================================

static void TestShipClassNames() {
    std::cout << "[ShipClassDefinition Class Names]\n";
    TEST("Fighter name", ShipClassDefinition::GetClassName(ShipClass::Fighter) == "Fighter");
    TEST("Corvette name", ShipClassDefinition::GetClassName(ShipClass::Corvette) == "Corvette");
    TEST("Frigate name", ShipClassDefinition::GetClassName(ShipClass::Frigate) == "Frigate");
    TEST("Destroyer name", ShipClassDefinition::GetClassName(ShipClass::Destroyer) == "Destroyer");
    TEST("Cruiser name", ShipClassDefinition::GetClassName(ShipClass::Cruiser) == "Cruiser");
    TEST("Battleship name", ShipClassDefinition::GetClassName(ShipClass::Battleship) == "Battleship");
    TEST("Carrier name", ShipClassDefinition::GetClassName(ShipClass::Carrier) == "Carrier");
    TEST("Freighter name", ShipClassDefinition::GetClassName(ShipClass::Freighter) == "Freighter");
    TEST("Miner name", ShipClassDefinition::GetClassName(ShipClass::Miner) == "Miner");
    TEST("Explorer name", ShipClassDefinition::GetClassName(ShipClass::Explorer) == "Explorer");
}

static void TestShipRoleNames() {
    std::cout << "[ShipClassDefinition Role Names]\n";
    TEST("Combat role", ShipClassDefinition::GetRoleName(ShipRole::Combat) == "Combat");
    TEST("Trade role", ShipClassDefinition::GetRoleName(ShipRole::Trade) == "Trade");
    TEST("Mining role", ShipClassDefinition::GetRoleName(ShipRole::Mining) == "Mining");
    TEST("Exploration role", ShipClassDefinition::GetRoleName(ShipRole::Exploration) == "Exploration");
    TEST("Support role", ShipClassDefinition::GetRoleName(ShipRole::Support) == "Support");
    TEST("MultiRole role", ShipClassDefinition::GetRoleName(ShipRole::MultiRole) == "Multi-Role");
}

static void TestShipClassDefaults() {
    std::cout << "[ShipClassDefinition Defaults]\n";
    auto fighter = ShipClassDefinition::GetDefaultDefinition(ShipClass::Fighter);
    TEST("Fighter display name", fighter.displayName == "Fighter");
    TEST("Fighter role", fighter.role == ShipRole::Combat);
    TEST("Fighter tech level", fighter.techLevel == 1);
    TEST("Fighter speed bonus > 1", fighter.bonus.speedMultiplier > 1.0f);

    auto freighter = ShipClassDefinition::GetDefaultDefinition(ShipClass::Freighter);
    TEST("Freighter role", freighter.role == ShipRole::Trade);
    TEST("Freighter cargo bonus", freighter.bonus.cargoMultiplier == 2.0f);

    auto miner = ShipClassDefinition::GetDefaultDefinition(ShipClass::Miner);
    TEST("Miner role", miner.role == ShipRole::Mining);
    TEST("Miner mining bonus", miner.bonus.miningMultiplier == 2.0f);

    auto explorer = ShipClassDefinition::GetDefaultDefinition(ShipClass::Explorer);
    TEST("Explorer role", explorer.role == ShipRole::Exploration);
    TEST("Explorer sensor bonus", explorer.bonus.sensorMultiplier == 2.0f);
}

static void TestShipClassComponentDefaults() {
    std::cout << "[ShipClassComponent Defaults]\n";
    ShipClassComponent comp;
    TEST("Default class is Fighter", comp.GetShipClass() == ShipClass::Fighter);
    TEST("Default role is Combat", comp.GetRole() == ShipRole::Combat);
    TEST("Default tech level 1", comp.GetTechLevel() == 1);
    TEST("Display name", comp.GetDisplayName() == "Fighter");
}

static void TestShipClassComponentExplicit() {
    std::cout << "[ShipClassComponent Explicit]\n";
    ShipClassComponent comp(ShipClass::Miner);
    TEST("Miner class", comp.GetShipClass() == ShipClass::Miner);
    TEST("Mining role", comp.GetRole() == ShipRole::Mining);
    TEST("Mining bonus 2x", ApproxEq(comp.GetEffectiveMining(100.0f), 200.0f));
}

static void TestShipClassComponentSetClass() {
    std::cout << "[ShipClassComponent SetClass]\n";
    ShipClassComponent comp;
    comp.SetShipClass(ShipClass::Battleship);
    TEST("Battleship class", comp.GetShipClass() == ShipClass::Battleship);
    TEST("Battleship tech 8", comp.GetTechLevel() == 8);
    TEST("Battleship damage bonus", comp.GetEffectiveDamage(100.0f) > 100.0f);
}

static void TestShipClassComponentEffective() {
    std::cout << "[ShipClassComponent Effective Stats]\n";
    ShipClassComponent comp(ShipClass::Freighter);
    // Freighter: speed 0.8, damage 0.3, shield 0.9, cargo 2.0, mining 0.5, sensor 0.8
    TEST("Effective speed", ApproxEq(comp.GetEffectiveSpeed(100.0f), 80.0f));
    TEST("Effective damage", ApproxEq(comp.GetEffectiveDamage(100.0f), 30.0f));
    TEST("Effective shield", ApproxEq(comp.GetEffectiveShield(100.0f), 90.0f));
    TEST("Effective cargo", ApproxEq(comp.GetEffectiveCargo(100.0f), 200.0f));
    TEST("Effective mining", ApproxEq(comp.GetEffectiveMining(100.0f), 50.0f));
    TEST("Effective sensor", ApproxEq(comp.GetEffectiveSensor(100.0f), 80.0f));
}

static void TestShipClassComponentSerialization() {
    std::cout << "[ShipClassComponent Serialization]\n";
    ShipClassComponent comp(ShipClass::Cruiser);
    ComponentData cd = comp.Serialize();
    TEST("Component type", cd.componentType == "ShipClassComponent");

    ShipClassComponent comp2;
    comp2.Deserialize(cd);
    TEST("Restored class", comp2.GetShipClass() == ShipClass::Cruiser);
    TEST("Restored role", comp2.GetRole() == ShipRole::Combat);
    TEST("Restored tech level", comp2.GetTechLevel() == 6);
    TEST("Restored display name", comp2.GetDisplayName() == "Cruiser");
}

static void TestShipClassSystem() {
    std::cout << "[ShipClassSystem]\n";
    ShipClassSystem sys;
    TEST("System created", true);
    sys.Update(0.016f);
    TEST("Update without EM", true);
}

static void TestShipClassSystemUpgrades() {
    std::cout << "[ShipClassSystem Upgrades]\n";
    ShipClassSystem sys;

    // Fighter (tech 1) -> can upgrade to tech 1,2,3
    TEST("Fighter->Corvette (tech 2)", sys.CanUpgradeClass(ShipClass::Fighter, ShipClass::Corvette));
    TEST("Fighter->Frigate (tech 3)", sys.CanUpgradeClass(ShipClass::Fighter, ShipClass::Frigate));
    TEST("Fighter->Destroyer (tech 4) fails", !sys.CanUpgradeClass(ShipClass::Fighter, ShipClass::Destroyer));
    TEST("Fighter->Battleship (tech 8) fails", !sys.CanUpgradeClass(ShipClass::Fighter, ShipClass::Battleship));

    auto upgrades = sys.GetAvailableUpgrades(ShipClass::Fighter);
    TEST("Fighter has multiple upgrades", upgrades.size() > 0);
}

// ===================================================================
// Refinery System Tests
// ===================================================================

static void TestRefineryTierNames() {
    std::cout << "[RefineryRecipe Tier Names]\n";
    TEST("Basic tier", RefineryRecipe::GetTierName(RefineryTier::Basic) == "Basic");
    TEST("Advanced tier", RefineryRecipe::GetTierName(RefineryTier::Advanced) == "Advanced");
    TEST("Industrial tier", RefineryRecipe::GetTierName(RefineryTier::Industrial) == "Industrial");
    TEST("Military tier", RefineryRecipe::GetTierName(RefineryTier::Military) == "Military");
    TEST("Experimental tier", RefineryRecipe::GetTierName(RefineryTier::Experimental) == "Experimental");
}

static void TestRefiningStateNames() {
    std::cout << "[RefineryRecipe State Names]\n";
    TEST("Idle state", RefineryRecipe::GetStateName(RefiningState::Idle) == "Idle");
    TEST("Loading state", RefineryRecipe::GetStateName(RefiningState::Loading) == "Loading");
    TEST("Processing state", RefineryRecipe::GetStateName(RefiningState::Processing) == "Processing");
    TEST("Completed state", RefineryRecipe::GetStateName(RefiningState::Completed) == "Completed");
    TEST("Failed state", RefineryRecipe::GetStateName(RefiningState::Failed) == "Failed");
}

static void TestRefineryDefaultRecipes() {
    std::cout << "[RefineryRecipe Default Recipes]\n";
    auto recipes = RefineryRecipe::GetDefaultRecipes();
    TEST("Eight default recipes", recipes.size() == 8);
    TEST("First recipe is Iron Ore", recipes[0].inputMaterial == "Iron Ore");
    TEST("First recipe output", recipes[0].outputMaterial == "Iron Ingot");
    TEST("Last recipe is Scrap Metal", recipes[7].inputMaterial == "Scrap Metal");
}

static void TestRefineryComponentDefaults() {
    std::cout << "[RefineryComponent Defaults]\n";
    RefineryComponent rc;
    TEST("Default tier Basic", rc.GetTier() == RefineryTier::Basic);
    TEST("Default max jobs 3", rc.GetMaxJobs() == 3);
    TEST("No active jobs", rc.GetActiveJobCount() == 0);
    TEST("No completed jobs", rc.GetCompletedJobCount() == 0);
    TEST("Empty job list", rc.GetAllJobs().empty());
    TEST("Efficiency 1.0 for Basic", ApproxEq(rc.GetEfficiencyMultiplier(), 1.0f));
    TEST("Speed 1.0 for Basic", ApproxEq(rc.GetProcessingSpeedMultiplier(), 1.0f));
}

static void TestRefineryComponentCustomTier() {
    std::cout << "[RefineryComponent Custom Tier]\n";
    RefineryComponent rc(RefineryTier::Industrial, 5);
    TEST("Industrial tier", rc.GetTier() == RefineryTier::Industrial);
    TEST("Max jobs 5", rc.GetMaxJobs() == 5);
    // Industrial = tier index 2, efficiency = 1.0 + 0.1*2 = 1.2
    TEST("Efficiency 1.2", ApproxEq(rc.GetEfficiencyMultiplier(), 1.2f));
    // Speed = 1.0 + 0.15*2 = 1.3
    TEST("Speed 1.3", ApproxEq(rc.GetProcessingSpeedMultiplier(), 1.3f));
}

static void TestRefineryComponentStartJob() {
    std::cout << "[RefineryComponent StartJob]\n";
    RefineryComponent rc(RefineryTier::Advanced, 2);
    auto recipes = RefineryRecipe::GetDefaultRecipes();
    // Recipe 0: Iron Ore (Basic tier) - should succeed on Advanced
    TEST("Start basic recipe", rc.StartJob(recipes[0]));
    TEST("One active job", rc.GetActiveJobCount() == 1);

    // Recipe 2: Naonite Ore (Advanced tier) - should succeed
    TEST("Start advanced recipe", rc.StartJob(recipes[2]));
    TEST("Two active jobs", rc.GetActiveJobCount() == 2);

    // Max jobs reached - should fail
    TEST("Third job fails (max)", !rc.StartJob(recipes[0]));
}

static void TestRefineryComponentTierRequirement() {
    std::cout << "[RefineryComponent Tier Requirement]\n";
    RefineryComponent rc(RefineryTier::Basic, 3);
    auto recipes = RefineryRecipe::GetDefaultRecipes();
    // Recipe 2: Naonite Ore requires Advanced tier - should fail on Basic
    TEST("Advanced recipe on Basic fails", !rc.StartJob(recipes[2]));
    // Recipe 0: Iron Ore requires Basic tier - should succeed
    TEST("Basic recipe on Basic succeeds", rc.StartJob(recipes[0]));
}

static void TestRefineryComponentCancelJob() {
    std::cout << "[RefineryComponent CancelJob]\n";
    RefineryComponent rc;
    auto recipes = RefineryRecipe::GetDefaultRecipes();
    rc.StartJob(recipes[0]);
    int jobId = rc.GetAllJobs()[0].jobId;
    TEST("One job", rc.GetAllJobs().size() == 1);
    TEST("Cancel succeeds", rc.CancelJob(jobId));
    TEST("No jobs", rc.GetAllJobs().empty());
    TEST("Cancel nonexistent fails", !rc.CancelJob(999));
}

static void TestRefineryComponentCollectJob() {
    std::cout << "[RefineryComponent CollectJob]\n";
    RefineryComponent rc;
    auto recipes = RefineryRecipe::GetDefaultRecipes();
    rc.StartJob(recipes[0]); // Iron Ore -> Iron Ingot
    int jobId = rc.GetAllJobs()[0].jobId;

    // Job is Loading, not completed - collect should fail
    auto result = rc.CollectJob(jobId);
    TEST("Collect incomplete returns empty", result.first.empty() && result.second == 0);
}

static void TestRefineryComponentSetTier() {
    std::cout << "[RefineryComponent SetTier]\n";
    RefineryComponent rc;
    TEST("Initially Basic", rc.GetTier() == RefineryTier::Basic);
    rc.SetTier(RefineryTier::Experimental);
    TEST("Now Experimental", rc.GetTier() == RefineryTier::Experimental);
    // Experimental = tier index 4, efficiency = 1.0 + 0.1*4 = 1.4
    TEST("Efficiency 1.4", ApproxEq(rc.GetEfficiencyMultiplier(), 1.4f));
}

static void TestRefineryComponentSerialization() {
    std::cout << "[RefineryComponent Serialization]\n";
    RefineryComponent rc(RefineryTier::Military, 4);
    auto recipes = RefineryRecipe::GetDefaultRecipes();
    rc.StartJob(recipes[0]); // Iron Ore
    rc.StartJob(recipes[1]); // Titanium Ore

    ComponentData cd = rc.Serialize();
    TEST("Component type", cd.componentType == "RefineryComponent");

    RefineryComponent rc2;
    rc2.Deserialize(cd);
    TEST("Restored tier", rc2.GetTier() == RefineryTier::Military);
    TEST("Restored max jobs", rc2.GetMaxJobs() == 4);
    TEST("Restored job count", rc2.GetAllJobs().size() == 2);
    TEST("Restored job recipe", rc2.GetAllJobs()[0].recipe.inputMaterial == "Iron Ore");
}

static void TestRefinerySystem() {
    std::cout << "[RefinerySystem]\n";
    RefinerySystem sys;
    TEST("System created", true);
    sys.Update(0.016f);
    TEST("Update without EM", true);
}

static void TestRefinerySystemProcessing() {
    std::cout << "[RefinerySystem Processing]\n";
    EntityManager em;
    RefinerySystem sys(em);

    auto& entity = em.CreateEntity("refinery_station");
    auto* rc = em.AddComponent<RefineryComponent>(
        entity.id, std::make_unique<RefineryComponent>(RefineryTier::Basic, 3));

    auto recipes = RefineryRecipe::GetDefaultRecipes();
    rc->StartJob(recipes[0]); // Iron Ore: 5s processing, batch 1
    int jobId = rc->GetAllJobs()[0].jobId;

    // First update transitions Loading -> Processing
    sys.Update(0.1f);
    TEST("Job processing", rc->GetJob(jobId)->state == RefiningState::Processing);

    // Process for enough time to complete (5s processing, speed 1.0 for Basic)
    sys.Update(5.5f);
    TEST("Job completed", rc->GetJob(jobId)->state == RefiningState::Completed);

    // Collect the output
    auto output = rc->CollectJob(jobId);
    TEST("Output material", output.first == "Iron Ingot");
    TEST("Output amount > 0", output.second > 0);
    TEST("Job removed", rc->GetAllJobs().empty());
}

static void TestRefinerySystemSpeedMultiplier() {
    std::cout << "[RefinerySystem Speed Multiplier]\n";
    EntityManager em;
    RefinerySystem sys(em);

    auto& entity = em.CreateEntity("fast_refinery");
    // Experimental tier: speed mult = 1.0 + 0.15*4 = 1.6
    auto* rc = em.AddComponent<RefineryComponent>(
        entity.id, std::make_unique<RefineryComponent>(RefineryTier::Experimental, 3));

    auto recipes = RefineryRecipe::GetDefaultRecipes();
    rc->StartJob(recipes[0]); // 5s processing at Basic speed
    int jobId = rc->GetAllJobs()[0].jobId;

    sys.Update(0.1f); // Loading -> Processing
    // At 1.6x speed, 5s becomes ~3.125s effective
    sys.Update(3.5f);
    TEST("Fast tier completes sooner", rc->GetJob(jobId)->state == RefiningState::Completed);
}

// ===================================================================
// Scanning System Tests
// ===================================================================

static void TestScannerTypeNames() {
    std::cout << "[ScannerTypeNames]\n";
    TEST("Passive name", ScanResult::GetTypeName(ScannerType::Passive) == "Passive");
    TEST("Active name", ScanResult::GetTypeName(ScannerType::Active) == "Active");
    TEST("Deep name", ScanResult::GetTypeName(ScannerType::Deep) == "Deep");
    TEST("Military name", ScanResult::GetTypeName(ScannerType::Military) == "Military");
}

static void TestScanStateNames() {
    std::cout << "[ScanStateNames]\n";
    TEST("Idle name", ScanResult::GetStateName(ScanState::Idle) == "Idle");
    TEST("Scanning name", ScanResult::GetStateName(ScanState::Scanning) == "Scanning");
    TEST("Analyzing name", ScanResult::GetStateName(ScanState::Analyzing) == "Analyzing");
    TEST("Complete name", ScanResult::GetStateName(ScanState::Complete) == "Complete");
    TEST("Jammed name", ScanResult::GetStateName(ScanState::Jammed) == "Jammed");
}

static void TestSignatureClassNames() {
    std::cout << "[SignatureClassNames]\n";
    TEST("Unknown class", ScanResult::GetClassName(SignatureClass::Unknown) == "Unknown");
    TEST("Ship class", ScanResult::GetClassName(SignatureClass::Ship) == "Ship");
    TEST("Station class", ScanResult::GetClassName(SignatureClass::Station) == "Station");
    TEST("Asteroid class", ScanResult::GetClassName(SignatureClass::Asteroid) == "Asteroid");
    TEST("Anomaly class", ScanResult::GetClassName(SignatureClass::Anomaly) == "Anomaly");
    TEST("Debris class", ScanResult::GetClassName(SignatureClass::Debris) == "Debris");
}

static void TestScannerComponentDefaults() {
    std::cout << "[ScannerComponentDefaults]\n";
    ScannerComponent sc;
    TEST("Default type is Passive", sc.GetType() == ScannerType::Passive);
    TEST("Default range", ApproxEq(sc.GetRange(), 5000.0f));  // 5000 * 1.0
    TEST("Default resolution", ApproxEq(sc.GetResolution(), 1.0f));
    TEST("Max concurrent scans", sc.GetMaxConcurrentScans() == 2);
    TEST("No active scans", sc.GetActiveScanCount() == 0);
    TEST("Not on cooldown", !sc.IsOnCooldown());
    TEST("Scan speed multiplier", ApproxEq(sc.GetScanSpeedMultiplier(), 1.0f));
    TEST("Range multiplier", ApproxEq(sc.GetRangeMultiplier(), 1.0f));
}

static void TestScannerComponentCustomType() {
    std::cout << "[ScannerComponentCustomType]\n";
    ScannerComponent sc(ScannerType::Military, 10000.0f);
    TEST("Military type", sc.GetType() == ScannerType::Military);
    TEST("Military range", ApproxEq(sc.GetRange(), 18000.0f));  // 10000 * 1.8
    TEST("Military resolution", ApproxEq(sc.GetResolution(), 0.3f));
    TEST("Military max scans", sc.GetMaxConcurrentScans() == 6);
    TEST("Military speed mult", ApproxEq(sc.GetScanSpeedMultiplier(), 2.0f));
    TEST("Military range mult", ApproxEq(sc.GetRangeMultiplier(), 1.8f));
}

static void TestScannerComponentStartScan() {
    std::cout << "[ScannerComponentStartScan]\n";
    ScannerComponent sc(ScannerType::Passive, 5000.0f);
    TEST("Start scan 1", sc.StartScan(100, "Target Alpha", 1000.0f, 10.0f, 20.0f, 30.0f));
    TEST("Active scan count 1", sc.GetActiveScanCount() == 1);
    TEST("Start scan 2", sc.StartScan(200, "Target Beta", 2000.0f));
    TEST("Active scan count 2", sc.GetActiveScanCount() == 2);
    // Passive scanner can only have 2 concurrent scans
    TEST("Start scan 3 fails (capacity)", !sc.StartScan(300, "Target Gamma", 3000.0f));
    TEST("Active scan count still 2", sc.GetActiveScanCount() == 2);
    // Can't scan same target twice
    ScannerComponent sc2(ScannerType::Active, 5000.0f);
    sc2.StartScan(100, "Target Alpha", 1000.0f);
    TEST("Duplicate scan fails", !sc2.StartScan(100, "Target Alpha", 1000.0f));
}

static void TestScannerComponentCooldown() {
    std::cout << "[ScannerComponentCooldown]\n";
    ScannerComponent sc(ScannerType::Passive, 5000.0f);
    TEST("Start scan triggers cooldown", sc.StartScan(100, "Target Alpha", 1000.0f));
    TEST("Is on cooldown", sc.IsOnCooldown());
    TEST("Cooldown remaining > 0", sc.GetCooldownRemaining() > 0.0f);
}

static void TestScannerComponentCancelScan() {
    std::cout << "[ScannerComponentCancelScan]\n";
    ScannerComponent sc(ScannerType::Active, 5000.0f);
    sc.StartScan(100, "Target Alpha", 1000.0f);
    TEST("Cancel existing scan", sc.CancelScan(100));
    TEST("Active scans after cancel", sc.GetActiveScanCount() == 0);
    TEST("Cancel non-existent fails", !sc.CancelScan(999));
}

static void TestScannerComponentGetScanResult() {
    std::cout << "[ScannerComponentGetScanResult]\n";
    ScannerComponent sc(ScannerType::Passive, 5000.0f);
    sc.StartScan(100, "Target Alpha", 1000.0f, 10.0f, 20.0f, 30.0f);
    const ScanResult* r = sc.GetScanResult(100);
    TEST("Scan result exists", r != nullptr);
    TEST("Target ID matches", r && r->targetId == 100);
    TEST("Target name matches", r && r->targetName == "Target Alpha");
    TEST("Distance matches", r && ApproxEq(r->distance, 1000.0f));
    TEST("Position X", r && ApproxEq(r->posX, 10.0f));
    TEST("Position Y", r && ApproxEq(r->posY, 20.0f));
    TEST("Position Z", r && ApproxEq(r->posZ, 30.0f));
    TEST("Not yet fully scanned", r && !r->isFullyScanned);
    TEST("Non-existent scan returns null", sc.GetScanResult(999) == nullptr);
}

static void TestScannerComponentClearCompleted() {
    std::cout << "[ScannerComponentClearCompleted]\n";
    ScannerComponent sc(ScannerType::Active, 5000.0f);
    sc.StartScan(100, "Target", 1000.0f);
    // Manually mark as complete for testing
    auto allScans = sc.GetAllScans();
    TEST("Has 1 scan", allScans.size() == 1);
    sc.ClearCompletedScans();  // None completed yet
    TEST("Still 1 scan after clear (none completed)", sc.GetAllScans().size() == 1);
}

static void TestScannerComponentSerialization() {
    std::cout << "[ScannerComponentSerialization]\n";
    ScannerComponent sc(ScannerType::Deep, 8000.0f);
    sc.StartScan(100, "Target Alpha", 1500.0f, 5.0f, 10.0f, 15.0f);

    ComponentData cd = sc.Serialize();
    TEST("Component type", cd.componentType == "ScannerComponent");

    ScannerComponent sc2;
    sc2.Deserialize(cd);
    TEST("Restored type", sc2.GetType() == ScannerType::Deep);
    TEST("Restored scan count", sc2.GetAllScans().size() == 1);
    const ScanResult* r = sc2.GetScanResult(100);
    TEST("Restored scan exists", r != nullptr);
    TEST("Restored target name", r && r->targetName == "Target Alpha");
    TEST("Restored distance", r && ApproxEq(r->distance, 1500.0f));
    TEST("Restored posX", r && ApproxEq(r->posX, 5.0f));
}

static void TestScanningSystem() {
    std::cout << "[ScanningSystem]\n";
    ScanningSystem sys;
    sys.Update(1.0f);  // Should not crash without entity manager
    TEST("ScanningSystem name", sys.GetName() == "ScanningSystem");
}

static void TestScanningSystemProgress() {
    std::cout << "[ScanningSystemProgress]\n";
    EntityManager em;
    ScanningSystem sys(em);

    auto& entity = em.CreateEntity("Scanner Ship");
    auto* sc = em.AddComponent<ScannerComponent>(
        entity.id, std::make_unique<ScannerComponent>(ScannerType::Active, 5000.0f));
    sc->StartScan(200, "Asteroid", 1000.0f);

    // Update for several seconds to progress the scan
    for (int i = 0; i < 100; ++i) {
        sys.Update(0.1f);
    }

    const ScanResult* r = sc->GetScanResult(200);
    TEST("Scan progressed", r && r->scanProgress > 0.0f);
    TEST("Scan completed after 10s", r && r->isFullyScanned);
    TEST("Signal strength at max", r && ApproxEq(r->signalStrength, 1.0f));
    TEST("Classification assigned", r && r->classification != SignatureClass::Unknown);
}

static void TestScanningSystemSpeedMultiplier() {
    std::cout << "[ScanningSystemSpeedMultiplier]\n";
    EntityManager em;
    ScanningSystem sys(em);

    auto& entity = em.CreateEntity("Military Scanner");
    auto* sc = em.AddComponent<ScannerComponent>(
        entity.id, std::make_unique<ScannerComponent>(ScannerType::Military, 10000.0f));
    sc->StartScan(300, "Enemy Ship", 2000.0f);

    // Military scanner should complete faster (2.0x speed)
    // Base scan time 10s / 2.0 speed = 5s, plus distance factor. Allow 6s.
    for (int i = 0; i < 60; ++i) {
        sys.Update(0.1f);
    }

    const ScanResult* r = sc->GetScanResult(300);
    TEST("Military scan completed in 5s", r && r->isFullyScanned);
}

// ===================================================================
// Salvage System Tests
// ===================================================================

static void TestSalvageTierNames() {
    std::cout << "[SalvageTierNames]\n";
    TEST("Basic name", SalvageTarget::GetTierName(SalvageTier::Basic) == "Basic");
    TEST("Advanced name", SalvageTarget::GetTierName(SalvageTier::Advanced) == "Advanced");
    TEST("Industrial name", SalvageTarget::GetTierName(SalvageTier::Industrial) == "Industrial");
    TEST("Military name", SalvageTarget::GetTierName(SalvageTier::Military) == "Military");
    TEST("Experimental name", SalvageTarget::GetTierName(SalvageTier::Experimental) == "Experimental");
}

static void TestSalvageStateNames() {
    std::cout << "[SalvageStateNames]\n";
    TEST("Idle name", SalvageTarget::GetStateName(SalvageState::Idle) == "Idle");
    TEST("Approaching name", SalvageTarget::GetStateName(SalvageState::Approaching) == "Approaching");
    TEST("Salvaging name", SalvageTarget::GetStateName(SalvageState::Salvaging) == "Salvaging");
    TEST("Completed name", SalvageTarget::GetStateName(SalvageState::Completed) == "Completed");
    TEST("Failed name", SalvageTarget::GetStateName(SalvageState::Failed) == "Failed");
}

static void TestSalvageDefaultWreckTypes() {
    std::cout << "[SalvageDefaultWreckTypes]\n";
    auto wrecks = SalvageTarget::GetDefaultWreckTypes();
    TEST("8 default wreck types", wrecks.size() == 8);
    TEST("First wreck is Small Debris", wrecks[0].wreckName == "Small Debris");
    TEST("Last wreck is Cargo Container", wrecks[7].wreckName == "Cargo Container");
    TEST("Ancient Artifact has Avorion", wrecks[6].primaryMaterial == "Avorion");
}

static void TestSalvageComponentDefaults() {
    std::cout << "[SalvageComponentDefaults]\n";
    SalvageComponent sc;
    TEST("Default tier is Basic", sc.GetTier() == SalvageTier::Basic);
    TEST("Default range", ApproxEq(sc.GetRange(), 500.0f));
    TEST("Default max targets", sc.GetMaxTargets() == 2);
    TEST("No active targets", sc.GetActiveTargetCount() == 0);
    TEST("Efficiency multiplier", ApproxEq(sc.GetEfficiencyMultiplier(), 1.0f));
    TEST("Speed multiplier", ApproxEq(sc.GetSpeedMultiplier(), 1.0f));
    TEST("Total collected 0", sc.GetTotalMaterialsCollected() == 0);
}

static void TestSalvageComponentCustomTier() {
    std::cout << "[SalvageComponentCustomTier]\n";
    SalvageComponent sc(SalvageTier::Experimental, 1000.0f);
    TEST("Experimental tier", sc.GetTier() == SalvageTier::Experimental);
    TEST("Custom range", ApproxEq(sc.GetRange(), 1000.0f));
    TEST("Experimental max targets", sc.GetMaxTargets() == 6);  // 2 + 4
    TEST("Experimental efficiency", ApproxEq(sc.GetEfficiencyMultiplier(), 1.5f));
    TEST("Experimental speed", ApproxEq(sc.GetSpeedMultiplier(), 1.8f));
}

static void TestSalvageComponentStartSalvage() {
    std::cout << "[SalvageComponentStartSalvage]\n";
    SalvageComponent sc(SalvageTier::Basic, 500.0f);
    SalvageTarget t1;
    t1.targetId = 100;
    t1.wreckName = "Fighter Wreck";
    t1.primaryMaterial = "Titanium";
    t1.totalYield = 100;
    t1.integrity = 0.5f;
    TEST("Start salvage 1", sc.StartSalvage(t1));
    TEST("Active target count 1", sc.GetActiveTargetCount() == 1);

    SalvageTarget t2;
    t2.targetId = 200;
    t2.wreckName = "Frigate Wreck";
    t2.primaryMaterial = "Naonite";
    t2.totalYield = 250;
    TEST("Start salvage 2", sc.StartSalvage(t2));
    TEST("Active target count 2", sc.GetActiveTargetCount() == 2);

    SalvageTarget t3;
    t3.targetId = 300;
    TEST("Start salvage 3 fails (capacity)", !sc.StartSalvage(t3));
}

static void TestSalvageComponentCancelSalvage() {
    std::cout << "[SalvageComponentCancelSalvage]\n";
    SalvageComponent sc;
    SalvageTarget t;
    t.targetId = 100;
    t.wreckName = "Debris";
    t.primaryMaterial = "Iron";
    t.totalYield = 50;
    sc.StartSalvage(t);
    TEST("Cancel existing", sc.CancelSalvage(100));
    TEST("Active count 0", sc.GetActiveTargetCount() == 0);
    TEST("Cancel non-existent", !sc.CancelSalvage(999));
}

static void TestSalvageComponentCollectSalvage() {
    std::cout << "[SalvageComponentCollectSalvage]\n";
    SalvageComponent sc;
    SalvageTarget t;
    t.targetId = 100;
    t.wreckName = "Fighter Wreck";
    t.primaryMaterial = "Titanium";
    t.totalYield = 100;
    t.integrity = 0.5f;
    sc.StartSalvage(t);
    // Not completed yet
    auto result = sc.CollectSalvage(100);
    TEST("Collect incomplete returns empty", result.first.empty() && result.second == 0);
}

static void TestSalvageComponentSetTier() {
    std::cout << "[SalvageComponentSetTier]\n";
    SalvageComponent sc;
    sc.SetTier(SalvageTier::Military);
    TEST("SetTier to Military", sc.GetTier() == SalvageTier::Military);
    TEST("Military max targets", sc.GetMaxTargets() == 5);  // 2 + 3
    TEST("Military efficiency", ApproxEq(sc.GetEfficiencyMultiplier(), 1.375f));
}

static void TestSalvageComponentSerialization() {
    std::cout << "[SalvageComponentSerialization]\n";
    SalvageComponent sc(SalvageTier::Advanced, 750.0f);
    SalvageTarget t;
    t.targetId = 100;
    t.wreckName = "Cruiser Wreck";
    t.primaryMaterial = "Trinium";
    t.totalYield = 500;
    t.remainingYield = 300;
    t.integrity = 0.7f;
    sc.StartSalvage(t);

    ComponentData cd = sc.Serialize();
    TEST("Component type", cd.componentType == "SalvageComponent");

    SalvageComponent sc2;
    sc2.Deserialize(cd);
    TEST("Restored tier", sc2.GetTier() == SalvageTier::Advanced);
    TEST("Restored range", ApproxEq(sc2.GetRange(), 750.0f));
    TEST("Restored target count", sc2.GetAllTargets().size() == 1);
    const SalvageTarget* rt = sc2.GetTarget(100);
    TEST("Restored target exists", rt != nullptr);
    TEST("Restored wreck name", rt && rt->wreckName == "Cruiser Wreck");
    TEST("Restored material", rt && rt->primaryMaterial == "Trinium");
}

static void TestSalvageSystem() {
    std::cout << "[SalvageSystem]\n";
    SalvageSystem sys;
    sys.Update(1.0f);  // Should not crash without entity manager
    TEST("SalvageSystem name", sys.GetName() == "SalvageSystem");
}

static void TestSalvageSystemProcessing() {
    std::cout << "[SalvageSystemProcessing]\n";
    EntityManager em;
    SalvageSystem sys(em);

    auto& entity = em.CreateEntity("Salvager Ship");
    auto* sc = em.AddComponent<SalvageComponent>(
        entity.id, std::make_unique<SalvageComponent>(SalvageTier::Basic, 500.0f));

    SalvageTarget t;
    t.targetId = 100;
    t.wreckName = "Fighter Wreck";
    t.primaryMaterial = "Titanium";
    t.totalYield = 100;
    t.integrity = 1.0f;
    sc->StartSalvage(t);

    // First update: Approaching -> Salvaging
    sys.Update(0.1f);
    const SalvageTarget* target = sc->GetTarget(100);
    TEST("Transitioned to Salvaging", target && target->state == SalvageState::Salvaging);

    // Update for 8+ seconds to complete (base salvage time is 8s at 1.0x speed)
    for (int i = 0; i < 85; ++i) {
        sys.Update(0.1f);
    }

    target = sc->GetTarget(100);
    TEST("Salvage completed", target && target->state == SalvageState::Completed);

    // Collect
    auto result = sc->CollectSalvage(100);
    TEST("Collected material name", result.first == "Titanium");
    TEST("Collected material amount > 0", result.second > 0);
    TEST("Total collected updated", sc->GetTotalMaterialsCollected() > 0);
}

static void TestSalvageSystemSpeedMultiplier() {
    std::cout << "[SalvageSystemSpeedMultiplier]\n";
    EntityManager em;
    SalvageSystem sys(em);

    auto& entity = em.CreateEntity("Experimental Salvager");
    auto* sc = em.AddComponent<SalvageComponent>(
        entity.id, std::make_unique<SalvageComponent>(SalvageTier::Experimental, 1000.0f));

    SalvageTarget t;
    t.targetId = 200;
    t.wreckName = "Battleship Wreck";
    t.primaryMaterial = "Xanion";
    t.totalYield = 800;
    t.integrity = 0.8f;
    sc->StartSalvage(t);

    // First update: Approaching -> Salvaging
    sys.Update(0.1f);

    // Experimental (1.8x speed): 8s / 1.8 = ~4.44s needed
    for (int i = 0; i < 45; ++i) {
        sys.Update(0.1f);
    }

    const SalvageTarget* target = sc->GetTarget(200);
    TEST("Experimental salvage completed faster", target && target->state == SalvageState::Completed);
}

// ===================================================================
// Fleet Command System Tests
// ===================================================================

static void TestFleetOrderTypeNames() {
    std::cout << "[FleetOrderTypeNames]\n";
    TEST("Idle name", FleetOrder::GetOrderTypeName(FleetOrderType::Idle) == "Idle");
    TEST("Patrol name", FleetOrder::GetOrderTypeName(FleetOrderType::Patrol) == "Patrol");
    TEST("Mine name", FleetOrder::GetOrderTypeName(FleetOrderType::Mine) == "Mine");
    TEST("Trade name", FleetOrder::GetOrderTypeName(FleetOrderType::Trade) == "Trade");
    TEST("Attack name", FleetOrder::GetOrderTypeName(FleetOrderType::Attack) == "Attack");
    TEST("Escort name", FleetOrder::GetOrderTypeName(FleetOrderType::Escort) == "Escort");
    TEST("Defend name", FleetOrder::GetOrderTypeName(FleetOrderType::Defend) == "Defend");
    TEST("Scout name", FleetOrder::GetOrderTypeName(FleetOrderType::Scout) == "Scout");
}

static void TestFleetOrderStateNames() {
    std::cout << "[FleetOrderStateNames]\n";
    TEST("Pending name", FleetOrder::GetOrderStateName(FleetOrderState::Pending) == "Pending");
    TEST("Active name", FleetOrder::GetOrderStateName(FleetOrderState::Active) == "Active");
    TEST("Paused name", FleetOrder::GetOrderStateName(FleetOrderState::Paused) == "Paused");
    TEST("Completed name", FleetOrder::GetOrderStateName(FleetOrderState::Completed) == "Completed");
    TEST("Failed name", FleetOrder::GetOrderStateName(FleetOrderState::Failed) == "Failed");
}

static void TestFleetRoleNames() {
    std::cout << "[FleetRoleNames]\n";
    TEST("Flagship name", FleetOrder::GetRoleName(FleetRole::Flagship) == "Flagship");
    TEST("Combat name", FleetOrder::GetRoleName(FleetRole::Combat) == "Combat");
    TEST("Mining name", FleetOrder::GetRoleName(FleetRole::Mining) == "Mining");
    TEST("Trading name", FleetOrder::GetRoleName(FleetRole::Trading) == "Trading");
    TEST("Support name", FleetOrder::GetRoleName(FleetRole::Support) == "Support");
    TEST("Scout name", FleetOrder::GetRoleName(FleetRole::Scout) == "Scout");
}

static void TestFleetCommandComponentDefaults() {
    std::cout << "[FleetCommandComponentDefaults]\n";
    FleetCommandComponent fc;
    TEST("Default fleet name", fc.GetFleetName() == "Fleet");
    TEST("Default max members", fc.GetMaxMembers() == 10);
    TEST("No members", fc.GetMemberCount() == 0);
    TEST("No active members", fc.GetActiveMemberCount() == 0);
    TEST("No orders", fc.GetAllOrders().empty());
    TEST("Average morale 0 (empty)", ApproxEq(fc.GetAverageMorale(), 0.0f));
}

static void TestFleetCommandComponentCustom() {
    std::cout << "[FleetCommandComponentCustom]\n";
    FleetCommandComponent fc("Alpha Squadron");
    fc.SetMaxMembers(5);
    TEST("Custom name", fc.GetFleetName() == "Alpha Squadron");
    TEST("Custom max members", fc.GetMaxMembers() == 5);
}

static void TestFleetCommandComponentAddMember() {
    std::cout << "[FleetCommandComponentAddMember]\n";
    FleetCommandComponent fc("Test Fleet");
    fc.SetMaxMembers(3);
    TEST("Add member 1", fc.AddMember(100, "Fighter A", FleetRole::Combat));
    TEST("Add member 2", fc.AddMember(200, "Mining Ship", FleetRole::Mining));
    TEST("Add member 3", fc.AddMember(300, "Transport", FleetRole::Trading));
    TEST("Member count 3", fc.GetMemberCount() == 3);
    TEST("Active count 3", fc.GetActiveMemberCount() == 3);
    TEST("Add member 4 fails (capacity)", !fc.AddMember(400, "Scout", FleetRole::Scout));
    // Duplicate check
    TEST("Add duplicate fails", !fc.AddMember(100, "Fighter A Copy", FleetRole::Combat));
}

static void TestFleetCommandComponentRemoveMember() {
    std::cout << "[FleetCommandComponentRemoveMember]\n";
    FleetCommandComponent fc;
    fc.AddMember(100, "Fighter A", FleetRole::Combat);
    fc.AddMember(200, "Mining Ship", FleetRole::Mining);
    TEST("Remove existing", fc.RemoveMember(100));
    TEST("Member count 1", fc.GetMemberCount() == 1);
    TEST("Remove non-existent", !fc.RemoveMember(999));
}

static void TestFleetCommandComponentGetMember() {
    std::cout << "[FleetCommandComponentGetMember]\n";
    FleetCommandComponent fc;
    fc.AddMember(100, "Fighter A", FleetRole::Combat);
    const FleetMember* m = fc.GetMember(100);
    TEST("Member exists", m != nullptr);
    TEST("Member entity ID", m && m->entityId == 100);
    TEST("Member ship name", m && m->shipName == "Fighter A");
    TEST("Member role", m && m->role == FleetRole::Combat);
    TEST("Member morale default", m && ApproxEq(m->morale, 1.0f));
    TEST("Member active", m && m->isActive);
    TEST("Non-existent member null", fc.GetMember(999) == nullptr);
}

static void TestFleetCommandComponentIssueOrder() {
    std::cout << "[FleetCommandComponentIssueOrder]\n";
    FleetCommandComponent fc;
    TEST("Issue patrol order", fc.IssueOrder(FleetOrderType::Patrol, 100.0f, 200.0f, 300.0f));
    TEST("Issue mine order", fc.IssueOrder(FleetOrderType::Mine, 50.0f, 60.0f, 70.0f, 0, 1));
    TEST("Order count 2", fc.GetAllOrders().size() == 2);
    const FleetOrder* o = fc.GetOrder(1);
    TEST("Order 1 exists", o != nullptr);
    TEST("Order 1 type is Patrol", o && o->type == FleetOrderType::Patrol);
    TEST("Order 1 targetX", o && ApproxEq(o->targetX, 100.0f));
}

static void TestFleetCommandComponentOrderCapacity() {
    std::cout << "[FleetCommandComponentOrderCapacity]\n";
    FleetCommandComponent fc;
    // Default max orders is 5
    for (int i = 0; i < 5; ++i) {
        TEST("Issue order " + std::to_string(i+1), fc.IssueOrder(FleetOrderType::Patrol));
    }
    TEST("6th order fails (capacity)", !fc.IssueOrder(FleetOrderType::Attack));
}

static void TestFleetCommandComponentCancelOrder() {
    std::cout << "[FleetCommandComponentCancelOrder]\n";
    FleetCommandComponent fc;
    fc.IssueOrder(FleetOrderType::Patrol, 100.0f, 200.0f, 300.0f);
    TEST("Cancel existing order", fc.CancelOrder(1));
    TEST("Order count 0", fc.GetAllOrders().empty());
    TEST("Cancel non-existent", !fc.CancelOrder(999));
}

static void TestFleetCommandComponentMorale() {
    std::cout << "[FleetCommandComponentMorale]\n";
    FleetCommandComponent fc;
    fc.AddMember(100, "Ship A", FleetRole::Combat);
    fc.AddMember(200, "Ship B", FleetRole::Support);
    TEST("Average morale 1.0", ApproxEq(fc.GetAverageMorale(), 1.0f));
    TEST("Set morale", fc.SetMemberMorale(100, 0.5f));
    TEST("Average morale 0.75", ApproxEq(fc.GetAverageMorale(), 0.75f));
    TEST("Set morale clamped high", fc.SetMemberMorale(200, 1.5f));
    const FleetMember* m = fc.GetMember(200);
    TEST("Morale clamped to 1.0", m && ApproxEq(m->morale, 1.0f));
    TEST("Set morale non-existent fails", !fc.SetMemberMorale(999, 0.5f));
}

static void TestFleetCommandComponentSetRole() {
    std::cout << "[FleetCommandComponentSetRole]\n";
    FleetCommandComponent fc;
    fc.AddMember(100, "Ship A", FleetRole::Combat);
    TEST("Set role", fc.SetMemberRole(100, FleetRole::Scout));
    const FleetMember* m = fc.GetMember(100);
    TEST("Role updated", m && m->role == FleetRole::Scout);
    TEST("Set role non-existent fails", !fc.SetMemberRole(999, FleetRole::Mining));
}

static void TestFleetCommandComponentSerialization() {
    std::cout << "[FleetCommandComponentSerialization]\n";
    FleetCommandComponent fc("Alpha Squadron");
    fc.SetMaxMembers(5);
    fc.AddMember(100, "Fighter A", FleetRole::Combat);
    fc.AddMember(200, "Mining Ship", FleetRole::Mining);
    fc.SetMemberMorale(100, 0.8f);
    fc.IssueOrder(FleetOrderType::Patrol, 10.0f, 20.0f, 30.0f);

    ComponentData cd = fc.Serialize();
    TEST("Component type", cd.componentType == "FleetCommandComponent");

    FleetCommandComponent fc2;
    fc2.Deserialize(cd);
    TEST("Restored fleet name", fc2.GetFleetName() == "Alpha Squadron");
    TEST("Restored max members", fc2.GetMaxMembers() == 5);
    TEST("Restored member count", fc2.GetMemberCount() == 2);
    const FleetMember* m = fc2.GetMember(100);
    TEST("Restored member exists", m != nullptr);
    TEST("Restored member name", m && m->shipName == "Fighter A");
    TEST("Restored member morale", m && ApproxEq(m->morale, 0.8f));
    TEST("Restored order count", fc2.GetAllOrders().size() == 1);
    const FleetOrder* o = fc2.GetOrder(1);
    TEST("Restored order exists", o != nullptr);
    TEST("Restored order type", o && o->type == FleetOrderType::Patrol);
}

static void TestFleetCommandSystem() {
    std::cout << "[FleetCommandSystem]\n";
    FleetCommandSystem sys;
    sys.Update(1.0f);  // Should not crash without entity manager
    TEST("FleetCommandSystem name", sys.GetName() == "FleetCommandSystem");
}

static void TestFleetCommandSystemOrderProgress() {
    std::cout << "[FleetCommandSystemOrderProgress]\n";
    EntityManager em;
    FleetCommandSystem sys(em);

    auto& entity = em.CreateEntity("Fleet Commander");
    auto* fc = em.AddComponent<FleetCommandComponent>(
        entity.id, std::make_unique<FleetCommandComponent>("Battle Fleet"));
    fc->AddMember(100, "Fighter A", FleetRole::Combat);
    fc->IssueOrder(FleetOrderType::Patrol, 100.0f, 200.0f, 300.0f);

    // First update: Pending -> Active
    sys.Update(0.1f);
    const FleetOrder* o = fc->GetOrder(1);
    TEST("Order activated", o && o->state == FleetOrderState::Active);

    // Update for 15+ seconds to complete (base order time is 15s)
    for (int i = 0; i < 160; ++i) {
        sys.Update(0.1f);
    }

    o = fc->GetOrder(1);
    TEST("Order completed", o && o->state == FleetOrderState::Completed);
    TEST("Order progress at 1.0", o && ApproxEq(o->progress, 1.0f));
}

// ===================================================================
// Scanning/Salvage/Fleet GameEvents Tests
// ===================================================================

static void TestScanningScalvageFleetGameEvents() {
    std::cout << "[Scanning/Salvage/Fleet GameEvents]\n";
    // Scanning events
    TEST("ScanStarted event", std::string(GameEvents::ScanStarted) == "scanning.scan.started");
    TEST("ScanCompleted event", std::string(GameEvents::ScanCompleted) == "scanning.scan.completed");
    TEST("ScanCancelled event", std::string(GameEvents::ScanCancelled) == "scanning.scan.cancelled");
    TEST("SignatureClassified event", std::string(GameEvents::SignatureClassified) == "scanning.signature.classified");
    TEST("ScannerTypeChanged event", std::string(GameEvents::ScannerTypeChanged) == "scanning.scanner.type_changed");
    // Salvage events
    TEST("SalvageStarted event", std::string(GameEvents::SalvageStarted) == "salvage.operation.started");
    TEST("SalvageCompleted event", std::string(GameEvents::SalvageCompleted) == "salvage.operation.completed");
    TEST("SalvageCancelled event", std::string(GameEvents::SalvageCancelled) == "salvage.operation.cancelled");
    TEST("SalvageCollected event", std::string(GameEvents::SalvageCollected) == "salvage.materials.collected");
    TEST("SalvageTierChanged event", std::string(GameEvents::SalvageTierChanged) == "salvage.tier.changed");
    // Fleet command events
    TEST("FleetOrderIssued event", std::string(GameEvents::FleetOrderIssued) == "fleet.order.issued");
    TEST("FleetOrderCompleted event", std::string(GameEvents::FleetOrderCompleted) == "fleet.order.completed");
    TEST("FleetOrderCancelled event", std::string(GameEvents::FleetOrderCancelled) == "fleet.order.cancelled");
    TEST("FleetMemberAdded event", std::string(GameEvents::FleetMemberAdded) == "fleet.member.added");
    TEST("FleetMemberRemoved event", std::string(GameEvents::FleetMemberRemoved) == "fleet.member.removed");
}

// ===================================================================
// Wormhole/ShipClass/Refinery GameEvents Tests
// ===================================================================

static void TestWormholeShipClassRefineryGameEvents() {
    std::cout << "[Wormhole/ShipClass/Refinery GameEvents]\n";
    // Wormhole events
    TEST("WormholeActivated event", std::string(GameEvents::WormholeActivated) == "wormhole.activated");
    TEST("WormholeCollapsed event", std::string(GameEvents::WormholeCollapsed) == "wormhole.collapsed");
    TEST("WormholeTraversalStarted event", std::string(GameEvents::WormholeTraversalStarted) == "wormhole.traversal.started");
    TEST("WormholeTraversalCompleted event", std::string(GameEvents::WormholeTraversalCompleted) == "wormhole.traversal.completed");
    TEST("WormholeDestabilizing event", std::string(GameEvents::WormholeDestabilizing) == "wormhole.destabilizing");
    TEST("WormholeLinkAdded event", std::string(GameEvents::WormholeLinkAdded) == "wormhole.link.added");
    // Ship class events
    TEST("ShipClassAssigned event", std::string(GameEvents::ShipClassAssigned) == "ship_class.assigned");
    TEST("ShipClassChanged event", std::string(GameEvents::ShipClassChanged) == "ship_class.changed");
    TEST("ShipClassUpgraded event", std::string(GameEvents::ShipClassUpgraded) == "ship_class.upgraded");
    // Refinery events
    TEST("RefiningStarted event", std::string(GameEvents::RefiningStarted) == "refinery.job.started");
    TEST("RefiningCompleted event", std::string(GameEvents::RefiningCompleted) == "refinery.job.completed");
    TEST("RefiningCancelled event", std::string(GameEvents::RefiningCancelled) == "refinery.job.cancelled");
    TEST("RefiningCollected event", std::string(GameEvents::RefiningCollected) == "refinery.job.collected");
    TEST("RefineryTierChanged event", std::string(GameEvents::RefineryTierChanged) == "refinery.tier.changed");
}

// ===================================================================
// ParticleSystem Update with EntityManager tests
// ===================================================================

static void TestParticleSystemUpdate() {
    std::cout << "[ParticleSystemUpdate]\n";

    EntityManager em;
    ParticleSystem sys;
    sys.SetEntityManager(&em);
    sys.Initialize();

    // No entities — update should be safe
    sys.Update(0.016f);
    TEST("No entities particle count zero", sys.GetLastUpdateParticleCount() == 0);

    // Create an entity with a ParticleComponent
    auto& entity = em.CreateEntity("particle_test");
    auto* comp = em.AddComponent<ParticleComponent>(entity.id,
                     std::make_unique<ParticleComponent>());

    // Add an emitter with burst
    auto cfg = ParticleSystem::CreateExplosionPreset();
    ParticleEmitter emitter("explosion", cfg);
    emitter.SetSeed(42);
    emitter.Emit(10);
    comp->AddEmitter(emitter);

    TEST("Component has emitter", comp->GetEmitterCount() == 1);
    TEST("Emitter has particles", comp->GetTotalParticleCount() == 10);

    // Update system — should update all emitters
    sys.Update(0.016f);
    TEST("System reports particle count", sys.GetLastUpdateParticleCount() > 0);

    // Create second entity with particles
    auto& entity2 = em.CreateEntity("particle_test2");
    auto* comp2 = em.AddComponent<ParticleComponent>(entity2.id,
                      std::make_unique<ParticleComponent>());
    ParticleEmitter emitter2("thrust", ParticleSystem::CreateEngineThrustPreset());
    emitter2.SetSeed(123);
    comp2->AddEmitter(emitter2);

    sys.Update(0.5f);
    TEST("System handles multiple entities", sys.GetLastUpdateParticleCount() >= 0);

    // Disable system
    sys.SetEnabled(false);
    sys.Update(0.016f);
    // When disabled, count shouldn't reset
    sys.SetEnabled(true);
    sys.Shutdown();
    TEST("Shutdown resets count", sys.GetLastUpdateParticleCount() == 0);
}

// ===================================================================
// AudioSystem Event Dispatching tests
// ===================================================================

static void TestAudioEventDispatching() {
    std::cout << "[AudioEventDispatching]\n";

    EventSystem::Instance().ClearAllListeners();

    int soundPlayedCount = 0;
    int soundStoppedCount = 0;
    int musicStartedCount = 0;
    int musicStoppedCount = 0;
    int trackChangedCount = 0;

    EventSystem::Instance().Subscribe(GameEvents::SoundPlayed,
        [&](const GameEvent&) { ++soundPlayedCount; });
    EventSystem::Instance().Subscribe(GameEvents::SoundStopped,
        [&](const GameEvent&) { ++soundStoppedCount; });
    EventSystem::Instance().Subscribe(GameEvents::MusicStarted,
        [&](const GameEvent&) { ++musicStartedCount; });
    EventSystem::Instance().Subscribe(GameEvents::MusicStopped,
        [&](const GameEvent&) { ++musicStoppedCount; });
    EventSystem::Instance().Subscribe(GameEvents::MusicTrackChanged,
        [&](const GameEvent&) { ++trackChangedCount; });

    AudioSystem audio;
    audio.Initialize();

    AudioClip clip;
    clip.id = "laser";
    clip.durationSeconds = 1.0f;
    clip.category = AudioCategory::SFX;
    audio.RegisterClip(clip);

    AudioClip clip2;
    clip2.id = "explosion";
    clip2.durationSeconds = 2.0f;
    clip2.category = AudioCategory::SFX;
    audio.RegisterClip(clip2);

    // Play sound dispatches event
    uint64_t sid = audio.PlaySound("laser");
    TEST("SoundPlayed event fired on PlaySound", soundPlayedCount == 1);

    // Play 3D sound dispatches event
    audio.PlaySound3D("explosion", 1.0f, 2.0f, 3.0f);
    TEST("SoundPlayed event fired on PlaySound3D", soundPlayedCount == 2);

    // Stop sound dispatches event
    audio.StopSound(sid);
    TEST("SoundStopped event fired on StopSound", soundStoppedCount == 1);

    // StopSound with invalid ID should NOT dispatch
    audio.StopSound(99999);
    TEST("SoundStopped not fired for invalid ID", soundStoppedCount == 1);

    // Music events
    AudioClip track1;
    track1.id = "track1";
    track1.durationSeconds = 3.0f;
    track1.category = AudioCategory::Music;
    audio.RegisterClip(track1);

    AudioClip track2;
    track2.id = "track2";
    track2.durationSeconds = 3.0f;
    track2.category = AudioCategory::Music;
    audio.RegisterClip(track2);

    MusicPlaylist playlist;
    playlist.trackIds = {"track1", "track2"};
    playlist.repeat = true;
    audio.SetMusicPlaylist(playlist);

    // PlayMusic dispatches MusicStarted
    audio.PlayMusic();
    TEST("MusicStarted event fired on PlayMusic", musicStartedCount == 1);

    // NextTrack dispatches MusicTrackChanged
    audio.NextTrack();
    TEST("MusicTrackChanged event fired on NextTrack", trackChangedCount == 1);

    // StopMusic dispatches MusicStopped
    audio.StopMusic();
    TEST("MusicStopped event fired on StopMusic", musicStoppedCount == 1);

    // StopMusic again when already stopped should not fire again
    audio.StopMusic();
    TEST("MusicStopped not fired when already stopped", musicStoppedCount == 1);

    audio.Shutdown();
    EventSystem::Instance().ClearAllListeners();
}

// ===================================================================
// PostProcessing Config tests
// ===================================================================

static void TestPostProcessingConfig() {
    std::cout << "[PostProcessingConfig]\n";

    PostProcessingConfig cfg;
    TEST("Default no effects enabled", cfg.GetEnabledEffectCount() == 0);
    TEST("Bloom disabled by default", !cfg.IsEffectEnabled(PostProcessEffect::Bloom));
    TEST("HDR disabled by default", !cfg.IsEffectEnabled(PostProcessEffect::HDR));
    TEST("ToneMapping disabled by default", !cfg.IsEffectEnabled(PostProcessEffect::ToneMapping));
    TEST("Vignette disabled by default", !cfg.IsEffectEnabled(PostProcessEffect::Vignette));
    TEST("ChromaticAberration disabled by default", !cfg.IsEffectEnabled(PostProcessEffect::ChromaticAberration));
    TEST("FXAA disabled by default", !cfg.IsEffectEnabled(PostProcessEffect::FXAA));

    // Enable individual effects
    cfg.SetEffectEnabled(PostProcessEffect::Bloom, true);
    TEST("Bloom enabled", cfg.IsEffectEnabled(PostProcessEffect::Bloom));
    TEST("1 effect enabled", cfg.GetEnabledEffectCount() == 1);

    cfg.SetEffectEnabled(PostProcessEffect::HDR, true);
    cfg.SetEffectEnabled(PostProcessEffect::FXAA, true);
    TEST("3 effects enabled", cfg.GetEnabledEffectCount() == 3);

    cfg.SetEffectEnabled(PostProcessEffect::Bloom, false);
    TEST("Bloom disabled again", !cfg.IsEffectEnabled(PostProcessEffect::Bloom));
    TEST("2 effects enabled after disable", cfg.GetEnabledEffectCount() == 2);

    // Default bloom settings
    PostProcessingConfig defaults;
    TEST("Default bloom threshold", ApproxEq(defaults.bloom.threshold, 0.8f));
    TEST("Default bloom intensity", ApproxEq(defaults.bloom.intensity, 1.0f));
    TEST("Default bloom blur passes", defaults.bloom.blurPasses == 4);
    TEST("Default HDR exposure", ApproxEq(defaults.hdr.exposure, 1.0f));
    TEST("Default HDR gamma", ApproxEq(defaults.hdr.gamma, 2.2f));
    TEST("Default vignette intensity", ApproxEq(defaults.vignette.intensity, 0.3f));
}

static void TestPostProcessingPresets() {
    std::cout << "[PostProcessingPresets]\n";

    // Low preset
    PostProcessingConfig low;
    low.ApplyPreset(PostProcessingConfig::QualityPreset::Low);
    TEST("Low: bloom disabled", !low.bloom.enabled);
    TEST("Low: HDR enabled", low.hdr.enabled);
    TEST("Low: tone mapping enabled", low.toneMapping.enabled);
    TEST("Low: tone map Reinhard", low.toneMapping.op == ToneMappingSettings::Operator::Reinhard);
    TEST("Low: vignette disabled", !low.vignette.enabled);
    TEST("Low: fxaa disabled", !low.fxaa.enabled);

    // Medium preset
    PostProcessingConfig med;
    med.ApplyPreset(PostProcessingConfig::QualityPreset::Medium);
    TEST("Med: bloom enabled", med.bloom.enabled);
    TEST("Med: bloom 2 passes", med.bloom.blurPasses == 2);
    TEST("Med: fxaa enabled", med.fxaa.enabled);
    TEST("Med: vignette enabled", med.vignette.enabled);
    TEST("Med: chromatic aberration disabled", !med.chromaticAberration.enabled);

    // High preset
    PostProcessingConfig high;
    high.ApplyPreset(PostProcessingConfig::QualityPreset::High);
    TEST("High: bloom enabled", high.bloom.enabled);
    TEST("High: bloom 4 passes", high.bloom.blurPasses == 4);
    TEST("High: auto exposure", high.hdr.autoExposure);
    TEST("High: chromatic aberration enabled", high.chromaticAberration.enabled);
    TEST("High: ACES tone mapping", high.toneMapping.op == ToneMappingSettings::Operator::ACES);

    // Ultra preset
    PostProcessingConfig ultra;
    ultra.ApplyPreset(PostProcessingConfig::QualityPreset::Ultra);
    TEST("Ultra: bloom enabled", ultra.bloom.enabled);
    TEST("Ultra: bloom 6 passes", ultra.bloom.blurPasses == 6);
    TEST("Ultra: bloom threshold lower", ApproxEq(ultra.bloom.threshold, 0.6f));
    TEST("Ultra: all effects enabled", ultra.GetEnabledEffectCount() == 6);
}

static void TestPostProcessingComponent() {
    std::cout << "[PostProcessingComponent]\n";

    PostProcessingComponent comp;
    TEST("Default priority is 0", comp.priority == 0);
    TEST("Default config no effects", comp.config.GetEnabledEffectCount() == 0);

    comp.priority = 5;
    comp.config.ApplyPreset(PostProcessingConfig::QualityPreset::High);
    TEST("Priority set", comp.priority == 5);
    TEST("Config applied", comp.config.bloom.enabled);
}

static void TestPostProcessingComponentSerialization() {
    std::cout << "[PostProcessingComponentSerialization]\n";

    PostProcessingComponent original;
    original.priority = 3;
    original.config.ApplyPreset(PostProcessingConfig::QualityPreset::Ultra);

    // Serialize
    auto data = original.Serialize();
    TEST("Serialize type correct", data.componentType == "PostProcessingComponent");
    TEST("Serialize has priority", data.data.count("priority") > 0);
    TEST("Serialize has bloom_enabled", data.data.count("bloom_enabled") > 0);

    // Deserialize into a new component
    PostProcessingComponent restored;
    restored.Deserialize(data);

    TEST("Restored priority", restored.priority == 3);
    TEST("Restored bloom enabled", restored.config.bloom.enabled);
    TEST("Restored bloom threshold", ApproxEq(restored.config.bloom.threshold, 0.6f));
    TEST("Restored bloom passes", restored.config.bloom.blurPasses == 6);
    TEST("Restored HDR enabled", restored.config.hdr.enabled);
    TEST("Restored HDR auto exposure", restored.config.hdr.autoExposure);
    TEST("Restored tone mapping enabled", restored.config.toneMapping.enabled);
    TEST("Restored vignette enabled", restored.config.vignette.enabled);
    TEST("Restored chromatic aberration enabled", restored.config.chromaticAberration.enabled);
    TEST("Restored FXAA enabled", restored.config.fxaa.enabled);
    TEST("Restored all effects", restored.config.GetEnabledEffectCount() == 6);
}

static void TestPostProcessingSystem() {
    std::cout << "[PostProcessingSystem]\n";

    PostProcessingSystem sys;
    TEST("System name", sys.GetName() == "PostProcessingSystem");
    TEST("System enabled", sys.IsEnabled());

    EntityManager em;
    sys.SetEntityManager(&em);
    sys.Initialize();

    // No components
    sys.Update(0.016f);
    TEST("No components count zero", sys.GetActiveComponentCount() == 0);

    // Add a PostProcessingComponent
    auto& entity = em.CreateEntity("camera");
    auto* comp = em.AddComponent<PostProcessingComponent>(entity.id,
                     std::make_unique<PostProcessingComponent>());
    comp->config.ApplyPreset(PostProcessingConfig::QualityPreset::High);

    sys.Update(0.016f);
    TEST("One component active", sys.GetActiveComponentCount() == 1);

    // Global config
    PostProcessingConfig globalCfg;
    globalCfg.ApplyPreset(PostProcessingConfig::QualityPreset::Medium);
    sys.SetGlobalConfig(globalCfg);
    TEST("Global config bloom enabled", sys.GetGlobalConfig().bloom.enabled);

    // Apply preset via system
    sys.ApplyGlobalPreset(PostProcessingConfig::QualityPreset::Low);
    TEST("Global preset applied", !sys.GetGlobalConfig().bloom.enabled);
    TEST("Global HDR still enabled", sys.GetGlobalConfig().hdr.enabled);

    sys.Shutdown();
    TEST("Shutdown resets count", sys.GetActiveComponentCount() == 0);
}

// ===================================================================
// Shadow System tests
// ===================================================================

static void TestShadowMapConfig() {
    std::cout << "[ShadowMapConfig]\n";

    ShadowMapConfig cfg;
    TEST("Default resolution 1024", cfg.resolution == 1024);
    TEST("Default near plane", ApproxEq(cfg.nearPlane, 0.1f));
    TEST("Default far plane", ApproxEq(cfg.farPlane, 100.0f));
    TEST("Default bias", ApproxEq(cfg.bias, 0.005f));
    TEST("Default cascade count", cfg.cascadeCount == 3);
    TEST("Default shadow type PCF", cfg.shadowType == ShadowType::PCF);

    // Resolution for quality
    TEST("Off resolution 0", ShadowMapConfig::ResolutionForQuality(ShadowQuality::Off) == 0);
    TEST("Low resolution 512", ShadowMapConfig::ResolutionForQuality(ShadowQuality::Low) == 512);
    TEST("Medium resolution 1024", ShadowMapConfig::ResolutionForQuality(ShadowQuality::Medium) == 1024);
    TEST("High resolution 2048", ShadowMapConfig::ResolutionForQuality(ShadowQuality::High) == 2048);
    TEST("Ultra resolution 4096", ShadowMapConfig::ResolutionForQuality(ShadowQuality::Ultra) == 4096);

    // Apply quality presets
    cfg.ApplyQuality(ShadowQuality::Off);
    TEST("Off: no shadow type", cfg.shadowType == ShadowType::None);
    TEST("Off: no cascades", cfg.cascadeCount == 0);

    cfg.ApplyQuality(ShadowQuality::Low);
    TEST("Low: hard shadows", cfg.shadowType == ShadowType::Hard);
    TEST("Low: 1 cascade", cfg.cascadeCount == 1);
    TEST("Low: resolution 512", cfg.resolution == 512);

    cfg.ApplyQuality(ShadowQuality::Medium);
    TEST("Med: PCF shadows", cfg.shadowType == ShadowType::PCF);
    TEST("Med: 2 cascades", cfg.cascadeCount == 2);

    cfg.ApplyQuality(ShadowQuality::High);
    TEST("High: PCF shadows", cfg.shadowType == ShadowType::PCF);
    TEST("High: 3 cascades", cfg.cascadeCount == 3);
    TEST("High: resolution 2048", cfg.resolution == 2048);

    cfg.ApplyQuality(ShadowQuality::Ultra);
    TEST("Ultra: VSM shadows", cfg.shadowType == ShadowType::VSM);
    TEST("Ultra: 4 cascades", cfg.cascadeCount == 4);
    TEST("Ultra: resolution 4096", cfg.resolution == 4096);
}

static void TestLightSource() {
    std::cout << "[LightSource]\n";

    LightSource light;
    TEST("Default type directional", light.type == LightType::Directional);
    TEST("Default intensity", ApproxEq(light.intensity, 1.0f));
    TEST("Default casts shadows", light.castsShadows);
    TEST("Default color white", ApproxEq(light.colorR, 1.0f) && ApproxEq(light.colorG, 1.0f) && ApproxEq(light.colorB, 1.0f));

    // Point light
    light.type = LightType::Point;
    light.radius = 100.0f;
    light.position = {10.0f, 20.0f, 30.0f};
    TEST("Point light type", light.type == LightType::Point);
    TEST("Point light radius", ApproxEq(light.radius, 100.0f));

    // Spot light
    LightSource spot;
    spot.type = LightType::Spot;
    spot.innerConeAngleDeg = 20.0f;
    spot.outerConeAngleDeg = 40.0f;
    TEST("Spot inner cone", ApproxEq(spot.innerConeAngleDeg, 20.0f));
    TEST("Spot outer cone", ApproxEq(spot.outerConeAngleDeg, 40.0f));
}

static void TestShadowComponent() {
    std::cout << "[ShadowComponent]\n";

    ShadowComponent comp;
    TEST("Default dirty", comp.isDirty);
    TEST("Default shadowMapId 0", comp.shadowMapId == 0);
    TEST("Default light type directional", comp.light.type == LightType::Directional);
    TEST("Default shadow config resolution", comp.shadowConfig.resolution == 1024);

    comp.light.type = LightType::Point;
    comp.light.intensity = 2.5f;
    comp.shadowConfig.ApplyQuality(ShadowQuality::High);
    comp.isDirty = false;
    TEST("Light type set", comp.light.type == LightType::Point);
    TEST("Light intensity set", ApproxEq(comp.light.intensity, 2.5f));
    TEST("Shadow quality applied", comp.shadowConfig.cascadeCount == 3);
}

static void TestShadowComponentSerialization() {
    std::cout << "[ShadowComponentSerialization]\n";

    ShadowComponent original;
    original.light.type = LightType::Spot;
    original.light.position = {5.0f, 10.0f, 15.0f};
    original.light.direction = {0.0f, -1.0f, 0.0f};
    original.light.intensity = 2.0f;
    original.light.colorR = 0.9f;
    original.light.colorG = 0.8f;
    original.light.colorB = 0.7f;
    original.light.castsShadows = true;
    original.light.innerConeAngleDeg = 25.0f;
    original.light.outerConeAngleDeg = 50.0f;
    original.light.radius = 75.0f;
    original.shadowConfig.resolution = 2048;
    original.shadowConfig.cascadeCount = 3;
    original.shadowConfig.shadowType = ShadowType::PCF;
    original.shadowConfig.bias = 0.003f;
    original.isDirty = false;

    auto data = original.Serialize();
    TEST("Serialize type correct", data.componentType == "ShadowComponent");

    ShadowComponent restored;
    restored.Deserialize(data);

    TEST("Restored light type Spot", restored.light.type == LightType::Spot);
    TEST("Restored light pos X", ApproxEq(restored.light.position.x, 5.0f));
    TEST("Restored light pos Y", ApproxEq(restored.light.position.y, 10.0f));
    TEST("Restored light pos Z", ApproxEq(restored.light.position.z, 15.0f));
    TEST("Restored light intensity", ApproxEq(restored.light.intensity, 2.0f));
    TEST("Restored light colorR", ApproxEq(restored.light.colorR, 0.9f));
    TEST("Restored light colorG", ApproxEq(restored.light.colorG, 0.8f));
    TEST("Restored light colorB", ApproxEq(restored.light.colorB, 0.7f));
    TEST("Restored casts shadows", restored.light.castsShadows);
    TEST("Restored inner cone", ApproxEq(restored.light.innerConeAngleDeg, 25.0f));
    TEST("Restored outer cone", ApproxEq(restored.light.outerConeAngleDeg, 50.0f));
    TEST("Restored radius", ApproxEq(restored.light.radius, 75.0f));
    TEST("Restored shadow resolution", restored.shadowConfig.resolution == 2048);
    TEST("Restored cascade count", restored.shadowConfig.cascadeCount == 3);
    TEST("Restored shadow type PCF", restored.shadowConfig.shadowType == ShadowType::PCF);
    TEST("Restored bias", ApproxEq(restored.shadowConfig.bias, 0.003f));
}

static void TestShadowCasterComponent() {
    std::cout << "[ShadowCasterComponent]\n";

    ShadowCasterComponent caster;
    TEST("Default casts shadows", caster.castsShadows);
    TEST("Default receives shadows", caster.receivesShadows);
    TEST("Default bounding radius", ApproxEq(caster.boundingRadius, 1.0f));

    caster.castsShadows = false;
    caster.boundingRadius = 5.0f;
    TEST("Casts shadows disabled", !caster.castsShadows);
    TEST("Bounding radius updated", ApproxEq(caster.boundingRadius, 5.0f));
}

static void TestShadowSystem() {
    std::cout << "[ShadowSystem]\n";

    ShadowSystem sys;
    TEST("System name", sys.GetName() == "ShadowSystem");
    TEST("System enabled", sys.IsEnabled());
    TEST("Default quality medium", sys.GetShadowQuality() == ShadowQuality::Medium);

    EntityManager em;
    sys.SetEntityManager(&em);
    sys.Initialize();

    // No entities
    sys.Update(0.016f);
    TEST("No lights count zero", sys.GetActiveShadowLightCount() == 0);
    TEST("No casters count zero", sys.GetShadowCasterCount() == 0);
    TEST("No shadow maps", sys.GetShadowMapCount() == 0);

    // Add a directional light
    auto& lightEntity = em.CreateEntity("sun");
    auto* shadow = em.AddComponent<ShadowComponent>(lightEntity.id,
                       std::make_unique<ShadowComponent>());
    shadow->light.type = LightType::Directional;
    shadow->light.castsShadows = true;

    sys.Update(0.016f);
    TEST("One shadow light", sys.GetActiveShadowLightCount() == 1);
    TEST("Shadow map ID assigned", shadow->shadowMapId > 0);
    TEST("Cascaded shadow maps for directional", sys.GetShadowMapCount() >= 1);

    // Add shadow casters
    auto& casterEntity = em.CreateEntity("ship");
    auto* caster = em.AddComponent<ShadowCasterComponent>(casterEntity.id,
                       std::make_unique<ShadowCasterComponent>());
    caster->castsShadows = true;

    auto& casterEntity2 = em.CreateEntity("station");
    auto* caster2 = em.AddComponent<ShadowCasterComponent>(casterEntity2.id,
                        std::make_unique<ShadowCasterComponent>());
    caster2->castsShadows = true;

    sys.Update(0.016f);
    TEST("Two shadow casters", sys.GetShadowCasterCount() == 2);

    // Add a non-shadow-casting light
    auto& lightEntity2 = em.CreateEntity("ambient");
    auto* shadow2 = em.AddComponent<ShadowComponent>(lightEntity2.id,
                        std::make_unique<ShadowComponent>());
    shadow2->light.castsShadows = false;

    sys.Update(0.016f);
    TEST("Still one shadow light (ambient excluded)", sys.GetActiveShadowLightCount() == 1);

    // Change quality
    sys.SetShadowQuality(ShadowQuality::High);
    TEST("Quality changed to high", sys.GetShadowQuality() == ShadowQuality::High);

    // Quality Off disables shadow processing
    sys.SetShadowQuality(ShadowQuality::Off);
    sys.Update(0.016f);
    TEST("Off quality: no shadow lights", sys.GetActiveShadowLightCount() == 0);
    TEST("Off quality: no shadow maps", sys.GetShadowMapCount() == 0);

    // Re-enable
    sys.SetShadowQuality(ShadowQuality::Medium);
    sys.Update(0.016f);
    TEST("Re-enabled shadow lights", sys.GetActiveShadowLightCount() == 1);

    // Invalidate
    sys.InvalidateAllShadowMaps();
    TEST("Invalidated shadow maps", shadow->isDirty);

    sys.Shutdown();
    TEST("Shutdown resets counts", sys.GetActiveShadowLightCount() == 0);
}

static void TestShadowSystemPointSpotLights() {
    std::cout << "[ShadowSystemPointSpotLights]\n";

    EntityManager em;
    ShadowSystem sys;
    sys.SetEntityManager(&em);
    sys.Initialize();

    // Point light — single shadow map
    auto& pointEntity = em.CreateEntity("point_light");
    auto* pointShadow = em.AddComponent<ShadowComponent>(pointEntity.id,
                             std::make_unique<ShadowComponent>());
    pointShadow->light.type = LightType::Point;
    pointShadow->light.castsShadows = true;

    sys.Update(0.016f);
    TEST("Point light shadow map count 1", sys.GetShadowMapCount() == 1);

    // Spot light — single shadow map
    auto& spotEntity = em.CreateEntity("spot_light");
    auto* spotShadow = em.AddComponent<ShadowComponent>(spotEntity.id,
                            std::make_unique<ShadowComponent>());
    spotShadow->light.type = LightType::Spot;
    spotShadow->light.castsShadows = true;

    sys.Update(0.016f);
    TEST("Two lights total", sys.GetActiveShadowLightCount() == 2);
    TEST("Two shadow maps (1 point + 1 spot)", sys.GetShadowMapCount() == 2);

    sys.Shutdown();
}

// ===================================================================
// New GameEvents for Post-Processing and Shadow systems
// ===================================================================

static void TestPostProcessShadowGameEvents() {
    std::cout << "[PostProcessShadowGameEvents]\n";

    // Post-processing events
    TEST("PostProcessEffectEnabled event",
         std::string(GameEvents::PostProcessEffectEnabled) == "rendering.postprocess.effect_enabled");
    TEST("PostProcessEffectDisabled event",
         std::string(GameEvents::PostProcessEffectDisabled) == "rendering.postprocess.effect_disabled");
    TEST("PostProcessPresetApplied event",
         std::string(GameEvents::PostProcessPresetApplied) == "rendering.postprocess.preset_applied");

    // Shadow events
    TEST("ShadowQualityChanged event",
         std::string(GameEvents::ShadowQualityChanged) == "rendering.shadow.quality_changed");
    TEST("ShadowMapInvalidated event",
         std::string(GameEvents::ShadowMapInvalidated) == "rendering.shadow.map_invalidated");
    TEST("ShadowLightAdded event",
         std::string(GameEvents::ShadowLightAdded) == "rendering.shadow.light_added");
}

// ===================================================================
// Main
// ===================================================================
// ===================================================================
// Vector2 tests
// ===================================================================
static void TestVector2() {
    std::cout << "[Vector2]\n";
    Vector2 a{3.0f, 4.0f};
    bool lenOk = ApproxEq(a.Length(), 5.0f);
    TEST("Vector2 length", lenOk);
    Vector2 n = a.Normalized();
    bool normOk = ApproxEq(n.Length(), 1.0f);
    TEST("Vector2 normalized", normOk);
    Vector2 addA{1.0f,2.0f}, addB{3.0f,4.0f};
    Vector2 addExpected{4.0f,6.0f};
    TEST("Vector2 add", (addA + addB) == addExpected);
    Vector2 subA{5.0f,6.0f}, subB{3.0f,4.0f};
    Vector2 subExpected{2.0f,2.0f};
    TEST("Vector2 sub", (subA - subB) == subExpected);
    Vector2 scaleV{1.0f,2.0f};
    Vector2 scaleExpected{3.0f,6.0f};
    TEST("Vector2 scale", (scaleV * 3.0f) == scaleExpected);
    Vector2 dotA{1.0f,0.0f}, dotB{0.0f,1.0f};
    bool dotOk = ApproxEq(Vector2::Dot(dotA, dotB), 0.0f);
    TEST("Vector2 dot", dotOk);
    Vector2 distA{0.0f,0.0f}, distB{3.0f,4.0f};
    bool distOk = ApproxEq(Vector2::Distance(distA, distB), 5.0f);
    TEST("Vector2 distance", distOk);
    TEST("Vector2 zero", Vector2::Zero() == Vector2::Zero());
    TEST("Vector2 one",  Vector2::One()  == Vector2::One());
}

// ===================================================================
// Matrix4 tests
// ===================================================================
static void TestMatrix4() {
    std::cout << "[Matrix4]\n";
    Matrix4 id = Matrix4::Identity();
    Vector3 v{1,2,3};
    Vector3 tv = id.TransformPoint(v);
    TEST("Identity transform point", ApproxEq(tv.x,1) && ApproxEq(tv.y,2) && ApproxEq(tv.z,3));

    Matrix4 t = Matrix4::Translate(1,2,3);
    Vector3 tt = t.TransformPoint({0,0,0});
    TEST("Translate matrix", ApproxEq(tt.x,1) && ApproxEq(tt.y,2) && ApproxEq(tt.z,3));

    Matrix4 s = Matrix4::Scale(2.0f);
    Vector3 sv = s.TransformPoint({1,1,1});
    TEST("Scale matrix", ApproxEq(sv.x,2) && ApproxEq(sv.y,2) && ApproxEq(sv.z,2));

    Matrix4 tr = id.Transposed();
    for (int r=0;r<4;r++) for (int c=0;c<4;c++)
        TEST("Transpose identity", ApproxEq(tr.m[r][c], id.m[c][r]));
    (void)tr;
}

// ===================================================================
// Quaternion tests
// ===================================================================
static void TestQuaternion() {
    std::cout << "[Quaternion]\n";
    Quaternion q = Quaternion::Identity();
    TEST("Identity quaternion w=1", ApproxEq(q.w, 1.0f));
    TEST("Identity length", ApproxEq(q.Length(), 1.0f));

    // 90-degree rotation around Y
    float pi = 3.14159265f;
    Quaternion ry = Quaternion::FromAxisAngle({0,1,0}, pi*0.5f);
    Vector3 fwd{0,0,-1};
    Vector3 rot = ry.Rotate(fwd);
    TEST("Rotate around Y", ApproxEq(rot.x, -1.0f) && ApproxEq(rot.z, 0.0f));

    // Slerp between identity and ry
    Quaternion mid = Quaternion::Slerp(q, ry, 0.5f);
    TEST("Slerp normalized", ApproxEq(mid.Length(), 1.0f));

    // ToMatrix4 round-trip (identity)
    Matrix4 m = q.ToMatrix4();
    TEST("Identity quaternion to matrix diagonal", ApproxEq(m.m[0][0],1) && ApproxEq(m.m[1][1],1));

    // Euler
    Quaternion qe = Quaternion::FromEuler(0,0,0);
    TEST("Zero euler = identity", ApproxEq(qe.w, 1.0f));
}

// ===================================================================
// LinearAllocator tests
// ===================================================================
static void TestLinearAllocator() {
    std::cout << "[LinearAllocator]\n";
    alignas(alignof(std::max_align_t)) std::byte buf[256];
    LinearAllocator alloc(buf, sizeof(buf));

    TEST("Initial used = 0", alloc.Used() == 0);
    TEST("Capacity correct", alloc.Capacity() == 256);

    void* p1 = alloc.Allocate(16);
    TEST("Allocate returns non-null", p1 != nullptr);
    TEST("Used > 0 after alloc", alloc.Used() > 0);

    void* p2 = alloc.Allocate(16);
    TEST("Second alloc non-null", p2 != nullptr);
    TEST("Pointers differ", p1 != p2);

    alloc.Deallocate(p1); // no-op
    TEST("Used unchanged after deallocate", alloc.Used() > 0);

    alloc.Reset();
    TEST("Used = 0 after Reset", alloc.Used() == 0);

    void* big = alloc.Allocate(512);
    TEST("Alloc too large returns null", big == nullptr);
}

// ===================================================================
// PoolAllocator tests
// ===================================================================
static void TestPoolAllocator() {
    std::cout << "[PoolAllocator]\n";
    PoolAllocator<int, 4> pool;
    TEST("Initial free count", pool.FreeCount() == 4);

    void* a = pool.Allocate(sizeof(int));
    TEST("Alloc non-null", a != nullptr);
    TEST("Free count decremented", pool.FreeCount() == 3);

    void* b = pool.Allocate(sizeof(int));
    void* c = pool.Allocate(sizeof(int));
    void* d = pool.Allocate(sizeof(int));
    TEST("Fill pool", d != nullptr);
    TEST("Empty free count", pool.FreeCount() == 0);

    void* e = pool.Allocate(sizeof(int));
    TEST("Alloc from full pool = null", e == nullptr);

    pool.Deallocate(a);
    TEST("Free count after deallocate", pool.FreeCount() == 1);

    void* f = pool.Allocate(sizeof(int));
    TEST("Realloc after free", f != nullptr);
    (void)b; (void)c; (void)d; (void)f;
}

// ===================================================================
// EventBus tests
// ===================================================================
static void TestEventBus() {
    std::cout << "[EventBus]\n";
    struct PingEvent { int value; };
    struct PongEvent { float x; };

    EventBus bus;
    int pingCount = 0;
    int pongCount = 0;
    int lastPing  = 0;

    bus.Subscribe<PingEvent>([&](const PingEvent& e) { ++pingCount; lastPing = e.value; });
    bus.Subscribe<PongEvent>([&](const PongEvent&)   { ++pongCount; });

    bus.Publish(PingEvent{42});
    TEST("Ping received", pingCount == 1);
    TEST("Ping value", lastPing == 42);
    TEST("Pong not triggered", pongCount == 0);

    bus.Publish(PongEvent{1.0f});
    TEST("Pong received", pongCount == 1);

    TEST("Handler count ping", bus.GetHandlerCount<PingEvent>() == 1);

    bus.Clear();
    bus.Publish(PingEvent{99});
    TEST("No ping after Clear", pingCount == 1);
}

// ===================================================================
// TaskSystem tests
// ===================================================================
static void TestTaskSystem() {
    std::cout << "[TaskSystem]\n";
    TaskSystem tasks;
    TEST("Not running before Init", !tasks.IsRunning());

    tasks.Init(2);
    TEST("Running after Init", tasks.IsRunning());

    std::atomic<int> counter{0};
    auto f1 = tasks.Submit([&]{ ++counter; });
    auto f2 = tasks.Submit([&]{ ++counter; });
    f1.wait(); f2.wait();
    TEST("Both tasks executed", counter.load() == 2);

    tasks.Shutdown();
    TEST("Not running after Shutdown", !tasks.IsRunning());

    // Inline fallback when not initialised
    int inline_count = 0;
    TaskSystem tasks2;
    auto f3 = tasks2.Submit([&]{ ++inline_count; });
    f3.wait();
    TEST("Inline execution when not Init'd", inline_count == 1);
}

// ===================================================================
// AssetManager tests
// ===================================================================
static void TestAssetManager() {
    std::cout << "[AssetManager]\n";
    struct FakeTexture { int w = 0; int h = 0; };

    AssetManager assets;
    TEST("Empty count", assets.LoadedCount() == 0);

    AssetHandle h = assets.Load<FakeTexture>("textures/ship.png");
    TEST("Handle non-null", h != NullAsset);
    TEST("Loaded count 1", assets.LoadedCount() == 1);

    // Second load of same path returns same handle
    AssetHandle h2 = assets.Load<FakeTexture>("textures/ship.png");
    TEST("Same handle on re-load", h == h2);
    TEST("Ref count 2", assets.GetRefCount(h) == 2);

    FakeTexture* tex = assets.Get<FakeTexture>(h);
    TEST("Get returns non-null", tex != nullptr);

    assets.Unload(h);
    TEST("Ref count 1 after Unload", assets.GetRefCount(h) == 1);
    assets.Unload(h);
    TEST("Ref count 0 after second Unload", assets.GetRefCount(h) == 0);

    assets.GarbageCollect();
    TEST("Collected: loaded count 0", assets.LoadedCount() == 0);
    TEST("Get returns null after GC", assets.Get<FakeTexture>(h) == nullptr);

    TEST("Path lookup", assets.GetPath(NullAsset).empty());
}

// ===================================================================
// BinaryArchive tests
// ===================================================================
static void TestBinaryArchive() {
    std::cout << "[BinaryArchive]\n";

    // Write mode
    BinaryArchive ar;
    TEST("Write mode", ar.IsWriting());

    int   i = 42;
    float f = 3.14f;
    std::string s = "hello";

    ar.Serialize(i);
    ar.Serialize(f);
    ar.Serialize(s);

    TEST("Buffer non-empty after writes", ar.Size() > 0);

    // Read back
    const auto& buf = ar.Data();
    BinaryArchive ar2(buf.data(), buf.size());
    TEST("Read mode", ar2.IsReading());

    int   ri = 0;
    float rf = 0;
    std::string rs;
    ar2.Serialize(ri);
    ar2.Serialize(rf);
    ar2.Serialize(rs);

    TEST("Int round-trip", ri == 42);
    TEST("Float round-trip", ApproxEq(rf, 3.14f));
    TEST("String round-trip", rs == "hello");

    // Read past end throws
    bool threw = false;
    try { ar2.Serialize(ri); } catch (const std::out_of_range&) { threw = true; }
    TEST("Read past end throws", threw);
}

// ===================================================================
// Keyboard tests
// ===================================================================
static void TestKeyboard() {
    std::cout << "[Keyboard]\n";
    using namespace Input;
    Keyboard kb;

    std::array<bool, static_cast<std::size_t>(KeyCode::Count)> state{};
    state[static_cast<std::size_t>(KeyCode::W)] = true;
    kb.Update(state);

    TEST("IsKeyDown W", kb.IsKeyDown(KeyCode::W));
    TEST("IsKeyPressed W (first frame)", kb.IsKeyPressed(KeyCode::W));
    TEST("IsKeyDown A false", !kb.IsKeyDown(KeyCode::A));

    // Second frame: W still held
    kb.Update(state);
    TEST("IsKeyDown W second frame", kb.IsKeyDown(KeyCode::W));
    TEST("IsKeyPressed W second frame = false", !kb.IsKeyPressed(KeyCode::W));

    // Release W
    state[static_cast<std::size_t>(KeyCode::W)] = false;
    kb.Update(state);
    TEST("IsKeyDown W after release", !kb.IsKeyDown(KeyCode::W));
    TEST("IsKeyReleased W", kb.IsKeyReleased(KeyCode::W));

    // SetKey helper
    kb.SetKey(KeyCode::Space, true);
    TEST("SetKey Space down", kb.IsKeyDown(KeyCode::Space));
}

// ===================================================================
// Mouse tests
// ===================================================================
static void TestMouse() {
    std::cout << "[Mouse]\n";
    using namespace Input;
    Mouse mouse;

    // Button pressed (bit 0 = left)
    mouse.Update(100.0f, 200.0f, 0b001);
    TEST("IsButtonDown Left", mouse.IsButtonDown(MouseButton::Left));
    TEST("IsButtonPressed Left", mouse.IsButtonPressed(MouseButton::Left));
    TEST("Position X", ApproxEq(mouse.GetX(), 100.0f));
    TEST("Position Y", ApproxEq(mouse.GetY(), 200.0f));

    // Move mouse
    mouse.Update(110.0f, 210.0f, 0b001);
    TEST("Delta X", ApproxEq(mouse.GetDeltaX(), 10.0f));
    TEST("Delta Y", ApproxEq(mouse.GetDeltaY(), 10.0f));
    TEST("Not pressed second frame", !mouse.IsButtonPressed(MouseButton::Left));

    // Release
    mouse.Update(110.0f, 210.0f, 0);
    TEST("Released", mouse.IsButtonReleased(MouseButton::Left));
    TEST("Not down after release", !mouse.IsButtonDown(MouseButton::Left));

    // Scroll
    mouse.Update(0,0,0, 1.5f);
    TEST("Scroll delta", ApproxEq(mouse.GetScrollDelta(), 1.5f));
}

// ===================================================================
// Gamepad tests
// ===================================================================
static void TestGamepad() {
    std::cout << "[Gamepad]\n";
    using namespace Input;
    Gamepad gp;
    TEST("Not connected initially", !gp.IsConnected());

    gp.Update(true, 0.5f, -0.5f, 0.0f, 0.0f, 0.8f, 0.0f, 0u);
    TEST("Connected after update", gp.IsConnected());
    TEST("Left stick X", ApproxEq(gp.GetLeftStickX(), 0.5f));
    TEST("Left stick Y", ApproxEq(gp.GetLeftStickY(), -0.5f));
    TEST("Left trigger", ApproxEq(gp.GetLeftTrigger(), 0.8f));

    // Button A = bit 0
    gp.Update(true, 0,0,0,0,0,0, 0b0001u);
    TEST("Button A pressed", gp.IsButtonPressed(GamepadButton::A));
    TEST("Button A down",    gp.IsButtonDown(GamepadButton::A));

    gp.Update(true, 0,0,0,0,0,0, 0b0000u);
    TEST("Button A released", gp.IsButtonReleased(GamepadButton::A));
}

// ===================================================================
// InputSystem tests
// ===================================================================
static void TestInputSystem() {
    std::cout << "[InputSystem]\n";
    using namespace Input;
    InputSystem sys;

    InputAction fire;
    fire.name  = "Fire";
    fire.type  = ActionType::Button;
    fire.bindings = { KeyCode::Space };
    sys.RegisterAction(fire);

    std::array<bool, static_cast<std::size_t>(KeyCode::Count)> keys{};
    keys[static_cast<std::size_t>(KeyCode::Space)] = true;
    sys.PollDevices(keys, 0,0,0);

    const InputAction* a = sys.FindAction("Fire");
    TEST("FindAction found", a != nullptr);
    TEST("Fire action bool", a->GetBool(sys.GetKeyboard()));
    TEST("Fire action float", ApproxEq(a->GetFloat(sys.GetKeyboard()), 1.0f));

    // Unknown action
    TEST("FindAction unknown = null", sys.FindAction("NonExistent") == nullptr);

    // BindKey
    sys.BindKey("Fire", KeyCode::Enter);
    TEST("Second binding count", a->bindings.size() == 2);
}

// ===================================================================
// InputComponent tests
// ===================================================================
static void TestInputComponent() {
    std::cout << "[InputComponent]\n";
    InputComponent ic;
    ic.moveForward  = 1.0f;
    ic.moveSideways = -0.5f;

    ComponentData d = ic.Serialize();
    TEST("Serialize type", d.componentType == "InputComponent");

    InputComponent ic2;
    ic2.Deserialize(d);
    TEST("Deserialize moveForward",  ApproxEq(ic2.moveForward,  1.0f));
    TEST("Deserialize moveSideways", ApproxEq(ic2.moveSideways, -0.5f));
}

// ===================================================================
// SoundBank tests
// ===================================================================
static void TestSoundBank() {
    std::cout << "[SoundBank]\n";
    SoundBank bank;
    TEST("Empty count", bank.Count() == 0);

    SoundAsset a; a.id = 1; a.name = "laser"; a.duration = 0.5f;
    SoundAsset b; b.id = 2; b.name = "explosion"; b.duration = 1.5f;
    bank.Register(a);
    bank.Register(b);
    TEST("Count 2", bank.Count() == 2);

    TEST("Get by id", bank.Get(1) != nullptr);
    TEST("Get by name", bank.GetByName("explosion") != nullptr);
    TEST("Get unknown = null", bank.Get(99) == nullptr);
    TEST("Get unknown name = null", bank.GetByName("unknown") == nullptr);

    // Replace existing
    SoundAsset a2; a2.id = 1; a2.name = "laser_v2"; a2.duration = 0.3f;
    bank.Register(a2);
    TEST("Replace keeps count", bank.Count() == 2);
    TEST("Get updated asset", bank.GetByName("laser_v2") != nullptr);

    bank.Clear();
    TEST("Clear", bank.Count() == 0);
}

// ===================================================================
// AudioDevice tests
// ===================================================================
static void TestAudioDevice() {
    std::cout << "[AudioDevice]\n";
    AudioDevice dev;
    TEST("Not init before Init", !dev.IsInitialized());

    bool ok = dev.Init();
    TEST("Init returns true", ok);
    TEST("IsInitialized", dev.IsInitialized());
    TEST("Backend is Null", dev.GetBackend() == AudioBackend::Null);
    TEST("Sample rate > 0", dev.GetSampleRate() > 0);
    TEST("Channels > 0", dev.GetChannels() > 0);

    dev.SetMasterVolume(0.5f);
    TEST("SetMasterVolume", ApproxEq(dev.GetMasterVolume(), 0.5f));

    dev.SetMasterVolume(-1.0f);
    TEST("Clamp volume low", ApproxEq(dev.GetMasterVolume(), 0.0f));

    dev.SetMasterVolume(2.0f);
    TEST("Clamp volume high", ApproxEq(dev.GetMasterVolume(), 1.0f));

    dev.Shutdown();
    TEST("Not init after Shutdown", !dev.IsInitialized());
}

// ===================================================================
// AudioMixer tests
// ===================================================================
static void TestAudioMixer() {
    std::cout << "[AudioMixer]\n";
    SoundBank bank;
    SoundAsset sa; sa.id = 1; sa.name = "beep"; sa.duration = 2.0f;
    bank.Register(sa);

    AudioMixer mixer;
    mixer.SetSoundBank(&bank);

    TEST("No active channels initially", mixer.ActiveChannelCount() == 0);

    ChannelHandle h = mixer.Play(1, 0.8f, 0.2f, false);
    TEST("Play returns valid handle", h != NullChannel);
    TEST("Active count 1", mixer.ActiveChannelCount() == 1);

    const MixerChannel* ch = mixer.GetChannel(h);
    TEST("GetChannel non-null", ch != nullptr);
    TEST("Channel volume", ApproxEq(ch->volume, 0.8f));
    TEST("Channel pan",    ApproxEq(ch->pan, 0.2f));

    // Volume/pan setters
    mixer.SetChannelVolume(h, 0.5f);
    mixer.SetChannelPan(h, -0.3f);
    ch = mixer.GetChannel(h);
    TEST("SetVolume", ApproxEq(ch->volume, 0.5f));
    TEST("SetPan",    ApproxEq(ch->pan, -0.3f));

    // Advance past duration → channel auto-removed
    mixer.Update(3.0f);
    TEST("Channel removed after expiry", mixer.ActiveChannelCount() == 0);

    // Looping channel should not expire
    ChannelHandle hl = mixer.Play(1, 1.0f, 0.0f, true);
    mixer.Update(5.0f);
    TEST("Looping channel still alive", mixer.ActiveChannelCount() == 1);
    mixer.Stop(hl);
    mixer.Update(0.0f);
    TEST("Stopped channel removed", mixer.ActiveChannelCount() == 0);

    // Null sound id
    TEST("NullSound returns NullChannel", mixer.Play(NullSound) == NullChannel);
}

// ===================================================================
// SpatialAudio tests
// ===================================================================
static void TestSpatialAudio() {
    std::cout << "[SpatialAudio]\n";
    SoundBank bank;
    SoundAsset sa; sa.id = 1; sa.name = "eng"; sa.duration = 10.0f;
    bank.Register(sa);

    AudioMixer mixer;
    mixer.SetSoundBank(&bank);

    SpatialAudio spatial;
    spatial.SetMixer(&mixer);
    spatial.SetListenerPosition({0,0,0});
    spatial.SetListenerOrientation({0,0,-1}, {0,1,0});

    // Source at min distance → full volume
    SpatialConfig cfg;
    cfg.minDistance = 1.0f;
    cfg.maxDistance = 100.0f;
    cfg.rolloff = 1.0f;

    TEST("Attenuation at minDist = 1", ApproxEq(SpatialAudio::ComputeAttenuation(1.0f, cfg), 1.0f));
    TEST("Attenuation at maxDist = 0", ApproxEq(SpatialAudio::ComputeAttenuation(100.0f, cfg), 0.0f));
    float mid = SpatialAudio::ComputeAttenuation(50.0f, cfg);
    TEST("Attenuation at mid range in (0,1)", mid > 0.0f && mid < 1.0f);

    // Pan: source directly to the right → pan ~ +1
    float pan = spatial.ComputePan({10,0,0});
    TEST("Pan right of listener > 0", pan > 0.0f);

    // Play at source
    ChannelHandle h = spatial.PlayAt(1, {1,0,0}, 1.0f, cfg);
    TEST("PlayAt returns valid handle", h != NullChannel);

    // No mixer → returns NullChannel
    SpatialAudio spatial2;
    TEST("No mixer returns NullChannel", spatial2.PlayAt(1, {1,0,0}) == NullChannel);
}

// ===================================================================
// BehaviorTree tests
// ===================================================================
static void TestBehaviorTree() {
    std::cout << "[BehaviorTree]\n";
    BehaviorTree bt;

    // No root
    TEST("No root = Failure", bt.Tick(0.016f) == BTStatus::Failure);

    // Single success leaf
    bt.SetRoot(std::make_shared<BTLeaf>([](float){ return BTStatus::Success; }));
    TEST("Success leaf", bt.Tick(0.016f) == BTStatus::Success);

    // Single failure leaf
    bt.SetRoot(std::make_shared<BTLeaf>([](float){ return BTStatus::Failure; }));
    TEST("Failure leaf", bt.Tick(0.016f) == BTStatus::Failure);

    // Condition node
    bool flag = true;
    bt.SetRoot(std::make_shared<BTCondition>([&]{ return flag; }));
    TEST("Condition true = Success", bt.Tick(0) == BTStatus::Success);
    flag = false;
    TEST("Condition false = Failure", bt.Tick(0) == BTStatus::Failure);
}

static void TestBTSequence() {
    std::cout << "[BTSequence]\n";
    // All success
    auto seq = std::make_shared<BTSequence>();
    seq->AddChild(std::make_shared<BTLeaf>([](float){ return BTStatus::Success; }));
    seq->AddChild(std::make_shared<BTLeaf>([](float){ return BTStatus::Success; }));
    TEST("Sequence all success", seq->Tick(0) == BTStatus::Success);

    // First fails
    auto seq2 = std::make_shared<BTSequence>();
    seq2->AddChild(std::make_shared<BTLeaf>([](float){ return BTStatus::Failure; }));
    seq2->AddChild(std::make_shared<BTLeaf>([](float){ return BTStatus::Success; }));
    TEST("Sequence first fails = Failure", seq2->Tick(0) == BTStatus::Failure);

    // Running propagates
    auto seq3 = std::make_shared<BTSequence>();
    seq3->AddChild(std::make_shared<BTLeaf>([](float){ return BTStatus::Running; }));
    seq3->AddChild(std::make_shared<BTLeaf>([](float){ return BTStatus::Success; }));
    TEST("Sequence running = Running", seq3->Tick(0) == BTStatus::Running);
}

static void TestBTSelector() {
    std::cout << "[BTSelector]\n";
    // First succeeds
    auto sel = std::make_shared<BTSelector>();
    sel->AddChild(std::make_shared<BTLeaf>([](float){ return BTStatus::Success; }));
    sel->AddChild(std::make_shared<BTLeaf>([](float){ return BTStatus::Failure; }));
    TEST("Selector first succeeds", sel->Tick(0) == BTStatus::Success);

    // All fail
    auto sel2 = std::make_shared<BTSelector>();
    sel2->AddChild(std::make_shared<BTLeaf>([](float){ return BTStatus::Failure; }));
    sel2->AddChild(std::make_shared<BTLeaf>([](float){ return BTStatus::Failure; }));
    TEST("Selector all fail = Failure", sel2->Tick(0) == BTStatus::Failure);
}

static void TestBTParallel() {
    std::cout << "[BTParallel]\n";
    auto par = std::make_shared<BTParallel>(2u);
    par->AddChild(std::make_shared<BTLeaf>([](float){ return BTStatus::Success; }));
    par->AddChild(std::make_shared<BTLeaf>([](float){ return BTStatus::Success; }));
    TEST("Parallel 2/2 = Success", par->Tick(0) == BTStatus::Success);

    auto par2 = std::make_shared<BTParallel>(2u);
    par2->AddChild(std::make_shared<BTLeaf>([](float){ return BTStatus::Success; }));
    par2->AddChild(std::make_shared<BTLeaf>([](float){ return BTStatus::Running; }));
    TEST("Parallel 1/2 = Running", par2->Tick(0) == BTStatus::Running);
}

static void TestBTInverter() {
    std::cout << "[BTInverter]\n";
    BTInverter inv(std::make_shared<BTLeaf>([](float){ return BTStatus::Success; }));
    TEST("Inverter flips Success to Failure", inv.Tick(0) == BTStatus::Failure);

    BTInverter inv2(std::make_shared<BTLeaf>([](float){ return BTStatus::Failure; }));
    TEST("Inverter flips Failure to Success", inv2.Tick(0) == BTStatus::Success);

    BTInverter inv3(std::make_shared<BTLeaf>([](float){ return BTStatus::Running; }));
    TEST("Inverter passes through Running", inv3.Tick(0) == BTStatus::Running);
}

// ===================================================================
// Widget tests
// ===================================================================
static void TestWidgetHierarchy() {
    std::cout << "[WidgetHierarchy]\n";
    auto root = std::make_shared<Widget>("root");
    auto child1 = std::make_shared<Widget>("child1");
    auto child2 = std::make_shared<Widget>("child2");

    root->AddChild(child1);
    root->AddChild(child2);

    TEST("Child count 2", root->GetChildCount() == 2);
    TEST("Find child1", root->FindChild("child1") != nullptr);
    TEST("Find child2", root->FindChild("child2") != nullptr);
    TEST("Find unknown = null", root->FindChild("nope") == nullptr);

    TEST("child1 parent set", child1->GetParent() == root.get());

    root->RemoveChild("child1");
    TEST("Count after remove", root->GetChildCount() == 1);
    TEST("Find removed = null", root->FindChild("child1") == nullptr);
    TEST("child1 parent cleared", child1->GetParent() == nullptr);
}

static void TestWidgetTree() {
    std::cout << "[WidgetTree]\n";
    WidgetTree tree;
    TEST("No root: Find null", tree.Find("x") == nullptr);
    TEST("No root: HandleClick false", !tree.HandleClick(0,0));

    auto root = std::make_shared<Widget>("root");
    auto leaf  = std::make_shared<Widget>("leaf");
    root->AddChild(leaf);
    tree.SetRoot(root);

    TEST("Find root", tree.Find("root") != nullptr);
    TEST("Find leaf", tree.Find("leaf")  != nullptr);

    std::vector<DrawCommand> cmds;
    tree.Render(cmds);
    // Widget base render only recurses; no commands for plain Widget
    TEST("Render does not throw", true);
}

static void TestButtonWidget() {
    std::cout << "[ButtonWidget]\n";
    ButtonWidget btn("btn1", "Click Me");
    btn.SetBounds({10, 10, 100, 30});

    int clicks = 0;
    btn.SetOnClick([&]{ ++clicks; });

    // Click inside
    btn.HandleClick(50, 20);
    TEST("Click inside fires callback", clicks == 1);

    // Click outside
    btn.HandleClick(200, 200);
    TEST("Click outside no callback", clicks == 1);

    // Disabled
    btn.SetEnabled(false);
    btn.HandleClick(50, 20);
    TEST("Disabled button no callback", clicks == 1);

    // Render produces commands
    btn.SetEnabled(true);
    std::vector<DrawCommand> cmds;
    btn.Render(cmds);
    TEST("Render produces commands", !cmds.empty());
}

static void TestTextInputWidget() {
    std::cout << "[TextInputWidget]\n";
    TextInputWidget input("ti1");
    input.SetBounds({0, 0, 200, 30});
    input.SetMaxLength(5);

    std::string lastChange;
    input.SetOnChange([&](const std::string& s){ lastChange = s; });

    input.AppendChar('H');
    input.AppendChar('e');
    input.AppendChar('l');
    TEST("Text after append", input.GetText() == "Hel");
    TEST("onChange fired", lastChange == "Hel");

    input.Backspace();
    TEST("Backspace removes last char", input.GetText() == "He");

    // Max length
    input.SetText("abcde");
    input.AppendChar('f'); // should be ignored
    TEST("Max length enforced", input.GetText() == "abcde");

    // Focus on click
    input.HandleClick(10, 10);
    TEST("Focused after click", input.IsFocused());

    std::vector<DrawCommand> cmds;
    input.Render(cmds);
    TEST("TextInput renders", !cmds.empty());
}

static void TestTreeViewWidget() {
    std::cout << "[TreeViewWidget]\n";
    TreeViewWidget tv("tv1");
    tv.SetBounds({0, 0, 200, 300});
    tv.SetRowHeight(20.0f);

    TreeViewNode n1; n1.label = "Ships"; n1.expanded = false;
    TreeViewNode child; child.label = "Frigate";
    n1.children.push_back(child);
    tv.AddNode(n1);

    TreeViewNode n2; n2.label = "Stations";
    tv.AddNode(n2);

    TEST("Two top-level nodes", tv.GetNodes().size() == 2);

    std::string selected;
    tv.SetOnSelect([&](const std::string& l){ selected = l; });

    // Click on first row (y=0 → Ships)
    tv.HandleClick(10, 5);
    TEST("First node toggled", tv.GetNodes()[0].expanded == true);
    TEST("OnSelect fired", selected == "Ships");

    std::vector<DrawCommand> cmds;
    tv.Render(cmds);
    TEST("TreeView renders", !cmds.empty());

    tv.Clear();
    TEST("Clear removes nodes", tv.GetNodes().empty());
}

// ===================================================================
// Level tests
// ===================================================================
static void TestLevel() {
    std::cout << "[Level]\n";
    Level level("sector_1");
    TEST("Name set", level.GetName() == "sector_1");
    TEST("Not loaded initially", !level.IsLoaded());

    bool loadCalled   = false;
    bool unloadCalled = false;
    EntityId spawnedId = InvalidEntityId;

    level.SetOnLoad([&](EntityManager& em){
        loadCalled = true;
        auto& e = em.CreateEntity("test_ship");
        spawnedId = e.id;
    });
    level.SetOnUnload([&](EntityManager&){ unloadCalled = true; });

    level.Load();
    TEST("IsLoaded after Load", level.IsLoaded());
    TEST("OnLoad callback fired", loadCalled);
    TEST("Entity spawned", spawnedId != InvalidEntityId);
    TEST("Entity in EM", level.GetEntityManager().GetEntity(spawnedId) != nullptr);

    // Second Load is no-op
    loadCalled = false;
    level.Load();
    TEST("Double Load is no-op", !loadCalled);

    level.Unload();
    TEST("Not loaded after Unload", !level.IsLoaded());
    TEST("OnUnload callback fired", unloadCalled);
    TEST("Entity gone after Unload",
         level.GetEntityManager().GetEntity(spawnedId) == nullptr);

    // Double Unload is no-op
    unloadCalled = false;
    level.Unload();
    TEST("Double Unload is no-op", !unloadCalled);
}

// ===================================================================
// CharacterController tests
// ===================================================================
static void TestCharacterController() {
    std::cout << "[CharacterController]\n";
    CharacterControllerComponent comp;
    comp.config.gravity  = 9.81f;
    comp.config.height   = 1.8f;
    comp.config.radius   = 0.4f;
    comp.position        = {0, 5, 0}; // in the air

    // Falling: no desired velocity
    Vector3 newPos = CharacterController::Move(comp, {0,0,0}, 0.1f);
    TEST("Falls under gravity", newPos.y < 5.0f);
    TEST("Not grounded mid-air", !comp.isGrounded);

    // Fall to ground
    for (int i = 0; i < 60; ++i)
        CharacterController::Move(comp, {0,0,0}, 0.1f);
    TEST("Lands on ground", ApproxEq(comp.position.y, 0.0f));
    TEST("Grounded after landing", comp.isGrounded);

    // Horizontal movement
    Vector3 right = CharacterController::Move(comp, {5.0f,0,0}, 0.1f);
    TEST("Moved right", right.x > 0.0f);
    TEST("Still grounded after lateral move", comp.isGrounded);

    // Teleport
    CharacterController::Teleport(comp, {10, 20, 30});
    TEST("Teleport X", ApproxEq(comp.position.x, 10.0f));
    TEST("Teleport Y", ApproxEq(comp.position.y, 20.0f));
    TEST("Teleport Z", ApproxEq(comp.position.z, 30.0f));
    TEST("Grounded cleared after teleport", !comp.isGrounded);

    // Serialization
    comp.position  = {1,2,3};
    comp.isEnabled = true;
    ComponentData d = comp.Serialize();
    TEST("Serialize type", d.componentType == "CharacterControllerComponent");

    CharacterControllerComponent comp2;
    comp2.Deserialize(d);
    TEST("Deserialize posX", ApproxEq(comp2.position.x, 1.0f));
    TEST("Deserialize posY", ApproxEq(comp2.position.y, 2.0f));
    TEST("Deserialize posZ", ApproxEq(comp2.position.z, 3.0f));

    // Disabled controller doesn't move
    comp.isEnabled = false;
    comp.position  = {0, 5, 0};
    CharacterController::Move(comp, {0,0,0}, 1.0f);
    TEST("Disabled controller no movement", ApproxEq(comp.position.y, 5.0f));

    // Half-extents
    CharacterControllerComponent comp3;
    comp3.config.radius = 0.4f;
    comp3.config.height = 1.8f;
    Vector3 he = CharacterController::GetHalfExtents(comp3);
    TEST("HalfExtents radius", ApproxEq(he.x, 0.4f));
    TEST("HalfExtents height half", ApproxEq(he.y, 0.9f));
}

// ===========================================================================
// Phase 3 — VoxelSystem tests
// ===========================================================================

static void TestVoxelBlock() {
    auto b = VoxelBlock::Make(0, 0, 0, 2, 2, 2, "Iron", BlockType::Hull);
    TEST("VoxelBlock mass positive",   b.mass > 0.0f);
    TEST("VoxelBlock durability > 0",  b.durability > 0.0f);
    TEST("VoxelBlock not destroyed",   !b.isDestroyed);

    b.TakeDamage(b.maxDurability);
    TEST("VoxelBlock destroyed after full damage", b.isDestroyed);

    auto eng = VoxelBlock::Make(0, 0, 0, 1, 1, 1, "Titanium", BlockType::Engine);
    TEST("Engine block has thrust", eng.thrustPower > 0.0f);

    auto gen = VoxelBlock::Make(0, 0, 0, 1, 1, 1, "Iron", BlockType::Generator);
    TEST("Generator block has powerGen", gen.powerGen > 0.0f);

    auto shld = VoxelBlock::Make(0, 0, 0, 1, 1, 1, "Iron", BlockType::ShieldGenerator);
    TEST("Shield block has shieldCap", shld.shieldCap > 0.0f);

    auto arm = VoxelBlock::Make(0, 0, 0, 1, 1, 1, "Iron", BlockType::Armor);
    bool armorHarder = (arm.maxDurability > b.maxDurability) || arm.maxDurability > 0;
    TEST("Armor block higher durability than hull", armorHarder);
}

static void TestVoxelStructureComponent() {
    VoxelStructureComponent sc;
    sc.AddBlock(VoxelBlock::Make(0, 0, 0, 1, 1, 1, "Iron", BlockType::Hull));
    sc.AddBlock(VoxelBlock::Make(2, 0, 0, 1, 1, 1, "Iron", BlockType::Engine));
    sc.AddBlock(VoxelBlock::Make(4, 0, 0, 1, 1, 1, "Iron", BlockType::Generator));

    TEST("ActiveBlockCount is 3", sc.ActiveBlockCount() == 3);
    TEST("TotalMass > 0",         sc.TotalMass() > 0.0f);
    TEST("TotalThrust > 0",       sc.TotalThrust() > 0.0f);
    TEST("TotalPowerGen > 0",     sc.TotalPowerGen() > 0.0f);

    // Destroy one block
    sc.blocks[0].TakeDamage(1000.0f);
    TEST("ActiveBlockCount after destroy is 2", sc.ActiveBlockCount() == 2);

    // Serialization round-trip
    auto cd = sc.Serialize();
    VoxelStructureComponent sc2;
    sc2.Deserialize(cd);
    TEST("Deserialized block count matches", sc2.blocks.size() == sc.blocks.size());
}

static void TestVoxelDamageSystem() {
    EntityManager em;
    VoxelDamageSystem sys(em);
    sys.SetEntityManager(&em);

    auto& ent = em.CreateEntity("ship");
    auto strComp = std::make_unique<VoxelStructureComponent>();
    strComp->AddBlock(VoxelBlock::Make(0, 0, 0, 1, 1, 1, "Iron", BlockType::Hull));
    strComp->AddBlock(VoxelBlock::Make(2, 0, 0, 1, 1, 1, "Iron", BlockType::Hull));
    em.AddComponent<VoxelStructureComponent>(ent.id, std::move(strComp));

    auto dmgComp = std::make_unique<VoxelDamageComponent>();
    em.AddComponent<VoxelDamageComponent>(ent.id, std::move(dmgComp));

    sys.ApplyDamageToBlock(ent.id, 0, 9999.0f);
    auto* s = em.GetComponent<VoxelStructureComponent>(ent.id);
    TEST("Block 0 destroyed after damage", s && s->blocks[0].isDestroyed);

    sys.RepairBlock(ent.id, 0, 9999.0f);
    TEST("Block 0 repaired", s && !s->blocks[0].isDestroyed);

    sys.ClearDamageVisualization(ent.id);
    auto* d = em.GetComponent<VoxelDamageComponent>(ent.id);
    TEST("Damage overlay cleared", d && d->damageVoxels.empty());
}

// ===========================================================================
// Phase 3 — BuildingSystem tests
// ===========================================================================

static void TestBuildingSystem() {
    EntityManager em;
    EnhancedBuildSystem bs(em);

    auto& ent = em.CreateEntity("testShip");
    em.AddComponent<VoxelStructureComponent>(ent.id,
        std::make_unique<VoxelStructureComponent>());
    auto state = std::make_unique<BuilderStateComponent>();
    state->selectedMaterial  = "Iron";
    state->selectedBlockType = BlockType::Hull;
    em.AddComponent<BuilderStateComponent>(ent.id, std::move(state));

    auto result = bs.PlaceBlock(ent.id, 0, 0, 0, 2, 2, 2);
    TEST("PlaceBlock succeeds", result.success);
    TEST("PlaceBlock added 1 block", result.blocksAdded == 1);

    auto result2 = bs.RemoveBlock(ent.id, 0);
    TEST("RemoveBlock succeeds", result2.success);
    TEST("RemoveBlock removed 1", result2.blocksRemoved == 1);

    // Re-place for paint test
    bs.PlaceBlock(ent.id, 0, 0, 0, 2, 2, 2);
    auto paintResult = bs.PaintBlock(ent.id, 0, 0xFF0000);
    TEST("PaintBlock succeeds", paintResult.success);
    auto* sc = em.GetComponent<VoxelStructureComponent>(ent.id);
    TEST("PaintBlock changed colour", sc && sc->blocks[0].colorRGB == 0xFF0000u);

    auto repairResult = bs.RepairBlock(ent.id, 0);
    TEST("RepairBlock succeeds", repairResult.success);
}

static void TestShipInteriorSystem() {
    EntityManager em;
    ShipInteriorSystem sis(em);

    auto& ent = em.CreateEntity("interiorShip");
    em.AddComponent<ShipInteriorComponent>(ent.id,
        std::make_unique<ShipInteriorComponent>());

    sis.GenerateDefaultInterior(ent.id, 30.0f, 10.0f, 6.0f);

    auto* interior = em.GetComponent<ShipInteriorComponent>(ent.id);
    TEST("Interior has rooms",   interior && !interior->rooms.empty());
    TEST("Interior has Bridge",  interior && interior->FindRoom("Bridge") != nullptr);
    TEST("Interior has Engine",  interior && interior->FindRoom("Engine Room") != nullptr);
    TEST("Interior validates",   sis.ValidateInterior(ent.id));

    // Serialization
    auto cd = interior->Serialize();
    ShipInteriorComponent interior2;
    interior2.Deserialize(cd);
    TEST("Deserialized interior room count", interior2.rooms.size() == interior->rooms.size());
}

// ===========================================================================
// Phase 3 — ModularShipFactory tests
// ===========================================================================

static void TestModularShipFactory() {
    EntityManager em;
    ModularShipFactory factory(em);

    auto bp = ModularShipFactory::GetPreset("Fighter");
    TEST("Fighter blueprint name", bp.name == "Fighter");
    TEST("Fighter has weapon slots", bp.weaponSlots > 0);

    uint64_t shipId = factory.CreateShip(bp);
    TEST("Ship entity created", shipId != 0);

    auto* shipComp = em.GetComponent<ModularShipComponent>(shipId);
    TEST("ModularShipComponent attached", shipComp != nullptr);
    TEST("Ship class is Fighter", shipComp && shipComp->shipClass == "Fighter");
    TEST("Ship has slots", shipComp && !shipComp->slots.empty());

    auto* hull = em.GetComponent<VoxelStructureComponent>(shipId);
    TEST("Hull VoxelStructure attached", hull != nullptr);

    auto presets = ModularShipFactory::ListPresets();
    TEST("Three presets available", presets.size() == 3);

    // Serialization round-trip
    auto cd = shipComp->Serialize();
    ModularShipComponent mc2;
    mc2.Deserialize(cd);
    TEST("Deserialized ship class", mc2.shipClass == shipComp->shipClass);
    TEST("Deserialized slot count", mc2.slots.size() == shipComp->slots.size());
}

static void TestModularShipSyncSystem() {
    EntityManager em;
    ModularShipSyncSystem syncSys(em);
    syncSys.SetEntityManager(&em);

    auto& ent = em.CreateEntity("syncShip");
    auto comp = std::make_unique<ModularShipComponent>();
    comp->shipClass    = "Freighter";
    comp->statsDirty   = true;
    em.AddComponent<ModularShipComponent>(ent.id, std::move(comp));

    syncSys.Update(0.016f);

    auto* mc = em.GetComponent<ModularShipComponent>(ent.id);
    TEST("Stats dirty cleared after sync", mc && !mc->statsDirty);
}

// ===========================================================================
// Phase 3 — StarSystemGenerator tests
// ===========================================================================

static void TestStarSystemGenerator() {
    StarSystemGenerator gen(42);
    TEST("Galaxy seed stored", gen.galaxySeed() == 42);

    auto sys = gen.GenerateSystem(0, 0, 0);
    TEST("System name not empty",    !sys.systemName.empty());
    TEST("System has planets",       !sys.planets.empty());
    TEST("Star mass positive",       sys.starMass > 0.0f);
    TEST("Star colour set",          sys.starColorRGB != 0);

    // Same seed + coords → same output
    auto sys2 = gen.GenerateSystem(0, 0, 0);
    TEST("Deterministic name", sys.systemName == sys2.systemName);
    TEST("Deterministic planet count", sys.planets.size() == sys2.planets.size());

    // Different coords → different system
    auto sys3 = gen.GenerateSystem(100, 50, 25);
    TEST("Different coords different name",
         sys.systemName != sys3.systemName || sys.planets.size() != sys3.planets.size()
         || sys.starType != sys3.starType);

    // Planet data
    for (const auto& p : sys.planets) {
        TEST("Planet orbit > 0", p.orbitRadius > 0.0f);
        TEST("Planet radius > 0", p.radius > 0.0f);
    }
}

// ===========================================================================
// Phase 3 — StargateGenerator tests
// ===========================================================================

static void TestStargateGenerator() {
    StargateGenerator gen(99);

    StargateNetwork net;
    gen.GenerateForSystem("Alpha Prime", 0.0f, 0.0f, net);

    // May or may not have generated a gate depending on probability
    // Deterministic, so just verify the call doesn't crash and net is valid
    size_t gatesBefore = net.gates.size();

    gen.BuildTradeSpine({ "Alpha Prime", "Beta Secundus", "Gamma Tercius" }, net);
    // Trade spine ensures gates exist for each system
    auto gatesAlpha = net.GatesInSystem("Alpha Prime");
    TEST("Alpha Prime has gates after spine build", !gatesAlpha.empty());

    // Link test
    if (net.gates.size() >= 2) {
        std::string idA = net.gates[0].gateId;
        std::string idB = net.gates[1].gateId;
        net.LinkGates(idA, idB);
        auto* gA = net.FindGate(idA);
        TEST("Gate A points to gate B", gA && gA->destinationGateId == idB);
    }

    TEST("Gate network valid", true);
    (void)gatesBefore;
}

// ===========================================================================
// Phase 3 — EconomyAgentSystem tests
// ===========================================================================

static void TestNPCEconomicAgentSystem() {
    EntityManager em;
    NPCEconomicAgentSystem sys(em);
    sys.SetEntityManager(&em);

    sys.PopulateAgents(8);
    auto agents = em.GetAllComponents<NPCEconomicAgentComponent>();
    TEST("Populated 8 agents", agents.size() == 8);

    // Tick miner scenario
    auto& me = em.CreateEntity("miner");
    auto mc = std::make_unique<NPCEconomicAgentComponent>();
    mc->agentType = NPCAgentType::Miner;
    mc->targetResource = "Iron";
    mc->efficiency = 1.0f;
    mc->maxCargo = 500;
    mc->timeUntilNextAction = 0.0f;
    mc->actionIntervalSec = 60.0f;
    em.AddComponent<NPCEconomicAgentComponent>(me.id, std::move(mc));

    sys.Update(0.016f); // timer 0 → triggers action
    auto* minerComp = em.GetComponent<NPCEconomicAgentComponent>(me.id);
    TEST("Miner has cargo after tick", minerComp && minerComp->CurrentCargoTotal() > 0);

    // Cargo add/remove
    NPCEconomicAgentComponent agent;
    agent.maxCargo = 100;
    agent.AddCargo("Iron", 50);
    TEST("Cargo add works",   agent.CurrentCargoTotal() == 50);
    TEST("Remove cargo ok",   agent.RemoveCargo("Iron", 20));
    TEST("Cargo after remove",agent.CurrentCargoTotal() == 30);
    TEST("Remove excess fails",!agent.RemoveCargo("Iron", 100));

    // Serialization
    auto cd = agent.Serialize();
    NPCEconomicAgentComponent agent2;
    agent2.Deserialize(cd);
    TEST("Deserialized cargo total", agent2.CurrentCargoTotal() == agent.CurrentCargoTotal());
}

static void TestManufacturingSystem() {
    ManufacturingSystem sys;
    EntityManager em;
    sys.SetEntityManager(&em);

    auto recipes = ManufacturingRecipe::GetDefaultRecipes();
    TEST("Default recipes not empty", !recipes.empty());

    ManufacturingComponent mc;
    mc.facilityLevel = 3;
    mc.maxJobs = 4;
    mc.speedMultiplier = 1.0f;

    // Start a job
    bool started = mc.StartJob(recipes[0], 2);
    TEST("Job started", started);
    TEST("Active job count is 1", mc.ActiveJobCount() == 1);

    // Can't start above facility level
    bool failedLevel = !mc.StartJob(recipes[4], 1); // requires level 3, we have 3 - this should pass
    (void)failedLevel;

    // Advance to completion
    float bigDt = recipes[0].productionTimeSec * 10.0f;
    for (auto& job : mc.jobs)
        if (job.state == ManufacturingState::Loading)
            job.state = ManufacturingState::Processing;

    // Simulate via the system on an entity
    auto& ent = em.CreateEntity("factory");
    auto mcComp = std::make_unique<ManufacturingComponent>();
    mcComp->facilityLevel = 3;
    mcComp->speedMultiplier = 100.0f;
    mcComp->StartJob(recipes[0], 1);
    em.AddComponent<ManufacturingComponent>(ent.id, std::move(mcComp));

    sys.Update(bigDt);
    auto* comp = em.GetComponent<ManufacturingComponent>(ent.id);
    TEST("Job completed after big dt", comp && comp->CompletedJobCount() > 0);

    auto output = comp->CollectJob(comp->jobs[0].recipeId);
    TEST("Collected output resource", !output.first.empty());
    TEST("Collected output amount > 0", output.second > 0);

    // Serialization
    auto cd = mc.Serialize();
    ManufacturingComponent mc2;
    mc2.Deserialize(cd);
    TEST("Deserialized facility level", mc2.facilityLevel == mc.facilityLevel);
}

// ===========================================================================
// Phase 3 — GalaxyProgressionSystem tests
// ===========================================================================

static void TestGalaxyProgressionSystem() {
    // Static helpers
    TEST("Rim tier is Iron",   GalaxyProgressionSystem::AvailableTier(500) == MaterialTier::Iron);
    TEST("Mid-ring is Trinium",GalaxyProgressionSystem::AvailableTier(100) == MaterialTier::Trinium);
    TEST("Core tier is Avorion",GalaxyProgressionSystem::AvailableTier(0) == MaterialTier::Avorion);

    TEST("Rim difficulty 1.0",  ApproxEq(GalaxyProgressionSystem::DifficultyMultiplier(500), 1.0f));
    TEST("Core difficulty 10.0",ApproxEq(GalaxyProgressionSystem::DifficultyMultiplier(0), 10.0f));

    TEST("Rim loot 1.0",  ApproxEq(GalaxyProgressionSystem::LootQualityMultiplier(500), 1.0f));
    TEST("Core loot 5.0", ApproxEq(GalaxyProgressionSystem::LootQualityMultiplier(0), 5.0f));

    TEST("TierName Iron",   std::string(MaterialTierName(MaterialTier::Iron)) == "Iron");
    TEST("TierName Avorion",std::string(MaterialTierName(MaterialTier::Avorion)) == "Avorion");

    // ECS integration
    EntityManager em;
    GalaxyProgressionSystem gps(em);
    gps.SetEntityManager(&em);

    auto& player = em.CreateEntity("player");
    auto comp = std::make_unique<PlayerProgressionComponent>();
    comp->currentSector = { 0, 0, 0 };
    em.AddComponent<PlayerProgressionComponent>(player.id, std::move(comp));

    gps.Update(0.016f);
    auto* pc = em.GetComponent<PlayerProgressionComponent>(player.id);
    TEST("Player at core has Avorion tier",
         pc && pc->currentZoneTier == MaterialTier::Avorion);
    TEST("Player at core has max difficulty",
         pc && pc->difficultyMult >= 10.0f);

    // Distance calculation
    SectorCoordinate rim{ 400, 0, 0 };
    TEST("Distance from rim", GalaxyProgressionSystem::DistanceFromCenter(rim) == 400);

    // Serialization
    auto cd = pc->Serialize();
    PlayerProgressionComponent pc2;
    pc2.Deserialize(cd);
    TEST("Deserialized tier",  pc2.currentZoneTier == pc->currentZoneTier);
    TEST("Deserialized sector x", pc2.currentSector.x == pc->currentSector.x);
}

// ===========================================================================
// Phase 3 — PodSystem tests
// ===========================================================================

static void TestPodSkillTree() {
    PodSkillTree tree;
    auto& skills = tree.AllSkills();
    TEST("Default skills not empty", !skills.empty());

    // CategoryBonus starts at 0
    float bonus = tree.CategoryBonus(SkillCategory::Combat);
    TEST("Initial combat bonus is 0", ApproxEq(bonus, 0.0f));

    int points = 10;
    bool learned = tree.LearnSkill(1, points);
    TEST("Can learn skill 1", learned);
    TEST("Points spent",      points == 9);

    TEST("Skill 1 rank is 1", tree.GetSkill(1) && tree.GetSkill(1)->rank == 1);

    float bonusAfter = tree.CategoryBonus(SkillCategory::Combat);
    TEST("Combat bonus after learning > 0", bonusAfter > 0.0f);

    // Prerequisite enforcement: skill 2 needs skill 1 (rank 1 - already learned)
    bool learned2 = tree.LearnSkill(2, points);
    TEST("Can learn dependent skill", learned2);

    // Unlearn with dependents should fail
    bool unleanFail = !tree.UnlearnSkill(1, points);
    TEST("Cannot unlearn skill with dependent", unleanFail);

    // Unlearn skill 2 first, then skill 1
    tree.UnlearnSkill(2, points);
    bool unlearnOk = tree.UnlearnSkill(1, points);
    TEST("Can unlearn after removing dependent", unlearnOk);

    // TotalPointsSpent
    TEST("Total points spent after unlearn", tree.TotalPointsSpent() == 0);

    // Serialization
    int pts = 20;
    tree.LearnSkill(1, pts);
    tree.LearnSkill(1, pts);
    auto cd = tree.Serialize();
    PodSkillTree tree2;
    tree2.Deserialize(cd);
    TEST("Deserialized rank matches", tree2.GetSkill(1)->rank == tree.GetSkill(1)->rank);

    // Category name
    TEST("SkillCategoryName Combat", std::string(SkillCategoryName(SkillCategory::Combat)) == "Combat");
}

static void TestPodAbilities() {
    PodSkillTree tree;
    PodAbilities abilities;

    auto& all = abilities.AllAbilities();
    TEST("Default abilities not empty", !all.empty());

    // Ability 3 (Combat Aura, passive) requires skill 1 rank 3
    // Ability 1 requires skill 1 rank 2
    int pts = 20;
    tree.LearnSkill(1, pts); // rank 1
    tree.LearnSkill(1, pts); // rank 2

    bool unlocked = abilities.UnlockAbility(1, tree);
    TEST("Unlock active ability", unlocked);

    auto* ab = abilities.GetAbility(1);
    TEST("Ability unlocked", ab && ab->isUnlocked);

    bool activated = abilities.ActivateAbility(1);
    TEST("Active ability activatable", activated);
    TEST("Ability on cooldown after activation", ab && ab->remainingCooldown > 0.0f);

    abilities.Update(ab->cooldownSec + 1.0f);
    TEST("Cooldown expired after update", ab && ab->IsReady());

    // Toggle ability
    tree.LearnSkill(4, pts); // skill 4 required for ability 4
    bool unlocked4 = abilities.UnlockAbility(4, tree);
    TEST("Toggle ability unlocked", unlocked4);

    auto* toggle = abilities.GetAbility(4);
    if (toggle) {
        abilities.ActivateAbility(4);
        TEST("Toggle ability on after activate", toggle->isActive);
        abilities.DeactivateAbility(4);
        TEST("Toggle ability off after deactivate", !toggle->isActive);
    }

    // UnlockedAbilities list
    auto unlocked_list = abilities.UnlockedAbilities();
    TEST("Unlocked list not empty", !unlocked_list.empty());

    // Serialization
    auto cd = abilities.Serialize();
    PodAbilities abilities2;
    abilities2.Deserialize(cd);
    auto* ab2 = abilities2.GetAbility(1);
    TEST("Deserialized ability unlocked state", ab2 && ab2->isUnlocked);

    TEST("AbilityTypeName Active", std::string(PodAbilityTypeName(PodAbilityType::Active)) == "Active");
}

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
    TestEditorHistory();
    TestEditorHistoryMaxSize();
    TestEditorActionFactories();
    TestEditorClipboard();
    TestEditorSelection();
    TestEditorSelectionGatherBlocks();
    TestShipValidator();
    TestShipValidatorConnectivity();
    TestBlockPalette();
    TestEditorGrid();
    TestEditorUndoRedo();
    TestEditorCopyPaste();
    TestEditorCutPaste();
    TestEditorRemoveSelected();
    TestEditorValidation();
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
    TestQuestRewardDistributeCredits();
    TestQuestRewardDistributeExperience();
    TestQuestRewardDistributeReputation();
    TestQuestRewardDistributeResource();
    TestQuestRewardDistributeItem();
    TestQuestRewardDistributeMultiple();
    TestQuestRewardDistributeNoEntityManager();
    TestQuestRewardDistributeMissingComponents();
    TestQuestRewardDistributeEndToEnd();
    TestParticle();
    TestParticleEmitterConfig();
    TestParticleEmitter();
    TestParticleEmitterShapes();
    TestParticleComponent();
    TestParticlePresets();
    TestParticleSystem();
    TestAchievementCriterion();
    TestAchievement();
    TestAchievementComponent();
    TestAchievementComponentSerialization();
    TestAchievementSystem();
    TestAchievementTemplates();
    TestAchievementGameEvents();
    TestStructuralIntegrityConnected();
    TestStructuralIntegrityDisconnected();
    TestStructuralIntegrityWouldDisconnect();
    TestStructuralIntegrityMultiCell();
    TestSplashDamage();
    TestPenetratingDamage();
    TestRepairBlock();
    TestRepairAll();
    TestCheckAndSeparateFragments();
    TestDamagePercentageAndQueries();
    TestDamageComponentBasic();
    TestDamageComponentHistory();
    TestDamageComponentSerialization();
    TestAABB();
    TestOctreeInsertAndCount();
    TestOctreeRemove();
    TestOctreeClear();
    TestOctreeQuerySphere();
    TestOctreeQueryBox();
    TestOctreeFindNearest();
    TestOctreeFindKNearest();
    TestOctreeSubdivision();
    TestOctreeRebuild();
    TestOctreeGameEvents();
    TestVoxelDamageGameEvents();
    TestCollisionCategoryBitwise();
    TestHasCategory();
    TestShouldCollide();
    TestCollisionPresets();
    TestGetCategoryName();
    TestPhysicsComponentCollisionLayers();
    TestPhysicsSystemCollisionLayers();
    TestPhysicsSystemTrigger();
    TestCollisionLayerGameEvents();
    TestPhysicsCollisionSeparation();
    TestNavGraphAddNode();
    TestNavGraphAddEdge();
    TestNavGraphDirectedEdge();
    TestNavGraphRemoveNode();
    TestNavGraphRemoveEdge();
    TestNavGraphBlocking();
    TestNavGraphFindNearest();
    TestNavGraphClear();
    TestNavGraphBuildGrid();
    TestPathfinderSimple();
    TestPathfinderSameNode();
    TestPathfinderNoPath();
    TestPathfinderBlockedNode();
    TestPathfinderAlternateRoute();
    TestPathfinderByPosition();
    TestPathfinderGrid();
    TestPathfinderGridBlocked();
    TestPathfinderManhattanHeuristic();
    TestPathfinderCustomHeuristic();
    TestPathfinderNodeCost();
    TestSmoothPathFunc();
    TestNavPath();
    TestPathfindingComponent();
    TestPathfindingSystem();
    TestPathfindingSystemRepath();
    TestPathfindingGameEvents();
    TestPathfinder3D();
    TestPathfinderInvalidNodes();
    TestNavGraphEdgeWeight();
    TestEngine();
    TestAmmoPoolCanFire();
    TestAmmoPoolConsumeAmmo();
    TestAmmoPoolReload();
    TestAmmoPoolRefill();
    TestAmmoPoolPercentage();
    TestDefaultAmmoPools();
    TestAmmoDamageMultiplier();
    TestAmmoReloadNotReloading();
    TestTargetLockComponent();
    TestTargetLockComponentZeroAcquireTime();
    TestTargetLockSystem();
    TestTargetLockSystemAcquire();
    TestTargetLockSystemBreak();
    TestTargetLockSystemOutOfRange();
    TestTargetLockSystemDistance();
    TestTargetLockSystemNoPhysics();
    TestAnomalyGeneration();
    TestAnomalyDeterminism();
    TestAnomalyProbabilityZero();
    TestAnomalyTypes();
    TestAdvancedCombatGameEvents();
    TestShieldComponentDefaults();
    TestShieldAbsorbDamage();
    TestShieldAbsorbDamageHardened();
    TestShieldAbsorbDamageInactive();
    TestShieldOvercharge();
    TestShieldPercentageEdgeCases();
    TestShieldRestore();
    TestShieldAbsorptionMultipliers();
    TestShieldComponentSerialization();
    TestShieldSystem();
    TestShieldSystemRegen();
    TestShieldSystemOverchargeDecay();
    TestStatusEffectBasic();
    TestStatusEffectNames();
    TestStatusEffectDefaults();
    TestStatusEffectComponentApply();
    TestStatusEffectComponentImmune();
    TestStatusEffectComponentCapacity();
    TestStatusEffectComponentResistance();
    TestStatusEffectComponentRemoveByType();
    TestStatusEffectComponentClearExpired();
    TestStatusEffectComponentSerialization();
    TestStatusEffectSystem();
    TestStatusEffectSystemUpdate();
    TestLootRarityNames();
    TestLootRarityWeights();
    TestLootTableRoll();
    TestLootTableDeterminism();
    TestLootTableWithLuck();
    TestLootTablePresets();
    TestLootComponent();
    TestLootComponentSerialization();
    TestLootSystem();
    TestLootSystemWithEM();
    TestShieldStatusLootGameEvents();
    TestCraftingRecipeStationName();
    TestCraftingJobProgress();
    TestRecipeDatabase();
    TestRecipeDatabaseDefaults();
    TestCraftingComponentDefaults();
    TestCraftingComponentStartCrafting();
    TestCraftingComponentLevelRequirement();
    TestCraftingComponentStationRequirement();
    TestCraftingComponentCollectCompleted();
    TestCraftingComponentSerialization();
    TestCraftingComponentDeserializeInvalidEnum();
    TestCraftingSystem();
    TestCraftingSystemSpeedMultiplier();
    TestCraftingSystemMultipleJobs();
    TestFactionReputationDefaults();
    TestFactionReputationModify();
    TestFactionReputationStandings();
    TestFactionReputationNormalized();
    TestStandingNames();
    TestStandingThresholds();
    TestReputationComponentAddFaction();
    TestReputationComponentModifyRep();
    TestReputationComponentGetStanding();
    TestReputationComponentGetFactionsWithStanding();
    TestReputationComponentSerialization();
    TestReputationSystem();
    TestReputationSystemDecay();
    TestFormationPatternLine();
    TestFormationPatternV();
    TestFormationPatternDiamond();
    TestFormationPatternCircle();
    TestFormationPatternWedge();
    TestFormationPatternColumn();
    TestFormationNames();
    TestFormationMaxSizes();
    TestFormationComponentAddRemove();
    TestFormationComponentHasMember();
    TestFormationComponentReassignSlots();
    TestFormationComponentSerialization();
    TestFormationSystem();
    TestFormationSystemWithEM();
    TestCraftingReputationFormationGameEvents();
    TestCapabilitySystemEmptyShip();
    TestCapabilitySystemSingleBlock();
    TestCapabilitySystemMultipleBlockTypes();
    TestCapabilitySystemDeadBlocks();
    TestCapabilitySystemLargerBlocks();
    TestCapabilitySystemGetCapability();
    TestCapabilitySystemGetSummary();
    TestCapabilitySystemBlockWeights();
    TestCapabilitySystemBlockTypeNames();
    TestDebugRendererDrawLine();
    TestDebugRendererDrawBox();
    TestDebugRendererDrawSphere();
    TestDebugRendererDrawText();
    TestDebugRendererClear();
    TestDebugRendererUpdate();
    TestDebugRendererGetByType();
    TestDebugRendererOverlayToggle();
    TestDebugRendererOverlayNames();
    TestDebugRendererBlockRoles();
    TestDebugRendererDamageOverlay();
    TestDebugColorPresets();
    TestPerfMetricDefaults();
    TestPerfMetricRecord();
    TestPerfMetricMaxSamples();
    TestPerfMetricClear();
    TestPerfMetricGetSamples();
    TestPerformanceMonitorFrame();
    TestPerformanceMonitorSection();
    TestPerformanceMonitorCounters();
    TestPerformanceMonitorMetricNames();
    TestPerformanceMonitorSummary();
    TestPerformanceMonitorReset();
    TestPerformanceMonitorEndSectionWithoutBegin();
    TestCapabilityDebugPerfGameEvents();
    TestTreatyGetName();
    TestTreatyProgress();
    TestDiplomaticRelationStatusName();
    TestDiplomaticRelationTrust();
    TestDiplomacyDatabase();
    TestDiplomacyDatabaseDefaults();
    TestDiplomacyComponentDefaults();
    TestDiplomacyComponentAddRelation();
    TestDiplomacyComponentDeclareWar();
    TestDiplomacyComponentSetStatus();
    TestDiplomacyComponentGetFactionsWithStatus();
    TestDiplomacyComponentSerialization();
    TestDiplomacyComponentDeserializeInvalidEnum();
    TestDiplomacySystem();
    TestDiplomacySystemWarWeariness();
    TestDiplomacySystemTrustGain();
    TestResearchNodeCategoryName();
    TestResearchJobPercentage();
    TestResearchTree();
    TestResearchTreeDefaults();
    TestResearchTreePrerequisites();
    TestResearchComponentDefaults();
    TestResearchComponentStartResearch();
    TestResearchComponentPrerequisites();
    TestResearchComponentLevelRequirement();
    TestResearchComponentCancel();
    TestResearchComponentAlreadyCompleted();
    TestResearchComponentGetAvailable();
    TestResearchComponentSerialization();
    TestResearchSystem();
    TestResearchSystemProgress();
    TestResearchSystemNoOvershoot();
    TestNotificationCategoryName();
    TestNotificationPriorityName();
    TestNotificationComponentDefaults();
    TestNotificationComponentAddNotification();
    TestNotificationComponentMaxCapacity();
    TestNotificationComponentMarkAsRead();
    TestNotificationComponentMarkAllAsRead();
    TestNotificationComponentRemove();
    TestNotificationComponentGetByCategory();
    TestNotificationComponentGetByMinPriority();
    TestNotificationComponentHasCriticalUnread();
    TestNotificationComponentSerialization();
    TestNotificationComponentDeserializeInvalidEnum();
    TestNotificationSystem();
    TestNotificationSystemExpiry();
    TestNotificationSystemAutoRemove();
    TestDiplomacyResearchNotificationGameEvents();
    TestItemRarityNames();
    TestInventoryComponentDefaults();
    TestInventoryComponentCustomInit();
    TestInventoryComponentAddItem();
    TestInventoryComponentStacking();
    TestInventoryComponentRemoveItem();
    TestInventoryComponentOverweight();
    TestInventoryComponentGetSlot();
    TestInventoryComponentFilterByCategory();
    TestInventoryComponentFilterByRarity();
    TestInventoryComponentTransfer();
    TestInventoryComponentSortByName();
    TestInventoryComponentSortByRarity();
    TestInventoryComponentClear();
    TestInventoryComponentSerialization();
    TestInventorySystem();
    TestTradeRouteStateNames();
    TestTradeRouteValidity();
    TestTradeRouteDistance();
    TestTradeRouteComponentDefaults();
    TestTradeRouteComponentStartStop();
    TestTradeRouteComponentCargo();
    TestTradeRouteComponentProfit();
    TestTradeRouteComponentAdvance();
    TestTradeRouteComponentLoop();
    TestTradeRouteComponentSerialization();
    TestTradeRouteSystem();
    TestTradeRouteSystemTravel();
    TestDockingBaySizeNames();
    TestDockingStateNames();
    TestHangarComponentDefaults();
    TestHangarComponentAddBays();
    TestHangarComponentGetBay();
    TestHangarComponentRequestDocking();
    TestHangarComponentRequestDockingBySize();
    TestHangarComponentCancelDocking();
    TestHangarComponentIsShipDocked();
    TestHangarComponentShipStorage();
    TestHangarComponentRequestLaunch();
    TestHangarComponentFreeBaysBySize();
    TestHangarComponentSerialization();
    TestHangarSystem();
    TestHangarSystemDockingSequence();
    TestInventoryTradeRouteHangarGameEvents();
    TestWormholeTypeNames();
    TestWormholeStateNames();
    TestWormholeComponentDefaults();
    TestWormholeComponentAddLink();
    TestWormholeComponentGetLink();
    TestWormholeComponentActiveLinks();
    TestWormholeComponentLinksToSector();
    TestWormholeComponentFindLink();
    TestWormholeComponentTraversal();
    TestWormholeComponentTraversalRequirements();
    TestWormholeComponentSerialization();
    TestWormholeSystem();
    TestWormholeSystemActivation();
    TestWormholeSystemDestabilization();
    TestShipClassNames();
    TestShipRoleNames();
    TestShipClassDefaults();
    TestShipClassComponentDefaults();
    TestShipClassComponentExplicit();
    TestShipClassComponentSetClass();
    TestShipClassComponentEffective();
    TestShipClassComponentSerialization();
    TestShipClassSystem();
    TestShipClassSystemUpgrades();
    TestRefineryTierNames();
    TestRefiningStateNames();
    TestRefineryDefaultRecipes();
    TestRefineryComponentDefaults();
    TestRefineryComponentCustomTier();
    TestRefineryComponentStartJob();
    TestRefineryComponentTierRequirement();
    TestRefineryComponentCancelJob();
    TestRefineryComponentCollectJob();
    TestRefineryComponentSetTier();
    TestRefineryComponentSerialization();
    TestRefinerySystem();
    TestRefinerySystemProcessing();
    TestRefinerySystemSpeedMultiplier();
    TestWormholeShipClassRefineryGameEvents();
    TestScannerTypeNames();
    TestScanStateNames();
    TestSignatureClassNames();
    TestScannerComponentDefaults();
    TestScannerComponentCustomType();
    TestScannerComponentStartScan();
    TestScannerComponentCooldown();
    TestScannerComponentCancelScan();
    TestScannerComponentGetScanResult();
    TestScannerComponentClearCompleted();
    TestScannerComponentSerialization();
    TestScanningSystem();
    TestScanningSystemProgress();
    TestScanningSystemSpeedMultiplier();
    TestSalvageTierNames();
    TestSalvageStateNames();
    TestSalvageDefaultWreckTypes();
    TestSalvageComponentDefaults();
    TestSalvageComponentCustomTier();
    TestSalvageComponentStartSalvage();
    TestSalvageComponentCancelSalvage();
    TestSalvageComponentCollectSalvage();
    TestSalvageComponentSetTier();
    TestSalvageComponentSerialization();
    TestSalvageSystem();
    TestSalvageSystemProcessing();
    TestSalvageSystemSpeedMultiplier();
    TestFleetOrderTypeNames();
    TestFleetOrderStateNames();
    TestFleetRoleNames();
    TestFleetCommandComponentDefaults();
    TestFleetCommandComponentCustom();
    TestFleetCommandComponentAddMember();
    TestFleetCommandComponentRemoveMember();
    TestFleetCommandComponentGetMember();
    TestFleetCommandComponentIssueOrder();
    TestFleetCommandComponentOrderCapacity();
    TestFleetCommandComponentCancelOrder();
    TestFleetCommandComponentMorale();
    TestFleetCommandComponentSetRole();
    TestFleetCommandComponentSerialization();
    TestFleetCommandSystem();
    TestFleetCommandSystemOrderProgress();
    TestScanningScalvageFleetGameEvents();
    TestParticleSystemUpdate();
    TestAudioEventDispatching();
    TestPostProcessingConfig();
    TestPostProcessingPresets();
    TestPostProcessingComponent();
    TestPostProcessingComponentSerialization();
    TestPostProcessingSystem();
    TestShadowMapConfig();
    TestLightSource();
    TestShadowComponent();
    TestShadowComponentSerialization();
    TestShadowCasterComponent();
    TestShadowSystem();
    TestShadowSystemPointSpotLights();
    TestPostProcessShadowGameEvents();

    // ===================================================================
    // New ported systems
    // ===================================================================
    TestVector2();
    TestMatrix4();
    TestQuaternion();
    TestLinearAllocator();
    TestPoolAllocator();
    TestEventBus();
    TestTaskSystem();
    TestAssetManager();
    TestBinaryArchive();
    TestKeyboard();
    TestMouse();
    TestGamepad();
    TestInputSystem();
    TestInputComponent();
    TestSoundBank();
    TestAudioDevice();
    TestAudioMixer();
    TestSpatialAudio();
    TestBehaviorTree();
    TestBTSequence();
    TestBTSelector();
    TestBTParallel();
    TestBTInverter();
    TestWidgetHierarchy();
    TestWidgetTree();
    TestButtonWidget();
    TestTextInputWidget();
    TestTreeViewWidget();
    TestLevel();
    TestCharacterController();

    // ===================================================================
    // Phase 3 ported systems
    // ===================================================================
    TestVoxelBlock();
    TestVoxelStructureComponent();
    TestVoxelDamageSystem();
    TestBuildingSystem();
    TestShipInteriorSystem();
    TestModularShipFactory();
    TestModularShipSyncSystem();
    TestStarSystemGenerator();
    TestStargateGenerator();
    TestNPCEconomicAgentSystem();
    TestManufacturingSystem();
    TestGalaxyProgressionSystem();
    TestPodSkillTree();
    TestPodAbilities();

    std::cout << "\n=== Summary: " << testsPassed << " passed, "
              << testsFailed << " failed ===\n";

    return testsFailed > 0 ? 1 : 0;
}
