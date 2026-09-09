#pragma once

#include "rendering/ProceduralVisualVariantSystem.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace subspace {

enum class ShipyardModuleClass {
    Hull,
    Command,
    Propulsion,
    Hardpoint,
    Detail,
    Wing,
    Adapter,
    Component
};

enum class ShipyardModuleSemantic {
    HullBow,
    HullMid,
    HullAft,
    StructuralFrame,
    CommandCockpit,
    CommandBridge,
    Adapter,
    EngineHousing,
    MainEngine,
    EngineNozzle,
    RcsThruster,
    TurretHardpoint,
    WeaponMount,
    Wing,
    Sensor,
    SurfaceDetail,
    Component
};

enum class ShipyardModuleSize {
    XS,
    S,
    M,
    L,
    XL
};

// Canonical R5 builder-facing taxonomy types.  These live beside the module
// semantics so every subsystem that includes ShipyardModuleSystem.h sees the
// complete enum definitions and enumerators.  Decoration is an intentional
// compatibility alias for the older Pass426-450 builder/itemization API.
enum class ShipyardPartCategory {
    Hull = 0,
    Command = 1,
    Propulsion = 2,
    Hardpoint = 3,
    Detail = 4,
    Decoration = Detail,
    Wing = 5,
    Adapter = 6,
    Component = 7
};

enum class ShipyardPartRole {
    Unknown = 0,
    PrimaryHull = 1,
    HullAdapter = 2,
    Cockpit = 3,
    Bridge = 4,
    EngineHousing = 5,
    EngineMount = 6,
    MainEngine = 7,
    RcsThruster = 8,
    HardpointBase = 9,
    WeaponTurret = 10,
    MissileMount = 11,
    SensorDish = 12,
    SensorMast = 13,
    Cargo = 14,
    Tank = 15,
    StructuralFrame = 16,
    // Compatibility names used by the manufacturing/itemization passes.
    // They intentionally collapse onto the canonical structural/wing roles;
    // R5 keeps one taxonomy authority while accepting older descriptive names.
    StructuralBrace = 19,
    StructuralBlock = 20,
    SurfaceDetail = 17,
    Decoration = SurfaceDetail,
    Wing = 18,
    Fin = 21,
    EngineStrut = 22,
    EngineNozzle = 23,
    SensorAntenna = 24,
    Telescope = 25,
    Hangar = 26,
    WindowCanopy = 27,
    StructuralAttachment = 28,
    Ram = 29,
    Outrigger = 30,
    ReviewRequired = 31
};

struct ShipyardModuleRecord;

class ShipyardPartTaxonomySystem {
public:
    static ShipyardPartCategory CategoryFor(ShipyardModuleClass moduleClass);
    static ShipyardPartRole RoleFor(ShipyardModuleSemantic semantic, const std::string& nameOrId = {});
    // std::string is deliberate: several builder/itemization callers compose
    // these names directly with string literals. Returning const char* makes
    // those expressions pointer arithmetic on MSVC.
    static std::string DisplayName(ShipyardPartCategory category);
    static std::string DisplayName(ShipyardPartRole role);
    static std::string DisplayName(ShipyardModuleClass moduleClass);
    static std::string DisplayName(ShipyardModuleSemantic semantic);
    static std::string DisplayName(const ShipyardModuleRecord& record);
    static std::string DisplayName(const VisualModuleSource& source);
    static std::string DisplayName(std::string_view moduleNameOrId);
    static std::string CategoryName(ShipyardPartCategory category);
    static std::string CategoryName(ShipyardModuleClass moduleClass);
};

struct ShipyardAssemblySocket {
    std::string name;
    std::string type;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float dirX = 0.0f;
    float dirY = 0.0f;
    float dirZ = 0.0f;
    float insertionDepth = 0.0f;
    // Authorable socket frame up-vector. Existing generated/catalog aggregate
    // initializers intentionally stop at insertionDepth, so these remain
    // backward-compatible defaults until a user edits the socket orientation.
    float upX = 0.0f;
    float upY = 0.0f;
    float upZ = 1.0f;
    bool manualOverride = false;
};

