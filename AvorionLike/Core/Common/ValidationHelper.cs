namespace AvorionLike.Core.Common;

/// <summary>
/// Validation helper for parameter and state validation
/// </summary>
public static class ValidationHelper
{
    /// <summary>
    /// Validate that an object is not null
    /// </summary>
    public static void ValidateNotNull(object? obj, string paramName)
    {
        if (obj == null)
        {
            throw new ArgumentNullException(paramName, $"Parameter '{paramName}' cannot be null");
        }
    }

    /// <summary>
    /// Validate that a string is not null or empty
    /// </summary>
    public static void ValidateNotNullOrEmpty(string? value, string paramName)
    {
        if (string.IsNullOrEmpty(value))
        {
            throw new ArgumentException($"Parameter '{paramName}' cannot be null or empty", paramName);
        }
    }

    /// <summary>
    /// Validate that a value is within a specified range
    /// </summary>
    public static void ValidateRange(float value, float min, float max, string paramName)
    {
        if (value < min || value > max)
        {
            throw new ArgumentOutOfRangeException(
                paramName,
                value,
                $"Parameter '{paramName}' must be between {min} and {max}, but was {value}"
            );
        }
    }

    /// <summary>
    /// Validate that an integer value is within a specified range
    /// </summary>
    public static void ValidateRange(int value, int min, int max, string paramName)
    {
        if (value < min || value > max)
        {
            throw new ArgumentOutOfRangeException(
                paramName,
                value,
                $"Parameter '{paramName}' must be between {min} and {max}, but was {value}"
            );
        }
    }

    /// <summary>
    /// Validate that a value is positive
    /// </summary>
    public static void ValidatePositive(float value, string paramName)
    {
        if (value <= 0)
        {
            throw new ArgumentException($"Parameter '{paramName}' must be positive, but was {value}", paramName);
        }
    }

    /// <summary>
    /// Validate that a value is non-negative
    /// </summary>
    public static void ValidateNonNegative(float value, string paramName)
    {
        if (value < 0)
        {
            throw new ArgumentException($"Parameter '{paramName}' must be non-negative, but was {value}", paramName);
        }
    }

    /// <summary>
    /// Validate that a collection is not null or empty
    /// </summary>
    public static void ValidateNotNullOrEmpty<T>(IEnumerable<T>? collection, string paramName)
    {
        if (collection == null || !collection.Any())
        {
            throw new ArgumentException($"Parameter '{paramName}' cannot be null or empty", paramName);
        }
    }

    /// <summary>
    /// Validate that a GUID is not empty
    /// </summary>
    public static void ValidateNotEmpty(Guid value, string paramName)
    {
        if (value == Guid.Empty)
        {
            throw new ArgumentException($"Parameter '{paramName}' cannot be an empty GUID", paramName);
        }
    }

    /// <summary>
    /// Validate that a condition is true
    /// </summary>
    public static void Validate(bool condition, string message)
    {
        if (!condition)
        {
            throw new InvalidOperationException(message);
        }
    }
}
