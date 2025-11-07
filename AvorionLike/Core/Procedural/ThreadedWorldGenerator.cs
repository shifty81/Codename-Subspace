using System.Collections.Concurrent;
using System.Numerics;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Multithreaded world generation system
/// </summary>
public class ThreadedWorldGenerator
{
    private readonly GalaxyGenerator _galaxyGenerator;
    private readonly AsteroidVoxelGenerator _asteroidGenerator;
    private readonly ChunkManager _chunkManager;
    private readonly ConcurrentQueue<GenerationTask> _taskQueue = new();
    private readonly ConcurrentQueue<GenerationResult> _resultQueue = new();
    private readonly CancellationTokenSource _cancellationTokenSource = new();
    private readonly Thread[] _workerThreads;
    private readonly int _threadCount;
    private bool _isRunning = false;
    
    public ThreadedWorldGenerator(
        int seed,
        ChunkManager chunkManager,
        int threadCount = 0)
    {
        _galaxyGenerator = new GalaxyGenerator(seed);
        _asteroidGenerator = new AsteroidVoxelGenerator(seed);
        _chunkManager = chunkManager;
        
        // Use CPU core count if not specified
        _threadCount = threadCount > 0 ? threadCount : Math.Max(1, Environment.ProcessorCount - 1);
        _workerThreads = new Thread[_threadCount];
    }
    
    /// <summary>
    /// Start the generation threads
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
                Name = $"WorldGen-{i}"
            };
            _workerThreads[i].Start();
        }
    }
    
    /// <summary>
    /// Stop the generation threads
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
            thread?.Join(1000); // Wait up to 1 second per thread
        }
    }
    
    /// <summary>
    /// Request generation of a sector
    /// </summary>
    public void RequestSectorGeneration(int x, int y, int z)
    {
        _taskQueue.Enqueue(new GenerationTask
        {
            Type = TaskType.Sector,
            SectorX = x,
            SectorY = y,
            SectorZ = z
        });
    }
    
    /// <summary>
    /// Request generation of an asteroid
    /// </summary>
    public void RequestAsteroidGeneration(AsteroidData asteroidData, int resolution = 8)
    {
        _taskQueue.Enqueue(new GenerationTask
        {
            Type = TaskType.Asteroid,
            AsteroidData = asteroidData,
            Resolution = resolution
        });
    }
    
    /// <summary>
    /// Process completed generation results (call from main thread)
    /// </summary>
    public void ProcessResults()
    {
        int processedCount = 0;
        int maxPerFrame = 10; // Limit processing per frame to avoid hitching
        
        while (processedCount < maxPerFrame && _resultQueue.TryDequeue(out var result))
        {
            switch (result.Type)
            {
                case TaskType.Sector:
                    ProcessSectorResult(result);
                    break;
                    
                case TaskType.Asteroid:
                    ProcessAsteroidResult(result);
                    break;
            }
            
            processedCount++;
        }
    }
    
    /// <summary>
    /// Get number of pending tasks
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
                    Console.WriteLine($"Error in world generation thread: {ex.Message}");
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
    /// Process a generation task
    /// </summary>
    private GenerationResult ProcessTask(GenerationTask task)
    {
        var result = new GenerationResult
        {
            Type = task.Type,
            Task = task
        };
        
        switch (task.Type)
        {
            case TaskType.Sector:
                result.Sector = _galaxyGenerator.GenerateSector(
                    task.SectorX,
                    task.SectorY,
                    task.SectorZ
                );
                break;
                
            case TaskType.Asteroid:
                if (task.AsteroidData != null)
                {
                    result.VoxelBlocks = _asteroidGenerator.GenerateAsteroid(
                        task.AsteroidData,
                        task.Resolution
                    );
                }
                break;
        }
        
        return result;
    }
    
    /// <summary>
    /// Process a completed sector generation
    /// </summary>
    private void ProcessSectorResult(GenerationResult result)
    {
        if (result.Sector == null)
            return;
            
        // Queue asteroid generation for this sector
        foreach (var asteroid in result.Sector.Asteroids)
        {
            RequestAsteroidGeneration(asteroid, resolution: 6);
        }
    }
    
    /// <summary>
    /// Process a completed asteroid generation
    /// </summary>
    private void ProcessAsteroidResult(GenerationResult result)
    {
        if (result.VoxelBlocks == null)
            return;
            
        // Add blocks to chunk manager
        foreach (var block in result.VoxelBlocks)
        {
            _chunkManager.AddBlock(block);
        }
    }
}

/// <summary>
/// Type of generation task
/// </summary>
public enum TaskType
{
    Sector,
    Asteroid,
    Station
}

/// <summary>
/// Generation task data
/// </summary>
public class GenerationTask
{
    public TaskType Type { get; set; }
    public int SectorX { get; set; }
    public int SectorY { get; set; }
    public int SectorZ { get; set; }
    public AsteroidData? AsteroidData { get; set; }
    public int Resolution { get; set; } = 8;
}

/// <summary>
/// Generation result data
/// </summary>
public class GenerationResult
{
    public TaskType Type { get; set; }
    public GenerationTask? Task { get; set; }
    public GalaxySector? Sector { get; set; }
    public List<VoxelBlock>? VoxelBlocks { get; set; }
}
