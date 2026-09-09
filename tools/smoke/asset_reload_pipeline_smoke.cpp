#include "developer/AssetHotReloadService.h"
#include "developer/adapters/AssetRuntimeEditAdapter.h"
#include "developer/assets/AssetReloadPipeline.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

using namespace subspace;

namespace {

void WriteBytes(const std::filesystem::path& path, const std::string& bytes)
{
    std::ofstream stream(path, std::ios::binary);
    stream.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}

RuntimeEditCommand MakeCommand(std::string name, const std::filesystem::path& path, std::string kind = {})
{
    RuntimeEditCommand command;
    command.name = std::move(name);
    command.source = "asset-reload-pipeline-smoke";
    command.SetArg("path", path.string());
    if (!kind.empty()) {
        command.SetArg("kind", std::move(kind));
    }
    return command;
}

} // namespace

int main()
{
    const std::filesystem::path root = std::filesystem::temp_directory_path() / "subspace_asset_reload_pipeline_smoke";
    std::filesystem::create_directories(root);

    const std::filesystem::path texturePath = root / "test_texture.png";
    const std::filesystem::path badTexturePath = root / "bad_texture.txt";
    const std::filesystem::path materialPath = root / "test_material.json";

    WriteBytes(texturePath, std::string("\x89PNG\r\n\x1A\n", 8) + "fake-payload");
    WriteBytes(badTexturePath, "not-a-texture");
    WriteBytes(materialPath, "{\"name\":\"SmokeMaterial\"}\n");

    AssetHotReloadService hotReload;
    AssetRuntimeEditAdapter adapter(&hotReload);

    RuntimeEditResult watch = adapter.Apply(MakeCommand("asset.watch", texturePath, "texture"));
    assert(watch.handled);
    assert(watch.success);
    assert(hotReload.IsWatching(texturePath.string()));

    RuntimeEditResult textureReload = adapter.Apply(MakeCommand("texture.reload", texturePath));
    assert(textureReload.handled);
    assert(textureReload.success);
    assert(!textureReload.warnings.empty());

    RuntimeEditResult badTextureReload = adapter.Apply(MakeCommand("texture.reload", badTexturePath));
    assert(badTextureReload.handled);
    assert(!badTextureReload.success);

    RuntimeEditResult materialValidate = adapter.Apply(MakeCommand("asset.validate", materialPath, "material"));
    assert(materialValidate.handled);
    assert(materialValidate.success);

    bool commitCalled = false;
    adapter.GetReloadPipeline().RegisterCommitCallback("texture", [&commitCalled](const AssetReloadRequest&, const AssetReloadReport& validated) {
        commitCalled = true;
        AssetReloadReport report = validated;
        report.success = true;
        report.committed = true;
        report.message = "Texture runtime swap callback executed.";
        return report;
    });

    RuntimeEditResult committedReload = adapter.Apply(MakeCommand("texture.reload", texturePath));
    assert(committedReload.handled);
    assert(committedReload.success);
    assert(commitCalled);
    assert(committedReload.message == "Texture runtime swap callback executed.");

    std::cout << "AssetReloadPipeline smoke test passed.\n";
    return 0;
}
