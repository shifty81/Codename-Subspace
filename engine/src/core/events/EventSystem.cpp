#include "core/events/EventSystem.h"
#include "core/logging/Logger.h"

#include <algorithm>

namespace subspace {

EventSystem& EventSystem::Instance()
{
    static EventSystem instance;
    return instance;
}

void EventSystem::Subscribe(const std::string& eventType, EventCallback callback)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _listeners[eventType].push_back(std::move(callback));
    Logger::Instance().Debug("EventSystem", "Subscribed to event: " + eventType);
}

void EventSystem::UnsubscribeAll(const std::string& eventType)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _listeners.erase(eventType);
    Logger::Instance().Debug("EventSystem", "Unsubscribed all from event: " + eventType);
}

void EventSystem::Publish(const std::string& eventType, const GameEvent& eventData)
{
    std::vector<EventCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _listeners.find(eventType);
        if (it != _listeners.end()) {
            callbacks = it->second;
        }
    }

    for (auto& cb : callbacks) {
        try {
            cb(eventData);
        } catch (const std::exception& ex) {
            Logger::Instance().Error("EventSystem",
                "Error in event callback for " + eventType + ": " + ex.what());
        }
    }
}

void EventSystem::QueueEvent(const std::string& eventType, std::shared_ptr<GameEvent> eventData)
{
    eventData->eventType = eventType;
    std::lock_guard<std::mutex> lock(_mutex);
    _eventQueue.emplace_back(eventType, std::move(eventData));
}

void EventSystem::ProcessQueuedEvents()
{
    std::vector<std::pair<std::string, std::shared_ptr<GameEvent>>> toProcess;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        toProcess.swap(_eventQueue);
    }

    for (auto& [eventType, eventData] : toProcess) {
        Publish(eventType, *eventData);
    }
}

void EventSystem::ClearAllListeners()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _listeners.clear();
    _eventQueue.clear();
    Logger::Instance().Info("EventSystem", "All event listeners cleared");
}

int EventSystem::GetListenerCount(const std::string& eventType) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _listeners.find(eventType);
    return it != _listeners.end() ? static_cast<int>(it->second.size()) : 0;
}

} // namespace subspace
