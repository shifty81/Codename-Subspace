#include "developer/assets/AssetReloadPipeline.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <sstream>
#include <utility>

namespace subspace {
namespace {

std::string ToLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string LowerExtension(const std::filesystem::path& path)
{
    return ToLower(path.extension().string());
}

bool ContainsExtension(const std::vector<std::string>& extensions, const std::string& extension)
{
    return std::find(extensions.begin(), extensions.end(), extension) != extensions.end();
}

bool ReadPrefix(const std::filesystem::path& path, std::vector<unsigned char>& out, std::size_t count)
{
    out.clear();
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        return false;
    }
    out.resize(count);
    stream.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(out.size()));
    out.resize(static_cast<std::size_t>(std::max<std::streamsize>(0, stream.gcount())));
    return !out.empty();
}

bool LooksLikePng(const std::vector<unsigned char>& data)
{
    static constexpr std::array<unsigned char, 8> sig = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    return data.size() >= sig.size() && std::equal(sig.begin(), sig.end(), data.begin());
}

bool LooksLikeJpeg(const std::vector<unsigned char>& data)
{
    return data.size() >= 3 && data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF;
}

bool LooksLikeGlb(const std::vector<unsigned char>& data)
{
    return data.size() >= 4 && data[0] == 'g' && data[1] == 'l' && data[2] == 'T' && data[3] == 'F';
}

bool FirstNonSpaceIs(const std::filesystem::path& path, char expected)
{
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        return false;
    }
    char ch = '\0';
    while (stream.get(ch)) {
        if (!std::isspace(static_cast<unsigned char>(ch))) {
            return ch == expected;
        }
    }
    return false;
}

AssetReloadHandler MakeFileHandler(std::string kind, std::vector<std::string> extensions)
{
    AssetReloadHandler handler;
    handler.kind = std::move(kind);
    handler.allowedExtensions = std::move(extensions);
    handler.requiresExistingFile = true;
    handler.commitWithoutCallbackSucceeds = true;
    return handler;
}

void AppendWarnings(std::vector<std::string>& target, const std::vector<std::string>& source)
{
    target.insert(target.end(), source.begin(), source.end());
}

} // namespace

RuntimeEditResult AssetReloadReport::ToRuntimeEditResult(RuntimeEditCommand command) const
{
    RuntimeEditResult result;
    result.handled = handled;
    result.success = success;
    result.undoable = false;
    result.message = message;
    result.command = std::move(command);
    result.warnings = warnings;
    return result;
}

AssetReloadPipeline::AssetReloadPipeline()
{
    RegisterDefaultFileHandlers();
}

void AssetReloadPipeline::RegisterHandler(AssetReloadHandler handler)
{
    handler.kind = NormalizeKind(handler.kind);
    if (!handler.kind.empty()) {
        _handlers[handler.kind] = std::move(handler);
    }
}

void AssetReloadPipeline::RegisterCommitCallback(const std::string& kind, AssetReloadHandler::CommitCallback callback)
{
    const std::string normalized = NormalizeKind(kind);
    auto found = _handlers.find(normalized);
    if (found == _handlers.end()) {
        AssetReloadHandler handler;
        handler.kind = normalized;
        handler.commit = std::move(callback);
        _handlers[normalized] = std::move(handler);
        return;
    }
    found->second.commit = std::move(callback);
}

void AssetReloadPipeline::ClearHandlers()
{
    _handlers.clear();
}

