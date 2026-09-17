#pragma once

#include "content/ShipyardModuleSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "ship_editor/ShipyardEquipmentSystem.h"
#include "ship_editor/ShipyardTransformSystem.h"
#include "ship_editor/ShipyardDragDropSystem.h"
#include "ship_editor/ShipyardWorkspaceSystem.h"
#include "ship_editor/ShipyardDccUiSystem.h"
#include "content/UniversalKitbashAuthority.h"
#include "construction/UniversalConstructionSystem.h"
#include "editor/ConstructionSymmetrySystem.h"
#include "modeling/ShipyardModelingSystem.h"
#include "procedural/ShipyardPcgStudioSystem.h"
#include "ship_editor/ShipyardCapabilitySystem.h"
#include "world/PlanetWorldEngineSystem.h"
#include "world/WorldScaleAuthoritySystem.h"
#include "character/CharacterAnimationLibrarySystem.h"
#include "developer/ShipyardDevWorldSystem.h"
#include "interior/ShipModuleInteriorLinkSystem.h"
#include "interior/ShipInteriorAuthoringSystem.h"
#include "interior/ShipInteriorStructureAuthoringSystem.h"
#include "ships/ShipClassRoleSystem.h"
#include "generator/GeneratorParitySystem.h"
#include "editor/ForgeWorkspaceSystem.h"

#include <optional>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class ShipyardInspectorTab {
    Transform,
    Assembly,
    Sockets,
    Authoring,
    Appearance
};

enum class ShipyardTransformConstraint {
    Free,
    X,
    Y,
    Z,
    XY,
    XZ,
    YZ
};

