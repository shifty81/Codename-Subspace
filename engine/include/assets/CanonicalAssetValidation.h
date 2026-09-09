#pragma once

#include "assets/CanonicalAsset.h"

#include <cmath>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace subspace::assets {

enum class ValidationSeverity : std::uint8_t {
    Info,
    Warning,
    Error,
};

struct ValidationIssue {
    ValidationSeverity severity = ValidationSeverity::Error;
    std::string code;
    std::string path;
    std::string message;
};

struct CanonicalAssetValidationReport {
    std::vector<ValidationIssue> issues;

    bool Passed() const {
        for (const auto& issue : issues) {
            if (issue.severity == ValidationSeverity::Error) return false;
        }
        return true;
    }

    std::size_t ErrorCount() const {
        std::size_t count = 0;
        for (const auto& issue : issues) {
            if (issue.severity == ValidationSeverity::Error) ++count;
        }
        return count;
    }
};

class CanonicalAssetValidator {
public:
    static CanonicalAssetValidationReport Validate(const CanonicalAsset& asset) {
        CanonicalAssetValidationReport report;

        if (asset.schemaVersion != kCanonicalAssetSchemaVersion) {
            AddError(report, "CERT_SCHEMA_VERSION", "asset.schemaVersion",
                "Unsupported canonical asset schema version.");
        }
        if (asset.assetId.empty()) {
            AddError(report, "CERT_ASSET_ID_MISSING", "asset.assetId",
                "Canonical assets require a stable asset id.");
        }
        if (!asset.rightHanded || std::fabs(asset.metersPerUnit - 1.0f) > 0.0001f ||
            asset.coordinateBasis != CoordinateBasis::XRightYForwardZUp) {
            AddError(report, "CERT_COORDINATE_SPACE", "asset.coordinateSpace",
                "Certified assets must be right-handed meters in +X right / +Y forward / +Z up Subspace space.");
        }
        if (asset.provenance.importPolicy != "PRESERVE_AUTHORED_OBJECTS") {
            AddError(report, "CERT_IMPORT_POLICY", "asset.provenance.importPolicy",
                "Shipyard certification must preserve authored object boundaries.");
        }

        ValidateTextures(asset, report);
        ValidateMaterials(asset, report);
        ValidateMeshes(asset, report);
        ValidateNodes(asset, report);
        ValidateProxies(asset, report);
        ValidateSockets(asset, report);
        ValidateModules(asset, report);

        return report;
    }

private:
    static bool TextureIndexValid(const CanonicalAsset& asset, AssetIndex index) {
        return index == kInvalidAssetIndex || index < asset.textures.size();
    }

    static bool MaterialIndexValid(const CanonicalAsset& asset, AssetIndex index) {
        return index == kInvalidAssetIndex || index < asset.materials.size();
    }

    static bool NodeIndexValid(const CanonicalAsset& asset, AssetIndex index) {
        return index == kInvalidAssetIndex || index < asset.nodes.size();
    }

    static bool ProxyIndexValid(const CanonicalAsset& asset, AssetIndex index) {
        return index == kInvalidAssetIndex || index < asset.proxies.size();
    }

    static bool SocketIndexValid(const CanonicalAsset& asset, AssetIndex index) {
        return index < asset.sockets.size();
    }

    static void ValidateTextures(const CanonicalAsset& asset, CanonicalAssetValidationReport& report) {
        for (std::size_t i = 0; i < asset.textures.size(); ++i) {
            const auto& texture = asset.textures[i];
            const std::string path = "textures[" + std::to_string(i) + "]";
            if (texture.id.empty()) {
                AddError(report, "CERT_TEXTURE_ID_MISSING", path + ".id", "Texture id is required.");
            }
            if (texture.path.empty() && !texture.embedded) {
                AddError(report, "CERT_MATERIAL_TEXTURE_MISSING", path + ".path",
                    "Non-embedded texture has no source path.");
            }

            const bool expectsSrgb = texture.semantic == TextureSemantic::BaseColor ||
                                     texture.semantic == TextureSemantic::Emissive;
            const bool expectsLinear = texture.semantic == TextureSemantic::MetallicRoughness ||
                                       texture.semantic == TextureSemantic::Normal ||
                                       texture.semantic == TextureSemantic::Occlusion;
            if (expectsSrgb && texture.colorSpace != TextureColorSpace::SRGB) {
                AddError(report, "CERT_INVALID_TEXTURE_COLORSPACE", path + ".colorSpace",
                    "BaseColor and Emissive textures must be tagged sRGB.");
            }
            if (expectsLinear && texture.colorSpace != TextureColorSpace::Linear) {
                AddError(report, "CERT_INVALID_TEXTURE_COLORSPACE", path + ".colorSpace",
                    "Normal, MetallicRoughness, and Occlusion textures must be tagged linear.");
            }
        }
    }

