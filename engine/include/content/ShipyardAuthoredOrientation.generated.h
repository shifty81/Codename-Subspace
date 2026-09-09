#pragma once

#include <array>
#include <string_view>

namespace subspace {

struct ShipyardAuthoredOrientationRow {
    const char* canonical;
    float forwardX;
    float forwardY;
    float forwardZ;
    float upX;
    float upY;
    float upZ;
    const char* authority;
};

/// User-verified source-axis corrections for Greyoxide modules whose authored
/// object basis cannot be inferred reliably from the filename or bounding box.
///
/// These vectors are expressed in the module's runtime-local basis AFTER OBJ
/// remap (X lateral, Y ship-forward, Z visual-up). The generic attachment
/// solver remains responsible for choosing the world transform and handedness.
inline constexpr std::array<ShipyardAuthoredOrientationRow, 2> kShipyardAuthoredOrientation = {{
    // User visual certification 2026-09-05:
    // miscBlockFinger's smaller end is the forward edge and the broad flat face
    // is the dorsal/outward presentation face. Both are opposite the historical
    // +Y/+Z assumption, so a root-preserving 180-degree X-axis flip is required.
    {"miscblockfinger", 0.0f,-1.0f, 0.0f, 0.0f, 0.0f,-1.0f, "USER_CERTIFIED_20260905"},
    // Prior Pass497 certification: miscFinHanger is authored upright. Local +Y
    // is forward, while local +X is its dorsal axis after the required lay-flat
    // quarter turn.
    {"miscfinhanger",   0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, "USER_CERTIFIED_20260904"},
}};

inline constexpr const ShipyardAuthoredOrientationRow*
FindShipyardAuthoredOrientation(std::string_view canonical) {
    for (const auto& row : kShipyardAuthoredOrientation)
        if (canonical == row.canonical) return &row;
    return nullptr;
}

} // namespace subspace
