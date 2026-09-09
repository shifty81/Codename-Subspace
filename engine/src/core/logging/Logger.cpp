#include "core/logging/Logger.h"

namespace subspace {

Logger& Logger::Instance()
{
    static Logger instance;
    return instance;
}

void Logger::SetMinimumLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _minimumLevel = level;
}

LogLevel Logger::GetMinimumLevel() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _minimumLevel;
}

void Logger::Log(LogLevel level, const std::string& category, const std::string& message)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (level < _minimumLevel) return;

    LogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.level = level;
    entry.category = category;
    entry.message = message;

    _recentLogs.push_back(entry);
    if (_recentLogs.size() > kMaxRecentLogs) {
        _recentLogs.erase(_recentLogs.begin());
    }

    // Console output
    const char* tag = "INFO";
    switch (level) {
        case LogLevel::Debug:    tag = "DEBUG"; break;
        case LogLevel::Info:     tag = "INFO";  break;
        case LogLevel::Warning:  tag = "WARN";  break;
        case LogLevel::Error:    tag = "ERROR"; break;
        case LogLevel::Critical: tag = "CRIT";  break;
    }
    std::cout << "[" << tag << "] [" << category << "] " << message << "\n";
}

void Logger::Debug(const std::string& category, const std::string& message)
{
    Log(LogLevel::Debug, category, message);
}

void Logger::Info(const std::string& category, const std::string& message)
{
    Log(LogLevel::Info, category, message);
}

void Logger::Warning(const std::string& category, const std::string& message)
{
    Log(LogLevel::Warning, category, message);
}

void Logger::Error(const std::string& category, const std::string& message)
{
    Log(LogLevel::Error, category, message);
}

void Logger::Critical(const std::string& category, const std::string& message)
{
    Log(LogLevel::Critical, category, message);
}

std::vector<LogEntry> Logger::GetRecentLogs(int count) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (count <= 0 || _recentLogs.empty()) return {};
    size_t n = std::min(static_cast<size_t>(count), _recentLogs.size());
    return std::vector<LogEntry>(_recentLogs.end() - static_cast<ptrdiff_t>(n), _recentLogs.end());
}

} // namespace subspace
