#include "assets/CanonicalAsset.h"
#include "assets/CanonicalAssetValidation.h"

#include <iostream>

using namespace subspace::assets;

static CanonicalAsset MakeValidFixture() {
    CanonicalAsset asset;
    asset.assetId = "fixture.engine_housing";
    asset.provenance.sourcePath = "fixture.fbx";
    asset.provenance.sourceFormat = "fbx";
    asset.provenance.importer = "ufbx";
    asset.provenance.importerVersion = "0.23.0";

    asset.textures.push_back({"base", "base.png", TextureSemantic::BaseColor, TextureColorSpace::SRGB, false});
    asset.textures.push_back({"normal", "normal.png", TextureSemantic::Normal, TextureColorSpace::Linear, false});

    PbrMaterial material;
    material.name = "fixture_pbr";
    material.baseColorTexture = 0;
    material.normalTexture = 1;
    asset.materials.push_back(material);

    MeshPrimitive primitive;
    primitive.vertices.resize(3);
    primitive.indices = {0, 1, 2};
    primitive.materialIndex = 0;
    primitive.hasNormals = true;
    primitive.hasTangents = true;
    primitive.hasUv0 = true;
    CanonicalMesh mesh;
    mesh.name = "engine_housing";
    mesh.primitives.push_back(primitive);
    asset.meshes.push_back(mesh);

    CanonicalNode node;
    node.name = "EngineHousing";
    node.meshIndex = 0;
    asset.nodes.push_back(node);

    BoxProxy mount;
    mount.id = "engine_mount_plane";
    mount.type = ProxyType::Mount;
    mount.nodeIndex = 0;
    mount.halfExtents = {0.5f, 0.5f, 0.05f};
    asset.proxies.push_back(mount);

    ModuleSocket socket;
    socket.id = "engine_cavity";
    socket.nodeIndex = 0;
    socket.type = SocketType::EngineCavity;
    socket.size = SocketSize::M;
    socket.authority = SocketAuthority::Authored;
    socket.accepts = {SocketType::EngineMount};
    socket.minInsertionMeters = 0.10f;
    socket.maxInsertionMeters = 0.35f;
    socket.mountProxyIndex = 0;
    asset.sockets.push_back(socket);

    ModuleDefinition module;
    module.moduleId = "fixture.engine_housing";
    module.role = ModuleRole::EngineHousing;
    module.rootNodeIndex = 0;
    module.socketIndices = {0};
    asset.modules.push_back(module);

    return asset;
}

int main() {
    auto valid = MakeValidFixture();
    auto validReport = CanonicalAssetValidator::Validate(valid);
    if (!validReport.Passed()) {
        std::cerr << "valid fixture failed with " << validReport.ErrorCount() << " errors\n";
        return 1;
    }

    auto broken = valid;
    broken.textures[1].colorSpace = TextureColorSpace::SRGB;
    broken.meshes[0].primitives[0].hasTangents = false;
    broken.sockets[0].accepts.clear();
    auto brokenReport = CanonicalAssetValidator::Validate(broken);
    if (brokenReport.Passed() || brokenReport.ErrorCount() < 3) {
        std::cerr << "broken fixture was not rejected strongly enough\n";
        return 2;
    }

    std::cout << "PASS425R3A asset foundation smoke PASS: "
              << validReport.ErrorCount() << " valid errors, "
              << brokenReport.ErrorCount() << " expected broken errors\n";
    return 0;
}
