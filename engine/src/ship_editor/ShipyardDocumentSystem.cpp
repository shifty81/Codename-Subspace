#include "ship_editor/ShipyardDocumentSystem.h"

#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace subspace {
namespace {

std::string DocumentScope(const ProceduralShipVisualRecipe& recipe, const std::string& persistentShipId) {
    if (!persistentShipId.empty()) return persistentShipId;
    if (!recipe.recipeId.empty()) return recipe.recipeId;
    std::ostringstream ss;
    ss << "recipe-seed-" << recipe.seed;
    return ss.str();
}

ShipyardObjectId NextId(ShipyardDocument& document, ShipyardObjectKind kind) {
    const auto id = ShipyardStableIdSystem::Local(kind, document.documentId, document.nextLocalOrdinal++);
    return id;
}

} // namespace

ShipyardDocument ShipyardDocumentSystem::Create(const ProceduralShipVisualRecipe& recipe,
                                                 const ShipAppearanceState& appearance,
                                                 std::string persistentShipId) {
    ShipyardDocument out;
    out.recipe = recipe;
    out.appearance = appearance;
    out.persistentShipId = std::move(persistentShipId);
    const auto scope = DocumentScope(recipe, out.persistentShipId);
    out.documentId = ShipyardStableIdSystem::Deterministic(ShipyardObjectKind::Document, "subspace.shipyard", scope);

    out.moduleIds.reserve(out.recipe.modules.size());
    for (std::size_t i = 0; i < out.recipe.modules.size(); ++i)
        out.moduleIds.push_back(NextId(out, ShipyardObjectKind::Module));

    out.attachmentIds.reserve(out.recipe.attachments.size());
    for (std::size_t i = 0; i < out.recipe.attachments.size(); ++i)
        out.attachmentIds.push_back(NextId(out, ShipyardObjectKind::Attachment));

    out.revision = 1;
    out.savedRevision = 1;
    return out;
}

ShipyardDocumentSnapshot ShipyardDocumentSystem::Capture(const ShipyardDocument& document) {
    return {document};
}

void ShipyardDocumentSystem::Restore(ShipyardDocument& target, const ShipyardDocumentSnapshot& snapshot) {
    target = snapshot.document;
}

ShipyardObjectId ShipyardDocumentSystem::ModuleIdAt(const ShipyardDocument& document, std::size_t index) {
    return index < document.moduleIds.size() ? document.moduleIds[index] : ShipyardObjectId{};
}

ShipyardObjectId ShipyardDocumentSystem::AttachmentIdAt(const ShipyardDocument& document, std::size_t index) {
    return index < document.attachmentIds.size() ? document.attachmentIds[index] : ShipyardObjectId{};
}

std::optional<std::size_t> ShipyardDocumentSystem::FindModuleIndex(const ShipyardDocument& document,
                                                                    const ShipyardObjectId& moduleId) {
    const auto it = std::find(document.moduleIds.begin(), document.moduleIds.end(), moduleId);
    if (it == document.moduleIds.end()) return std::nullopt;
    return static_cast<std::size_t>(std::distance(document.moduleIds.begin(), it));
}

std::optional<std::size_t> ShipyardDocumentSystem::FindAttachmentIndex(const ShipyardDocument& document,
                                                                        const ShipyardObjectId& attachmentId) {
    const auto it = std::find(document.attachmentIds.begin(), document.attachmentIds.end(), attachmentId);
    if (it == document.attachmentIds.end()) return std::nullopt;
    return static_cast<std::size_t>(std::distance(document.attachmentIds.begin(), it));
}

ShipyardObjectId ShipyardDocumentSystem::AppendModule(ShipyardDocument& document,
                                                       const VisualModulePlacement& placement) {
    document.recipe.modules.push_back(placement);
    const auto id = NextId(document, ShipyardObjectKind::Module);
    document.moduleIds.push_back(id);
    MarkModified(document);
    return id;
}

ShipyardObjectId ShipyardDocumentSystem::AppendAttachment(ShipyardDocument& document,
                                                           const VisualAssemblyAttachment& attachment) {
    if (attachment.parentModuleIndex >= document.recipe.modules.size() ||
        attachment.childModuleIndex >= document.recipe.modules.size()) return {};
    document.recipe.attachments.push_back(attachment);
    const auto id = NextId(document, ShipyardObjectKind::Attachment);
    document.attachmentIds.push_back(id);
    MarkModified(document);
    return id;
}