enum class ShipyardBuilderCommand {
    None,
    UndoAuthoring,
    RedoAuthoring,
    WorkspaceBuild,
    WorkspaceModel,
    WorkspaceInterior,
    WorkspaceAppearance,
    WorkspaceSystems,
    WorkspaceCharacter,
    WorkspacePcg,
    WorkspaceWorld,
    WorkspaceDevWorld,
    WorkspaceProjectTools,
    WorkspaceAuthoring,
    InspectorTransform,
    InspectorAssembly,
    InspectorSockets,
    InspectorAuthoring,
    InspectorAppearance,
    DccToggleAssetBrowser,
    DccToggleToolRail,
    DccToggleSidebar,
    DccToggleOutliner,
    DccToggleProperties,
    DccToggleStatusBar,
    DccToggleMaximizeViewport,
    DccResetLayout,
    DccToggleGrid,
    DccToggleGizmos,
    DccToggleSocketsOverlay,
    DccToggleStatsOverlay,
    DccToggleShieldPreview,
    DccCycleShading,
    DccCycleOutlinerMode,
    DccCycleAssetDensity,
    DccAssetZoomIn,
    DccAssetZoomOut,
    DccPreviousAssetPreset,
    DccNextAssetPreset,
    DccToggleFavoriteSelected,
    DccClearAssetFilters,
    DccAssetPrevious,
    DccAssetNext,
    DccAssetFocusSearch,
    DccAssetClearSearch,
    DccOutlinerPrevious,
    DccOutlinerNext,
    DccToggleCommandPalette,
    DccWorkspacePrevious,
    DccWorkspaceNext,
    DccPropertiesPrevious,
    DccPropertiesNext,
    DccPanelToggleCollapse,
    DccPanelToggleFloat,
    DccPanelTogglePin,
    DccPanelToggleAutoHide,
    DccPanelToggleVisible,
    DccPanelResetWorkspace,
    DccToggleGuidedWorkflow,
    TransformConstraintX,
    TransformConstraintY,
    TransformConstraintZ,
    TransformConstraintClear,
    SelectClass,
    SelectModule,
    SelectPlaced,
    PreviousModule,
    NextModule,
    PreviousPlaced,
    NextPlaced,
    AddModule,
    ReplaceModule,
    RemoveModule,
    DetachModule,
    ReattachNearest,
    PreviousSnapCandidate,
    NextSnapCandidate,
    ConfirmPlacement,
    CancelPlacement,
    PreviousSocket,
    NextSocket,
    AddSocket,
    RemoveSocket,
    MirrorSocketX,
    CycleSocketType,
    ResetSocket,
    UndoSocketEdit,
    RedoSocketEdit,
    SaveSocketOverrides,
    ArticulationToggle,
    ArticulationUseSelectedSocketPivot,
    ArticulationCycleMode,
    ArticulationSpeedDown,
    ArticulationSpeedUp,
    PreviousSemantic,
    NextSemantic,
    ToggleGeneratorEligible,
    TogglePairedPlacement,
    CyclePreferredMountFace,
    ResetDefinitionOverride,
    SaveDefinitionOverrides,
    ToggleMirrorX,
    PreviousLiveryPreset,
    NextLiveryPreset,
    PreviousPrimaryPaint,
    NextPrimaryPaint,
    PreviousSecondaryPaint,
    NextSecondaryPaint,
    PreviousTrimPaint,
    NextTrimPaint,
    PreviousPrimaryFinish,
    NextPrimaryFinish,
    PreviousSecondaryFinish,
    NextSecondaryFinish,
    PreviousTrimFinish,
    NextTrimFinish,
    MaterialRemoveSelected,
    MaterialRestoreSelected,
    PreviousDecalPreset,
    NextDecalPreset,
    AddDecal,
    RemoveDecal,
    ToolSelect,
    ToolMove,
    ToolRotate,
    ToolScale,
    ModelPreviousPrimitive,
    ModelNextPrimitive,
    ModelAddShape,
    ModelAddBox,
    ModelAddWedge,
    ModelAddCylinder,
    ModelAddSphere,
    ModelAddPlate,
    ModelAddBeam,
    ModelAddDoor,
    ModelAddAirlock,
    ModelAddFloor,
    ModelDuplicatePrimitive,
    ModelRemovePrimitive,
    ModelAddMirrorModifier,
    ModelAddLinearArrayModifier,
    ModelAddBevelModifier,
    ModelPreviousPurpose,
    ModelNextPurpose,
    ModelAssignPurpose,
    ModelCycleSelectionMode,
    ModelStretchXNegative,
    ModelStretchXPositive,
    ModelStretchYNegative,
    ModelStretchYPositive,
    ModelStretchZNegative,
    ModelStretchZPositive,
    ModelToggleSymmetricStretch,
    ModelValidate,
    ModelPublishCanonical,
    PcgReroll,
    PcgAudit,
    PcgToggleOccupancy,
    PcgToggleDetailDensity,
    PcgTeachFromAssembly,
    WorldPreviousBrush,
    WorldNextBrush,
    WorldRadiusDown,
    WorldRadiusUp,
    WorldStrengthDown,
    WorldStrengthUp,
    WorldApplyBrush,
    WorldToggleHydrology,
    WorldToggleResources,
    DevWorldPreviousBackdrop,
    DevWorldNextBackdrop,
    DevWorldToggleCertifiedOnly,
    DevWorldTogglePlayerReference,
    ScalePlayerDown,
    ScalePlayerUp,
    ToggleTransformSpace,
    ToggleTransformSnap,
    NudgePort,
    NudgeStarboard,
    NudgeForward,
    NudgeAft,
    NudgeDorsal,
    NudgeVentral,
    RotatePitchPositive,
    RotatePitchNegative,
    RotateYawPositive,
    RotateYawNegative,
    RotateRollPositive,
    RotateRollNegative,
    FlipPitch,
    FlipYaw,
    FlipRoll,
    MirrorSelectedX,
    SymmetryAxisPortStarboard,
    SymmetryAxisForeAft,
    SymmetryAxisDorsalVentral,
    ToggleLiveSymmetry,
    SymmetryPlaneNegative,
    SymmetryPlanePositive,
    ResetSymmetryFrame,
    MirrorSelectedAcrossSymmetry,
    BreakSymmetryPair,
    CycleRotationStep,
    ResetRotation,
    ScaleUniformNegative,
    ScaleUniformPositive,
    ResetScale,
    ScaleAssemblyDown,
    ScaleAssemblyUp,
    FrameSelected,
    FrameShip,
    PreviousRole,
    NextRole,
    PreviousGeneratorDomain,
    NextGeneratorDomain,
    PreviousShipClass,
    NextShipClass,
    PreviousTargetSize,
    NextTargetSize,
    CycleConstructionMode,
    GenerateVariant,
    GenerateInteriorProgram,
    InteriorAddFloor,
    InteriorAddWall,
    InteriorAddDoor,
    InteriorAddAirlock,
    InteriorAddHatch,
    InteriorRemoveElement,
    InteriorPreviousElement,
    InteriorNextElement,
    RunProjectTool,
    Validate,
    SaveBlueprint,
    Apply,
    Reset,
    // Recovery action: does not toggle a hidden/inactive Assets tab closed.
    // Appended to preserve existing command identifiers.
    DccRevealAssetBrowser
};

