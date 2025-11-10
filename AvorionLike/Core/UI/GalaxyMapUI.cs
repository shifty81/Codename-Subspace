using ImGuiNET;
using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Navigation;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.Resources;

namespace AvorionLike.Core.UI;

/// <summary>
/// Interactive galaxy map UI for navigation, exploration, and sector management
/// Shows current location, nearby sectors, points of interest, and allows waypoint selection
/// </summary>
public class GalaxyMapUI
{
    private readonly EntityManager _entityManager;
    private readonly NavigationSystem _navigationSystem;
    private readonly GalaxyGenerator _galaxyGenerator;
    
    private bool _isOpen = false;
    private Guid? _playerShipId = null;
    
    // View state
    private Vector2 _viewOffset = Vector2.Zero;
    private float _zoomLevel = 1.0f;
    private int _viewSliceZ = 0; // Which Z-slice of the galaxy to display
    
    // Selection state
    private SectorCoordinate? _selectedSector = null;
    private SectorCoordinate? _hoveredSector = null;
    
    // Filter state
    private bool _showStations = true;
    private bool _showAsteroids = true;
    private bool _showShips = true;
    private bool _showJumpRange = true;
    
    // Cached sector data
    private Dictionary<string, GalaxySector> _sectorCache = new();
    private const int CACHE_RADIUS = 10; // Cache sectors in a 10-sector radius
    
    public bool IsOpen
    {
        get => _isOpen;
        set => _isOpen = value;
    }
    
    public Guid? PlayerShipId
    {
        get => _playerShipId;
        set => _playerShipId = value;
    }
    
    public GalaxyMapUI(EntityManager entityManager, NavigationSystem navigationSystem, int galaxySeed = 0)
    {
        _entityManager = entityManager;
        _navigationSystem = navigationSystem;
        _galaxyGenerator = new GalaxyGenerator(galaxySeed);
    }
    
    public void HandleInput()
    {
        var io = ImGui.GetIO();
        
        // Toggle map with M key
        if (ImGui.IsKeyPressed(ImGuiKey.M) && !io.WantCaptureKeyboard)
        {
            _isOpen = !_isOpen;
        }
    }
    
