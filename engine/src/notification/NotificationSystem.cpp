#include "notification/NotificationSystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// Notification
// ---------------------------------------------------------------------------

std::string Notification::GetCategoryName(NotificationCategory cat) {
    switch (cat) {
        case NotificationCategory::Combat:     return "Combat";
        case NotificationCategory::Trade:      return "Trade";
        case NotificationCategory::Diplomacy:  return "Diplomacy";
        case NotificationCategory::Research:   return "Research";
        case NotificationCategory::Navigation: return "Navigation";
        case NotificationCategory::System:     return "System";
    }
    return "System";
}

std::string Notification::GetPriorityName(NotificationPriority prio) {
    switch (prio) {
        case NotificationPriority::Low:      return "Low";
        case NotificationPriority::Normal:   return "Normal";
        case NotificationPriority::High:     return "High";
        case NotificationPriority::Critical: return "Critical";
    }
    return "Normal";
}

// ---------------------------------------------------------------------------
// NotificationComponent
// ---------------------------------------------------------------------------

int NotificationComponent::AddNotification(const std::string& title,
                                           const std::string& message,
                                           NotificationCategory cat,
                                           NotificationPriority prio,
                                           float duration) {
    // Remove oldest if at capacity
    while (static_cast<int>(notifications.size()) >= maxNotifications) {
        notifications.erase(notifications.begin());
    }

    Notification n;
    n.notificationId = _nextId++;
    n.title = title;
    n.message = message;
    n.category = cat;
    n.priority = prio;
    n.timeRemaining = duration;
    n.totalTime = duration;
    n.isRead = false;
    n.isExpired = false;
    notifications.push_back(n);
    return n.notificationId;
}

bool NotificationComponent::MarkAsRead(int id) {
    for (auto& n : notifications) {
        if (n.notificationId == id) {
            n.isRead = true;
            return true;
        }
    }
    return false;
}

void NotificationComponent::MarkAllAsRead() {
    for (auto& n : notifications) {
        n.isRead = true;
    }
}

bool NotificationComponent::RemoveNotification(int id) {
    auto it = std::find_if(notifications.begin(), notifications.end(),
                           [id](const Notification& n) { return n.notificationId == id; });
    if (it == notifications.end()) return false;
    notifications.erase(it);
    return true;
}

int NotificationComponent::RemoveExpired() {
    int removed = 0;
    auto it = notifications.begin();
    while (it != notifications.end()) {
        if (it->isExpired) {
            it = notifications.erase(it);
            ++removed;
        } else {
            ++it;
        }
    }
    return removed;
}

int NotificationComponent::GetUnreadCount() const {
    int count = 0;
    for (const auto& n : notifications) {
        if (!n.isRead && !n.isExpired) ++count;
    }
    return count;
}

int NotificationComponent::GetActiveCount() const {
    int count = 0;
    for (const auto& n : notifications) {
        if (!n.isExpired) ++count;
    }
    return count;
}

std::vector<const Notification*> NotificationComponent::GetByCategory(NotificationCategory cat) const {
    std::vector<const Notification*> result;
    for (const auto& n : notifications) {
        if (n.category == cat && !n.isExpired) {
            result.push_back(&n);
        }
    }
    return result;
}

std::vector<const Notification*> NotificationComponent::GetByMinPriority(NotificationPriority minPrio) const {
    std::vector<const Notification*> result;
    for (const auto& n : notifications) {
        if (!n.isExpired && static_cast<int>(n.priority) >= static_cast<int>(minPrio)) {
            result.push_back(&n);
        }
    }
    return result;
}

const Notification* NotificationComponent::FindNotification(int id) const {
    for (const auto& n : notifications) {
        if (n.notificationId == id) return &n;
    }
    return nullptr;
}