struct ShipyardBuilderControl {
    ShipyardBuilderCommand command = ShipyardBuilderCommand::None;
    int value = 0;
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    std::string label;
    bool active = false;
    bool enabled = true;
    // GUI authority: generated controls identify their owning panel, so
    // floating z-order and hit tests never rely on guessing from coordinates.
    std::string panelId;

    bool Contains(float px, float py) const {
        return enabled && px >= x && py >= y && px <= x + width && py <= y + height;
    }
};


struct ShipyardBuilderLayout {
    bool valid = false;
    bool compact = false;
    float uiScale = 1.0f;
    float left = 0.0f;
    float top = 0.0f;
    float workspaceBarY = 0.0f;
    float workspaceBarHeight = 0.0f;
    float toolRailX = 0.0f;
    float toolRailY = 0.0f;
    float toolRailWidth = 0.0f;
    float toolRailHeight = 0.0f; // independent dock root column extent
    float leftWidth = 0.0f;
    float right = 0.0f;
    float rightWidth = 0.0f;
    float rowHeight = 0.0f;
    float rowGap = 0.0f;
    float libraryListY = 0.0f;
    float moduleCardsY = 0.0f;
    float moduleCardHeight = 0.0f;
    float tabRowY = 0.0f;
    float tabHeight = 0.0f;
    float contentTopY = 0.0f;
    float leftActionsY = 0.0f;
    float leftInfoY = 0.0f;
    float placedListY = 0.0f;
    std::size_t placedPageSize = 0;
    float selectedSummaryY = 0.0f;
    float editLabelY = 0.0f;
    float editRowY = 0.0f;
    float focusLabelY = 0.0f;
    float focusRowY = 0.0f;
    float moveLabelY = 0.0f;
    float moveRowY = 0.0f;
    float moveRow2Y = 0.0f;
    float rotateLabelY = 0.0f;
    float rotateRowY = 0.0f;
    float yawRowY = 0.0f;
    float rollRowY = 0.0f;
    float flipRowY = 0.0f;
    float blueprintLabelY = 0.0f;
    float classRowY = 0.0f;
    float sizeModeRowY = 0.0f;
    float generateRowY = 0.0f;
    float saveRowY = 0.0f;
    float liveryLabelY = 0.0f;
    float liveryRowY = 0.0f;
    float liverySecondaryRowY = 0.0f;
    float paintSecondaryRowY = 0.0f;
    float paintTrimRowY = 0.0f;
    float decalRowY = 0.0f;
    float validationY = 0.0f;
    float statusY = 0.0f;

