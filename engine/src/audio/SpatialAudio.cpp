#include "audio/SpatialAudio.h"

#include <algorithm>
#include <cmath>

namespace subspace {

ChannelHandle SpatialAudio::PlayAt(SoundId soundId,
                                   const Vector3& position,
                                   float baseVolume,
                                   const SpatialConfig& config) {
    if (!_mixer) return NullChannel;

    float dx = position.x - _listenerPos.x;
    float dy = position.y - _listenerPos.y;
    float dz = position.z - _listenerPos.z;
    float dist = std::sqrt(dx*dx + dy*dy + dz*dz);

    float atten = ComputeAttenuation(dist, config);
    float vol   = baseVolume * atten;
    float pan   = ComputePan(position);

    return _mixer->Play(soundId, vol, pan, false);
}

float SpatialAudio::ComputeAttenuation(float distance,
                                        const SpatialConfig& cfg) noexcept {
    if (distance <= cfg.minDistance) return 1.0f;
    if (distance >= cfg.maxDistance) return 0.0f;
    float denom = cfg.minDistance + cfg.rolloff * (distance - cfg.minDistance);
    return denom > 0.0f
               ? std::max(0.0f, std::min(1.0f, cfg.minDistance / denom))
               : 0.0f;
}

float SpatialAudio::ComputePan(const Vector3& sourcePos) const noexcept {
    // right = forward × up
    float rx = _listenerForward.y * _listenerUp.z - _listenerForward.z * _listenerUp.y;
    float ry = _listenerForward.z * _listenerUp.x - _listenerForward.x * _listenerUp.z;
    float rz = _listenerForward.x * _listenerUp.y - _listenerForward.y * _listenerUp.x;

    float dx = sourcePos.x - _listenerPos.x;
    float dy = sourcePos.y - _listenerPos.y;
    float dz = sourcePos.z - _listenerPos.z;
    float len = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (len < 1e-6f) return 0.0f;
    dx /= len; dy /= len; dz /= len;

    float pan = rx*dx + ry*dy + rz*dz;
    return std::max(-1.0f, std::min(1.0f, pan));
}

} // namespace subspace
