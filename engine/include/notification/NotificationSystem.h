#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <cstdint>

namespace subspace {

/// Notification categories for filtering.
enum class NotificationCategory { Combat, Trade, Diplomacy, Research, Navigation, System };

/// Priority levels for notifications.
enum class NotificationPriority { Low, Normal, High, Critical };

/// A single notification entry.
struct Notification {
    int notificationId = 0;
    std::string title;
    std::string message;
    NotificationCategory category = NotificationCategory::System;
    NotificationPriority priority = NotificationPriority::Normal;
    float timeRemaining = -1.0f;   // seconds until expiry, -1 = persistent
    float totalTime = -1.0f;
    bool isRead = false;
    bool isExpired = false;

    /// Get the display name for a category.
    static std::string GetCategoryName(NotificationCategory cat);

    /// Get the display name for a priority.
    static std::string GetPriorityName(NotificationPriority prio);
};

/// ECS component that stores notifications for an entity (typically the player).
struct NotificationComponent : public IComponent {
    std::vector<Notification> notifications;
    int maxNotifications = 50;
    bool autoRemoveExpired = true;

    /// Add a notification. Returns the assigned notification ID.
    int AddNotification(const std::string& title, const std::string& message,
                        NotificationCategory cat = NotificationCategory::System,
                        NotificationPriority prio = NotificationPriority::Normal,
                        float duration = -1.0f);

    /// Mark a notification as read by ID. Returns true if found.
    bool MarkAsRead(int id);

    /// Mark all notifications as read.
    void MarkAllAsRead();

    /// Remove a notification by ID. Returns true if removed.
    bool RemoveNotification(int id);

    /// Remove all expired notifications.
    int RemoveExpired();

    /// Get unread count.
    int GetUnreadCount() const;

    /// Get total count (non-expired).
    int GetActiveCount() const;

    /// Get notifications by category.
    std::vector<const Notification*> GetByCategory(NotificationCategory cat) const;

    /// Get notifications by minimum priority.
    std::vector<const Notification*> GetByMinPriority(NotificationPriority minPrio) const;

    /// Find a notification by ID. Returns nullptr if not found.
    const Notification* FindNotification(int id) const;

    /// Check if there are any critical unread notifications.
    bool HasCriticalUnread() const;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);

private:
    int _nextId = 1;
};

/// System that ticks notification lifetimes each frame.
class NotificationSystem : public SystemBase {
public:
    NotificationSystem();
    explicit NotificationSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
