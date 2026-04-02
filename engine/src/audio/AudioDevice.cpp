#include "audio/AudioDevice.h"

#include <algorithm>

namespace subspace {

bool AudioDevice::Init(AudioBackend backend) {
    if (_initialised) return true;
    // OpenAL support is planned but not yet linked; always fall back to Null.
    _backend     = AudioBackend::Null;
    (void)backend;
    _initialised = true;
    return true;
}

void AudioDevice::Shutdown() {
    _initialised = false;
}

void AudioDevice::SetMasterVolume(float volume) noexcept {
    _masterVolume = std::max(0.0f, std::min(1.0f, volume));
}

} // namespace subspace
