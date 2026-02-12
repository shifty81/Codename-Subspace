#pragma once

#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

namespace subspace {

enum class LogLevel { Debug = 0, Info = 1, Warning = 2, Error = 3, Critical = 4 };

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::string category;
    std::string message;
};

/// Centralized logging system (singleton).
class Logger {
public:
    static Logger& Instance();

    void SetMinimumLevel(LogLevel level);
    LogLevel GetMinimumLevel() const;

    void Log(LogLevel level, const std::string& category, const std::string& message);
    void Debug(const std::string& category, const std::string& message);
    void Info(const std::string& category, const std::string& message);
    void Warning(const std::string& category, const std::string& message);
    void Error(const std::string& category, const std::string& message);
    void Critical(const std::string& category, const std::string& message);

    std::vector<LogEntry> GetRecentLogs(int count = 100) const;

private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    mutable std::mutex _mutex;
    LogLevel _minimumLevel = LogLevel::Info;
    std::vector<LogEntry> _recentLogs;
    static constexpr size_t kMaxRecentLogs = 1000;
};

} // namespace subspace