struct ShipyardModuleRecord {
    VisualModuleSource source;
    ShipyardModuleClass moduleClass = ShipyardModuleClass::Component;
    ShipyardModuleSemantic semantic = ShipyardModuleSemantic::Component;
    ShipyardModuleSize size = ShipyardModuleSize::M;
    std::vector<ShipyardAssemblySocket> sockets;
    std::vector<std::string> preferredRoles;
    bool preserveAspectRatio = true;

    // R5 canonical builder/gameplay metadata. The semantic above remains the
    // authority; these normalized fields prevent parallel builder taxonomies.
    ShipyardPartCategory builderCategory = ShipyardPartCategory::Component;
    ShipyardPartRole partRole = ShipyardPartRole::Unknown;
    bool primaryHull = false;
    bool generatorEligible = true;
    bool surfaceOnly = false;
    bool functional = false;
    bool mirrorPreferred = false;
    // R6 assembly grammar metadata. Certification remains permissive for manual
    // Shipyard use, while procedural generation consumes these stricter roles.
    std::string placementRole = "EXCLUDED";
    bool pairedPlacement = false;
    std::string preferredMountFace;
    float mountFaceConfidence = 0.0f;
};

struct ShipyardAssemblyValidation {
    bool valid = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    explicit operator bool() const { return valid; }
};

struct ShipyardPropulsionPort {
    std::string moduleId;
    ShipyardModuleSemantic semantic = ShipyardModuleSemantic::MainEngine;
    Vector3 localPosition{};
    Vector3 exhaustDirection{0.0f,-1.0f,0.0f};
    float nozzleRadiusHint = 0.08f;
    float plumeLengthHint = 1.0f;
};

/// Greyoxide Shipyard v0.7 assembly grammar.
///
/// The imported kit meshes are treated as authored building pieces. R2 does
/// not remodel, split or non-uniformly stretch them. Subspace adds semantic
/// classification, size/role rules and typed mating sockets, then assembles
/// ships by aligning those sockets with a small intentional insertion depth.
class ShipyardModuleSystem {
public:
    static bool IsShipyardModule(const std::string& moduleId);
    static bool IsCertifiedShipyardModule(const std::string& moduleId);
    static ShipyardModuleClass Classify(const VisualModuleSource& source);
    static ShipyardModuleSemantic SemanticClassify(const VisualModuleSource& source);
    static ShipyardModuleSize SizeClassify(const VisualModuleSource& source);
    static const char* ClassName(ShipyardModuleClass moduleClass);
    static const char* SemanticName(ShipyardModuleSemantic semantic);
    static const char* SizeName(ShipyardModuleSize size);

    static std::vector<ShipyardAssemblySocket> BuildSockets(const VisualModuleSource& source,
                                                            ShipyardModuleSemantic semantic);
    static bool RoleSuitable(ShipyardModuleSemantic semantic, const std::string& role);
    static bool CanMate(const std::string& parentSocketType, const std::string& childSocketType);

    // Pass533A propulsion authority. Main-drive parts may only enter the
    // assembly through canonical rear-drive sockets (or a certified engine
    // housing chain). RCS remains intentionally exempt because maneuvering
    // thrusters must point in multiple directions.
    static bool IsRearDriveSocketName(std::string_view socketName);
    static bool ValidatePropulsionPlacement(const std::vector<ShipyardModuleRecord>& catalog,
                                            const ProceduralShipVisualRecipe& recipe,
                                            std::size_t moduleIndex,
                                            std::string* error = nullptr);

    // Stable R5 compatibility API used by the native kitbash builder, itemization
    // and equipment layers. These all resolve back to the canonical module record.
    static bool SizeCompatible(ShipyardModuleSize parent, ShipyardModuleSize child);
    static VisualModulePlacement BuildAttachmentPlacement(const VisualModulePlacement& parentPlacement,
                                                           const ShipyardAssemblySocket& parentSocket,
                                                           const ShipyardModuleRecord& child,
                                                           const ShipyardAssemblySocket& childSocket,
                                                           float uniformScale = 1.0f);

