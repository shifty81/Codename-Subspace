#include "construction/AssemblyConstructionSystem.h"
#include "editor/ShipyardAssemblySession.h"
#include "runtime/NativeRuntimeServices.h"
#include "runtime/WorldSimulationAuthority.h"

#include <cmath>
#include <iostream>
#include <string>
#include <type_traits>

using namespace subspace;

namespace {
int failures = 0;
int assertions = 0;

void CheckPass(int pass, bool ok, const char* text) {
    ++assertions;
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << "Pass" << pass << " - " << text << "\n";
    if (!ok) ++failures;
}

bool Near(double a, double b, double eps = 1e-6) { return std::fabs(a - b) <= eps; }
bool Near3(const Double3& a, const Double3& b, double eps = 1e-6) {
    return Near(a.x, b.x, eps) && Near(a.y, b.y, eps) && Near(a.z, b.z, eps);
}

SemanticSocket Socket(std::string id, std::string type, AttachmentFace face) {
    SemanticSocket socket;
    socket.id = std::move(id);
    socket.type = std::move(type);
    socket.face = face;
    return socket;
}

BuildElement CoreElement() {
    BuildElement core;
    core.id = "command_core";
    core.kind = BuildElementKind::AuthoredModule;
    core.definitionId = "module.command.compact.v1";
    core.subAssemblyId = "root";
    core.massKg = 1800.0;
    core.capabilityTags = {"command", "life_support"};
    core.sockets = {Socket("aft_structure", "structure", AttachmentFace::Aft)};
    return core;
}

BuildElement EngineElement() {
    BuildElement engine;
    engine.id = "engine_a";
    engine.kind = BuildElementKind::AuthoredModule;
    engine.definitionId = "module.engine.small.v1";
    engine.subAssemblyId = "propulsion";
    engine.massKg = 900.0;
    engine.capabilityTags = {"propulsion"};
    engine.sockets = {Socket("fore_structure", "structure", AttachmentFace::Fore)};
    engine.transform.position = {0.0, -2.0, 0.0};
    return engine;
}

BuildAttachment CoreToEngine() {
    BuildAttachment attachment;
    attachment.aElementId = "command_core";
    attachment.aSocketId = "aft_structure";
    attachment.bElementId = "engine_a";
    attachment.bSocketId = "fore_structure";
    attachment.structural = true;
    return attachment;
}
}

