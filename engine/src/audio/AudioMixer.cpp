#include "audio/AudioMixer.h"

#include <algorithm>

namespace subspace {

ChannelHandle AudioMixer::Play(SoundId soundId,
                               float   volume,
                               float   pan,
                               bool    loop,
                               uint8_t priority) {
    if (ActiveChannelCount() >= kMaxChannels) return NullChannel;
    if (soundId == NullSound)                 return NullChannel;

    ChannelHandle handle = _nextHandle++;
    MixerChannel ch;
    ch.soundId     = soundId;
    ch.volume      = std::max(0.0f, std::min(1.0f, volume));
    ch.pan         = std::max(-1.0f, std::min(1.0f, pan));
    ch.playbackPos = 0.0f;
    ch.playing     = true;
    ch.looping     = loop;
    ch.priority    = priority;
    _channels[handle] = ch;
    return handle;
}

void AudioMixer::Stop(ChannelHandle handle) {
    auto it = _channels.find(handle);
    if (it != _channels.end()) it->second.playing = false;
}

void AudioMixer::StopAll() {
    for (auto& [handle, ch] : _channels)
        ch.playing = false;
}

void AudioMixer::SetChannelVolume(ChannelHandle handle, float volume) {
    auto it = _channels.find(handle);
    if (it != _channels.end())
        it->second.volume = std::max(0.0f, std::min(1.0f, volume));
}

void AudioMixer::SetChannelPan(ChannelHandle handle, float pan) {
    auto it = _channels.find(handle);
    if (it != _channels.end())
        it->second.pan = std::max(-1.0f, std::min(1.0f, pan));
}

void AudioMixer::Update(float dt) {
    for (auto it = _channels.begin(); it != _channels.end(); ) {
        MixerChannel& ch = it->second;
        if (!ch.playing) { it = _channels.erase(it); continue; }

        ch.playbackPos += dt;

        if (_bank) {
            const SoundAsset* asset = _bank->Get(ch.soundId);
            if (asset && asset->duration > 0.0f &&
                ch.playbackPos >= asset->duration) {
                if (ch.looping) {
                    ch.playbackPos = 0.0f;
                } else {
                    it = _channels.erase(it);
                    continue;
                }
            }
        }
        ++it;
    }
}

int AudioMixer::ActiveChannelCount() const noexcept {
    int count = 0;
    for (const auto& [handle, ch] : _channels)
        if (ch.playing) ++count;
    return count;
}

const MixerChannel* AudioMixer::GetChannel(ChannelHandle handle) const noexcept {
    auto it = _channels.find(handle);
    return it != _channels.end() ? &it->second : nullptr;
}

} // namespace subspace