    // Pass426-450 compatibility adapter.  The older native kitbash builder used
    // a bool/out-parameter attachment API with a wider argument list.  Keep a
    // single permissive adapter here so those callers resolve onto the R5
    // socket/orientation implementation instead of reintroducing a second
    // placement algorithm.  The exact modern 5-argument overload above remains
    // preferred by overload resolution for current code.
    template <typename... Args>
    static bool BuildAttachmentPlacement(Args&&... args) {
        LegacyAttachmentCapture capture;
        (CaptureLegacyAttachmentArg(capture, std::forward<Args>(args)), ...);
        return ResolveLegacyAttachment(capture);
    }

    static ShipyardAssemblyValidation ValidateAssemblyGraph(const std::vector<ShipyardModuleRecord>& catalog,
                                                             const ProceduralShipVisualRecipe& recipe);

    // Compatibility for Pass426-455 builder call sites that supplied the
    // working recipe before the catalog. Both forms reach the same validator.
    static ShipyardAssemblyValidation ValidateAssemblyGraph(const ProceduralShipVisualRecipe& recipe,
                                                             const std::vector<ShipyardModuleRecord>& catalog);

    // Pass451-era kitbash builder compatibility: some callers retain the raw
    // VisualModuleSource inventory rather than a preclassified catalog. Build
    // the canonical R5 catalog here and forward to the same validator.
    static ShipyardAssemblyValidation ValidateAssemblyGraph(const ProceduralShipVisualRecipe& recipe,
                                                             const std::vector<VisualModuleSource>& availableModules);
    static ShipyardAssemblyValidation ValidateAssemblyGraph(const std::vector<VisualModuleSource>& availableModules,
                                                             const ProceduralShipVisualRecipe& recipe);

    // Legacy builder validation overload.  Classification is reconstructed
    // from the stable module IDs, so this still validates hull/command/drive
    // requirements without needing a second stored catalog.
    static ShipyardAssemblyValidation ValidateAssemblyGraph(const ProceduralShipVisualRecipe& recipe);

    // Explicit legacy/manual-builder error adapter. Keep this non-template so
    // std::string* call sites do not fall through the generic catalog-view
    // compatibility adapter and silently lose validation diagnostics.
    static bool ValidateAssemblyGraph(const ProceduralShipVisualRecipe& recipe, std::string* error);

    // Pass460 compatibility closure. Some late native builder revisions hold a
    // palette/catalog wrapper type that is intentionally private to the builder
    // layer. The catalog argument is only advisory at that call site: the R5
    // recipe already carries stable module IDs and the canonical one-argument
    // validator reconstructs/validates those IDs against Shipyard semantics.
    //
    // Keep concrete std::vector<ShipyardModuleRecord> and VisualModuleSource
    // overloads above; non-template overload resolution prefers them whenever
    // the canonical catalog type is available. Unknown builder-side views fall
    // through here instead of forcing ShipyardCore to depend on UI/editor types.
    // Late builder compatibility adapters deliberately return bool. The private
    // builder-side catalog/view types only use this API as a yes/no guard, while
    // canonical R5 catalog overloads above retain the full validation report.
    // This prevents legacy `return ValidateAssemblyGraph(...)` call sites from
    // requiring an implicit conversion from ShipyardAssemblyValidation.
    template <typename CatalogLike>
    static bool ValidateAssemblyGraph(
        const ProceduralShipVisualRecipe& recipe, const CatalogLike&) {
        return ValidateAssemblyGraph(recipe).valid;
    }

    template <typename CatalogLike>
    static bool ValidateAssemblyGraph(
        const CatalogLike&, const ProceduralShipVisualRecipe& recipe) {
        return ValidateAssemblyGraph(recipe).valid;
    }

