#pragma once

#include "content/ShipyardModuleSystem.h"
#include "world/WorldScaleAuthoritySystem.h"

#include <string>
#include <vector>

namespace subspace {

enum class SemanticObjectPurpose {
    Generic, Seat, Table, Storage, Console, Door, Hatch, Airlock, Bed, Workbench, Fixture
};

struct FlatSurfaceSnapCandidate {
    std::string id;
    Vector3 point{};
    Vector3 normal{0,0,1};
    Vector3 tangentU{1,0,0};
    Vector3 tangentV{0,1,0};
    float spanUMeters=0.0f;
    float spanVMeters=0.0f;
    float supportingArea=0.0f;
    float confidence=0.0f;
    float gridPitchMeters=0.50f;
    float maximumInsertionMeters=0.18f;
    int gridColumns=1;
    int gridRows=1;
};

struct SemanticObjectDefinition {
    std::string id;
    SemanticObjectPurpose purpose=SemanticObjectPurpose::Generic;
    Vector3 sizeMeters{1,1,1};
    Vector3 interactionPoint{};
    Vector3 facingDirection{0,1,0};
    float interactionReachMeters=.82f;
    float approachClearanceMeters=.75f;
    float minimumHeadClearanceMeters=0.0f;
    float snapStepMeters=.25f;
    bool requiresFloorContact=true;
    bool requiresHumanClearance=true;
    bool sitInteraction=false;
    bool storageInteraction=false;
    bool consoleInteraction=false;
};

struct CohesiveAssemblyBakePolicy {
    bool unionExteriorShell=true;
    bool removeOccludedInternalExteriorFaces=true;
    bool carveWalkableInterior=true;
    bool preservePortalOpenings=true;
    bool preserveServiceVolumes=true;
    bool outputSingleRuntimeMesh=true;
    std::string interchangeFormat="OBJ";
    float weldToleranceMeters=.0025f;
    float maximumAttachmentPenetrationMeters=.18f;
    float minimumWallThicknessMeters=.06f;
    float minimumWalkableHeadroomMeters=2.10f;
    float minimumWalkableWidthMeters=1.05f;
};

struct AuthoringValidationResult {
    bool valid=false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

/// Shared physical authoring rules for Shipyard, in-game dev mode and Blender.
/// The canonical player-scale profile is the reference for all interiors and
/// interactive props; measured geometry surfaces are preferred over BBox guesses.
class AuthoringStandardsSystem {
public:
    static std::vector<FlatSurfaceSnapCandidate> DiscoverFlatSnapSurfaces(
        const ShipyardModuleRecord& record,
        float minimumArea=.10f,
        float minimumConfidence=.55f,
        const WorldScaleProfile& scale=WorldScaleAuthoritySystem::DefaultProfile());
    static SemanticObjectDefinition DefaultObject(SemanticObjectPurpose purpose,
                                                   const WorldScaleProfile& scale=WorldScaleAuthoritySystem::DefaultProfile());
    static AuthoringValidationResult ValidateObject(const SemanticObjectDefinition& object,
                                                     const WorldScaleProfile& scale=WorldScaleAuthoritySystem::DefaultProfile());
    static CohesiveAssemblyBakePolicy DefaultBakePolicy(const WorldScaleProfile& scale=WorldScaleAuthoritySystem::DefaultProfile());
    static float AllowedAttachmentPenetration(const ShipyardAssemblySocket& socket,
                                              const CohesiveAssemblyBakePolicy& policy);
    static const char* PurposeName(SemanticObjectPurpose purpose);
};

} // namespace subspace
