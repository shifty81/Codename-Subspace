using AvorionLike.Core.Logging;
using AvorionLike.Core.Procedural;
using System.Diagnostics;
using System.Numerics;

namespace AvorionLike.Core.SolarSystem;

/// <summary>
/// Manages hyperspace jumps between solar systems with loading and animation
/// Integrates with galaxy network for stargate-based jumps
/// </summary>
public class HyperspaceJump
{
    private readonly Logger _logger;
    private readonly HyperspaceAnimation _animation;
    private JumpState _jumpState = JumpState.Ready;
    private string _destinationSystemId = "";
    private string _currentSystemId = "";
    private Vector3? _exitGatePosition;
    private Stopwatch _loadingTimer = new();
    private GalaxyNetwork? _galaxyNetwork;

    public JumpState State => _jumpState;
    public HyperspaceAnimation Animation => _animation;
    public bool IsJumping => _jumpState != JumpState.Ready && _jumpState != JumpState.Complete;
    public string CurrentSystemId => _currentSystemId;
    public Vector3? ExitGatePosition => _exitGatePosition;

    public HyperspaceJump()
    {
        _logger = Logger.Instance;
        _animation = new HyperspaceAnimation();
    }
    
    /// <summary>
    /// Set the galaxy network for stargate-based jumps
    /// </summary>
    public void SetGalaxyNetwork(GalaxyNetwork galaxyNetwork)
    {
        _galaxyNetwork = galaxyNetwork;
    }
    
    /// <summary>
    /// Set current system ID
    /// </summary>
    public void SetCurrentSystem(string systemId)
    {
        _currentSystemId = systemId;
    }

    /// <summary>
    /// Initiate a hyperspace jump to a destination system
    /// </summary>
    public bool InitiateJump(string destinationSystemId, Action<string> loadSystemCallback)
    {
        if (_jumpState != JumpState.Ready)
        {
            _logger.Warning("HyperspaceJump", "Cannot initiate jump - already jumping");
            return false;
        }

        _destinationSystemId = destinationSystemId;
        _jumpState = JumpState.Initiating;
        
        _logger.Info("HyperspaceJump", $"Initiating hyperspace jump to system: {destinationSystemId}");
        
        // Determine exit gate position if using galaxy network
        DetermineExitGatePosition();
        
        // Start animation
        _animation.StartJump(destinationSystemId);
        
        // Start loading in background
        _loadingTimer.Restart();
        Task.Run(() => LoadSystemAsync(destinationSystemId, loadSystemCallback));
        
        return true;
    }
    
    /// <summary>
    /// Initiate a jump through a specific stargate (no cost)
    /// </summary>
    public bool InitiateGateJump(string destinationSystemId, string? destinationGateId, 
        Action<string> loadSystemCallback)
    {
        if (_jumpState != JumpState.Ready)
        {
            _logger.Warning("HyperspaceJump", "Cannot initiate jump - already jumping");
            return false;
        }
        
        // Verify connection exists in galaxy network
        if (_galaxyNetwork != null && !string.IsNullOrEmpty(_currentSystemId))
        {
            var path = _galaxyNetwork.FindPath(_currentSystemId, destinationSystemId);
            if (path == null || path.Count < 2)
            {
                _logger.Warning("HyperspaceJump", $"No route found from {_currentSystemId} to {destinationSystemId}");
                return false;
            }
        }
        
        _destinationSystemId = destinationSystemId;
        _jumpState = JumpState.Initiating;
        
        _logger.Info("HyperspaceJump", $"Initiating gate jump to system: {destinationSystemId}");
        
        // Get exit gate position for the destination
        if (_galaxyNetwork != null)
        {
            var destCoords = ParseSystemCoordinates(destinationSystemId);
            var destSystem = _galaxyNetwork.GetOrGenerateSystem(destCoords);
            
            if (destinationGateId != null)
            {
                var exitGate = destSystem.Stargates.FirstOrDefault(g => g.GateId == destinationGateId);
                _exitGatePosition = exitGate?.Position;
            }
            else
            {
                // Use first available gate
                _exitGatePosition = destSystem.Stargates.FirstOrDefault()?.Position;
            }
        }
        
        // Start animation
        _animation.StartJump(destinationSystemId);
        
        // Start loading in background
        _loadingTimer.Restart();
        Task.Run(() => LoadSystemAsync(destinationSystemId, loadSystemCallback));
        
        return true;
    }
    
