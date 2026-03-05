using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Physics;
using Silk.NET.Input;

namespace AvorionLike.Core.Input;

/// <summary>
/// Handles player input for controlling ships
/// Uses Avorion-style thruster-based movement with directional thrust,
/// inertial dampening, and 6DOF control
/// </summary>
public class PlayerControlSystem
{
    private readonly EntityManager _entityManager;
    private Guid? _controlledShipId;
    
    // Input state
    private readonly HashSet<Key> _keysPressed = new();
    
    // Control sensitivity
    private float _thrustMultiplier = 1.0f;
    private float _rotationMultiplier = 1.0f;
    
    // Thruster configuration (Avorion-style directional thrust)
    // Forward thrusters are strongest, lateral/reverse are weaker
    private float _forwardThrustRatio = 1.0f;    // Main engines - full power
    private float _reverseThrustRatio = 0.4f;     // Reverse thrusters - weaker
    private float _lateralThrustRatio = 0.5f;     // Lateral (strafing) thrusters
    private float _verticalThrustRatio = 0.5f;    // Vertical thrusters
    
    // Inertial dampening - automatically counters unwanted drift
    private bool _inertialDampeningEnabled = true;
    private float _dampeningStrength = 0.3f;       // How strongly dampening counters drift (0-1)
    
    // Boost (afterburner) state
    private bool _boostActive = false;
    private float _boostMultiplier = 2.5f;         // Boost force multiplier
    
    public Guid? ControlledShipId
    {
        get => _controlledShipId;
        set => _controlledShipId = value;
    }
    
    public bool InertialDampeningEnabled
    {
        get => _inertialDampeningEnabled;
        set => _inertialDampeningEnabled = value;
    }
    
    public bool BoostActive => _boostActive;
    
    public PlayerControlSystem(EntityManager entityManager)
    {
        _entityManager = entityManager;
    }
    
    public void OnKeyDown(Key key)
    {
        _keysPressed.Add(key);
        
        // Toggle inertial dampening with V
        if (key == Key.V)
            _inertialDampeningEnabled = !_inertialDampeningEnabled;
    }
    
    public void OnKeyUp(Key key)
    {
        _keysPressed.Remove(key);
    }
    