bool NotificationComponent::HasCriticalUnread() const {
    for (const auto& n : notifications) {
        if (!n.isRead && !n.isExpired && n.priority == NotificationPriority::Critical) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData NotificationComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "NotificationComponent";
    cd.data["maxNotifications"]  = std::to_string(maxNotifications);
    cd.data["autoRemoveExpired"] = autoRemoveExpired ? "1" : "0";
    cd.data["nextId"]            = std::to_string(_nextId);

    // Only serialize non-expired notifications
    int count = 0;
    for (size_t i = 0; i < notifications.size(); ++i) {
        const auto& n = notifications[i];
        if (n.isExpired) continue;

        std::string prefix = "notif_" + std::to_string(count) + "_";
        cd.data[prefix + "id"]       = std::to_string(n.notificationId);
        cd.data[prefix + "title"]    = n.title;
        cd.data[prefix + "message"]  = n.message;
        cd.data[prefix + "category"] = std::to_string(static_cast<int>(n.category));
        cd.data[prefix + "priority"] = std::to_string(static_cast<int>(n.priority));
        cd.data[prefix + "timeRemaining"] = std::to_string(n.timeRemaining);
        cd.data[prefix + "totalTime"]     = std::to_string(n.totalTime);
        cd.data[prefix + "isRead"]   = n.isRead ? "1" : "0";
        ++count;
    }
    cd.data["notifCount"] = std::to_string(count);

    return cd;
}

void NotificationComponent::Deserialize(const ComponentData& data) {
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

    maxNotifications  = getInt("maxNotifications", 50);
    autoRemoveExpired = getStr("autoRemoveExpired") != "0";
    _nextId           = getInt("nextId", 1);

    int count = getInt("notifCount", 0);
    notifications.clear();
    notifications.reserve(static_cast<size_t>(count));

    for (int i = 0; i < count; ++i) {
        std::string prefix = "notif_" + std::to_string(i) + "_";
        Notification n;
        n.notificationId = getInt(prefix + "id", 0);
        n.title          = getStr(prefix + "title");
        n.message        = getStr(prefix + "message");

        constexpr int kMaxCategory = static_cast<int>(NotificationCategory::System);
        int catVal = getInt(prefix + "category", kMaxCategory);
        if (catVal >= 0 && catVal <= kMaxCategory) {
            n.category = static_cast<NotificationCategory>(catVal);
        } else {
            n.category = NotificationCategory::System;
        }

        constexpr int kMaxPriority = static_cast<int>(NotificationPriority::Critical);
        int prioVal = getInt(prefix + "priority", static_cast<int>(NotificationPriority::Normal));
        if (prioVal >= 0 && prioVal <= kMaxPriority) {
            n.priority = static_cast<NotificationPriority>(prioVal);
        } else {
            n.priority = NotificationPriority::Normal;
        }

        n.timeRemaining = getFloat(prefix + "timeRemaining", -1.0f);
        n.totalTime     = getFloat(prefix + "totalTime", -1.0f);
        n.isRead        = getStr(prefix + "isRead") == "1";
        n.isExpired     = false;

        notifications.push_back(n);
    }
}

// ---------------------------------------------------------------------------
// NotificationSystem
// ---------------------------------------------------------------------------

NotificationSystem::NotificationSystem() : SystemBase("NotificationSystem") {}

NotificationSystem::NotificationSystem(EntityManager& entityManager)
    : SystemBase("NotificationSystem")
    , _entityManager(&entityManager)
{
}

void NotificationSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto components = _entityManager->GetAllComponents<NotificationComponent>();
    for (auto* nc : components) {
        for (auto& n : nc->notifications) {
            if (n.isExpired) continue;
            if (n.timeRemaining < 0.0f) continue; // persistent

            n.timeRemaining -= deltaTime;
            if (n.timeRemaining <= 0.0f) {
                n.timeRemaining = 0.0f;
                n.isExpired = true;
            }
        }

        // Auto-remove expired if enabled
        if (nc->autoRemoveExpired) {
            nc->RemoveExpired();
        }
    }
}

} // namespace subspace
