#pragma once

#include "core/Math.h"

#include <array>
#include <string>
#include <vector>

namespace subspace {

struct ObjTexCoord { float u=0.0f; float v=0.0f; };
struct ObjTriangle {
    std::array<int,3> position{{0,0,0}};
    std::array<int,3> texcoord{{-1,-1,-1}};
    std::array<int,3> normal{{-1,-1,-1}};
    int materialIndex = -1;
};

struct ObjMaterialData {
    std::string name;
    float diffuseR = 1.0f;
    float diffuseG = 1.0f;
    float diffuseB = 1.0f;
    float alpha = 1.0f;
    float emissiveR = 0.0f;
    float emissiveG = 0.0f;
    float emissiveB = 0.0f;
    float shininess = 48.0f;
    float metallic = -1.0f;
    float roughness = -1.0f;
    bool resolvedFromMtl = false;
    bool hasDiffuseColor = false;
    std::string baseColorTexturePath;
    std::string normalTexturePath;
    std::string metallicTexturePath;
    std::string roughnessTexturePath;
    std::string emissiveTexturePath;
};

struct ObjMeshData {
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<ObjTexCoord> texcoords;
    std::vector<ObjTriangle> triangles;
    std::vector<std::string> materialLibraries;
    std::vector<std::string> materialNames;
    std::vector<ObjMaterialData> materials;
};

class ObjAssetLoader {
public:
    bool LoadFile(const std::string& path, ObjMeshData& out, std::string* error = nullptr) const;
    bool Parse(const std::string& text, ObjMeshData& out, std::string* error = nullptr) const;
};

} // namespace subspace
