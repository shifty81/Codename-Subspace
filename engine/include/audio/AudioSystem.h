#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/config/ConfigurationManager.h"
#include "core/persistence/SaveGameManager.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

// ---------------------------------------------------------------------------
// Audio clip / resource types
// ---------------------------------------------------------------------------

enum class AudioCategory { SFX, Music, Voice, Ambient, UI };

/// Describes a loaded audio clip (metadata only – actual PCM data is
/// handled by the platform audio backend which is not yet integrated).
struct AudioClip {
    std::string id;
    std::string filePath;
    AudioCategory category = AudioCategory::SFX;
    float durationSeconds = 0.0f;
    bool isLooping = false;
    float defaultVolume = 1.0f;

    bool IsValid() const { return !id.empty(); }
};

// ---------------------------------------------------------------------------
// Audio source – a playing (or queued) instance of a clip
// ---------------------------------------------------------------------------

enum class AudioSourceState { Stopped, Playing, Paused, FadingIn, FadingOut };

struct AudioSource {
    uint64_t sourceId = 0;
    std::string clipId;
    AudioSourceState state = AudioSourceState::Stopped;

    float volume = 1.0f;     // source-local volume [0,1]
    float pitch  = 1.0f;     // playback rate multiplier
    bool  loop   = false;

    // 3D positional audio
    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    float maxDistance = 100.0f;
    bool  is3D = false;

    // Fade support
    float fadeTimer    = 0.0f;
    float fadeDuration = 0.0f;
    float fadeStartVol = 0.0f;
    float fadeEndVol   = 0.0f;

    // Playback bookkeeping
    float playbackTime = 0.0f;

    /// Effective volume considering source + fade.
    float GetEffectiveVolume() const;

    /// Whether the source is currently audible.
    bool IsActive() const;
};

// ---------------------------------------------------------------------------
// Audio component – attach to an entity for positional sound
// ---------------------------------------------------------------------------

struct AudioComponent : public IComponent {
    std::vector<AudioSource> sources;
    int maxConcurrentSources = 8;

    /// Add a source. Returns its sourceId (0 on failure).
    uint64_t AddSource(const AudioSource& src);

    /// Remove a source by id. Returns true if found.
    bool RemoveSource(uint64_t sourceId);

    /// Find a source by id. Returns nullptr if absent.
    AudioSource* GetSource(uint64_t sourceId);

    /// Number of currently active (playing / fading) sources.
    int GetActiveSourceCount() const;

    /// Stop all sources.
    void StopAll();

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from a previously serialized ComponentData.
    void Deserialize(const ComponentData& data);
};

// ---------------------------------------------------------------------------
// Music playlist support
// ---------------------------------------------------------------------------

struct MusicPlaylist {
    std::string name;
    std::vector<std::string> trackIds;   // clip ids
    int currentIndex = 0;
    bool shuffle = false;
    bool repeat  = true;

    std::string CurrentTrackId() const;
    std::string NextTrackId();
    void Reset();
};

// ---------------------------------------------------------------------------
// Audio system – manages clips, global sources and music
// ---------------------------------------------------------------------------

class AudioSystem : public SystemBase {
public:
    AudioSystem();

    void Initialize() override;
    void Update(float deltaTime) override;
    void Shutdown() override;

    // -- Clip management --
    void RegisterClip(const AudioClip& clip);
    bool HasClip(const std::string& clipId) const;
    const AudioClip* GetClip(const std::string& clipId) const;
    size_t GetClipCount() const;

    // -- Playback (global, non-entity sources) --
    uint64_t PlaySound(const std::string& clipId, float volume = 1.0f,
                       float pitch = 1.0f);
    uint64_t PlaySound3D(const std::string& clipId, float x, float y, float z,
                         float volume = 1.0f);
    void StopSound(uint64_t sourceId);
    void StopAllSounds();
    AudioSource* GetGlobalSource(uint64_t sourceId);
    int GetActiveGlobalSourceCount() const;

    // -- Fade helpers --
    void FadeIn(uint64_t sourceId, float duration);
    void FadeOut(uint64_t sourceId, float duration);

    // -- Music --
    void SetMusicPlaylist(const MusicPlaylist& playlist);
    void PlayMusic();
    void PauseMusic();
    void StopMusic();
    void NextTrack();
    bool IsMusicPlaying() const;
    const MusicPlaylist& GetMusicPlaylist() const;

    // -- Listener (camera / player position for 3D audio) --
    void SetListenerPosition(float x, float y, float z);
    void GetListenerPosition(float& x, float& y, float& z) const;

    // -- Volume helpers that consult AudioSettings --
    float GetMasterVolume() const;
    float GetCategoryVolume(AudioCategory cat) const;
    float ComputeFinalVolume(const AudioSource& src, AudioCategory cat) const;

    // -- Global mute --
    void SetMuted(bool muted);
    bool IsMuted() const;

private:
    void UpdateSource(AudioSource& src, float deltaTime);
    void UpdateMusic(float deltaTime);
    uint64_t NextSourceId();

    std::unordered_map<std::string, AudioClip> _clips;
    std::vector<AudioSource> _globalSources;

    MusicPlaylist _musicPlaylist;
    AudioSource   _musicSource;

    float _listenerX = 0.0f, _listenerY = 0.0f, _listenerZ = 0.0f;
    bool  _isMuted = false;

    uint64_t _nextSourceId = 1;
};

} // namespace subspace
