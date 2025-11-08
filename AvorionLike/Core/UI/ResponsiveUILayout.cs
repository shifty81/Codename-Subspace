using System.Numerics;

namespace AvorionLike.Core.UI;

/// <summary>
/// Responsive UI layout system that adapts to different screen resolutions
/// Calculates positions and sizes based on screen dimensions and DPI scaling
/// </summary>
public class ResponsiveUILayout
{
    private float _screenWidth;
    private float _screenHeight;
    private float _scaleFactor;
    private ResolutionCategory _category;
    
    public enum ResolutionCategory
    {
        Small,      // < 1280x720
        Medium,     // 1280x720 to 1920x1080
        Large,      // 1920x1080 to 2560x1440
        ExtraLarge  // > 2560x1440
    }
    
    public float ScreenWidth => _screenWidth;
    public float ScreenHeight => _screenHeight;
    public float ScaleFactor => _scaleFactor;
    public ResolutionCategory Category => _category;
    
    public ResponsiveUILayout(float screenWidth, float screenHeight)
    {
        UpdateScreenSize(screenWidth, screenHeight);
    }
    
    /// <summary>
    /// Update screen dimensions and recalculate scaling
    /// </summary>
    public void UpdateScreenSize(float width, float height)
    {
        _screenWidth = width;
        _screenHeight = height;
        
        // Calculate scale factor based on a reference resolution of 1920x1080
        float widthScale = width / 1920f;
        float heightScale = height / 1080f;
        _scaleFactor = Math.Min(widthScale, heightScale);
        
        // Clamp scale factor to reasonable range
        _scaleFactor = Math.Clamp(_scaleFactor, 0.5f, 2.5f);
        
        // Determine resolution category
        if (width < 1280 || height < 720)
            _category = ResolutionCategory.Small;
        else if (width < 1920 || height < 1080)
            _category = ResolutionCategory.Medium;
        else if (width < 2560 || height < 1440)
            _category = ResolutionCategory.Large;
        else
            _category = ResolutionCategory.ExtraLarge;
    }
    
    /// <summary>
    /// Scale a value based on screen size
    /// </summary>
    public float Scale(float value)
    {
        return value * _scaleFactor;
    }
    
    /// <summary>
    /// Scale a size vector
    /// </summary>
    public Vector2 Scale(Vector2 size)
    {
        return new Vector2(size.X * _scaleFactor, size.Y * _scaleFactor);
    }
    
    /// <summary>
    /// Get absolute position from percentage (0-1 range)
    /// </summary>
    public Vector2 GetPositionFromPercent(float xPercent, float yPercent)
    {
        return new Vector2(
            _screenWidth * Math.Clamp(xPercent, 0f, 1f),
            _screenHeight * Math.Clamp(yPercent, 0f, 1f)
        );
    }
    
    /// <summary>
    /// Get size as percentage of screen
    /// </summary>
    public Vector2 GetSizeFromPercent(float widthPercent, float heightPercent)
    {
        return new Vector2(
            _screenWidth * Math.Clamp(widthPercent, 0f, 1f),
            _screenHeight * Math.Clamp(heightPercent, 0f, 1f)
        );
    }
    
    /// <summary>
    /// Get margin from edges based on percentage
    /// </summary>
    public float GetMargin(float percentOfSmallestDimension = 0.02f)
    {
        float smallestDim = Math.Min(_screenWidth, _screenHeight);
        return smallestDim * percentOfSmallestDimension;
    }
    
    /// <summary>
    /// Calculate panel size with responsive constraints
    /// </summary>
    public Vector2 GetPanelSize(float minWidth, float maxWidth, float minHeight, float maxHeight, 
                                float preferredWidthPercent = 0.15f, float preferredHeightPercent = 0.2f)
    {
        float width = _screenWidth * preferredWidthPercent;
        width = Math.Clamp(width, Scale(minWidth), Scale(maxWidth));
        
        float height = _screenHeight * preferredHeightPercent;
        height = Math.Clamp(height, Scale(minHeight), Scale(maxHeight));
        
        return new Vector2(width, height);
    }
    
