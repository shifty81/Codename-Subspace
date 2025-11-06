using System.Numerics;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.SolarSystem;

/// <summary>
/// Manages hyperspace jump animations during system loading
/// </summary>
public class HyperspaceAnimation
{
    private readonly Logger _logger;
    private AnimationState _state = AnimationState.Idle;
    private float _animationTime = 0f;
    private string _currentTip = "";
    private float _tipDisplayTime = 0f;
    private const float TIP_ROTATION_INTERVAL = 5f; // Change tip every 5 seconds

    public AnimationState State => _state;
    public string CurrentTip => _currentTip;
    public float Progress => _animationTime / GetTotalAnimationDuration();

    public HyperspaceAnimation()
    {
        _logger = Logger.Instance;
    }

    /// <summary>
    /// Start hyperspace jump animation
    /// </summary>
    public void StartJump(string destinationSystem)
    {
        _state = AnimationState.JumpInitiation;
        _animationTime = 0f;
        _tipDisplayTime = 0f;
        _currentTip = LoadingTipManager.Instance.GetRandomTip();
        
        _logger.Info("HyperspaceAnimation", $"Starting hyperspace jump to {destinationSystem}");
    }

    /// <summary>
    /// Update animation state
    /// </summary>
    public void Update(float deltaTime)
    {
        if (_state == AnimationState.Idle || _state == AnimationState.Complete)
            return;

        _animationTime += deltaTime;
        _tipDisplayTime += deltaTime;

        // Rotate tips during longer loads
        if (_tipDisplayTime >= TIP_ROTATION_INTERVAL)
        {
            _currentTip = LoadingTipManager.Instance.GetRandomTip();
            _tipDisplayTime = 0f;
        }

        // State machine for animation phases
        switch (_state)
        {
            case AnimationState.JumpInitiation:
                if (_animationTime >= 1.5f) // 1.5 second initiation
                {
                    _state = AnimationState.Tunnel;
                    _animationTime = 0f;
                }
                break;

            case AnimationState.Tunnel:
                // Tunnel phase lasts as long as loading takes
                // External system will call FinishJump() when loading is complete
                break;

            case AnimationState.Emergence:
                if (_animationTime >= 1.0f) // 1 second emergence
                {
                    _state = AnimationState.Complete;
                }
                break;
        }
    }

    /// <summary>
    /// Signal that loading is complete and start emergence animation
    /// </summary>
    public void FinishJump()
    {
        if (_state == AnimationState.Tunnel)
        {
            _state = AnimationState.Emergence;
            _animationTime = 0f;
            _logger.Info("HyperspaceAnimation", "Hyperspace jump complete, emerging into system");
        }
    }

    /// <summary>
    /// Reset animation to idle state
    /// </summary>
    public void Reset()
    {
        _state = AnimationState.Idle;
        _animationTime = 0f;
        _tipDisplayTime = 0f;
        _currentTip = "";
    }

    /// <summary>
    /// Get total duration of animation (excluding tunnel time)
    /// </summary>
    private float GetTotalAnimationDuration()
    {
        return 2.5f; // 1.5s initiation + 1.0s emergence
    }

    /// <summary>
    /// Get tunnel effect parameters for rendering
    /// </summary>
    public TunnelEffectParameters GetTunnelParameters()
    {
        return _state switch
        {
            AnimationState.JumpInitiation => new TunnelEffectParameters
            {
                Intensity = _animationTime / 1.5f, // Fade in
                Speed = _animationTime * 2f,
                Distortion = _animationTime * 0.5f,
                BlueShift = true
            },
            AnimationState.Tunnel => new TunnelEffectParameters
            {
                Intensity = 1.0f,
                Speed = 5.0f + (float)Math.Sin(_animationTime * 2) * 0.5f, // Pulsing speed
                Distortion = 0.8f + (float)Math.Sin(_animationTime * 3) * 0.2f,
                BlueShift = true
            },
            AnimationState.Emergence => new TunnelEffectParameters
            {
                Intensity = 1.0f - (_animationTime / 1.0f), // Fade out
                Speed = 5.0f - (_animationTime * 3f),
                Distortion = 0.8f - (_animationTime * 0.8f),
                BlueShift = false // Red shift on exit
            },
            _ => new TunnelEffectParameters()
        };
    }

    /// <summary>
    /// Check if animation is complete
    /// </summary>
    public bool IsComplete()
    {
        return _state == AnimationState.Complete;
    }
}

/// <summary>
/// Animation state for hyperspace jump
/// </summary>
public enum AnimationState
{
    Idle,               // Not animating
    JumpInitiation,     // Charging up, stars streaking
    Tunnel,             // In hyperspace tunnel (during loading)
    Emergence,          // Exiting hyperspace
    Complete            // Animation finished
}

/// <summary>
/// Parameters for rendering hyperspace tunnel effect
/// </summary>
public struct TunnelEffectParameters
{
    public float Intensity { get; set; }      // 0-1, overall effect strength
    public float Speed { get; set; }          // Tunnel movement speed
    public float Distortion { get; set; }     // Space-time distortion amount
    public bool BlueShift { get; set; }       // Blue shift (entering) or red shift (exiting)
    
    public Vector3 GetColorTint()
    {
        if (BlueShift)
        {
            // Blue-white hyperspace
            return new Vector3(0.6f, 0.8f, 1.0f);
        }
        else
        {
            // Reddish emergence
            return new Vector3(1.0f, 0.7f, 0.5f);
        }
    }
}
