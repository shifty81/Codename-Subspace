namespace AvorionLike.Core.Logging;

/// <summary>
/// Log severity levels
/// </summary>
public enum LogLevel
{
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
    Critical = 4
}

/// <summary>
/// Represents a single log entry
/// </summary>
public class LogEntry
{
    public DateTime Timestamp { get; set; }
    public LogLevel Level { get; set; }
    public string Category { get; set; } = "";
    public string Message { get; set; } = "";
    public string? StackTrace { get; set; }

    public override string ToString()
    {
        var levelStr = Level.ToString().ToUpper().PadRight(8);
        var categoryStr = Category.PadRight(15);
        return $"[{Timestamp:yyyy-MM-dd HH:mm:ss.fff}] [{levelStr}] [{categoryStr}] {Message}";
    }
}
