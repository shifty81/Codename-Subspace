using System.Collections.Concurrent;

namespace AvorionLike.Core.Logging;

/// <summary>
/// Centralized logging system with multiple output targets
/// </summary>
public class Logger
{
    private static Logger? _instance;
    private readonly ConcurrentQueue<LogEntry> _logQueue = new();
    private StreamWriter? _logFileWriter;
    private LogLevel _minimumLevel = LogLevel.Info;
    private readonly object _fileLock = new();
    private bool _fileLoggingEnabled = false;
    private Task? _logProcessorTask;
    private CancellationTokenSource? _cancellationTokenSource;

    public static Logger Instance
    {
        get
        {
            _instance ??= new Logger();
            return _instance;
        }
    }

    private Logger()
    {
        // Start background log processor
        _cancellationTokenSource = new CancellationTokenSource();
        _logProcessorTask = Task.Run(() => ProcessLogQueue(_cancellationTokenSource.Token));
    }

    /// <summary>
    /// Set the minimum log level to display/save
    /// </summary>
    public void SetMinimumLevel(LogLevel level)
    {
        _minimumLevel = level;
    }

    /// <summary>
    /// Enable file logging to specified path
    /// </summary>
    public void EnableFileLogging(string logDirectory)
    {
        try
        {
            if (!Directory.Exists(logDirectory))
            {
                Directory.CreateDirectory(logDirectory);
            }

            var timestamp = DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss");
            var logPath = Path.Combine(logDirectory, $"AvorionLike_{timestamp}.log");

            lock (_fileLock)
            {
                _logFileWriter?.Dispose();
                _logFileWriter = new StreamWriter(logPath, append: true) { AutoFlush = true };
                _fileLoggingEnabled = true;
            }

            Info("Logger", $"File logging enabled: {logPath}");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to enable file logging: {ex.Message}");
        }
    }

    /// <summary>
    /// Disable file logging
    /// </summary>
    public void DisableFileLogging()
    {
        lock (_fileLock)
        {
            _logFileWriter?.Dispose();
            _logFileWriter = null;
            _fileLoggingEnabled = false;
        }
    }

    /// <summary>
    /// Log a message with specified level
    /// </summary>
    public void Log(LogLevel level, string category, string message, Exception? exception = null)
    {
        if (level < _minimumLevel)
            return;

        var entry = new LogEntry
        {
            Timestamp = DateTime.Now,
            Level = level,
            Category = category,
            Message = message,
            StackTrace = exception?.StackTrace
        };

        _logQueue.Enqueue(entry);
    }

    /// <summary>
    /// Log debug message
    /// </summary>
    public void Debug(string category, string message)
    {
        Log(LogLevel.Debug, category, message);
    }

    /// <summary>
    /// Log info message
    /// </summary>
    public void Info(string category, string message)
    {
        Log(LogLevel.Info, category, message);
    }

    /// <summary>
    /// Log warning message
    /// </summary>
    public void Warning(string category, string message)
    {
        Log(LogLevel.Warning, category, message);
    }

    /// <summary>
    /// Log error message
    /// </summary>
    public void Error(string category, string message, Exception? exception = null)
    {
        Log(LogLevel.Error, category, message, exception);
    }

    /// <summary>
    /// Log critical message
    /// </summary>
    public void Critical(string category, string message, Exception? exception = null)
    {
        Log(LogLevel.Critical, category, message, exception);
    }

    /// <summary>
    /// Process log queue in background
    /// </summary>
    private async Task ProcessLogQueue(CancellationToken cancellationToken)
    {
        while (!cancellationToken.IsCancellationRequested)
        {
            if (_logQueue.TryDequeue(out var entry))
            {
                ProcessLogEntry(entry);
            }
            else
            {
                await Task.Delay(10, cancellationToken);
            }
        }
    }

    /// <summary>
    /// Process a single log entry
    /// </summary>
    private void ProcessLogEntry(LogEntry entry)
    {
        var logText = entry.ToString();

        // Console output with color
        WriteColoredConsole(entry.Level, logText);

        // File output
        if (_fileLoggingEnabled && _logFileWriter != null)
        {
            lock (_fileLock)
            {
                try
                {
                    _logFileWriter?.WriteLine(logText);
                    if (entry.StackTrace != null)
                    {
                        _logFileWriter?.WriteLine($"Stack Trace: {entry.StackTrace}");
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Error writing to log file: {ex.Message}");
                }
            }
        }
    }

    /// <summary>
    /// Write colored console output based on log level
    /// </summary>
    private void WriteColoredConsole(LogLevel level, string message)
    {
        var originalColor = Console.ForegroundColor;
        
        Console.ForegroundColor = level switch
        {
            LogLevel.Debug => ConsoleColor.Gray,
            LogLevel.Info => ConsoleColor.White,
            LogLevel.Warning => ConsoleColor.Yellow,
            LogLevel.Error => ConsoleColor.Red,
            LogLevel.Critical => ConsoleColor.DarkRed,
            _ => ConsoleColor.White
        };

        Console.WriteLine(message);
        Console.ForegroundColor = originalColor;
    }

    /// <summary>
    /// Flush all pending log entries and cleanup
    /// </summary>
    public void Shutdown()
    {
        Info("Logger", "Shutting down logger...");
        
        _cancellationTokenSource?.Cancel();
        _logProcessorTask?.Wait(1000); // Wait up to 1 second for processing to complete

        // Process any remaining entries
        while (_logQueue.TryDequeue(out var entry))
        {
            ProcessLogEntry(entry);
        }

        lock (_fileLock)
        {
            _logFileWriter?.Dispose();
            _logFileWriter = null;
        }

        _cancellationTokenSource?.Dispose();
    }

    /// <summary>
    /// Get recent log entries
    /// </summary>
    public List<LogEntry> GetRecentLogs(int count = 100)
    {
        // For now, return empty list. Could be extended to keep recent logs in memory
        return new List<LogEntry>();
    }
}