    static void ValidateMaterials(const CanonicalAsset& asset, CanonicalAssetValidationReport& report) {
        for (std::size_t i = 0; i < asset.materials.size(); ++i) {
            const auto& material = asset.materials[i];
            const std::string path = "materials[" + std::to_string(i) + "]";
            const std::array<std::pair<AssetIndex, const char*>, 5> refs{{
                {material.baseColorTexture, "baseColorTexture"},
                {material.metallicRoughnessTexture, "metallicRoughnessTexture"},
                {material.normalTexture, "normalTexture"},
                {material.occlusionTexture, "occlusionTexture"},
                {material.emissiveTexture, "emissiveTexture"},
            }};
            for (const auto& ref : refs) {
                if (!TextureIndexValid(asset, ref.first)) {
                    AddError(report, "CERT_TEXTURE_INDEX", path + "." + ref.second,
                        "Material references a texture outside the canonical texture table.");
                }
            }
            if (material.roughnessFactor < 0.0f || material.roughnessFactor > 1.0f) {
                AddError(report, "CERT_ROUGHNESS_RANGE", path + ".roughnessFactor",
                    "Roughness factor must be in [0,1].");
            }
            if (material.metallicFactor < 0.0f || material.metallicFactor > 1.0f) {
                AddError(report, "CERT_METALLIC_RANGE", path + ".metallicFactor",
                    "Metallic factor must be in [0,1].");
            }
        }
    }

    static void ValidateMeshes(const CanonicalAsset& asset, CanonicalAssetValidationReport& report) {
        for (std::size_t meshIndex = 0; meshIndex < asset.meshes.size(); ++meshIndex) {
            const auto& mesh = asset.meshes[meshIndex];
            for (std::size_t primitiveIndex = 0; primitiveIndex < mesh.primitives.size(); ++primitiveIndex) {
                const auto& primitive = mesh.primitives[primitiveIndex];
                const std::string path = "meshes[" + std::to_string(meshIndex) + "].primitives[" +
                                         std::to_string(primitiveIndex) + "]";
                if (primitive.vertices.empty() || primitive.indices.empty()) {
                    AddError(report, "CERT_EMPTY_PRIMITIVE", path,
                        "Renderable primitives require vertices and indices.");
                    continue;
                }
                if (primitive.indices.size() % 3 != 0) {
                    AddError(report, "CERT_NON_TRIANGULATED", path + ".indices",
                        "Certified runtime primitives must be triangulated.");
                }
                for (const auto index : primitive.indices) {
                    if (index >= primitive.vertices.size()) {
                        AddError(report, "CERT_INDEX_RANGE", path + ".indices",
                            "Primitive index references a missing vertex.");
                        break;
                    }
                }
                if (!MaterialIndexValid(asset, primitive.materialIndex)) {
                    AddError(report, "CERT_MATERIAL_INDEX", path + ".materialIndex",
                        "Primitive references a material outside the canonical material table.");
                }
                if (primitive.materialIndex != kInvalidAssetIndex &&
                    primitive.materialIndex < asset.materials.size()) {
                    const auto& material = asset.materials[primitive.materialIndex];
                    if (material.normalTexture != kInvalidAssetIndex && !primitive.hasUv0) {
                        AddError(report, "CERT_NORMAL_REQUIRES_UV", path + ".hasUv0",
                            "Normal-mapped primitives require UV0.");
                    }
                    if (material.normalTexture != kInvalidAssetIndex && !primitive.hasTangents) {
                        AddError(report, "CERT_NORMAL_REQUIRES_TANGENT", path + ".hasTangents",
                            "Normal-mapped primitives require certified tangents (authored or MikkTSpace-generated).");
                    }
                }
            }
        }
    }

    static void ValidateNodes(const CanonicalAsset& asset, CanonicalAssetValidationReport& report) {
        for (std::size_t i = 0; i < asset.nodes.size(); ++i) {
            const auto& node = asset.nodes[i];
            const std::string path = "nodes[" + std::to_string(i) + "]";
            if (node.parentIndex != kInvalidAssetIndex && node.parentIndex >= asset.nodes.size()) {
                AddError(report, "CERT_NODE_PARENT", path + ".parentIndex", "Node parent is out of range.");
            }
            if (node.meshIndex != kInvalidAssetIndex && node.meshIndex >= asset.meshes.size()) {
                AddError(report, "CERT_NODE_MESH", path + ".meshIndex", "Node mesh is out of range.");
            }
        }
    }