    // PASS1444-1453: normalized DCC-area geometry. These fields are the
    // visible-shell authority; legacy fields above remain for compatibility
    // with older automation/tests while the authoring UI migrates to the
    // universal DCC shell.
    float viewportLeft = 0.0f;
    float viewportTop = 0.0f;
    float viewportRight = 0.0f;
    float viewportBottom = 0.0f;
    float assetShelfX = 0.0f;
    float assetShelfY = 0.0f;
    float assetShelfWidth = 0.0f;
    float assetShelfHeight = 0.0f;
    float outlinerX = 0.0f;
    float outlinerY = 0.0f;
    float outlinerWidth = 0.0f;
    float outlinerHeight = 0.0f;
    float propertiesX = 0.0f;
    float propertiesY = 0.0f;
    float propertiesWidth = 0.0f;
    float propertiesHeight = 0.0f;
};

struct ShipyardBuilderValidation {
    bool valid = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

struct ShipyardBuilderRuntimeModel {
    bool initialized = false;
    bool dirty = false;
    bool mirrorX = true; // compatibility alias for live port/starboard symmetry
    ConstructionSymmetryFrame symmetryFrame{};
    std::vector<ConstructionSymmetryPair> symmetryPairs;
    bool liveApplyEnabled = false;
    bool standaloneDesign = false;
    ShipyardAccessMode accessMode = ShipyardAccessMode::PlayerDocked;
    ShipyardCapabilityProfile capabilities{};
    ShipyardModelingState modeling{};
    ShipyardPcgStudioState pcgStudio{};
    PlanetWorldAuthoringState worldAuthoring{};
    WorldScaleProfile worldScale{};
    CharacterAnimationLibraryProfile animationLibrary{};
    DevAnimationPreviewState animationPreview{};
    ShipyardDevWorldState devWorld{};
    ShipInteriorConnectionPlan interiorPlan{};
    GeneratedShipInteriorProgram interiorProgram{};
    ShipInteriorStructuralModel interiorStructure{};
    std::string interiorProgramStatus = "Interior program not generated";
    std::size_t generatedInteriorRoomCount = 0;
    std::size_t generatedInteriorPortalCount = 0;
    ShipyardModuleClass selectedClass = ShipyardModuleClass::Hull;
    std::size_t selectedFilteredModule = 0;
    std::size_t selectedPlacedModule = 0;
    std::size_t selectedSocket = 0;
    std::size_t catalogScrollStart = 0;
    bool assetSearchFocused = false;
    std::size_t placedScrollStart = 0;
    bool socketOverridesDirty = false;
    bool definitionOverridesDirty = false;
    ShipyardInspectorTab inspectorTab = ShipyardInspectorTab::Transform;
    ShipyardWorkspaceMode workspaceMode = ShipyardWorkspaceMode::Build;
    ShipyardDccUiState dcc = ShipyardDccUiSystem::DefaultState();
    bool developerWorkspacesVisible = false;
    bool testWorkspaceActive = false;
    bool guidedWorkflow = true;
    SubspaceDockWorkspace dockWorkspace{};
    std::string role = "INDUSTRIAL";
    GeneratorDomain authoringDomain = GeneratorDomain::Ship;
    ForgeWorkspaceSnapshot projectTools{};
    ShipClass shipClass = ShipClass::Frigate;
    UniversalSizeClass targetModuleSize = UniversalSizeClass::XS;
    ConstructionWorkspaceMode constructionMode = ConstructionWorkspaceMode::Ship;
    std::string factionId = "PLAYER";
    int hullFamilyIndex = 1;
    std::uint32_t seed = 0x51A7D007u;
    std::string status = "Shipyard ready";
    std::size_t liveryPreset = 0;
    std::size_t decalPreset = 0;
    std::string liveryName = "CUSTOM";
    std::string primaryPaintName = "STEEL";
    std::string secondaryPaintName = "GRAPHITE";
    std::string trimPaintName = "AMBER";
    ShipAppearanceState appearance{};
    ShipyardTransformTool transformTool = ShipyardTransformTool::Select;
    ShipyardTransformSpace transformSpace = ShipyardTransformSpace::View;
    ShipyardTransformConstraint transformConstraint = ShipyardTransformConstraint::Free;
    bool transformConstraintLocal = false;
    bool transformSnap = true;
    float rotationStepDegrees = 15.0f;
    ShipyardTransformTransaction transform{};
    ShipyardDragPreview dragPreview{};
    ShipyardBuilderValidation validation{};
    std::vector<ShipyardModuleRecord> catalog;
    std::vector<std::string> availableModuleIds; // read-only catalog eligibility snapshot for renderer/input parity
    ProceduralShipVisualRecipe recipe{};
};

/// Runtime-native Greyoxide Shipyard editor authority.
///
/// This restores the interactive C++ ship-builder path that was lost during
/// the C# -> C++ frontend conversion.  It edits the same certified module IDs,
/// typed mating sockets and procedural recipe representation used by the
/// runtime renderer instead of maintaining a second fake/paint-only builder.
class ShipyardBuilderSystem {
public:
    void Initialize(std::vector<ShipyardModuleRecord> catalog,
                    const ProceduralShipVisualRecipe& starterRecipe);
    void SetAvailableModuleIds(std::vector<std::string> moduleIds);
    void SetLiveApplyEnabled(bool enabled, bool standaloneDesign = false);
    void SetAccessMode(ShipyardAccessMode mode);

