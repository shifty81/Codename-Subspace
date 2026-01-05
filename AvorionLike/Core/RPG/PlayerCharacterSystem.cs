using System.Numerics;
using AvorionLike.Core.ECS;

namespace AvorionLike.Core.RPG;

/// <summary>
/// Player character that can walk around ship interiors
/// FPS-style movement and interaction
/// </summary>
public class PlayerCharacterComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    // Character state
    public Vector3 Position { get; set; }
    public Vector3 Velocity { get; set; }
    public float Yaw { get; set; } // Horizontal rotation
    public float Pitch { get; set; } // Vertical look
    
    // Movement parameters
    public float WalkSpeed { get; set; } = 3.0f; // m/s
    public float RunSpeed { get; set; } = 6.0f; // m/s
    public float CrouchSpeed { get; set; } = 1.5f; // m/s
    public float JumpForce { get; set; } = 5.0f;
    
    // Character properties
    public float Height { get; set; } = 1.8f; // meters
    public float CrouchHeight { get; set; } = 1.2f; // meters
    public float Radius { get; set; } = 0.4f; // collision radius
    
    // State flags
    public bool IsGrounded { get; set; } = false;
    public bool IsCrouching { get; set; } = false;
    public bool IsRunning { get; set; } = false;
    public bool IsInZeroG { get; set; } = false; // In zero gravity area
    
    // Ship association
    public Guid? CurrentShipId { get; set; } // Which ship the character is in
    public Guid? CurrentInteriorCellId { get; set; } // Which interior cell/room
    
    // Interaction
    public float InteractionRange { get; set; } = 2.0f; // meters
    public Guid? LookingAtObject { get; set; } // Object in crosshair
    
    /// <summary>
    /// Get current movement speed based on state
    /// </summary>
    public float GetCurrentSpeed()
    {
        if (IsCrouching) return CrouchSpeed;
        if (IsRunning) return RunSpeed;
        return WalkSpeed;
    }
    
    /// <summary>
    /// Get current height based on state
    /// </summary>
    public float GetCurrentHeight()
    {
        return IsCrouching ? CrouchHeight : Height;
    }
}

/// <summary>
/// Camera component for first-person view
/// </summary>
public class PlayerCameraComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    public Vector3 Position { get; set; }
    public Vector3 Forward { get; set; } = new Vector3(0, 0, -1);
    public Vector3 Up { get; set; } = new Vector3(0, 1, 0);
    
    public float FieldOfView { get; set; } = 90f;
    public float NearPlane { get; set; } = 0.1f;
    public float FarPlane { get; set; } = 1000f;
    
    // Camera effects
    public float HeadBob { get; set; } = 0f; // For walking animation
    public float HeadBobSpeed { get; set; } = 2f;
    public float HeadBobAmount { get; set; } = 0.05f;
}

/// <summary>
/// Interaction prompt for objects the player can interact with
/// </summary>
public class InteractableObject
{
    public Guid Id { get; set; } = Guid.NewGuid();
    public Vector3 Position { get; set; }
    public string Name { get; set; } = "Object";
    public string InteractionPrompt { get; set; } = "Press E to interact";
    public InteractionType Type { get; set; } = InteractionType.Generic;
    public Action<Guid>? OnInteract { get; set; } // Callback when interacted with
}

/// <summary>
/// Types of interactions
/// </summary>
public enum InteractionType
{
    Generic,
    Terminal,       // Computer terminal
    Door,           // Door open/close
    Storage,        // Storage container
    Seat,           // Sit down (pilot seat, passenger seat)
    Button,         // Button/switch
    Workbench,      // Crafting station
    MedicalStation, // Healing station
    TeleportPad,    // Teleporter
    Turret          // Manual turret control
}

/// <summary>
/// System for managing player character movement and interactions
/// </summary>
public class PlayerCharacterSystem
{
    private readonly Dictionary<Guid, InteractableObject> _interactables = new();
    
