#pragma once

#include "modeling/ShipyardModelingSystem.h"

#include <filesystem>
#include <string>

namespace subspace {

// Versioned, typed Studio document codec for model-asset authoring.  Ship
// blueprints remain backward-compatible .subspace_ship artifacts; model-only
// authoring no longer pretends a ship blueprint can preserve its data.
class StudioModelDocumentCodec {
public:
    static constexpr int kSchemaVersion = 1;
    static bool IsStudioModelPath(const std::filesystem::path& path) noexcept;

    // Atomic write with verified round-trip.  overwrite=false is Save As;
    // overwrite=true is ordinary Save to the current document.
    static bool Save(const std::filesystem::path& destination,
                     const ShipyardModelingState& state,
                     bool overwrite,
                     std::string& error);

    static bool Load(const std::filesystem::path& source,
                     ShipyardModelingState& state,
                     std::string& error);
};

} // namespace subspace