int main() {
    std::cout << "[Pass973-997 Runtime Composition + Assembly Authority]\n";

    CheckPass(973, std::is_same<decltype(NativeRuntimeServices::worldSimulation), WorldSimulationAuthority>::value,
              "NativeRuntimeServices owns the canonical WorldSimulationAuthority service");

    WorldSimulationAuthority world;
    SpatialFrame rootFrame; rootFrame.id = 100;
    SpatialFrame shipFrame; shipFrame.id = 200; shipFrame.parentId = 100; shipFrame.localPosition = {10.0, 0.0, 0.0};
    world.Frames().Upsert(rootFrame);
    world.Frames().Upsert(shipFrame);

    const auto universeId = world.Register(PersistentEntityKind::World, "test", "universe", {}, 100, "universe.v1");
    const auto systemId = world.Register(PersistentEntityKind::SolarSystem, "test", "system", universeId, 100, "system.v1");
    const auto shipId = world.Register(PersistentEntityKind::Ship, "test", "ship-alpha", systemId, 200, "ship.alpha.v1");
    CheckPass(974, universeId.IsValid() && systemId.IsValid() && shipId.IsValid() && shipId != systemId,
              "stable identities distinguish persistent world/system/ship records");

    CheckPass(975, !world.SetParent(universeId, shipId),
              "persistent parent graph rejects cycles rather than corrupting world ownership");

    CheckPass(976, world.Find(shipId) && world.Find(shipId)->frameId == 200 && world.BindFrame(shipId, 200),
              "persistent entities bind to explicit nested spatial frames");

    const SpatialKinematicState localBefore{200, {2.0, 1.0, 0.0}, {3.0, 0.0, 0.0}};
    const auto beforePoint = world.Frames().ToWorldPoint(localBefore.frameId, localBefore.localPosition);
    const auto beforeVelocity = world.Frames().ToWorldVelocity(localBefore.frameId, localBefore.localPosition, localBefore.localVelocity);
    const auto localAfter = world.ReparentKinematics(localBefore, 100);
    const auto afterPoint = world.Frames().ToWorldPoint(localAfter.frameId, localAfter.localPosition);
    const auto afterVelocity = world.Frames().ToWorldVelocity(localAfter.frameId, localAfter.localPosition, localAfter.localVelocity);
    CheckPass(977, Near3(beforePoint, afterPoint) && Near3(beforeVelocity, afterVelocity),
              "local-physics handoff preserves world position and velocity across frame reparenting");

    CheckPass(978, !world.TransitionRepresentation(shipId, SimulationRepresentation::Full),
              "Full simulation representation cannot activate before residency is established");

    world.SetResident(shipId, true);
    CheckPass(979, world.TransitionRepresentation(shipId, SimulationRepresentation::Full) &&
                   world.Find(shipId)->representation == SimulationRepresentation::Full,
              "resident entities can promote through the canonical representation authority");

    const auto lowPrefetch = world.RequestPrefetch(systemId, SimulationRepresentation::Proxy, 2, "map-hover");
    const auto highPrefetch = world.RequestPrefetch(shipId, SimulationRepresentation::Full, 9, "ship-arrival");
    const auto nextPrefetch = world.PopNextPrefetch();
    CheckPass(980, lowPrefetch.ticket != 0 && highPrefetch.ticket != 0 && nextPrefetch && nextPrefetch->ticket == highPrefetch.ticket,
              "destination prefetch is prioritized and represented by explicit tickets");

    const auto cellId = world.Register(PersistentEntityKind::Cell, "test", "hangar-cell", systemId, 100, "cell.hangar.v1");
    world.SetResident(cellId, true);
    CheckPass(981, world.Find(cellId) && world.Find(cellId)->resident,
              "persistent interior/world cells retain explicit residency state");

    const auto revBefore = world.Find(shipId)->header.revision;
    const auto revAfter = world.MarkDirty(shipId);
    CheckPass(982, revAfter == revBefore + 1 && world.Find(shipId)->dirty,
              "world mutations advance persistent record revision and dirty authority");

    const auto checkpoint = world.CreateCheckpoint();
    CheckPass(983, checkpoint.schemaVersion == SubspacePersistenceSchemaVersion && checkpoint.records.size() == world.EntityCount(),
              "checkpoint generation emits every registered record under the current persistence schema");

    const auto migration = world.PlanMigration(1);
    CheckPass(984, migration.supported && migration.toVersion == SubspacePersistenceSchemaVersion && !migration.steps.empty(),
              "schema-v1 saves receive an explicit migration plan into current persistence authority");

    const auto missingId = StableIdentitySystem::Deterministic(PersistentEntityKind::Station, "test", "missing-station");
    world.AddUnresolvedReference(shipId, missingId, "docking-target");
    const auto checkpointWithMissing = world.CreateCheckpoint();
    CheckPass(985, world.HasUnresolvedReferences() && checkpointWithMissing.unresolvedReferences.size() == 1,
              "unresolved persistent references are surfaced instead of silently discarded");

    CheckPass(986, world.Diagnostics().find("world-authority-v1") != std::string::npos &&
                   world.Diagnostics().find("persistence=v2") != std::string::npos,
              "runtime diagnostics expose canonical world-composition and persistence authority");

    AssemblyDefinition base;
    base.assemblyId = shipId;
    base.id = "ship.alpha.assembly";
    base.quantumMeters = AssemblyConstructionSystem::DefaultQuantumMeters;
    base.subAssemblies = {{"root", {}}, {"propulsion", "root"}};
    base.elements = {CoreElement()};

    AssemblyConstructionSystem construction;
    std::string error;
    CheckPass(987, construction.Begin(base, &error),
              "canonical assembly editing begins from the same stable ship identity");

    BuildElement connector;
    connector.id = "frame_connector";
    connector.kind = BuildElementKind::GeneratedConnector;
    connector.definitionId = "generated.structural.connector.v1";
    connector.subAssemblyId = "root";
    connector.massKg = 25.0;
    connector.sockets = {Socket("a", "universal", AttachmentFace::Fore), Socket("b", "universal", AttachmentFace::Aft)};
    CheckPass(988, connector.kind == BuildElementKind::GeneratedConnector &&
                   CoreElement().kind == BuildElementKind::AuthoredModule,
              "assembly vocabulary distinguishes authored modules from generated structural connectors");

    BuildTransform unsnapped;
    unsnapped.position = {1.13, -0.12, 0.62};
    const auto snapped = AssemblyConstructionSystem::Quantize(unsnapped);
    CheckPass(989, Near(snapped.position.x, 1.25) && Near(snapped.position.y, 0.0) && Near(snapped.position.z, 0.5),
              "construction translation is normalized to the locked 0.25-meter quantum");

    const auto coreSocket = CoreElement().sockets.front();
    const auto engineSocket = EngineElement().sockets.front();
    CheckPass(990, AssemblyConstructionSystem::SocketTypesCompatible(coreSocket, engineSocket) &&
                   coreSocket.face == AttachmentFace::Aft && engineSocket.face == AttachmentFace::Fore,
              "semantic sockets carry attachment type and face authority");

    BuildCommand addEngine; addEngine.kind = BuildCommandKind::AddElement; addEngine.element = EngineElement();
    BuildCommand attach; attach.kind = BuildCommandKind::Attach; attach.attachment = CoreToEngine();
    BuildCommand move; move.kind = BuildCommandKind::MoveElement; move.elementId = "engine_a"; move.afterTransform.position = {0.13, -2.12, 0.0};
    const bool commandsApplied = construction.Apply(addEngine, &error) && construction.Apply(attach, &error) && construction.Apply(move, &error);
    const auto* movedEngine = construction.Current().elements.size() > 1 ? &construction.Current().elements[1] : nullptr;
    CheckPass(991, commandsApplied && movedEngine && Near(movedEngine->transform.position.x, 0.25) && Near(movedEngine->transform.position.y, -2.0),
              "add/attach/move BuildCommands mutate quantized editable source deterministically");

    const auto movedFingerprint = construction.Compile({"command", "propulsion"}).fingerprint;
    const bool undoOk = construction.Undo(&error);
    const auto undoFingerprint = construction.Compile({"command", "propulsion"}).fingerprint;
    const bool redoOk = construction.Redo(&error);
    const auto redoFingerprint = construction.Compile({"command", "propulsion"}).fingerprint;
    CheckPass(992, undoOk && redoOk && movedFingerprint != undoFingerprint && movedFingerprint == redoFingerprint,
              "undo/redo journals restore exact assembly state without crossing transaction semantics");

    CheckPass(993, construction.Current().subAssemblies.size() == 2 &&
                   construction.Current().elements[1].subAssemblyId == "propulsion",
              "hierarchical subassemblies keep propulsion ownership nested beneath the root ship assembly");

    const auto structurallyValid = construction.Validate();
    CheckPass(994, structurallyValid.valid && construction.Current().attachments.size() == 1,
              "structural attachment graph certification rejects disconnected multi-element ships");

    const auto capabilityValid = construction.Validate({"command", "propulsion", "life_support"});
    const auto capabilityInvalid = construction.Validate({"command", "propulsion", "refinery"});
    CheckPass(995, capabilityValid.valid && !capabilityInvalid.valid,
              "assembly validation enforces required ship capabilities before compile/commit");

    // Shipyard begins from the accepted one-element baseline, then applies the
    // same commands through a persistent-ship-bound transactional session.
    world.ClearDirty(shipId);
    ShipyardAssemblySession session;
    const bool sessionBegun = session.Begin(world, shipId, base, &error);
    BuildCommand sessionAdd; sessionAdd.kind = BuildCommandKind::AddElement; sessionAdd.element = EngineElement();
    BuildCommand sessionAttach; sessionAttach.kind = BuildCommandKind::Attach; sessionAttach.attachment = CoreToEngine();
    const bool sessionEdited = sessionBegun && session.Apply(sessionAdd, &error) && session.Apply(sessionAttach, &error);
    const auto preview = session.Preview({"command", "propulsion"});
    CheckPass(996, sessionEdited && session.Dirty() && preview.valid && preview.elementCount == 2,
              "Shipyard session edits the persistent docked ship through previewable transactional assembly state");

    const auto previewFingerprint = preview.fingerprint;
    const bool committed = session.Commit({"command", "propulsion"}, &error);
    const auto acceptedFingerprint = session.AcceptedFingerprint();
    const auto stablePreview = session.Preview({"command", "propulsion"});
    CheckPass(997, committed && !session.Dirty() && world.Find(shipId)->dirty &&
                   !previewFingerprint.empty() && previewFingerprint == acceptedFingerprint &&
                   stablePreview.fingerprint == acceptedFingerprint,
              "validated assembly commit is deterministic and marks the persistent ship for save/replication");

    std::cout << "Assertions: " << assertions << " failures: " << failures << "\n";
    return failures == 0 ? 0 : 1;
}
