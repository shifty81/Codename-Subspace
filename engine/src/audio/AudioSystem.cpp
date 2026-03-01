#include "audio/AudioSystem.h"

#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// AudioSource helpers
// ---------------------------------------------------------------------------

float AudioSource::GetEffectiveVolume() const {
    if (state == AudioSourceState::FadingIn || state == AudioSourceState::FadingOut) {
        if (fadeDuration <= 0.0f) return fadeEndVol;
        float t = std::min(fadeTimer / fadeDuration, 1.0f);
        return fadeStartVol + (fadeEndVol - fadeStartVol) * t;
    }
    return volume;
}

bool AudioSource::IsActive() const {
    return state == AudioSourceState::Playing ||
           state == AudioSourceState::FadingIn ||
           state == AudioSourceState::FadingOut;
}

// ---------------------------------------------------------------------------
// AudioComponent
// ---------------------------------------------------------------------------

uint64_t AudioComponent::AddSource(const AudioSource& src) {
    if (static_cast<int>(sources.size()) >= maxConcurrentSources) return 0;
    sources.push_back(src);
    return src.sourceId;
}

bool AudioComponent::RemoveSource(uint64_t sourceId) {
    auto it = std::find_if(sources.begin(), sources.end(),
                           [sourceId](const AudioSource& s) { return s.sourceId == sourceId; });
    if (it == sources.end()) return false;
    sources.erase(it);
    return true;
}

AudioSource* AudioComponent::GetSource(uint64_t sourceId) {
    for (auto& s : sources) {
        if (s.sourceId == sourceId) return &s;
    }
    return nullptr;
}

int AudioComponent::GetActiveSourceCount() const {
    int count = 0;
    for (const auto& s : sources) {
        if (s.IsActive()) ++count;
    }
    return count;
}

void AudioComponent::StopAll() {
    for (auto& s : sources) {
        s.state = AudioSourceState::Stopped;
        s.playbackTime = 0.0f;
    }
}

// -- Serialization ----------------------------------------------------------

static std::string AudioCategoryToString(AudioCategory c) {
    switch (c) {
    case AudioCategory::SFX:     return "SFX";
    case AudioCategory::Music:   return "Music";
    case AudioCategory::Voice:   return "Voice";
    case AudioCategory::Ambient: return "Ambient";
    case AudioCategory::UI:      return "UI";
    }
    return "SFX";
}

static AudioCategory AudioCategoryFromString(const std::string& s) {
    if (s == "Music")   return AudioCategory::Music;
    if (s == "Voice")   return AudioCategory::Voice;
    if (s == "Ambient") return AudioCategory::Ambient;
    if (s == "UI")      return AudioCategory::UI;
    return AudioCategory::SFX;
}

static std::string SourceStateToString(AudioSourceState st) {
    switch (st) {
    case AudioSourceState::Stopped:   return "Stopped";
    case AudioSourceState::Playing:   return "Playing";
    case AudioSourceState::Paused:    return "Paused";
    case AudioSourceState::FadingIn:  return "FadingIn";
    case AudioSourceState::FadingOut: return "FadingOut";
    }
    return "Stopped";
}

static AudioSourceState SourceStateFromString(const std::string& s) {
    if (s == "Playing")   return AudioSourceState::Playing;
    if (s == "Paused")    return AudioSourceState::Paused;
    if (s == "FadingIn")  return AudioSourceState::FadingIn;
    if (s == "FadingOut") return AudioSourceState::FadingOut;
    return AudioSourceState::Stopped;
}