    const ShipyardBuilderRuntimeModel& Model() const { return model_; }
    // PASS1508: constrained dock-only mutation for native pointer routing.
    SubspaceDockWorkspace& MutableDockWorkspace() { return model_.dockWorkspace; }
    const ProceduralShipVisualRecipe& Recipe() const { return model_.recipe; }
    const ShipAppearanceState& Appearance() const { return model_.appearance; }
    const ProceduralShipVisualRecipe& BaselineRecipe() const { return initialRecipe_; }
    const ShipAppearanceState& BaselineAppearance() const { return initialAppearance_; }
    void SetAppearance(const ShipAppearanceState& appearance);
    bool IsInitialized() const { return model_.initialized; }

    static std::vector<std::size_t> VisibleCatalogIndices(const ShipyardBuilderRuntimeModel& model);
    std::vector<std::size_t> FilteredCatalogIndices() const;
    const ShipyardModuleRecord* SelectedCatalogModule() const;
    const VisualModulePlacement* SelectedPlacedModule() const;
    const ShipyardAssemblySocket* SelectedSocket() const;

    bool Activate(ShipyardBuilderCommand command, int value = 0);
    // Pass1268+: transitional legacy implementation entrypoints. The current
    // monolithic .cpp is compiled through a confined compatibility alias while
    // the public methods below are owned by the professional visible shell.
    bool LegacyActivate(ShipyardBuilderCommand command, int value = 0);
    bool CanUndoAuthoring() const { return !authoringUndo_.empty(); }
    bool CanRedoAuthoring() const { return !authoringRedo_.empty(); }
    bool UndoAuthoring();
    bool RedoAuthoring();
    std::size_t AuthoringUndoCount() const { return authoringUndo_.size(); }
    std::size_t AuthoringRedoCount() const { return authoringRedo_.size(); }
    ShipyardBuilderValidation Validate() const;

