using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Physics;
using Silk.NET.Input;

namespace AvorionLike.Core.Input;

/// <summary>
/// Handles player input for controlling ships
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
    
    public Guid? ControlledShipId
    {
        get => _controlledShipId;
        set => _controlledShipId = value;
    }
    
    public PlayerControlSystem(EntityManager entityManager)
    {
        _entityManager = entityManager;
    }
    
    public void OnKeyDown(Key key)
    {
        _keysPressed.Add(key);
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
        
        // Thrust controls (WASD for lateral movement)
        Vector3 thrustDirection = Vector3.Zero;
        
        if (_keysPressed.Contains(Key.W))
            thrustDirection += new Vector3(1, 0, 0); // Forward
        if (_keysPressed.Contains(Key.S))
            thrustDirection += new Vector3(-1, 0, 0); // Backward
        if (_keysPressed.Contains(Key.A))
            thrustDirection += new Vector3(0, 1, 0); // Left
        if (_keysPressed.Contains(Key.D))
            thrustDirection += new Vector3(0, -1, 0); // Right
        if (_keysPressed.Contains(Key.Space))
            thrustDirection += new Vector3(0, 0, 1); // Up
        if (_keysPressed.Contains(Key.ShiftLeft))
            thrustDirection += new Vector3(0, 0, -1); // Down
        
        // Apply thrust
        if (thrustDirection.Length() > 0)
        {
            thrustDirection = Vector3.Normalize(thrustDirection);
            float thrustForce = physics.MaxThrust * _thrustMultiplier;
            physics.AddForce(thrustDirection * thrustForce);
        }
        
        // Rotation controls (Q/E for roll, Arrow keys for pitch/yaw)
        Vector3 torque = Vector3.Zero;
        
        if (_keysPressed.Contains(Key.Up))
            torque += new Vector3(1, 0, 0); // Pitch up
        if (_keysPressed.Contains(Key.Down))
            torque += new Vector3(-1, 0, 0); // Pitch down
        if (_keysPressed.Contains(Key.Left))
            torque += new Vector3(0, 1, 0); // Yaw left
        if (_keysPressed.Contains(Key.Right))
            torque += new Vector3(0, -1, 0); // Yaw right
        if (_keysPressed.Contains(Key.Q))
            torque += new Vector3(0, 0, 1); // Roll left
        if (_keysPressed.Contains(Key.E))
            torque += new Vector3(0, 0, -1); // Roll right
        
        // Apply torque
        if (torque.Length() > 0)
        {
            torque = Vector3.Normalize(torque);
            float torqueForce = physics.MaxTorque * _rotationMultiplier;
            physics.AddTorque(torque * torqueForce);
        }
        
        // Emergency brake
        if (_keysPressed.Contains(Key.X))
        {
            // Apply counter-force to slow down
            if (physics.Velocity.Length() > 0.1f)
            {
                Vector3 brakeForce = -Vector3.Normalize(physics.Velocity) * physics.MaxThrust * 2f;
                physics.AddForce(brakeForce);
            }
            
            // Apply counter-torque to stop rotation
            if (physics.AngularVelocity.Length() > 0.01f)
            {
                Vector3 brakeTorque = -Vector3.Normalize(physics.AngularVelocity) * physics.MaxTorque * 2f;
                physics.AddTorque(brakeTorque);
            }
        }
    }
}