ComponentData AudioComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "AudioComponent";
    cd.data["sourceCount"] = std::to_string(sources.size());
    cd.data["maxConcurrent"] = std::to_string(maxConcurrentSources);

    for (size_t i = 0; i < sources.size(); ++i) {
        const auto& s = sources[i];
        std::string p = "src_" + std::to_string(i) + "_";
        cd.data[p + "id"]    = std::to_string(s.sourceId);
        cd.data[p + "clip"]  = s.clipId;
        cd.data[p + "state"] = SourceStateToString(s.state);
        cd.data[p + "vol"]   = std::to_string(s.volume);
        cd.data[p + "pitch"] = std::to_string(s.pitch);
        cd.data[p + "loop"]  = s.loop ? "true" : "false";
        cd.data[p + "is3D"]  = s.is3D ? "true" : "false";
        cd.data[p + "posX"]  = std::to_string(s.posX);
        cd.data[p + "posY"]  = std::to_string(s.posY);
        cd.data[p + "posZ"]  = std::to_string(s.posZ);
    }
    return cd;
}

void AudioComponent::Deserialize(const ComponentData& data) {
    sources.clear();

    auto getStr = [&](const std::string& key) -> std::string {
        auto it = data.data.find(key);
        return it != data.data.end() ? it->second : "";
    };
    auto getInt = [&](const std::string& key, int def = 0) -> int {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stoi(it->second); } catch (...) { return def; }
    };
    auto getFloat = [&](const std::string& key, float def = 0.0f) -> float {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stof(it->second); } catch (...) { return def; }
    };

    maxConcurrentSources = getInt("maxConcurrent", 8);
    int count = getInt("sourceCount", 0);

    for (int i = 0; i < count; ++i) {
        std::string p = "src_" + std::to_string(i) + "_";
        AudioSource s;
        s.sourceId = static_cast<uint64_t>(getInt(p + "id", 0));
        s.clipId   = getStr(p + "clip");
        s.state    = SourceStateFromString(getStr(p + "state"));
        s.volume   = getFloat(p + "vol", 1.0f);
        s.pitch    = getFloat(p + "pitch", 1.0f);
        s.loop     = getStr(p + "loop") == "true";
        s.is3D     = getStr(p + "is3D") == "true";
        s.posX     = getFloat(p + "posX", 0.0f);
        s.posY     = getFloat(p + "posY", 0.0f);
        s.posZ     = getFloat(p + "posZ", 0.0f);
        sources.push_back(std::move(s));
    }
}

// ---------------------------------------------------------------------------
// MusicPlaylist
// ---------------------------------------------------------------------------

std::string MusicPlaylist::CurrentTrackId() const {
    if (trackIds.empty()) return "";
    int idx = currentIndex % static_cast<int>(trackIds.size());
    return trackIds[static_cast<size_t>(idx)];
}

std::string MusicPlaylist::NextTrackId() {
    if (trackIds.empty()) return "";
    ++currentIndex;
    if (currentIndex >= static_cast<int>(trackIds.size())) {
        currentIndex = repeat ? 0 : static_cast<int>(trackIds.size()) - 1;
    }
    return CurrentTrackId();
}

void MusicPlaylist::Reset() {
    currentIndex = 0;
}

// ---------------------------------------------------------------------------
// AudioSystem
// ---------------------------------------------------------------------------

AudioSystem::AudioSystem() : SystemBase("AudioSystem") {}

void AudioSystem::Initialize() {
    _clips.clear();
    _globalSources.clear();
    _musicPlaylist = MusicPlaylist{};
    _musicSource = AudioSource{};
    _musicSource.sourceId = NextSourceId();
    _isMuted = false;
}

void AudioSystem::Update(float deltaTime) {
    if (!_isEnabled) return;

    // Update global sources
    for (auto& src : _globalSources) {
        UpdateSource(src, deltaTime);
    }

    // Remove stopped, non-looping global sources to avoid unbounded growth
    _globalSources.erase(
        std::remove_if(_globalSources.begin(), _globalSources.end(),
                       [](const AudioSource& s) {
                           return s.state == AudioSourceState::Stopped && !s.loop;
                       }),
        _globalSources.end());

    // Update music
    UpdateMusic(deltaTime);
}

void AudioSystem::Shutdown() {
    StopAllSounds();
    StopMusic();
    _clips.clear();
}

// -- Clip management --------------------------------------------------------

void AudioSystem::RegisterClip(const AudioClip& clip) {
    if (clip.id.empty()) return;
    _clips[clip.id] = clip;
}

