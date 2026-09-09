#pragma once

#include <deque>
#include <string>
#include <vector>

namespace subspace {

enum class DeveloperEventKind {
    Info,
    Command,
    AssetReload,
    EntityEdit,
    ShipEdit,
    Validation,
    Warning,
    Error,
};

struct DeveloperEvent {
    DeveloperEventKind kind = DeveloperEventKind::Info;
    std::string title;
    std::string message;
    std::string source;
    double timestampSeconds = 0.0;
};

class DeveloperEventFeed {
public:
    explicit DeveloperEventFeed(std::size_t maxEvents = 200);
    void Push(DeveloperEvent event);
    std::vector<DeveloperEvent> GetEvents() const;
    std::vector<DeveloperEvent> GetLatest(std::size_t count) const;
    void Clear();

private:
    std::size_t _maxEvents;
    std::deque<DeveloperEvent> _events;
};

const char* ToString(DeveloperEventKind kind);

} // namespace subspace