void AssetReloadPipeline::RegisterDefaultFileHandlers()
{
    RegisterHandler(MakeFileHandler("texture", {".png", ".jpg", ".jpeg", ".tga", ".bmp", ".dds", ".ktx", ".ktx2"}));
    RegisterHandler(MakeFileHandler("model", {".obj", ".fbx", ".glb", ".gltf"}));
    RegisterHandler(MakeFileHandler("material", {".json", ".material", ".mat", ".yaml", ".yml", ".toml"}));
    RegisterHandler(MakeFileHandler("shader", {".glsl", ".vert", ".frag", ".geom", ".comp", ".hlsl", ".fx"}));
    RegisterHandler(MakeFileHandler("data", {".json", ".yaml", ".yml", ".toml", ".ron", ".csv"}));
    RegisterHandler(MakeFileHandler("tuning", {".json", ".yaml", ".yml", ".toml", ".ron"}));
    RegisterHandler(MakeFileHandler("script", {".lua", ".js", ".ts", ".py", ".nut", ".csx"}));
    RegisterHandler(MakeFileHandler("audio", {".wav", ".ogg", ".mp3", ".flac"}));
    RegisterHandler(MakeFileHandler("ship.blueprint", {".json", ".blueprint", ".ship", ".toml", ".ron"}));
    RegisterHandler(MakeFileHandler("generic", {}));
}

AssetReloadReport AssetReloadPipeline::Reload(const AssetReloadRequest& request) const
{
    return Run(request, false);
}

AssetReloadReport AssetReloadPipeline::Validate(const AssetReloadRequest& request) const
{
    return Run(request, true);
}

bool AssetReloadPipeline::HasHandler(const std::string& kind) const
{
    return FindHandler(kind) != nullptr;
}

std::vector<std::string> AssetReloadPipeline::GetRegisteredKinds() const
{
    std::vector<std::string> kinds;
    kinds.reserve(_handlers.size());
    for (const auto& pair : _handlers) {
        kinds.push_back(pair.first);
    }
    std::sort(kinds.begin(), kinds.end());
    return kinds;
}

std::string AssetReloadPipeline::InferKindFromPath(const std::filesystem::path& path)
{
    const std::string ext = LowerExtension(path);
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp" || ext == ".dds" || ext == ".ktx" || ext == ".ktx2") {
        return "texture";
    }
    if (ext == ".obj" || ext == ".fbx" || ext == ".glb" || ext == ".gltf") {
        return "model";
    }
    if (ext == ".glsl" || ext == ".vert" || ext == ".frag" || ext == ".geom" || ext == ".comp" || ext == ".hlsl" || ext == ".fx") {
        return "shader";
    }
    if (ext == ".wav" || ext == ".ogg" || ext == ".mp3" || ext == ".flac") {
        return "audio";
    }
    if (ext == ".lua" || ext == ".js" || ext == ".ts" || ext == ".py" || ext == ".nut" || ext == ".csx") {
        return "script";
    }
    if (ext == ".material" || ext == ".mat") {
        return "material";
    }
    if (ext == ".blueprint" || ext == ".ship") {
        return "ship.blueprint";
    }
    if (ext == ".json" || ext == ".yaml" || ext == ".yml" || ext == ".toml" || ext == ".ron" || ext == ".csv") {
        return "data";
    }
    return "generic";
}

std::string AssetReloadPipeline::InferKindFromCommand(const std::string& commandName)
{
    if (commandName == "texture.reload") return "texture";
    if (commandName == "model.reload") return "model";
    if (commandName == "material.reload") return "material";
    if (commandName == "shader.reload") return "shader";
    if (commandName == "data.reload") return "data";
    if (commandName == "tuning.reload") return "tuning";
    if (commandName == "script.reload") return "script";
    if (commandName == "audio.reload") return "audio";
    if (commandName == "ship.blueprint.reload") return "ship.blueprint";
    return {};
}

std::string AssetReloadPipeline::NormalizeKind(std::string kind)
{
    kind = ToLower(std::move(kind));
    if (kind == "textures") return "texture";
    if (kind == "models" || kind == "mesh" || kind == "meshes" || kind == "glb" || kind == "gltf") return "model";
    if (kind == "materials") return "material";
    if (kind == "shaders") return "shader";
    if (kind == "json" || kind == "game.data") return "data";
    if (kind == "balance") return "tuning";
    if (kind == "scripts") return "script";
    if (kind == "sounds" || kind == "sound") return "audio";
    if (kind == "blueprint" || kind == "ship_blueprint" || kind == "ship-blueprint") return "ship.blueprint";
    return kind;
}

