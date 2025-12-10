using System.Numerics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// 3D Camera for viewing the game world
/// Supports both position and rotation for free-look camera
/// </summary>
public class Camera
{
    public Vector3 Position { get; set; }
    public Vector3 Front { get; private set; }
    public Vector3 Up { get; private set; }
    public Vector3 Right { get; private set; }
    
    public float Yaw { get; private set; } = -90.0f;
    public float Pitch { get; private set; } = 0.0f;
    
    public float MovementSpeed { get; set; } = 50.0f;
    public float MouseSensitivity { get; set; } = 0.1f;
    public float Fov { get; set; } = 45.0f;
    
    // Chase camera properties
    private Vector3 _targetPosition;
    private float _chaseDistance = 50.0f;
    private float _chaseHeight = 30.0f;
    private float _chaseSmoothness = 5.0f;

    public Camera(Vector3 position)
    {
        Position = position;
        _targetPosition = position;
        Up = Vector3.UnitY;
        UpdateCameraVectors();
    }
    
    /// <summary>
    /// Updates camera to smoothly follow a target (chase camera)
    /// </summary>
    public void FollowTarget(Vector3 targetPosition, Vector3 targetVelocity, float deltaTime)
    {
        // Calculate desired camera position based on target velocity
        Vector3 targetDirection = targetVelocity.Length() > 0.1f 
            ? Vector3.Normalize(targetVelocity) 
            : Front;
        
        // Position camera behind and above the target
        Vector3 desiredPosition = targetPosition 
            - targetDirection * _chaseDistance 
            + Vector3.UnitY * _chaseHeight;
        
        // Smoothly interpolate to desired position
        Position = Vector3.Lerp(Position, desiredPosition, deltaTime * _chaseSmoothness);
        
        // Look at the target
        Vector3 direction = Vector3.Normalize(targetPosition - Position);
        
        // Update yaw and pitch based on look direction
        Yaw = MathF.Atan2(direction.Z, direction.X) * (180.0f / MathF.PI);
        Pitch = MathF.Asin(direction.Y) * (180.0f / MathF.PI);
        
        UpdateCameraVectors();
    }
    
    /// <summary>
    /// Sets chase camera parameters
    /// </summary>
    public void SetChaseParameters(float distance, float height, float smoothness)
    {
        _chaseDistance = distance;
        _chaseHeight = height;
        _chaseSmoothness = smoothness;
    }

    public Matrix4x4 GetViewMatrix()
    {
        return Matrix4x4.CreateLookAt(Position, Position + Front, Up);
    }

    public Matrix4x4 GetProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 50000.0f)
    {
        return Matrix4x4.CreatePerspectiveFieldOfView(
            Fov * (MathF.PI / 180.0f),
            aspectRatio,
            nearPlane,
            farPlane
        );
    }

    public void ProcessKeyboard(CameraMovement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        
        switch (direction)
        {
            case CameraMovement.Forward:
                Position += Front * velocity;
                break;
            case CameraMovement.Backward:
                Position -= Front * velocity;
                break;
            case CameraMovement.Left:
                Position -= Right * velocity;
                break;
            case CameraMovement.Right:
                Position += Right * velocity;
                break;
            case CameraMovement.Up:
                Position += Up * velocity;
                break;
            case CameraMovement.Down:
                Position -= Up * velocity;
                break;
        }
    }

    public void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true)
    {
        xOffset *= MouseSensitivity;
        yOffset *= MouseSensitivity;

        Yaw += xOffset;
        Pitch += yOffset;

        if (constrainPitch)
        {
            Pitch = Math.Clamp(Pitch, -89.0f, 89.0f);
        }

        UpdateCameraVectors();
    }

    private void UpdateCameraVectors()
    {
        Vector3 front;
        front.X = MathF.Cos(Yaw * (MathF.PI / 180.0f)) * MathF.Cos(Pitch * (MathF.PI / 180.0f));
        front.Y = MathF.Sin(Pitch * (MathF.PI / 180.0f));
        front.Z = MathF.Sin(Yaw * (MathF.PI / 180.0f)) * MathF.Cos(Pitch * (MathF.PI / 180.0f));
        Front = Vector3.Normalize(front);
        
        Right = Vector3.Normalize(Vector3.Cross(Front, Vector3.UnitY));
        Up = Vector3.Normalize(Vector3.Cross(Right, Front));
    }
}

public enum CameraMovement
{
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
}
