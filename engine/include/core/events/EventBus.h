#pragma once

#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace subspace {

/// Lightweight, synchronous publish-subscribe event bus.
///
/// Events are plain structs or classes.  Subscriptions persist until the bus
/// is destroyed or Clear() is called.  Publish() calls all registered
/// handlers inline on the calling thread.
///
/// This is complementary to the existing string-keyed EventSystem.  Use
/// EventBus<T> when strong typing is preferred (e.g. new input, audio
/// backend, and UI systems); use EventSystem for legacy game events to avoid
/// disrupting the existing 15 000+ lines of game code.
///
/// Usage:
/// @code
///   EventBus bus;
///
///   struct PlayerDied { int playerId; };
///
///   bus.Subscribe<PlayerDied>([](const PlayerDied& e) {
///       std::cout << "player " << e.playerId << " died\n";
///   });
///
///   bus.Publish(PlayerDied{42});
/// @endcode
class EventBus {
public:
    /// Register a callback to be invoked whenever an event of type T is
    /// published.
    /// @tparam T        The event type (must be copyable).
    /// @param  handler  Callback with signature void(const T&).
    template<typename T>
    void Subscribe(std::function<void(const T&)> handler) {
        auto key = std::type_index(typeid(T));
        _handlers[key].emplace_back(
            [h = std::move(handler)](const void* evt) {
                h(*static_cast<const T*>(evt));
            });
    }

    /// Broadcast an event to all subscribers registered for type T.
    /// @tparam T     The event type.
    /// @param  event The event instance; passed by const-ref to each handler.
    template<typename T>
    void Publish(const T& event) {
        auto it = _handlers.find(std::type_index(typeid(T)));
        if (it == _handlers.end()) return;
        for (auto& fn : it->second)
            fn(static_cast<const void*>(&event));
    }

    /// Return the number of handlers registered for type T.
    template<typename T>
    int GetHandlerCount() const {
        auto it = _handlers.find(std::type_index(typeid(T)));
        return it != _handlers.end()
                   ? static_cast<int>(it->second.size())
                   : 0;
    }

    /// Unsubscribe all handlers for every event type.
    void Clear() noexcept { _handlers.clear(); }

private:
    using ErasedHandler = std::function<void(const void*)>;
    std::unordered_map<std::type_index, std::vector<ErasedHandler>> _handlers;
};

} // namespace subspace