AssetReloadReport AssetReloadPipeline::Run(const AssetReloadRequest& originalRequest, bool validateOnly) const
{
    AssetReloadRequest request = originalRequest;
    request.kind = NormalizeKind(request.kind);
    request.validateOnly = request.validateOnly || validateOnly;

    if (request.kind.empty()) {
        request.kind = NormalizeKind(InferKindFromCommand(request.commandName));
    }
    if (request.kind.empty() || request.kind == "generic") {
        const std::string inferred = InferKindFromPath(request.path);
        if (!inferred.empty()) {
            request.kind = NormalizeKind(inferred);
        }
    }
    if (request.kind.empty()) {
        request.kind = "generic";
    }

    const AssetReloadHandler* handler = FindHandler(request.kind);
    if (!handler) {
        handler = FindHandler("generic");
    }

    if (!handler) {
        AssetReloadReport failure;
        failure.handled = true;
        failure.success = false;
        failure.stage = AssetReloadStage::Failed;
        failure.assetId = request.assetId;
        failure.path = request.path;
        failure.kind = request.kind;
        failure.message = "No asset reload handler is registered for kind '" + request.kind + "'.";
        return failure;
    }

    AssetReloadReport staged = handler->stage ? handler->stage(request) : StageFileAsset(request, *handler);
    staged.handled = true;
    staged.assetId = request.assetId;
    staged.path = request.path;
    staged.kind = request.kind;
    if (!staged.success) {
        staged.stage = AssetReloadStage::Failed;
        return staged;
    }

    AssetReloadReport validated = handler->validate ? handler->validate(request, staged) : ValidateFileAsset(request, *handler, staged);
    validated.handled = true;
    validated.assetId = request.assetId;
    validated.path = request.path;
    validated.kind = request.kind;
    validated.staged = staged.staged;
    validated.byteSize = staged.byteSize;
    AppendWarnings(validated.warnings, staged.warnings);
    if (!validated.success) {
        validated.stage = AssetReloadStage::Failed;
        return validated;
    }

    if (request.validateOnly) {
        validated.committed = false;
        validated.stage = AssetReloadStage::Validated;
        if (validated.message.empty()) {
            validated.message = "Asset '" + (request.assetId.empty() ? request.path.string() : request.assetId) + "' validated as kind '" + request.kind + "'.";
        }
        return validated;
    }

    AssetReloadReport committed = handler->commit ? handler->commit(request, validated) : CommitFileAsset(request, *handler, validated);
    committed.handled = true;
    committed.assetId = request.assetId;
    committed.path = request.path;
    committed.kind = request.kind;
    committed.staged = validated.staged;
    committed.validated = validated.validated;
    committed.byteSize = validated.byteSize;
    AppendWarnings(committed.warnings, validated.warnings);
    committed.stage = committed.success ? AssetReloadStage::Committed : AssetReloadStage::Failed;
    return committed;
}

AssetReloadReport AssetReloadPipeline::StageFileAsset(const AssetReloadRequest& request, const AssetReloadHandler& handler) const
{
    AssetReloadReport report;
    report.handled = true;
    report.assetId = request.assetId;
    report.path = request.path;
    report.kind = request.kind;

    if (request.path.empty()) {
        report.success = false;
        report.stage = AssetReloadStage::Failed;
        report.message = "Asset reload requires path=... until an asset registry can resolve id-only reloads.";
        return report;
    }

    std::error_code ec;
    if (handler.requiresExistingFile && !std::filesystem::exists(request.path, ec)) {
        report.success = false;
        report.stage = AssetReloadStage::Failed;
        report.message = "Asset path does not exist: " + request.path.string();
        return report;
    }
    if (handler.requiresExistingFile && !std::filesystem::is_regular_file(request.path, ec)) {
        report.success = false;
        report.stage = AssetReloadStage::Failed;
        report.message = "Asset path is not a regular file: " + request.path.string();
        return report;
    }

    if (handler.requiresExistingFile) {
        report.byteSize = std::filesystem::file_size(request.path, ec);
        if (ec || report.byteSize == 0) {
            report.success = false;
            report.stage = AssetReloadStage::Failed;
            report.message = "Asset file is empty or unreadable: " + request.path.string();
            return report;
        }
    }

    report.success = true;
    report.staged = true;
    report.stage = AssetReloadStage::Staged;
    report.message = "Asset staged for reload: " + request.path.string();
    return report;
}

