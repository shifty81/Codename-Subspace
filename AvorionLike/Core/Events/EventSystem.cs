using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Events;

/// <summary>
/// Event data base class
/// </summary>
public class GameEvent
{
    public DateTime Timestamp { get; set; } = DateTime.UtcNow;
    public string EventType { get; set; } = "";
}

/// <summary>
/// Centralized event system for decoupled communication between systems
/// </summary>
public class EventSystem
{
    private static EventSystem? _instance;
    private readonly Dictionary<string, List<Action<GameEvent>>> _listeners = new();
    private readonly object _lock = new();
    private readonly Queue<(string eventType, GameEvent eventData)> _eventQueue = new();

    public static EventSystem Instance
    {
        get
        {
            _instance ??= new EventSystem();
            return _instance;
        }
    }

    /// <summary>
    /// Subscribe to an event
    /// </summary>
    public void Subscribe(string eventType, Action<GameEvent> callback)
    {
        lock (_lock)
        {
            if (!_listeners.ContainsKey(eventType))
            {
                _listeners[eventType] = new List<Action<GameEvent>>();
            }

            _listeners[eventType].Add(callback);
            Logger.Instance.Debug("EventSystem", $"Subscribed to event: {eventType}");
        }
    }

    /// <summary>
    /// Unsubscribe from an event
    /// </summary>
    public void Unsubscribe(string eventType, Action<GameEvent> callback)
    {
        lock (_lock)
        {
            if (_listeners.ContainsKey(eventType))
            {
                _listeners[eventType].Remove(callback);
                Logger.Instance.Debug("EventSystem", $"Unsubscribed from event: {eventType}");
            }
        }
    }

    /// <summary>
    /// Publish an event immediately
    /// </summary>
    public void Publish(string eventType, GameEvent eventData)
    {
        eventData.EventType = eventType;

        List<Action<GameEvent>>? callbacks = null;
        
        lock (_lock)
        {
            if (_listeners.ContainsKey(eventType))
            {
                callbacks = new List<Action<GameEvent>>(_listeners[eventType]);
            }
        }

        if (callbacks != null && callbacks.Count > 0)
        {
            Logger.Instance.Debug("EventSystem", $"Publishing event: {eventType} to {callbacks.Count} listeners");
            
            foreach (var callback in callbacks)
            {
                try
                {
                    callback(eventData);
                }
                catch (Exception ex)
                {
                    Logger.Instance.Error("EventSystem", $"Error in event callback for {eventType}", ex);
                }
            }
        }
    }

    /// <summary>
    /// Queue an event for deferred processing
    /// </summary>
    public void QueueEvent(string eventType, GameEvent eventData)
    {
        eventData.EventType = eventType;
        
        lock (_lock)
        {
            _eventQueue.Enqueue((eventType, eventData));
        }
    }

    /// <summary>
    /// Process all queued events
    /// </summary>
    public void ProcessQueuedEvents()
    {
        List<(string eventType, GameEvent eventData)> eventsToProcess;
        
        lock (_lock)
        {
            eventsToProcess = new List<(string, GameEvent)>(_eventQueue);
            _eventQueue.Clear();
        }

        foreach (var (eventType, eventData) in eventsToProcess)
        {
            Publish(eventType, eventData);
        }
    }

    /// <summary>
    /// Clear all event listeners
    /// </summary>
    public void ClearAllListeners()
    {
        lock (_lock)
        {
            _listeners.Clear();
            _eventQueue.Clear();
            Logger.Instance.Info("EventSystem", "All event listeners cleared");
        }
    }

    /// <summary>
    /// Get number of listeners for an event type
    /// </summary>
    public int GetListenerCount(string eventType)
    {
        lock (_lock)
        {
            return _listeners.ContainsKey(eventType) ? _listeners[eventType].Count : 0;
        }
    }
}
