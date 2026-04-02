#pragma once

#include <cstdint>

namespace subspace {

/// Platform audio backend identifier.
enum class AudioBackend : uint8_t {
    Null   = 0, ///< Software-only; no platform output (always available).
    OpenAL = 1, ///< OpenAL Soft (not yet linked; falls back to Null).
};

/// Manages the audio output device and global master volume.
///
/// Currently provides a Null (software-only) backend that lets the mixer
/// simulate playback without any platform dependency.  A real OpenAL or
/// miniaudio backend can be added later by extending Init().
///
/// Usage:
/// @code
///   AudioDevice dev;
///   dev.Init(AudioBackend::Null);
///   dev.SetMasterVolume(0.8f);
///   // ... per-frame mixing ...
///   dev.Shutdown();
/// @endcode
class AudioDevice {
public:
    /// Initialise the audio device.
    /// @param backend  Requested backend; falls back to Null when unavailable.
    /// @return true on success.
    bool Init(AudioBackend backend = AudioBackend::Null);

    /// Release all audio device resources.
    void Shutdown();

    /// Whether Init() succeeded.
    bool IsInitialized() const noexcept { return _initialised; }

    /// Active backend.
    AudioBackend GetBackend() const noexcept { return _backend; }

    /// Set master output volume in [0, 1].
    void SetMasterVolume(float volume) noexcept;

    /// Master output volume in [0, 1].
    float GetMasterVolume() const noexcept { return _masterVolume; }

    /// Output sample rate (default 44 100 Hz).
    uint32_t GetSampleRate() const noexcept { return _sampleRate; }

    /// Number of output channels (default 2 = stereo).
    uint32_t GetChannels() const noexcept { return _channels; }

private:
    float        _masterVolume = 1.0f;
    bool         _initialised  = false;
    AudioBackend _backend      = AudioBackend::Null;
    uint32_t     _sampleRate   = 44100;
    uint32_t     _channels     = 2;
};

} // namespace subspace