    static void ValidateProxies(const CanonicalAsset& asset, CanonicalAssetValidationReport& report) {
        for (std::size_t i = 0; i < asset.proxies.size(); ++i) {
            const auto& proxy = asset.proxies[i];
            const std::string path = "proxies[" + std::to_string(i) + "]";
            if (!NodeIndexValid(asset, proxy.nodeIndex) || proxy.nodeIndex == kInvalidAssetIndex) {
                AddError(report, "CERT_PROXY_NODE", path + ".nodeIndex", "Proxy must belong to a valid node.");
            }
            if (proxy.halfExtents.x <= 0.0f || proxy.halfExtents.y <= 0.0f || proxy.halfExtents.z <= 0.0f) {
                AddError(report, "CERT_PROXY_EXTENTS", path + ".halfExtents", "Proxy half extents must be positive.");
            }
        }
    }

    static void ValidateSockets(const CanonicalAsset& asset, CanonicalAssetValidationReport& report) {
        std::unordered_set<std::string> ids;
        for (std::size_t i = 0; i < asset.sockets.size(); ++i) {
            const auto& socket = asset.sockets[i];
            const std::string path = "sockets[" + std::to_string(i) + "]";
            if (socket.id.empty() || !ids.insert(socket.id).second) {
                AddError(report, "CERT_SOCKET_ID", path + ".id", "Socket ids must be non-empty and unique.");
            }
            if (!NodeIndexValid(asset, socket.nodeIndex) || socket.nodeIndex == kInvalidAssetIndex) {
                AddError(report, "CERT_SOCKET_NODE", path + ".nodeIndex", "Socket must belong to a valid node.");
            }
            if (socket.type == SocketType::Unknown) {
                AddError(report, "CERT_SOCKET_TYPE", path + ".type", "Certified sockets require a typed role.");
            }
            if (socket.accepts.empty()) {
                AddError(report, "CERT_SOCKET_COMPATIBILITY", path + ".accepts",
                    "Certified sockets must declare compatible mating socket types.");
            }
            if (socket.minInsertionMeters < 0.0f || socket.maxInsertionMeters < socket.minInsertionMeters) {
                AddError(report, "CERT_SOCKET_INSERTION", path + ".insertion",
                    "Socket insertion range is invalid.");
            }
            if (!ProxyIndexValid(asset, socket.mountProxyIndex)) {
                AddError(report, "CERT_SOCKET_MOUNT_PROXY", path + ".mountProxyIndex", "Socket mount proxy is invalid.");
            }
            if (!ProxyIndexValid(asset, socket.clearanceProxyIndex)) {
                AddError(report, "CERT_SOCKET_CLEARANCE_PROXY", path + ".clearanceProxyIndex", "Socket clearance proxy is invalid.");
            }
            if (!ProxyIndexValid(asset, socket.keepoutProxyIndex)) {
                AddError(report, "CERT_SOCKET_KEEPOUT_PROXY", path + ".keepoutProxyIndex", "Socket keepout proxy is invalid.");
            }
        }
    }

    static void ValidateModules(const CanonicalAsset& asset, CanonicalAssetValidationReport& report) {
        std::unordered_set<std::string> ids;
        for (std::size_t i = 0; i < asset.modules.size(); ++i) {
            const auto& module = asset.modules[i];
            const std::string path = "modules[" + std::to_string(i) + "]";
            if (module.moduleId.empty() || !ids.insert(module.moduleId).second) {
                AddError(report, "CERT_MODULE_ID", path + ".moduleId", "Module ids must be non-empty and unique.");
            }
            if (!NodeIndexValid(asset, module.rootNodeIndex) || module.rootNodeIndex == kInvalidAssetIndex) {
                AddError(report, "CERT_MODULE_ROOT", path + ".rootNodeIndex", "Module requires a valid authored root node.");
            }
            for (const auto socketIndex : module.socketIndices) {
                if (!SocketIndexValid(asset, socketIndex)) {
                    AddError(report, "CERT_MODULE_SOCKET", path + ".socketIndices", "Module references an invalid socket.");
                    break;
                }
            }
        }
    }

    static void AddError(CanonicalAssetValidationReport& report, std::string code,
                         std::string path, std::string message) {
        report.issues.push_back({ValidationSeverity::Error, std::move(code),
                                 std::move(path), std::move(message)});
    }
};

} // namespace subspace::assets
