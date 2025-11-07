using System.Collections.Concurrent;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Multithreaded mesh generation for voxel chunks
/// </summary>
public class ThreadedMeshBuilder
{
    private readonly ConcurrentQueue<MeshBuildTask> _taskQueue = new();
    private readonly ConcurrentQueue<MeshBuildResult> _resultQueue = new();
    private readonly CancellationTokenSource _cancellationTokenSource = new();
    private readonly Thread[] _workerThreads;
    private readonly int _threadCount;
    private bool _isRunning = false;
    
    public ThreadedMeshBuilder(int threadCount = 0)
    {
        // Use half the CPU cores for meshing (leave some for other tasks)
        _threadCount = threadCount > 0 ? threadCount : Math.Max(1, Environment.ProcessorCount / 2);
        _workerThreads = new Thread[_threadCount];
    }
    
    /// <summary>
    /// Start the mesh building threads
    /// </summary>
    public void Start()
    {
        if (_isRunning)
            return;
            
        _isRunning = true;
        
        for (int i = 0; i < _threadCount; i++)
        {
            _workerThreads[i] = new Thread(WorkerThreadLoop)
            {
                IsBackground = true,
                Name = $"MeshBuilder-{i}"
            };
            _workerThreads[i].Start();
        }
    }
    
    /// <summary>
    /// Stop the mesh building threads
    /// </summary>
    public void Stop()
    {
        if (!_isRunning)
            return;
            
        _isRunning = false;
        _cancellationTokenSource.Cancel();
        
        // Wait for threads to finish
        foreach (var thread in _workerThreads)
        {
            thread?.Join(1000);
        }
    }
    
    /// <summary>
    /// Request mesh build for a chunk
    /// </summary>
    public void RequestMeshBuild(VoxelChunk chunk, bool useGreedyMeshing = false)
    {
        _taskQueue.Enqueue(new MeshBuildTask
        {
            Chunk = chunk,
            UseGreedyMeshing = useGreedyMeshing
        });
    }
    
    /// <summary>
    /// Process completed mesh build results (call from main thread)
    /// </summary>
    public List<MeshBuildResult> ProcessResults()
    {
        var results = new List<MeshBuildResult>();
        int processedCount = 0;
        int maxPerFrame = 5; // Limit processing per frame
        
        while (processedCount < maxPerFrame && _resultQueue.TryDequeue(out var result))
        {
            results.Add(result);
            processedCount++;
        }
        
        return results;
    }
    
    /// <summary>
    /// Get number of pending mesh build tasks
    /// </summary>
    public int GetPendingTaskCount()
    {
        return _taskQueue.Count;
    }
    
    /// <summary>
    /// Get number of completed results waiting to be processed
    /// </summary>
    public int GetPendingResultCount()
    {
        return _resultQueue.Count;
    }
    
    /// <summary>
    /// Worker thread loop
    /// </summary>
    private void WorkerThreadLoop()
    {
        while (_isRunning && !_cancellationTokenSource.Token.IsCancellationRequested)
        {
            if (_taskQueue.TryDequeue(out var task))
            {
                try
                {
                    var result = ProcessTask(task);
                    _resultQueue.Enqueue(result);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Error in mesh building thread: {ex.Message}");
                }
            }
            else
            {
                // No tasks, sleep briefly
                Thread.Sleep(10);
            }
        }
    }
    
    /// <summary>
    /// Process a mesh build task
    /// </summary>
    private MeshBuildResult ProcessTask(MeshBuildTask task)
    {
        var result = new MeshBuildResult
        {
            Chunk = task.Chunk
        };
        
        try
        {
            // Build mesh based on strategy
            if (task.UseGreedyMeshing)
            {
                result.Mesh = GreedyMeshBuilder.BuildGreedyMesh(task.Chunk.Blocks);
            }
            else
            {
                result.Mesh = GreedyMeshBuilder.BuildMesh(task.Chunk.Blocks);
            }
            
            result.Success = true;
        }
        catch (Exception ex)
        {
            result.Success = false;
            result.ErrorMessage = ex.Message;
        }
        
        return result;
    }
}

/// <summary>
/// Mesh build task data
/// </summary>
public class MeshBuildTask
{
    public VoxelChunk Chunk { get; set; } = null!;
    public bool UseGreedyMeshing { get; set; } = false;
}

/// <summary>
/// Mesh build result data
/// </summary>
public class MeshBuildResult
{
    public VoxelChunk Chunk { get; set; } = null!;
    public OptimizedMesh? Mesh { get; set; }
    public bool Success { get; set; }
    public string ErrorMessage { get; set; } = string.Empty;
}
