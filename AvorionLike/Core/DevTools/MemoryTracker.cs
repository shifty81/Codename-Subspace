using System.Diagnostics;

namespace AvorionLike.Core.DevTools;

/// <summary>
/// Memory Tracker - Monitors memory usage including GPU memory (when available)
/// </summary>
public class MemoryTracker
{
    private Process currentProcess;
    private long peakMemoryUsage = 0;
    private Queue<long> memoryHistory = new(100);

    public long CurrentMemoryUsage => currentProcess.WorkingSet64;
    public long PeakMemoryUsage => peakMemoryUsage;
    public long ManagedMemory => GC.GetTotalMemory(false);
    public double MemoryUsageMB => CurrentMemoryUsage / (1024.0 * 1024.0);
    public double ManagedMemoryMB => ManagedMemory / (1024.0 * 1024.0);
    
    // GPU memory tracking (placeholder for future OpenGL implementation)
    private long gpuMemoryUsed = 0;
    private long gpuMemoryTotal = 0;
    
    public long GPUMemoryUsed => gpuMemoryUsed;
    public long GPUMemoryTotal => gpuMemoryTotal;
    public double GPUMemoryUsedMB => gpuMemoryUsed / (1024.0 * 1024.0);

    public MemoryTracker()
    {
        currentProcess = Process.GetCurrentProcess();
    }

    /// <summary>
    /// Update memory tracking metrics
    /// </summary>
    public void Update()
    {
        currentProcess.Refresh();
        long currentMemory = CurrentMemoryUsage;
        
        if (currentMemory > peakMemoryUsage)
            peakMemoryUsage = currentMemory;

        memoryHistory.Enqueue(currentMemory);
        if (memoryHistory.Count > 100)
            memoryHistory.Dequeue();
    }

    /// <summary>
    /// Set GPU memory usage (to be called by OpenGL renderer)
    /// </summary>
    public void SetGPUMemoryUsage(long used, long total)
    {
        gpuMemoryUsed = used;
        gpuMemoryTotal = total;
    }

    /// <summary>
    /// Force garbage collection and return freed memory
    /// </summary>
    public long ForceGarbageCollection()
    {
        long beforeGC = ManagedMemory;
        GC.Collect();
        GC.WaitForPendingFinalizers();
        GC.Collect();
        long afterGC = ManagedMemory;
        return beforeGC - afterGC;
    }

    /// <summary>
    /// Get memory usage trend (increasing/decreasing/stable)
    /// </summary>
    public string GetMemoryTrend()
    {
        if (memoryHistory.Count < 10)
            return "Insufficient Data";

        var recent = memoryHistory.TakeLast(10).ToArray();
        var older = memoryHistory.Take(10).ToArray();

        double recentAvg = recent.Average();
        double olderAvg = older.Average();

        double change = (recentAvg - olderAvg) / olderAvg * 100;

        if (Math.Abs(change) < 5)
            return "Stable";
        else if (change > 0)
            return $"Increasing ({change:F1}%)";
        else
            return $"Decreasing ({Math.Abs(change):F1}%)";
    }

    /// <summary>
    /// Generate a memory usage report
    /// </summary>
    public string GenerateReport()
    {
        var report = new System.Text.StringBuilder();
        report.AppendLine("=== Memory Usage Report ===");
        report.AppendLine($"Current Memory: {MemoryUsageMB:F2} MB");
        report.AppendLine($"Peak Memory: {PeakMemoryUsage / (1024.0 * 1024.0):F2} MB");
        report.AppendLine($"Managed Memory: {ManagedMemoryMB:F2} MB");
        report.AppendLine($"Memory Trend: {GetMemoryTrend()}");
        
        if (gpuMemoryTotal > 0)
        {
            report.AppendLine();
            report.AppendLine("=== GPU Memory ===");
            report.AppendLine($"GPU Memory Used: {GPUMemoryUsedMB:F2} MB");
            report.AppendLine($"GPU Memory Total: {gpuMemoryTotal / (1024.0 * 1024.0):F2} MB");
            report.AppendLine($"GPU Memory Usage: {(double)gpuMemoryUsed / gpuMemoryTotal * 100:F1}%");
        }
        else
        {
            report.AppendLine();
            report.AppendLine("GPU Memory Tracking: Not Available (OpenGL not initialized)");
        }
        
        return report.ToString();
    }

    /// <summary>
    /// Get GC generation counts
    /// </summary>
    public (int Gen0, int Gen1, int Gen2) GetGCCounts()
    {
        return (
            GC.CollectionCount(0),
            GC.CollectionCount(1),
            GC.CollectionCount(2)
        );
    }
}