bool ShipyardDocumentSystem::EraseAttachment(ShipyardDocument& document,
                                              const ShipyardObjectId& attachmentId) {
    const auto index = FindAttachmentIndex(document, attachmentId);
    if (!index || *index >= document.recipe.attachments.size()) return false;
    document.recipe.attachments.erase(document.recipe.attachments.begin() + static_cast<std::ptrdiff_t>(*index));
    document.attachmentIds.erase(document.attachmentIds.begin() + static_cast<std::ptrdiff_t>(*index));
    MarkModified(document);
    return true;
}

bool ShipyardDocumentSystem::EraseModule(ShipyardDocument& document, const ShipyardObjectId& moduleId) {
    const auto indexOpt = FindModuleIndex(document, moduleId);
    if (!indexOpt) return false;
    const std::size_t index = *indexOpt;

    std::vector<VisualAssemblyAttachment> keptAttachments;
    std::vector<ShipyardObjectId> keptAttachmentIds;
    keptAttachments.reserve(document.recipe.attachments.size());
    keptAttachmentIds.reserve(document.attachmentIds.size());
    for (std::size_t i = 0; i < document.recipe.attachments.size(); ++i) {
        auto attachment = document.recipe.attachments[i];
        if (attachment.parentModuleIndex == index || attachment.childModuleIndex == index) continue;
        if (attachment.parentModuleIndex > index) --attachment.parentModuleIndex;
        if (attachment.childModuleIndex > index) --attachment.childModuleIndex;
        keptAttachments.push_back(std::move(attachment));
        if (i < document.attachmentIds.size()) keptAttachmentIds.push_back(document.attachmentIds[i]);
    }

    document.recipe.modules.erase(document.recipe.modules.begin() + static_cast<std::ptrdiff_t>(index));
    document.moduleIds.erase(document.moduleIds.begin() + static_cast<std::ptrdiff_t>(index));
    document.recipe.attachments = std::move(keptAttachments);
    document.attachmentIds = std::move(keptAttachmentIds);

    for (auto& decal : document.appearance.decals) {
        if (decal.moduleIndex > index) --decal.moduleIndex;
    }
    document.appearance.decals.erase(
        std::remove_if(document.appearance.decals.begin(), document.appearance.decals.end(),
                       [&](const ShipDecalLayer& decal) { return decal.moduleIndex == index; }),
        document.appearance.decals.end());

    MarkModified(document);
    return true;
}

void ShipyardDocumentSystem::MarkModified(ShipyardDocument& document) {
    ++document.revision;
    if (document.revision == 0) document.revision = 1;
}

void ShipyardDocumentSystem::MarkSaved(ShipyardDocument& document) {
    document.savedRevision = document.revision;
}

bool ShipyardDocumentSystem::IsDirty(const ShipyardDocument& document) {
    return document.revision != document.savedRevision;
}

ShipyardDocumentValidation ShipyardDocumentSystem::ValidateIdentity(const ShipyardDocument& document) {
    ShipyardDocumentValidation out;
    if (document.schemaVersion != ShipyardDocument::SchemaVersion) out.errors.push_back("Unsupported Shipyard document schema");
    if (!document.documentId.Valid()) out.errors.push_back("Document ID is invalid");
    if (document.moduleIds.size() != document.recipe.modules.size()) out.errors.push_back("Module identity table is out of sync with recipe modules");
    if (document.attachmentIds.size() != document.recipe.attachments.size()) out.errors.push_back("Attachment identity table is out of sync with recipe attachments");

    std::unordered_set<std::uint64_t> seen;
    for (const auto id : document.moduleIds) {
        if (!id.Valid() || id.kind != ShipyardObjectKind::Module) out.errors.push_back("Invalid module stable ID");
        else if (!seen.insert(id.value).second) out.errors.push_back("Duplicate module stable ID");
    }
    for (const auto id : document.attachmentIds) {
        if (!id.Valid() || id.kind != ShipyardObjectKind::Attachment) out.errors.push_back("Invalid attachment stable ID");
        else if (!seen.insert(id.value).second) out.errors.push_back("Duplicate attachment stable ID");
    }
    for (const auto& attachment : document.recipe.attachments) {
        if (attachment.parentModuleIndex >= document.recipe.modules.size() ||
            attachment.childModuleIndex >= document.recipe.modules.size())
            out.errors.push_back("Attachment references an invalid module index");
    }
    out.valid = out.errors.empty();
    return out;
}

} // namespace subspace
