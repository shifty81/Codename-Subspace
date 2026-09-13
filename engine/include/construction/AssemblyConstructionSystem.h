#pragma once

#include "runtime/StableIdentitySystem.h"
#include "world/SpatialFrameSystem.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class BuildElementKind : std::uint8_t {
    ParametricStructure,
    AuthoredModule,
    GeneratedConnector
};

enum class AttachmentFace : std::uint8_t {
    None,
    Fore,
    Aft,
    Port,
    Starboard,
    Dorsal,
    Ventral
};

struct BuildTransform {
    Double3 position{};
    DoubleQuat rotation{};
    Double3 scale{1.0, 1.0, 1.0};
};

struct SemanticSocket {
    std::string id;
    std::string type;
    AttachmentFace face = AttachmentFace::None;
    Double3 localPosition{};
};

struct BuildElement {
    std::string id;
    BuildElementKind kind = BuildElementKind::AuthoredModule;
    std::string definitionId;
    std::string subAssemblyId;
    BuildTransform transform{};
    double massKg = 0.0;
    std::vector<std::string> capabilityTags;
    std::vector<SemanticSocket> sockets;
};

struct BuildAttachment {
    std::string aElementId;
    std::string aSocketId;
    std::string bElementId;
    std::string bSocketId;
    bool structural = true;

    bool operator==(const BuildAttachment& other) const {
        return aElementId == other.aElementId && aSocketId == other.aSocketId &&
               bElementId == other.bElementId && bSocketId == other.bSocketId &&
               structural == other.structural;
    }
};

struct SubAssemblyDefinition {
    std::string id;
    std::string parentId;
};

struct AssemblyDefinition {
    PersistentEntityId assemblyId{};
    std::string id;
    std::string rootSubAssemblyId = "root";
    double quantumMeters = 0.25;
    std::uint64_t revision = 0;
    std::vector<SubAssemblyDefinition> subAssemblies;
    std::vector<BuildElement> elements;
    std::vector<BuildAttachment> attachments;
};

enum class BuildCommandKind : std::uint8_t {
    AddElement,
    RemoveElement,
    MoveElement,
    Attach,
    Detach
};

struct BuildCommand {
    BuildCommandKind kind = BuildCommandKind::AddElement;
    BuildElement element{};
    std::string elementId;
    BuildTransform beforeTransform{};
    BuildTransform afterTransform{};
    BuildAttachment attachment{};
    std::vector<BuildAttachment> removedAttachments;
};

struct AssemblyValidationResult {
    bool valid = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    double totalMassKg = 0.0;
    std::vector<std::string> capabilities;
};

struct AssemblyCompileSnapshot {
    bool valid = false;
    std::string fingerprint;
    std::uint64_t revision = 0;
    std::size_t elementCount = 0;
    std::size_t attachmentCount = 0;
    double totalMassKg = 0.0;
    std::vector<std::string> capabilities;
};

/// Editable canonical assembly authority. It is intentionally independent of
/// renderer meshes: editable source is validated/compiled into snapshots that
/// downstream render/collision/nav/atmosphere builders can consume.
class AssemblyConstructionSystem {
public:
    static constexpr double DefaultQuantumMeters = 0.25;

    bool Begin(AssemblyDefinition definition, std::string* error = nullptr);
    const AssemblyDefinition& Current() const { return current_; }
    AssemblyDefinition& Current() { return current_; }

    bool Apply(BuildCommand command, std::string* error = nullptr);
    bool Undo(std::string* error = nullptr);
    bool Redo(std::string* error = nullptr);
    std::size_t UndoCount() const { return undo_.size(); }
    std::size_t RedoCount() const { return redo_.size(); }

    AssemblyValidationResult Validate(const std::vector<std::string>& requiredCapabilities = {}) const;
    AssemblyCompileSnapshot Compile(const std::vector<std::string>& requiredCapabilities = {}) const;

    static BuildTransform Quantize(BuildTransform transform, double quantumMeters = DefaultQuantumMeters);
    static bool IsQuantized(const Double3& point, double quantumMeters = DefaultQuantumMeters, double epsilon = 1e-6);
    static bool SocketTypesCompatible(const SemanticSocket& a, const SemanticSocket& b);

private:
    BuildElement* FindElement(const std::string& id);
    const BuildElement* FindElement(const std::string& id) const;
    const SemanticSocket* FindSocket(const BuildElement& element, const std::string& id) const;
    bool HasSubAssembly(const std::string& id) const;
    bool ApplyInternal(BuildCommand& command, BuildCommand* inverse, std::string* error);
    static std::string Fingerprint(const AssemblyDefinition& assembly,
                                   const AssemblyValidationResult& validation);

    AssemblyDefinition current_{};
    std::vector<BuildCommand> undo_;
    std::vector<BuildCommand> redo_;
    bool begun_ = false;
};

} // namespace subspace
