#pragma once
#include "ship_editor/ShipBlueprintLibrarySystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include "studio/StudioModelDocumentCodec.h"
#include <filesystem>
#include <string>

namespace subspace {
// A Studio document is the existing game blueprint schema; no parallel format.
// Save is transactional, Open rejects unsaved changes, and runtime content is
// never silently overwritten by a draft editor session.
class StudioDocumentStore {
public:
    bool Open(const std::filesystem::path& source, ShipyardBuilderSystem& builder, std::string& error);
    bool Save(ShipyardBuilderSystem& builder, std::string& error);
    bool SaveAs(const std::filesystem::path& destination, ShipyardBuilderSystem& builder, std::string& error);
    bool New(ShipyardBuilderSystem& builder, std::string& error);
    // Exit-only salvage of the blueprint portion into a unique separate file.
    // Editable model recovery is a separate verified .subspace_studio document;
    // interior drafts and unpublished overrides are not recovered here.
    bool SaveExitRecovery(ShipyardBuilderSystem& builder, std::filesystem::path& recovered,
                          std::string& error);
    // Independently recover editable modeling, including when ship modules
    // coexist. This never changes the active document or marks its model saved.
    bool SaveModelExitRecovery(ShipyardBuilderSystem& builder, std::filesystem::path& recovered,
                               std::string& error);
    const std::filesystem::path& Path() const noexcept { return path_; }
private:
    std::filesystem::path path_;
};
} // namespace subspace
