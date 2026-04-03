#pragma once

#include "audio/SoundAsset.h"
#include "audio/AudioMixer.h"
#include "core/Math.h"

namespace subspace {

/// Configuration for 3-D sound attenuation.
struct SpatialConfig {
    float minDistance = 1.0f;  ///< Distance at which volume starts attenuating.
    float maxDistance = 50.0f; ///< Distance beyond which volume is zero.
    float rolloff     = 1.0f;  ///< Rolloff factor (1 = inverse-distance).
};

/// Manages 3-D audio playback and listener state.
///
/// Uses inverse-distance attenuation and simple stereo panning based on the
/// relative angle between the listener forward direction and the sound source.
///
/// The SpatialAudio layer sits on top of AudioMixer and routes all 3-D sounds
/// through it with automatically computed volume/pan values.
class SpatialAudio {
public:
    /// Set the AudioMixer to route channels through.
    void SetMixer(AudioMixer* mixer) noexcept { _mixer = mixer; }

    // ---- Listener ----------------------------------------------------------

    void SetListenerPosition(const Vector3& pos) noexcept { _listenerPos = pos; }

    void SetListenerOrientation(const Vector3& forward,
                                const Vector3& up) noexcept {
        _listenerForward = forward;
        _listenerUp      = up;
    }

    const Vector3& GetListenerPosition() const noexcept { return _listenerPos; }
    const Vector3& GetListenerForward()  const noexcept { return _listenerForward; }
    const Vector3& GetListenerUp()       const noexcept { return _listenerUp; }

    // ---- 3-D playback ------------------------------------------------------

    /// Play a sound at a world-space position with spatial attenuation.
    /// @param soundId   Identifier of the sound to play.
    /// @param position  World-space emission point.
    /// @param baseVolume  Volume before attenuation [0, 1].
    /// @param config    Attenuation settings.
    /// @return ChannelHandle from the mixer, or NullChannel on failure.
    ChannelHandle PlayAt(SoundId soundId,
                         const Vector3& position,
                         float baseVolume = 1.0f,
                         const SpatialConfig& config = {});

    // ---- Attenuation helpers -----------------------------------------------

    /// Compute the attenuation factor for a source at @p distance.
    ///
    /// Uses clamped inverse-distance:
    ///   factor = clamp(min / (min + rolloff * (dist - min)), 0, 1)
    static float ComputeAttenuation(float distance,
                                    const SpatialConfig& cfg) noexcept;

    /// Compute stereo pan in [-1, +1] based on listener-to-source direction.
    float ComputePan(const Vector3& sourcePos) const noexcept;

private:
    AudioMixer* _mixer = nullptr;
    Vector3 _listenerPos{};
    Vector3 _listenerForward{0.0f, 0.0f, -1.0f};
    Vector3 _listenerUp    {0.0f, 1.0f,  0.0f};
};

} // namespace subspace