    static std::vector<ShipyardModuleRecord> BuildCatalog(
        const std::vector<VisualModuleSource>& availableModules);

    /// Resolve visual exhaust ports from the propulsion modules already placed
    /// in a Shipyard recipe. This is the authoritative presentation bridge: no
    /// second detached thruster layout or generated pylon geometry is required.
    static std::vector<ShipyardPropulsionPort> BuildPropulsionPorts(
        const std::vector<ShipyardModuleRecord>& catalog,
        const ProceduralShipVisualRecipe& recipe);

    /// Two deterministic examples each for industrial, combat, mining, hauler
    /// and exploration. Structural Shipyard pieces share one uniform ship scale
    /// so their authored proportions/mating surfaces are preserved. Engines are
    /// inserted into aft hull engine-cavity sockets rather than placed behind
    /// the ship at guessed offsets.
    static std::vector<ProceduralShipVisualRecipe> BuildShowcaseRecipes(
        const std::vector<VisualModuleSource>& availableModules,
        std::uint32_t seed = 0x51A7D007u);
    static std::vector<ProceduralShipVisualRecipe> BuildShowcaseRecipes(
        const std::vector<ShipyardModuleRecord>& catalog,
        std::uint32_t seed);

private:
    struct LegacyAttachmentCapture {
        std::vector<const ShipyardModuleRecord*> records;
        std::vector<const VisualModuleSource*> sources;
        std::vector<const ShipyardAssemblySocket*> sockets;
        std::vector<const VisualModulePlacement*> placements;
        std::vector<VisualModulePlacement*> writablePlacements;
        std::vector<std::string> texts;
        float uniformScale = 1.0f;
        bool hasScale = false;
    };

    template <typename T>
    static void CaptureLegacyAttachmentArg(LegacyAttachmentCapture& capture, T&& arg) {
        using Ref = std::remove_reference_t<T>;
        using Raw = std::remove_cv_t<Ref>;

        if constexpr (std::is_same_v<Raw, ShipyardModuleRecord>) {
            capture.records.push_back(&arg);
        } else if constexpr (std::is_same_v<Raw, VisualModuleSource>) {
            capture.sources.push_back(&arg);
        } else if constexpr (std::is_same_v<Raw, ShipyardAssemblySocket>) {
            capture.sockets.push_back(&arg);
        } else if constexpr (std::is_same_v<Raw, VisualModulePlacement>) {
            capture.placements.push_back(&arg);
            if constexpr (!std::is_const_v<Ref>) capture.writablePlacements.push_back(&arg);
        } else if constexpr (std::is_floating_point_v<Raw>) {
            const float value = static_cast<float>(arg);
            if (!capture.hasScale && value > 0.0f && value <= 100.0f) {
                capture.uniformScale = value;
                capture.hasScale = true;
            }
        } else if constexpr (std::is_convertible_v<T, std::string_view>) {
            capture.texts.emplace_back(std::string_view(arg));
        } else if constexpr (std::is_pointer_v<Raw>) {
            using Pointee = std::remove_cv_t<std::remove_pointer_t<Raw>>;
            if (!arg) return;
            if constexpr (std::is_same_v<Pointee, ShipyardModuleRecord>) {
                capture.records.push_back(arg);
            } else if constexpr (std::is_same_v<Pointee, VisualModuleSource>) {
                capture.sources.push_back(arg);
            } else if constexpr (std::is_same_v<Pointee, ShipyardAssemblySocket>) {
                capture.sockets.push_back(arg);
            } else if constexpr (std::is_same_v<Pointee, VisualModulePlacement>) {
                capture.placements.push_back(arg);
                using OriginalPointee = std::remove_pointer_t<Raw>;
                if constexpr (!std::is_const_v<OriginalPointee>) capture.writablePlacements.push_back(arg);
            }
        }
    }

    static bool ResolveLegacyAttachment(const LegacyAttachmentCapture& capture);
};

} // namespace subspace
