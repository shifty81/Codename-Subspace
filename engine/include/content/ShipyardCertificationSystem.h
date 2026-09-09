#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

enum class ShipyardCertificationGrade {
    A,
    B,
    C,
    Reject
};

enum class ShipyardIslandRole {
    StructuralRoot,
    FunctionalComponent,
    SurfaceDetail,
    ReviewRequired
};

struct ShipyardSocketCandidate {
    std::string name;
    std::string type;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float dirX = 0.0f;
    float dirY = 0.0f;
    float dirZ = 0.0f;
    float clearanceRadius = 0.0f;
    float surfaceConfidence = 0.0f;
    bool surfaceDerived = false;
};

/// Diagnostic only. Loose geometry is inspected so bad imports can be found,
/// but authored Shipyard objects are not automatically decomposed into runtime
/// modules. The original extracted object remains the module authority.
struct ShipyardCertifiedIsland {
    std::size_t islandIndex = 0;
    ShipyardCertificationGrade grade = ShipyardCertificationGrade::Reject;
    ShipyardIslandRole role = ShipyardIslandRole::ReviewRequired;
    std::string moduleClass = "component";
    std::size_t vertexCount = 0;
    std::size_t triangleCount = 0;
    float minX = 0.0f, maxX = 0.0f;
    float minY = 0.0f, maxY = 0.0f;
    float minZ = 0.0f, maxZ = 0.0f;
    float volumeRatio = 0.0f;
    float triangleRatio = 0.0f;
    bool primary = false;
    std::vector<int> sourceVertexIndices;
    std::vector<int> sourceTriangleIndices;
};

struct ShipyardCertifiedMaterial {
    std::string name;
    bool resolvedFromMtl = false;
    bool hasBaseColorTexture = false;
    bool hasNormalTexture = false;
    bool hasMetallicTexture = false;
    bool hasRoughnessTexture = false;
    bool hasEmissiveTexture = false;
};

struct ShipyardObjCertification {
    std::string sourceName;
    std::string sourceClass;
    std::vector<float> positions; // xyz triples
    std::vector<int> triangles;   // source vertex indices, 0-based, triples
    std::vector<ShipyardCertifiedIsland> islands; // diagnostic connectivity only
    std::vector<ShipyardSocketCandidate> sockets; // whole authored-module sockets
    std::vector<std::string> materialLibraries;
    std::vector<ShipyardCertifiedMaterial> materials;
    bool hasTexcoords = false;
    bool hasNormals = false;
    ShipyardCertificationGrade moduleGrade = ShipyardCertificationGrade::Reject;
    bool preserveAuthoredObject = true;
    bool valid = false;
    std::string error;
};

struct ShipyardCorpusCertificationSummary {
    int sourceObjects = 0;
    int gradeA = 0;
    int gradeB = 0;
    int gradeC = 0;
    int rejected = 0;
    int multiIslandSourceObjects = 0;
    int modulesWithMaterialSlots = 0;
    int modulesWithUnresolvedMaterialAuthority = 0;
    int materialSlots = 0;
    int resolvedMaterials = 0;
};

/// Pass425R3V1 Shipyard module classification/certification authority.
///
/// Shipyard's extracted objects are already authored kitbash pieces. R2 keeps
/// each authored OBJ intact and certifies how Subspace is allowed to use it.
/// Connected-component analysis is retained only as a diagnostic signal. It
/// does not silently split, delete, weld, or redesign authored geometry.
class ShipyardCertificationSystem {
public:
    static ShipyardObjCertification AnalyzeObj(const std::string& objText,
                                                const std::string& sourceName,
                                                const std::string& sourceClass);

    static std::vector<ShipyardSocketCandidate> SuggestModuleSockets(const ShipyardObjCertification& module);

    static const char* GradeName(ShipyardCertificationGrade grade);
    static const char* RoleName(ShipyardIslandRole role);

    /// Certify every normalized OBJ under inputRoot. Grade-A output is a
    /// byte-for-byte copy of the normalized authored object with classification,
    /// socket and connectivity-diagnostic metadata alongside it.
    static bool CertifyCorpus(const std::string& inputRoot,
                              const std::string& outputRoot,
                              ShipyardCorpusCertificationSummary* summary,
                              std::string* error = nullptr);
};

} // namespace subspace
