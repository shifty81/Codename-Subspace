using System.Diagnostics;

namespace AvorionLike.Core.DevTools;

/// <summary>
/// Performance Profiler - Tracks FPS, frame timing, and performance metrics
/// </summary>
public class PerformanceProfiler
{
    private Stopwatch frameTimer = new();
    private Queue<double> frameTimes = new(120);
    private double totalTime = 0;
    private int frameCount = 0;
    private double lastFpsUpdate = 0;
    private double currentFps = 0;

    public double CurrentFPS => currentFps;
    public double AverageFrameTime => frameTimes.Count > 0 ? frameTimes.Average() : 0;
    public double MinFrameTime => frameTimes.Count > 0 ? frameTimes.Min() : 0;
    public double MaxFrameTime => frameTimes.Count > 0 ? frameTimes.Max() : 0;
    public int FrameCount => frameCount;
    public double TotalTime => totalTime;

    private Dictionary<string, ProfileSection> sections = new();

    public PerformanceProfiler()
    {
        frameTimer.Start();
    }

    /// <summary>
    /// Begin a new frame
    /// </summary>
    public void BeginFrame()
    {
        frameTimer.Restart();
    }

    /// <summary>
    /// End the current frame and update metrics
    /// </summary>
    public void EndFrame()
    {
        double frameTime = frameTimer.Elapsed.TotalMilliseconds;
        frameTimes.Enqueue(frameTime);
        if (frameTimes.Count > 120)
            frameTimes.Dequeue();

        frameCount++;
        totalTime += frameTime;

        // Update FPS every 0.5 seconds
        if (totalTime - lastFpsUpdate >= 500)
        {
            currentFps = 1000.0 / AverageFrameTime;
            lastFpsUpdate = totalTime;
        }
    }

    /// <summary>
    /// Begin timing a specific section of code
    /// </summary>
    public void BeginSection(string name)
    {
        if (!sections.ContainsKey(name))
        {
            sections[name] = new ProfileSection { Name = name };
        }
        sections[name].Timer.Restart();
    }

    /// <summary>
    /// End timing a specific section of code
    /// </summary>
    public void EndSection(string name)
    {
        if (sections.ContainsKey(name))
        {
            sections[name].Timer.Stop();
            sections[name].TotalTime += sections[name].Timer.Elapsed.TotalMilliseconds;
            sections[name].CallCount++;
        }
    }

    /// <summary>
    /// Get timing information for a specific section
    /// </summary>
    public ProfileSection? GetSection(string name)
    {
        return sections.ContainsKey(name) ? sections[name] : null;
    }

    /// <summary>
    /// Get all profiled sections
    /// </summary>
    public IReadOnlyDictionary<string, ProfileSection> GetAllSections()
    {
        return sections;
    }

    /// <summary>
    /// Reset all profiling data
    /// </summary>
    public void Reset()
    {
        frameTimes.Clear();
        totalTime = 0;
        frameCount = 0;
        lastFpsUpdate = 0;
        currentFps = 0;
        sections.Clear();
        frameTimer.Restart();
    }

    /// <summary>
    /// Generate a performance report
    /// </summary>
    public string GenerateReport()
    {
        var report = new System.Text.StringBuilder();
        report.AppendLine("=== Performance Profile Report ===");
        report.AppendLine($"FPS: {currentFps:F2}");
        report.AppendLine($"Avg Frame Time: {AverageFrameTime:F2}ms");
        report.AppendLine($"Min Frame Time: {MinFrameTime:F2}ms");
        report.AppendLine($"Max Frame Time: {MaxFrameTime:F2}ms");
        report.AppendLine($"Total Frames: {frameCount}");
        report.AppendLine($"Total Time: {totalTime / 1000:F2}s");
        report.AppendLine();
        report.AppendLine("=== Timed Sections ===");
        foreach (var section in sections.Values.OrderByDescending(s => s.TotalTime))
        {
            report.AppendLine($"{section.Name}:");
            report.AppendLine($"  Total Time: {section.TotalTime:F2}ms");
            report.AppendLine($"  Calls: {section.CallCount}");
            report.AppendLine($"  Avg Time: {section.AverageTime:F2}ms");
        }
        return report.ToString();
    }

    public class ProfileSection
    {
        public string Name { get; set; } = "";
        public Stopwatch Timer { get; } = new();
        public double TotalTime { get; set; } = 0;
        public int CallCount { get; set; } = 0;
        public double AverageTime => CallCount > 0 && TotalTime > 0 ? TotalTime / CallCount : 0;
    }
}