    bool BeginSelectedTransform();
    // Rewind to the drag start before applying total pointer displacement.
    // This prevents incremental snapping from swallowing sub-step motion.
    bool ResetSelectedTransformPreview();
    bool BeginSelectedSocketTransform();
    bool TranslateSelected(const Vector3& delta,bool fine=false);
    bool TranslateSelectedSocket(const Vector3& delta,bool fine=false);
    bool RotateSelected(const Vector3& deltaDegrees,bool fine=false);
    bool RotateSelectedSocket(const Vector3& deltaDegrees,bool fine=false);
    bool ScaleSelected(const Vector3& deltaScale,bool fine=false);
    bool SetTransformConstraint(ShipyardTransformConstraint constraint, bool toggleLocalOnRepeat = true);
    void ClearTransformConstraint();
    static const char* TransformConstraintName(ShipyardTransformConstraint constraint);
    static Vector3 ApplyTransformConstraint(const Vector3& value, ShipyardTransformConstraint constraint);
    bool BeginDockPanelDrag(int panelValue, int viewportWidth, int viewportHeight, float pointerX, float pointerY);
    bool DragDockPanel(float deltaX, float deltaY);
    void EndDockPanelDrag();
    bool ScaleAssembly(float factor);
    bool CommitTransform();
    bool CancelTransform();
    bool CommitSocketTransform();
    bool CancelSocketTransform();
    bool CanUndoSocketEdit() const;
    bool CanRedoSocketEdit() const;
    bool UndoSocketEdit();
    bool RedoSocketEdit();
    bool LoadSocketOverrides(const std::string& path,std::string* error=nullptr,std::size_t* appliedModules=nullptr);
    bool SaveSocketOverrides(const std::string& path,std::string* error=nullptr,std::size_t* changedModules=nullptr) const;
    bool LoadDefinitionOverrides(const std::string& path,std::string* error=nullptr,std::size_t* appliedModules=nullptr);
    bool SaveDefinitionOverrides(const std::string& path,std::string* error=nullptr,std::size_t* changedModules=nullptr) const;
    bool BeginCatalogDrag(int filteredIndex);
    bool HandleWheel(float pointerX,float pointerY,float wheelDelta,int viewportWidth,int viewportHeight);
    bool HandleAssetSearchInput(const std::string& text);
    void BlurAssetSearch() { model_.assetSearchFocused=false; }
    bool UpdateCatalogDrag(const Vector3& shipLocalPointer);
    bool StageCatalogDrag();
    bool CycleStagedSnapCandidate(int delta);
    bool TranslateStagedPlacement(const Vector3& delta, bool fine = false);
    bool RotateStagedPlacement(const Vector3& deltaDegrees, bool fine = false);
    bool ScaleStagedPlacement(float delta, bool fine = false);
    bool CommitCatalogDrag();
    void CancelCatalogDrag();

    bool ConsumeApplyRequested();
    bool ConsumeProjectToolRequest(std::size_t* commandIndex = nullptr);
    bool ConsumeSaveRequested();
    bool ConsumeSocketOverridesSaveRequested();
    bool ConsumeDefinitionOverridesSaveRequested();
    void MarkApplied();
    void MarkSaved(const std::string& path);
    void MarkSocketOverridesSaved(const std::string& path);
    void MarkDefinitionOverridesSaved(const std::string& path);

