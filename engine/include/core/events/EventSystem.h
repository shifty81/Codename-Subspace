#pragma once

#include "core/events/GameEvents.h"

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

using EventCallback = std::function<void(const GameEvent&)>;

/// Centralized event system for decoupled communication between systems (singleton).
class EventSystem {
public:
    static EventSystem& Instance();

    /// Subscribe to an event type.
    void Subscribe(const std::string& eventType, EventCallback callback);

    /// Unsubscribe all callbacks for an event type.
    void UnsubscribeAll(const std::string& eventType);

    /// Publish an event immediately to all subscribers.
    void Publish(const std::string& eventType, const GameEvent& eventData);

    /// Queue an event for deferred processing.
    void QueueEvent(const std::string& eventType, std::shared_ptr<GameEvent> eventData);

    /// Process all queued events.
    void ProcessQueuedEvents();

    /// Clear all listeners and queued events.
    void ClearAllListeners();

    /// Get number of listeners for an event type.
    int GetListenerCount(const std::string& eventType) const;

private:
    EventSystem() = default;
    EventSystem(const EventSystem&) = delete;
    EventSystem& operator=(const EventSystem&) = delete;

    mutable std::mutex _mutex;
    std::unordered_map<std::string, std::vector<EventCallback>> _listeners;
    std::vector<std::pair<std::string, std::shared_ptr<GameEvent>>> _eventQueue;
};

} // namespace subspace
