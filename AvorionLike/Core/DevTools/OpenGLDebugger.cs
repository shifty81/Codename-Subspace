namespace AvorionLike.Core.DevTools;

/// <summary>
/// OpenGL Debugger - Error detection and logging for OpenGL operations
/// </summary>
public class OpenGLDebugger
{
    private bool isEnabled = true;
    private List<GLError> errors = new();
    private Dictionary<string, int> errorCounts = new();

    public bool IsEnabled
    {
        get => isEnabled;
        set => isEnabled = value;
    }

    public int ErrorCount => errors.Count;

    /// <summary>
    /// Log an OpenGL error
    /// </summary>
    public void LogError(string errorCode, string function, string message)
    {
        if (!isEnabled) return;

        var error = new GLError
        {
            ErrorCode = errorCode,
            Function = function,
            Message = message,
            Timestamp = DateTime.Now
        };

        errors.Add(error);

        string key = $"{errorCode}_{function}";
        if (!errorCounts.ContainsKey(key))
            errorCounts[key] = 0;
        errorCounts[key]++;

        // Log to console in debug mode
        Console.WriteLine($"[OpenGL Error] {errorCode} in {function}: {message}");
    }

    /// <summary>
    /// Check for OpenGL errors (placeholder for actual glGetError call)
    /// </summary>
    public void CheckError(string function)
    {
        if (!isEnabled) return;
        
        // In a real implementation, this would call glGetError()
        // For now, this is a placeholder that can be implemented when OpenGL is added
        // int error = GL.GetError();
        // if (error != GL.NO_ERROR)
        //     LogError(error.ToString(), function, GetErrorMessage(error));
    }

    /// <summary>
    /// Enable OpenGL debug output callback (placeholder)
    /// </summary>
    public void EnableDebugOutput()
    {
        // In a real implementation, this would set up OpenGL debug callback
        // GL.Enable(EnableCap.DebugOutput);
        // GL.DebugMessageCallback(DebugCallback, IntPtr.Zero);
        Console.WriteLine("[OpenGL Debug] Debug output enabled (placeholder)");
    }

    /// <summary>
    /// Log a debug message
    /// </summary>
    public void LogMessage(string severity, string message)
    {
        if (!isEnabled) return;
        Console.WriteLine($"[OpenGL {severity}] {message}");
    }

    /// <summary>
    /// Clear all logged errors
    /// </summary>
    public void ClearErrors()
    {
        errors.Clear();
        errorCounts.Clear();
    }

    /// <summary>
    /// Get all errors
    /// </summary>
    public IReadOnlyList<GLError> GetErrors()
    {
        return errors.AsReadOnly();
    }

    /// <summary>
    /// Get error statistics
    /// </summary>
    public Dictionary<string, int> GetErrorStatistics()
    {
        return new Dictionary<string, int>(errorCounts);
    }

    /// <summary>
    /// Generate an error report
    /// </summary>
    public string GenerateReport()
    {
        var report = new System.Text.StringBuilder();
        report.AppendLine("=== OpenGL Debug Report ===");
        report.AppendLine($"Total Errors: {ErrorCount}");
        report.AppendLine($"Debug Output: {(isEnabled ? "Enabled" : "Disabled")}");
        
        if (errorCounts.Count > 0)
        {
            report.AppendLine();
            report.AppendLine("=== Error Statistics ===");
            foreach (var kvp in errorCounts.OrderByDescending(x => x.Value))
            {
                report.AppendLine($"{kvp.Key}: {kvp.Value} occurrences");
            }
        }

        if (errors.Count > 0)
        {
            report.AppendLine();
            report.AppendLine("=== Recent Errors (Last 10) ===");
            foreach (var error in errors.TakeLast(10))
            {
                report.AppendLine($"[{error.Timestamp:HH:mm:ss}] {error.ErrorCode} in {error.Function}");
                report.AppendLine($"  {error.Message}");
            }
        }

        return report.ToString();
    }

    public struct GLError
    {
        public string ErrorCode { get; set; }
        public string Function { get; set; }
        public string Message { get; set; }
        public DateTime Timestamp { get; set; }
    }
}