    /// <summary>
    /// Update player character movement
    /// </summary>
    public void UpdateMovement(PlayerCharacterComponent character, Vector3 moveDirection, float deltaTime)
    {
        if (character.IsInZeroG)
        {
            // Zero-G movement (6DOF)
            character.Velocity += moveDirection * character.GetCurrentSpeed() * deltaTime;
            // Apply drag
            character.Velocity *= 0.95f;
        }
        else
        {
            // Normal gravity movement
            var speed = character.GetCurrentSpeed();
            var horizontalMove = new Vector3(moveDirection.X, 0, moveDirection.Z);
            
            if (horizontalMove.LengthSquared() > 0)
            {
                horizontalMove = Vector3.Normalize(horizontalMove);
            }
            
            character.Velocity = new Vector3(
                horizontalMove.X * speed,
                character.Velocity.Y, // Keep vertical velocity
                horizontalMove.Z * speed
            );
            
            // Apply gravity if not grounded
            if (!character.IsGrounded)
            {
                character.Velocity += new Vector3(0, -9.81f * deltaTime, 0);
            }
        }
        
        // Update position
        character.Position += character.Velocity * deltaTime;
        
        // Ground check (simplified)
        if (character.Position.Y <= 0)
        {
            character.Position = new Vector3(character.Position.X, 0, character.Position.Z);
            character.Velocity = new Vector3(character.Velocity.X, 0, character.Velocity.Z);
            character.IsGrounded = true;
        }
        else
        {
            character.IsGrounded = false;
        }
    }
    
    /// <summary>
    /// Update camera based on character
    /// </summary>
    public void UpdateCamera(PlayerCharacterComponent character, PlayerCameraComponent camera, float deltaTime)
    {
        // Position camera at eye level
        var eyeHeight = character.GetCurrentHeight() * 0.9f;
        camera.Position = character.Position + new Vector3(0, eyeHeight, 0);
        
        // Calculate forward direction from yaw and pitch
        float yawRad = character.Yaw * (float)Math.PI / 180f;
        float pitchRad = character.Pitch * (float)Math.PI / 180f;
        
        camera.Forward = new Vector3(
            (float)(Math.Cos(pitchRad) * Math.Sin(yawRad)),
            (float)Math.Sin(pitchRad),
            (float)(Math.Cos(pitchRad) * Math.Cos(yawRad))
        );
        
        // Head bob effect when moving
        if (character.Velocity.LengthSquared() > 0.1f && character.IsGrounded)
        {
            camera.HeadBob += deltaTime * camera.HeadBobSpeed;
            float bobOffset = (float)Math.Sin(camera.HeadBob) * camera.HeadBobAmount;
            camera.Position += new Vector3(0, bobOffset, 0);
        }
    }
    
    /// <summary>
    /// Handle jump
    /// </summary>
    public void Jump(PlayerCharacterComponent character)
    {
        if (character.IsGrounded && !character.IsInZeroG)
        {
            character.Velocity += new Vector3(0, character.JumpForce, 0);
            character.IsGrounded = false;
        }
    }
    
    /// <summary>
    /// Register an interactable object
    /// </summary>
    public void RegisterInteractable(InteractableObject obj)
    {
        _interactables[obj.Id] = obj;
    }
    
    /// <summary>
    /// Unregister an interactable object
    /// </summary>
    public void UnregisterInteractable(Guid id)
    {
        _interactables.Remove(id);
    }
    
    /// <summary>
    /// Find interactable objects near the player
    /// </summary>
    public InteractableObject? FindInteractableInRange(PlayerCharacterComponent character, Vector3 lookDirection)
    {
        InteractableObject? closest = null;
        float closestDist = character.InteractionRange;
        
        foreach (var obj in _interactables.Values)
        {
            var toObj = obj.Position - character.Position;
            var dist = toObj.Length();
            
            if (dist > character.InteractionRange) continue;
            
            // Check if object is in front of player
            var dot = Vector3.Dot(Vector3.Normalize(toObj), lookDirection);
            if (dot < 0.7f) continue; // ~45 degree cone
            
            if (dist < closestDist)
            {
                closest = obj;
                closestDist = dist;
            }
        }
        
        return closest;
    }
    
    /// <summary>
    /// Interact with object
    /// </summary>
    public void Interact(PlayerCharacterComponent character, InteractableObject obj)
    {
        obj.OnInteract?.Invoke(character.EntityId);
    }
    
    /// <summary>
    /// Teleport character to position (for entering/exiting ships)
    /// </summary>
    public void TeleportTo(PlayerCharacterComponent character, Vector3 position, Guid? shipId = null)
    {
        character.Position = position;
        character.Velocity = Vector3.Zero;
        character.CurrentShipId = shipId;
    }
}