    static ShipyardBuilderLayout Layout(int viewportWidth, int viewportHeight);
    static ShipyardBuilderLayout Layout(const ShipyardBuilderRuntimeModel& model, int viewportWidth, int viewportHeight);
    static ShipyardBuilderLayout LegacyLayout(int viewportWidth, int viewportHeight);
    static std::vector<ShipyardBuilderControl> BuildControls(const ShipyardBuilderRuntimeModel& model,
                                                             int viewportWidth,
                                                             int viewportHeight);
    static std::vector<ShipyardBuilderControl> LegacyBuildControls(const ShipyardBuilderRuntimeModel& model,
                                                                   int viewportWidth,
                                                                   int viewportHeight);
    static ShipyardBuilderControl HitTest(const ShipyardBuilderRuntimeModel& model,
                                          int viewportWidth,
                                          int viewportHeight,
                                          float x,
                                          float y);
    static ShipyardBuilderControl LegacyHitTest(const ShipyardBuilderRuntimeModel& model,
                                                int viewportWidth,
                                                int viewportHeight,
                                                float x,
                                                float y);

private:
    bool ActivateInternal(ShipyardBuilderCommand command, int value);
    static bool RecordsAuthoringHistory(ShipyardBuilderCommand command);
    void PushAuthoringSnapshot(const ShipyardBuilderRuntimeModel& before);
    void RestoreAuthoringSnapshot(ShipyardBuilderRuntimeModel snapshot, const char* action);
    const ShipyardModuleRecord* FindRecord(const std::string& moduleId) const;
    bool AddSelectedModule();
    bool ReplaceSelectedModule();
    ShipyardModuleRecord* MutableSelectedPlacedRecord();
    const ShipyardModuleRecord* SelectedPlacedRecord() const;
    bool AddSocket();
    bool RemoveSocket();
    bool MirrorSelectedSocketX();
    bool CycleSelectedSocketType();
    bool ResetSelectedSocket();
    bool CycleSelectedSemantic(int delta);
    bool ToggleSelectedGeneratorEligibility();
    bool ToggleSelectedPairedPlacement();
    bool CycleSelectedPreferredMountFace();
    bool ResetSelectedDefinitionOverride();
    void PushSocketHistory(const std::string& moduleId,
                           const std::vector<ShipyardAssemblySocket>& before,
                           const std::vector<ShipyardAssemblySocket>& after);
    void SyncSocketSelection();
    void ReflowRecipeAttachments();
    bool RemoveSelectedModule();
    bool DetachSelectedModule();
    bool ReattachSelectedModuleToNearest();
    bool ToggleSelectedArticulation();
    bool UseSelectedSocketAsArticulationPivot();
    bool CycleSelectedArticulationMode();
    bool AdjustSelectedArticulationSpeed(float delta);
    bool GenerateInteriorProgram();
    bool MirrorSelectedSubtreeX();
    bool MirrorSelectedAcrossActiveSymmetry();
    bool BreakSelectedSymmetryPair();
    bool SyncSymmetryPartner(std::size_t changedIndex);
    void RefreshDragSymmetryPreview();
    bool GenerateVariant();
    bool IsSocketOccupied(std::size_t parentIndex, const ShipyardAssemblySocket& socket) const;
    bool TryAttach(const ShipyardModuleRecord& child,
                   VisualModulePlacement& outPlacement,
                   std::size_t& outParentIndex,
                   std::string& outParentSocket) const;
    void NormalizeSelections();
    void SyncCatalogSelectionToPlaced();
    void RefreshForwardAuthority();
    void InvalidateRecipeMetadata();
    void ApplyLiveryPreset();
    bool AddSelectedDecal();
    bool RemoveSelectedDecal();

    struct SocketEditHistoryEntry {
        std::string moduleId;
        std::vector<ShipyardAssemblySocket> before;
        std::vector<ShipyardAssemblySocket> after;
    };
    struct SocketTransformTransaction {
        bool active = false;
        std::string moduleId;
        std::size_t socketIndex = 0;
        ShipyardAssemblySocket original{};
        ShipyardAssemblySocket working{};
    };

    ShipyardBuilderRuntimeModel model_{};
    std::vector<ShipyardBuilderRuntimeModel> authoringUndo_{};
    std::vector<ShipyardBuilderRuntimeModel> authoringRedo_{};
    std::optional<ShipyardBuilderRuntimeModel> pendingTransformHistory_{};
    std::optional<ShipyardBuilderRuntimeModel> pendingSocketTransformHistory_{};
    std::optional<ShipyardBuilderRuntimeModel> pendingDragHistory_{};
    ProceduralShipVisualRecipe initialRecipe_{};
    std::vector<ShipyardModuleRecord> initialCatalog_{};
    std::vector<SocketEditHistoryEntry> socketHistory_{};
    std::size_t socketHistoryCursor_ = 0;
    SocketTransformTransaction socketTransform_{};
    ShipAppearanceState initialAppearance_{};
    bool applyRequested_ = false;
    bool saveRequested_ = false;
    bool socketOverridesSaveRequested_ = false;
    bool definitionOverridesSaveRequested_ = false;
    int pendingProjectToolIndex_ = -1;
    std::string dockDragPanelId_;
    SubspaceUiRect dockDragStartRect_{};
    bool dockDragFloating_ = false;
    std::vector<std::string> availableModuleIds_;
};

} // namespace subspace

#define SUBSPACE_SHIPYARD_BUILDER_SYSTEM_DECLARED 1