    /// <summary>
    /// Get font scale for current resolution
    /// </summary>
    public float GetFontScale()
    {
        return _category switch
        {
            ResolutionCategory.Small => 0.85f,
            ResolutionCategory.Medium => 1.0f,
            ResolutionCategory.Large => 1.15f,
            ResolutionCategory.ExtraLarge => 1.3f,
            _ => 1.0f
        };
    }
    
    /// <summary>
    /// Get appropriate corner frame size
    /// </summary>
    public float GetCornerFrameSize()
    {
        return Scale(_category switch
        {
            ResolutionCategory.Small => 60f,
            ResolutionCategory.Medium => 80f,
            ResolutionCategory.Large => 100f,
            ResolutionCategory.ExtraLarge => 120f,
            _ => 80f
        });
    }
    
    /// <summary>
    /// Get HUD panel positions with safe margins
    /// </summary>
    public class HUDLayout
    {
        public Vector2 ShipStatusPosition { get; set; }
        public Vector2 ShipStatusSize { get; set; }
        public Vector2 VelocityPosition { get; set; }
        public Vector2 VelocitySize { get; set; }
        public Vector2 ResourcesPosition { get; set; }
        public Vector2 ResourcesSize { get; set; }
        public Vector2 RadarPosition { get; set; }
        public Vector2 RadarSize { get; set; }
        public Vector2 ControlsPosition { get; set; }
        public Vector2 ControlsSize { get; set; }
    }
    
    /// <summary>
    /// Calculate responsive HUD layout
    /// </summary>
    public HUDLayout CalculateHUDLayout()
    {
        float margin = GetMargin(0.015f);
        float cornerOffset = GetCornerFrameSize() + Scale(10f);
        
        var layout = new HUDLayout();
        
        // Ship status panel (top-left)
        layout.ShipStatusSize = GetPanelSize(200f, 350f, 140f, 200f, 0.18f, 0.18f);
        layout.ShipStatusPosition = new Vector2(margin, cornerOffset);
        
        // Velocity panel (top-right)
        layout.VelocitySize = GetPanelSize(180f, 300f, 100f, 140f, 0.16f, 0.12f);
        layout.VelocityPosition = new Vector2(
            _screenWidth - margin - layout.VelocitySize.X,
            cornerOffset
        );
        
        // Resources panel (top-right, below velocity)
        layout.ResourcesSize = GetPanelSize(180f, 300f, 100f, 150f, 0.16f, 0.12f);
        layout.ResourcesPosition = new Vector2(
            _screenWidth - margin - layout.ResourcesSize.X,
            layout.VelocityPosition.Y + layout.VelocitySize.Y + Scale(15f)
        );
        
        // Radar (bottom-left)
        float radarSize = Math.Min(
            Scale(220f),
            Math.Min(_screenWidth * 0.2f, _screenHeight * 0.25f)
        );
        layout.RadarSize = new Vector2(radarSize, radarSize);
        layout.RadarPosition = new Vector2(
            margin,
            _screenHeight - margin - radarSize
        );
        
        // Controls hint (bottom-center)
        layout.ControlsSize = GetPanelSize(300f, 600f, 70f, 100f, 0.35f, 0.08f);
        layout.ControlsPosition = new Vector2(
            (_screenWidth - layout.ControlsSize.X) / 2f,
            _screenHeight - margin - layout.ControlsSize.Y
        );
        
        return layout;
    }
    
    /// <summary>
    /// Get appropriate thickness for lines based on resolution
    /// </summary>
    public float GetLineThickness(float baseThickness = 2f)
    {
        return Math.Max(1f, baseThickness * _scaleFactor);
    }
    
    /// <summary>
    /// Get glow size based on resolution
    /// </summary>
    public float GetGlowSize(float baseSize = 10f)
    {
        return baseSize * _scaleFactor;
    }
}