AssetReloadReport AssetReloadPipeline::ValidateFileAsset(const AssetReloadRequest& request,
                                                         const AssetReloadHandler& handler,
                                                         const AssetReloadReport& staged) const
{
    AssetReloadReport report = staged;
    report.validated = false;
    report.stage = AssetReloadStage::Validated;

    const std::string extension = LowerExtension(request.path);
    if (!handler.allowedExtensions.empty() && !ContainsExtension(handler.allowedExtensions, extension)) {
        report.success = false;
        report.message = "Asset kind '" + request.kind + "' does not accept extension '" + extension + "'.";
        return report;
    }

    std::vector<unsigned char> prefix;
    if (extension == ".png") {
        if (!ReadPrefix(request.path, prefix, 8) || !LooksLikePng(prefix)) {
            report.success = false;
            report.message = "PNG texture failed signature validation: " + request.path.string();
            return report;
        }
    } else if (extension == ".jpg" || extension == ".jpeg") {
        if (!ReadPrefix(request.path, prefix, 3) || !LooksLikeJpeg(prefix)) {
            report.success = false;
            report.message = "JPEG texture failed signature validation: " + request.path.string();
            return report;
        }
    } else if (extension == ".glb") {
        if (!ReadPrefix(request.path, prefix, 4) || !LooksLikeGlb(prefix)) {
            report.success = false;
            report.message = "GLB model failed signature validation: " + request.path.string();
            return report;
        }
    } else if (extension == ".gltf" || extension == ".json") {
        if (!FirstNonSpaceIs(request.path, '{')) {
            report.success = false;
            report.message = "JSON-like asset failed basic object validation: " + request.path.string();
            return report;
        }
    }

    if (request.kind == "shader") {
        report.warnings.push_back("Shader text was staged, but no renderer shader-compile callback is registered yet.");
    }
    if (request.kind == "model") {
        report.warnings.push_back("Model file was staged, but mesh import and GPU buffer swap require a renderer/content callback.");
    }
    if (request.kind == "ship.blueprint") {
        report.warnings.push_back("Ship blueprint was staged, but runtime ship rebuild requires a blueprint parser callback.");
    }

    report.success = true;
    report.validated = true;
    report.message = "Asset validated for reload: " + request.path.string();
    return report;
}

AssetReloadReport AssetReloadPipeline::CommitFileAsset(const AssetReloadRequest& request,
                                                       const AssetReloadHandler& handler,
                                                       const AssetReloadReport& validated) const
{
    AssetReloadReport report = validated;
    report.committed = false;

    if (handler.commitWithoutCallbackSucceeds) {
        report.success = true;
        report.message = "Asset reload staged and validated for '" + (request.assetId.empty() ? request.path.string() : request.assetId) + "'.";
        report.warnings.push_back("No runtime swap callback is registered for kind '" + request.kind + "'; source file validation succeeded but no live GPU/game resource was replaced.");
        return report;
    }

    report.success = false;
    report.message = "No runtime swap callback is registered for kind '" + request.kind + "'.";
    return report;
}

const AssetReloadHandler* AssetReloadPipeline::FindHandler(const std::string& kind) const
{
    const std::string normalized = NormalizeKind(kind);
    const auto found = _handlers.find(normalized);
    return found == _handlers.end() ? nullptr : &found->second;
}

} // namespace subspace
