#pragma once

#include "audio/SoundAsset.h"
#include "audio/SoundBank.h"

#include <cstdint>
#include <unordered_map>

namespace subspace {

/// Per-channel playback state tracked by the AudioMixer.
struct MixerChannel {
    SoundId  soundId    = NullSound;
    float    volume     = 1.0f;   ///< Channel-local volume [0, 1].
    float    pan        = 0.0f;   ///< Stereo pan [-1 = left, 0 = centre, +1 = right].
    float    playbackPos= 0.0f;   ///< Elapsed playback time in seconds.
    bool     playing    = false;
    bool     looping    = false;
    uint8_t  priority   = 128;    ///< 0 = highest, 255 = lowest.
};

/// Handle identifying a playing channel in the AudioMixer.
using ChannelHandle = uint32_t;

/// Sentinel — invalid channel.
inline constexpr ChannelHandle NullChannel = 0u;

/// Mixes multiple concurrent sounds and manages their playback state.
///
/// Does not perform actual PCM mixing (that is the platform backend's job).
/// It maintains logical playback state and advance timers each frame so that
/// higher-level systems (SpatialAudio, AudioSystem) can query what is playing.
///
/// Maximum concurrent channels is controlled by kMaxChannels (soft limit).
/// Expired non-looping channels are released automatically during Update().
class AudioMixer {
public:
    AudioMixer() = default;

    /// Set the non-owning pointer to the sound bank used for duration lookups.
    void SetSoundBank(const SoundBank* bank) noexcept { _bank = bank; }

    // ---- Playback ----------------------------------------------------------

    /// Start playback of a sound.
    /// @param soundId   Asset id from SoundBank.
    /// @param volume    Channel volume [0, 1].
    /// @param pan       Stereo pan [-1, +1].
    /// @param loop      Whether to loop.
    /// @param priority  Priority (lower = more important).
    /// @return Handle to the playing channel, or NullChannel on failure.
    ChannelHandle Play(SoundId soundId,
                       float   volume   = 1.0f,
                       float   pan      = 0.0f,
                       bool    loop     = false,
                       uint8_t priority = 128);

    /// Stop a specific channel immediately.
    void Stop(ChannelHandle handle);

    /// Stop all active channels.
    void StopAll();

    /// Change the volume of a playing channel.
    void SetChannelVolume(ChannelHandle handle, float volume);

    /// Change the pan of a playing channel.
    void SetChannelPan(ChannelHandle handle, float pan);

    // ---- Per-frame ---------------------------------------------------------

    /// Advance all active channels by @p dt seconds.
    ///
    /// Channels that exceed their asset's duration are either looped or
    /// automatically removed.
    void Update(float dt);

    // ---- Queries -----------------------------------------------------------

    /// Number of currently active (playing) channels.
    int ActiveChannelCount() const noexcept;

    /// Retrieve channel state, or nullptr.
    const MixerChannel* GetChannel(ChannelHandle handle) const noexcept;

    /// Soft-limit on concurrent channels.
    static constexpr int kMaxChannels = 64;

private:
    const SoundBank* _bank = nullptr;
    ChannelHandle    _nextHandle = 1;
    std::unordered_map<ChannelHandle, MixerChannel> _channels;
};

} // namespace subspace
