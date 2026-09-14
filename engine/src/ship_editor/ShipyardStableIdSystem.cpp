#include "ship_editor/ShipyardStableIdSystem.h"

#include <iomanip>
#include <sstream>

namespace subspace {

std::uint64_t ShipyardStableIdSystem::Hash64(std::string_view text) {
    // FNV-1a 64-bit is deliberately simple, deterministic and portable. These
    // IDs are document identity, not security tokens.
    std::uint64_t hash = 1469598103934665603ull;
    for (unsigned char c : text) {
        hash ^= static_cast<std::uint64_t>(c);
        hash *= 1099511628211ull;
    }
    return hash == 0 ? 1 : hash;
}

ShipyardObjectId ShipyardStableIdSystem::Deterministic(ShipyardObjectKind kind,
                                                        std::string_view scope,
                                                        std::string_view key,
                                                        std::uint64_t ordinal) {
    std::ostringstream ss;
    ss << static_cast<unsigned int>(kind) << '|' << scope << '|' << key << '|' << ordinal;
    return {kind, Hash64(ss.str())};
}

ShipyardObjectId ShipyardStableIdSystem::Local(ShipyardObjectKind kind,
                                                const ShipyardObjectId& documentId,
                                                std::uint64_t ordinal) {
    std::ostringstream scope;
    scope << documentId.value;
    return Deterministic(kind, scope.str(), "local", ordinal);
}

const char* ShipyardStableIdSystem::KindName(ShipyardObjectKind kind) {
    switch (kind) {
        case ShipyardObjectKind::Document: return "document";
        case ShipyardObjectKind::Module: return "module";
        case ShipyardObjectKind::Attachment: return "attachment";
        case ShipyardObjectKind::Socket: return "socket";
        case ShipyardObjectKind::Room: return "room";
        case ShipyardObjectKind::Aperture: return "aperture";
        case ShipyardObjectKind::System: return "system";
        case ShipyardObjectKind::Decal: return "decal";
        case ShipyardObjectKind::SymmetryPair: return "symmetry";
        default: return "invalid";
    }
}

std::string ShipyardStableIdSystem::ToString(const ShipyardObjectId& id) {
    if (!id.Valid()) return "invalid:0";
    std::ostringstream ss;
    ss << KindName(id.kind) << ':' << std::hex << std::setw(16) << std::setfill('0') << id.value;
    return ss.str();
}

} // namespace subspace
