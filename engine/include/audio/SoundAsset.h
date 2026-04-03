#pragma once

#include <cstdint>
#include <string>

namespace subspace {

/// Identifier for a sound asset registered in a SoundBank.
using SoundId = uint32_t;

/// Sentinel — no sound.
inline constexpr SoundId NullSound = 0u;

/// Metadata for a loaded sound asset.
///
/// The actual PCM data (if any) is managed by the platform backend.
/// This struct carries only the logical properties needed by the mixer.
struct SoundAsset {
    SoundId     id       = NullSound;
    std::string name;               ///< Human-readable identifier.
    float       duration = 0.0f;    ///< Clip length in seconds (0 = unknown).
    bool        isLooping = false;  ///< Default looping flag.

    bool IsValid() const noexcept { return id != NullSound; }
};

} // namespace subspace