    /// <summary>
    /// Determine exit gate position in destination system
    /// </summary>
    private void DetermineExitGatePosition()
    {
        if (_galaxyNetwork == null || string.IsNullOrEmpty(_destinationSystemId))
        {
            _exitGatePosition = null;
            return;
        }
        
        try
        {
            var destCoords = ParseSystemCoordinates(_destinationSystemId);
            var destSystem = _galaxyNetwork.GetOrGenerateSystem(destCoords);
            
            // Use first available gate as exit point
            _exitGatePosition = destSystem.Stargates.FirstOrDefault()?.Position;
        }
        catch
        {
            _exitGatePosition = null;
        }
    }
    
    /// <summary>
    /// Parse system ID to coordinates
    /// </summary>
    private Vector3Int ParseSystemCoordinates(string systemId)
    {
        // Expected format: "System-X-Y-Z"
        var parts = systemId.Split('-');
        if (parts.Length >= 4 && 
            int.TryParse(parts[1], out int x) &&
            int.TryParse(parts[2], out int y) &&
            int.TryParse(parts[3], out int z))
        {
            return new Vector3Int(x, y, z);
        }
        return Vector3Int.Zero;
    }

    /// <summary>
    /// Load the destination system asynchronously
    /// </summary>
    private async Task LoadSystemAsync(string systemId, Action<string> loadSystemCallback)
    {
        try
        {
            _logger.Info("HyperspaceJump", $"Loading system: {systemId}");
            _jumpState = JumpState.Loading;
            
            // Call the system loader callback
            await Task.Run(() => loadSystemCallback(systemId));
            
            // Minimum loading time for animation smoothness (1 second)
            var elapsed = _loadingTimer.Elapsed.TotalSeconds;
            if (elapsed < 1.0)
            {
                await Task.Delay((int)((1.0 - elapsed) * 1000));
            }
            
            // Signal animation to finish
            _animation.FinishJump();
            _jumpState = JumpState.Emerging;
            
            _logger.Info("HyperspaceJump", $"System {systemId} loaded in {_loadingTimer.Elapsed.TotalSeconds:F2}s");
        }
        catch (Exception ex)
        {
            _logger.Error("HyperspaceJump", $"Failed to load system {systemId}: {ex.Message}");
            _jumpState = JumpState.Failed;
        }
    }

    /// <summary>
    /// Update jump state and animation
    /// </summary>
    public void Update(float deltaTime)
    {
        _animation.Update(deltaTime);
        
        // Check if emergence animation is complete
        if (_jumpState == JumpState.Emerging && _animation.IsComplete())
        {
            _jumpState = JumpState.Complete;
            _logger.Info("HyperspaceJump", "Hyperspace jump complete");
        }
    }

    /// <summary>
    /// Reset jump state after completion
    /// </summary>
    public void Reset()
    {
        _jumpState = JumpState.Ready;
        _animation.Reset();
        _destinationSystemId = "";
    }

    /// <summary>
    /// Get loading progress (0-1)
    /// </summary>
    public float GetLoadingProgress()
    {
        return _animation.Progress;
    }

    /// <summary>
    /// Cancel jump (only during initiation)
    /// </summary>
    public bool CancelJump()
    {
        if (_jumpState == JumpState.Initiating)
        {
            _logger.Info("HyperspaceJump", "Jump cancelled");
            Reset();
            return true;
        }
        return false;
    }
}

/// <summary>
/// State of hyperspace jump
/// </summary>
public enum JumpState
{
    Ready,          // Ready to jump
    Initiating,     // Starting jump sequence
    Loading,        // Loading destination system
    Emerging,       // Exiting hyperspace
    Complete,       // Jump finished
    Failed          // Jump failed
}
