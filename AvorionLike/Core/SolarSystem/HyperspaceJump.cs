using AvorionLike.Core.Logging;
using System.Diagnostics;

namespace AvorionLike.Core.SolarSystem;

/// <summary>
/// Manages hyperspace jumps between solar systems with loading and animation
/// </summary>
public class HyperspaceJump
{
    private readonly Logger _logger;
    private readonly HyperspaceAnimation _animation;
    private JumpState _jumpState = JumpState.Ready;
    private string _destinationSystemId = "";
    private Stopwatch _loadingTimer = new();

    public JumpState State => _jumpState;
    public HyperspaceAnimation Animation => _animation;
    public bool IsJumping => _jumpState != JumpState.Ready && _jumpState != JumpState.Complete;

    public HyperspaceJump()
    {
        _logger = Logger.Instance;
        _animation = new HyperspaceAnimation();
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
        
        // Start animation
        _animation.StartJump(destinationSystemId);
        
        // Start loading in background
        _loadingTimer.Restart();
        Task.Run(() => LoadSystemAsync(destinationSystemId, loadSystemCallback));
        
        return true;
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