bool AudioSystem::HasClip(const std::string& clipId) const {
    return _clips.count(clipId) > 0;
}

const AudioClip* AudioSystem::GetClip(const std::string& clipId) const {
    auto it = _clips.find(clipId);
    return (it != _clips.end()) ? &it->second : nullptr;
}

size_t AudioSystem::GetClipCount() const {
    return _clips.size();
}

// -- Playback ---------------------------------------------------------------

uint64_t AudioSystem::PlaySound(const std::string& clipId, float volume, float pitch) {
    auto it = _clips.find(clipId);
    if (it == _clips.end()) return 0;

    AudioSource src;
    src.sourceId = NextSourceId();
    src.clipId = clipId;
    src.volume = volume;
    src.pitch  = pitch;
    src.loop   = it->second.isLooping;
    src.state  = AudioSourceState::Playing;
    src.is3D   = false;

    _globalSources.push_back(src);
    return src.sourceId;
}

uint64_t AudioSystem::PlaySound3D(const std::string& clipId,
                                   float x, float y, float z,
                                   float volume) {
    auto it = _clips.find(clipId);
    if (it == _clips.end()) return 0;

    AudioSource src;
    src.sourceId = NextSourceId();
    src.clipId = clipId;
    src.volume = volume;
    src.pitch  = 1.0f;
    src.loop   = it->second.isLooping;
    src.state  = AudioSourceState::Playing;
    src.is3D   = true;
    src.posX   = x;
    src.posY   = y;
    src.posZ   = z;

    _globalSources.push_back(src);
    return src.sourceId;
}

void AudioSystem::StopSound(uint64_t sourceId) {
    for (auto& s : _globalSources) {
        if (s.sourceId == sourceId) {
            s.state = AudioSourceState::Stopped;
            s.playbackTime = 0.0f;
            return;
        }
    }
}

void AudioSystem::StopAllSounds() {
    for (auto& s : _globalSources) {
        s.state = AudioSourceState::Stopped;
        s.playbackTime = 0.0f;
    }
    _globalSources.clear();
}

AudioSource* AudioSystem::GetGlobalSource(uint64_t sourceId) {
    for (auto& s : _globalSources) {
        if (s.sourceId == sourceId) return &s;
    }
    return nullptr;
}

int AudioSystem::GetActiveGlobalSourceCount() const {
    int n = 0;
    for (const auto& s : _globalSources) {
        if (s.IsActive()) ++n;
    }
    return n;
}

// -- Fades ------------------------------------------------------------------

void AudioSystem::FadeIn(uint64_t sourceId, float duration) {
    AudioSource* src = GetGlobalSource(sourceId);
    if (!src) return;
    src->fadeStartVol = 0.0f;
    src->fadeEndVol   = src->volume;
    src->fadeDuration = duration;
    src->fadeTimer    = 0.0f;
    src->state        = AudioSourceState::FadingIn;
}

void AudioSystem::FadeOut(uint64_t sourceId, float duration) {
    AudioSource* src = GetGlobalSource(sourceId);
    if (!src) return;
    src->fadeStartVol = src->volume;
    src->fadeEndVol   = 0.0f;
    src->fadeDuration = duration;
    src->fadeTimer    = 0.0f;
    src->state        = AudioSourceState::FadingOut;
}

// -- Music ------------------------------------------------------------------

void AudioSystem::SetMusicPlaylist(const MusicPlaylist& playlist) {
    _musicPlaylist = playlist;
    _musicPlaylist.currentIndex = 0;
}

void AudioSystem::PlayMusic() {
    std::string trackId = _musicPlaylist.CurrentTrackId();
    if (trackId.empty()) return;

    _musicSource.clipId = trackId;
    _musicSource.state  = AudioSourceState::Playing;
    _musicSource.playbackTime = 0.0f;
    _musicSource.loop   = false; // playlist handles advancement
    _musicSource.volume = 1.0f;
}

void AudioSystem::PauseMusic() {
    if (_musicSource.state == AudioSourceState::Playing) {
        _musicSource.state = AudioSourceState::Paused;
    }
}

