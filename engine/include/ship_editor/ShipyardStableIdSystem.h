#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace subspace {

enum class ShipyardObjectKind : std::uint8_t {
    Invalid = 0,
    Document,
    Module,
    Attachment,
    Socket,
    Room,
    Aperture,
    System,
    Decal,
    SymmetryPair
};

struct ShipyardObjectId {
    ShipyardObjectKind kind = ShipyardObjectKind::Invalid;
    std::uint64_t value = 0;

    bool Valid() const { return kind != ShipyardObjectKind::Invalid && value != 0; }
    explicit operator bool() const { return Valid(); }

    bool operator==(const ShipyardObjectId& other) const {
        return kind == other.kind && value == other.value;
    }
    bool operator!=(const ShipyardObjectId& other) const { return !(*this == other); }
    bool operator<(const ShipyardObjectId& other) const {
        if (kind != other.kind) return kind < other.kind;
        return value < other.value;
    }
};

class ShipyardStableIdSystem {
public:
    static std::uint64_t Hash64(std::string_view text);
    static ShipyardObjectId Deterministic(ShipyardObjectKind kind,
                                          std::string_view scope,
                                          std::string_view key,
                                          std::uint64_t ordinal = 0);
    static ShipyardObjectId Local(ShipyardObjectKind kind,
                                  const ShipyardObjectId& documentId,
                                  std::uint64_t ordinal);
    static const char* KindName(ShipyardObjectKind kind);
    static std::string ToString(const ShipyardObjectId& id);
};

} // namespace subspace
