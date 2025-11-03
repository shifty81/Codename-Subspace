using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Common;

/// <summary>
/// Centralized error handling and recovery
/// </summary>
public static class ErrorHandler
{
    /// <summary>
    /// Handle an exception with logging and optional recovery action
    /// </summary>
    public static void Handle(Exception exception, string category, string context, Action? recoveryAction = null)
    {
        var message = $"{context}: {exception.Message}";
        Logger.Instance.Error(category, message, exception);

        // Attempt recovery if provided
        if (recoveryAction != null)
        {
            try
            {
                recoveryAction();
                Logger.Instance.Info(category, $"Recovery action completed for: {context}");
            }
            catch (Exception recoveryEx)
            {
                Logger.Instance.Error(category, $"Recovery action failed for {context}", recoveryEx);
            }
        }
    }

    /// <summary>
    /// Handle a critical exception that may require application shutdown
    /// </summary>
    public static void HandleCritical(Exception exception, string category, string context)
    {
        var message = $"CRITICAL ERROR in {context}: {exception.Message}";
        Logger.Instance.Critical(category, message, exception);
        
        // In a real application, this might trigger a crash report or graceful shutdown
        Console.WriteLine("\n==============================================");
        Console.WriteLine("CRITICAL ERROR DETECTED");
        Console.WriteLine($"Category: {category}");
        Console.WriteLine($"Context: {context}");
        Console.WriteLine($"Message: {exception.Message}");
        Console.WriteLine("==============================================\n");
    }

    /// <summary>
    /// Try to execute an action and handle any exceptions
    /// </summary>
    public static bool TryExecute(Action action, string category, string context)
    {
        try
        {
            action();
            return true;
        }
        catch (Exception ex)
        {
            Handle(ex, category, context);
            return false;
        }
    }

    /// <summary>
    /// Try to execute a function and handle any exceptions
    /// </summary>
    public static bool TryExecute<T>(Func<T> func, string category, string context, out T? result)
    {
        try
        {
            result = func();
            return true;
        }
        catch (Exception ex)
        {
            Handle(ex, category, context);
            result = default;
            return false;
        }
    }

    /// <summary>
    /// Validate a condition and throw a detailed exception if false
    /// </summary>
    public static void Assert(bool condition, string category, string message)
    {
        if (!condition)
        {
            var exception = new InvalidOperationException($"Assertion failed: {message}");
            Logger.Instance.Error(category, message, exception);
            throw exception;
        }
    }
}