void AudioSystem::StopMusic() {
    _musicSource.state = AudioSourceState::Stopped;
    _musicSource.playbackTime = 0.0f;
}

bool AudioSystem::IsMusicPlaying() const {
    return _musicSource.state == AudioSourceState::Playing;
}

const MusicPlaylist& AudioSystem::GetMusicPlaylist() const {
    return _musicPlaylist;
}

void AudioSystem::NextTrack() {
    std::string nextId = _musicPlaylist.NextTrackId();
    if (nextId.empty()) {
        StopMusic();
        return;
    }
    _musicSource.clipId = nextId;
    _musicSource.state  = AudioSourceState::Playing;
    _musicSource.playbackTime = 0.0f;
}

// -- Listener ---------------------------------------------------------------

void AudioSystem::SetListenerPosition(float x, float y, float z) {
    _listenerX = x; _listenerY = y; _listenerZ = z;
}

void AudioSystem::GetListenerPosition(float& x, float& y, float& z) const {
    x = _listenerX; y = _listenerY; z = _listenerZ;
}

// -- Volume helpers ---------------------------------------------------------

float AudioSystem::GetMasterVolume() const {
    const auto& cfg = ConfigurationManager::Instance().GetConfig();
    return cfg.audio.isMuted ? 0.0f : cfg.audio.masterVolume;
}

float AudioSystem::GetCategoryVolume(AudioCategory cat) const {
    const auto& audio = ConfigurationManager::Instance().GetConfig().audio;
    switch (cat) {
    case AudioCategory::Music:   return audio.musicVolume;
    case AudioCategory::Voice:   return audio.voiceVolume;
    case AudioCategory::SFX:     return audio.sfxVolume;
    case AudioCategory::Ambient: return audio.sfxVolume; // ambient follows SFX
    case AudioCategory::UI:      return audio.sfxVolume; // UI follows SFX
    }
    return 1.0f;
}

float AudioSystem::ComputeFinalVolume(const AudioSource& src, AudioCategory cat) const {
    if (_isMuted) return 0.0f;
    return src.GetEffectiveVolume() * GetCategoryVolume(cat) * GetMasterVolume();
}

void AudioSystem::SetMuted(bool muted) { _isMuted = muted; }
bool AudioSystem::IsMuted() const { return _isMuted; }

// -- Internal ---------------------------------------------------------------

void AudioSystem::UpdateSource(AudioSource& src, float deltaTime) {
    if (src.state == AudioSourceState::Stopped) return;

    // Advance playback timer
    src.playbackTime += deltaTime * src.pitch;

    // Handle fades
    if (src.state == AudioSourceState::FadingIn ||
        src.state == AudioSourceState::FadingOut) {
        src.fadeTimer += deltaTime;
        if (src.fadeTimer >= src.fadeDuration) {
            src.fadeTimer = src.fadeDuration;
            if (src.state == AudioSourceState::FadingOut) {
                src.state = AudioSourceState::Stopped;
                src.playbackTime = 0.0f;
            } else {
                src.state = AudioSourceState::Playing;
            }
        }
    }

    // Check if clip has finished (non-looping)
    const AudioClip* clip = GetClip(src.clipId);
    if (clip && clip->durationSeconds > 0.0f &&
        src.playbackTime >= clip->durationSeconds) {
        if (src.loop) {
            src.playbackTime = 0.0f;
        } else {
            src.state = AudioSourceState::Stopped;
            src.playbackTime = 0.0f;
        }
    }
}

void AudioSystem::UpdateMusic(float deltaTime) {
    if (_musicSource.state != AudioSourceState::Playing) return;

    _musicSource.playbackTime += deltaTime;

    const AudioClip* clip = GetClip(_musicSource.clipId);
    if (clip && clip->durationSeconds > 0.0f &&
        _musicSource.playbackTime >= clip->durationSeconds) {
        // Track finished – advance playlist
        NextTrack();
    }
}

uint64_t AudioSystem::NextSourceId() {
    return _nextSourceId++;
}

} // namespace subspace