    public void Render()
    {
        if (!_isOpen) return;
        
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), ImGuiCond.FirstUseEver, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(1000, 700), ImGuiCond.FirstUseEver);
        
        if (ImGui.Begin("Galaxy Map", ref _isOpen, ImGuiWindowFlags.NoCollapse))
        {
            RenderHeader();
            ImGui.Separator();
            
            // Split into map view and info panel
            ImGui.BeginChild("MapView", new Vector2(ImGui.GetContentRegionAvail().X * 0.7f, 0), true);
            RenderMapView();
            ImGui.EndChild();
            
            ImGui.SameLine();
            
            ImGui.BeginChild("InfoPanel", new Vector2(0, 0), true);
            RenderInfoPanel();
            ImGui.EndChild();
        }
        ImGui.End();
    }
    
    private void RenderHeader()
    {
        ImGui.TextColored(new Vector4(0.3f, 0.8f, 1.0f, 1.0f), "GALAXY MAP");
        
        if (_playerShipId.HasValue)
        {
            var location = _entityManager.GetComponent<SectorLocationComponent>(_playerShipId.Value);
            if (location != null)
            {
                ImGui.SameLine();
                ImGui.Text($"| Current Sector: ({location.CurrentSector.X}, {location.CurrentSector.Y}, {location.CurrentSector.Z})");
                
                var hyperdrive = _entityManager.GetComponent<HyperdriveComponent>(_playerShipId.Value);
                if (hyperdrive != null)
                {
                    ImGui.SameLine();
                    ImGui.Text($"| Jump Range: {hyperdrive.JumpRange:F1}");
                    
                    if (hyperdrive.IsCharging)
                    {
                        ImGui.SameLine();
                        float chargePercent = (hyperdrive.CurrentCharge / hyperdrive.ChargeTime) * 100f;
                        ImGui.TextColored(new Vector4(1.0f, 0.8f, 0.3f, 1.0f), $"| CHARGING: {chargePercent:F0}%%");
                    }
                    else if (!hyperdrive.CanJump)
                    {
                        float cooldownPercent = (hyperdrive.TimeSinceLastJump / hyperdrive.JumpCooldown) * 100f;
                        ImGui.SameLine();
                        ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1.0f), $"| Cooldown: {cooldownPercent:F0}%%");
                    }
                }
            }
        }
    }
    
    private void RenderMapView()
    {
        // Controls
        ImGui.Text("Controls: Drag to pan | Scroll to zoom | Click sector to select | Right-click to jump");
        
        // Z-slice selector
        ImGui.Text($"Z-Slice: {_viewSliceZ}");
        ImGui.SameLine();
        if (ImGui.Button("-##z")) _viewSliceZ--;
        ImGui.SameLine();
        if (ImGui.Button("+##z")) _viewSliceZ++;
        ImGui.SameLine();
        if (ImGui.Button("Reset View"))
        {
            _viewOffset = Vector2.Zero;
            _zoomLevel = 1.0f;
            if (_playerShipId.HasValue)
            {
                var location = _entityManager.GetComponent<SectorLocationComponent>(_playerShipId.Value);
                if (location != null)
                {
                    _viewSliceZ = location.CurrentSector.Z;
                }
            }
        }
        
        // Filters
        ImGui.Checkbox("Stations", ref _showStations);
        ImGui.SameLine();
        ImGui.Checkbox("Asteroids", ref _showAsteroids);
        ImGui.SameLine();
        ImGui.Checkbox("Ships", ref _showShips);
        ImGui.SameLine();
        ImGui.Checkbox("Jump Range", ref _showJumpRange);
        
        ImGui.Separator();
        
        // Map canvas
        var canvasPos = ImGui.GetCursorScreenPos();
        var canvasSize = ImGui.GetContentRegionAvail();
        var drawList = ImGui.GetWindowDrawList();
        
        // Background
        drawList.AddRectFilled(canvasPos, new Vector2(canvasPos.X + canvasSize.X, canvasPos.Y + canvasSize.Y), 
            ImGui.GetColorU32(new Vector4(0.05f, 0.05f, 0.1f, 1.0f)));
        
        // Handle input
        HandleMapInput(canvasPos, canvasSize);
        
        // Draw sectors
        DrawSectors(drawList, canvasPos, canvasSize);
        
        // Draw grid
        DrawGrid(drawList, canvasPos, canvasSize);
        
        ImGui.Dummy(canvasSize);
    }
    
    private void HandleMapInput(Vector2 canvasPos, Vector2 canvasSize)
    {
        var io = ImGui.GetIO();
        var mousePos = io.MousePos;
        
        // Check if mouse is over canvas
        bool isHovered = mousePos.X >= canvasPos.X && mousePos.X <= canvasPos.X + canvasSize.X &&
                        mousePos.Y >= canvasPos.Y && mousePos.Y <= canvasPos.Y + canvasSize.Y;
        
        if (!isHovered) return;
        
        // Pan with drag
        if (ImGui.IsMouseDragging(ImGuiMouseButton.Left))
        {
            var delta = io.MouseDelta;
            _viewOffset += delta / _zoomLevel;
        }
        
        // Zoom with scroll
        if (io.MouseWheel != 0)
        {
            float zoomFactor = 1.0f + (io.MouseWheel * 0.1f);
            _zoomLevel = Math.Clamp(_zoomLevel * zoomFactor, 0.2f, 5.0f);
        }
        
        // Sector selection and hover
        var sectorCoord = ScreenToSector(mousePos, canvasPos, canvasSize);
        if (sectorCoord.Z == _viewSliceZ)
        {
            _hoveredSector = sectorCoord;
            
            // Select sector on click
            if (ImGui.IsMouseClicked(ImGuiMouseButton.Left))
            {
                _selectedSector = sectorCoord;
            }
            
            // Initiate jump on right-click
            if (ImGui.IsMouseClicked(ImGuiMouseButton.Right) && _playerShipId.HasValue)
            {
                InitiateJumpToSector(sectorCoord);
            }
        }
        else
        {
            _hoveredSector = null;
        }
    }
    
    private void DrawGrid(ImDrawListPtr drawList, Vector2 canvasPos, Vector2 canvasSize)
    {
        var gridColor = ImGui.GetColorU32(new Vector4(0.2f, 0.2f, 0.3f, 0.5f));
        var centerX = canvasPos.X + canvasSize.X / 2 + _viewOffset.X;
        var centerY = canvasPos.Y + canvasSize.Y / 2 + _viewOffset.Y;
        
        float cellSize = 40f * _zoomLevel;
        
        // Draw vertical lines
        for (float x = centerX % cellSize; x < canvasSize.X; x += cellSize)
        {
            drawList.AddLine(new Vector2(canvasPos.X + x, canvasPos.Y), 
                           new Vector2(canvasPos.X + x, canvasPos.Y + canvasSize.Y), gridColor);
        }
        
        // Draw horizontal lines
        for (float y = centerY % cellSize; y < canvasSize.Y; y += cellSize)
        {
            drawList.AddLine(new Vector2(canvasPos.X, canvasPos.Y + y), 
                           new Vector2(canvasPos.X + canvasSize.X, canvasPos.Y + y), gridColor);
        }
        
        // Draw center lines (galaxy center)
        drawList.AddLine(new Vector2(centerX, canvasPos.Y), 
                        new Vector2(centerX, canvasPos.Y + canvasSize.Y), 
                        ImGui.GetColorU32(new Vector4(0.4f, 0.4f, 0.6f, 0.8f)), 2f);
        drawList.AddLine(new Vector2(canvasPos.X, centerY), 
                        new Vector2(canvasPos.X + canvasSize.X, centerY), 
                        ImGui.GetColorU32(new Vector4(0.4f, 0.4f, 0.6f, 0.8f)), 2f);
    }
    
    private void DrawSectors(ImDrawListPtr drawList, Vector2 canvasPos, Vector2 canvasSize)
    {
        SectorCoordinate? playerLocation = null;
        float jumpRange = 0f;
        
        if (_playerShipId.HasValue)
        {
            var location = _entityManager.GetComponent<SectorLocationComponent>(_playerShipId.Value);
            var hyperdrive = _entityManager.GetComponent<HyperdriveComponent>(_playerShipId.Value);
            
            if (location != null)
            {
                playerLocation = location.CurrentSector;
                jumpRange = hyperdrive?.JumpRange ?? 0f;
            }
        }
        
        // Determine visible sector range
        int viewRadius = (int)Math.Ceiling(Math.Max(canvasSize.X, canvasSize.Y) / (40f * _zoomLevel)) + 2;
        int centerX = playerLocation?.X ?? 0;
        int centerY = playerLocation?.Y ?? 0;
        
        // Draw jump range circle
        if (_showJumpRange && playerLocation != null && jumpRange > 0)
        {
            var rangeCenter = SectorToScreen(playerLocation, canvasPos, canvasSize);
            float rangeRadius = jumpRange * 40f * _zoomLevel;
            drawList.AddCircle(rangeCenter, rangeRadius, 
                ImGui.GetColorU32(new Vector4(0.3f, 0.8f, 1.0f, 0.3f)), 64, 2f);
        }
        
        // Draw sectors
        for (int x = centerX - viewRadius; x <= centerX + viewRadius; x++)
        {
            for (int y = centerY - viewRadius; y <= centerY + viewRadius; y++)
            {
                var sectorCoord = new SectorCoordinate(x, y, _viewSliceZ);
                DrawSector(drawList, canvasPos, canvasSize, sectorCoord, playerLocation, jumpRange);
            }
        }
    }
    
    private void DrawSector(ImDrawListPtr drawList, Vector2 canvasPos, Vector2 canvasSize, 
        SectorCoordinate coord, SectorCoordinate? playerLocation, float jumpRange)
    {
        var screenPos = SectorToScreen(coord, canvasPos, canvasSize);
        float sectorSize = 36f * _zoomLevel;
        
        // Get or generate sector data
        string sectorKey = $"{coord.X}_{coord.Y}_{coord.Z}";
        if (!_sectorCache.ContainsKey(sectorKey))
        {
            _sectorCache[sectorKey] = _galaxyGenerator.GenerateSector(coord.X, coord.Y, coord.Z);
            
            // Limit cache size
            if (_sectorCache.Count > CACHE_RADIUS * CACHE_RADIUS * 4)
            {
                var oldestKey = _sectorCache.Keys.First();
                _sectorCache.Remove(oldestKey);
            }
        }
        
        var sector = _sectorCache[sectorKey];
        
        // Determine sector color based on content and state
        Vector4 sectorColor = new Vector4(0.2f, 0.2f, 0.25f, 1.0f); // Default empty
        
        // Color based on distance from center (tech level)
        int techLevel = coord.GetTechLevel();
        sectorColor = GetTechLevelColor(techLevel);
        
        // Highlight if has station
        if (_showStations && sector.Station != null)
        {
            sectorColor = new Vector4(0.3f, 0.7f, 0.3f, 1.0f);
        }
        
        // Highlight if has many asteroids
        if (_showAsteroids && sector.Asteroids.Count > 15)
        {
            sectorColor = new Vector4(0.6f, 0.5f, 0.3f, 1.0f);
        }
        
        // Highlight current sector
        if (playerLocation != null && coord.X == playerLocation.X && coord.Y == playerLocation.Y && coord.Z == playerLocation.Z)
        {
            sectorColor = new Vector4(0.3f, 0.8f, 1.0f, 1.0f);
        }
        
        // Highlight selected sector
        if (_selectedSector != null && coord.X == _selectedSector.X && coord.Y == _selectedSector.Y && coord.Z == _selectedSector.Z)
        {
            drawList.AddRectFilled(
                new Vector2(screenPos.X - sectorSize/2 - 2, screenPos.Y - sectorSize/2 - 2),
                new Vector2(screenPos.X + sectorSize/2 + 2, screenPos.Y + sectorSize/2 + 2),
                ImGui.GetColorU32(new Vector4(1.0f, 0.8f, 0.3f, 0.8f)));
        }
        
        // Highlight hovered sector
        if (_hoveredSector != null && coord.X == _hoveredSector.X && coord.Y == _hoveredSector.Y && coord.Z == _hoveredSector.Z)
        {
            drawList.AddRect(
                new Vector2(screenPos.X - sectorSize/2 - 2, screenPos.Y - sectorSize/2 - 2),
                new Vector2(screenPos.X + sectorSize/2 + 2, screenPos.Y + sectorSize/2 + 2),
                ImGui.GetColorU32(new Vector4(1.0f, 1.0f, 1.0f, 0.8f)), 0f, ImDrawFlags.None, 2f);
        }
        
        // Draw sector
        drawList.AddRectFilled(
            new Vector2(screenPos.X - sectorSize/2, screenPos.Y - sectorSize/2),
            new Vector2(screenPos.X + sectorSize/2, screenPos.Y + sectorSize/2),
            ImGui.GetColorU32(sectorColor));
        
        // Draw border
        drawList.AddRect(
            new Vector2(screenPos.X - sectorSize/2, screenPos.Y - sectorSize/2),
            new Vector2(screenPos.X + sectorSize/2, screenPos.Y + sectorSize/2),
            ImGui.GetColorU32(new Vector4(0.4f, 0.4f, 0.5f, 0.8f)));
        
        // Draw icons for content (if zoomed in enough)
        if (_zoomLevel > 0.8f)
        {
            if (_showStations && sector.Station != null)
            {
                drawList.AddCircleFilled(screenPos, 3f * _zoomLevel, 
                    ImGui.GetColorU32(new Vector4(0.3f, 1.0f, 0.3f, 1.0f)));
            }
            
            if (_showAsteroids && sector.Asteroids.Count > 0)
            {
                drawList.AddCircleFilled(new Vector2(screenPos.X - 6 * _zoomLevel, screenPos.Y), 2f * _zoomLevel, 
                    ImGui.GetColorU32(new Vector4(0.8f, 0.7f, 0.5f, 1.0f)));
            }
        }
        
        // Check if out of jump range
        if (playerLocation != null && jumpRange > 0)
        {
            float distance = coord.DistanceTo(playerLocation);
            if (distance > jumpRange)
            {
                // Darken sectors out of range
                drawList.AddRectFilled(
                    new Vector2(screenPos.X - sectorSize/2, screenPos.Y - sectorSize/2),
                    new Vector2(screenPos.X + sectorSize/2, screenPos.Y + sectorSize/2),
                    ImGui.GetColorU32(new Vector4(0.0f, 0.0f, 0.0f, 0.5f)));
            }
        }
    }
    
    private Vector4 GetTechLevelColor(int techLevel)
    {
        return techLevel switch
        {
            7 => new Vector4(0.9f, 0.2f, 0.2f, 1.0f),  // Avorion - Red (core)
            6 => new Vector4(0.8f, 0.4f, 0.2f, 1.0f),  // Ogonite - Orange
            5 => new Vector4(0.7f, 0.6f, 0.3f, 1.0f),  // Xanion - Yellow
            4 => new Vector4(0.4f, 0.7f, 0.4f, 1.0f),  // Trinium - Green
            3 => new Vector4(0.3f, 0.5f, 0.7f, 1.0f),  // Naonite - Blue
            2 => new Vector4(0.3f, 0.4f, 0.6f, 1.0f),  // Titanium - Dark blue
            _ => new Vector4(0.25f, 0.25f, 0.3f, 1.0f) // Iron - Gray (outer)
        };
    }
    
    private Vector2 SectorToScreen(SectorCoordinate sector, Vector2 canvasPos, Vector2 canvasSize)
    {
        float cellSize = 40f * _zoomLevel;
        float centerX = canvasPos.X + canvasSize.X / 2 + _viewOffset.X;
        float centerY = canvasPos.Y + canvasSize.Y / 2 + _viewOffset.Y;
        
        return new Vector2(
            centerX + (sector.X * cellSize),
            centerY + (sector.Y * cellSize)
        );
    }
    
    private SectorCoordinate ScreenToSector(Vector2 screenPos, Vector2 canvasPos, Vector2 canvasSize)
    {
        float cellSize = 40f * _zoomLevel;
        float centerX = canvasPos.X + canvasSize.X / 2 + _viewOffset.X;
        float centerY = canvasPos.Y + canvasSize.Y / 2 + _viewOffset.Y;
        
        int x = (int)Math.Round((screenPos.X - centerX) / cellSize);
        int y = (int)Math.Round((screenPos.Y - centerY) / cellSize);
        
        return new SectorCoordinate(x, y, _viewSliceZ);
    }
    
    private void RenderInfoPanel()
    {
        ImGui.TextColored(new Vector4(0.8f, 0.8f, 1.0f, 1.0f), "SECTOR INFORMATION");
        ImGui.Separator();
        
        // Show hovered or selected sector info
        var infoSector = _selectedSector ?? _hoveredSector;
        
        if (infoSector != null)
        {
            ImGui.Text($"Sector: ({infoSector.X}, {infoSector.Y}, {infoSector.Z})");
            
            float distanceFromCenter = infoSector.DistanceFromCenter();
            ImGui.Text($"Distance from Center: {distanceFromCenter:F1}");
            
            int techLevel = infoSector.GetTechLevel();
            string[] techNames = { "", "Iron", "Titanium", "Naonite", "Trinium", "Xanion", "Ogonite", "Avorion" };
            ImGui.Text($"Tech Level: {techLevel} ({techNames[techLevel]})");
            
            // Get sector data
            string sectorKey = $"{infoSector.X}_{infoSector.Y}_{infoSector.Z}";
            if (_sectorCache.ContainsKey(sectorKey))
            {
                var sector = _sectorCache[sectorKey];
                
                ImGui.Separator();
                ImGui.TextColored(new Vector4(0.7f, 0.9f, 0.7f, 1.0f), "CONTENTS:");
                
                if (sector.Station != null)
                {
                    ImGui.BulletText($"Station: {sector.Station.Name}");
                    ImGui.Text($"  Type: {sector.Station.StationType}");
                }
                else
                {
                    ImGui.TextColored(new Vector4(0.6f, 0.6f, 0.6f, 1.0f), "No station");
                }
                
                ImGui.BulletText($"Asteroids: {sector.Asteroids.Count}");
                if (sector.Asteroids.Count > 0)
                {
                    var resourceCounts = sector.Asteroids.GroupBy(a => a.ResourceType)
                        .ToDictionary(g => g.Key, g => g.Count());
                    
                    foreach (var resource in resourceCounts)
                    {
                        ImGui.Text($"  {resource.Key}: {resource.Value}");
                    }
                }
                
                if (sector.Ships.Count > 0)
                {
                    ImGui.BulletText($"Ships: {sector.Ships.Count}");
                }
            }
            
            ImGui.Separator();
            
            // Show distance from player
            if (_playerShipId.HasValue)
            {
                var location = _entityManager.GetComponent<SectorLocationComponent>(_playerShipId.Value);
                var hyperdrive = _entityManager.GetComponent<HyperdriveComponent>(_playerShipId.Value);
                
                if (location != null)
                {
                    float distance = infoSector.DistanceTo(location.CurrentSector);
                    ImGui.Text($"Distance: {distance:F1} sectors");
                    
                    if (hyperdrive != null)
                    {
                        bool inRange = distance <= hyperdrive.JumpRange;
                        
                        if (inRange)
                        {
                            ImGui.TextColored(new Vector4(0.3f, 1.0f, 0.3f, 1.0f), "WITHIN JUMP RANGE");
                            
                            if (hyperdrive.CanJump)
                            {
                                if (ImGui.Button("Initiate Jump", new Vector2(-1, 30)))
                                {
                                    InitiateJumpToSector(infoSector);
                                }
                            }
                            else if (hyperdrive.IsCharging)
                            {
                                float chargePercent = (hyperdrive.CurrentCharge / hyperdrive.ChargeTime) * 100f;
                                ImGui.ProgressBar(chargePercent / 100f, new Vector2(-1, 0), $"Charging: {chargePercent:F0}%%");
                                
                                if (ImGui.Button("Cancel Jump", new Vector2(-1, 30)))
                                {
                                    _navigationSystem.CancelJump(_playerShipId.Value);
                                }
                            }
                            else
                            {
                                float cooldownPercent = (hyperdrive.TimeSinceLastJump / hyperdrive.JumpCooldown) * 100f;
                                ImGui.ProgressBar(cooldownPercent / 100f, new Vector2(-1, 0), $"Cooldown: {cooldownPercent:F0}%%");
                            }
                        }
                        else
                        {
                            ImGui.TextColored(new Vector4(1.0f, 0.4f, 0.4f, 1.0f), "OUT OF JUMP RANGE");
                            ImGui.Text($"Need range: {distance:F1}");
                            ImGui.Text($"Current range: {hyperdrive.JumpRange:F1}");
                        }
                    }
                }
            }
        }
        else
        {
            ImGui.TextColored(new Vector4(0.6f, 0.6f, 0.6f, 1.0f), "Hover or select a sector to view details");
        }
        
        ImGui.Separator();
        ImGui.Spacing();
        
        // Legend
        ImGui.TextColored(new Vector4(0.8f, 0.8f, 1.0f, 1.0f), "LEGEND:");
        ImGui.TextColored(new Vector4(0.3f, 0.8f, 1.0f, 1.0f), "■"); ImGui.SameLine(); ImGui.Text("Current Location");
        ImGui.TextColored(new Vector4(0.3f, 0.7f, 0.3f, 1.0f), "■"); ImGui.SameLine(); ImGui.Text("Station");
        ImGui.TextColored(new Vector4(0.6f, 0.5f, 0.3f, 1.0f), "■"); ImGui.SameLine(); ImGui.Text("Rich Asteroids");
        ImGui.TextColored(new Vector4(0.9f, 0.2f, 0.2f, 1.0f), "■"); ImGui.SameLine(); ImGui.Text("Core (Avorion)");
        ImGui.TextColored(new Vector4(0.25f, 0.25f, 0.3f, 1.0f), "■"); ImGui.SameLine(); ImGui.Text("Outer (Iron)");
        
        ImGui.Spacing();
        ImGui.Separator();
        ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1.0f), "Press M to close");
    }
    
    private void InitiateJumpToSector(SectorCoordinate targetSector)
    {
        if (!_playerShipId.HasValue) return;
        
        bool success = _navigationSystem.StartJumpCharge(_playerShipId.Value, targetSector);
        
        if (success)
        {
            _selectedSector = targetSector;
        }
    }
}