    public void Update(float deltaTime)
    {
        if (!_controlledShipId.HasValue) return;
        
        var physics = _entityManager.GetComponent<PhysicsComponent>(_controlledShipId.Value);
        if (physics == null) return;
        
        // Build ship-local rotation matrix from current rotation (Euler angles)
        var rotMatrix = Matrix4x4.CreateRotationX(physics.Rotation.X)
                      * Matrix4x4.CreateRotationY(physics.Rotation.Y)
                      * Matrix4x4.CreateRotationZ(physics.Rotation.Z);
        
        // Extract ship-local axes from rotation matrix
        Vector3 shipForward = Vector3.Normalize(new Vector3(rotMatrix.M11, rotMatrix.M12, rotMatrix.M13));
        Vector3 shipUp = Vector3.Normalize(new Vector3(rotMatrix.M31, rotMatrix.M32, rotMatrix.M33));
        Vector3 shipRight = Vector3.Normalize(new Vector3(rotMatrix.M21, rotMatrix.M22, rotMatrix.M23));
        
        // Check boost state (Tab key for afterburner)
        _boostActive = _keysPressed.Contains(Key.Tab);
        
        // --- Directional thruster-based movement ---
        // Each direction has its own thrust ratio (forward engines > lateral thrusters)
        Vector3 thrustForce = Vector3.Zero;
        bool anyThrustInput = false;
        
        if (_keysPressed.Contains(Key.W))
        {
            thrustForce += shipForward * _forwardThrustRatio;
            anyThrustInput = true;
        }
        if (_keysPressed.Contains(Key.S))
        {
            thrustForce -= shipForward * _reverseThrustRatio;
            anyThrustInput = true;
        }
        if (_keysPressed.Contains(Key.A))
        {
            thrustForce += shipRight * _lateralThrustRatio;
            anyThrustInput = true;
        }
        if (_keysPressed.Contains(Key.D))
        {
            thrustForce -= shipRight * _lateralThrustRatio;
            anyThrustInput = true;
        }
        if (_keysPressed.Contains(Key.Space))
        {
            thrustForce += shipUp * _verticalThrustRatio;
            anyThrustInput = true;
        }
        if (_keysPressed.Contains(Key.ShiftLeft))
        {
            thrustForce -= shipUp * _verticalThrustRatio;
            anyThrustInput = true;
        }
        
        // Apply thrust with boost modifier
        if (thrustForce.Length() > 0)
        {
            float baseThrust = physics.MaxThrust * _thrustMultiplier;
            float boostFactor = _boostActive ? _boostMultiplier : 1.0f;
            physics.AddForce(thrustForce * baseThrust * boostFactor);
        }
        
        // --- Inertial dampening ---
        // When not thrusting, automatically counter velocity to reduce drift
        // This mimics Avorion's flight feel where ships slow down without input
        if (_inertialDampeningEnabled && !anyThrustInput)
        {
            if (physics.Velocity.Length() > 0.5f)
            {
                // Apply counter-force proportional to velocity, scaled by dampening strength
                Vector3 dampeningForce = -physics.Velocity * physics.Mass * _dampeningStrength;
                
                // Clamp dampening force to not exceed max thrust
                float maxDampening = physics.MaxThrust * 0.5f;
                if (dampeningForce.Length() > maxDampening)
                {
                    dampeningForce = Vector3.Normalize(dampeningForce) * maxDampening;
                }
                
                physics.AddForce(dampeningForce);
            }
        }
        
        // --- Rotation controls (Q/E for roll, Arrow keys for pitch/yaw) ---
        Vector3 torque = Vector3.Zero;
        bool anyRotationInput = false;
        
        if (_keysPressed.Contains(Key.Up))
        {
            torque += new Vector3(1, 0, 0); // Pitch up
            anyRotationInput = true;
        }
        if (_keysPressed.Contains(Key.Down))
        {
            torque += new Vector3(-1, 0, 0); // Pitch down
            anyRotationInput = true;
        }
        if (_keysPressed.Contains(Key.Left))
        {
            torque += new Vector3(0, 1, 0); // Yaw left
            anyRotationInput = true;
        }
        if (_keysPressed.Contains(Key.Right))
        {
            torque += new Vector3(0, -1, 0); // Yaw right
            anyRotationInput = true;
        }
        if (_keysPressed.Contains(Key.Q))
        {
            torque += new Vector3(0, 0, 1); // Roll left
            anyRotationInput = true;
        }
        if (_keysPressed.Contains(Key.E))
        {
            torque += new Vector3(0, 0, -1); // Roll right
            anyRotationInput = true;
        }
        
        // Apply torque
        if (torque.Length() > 0)
        {
            torque = Vector3.Normalize(torque);
            float torqueForce = physics.MaxTorque * _rotationMultiplier;
            physics.AddTorque(torque * torqueForce);
        }
        
        // Rotational dampening - stop spinning when not actively rotating
        if (_inertialDampeningEnabled && !anyRotationInput)
        {
            if (physics.AngularVelocity.Length() > 0.01f)
            {
                Vector3 rotDampening = -physics.AngularVelocity * physics.MomentOfInertia * _dampeningStrength;
                float maxRotDamp = physics.MaxTorque * 0.5f;
                if (rotDampening.Length() > maxRotDamp)
                {
                    rotDampening = Vector3.Normalize(rotDampening) * maxRotDamp;
                }
                physics.AddTorque(rotDampening);
            }
        }
        
        // Emergency brake (X) — full counter-thrust regardless of dampening
        if (_keysPressed.Contains(Key.X))
        {
            if (physics.Velocity.Length() > 0.1f)
            {
                Vector3 brakeForce = -Vector3.Normalize(physics.Velocity) * physics.MaxThrust * 2f;
                physics.AddForce(brakeForce);
            }
            
            if (physics.AngularVelocity.Length() > 0.01f)
            {
                Vector3 brakeTorque = -Vector3.Normalize(physics.AngularVelocity) * physics.MaxTorque * 2f;
                physics.AddTorque(brakeTorque);
            }
        }
    }
}
