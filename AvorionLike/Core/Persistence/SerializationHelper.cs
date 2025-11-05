using System.Numerics;
using System.Text.Json;

namespace AvorionLike.Core.Persistence;

/// <summary>
/// Helper methods for serializing and deserializing complex types
/// </summary>
public static class SerializationHelper
{
    /// <summary>
    /// Serialize a Vector3 to a dictionary
    /// </summary>
    public static Dictionary<string, object> SerializeVector3(Vector3 vector)
    {
        return new Dictionary<string, object>
        {
            ["X"] = vector.X,
            ["Y"] = vector.Y,
            ["Z"] = vector.Z
        };
    }

    /// <summary>
    /// Deserialize a Vector3 from a dictionary
    /// </summary>
    public static Vector3 DeserializeVector3(Dictionary<string, object> data)
    {
        return new Vector3(
            Convert.ToSingle(data["X"]),
            Convert.ToSingle(data["Y"]),
            Convert.ToSingle(data["Z"])
        );
    }

    /// <summary>
    /// Deserialize a Vector3 from an object (handles JsonElement case)
    /// </summary>
    public static Vector3 DeserializeVector3(object obj)
    {
        if (obj is Dictionary<string, object> dict)
        {
            return DeserializeVector3(dict);
        }
        else if (obj is JsonElement jsonElement && jsonElement.ValueKind == JsonValueKind.Object)
        {
            return new Vector3(
                jsonElement.GetProperty("X").GetSingle(),
                jsonElement.GetProperty("Y").GetSingle(),
                jsonElement.GetProperty("Z").GetSingle()
            );
        }
        return Vector3.Zero;
    }

    /// <summary>
    /// Serialize a dictionary with enum keys to a JSON-compatible format
    /// </summary>
    public static Dictionary<string, object> SerializeDictionary<TKey, TValue>(Dictionary<TKey, TValue> dict) 
        where TKey : notnull
    {
        var result = new Dictionary<string, object>();
        foreach (var kvp in dict)
        {
            result[kvp.Key.ToString() ?? ""] = kvp.Value!;
        }
        return result;
    }

    /// <summary>
    /// Deserialize a dictionary with enum keys from a JSON-compatible format
    /// </summary>
    public static Dictionary<TKey, TValue> DeserializeDictionary<TKey, TValue>(Dictionary<string, object> data) 
        where TKey : struct, Enum
    {
        var result = new Dictionary<TKey, TValue>();
        foreach (var kvp in data)
        {
            if (Enum.TryParse<TKey>(kvp.Key, out var key))
            {
                if (kvp.Value is JsonElement jsonElement)
                {
                    // Handle JsonElement conversion
                    TValue? value = JsonSerializer.Deserialize<TValue>(jsonElement.GetRawText());
                    if (value != null)
                    {
                        result[key] = value;
                    }
                }
                else
                {
                    result[key] = (TValue)Convert.ChangeType(kvp.Value, typeof(TValue));
                }
            }
        }
        return result;
    }

    /// <summary>
    /// Deserialize a string-key dictionary
    /// </summary>
    public static Dictionary<string, TValue> DeserializeStringDictionary<TValue>(Dictionary<string, object> data)
    {
        var result = new Dictionary<string, TValue>();
        foreach (var kvp in data)
        {
            if (kvp.Value is JsonElement jsonElement)
            {
                // Handle JsonElement conversion
                TValue? value = JsonSerializer.Deserialize<TValue>(jsonElement.GetRawText());
                if (value != null)
                {
                    result[kvp.Key] = value;
                }
            }
            else
            {
                result[kvp.Key] = (TValue)Convert.ChangeType(kvp.Value, typeof(TValue));
            }
        }
        return result;
    }

    /// <summary>
    /// Safely get a value from a dictionary, with default fallback
    /// </summary>
    public static T GetValue<T>(Dictionary<string, object> data, string key, T defaultValue)
    {
        if (!data.ContainsKey(key))
            return defaultValue;

        try
        {
            var value = data[key];
            if (value is JsonElement jsonElement)
            {
                // Handle JsonElement conversion
                return JsonSerializer.Deserialize<T>(jsonElement.GetRawText()) ?? defaultValue;
            }
            return (T)Convert.ChangeType(value, typeof(T));
        }
        catch
        {
            return defaultValue;
        }
    }
}
