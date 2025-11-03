using System.Numerics;

namespace AvorionLike.Core.DevTools;

/// <summary>
/// Debug Renderer - Provides debug visualization capabilities for game objects and physics
/// </summary>
public class DebugRenderer
{
    private List<DebugLine> lines = new();
    private List<DebugBox> boxes = new();
    private bool isEnabled = true;

    public bool IsEnabled
    {
        get => isEnabled;
        set => isEnabled = value;
    }

    /// <summary>
    /// Draw a debug line between two points
    /// </summary>
    public void DrawLine(Vector3 start, Vector3 end, string color = "White", float duration = 0f)
    {
        if (!isEnabled) return;
        lines.Add(new DebugLine { Start = start, End = end, Color = color, Duration = duration });
    }

    /// <summary>
    /// Draw a debug box at a position with given size
    /// </summary>
    public void DrawBox(Vector3 position, Vector3 size, string color = "Green", float duration = 0f)
    {
        if (!isEnabled) return;
        boxes.Add(new DebugBox { Position = position, Size = size, Color = color, Duration = duration });
    }

    /// <summary>
    /// Draw coordinate axes at a position
    /// </summary>
    public void DrawAxes(Vector3 position, float size = 1f)
    {
        DrawLine(position, position + new Vector3(size, 0, 0), "Red");
        DrawLine(position, position + new Vector3(0, size, 0), "Green");
        DrawLine(position, position + new Vector3(0, 0, size), "Blue");
    }

    /// <summary>
    /// Clear all debug visualizations
    /// </summary>
    public void Clear()
    {
        lines.Clear();
        boxes.Clear();
    }

    /// <summary>
    /// Update debug visualizations (removes expired items)
    /// </summary>
    public void Update(float deltaTime)
    {
        lines.RemoveAll(l => l.Duration > 0 && (l.Duration -= deltaTime) <= 0);
        boxes.RemoveAll(b => b.Duration > 0 && (b.Duration -= deltaTime) <= 0);
    }

    /// <summary>
    /// Render all debug visualizations (console output for now)
    /// </summary>
    public void Render()
    {
        if (!isEnabled) return;
        // In a full implementation, this would render to OpenGL/DirectX
        // For console app, we just track the count
    }

    public int GetLineCount() => lines.Count;
    public int GetBoxCount() => boxes.Count;

    private struct DebugLine
    {
        public Vector3 Start { get; set; }
        public Vector3 End { get; set; }
        public string Color { get; set; }
        public float Duration { get; set; }
    }

    private struct DebugBox
    {
        public Vector3 Position { get; set; }
        public Vector3 Size { get; set; }
        public string Color { get; set; }
        public float Duration { get; set; }
    }
}
