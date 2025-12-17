using System.Numerics;
using ImGuiNET;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Building;

namespace AvorionLike.Core.UI;

/// <summary>
/// Full-screen builder interface matching Avorion's build mode
/// Provides comprehensive ship construction UI with tool sidebar, block palette, and statistics
/// </summary>
public class BuilderModeUI
{
    private readonly GameEngine _gameEngine;
    private readonly EnhancedBuildSystem _buildSystem;
    private bool _isActive = false;
    
    // Current selections
    private BuildTool _currentTool = BuildTool.Add;
    private string _selectedMaterial = "Iron";
    private BlockType _selectedBlockType = BlockType.Hull;
    private BlockShape _selectedBlockShape = BlockShape.Cube;
    private Vector3 _blockSize = new(2, 2, 2);
    
    // UI State
    private bool _showGrid = true;
    private bool _gridSnap = true;
    private float _gridSize = 2.0f;
    private MirrorAxis _mirrorMode = MirrorAxis.None;
    
    // Placement
    private Vector3 _placementPosition = Vector3.Zero;
    
    // Statistics
    private ShipStatistics? _currentStats;
    
    // Material colors for UI (RGB format for ImGui)
    private readonly Dictionary<string, Vector4> _materialColors = new()
    {
        ["Iron"] = new Vector4(0.72f, 0.72f, 0.75f, 1.0f),      // #B8B8C0
        ["Titanium"] = new Vector4(0.82f, 0.87f, 0.95f, 1.0f),   // #D0DEF2
        ["Naonite"] = new Vector4(0.15f, 0.92f, 0.35f, 1.0f),    // #26EB59
        ["Trinium"] = new Vector4(0.25f, 0.65f, 1.0f, 1.0f),     // #40A6FF
        ["Xanion"] = new Vector4(1.0f, 0.82f, 0.15f, 1.0f),      // #FFD126
        ["Ogonite"] = new Vector4(1.0f, 0.40f, 0.15f, 1.0f),     // #FF6626
        ["Avorion"] = new Vector4(0.85f, 0.20f, 1.0f, 1.0f)      // #D933FF
    };
    
    public bool IsActive => _isActive;
    public BuildTool CurrentTool => _currentTool;
    public Vector3 PlacementPosition => _placementPosition;
    
    public BuilderModeUI(GameEngine gameEngine)
    {
        _gameEngine = gameEngine;
        _buildSystem = new EnhancedBuildSystem(gameEngine.EntityManager);
    }
    
    /// <summary>
    /// Enter build mode for a specific ship
    /// </summary>
    public void EnterBuildMode(Guid shipId)
    {
        _buildSystem.EnterBuildMode(shipId);
        _isActive = true;
        UpdateStatistics();
    }
    
    /// <summary>
    /// Exit build mode
    /// </summary>
    public void ExitBuildMode()
    {
        _buildSystem.ExitBuildMode();
        _isActive = false;
    }
    
    /// <summary>
    /// Toggle build mode
    /// </summary>
    public void Toggle()
    {
        if (_isActive)
        {
            ExitBuildMode();
        }
        else
        {
            // Find first ship to build
            var ships = _gameEngine.EntityManager.GetAllEntities()
                .Where(e => _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(e.Id) != null)
                .ToList();
            
            if (ships.Any())
            {
                EnterBuildMode(ships.First().Id);
            }
        }
    }
    
    /// <summary>
    /// Handle keyboard shortcuts
    /// </summary>
    public void HandleInput()
    {
        if (!_isActive) return;
        
        // Toggle build mode with B key
        if (ImGui.IsKeyPressed(ImGuiKey.B))
        {
            Toggle();
        }
        
        // Tool shortcuts
        if (ImGui.IsKeyPressed(ImGuiKey.Keypad1) || ImGui.IsKeyPressed(ImGuiKey._1))
            _currentTool = BuildTool.Add;
        if (ImGui.IsKeyPressed(ImGuiKey.Keypad2) || ImGui.IsKeyPressed(ImGuiKey._2))
            _currentTool = BuildTool.Remove;
        if (ImGui.IsKeyPressed(ImGuiKey.Keypad3) || ImGui.IsKeyPressed(ImGuiKey._3))
            _currentTool = BuildTool.Select;
        if (ImGui.IsKeyPressed(ImGuiKey.Keypad4) || ImGui.IsKeyPressed(ImGuiKey._4))
            _currentTool = BuildTool.Paint;
        if (ImGui.IsKeyPressed(ImGuiKey.Keypad5) || ImGui.IsKeyPressed(ImGuiKey._5))
            _currentTool = BuildTool.Transform;
    }
    
    /// <summary>
    /// Render the full builder UI
    /// </summary>
    public void Render()
    {
        if (!_isActive) return;
        
        var io = ImGui.GetIO();
        var displaySize = io.DisplaySize;
        
        // Make window fullscreen, no decorations
        ImGui.SetNextWindowPos(Vector2.Zero);
        ImGui.SetNextWindowSize(displaySize);
        
        ImGuiWindowFlags flags = ImGuiWindowFlags.NoTitleBar | 
                                ImGuiWindowFlags.NoResize | 
                                ImGuiWindowFlags.NoMove |
                                ImGuiWindowFlags.NoCollapse |
                                ImGuiWindowFlags.NoBackground |
                                ImGuiWindowFlags.NoBringToFrontOnFocus;
        
        if (ImGui.Begin("##BuilderMode", flags))
        {
            RenderTopResourcePanel();
            RenderLeftToolbar();
            RenderBlockSelectionPanel();
            RenderRightStatsPanel();
            RenderBottomToolbar();
        }
        ImGui.End();
    }
    
    /// <summary>
    /// Render top-left resource panel
    /// </summary>
    private void RenderTopResourcePanel()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(10, 10));
        ImGui.SetNextWindowSize(new Vector2(400, 120));
        
        ImGuiWindowFlags flags = ImGuiWindowFlags.NoTitleBar | 
                                ImGuiWindowFlags.NoResize | 
                                ImGuiWindowFlags.NoMove |
                                ImGuiWindowFlags.NoCollapse;
        
        if (ImGui.Begin("##ResourcePanel", flags))
        {
            // Credits and Sector
            ImGui.Text("Credits");
            ImGui.SameLine(120);
            
            var inventory = GetCurrentInventory();
            int credits = inventory?.GetResourceAmount(ResourceType.Credits) ?? 0;
            ImGui.Text($"£{credits:N0}");
            
            ImGui.SameLine(250);
            ImGui.Text("Sector");
            ImGui.SameLine(320);
            ImGui.Text("-134 : 429"); // Placeholder - would be actual sector coords
            
            ImGui.Separator();
            
            // Materials in two columns
            ImGui.BeginGroup();
            {
                RenderMaterialRow("Iron", ResourceType.Iron, inventory);
                RenderMaterialRow("Titanium", ResourceType.Titanium, inventory);
                RenderMaterialRow("Naonite", ResourceType.Naonite, inventory);
                RenderMaterialRow("Trinium", ResourceType.Trinium, inventory);
            }
            ImGui.EndGroup();
            
            ImGui.SameLine(220);
            
            ImGui.BeginGroup();
            {
                RenderMaterialRow("Xanion", ResourceType.Xanion, inventory);
                RenderMaterialRow("Ogonite", ResourceType.Ogonite, inventory);
                RenderMaterialRow("Avorion", ResourceType.Avorion, inventory);
            }
            ImGui.EndGroup();
        }
        ImGui.End();
    }
    
    private void RenderMaterialRow(string name, ResourceType type, Inventory? inventory)
    {
        var color = _materialColors.GetValueOrDefault(name, Vector4.One);
        int amount = inventory?.GetResourceAmount(type) ?? 0;
        
        ImGui.TextColored(color, name);
        ImGui.SameLine(120);
        
        if (amount > 0)
        {
            ImGui.Text($"{amount:N0}");
        }
        else
        {
            ImGui.TextColored(new Vector4(1, 0, 0, 1), "0");
        }
    }
    
    /// <summary>
    /// Render left toolbar with tool icons
    /// </summary>
    private void RenderLeftToolbar()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(10, 150));
        ImGui.SetNextWindowSize(new Vector2(60, 500));
        
        ImGuiWindowFlags flags = ImGuiWindowFlags.NoTitleBar | 
                                ImGuiWindowFlags.NoResize | 
                                ImGuiWindowFlags.NoMove |
                                ImGuiWindowFlags.NoCollapse;
        
        if (ImGui.Begin("##ToolbarPanel", flags))
        {
            // Tool buttons (represented as text for now)
            RenderToolButton("Add", BuildTool.Add, "[1]");
            RenderToolButton("Remove", BuildTool.Remove, "[2]");
            RenderToolButton("Select", BuildTool.Select, "[3]");
            RenderToolButton("Paint", BuildTool.Paint, "[4]");
            RenderToolButton("Transform", BuildTool.Transform, "[5]");
            RenderToolButton("Rotate", BuildTool.Rotate, "[R]");
            RenderToolButton("Move", BuildTool.Move, "[M]");
            RenderToolButton("Scale", BuildTool.Scale, "[S]");
            
            ImGui.Separator();
            
            // Settings toggles
            bool showGrid = _showGrid;
            if (ImGui.Checkbox("Grid", ref showGrid))
            {
                _showGrid = showGrid;
            }
            
            bool gridSnap = _gridSnap;
            if (ImGui.Checkbox("Snap", ref gridSnap))
            {
                _gridSnap = gridSnap;
            }
        }
        ImGui.End();
    }
    
    private void RenderToolButton(string label, BuildTool tool, string shortcut)
    {
        bool isSelected = _currentTool == tool;
        
        if (isSelected)
        {
            ImGui.PushStyleColor(ImGuiCol.Button, new Vector4(0.4f, 0.6f, 0.8f, 1.0f));
        }
        
        if (ImGui.Button(label, new Vector2(50, 30)))
        {
            _currentTool = tool;
        }
        
        if (isSelected)
        {
            ImGui.PopStyleColor();
        }
        
        if (ImGui.IsItemHovered())
        {
            ImGui.SetTooltip($"{label} {shortcut}");
        }
    }
    
    /// <summary>
    /// Render center block selection panel
    /// </summary>
    private void RenderBlockSelectionPanel()
    {
        var io = ImGui.GetIO();
        float panelWidth = 630;
        float panelHeight = 550;
        float centerX = (io.DisplaySize.X - panelWidth) / 2;
        float centerY = (io.DisplaySize.Y - panelHeight) / 2;
        
        ImGui.SetNextWindowPos(new Vector2(centerX, centerY));
        ImGui.SetNextWindowSize(new Vector2(panelWidth, panelHeight));
        
        ImGuiWindowFlags flags = ImGuiWindowFlags.NoMove |
                                ImGuiWindowFlags.NoResize |
                                ImGuiWindowFlags.NoCollapse;
        
        if (ImGui.Begin("Blocks", flags))
        {
            // Material tabs
            if (ImGui.BeginTabBar("MaterialTabs"))
            {
                foreach (var materialName in MaterialProperties.Materials.Keys)
                {
                    var color = _materialColors.GetValueOrDefault(materialName, Vector4.One);
                    ImGui.PushStyleColor(ImGuiCol.Text, color);
                    
                    if (ImGui.BeginTabItem(materialName))
                    {
                        _selectedMaterial = materialName;
                        RenderBlockGrid(materialName);
                        ImGui.EndTabItem();
                    }
                    
                    ImGui.PopStyleColor();
                }
                
                ImGui.EndTabBar();
            }
            
            ImGui.Separator();
            
            // Block transformation tools at bottom
            ImGui.Text("Transform Block (Alt)");
            ImGui.Text("Match Block (Ctrl)");
            ImGui.Text("Match Shape (Ctrl)");
        }
        ImGui.End();
    }
    
    /// <summary>
    /// Render grid of blocks for selected material
    /// </summary>
    private void RenderBlockGrid(string materialName)
    {
        var color = _materialColors.GetValueOrDefault(materialName, Vector4.One);
        
        // Block types
        var blockTypes = Enum.GetValues<BlockType>();
        int columns = 8;
        int buttonSize = 65;
        
        for (int i = 0; i < blockTypes.Length; i++)
        {
            var blockType = blockTypes[i];
            bool isSelected = _selectedBlockType == blockType && _selectedMaterial == materialName;
            
            if (isSelected)
            {
                ImGui.PushStyleColor(ImGuiCol.Button, new Vector4(color.X, color.Y, color.Z, 0.7f));
            }
            else
            {
                ImGui.PushStyleColor(ImGuiCol.Button, new Vector4(0.2f, 0.2f, 0.2f, 0.8f));
            }
            
            string label = GetBlockTypeShortName(blockType);
            
            if (ImGui.Button($"{label}##{blockType}_{materialName}", new Vector2(buttonSize, buttonSize)))
            {
                _selectedBlockType = blockType;
                _selectedMaterial = materialName;
            }
            
            ImGui.PopStyleColor();
            
            if (ImGui.IsItemHovered())
            {
                ImGui.SetTooltip(blockType.ToString());
            }
            
            // Next column or new row
            if ((i + 1) % columns != 0)
            {
                ImGui.SameLine();
            }
        }
        
        ImGui.Separator();
        
        // Block shapes
        ImGui.Text("Shapes:");
        var shapes = Enum.GetValues<BlockShape>();
        for (int i = 0; i < shapes.Length; i++)
        {
            var shape = shapes[i];
            bool isSelected = _selectedBlockShape == shape;
            
            if (isSelected)
            {
                ImGui.PushStyleColor(ImGuiCol.Button, new Vector4(0.4f, 0.6f, 0.8f, 1.0f));
            }
            
            if (ImGui.Button(shape.ToString(), new Vector2(85, 30)))
            {
                _selectedBlockShape = shape;
            }
            
            if (isSelected)
            {
                ImGui.PopStyleColor();
            }
            
            if (i < shapes.Length - 1)
            {
                ImGui.SameLine();
            }
        }
    }
    
    private string GetBlockTypeShortName(BlockType type)
    {
        return type switch
        {
            BlockType.Hull => "Hull",
            BlockType.Armor => "Armor",
            BlockType.Engine => "Engine",
            BlockType.Thruster => "Thrust",
            BlockType.GyroArray => "Gyro",
            BlockType.Generator => "Power",
            BlockType.ShieldGenerator => "Shield",
            BlockType.TurretMount => "Turret",
            BlockType.HyperdriveCore => "Hyper",
            BlockType.Cargo => "Cargo",
            BlockType.CrewQuarters => "Crew",
            BlockType.PodDocking => "Dock",
            BlockType.Computer => "CPU",
            BlockType.Battery => "Battery",
            BlockType.IntegrityField => "Field",
            _ => type.ToString()
        };
    }
    
    /// <summary>
    /// Render right statistics panel
    /// </summary>
    private void RenderRightStatsPanel()
    {
        var io = ImGui.GetIO();
        float panelWidth = 300;
        float panelHeight = 600;
        float rightX = io.DisplaySize.X - panelWidth - 10;
        
        ImGui.SetNextWindowPos(new Vector2(rightX, 10));
        ImGui.SetNextWindowSize(new Vector2(panelWidth, panelHeight));
        
        ImGuiWindowFlags flags = ImGuiWindowFlags.NoTitleBar | 
                                ImGuiWindowFlags.NoResize | 
                                ImGuiWindowFlags.NoMove |
                                ImGuiWindowFlags.NoCollapse;
        
        if (ImGui.Begin("##StatsPanel", flags))
        {
            if (_currentStats != null)
            {
                ImGui.Text($"Hull:          {_currentStats.MaxHull:F0}/{_currentStats.MaxHull:F0}");
                ImGui.Text($"Mass:          {_currentStats.TotalMass:F1} t");
                ImGui.Text($"Volume:        {_currentStats.TotalVolume:F1} m³");
                
                ImGui.Separator();
                
                ImGui.Text($"Deceleration:  {_currentStats.Acceleration:F2} m/s²");
                ImGui.Text($"Yaw Speed:     {_currentStats.YawSpeed:F2}°/s");
                ImGui.Text($"Pitch Speed:   {_currentStats.PitchSpeed:F2}°/s");
                ImGui.Text($"Roll Speed:    {_currentStats.RollSpeed:F2}°/s");
                
                ImGui.Separator();
                
                ImGui.Text("Power:");
                float powerBalance = _currentStats.PowerBalance;
                var powerColor = powerBalance >= 0 ? new Vector4(0, 1, 0, 1) : new Vector4(1, 0, 0, 1);
                ImGui.TextColored(powerColor, $"  {_currentStats.PowerGeneration:F1} MW");
                ImGui.Text($"Required Power: {_currentStats.PowerConsumption:F1} MW");
                
                ImGui.Separator();
                
                ImGui.Text("Crew:");
                ImGui.Text($"  Max Crew:      {_currentStats.MaxCrew}");
                ImGui.Text($"  Required Crew: {_currentStats.RequiredCrew}");
                
                ImGui.Separator();
                
                ImGui.Text("Energy:");
                ImGui.Text($"  Storage:       {_currentStats.EnergyCapacity:F1} GJ");
                ImGui.Text($"  Regen Energy:  {(_currentStats.PowerGeneration * 0.1f):F2} GJ/s");
                
                ImGui.Separator();
                
                ImGui.Text("Shields:");
                ImGui.Text($"  Max Shields:   {_currentStats.MaxShields:F0}");
                
                ImGui.Separator();
                
                ImGui.Text("Cargo:");
                ImGui.Text($"  Capacity:      {_currentStats.CargoCapacity:F0}");
                
                ImGui.Separator();
                
                ImGui.Text($"Turret Slots:    {_currentStats.TurretSlots}");
                ImGui.Text($"Hyperdrive Range:{_currentStats.HyperdriveRange:F1}");
            }
            else
            {
                ImGui.Text("No statistics available");
            }
        }
        ImGui.End();
    }
    
    /// <summary>
    /// Render bottom toolbar
    /// </summary>
    private void RenderBottomToolbar()
    {
        var io = ImGui.GetIO();
        float toolbarHeight = 60;
        ImGui.SetNextWindowPos(new Vector2(0, io.DisplaySize.Y - toolbarHeight));
        ImGui.SetNextWindowSize(new Vector2(io.DisplaySize.X, toolbarHeight));
        
        ImGuiWindowFlags flags = ImGuiWindowFlags.NoTitleBar | 
                                ImGuiWindowFlags.NoResize | 
                                ImGuiWindowFlags.NoMove |
                                ImGuiWindowFlags.NoCollapse |
                                ImGuiWindowFlags.NoScrollbar;
        
        if (ImGui.Begin("##BottomToolbar", flags))
        {
            ImGui.Text("Rotate Camera [WASD+Shift]");
            ImGui.SameLine(250);
            ImGui.Text("Select [H]");
            ImGui.SameLine(350);
            ImGui.Text("Place Block [F]");
            ImGui.SameLine(500);
            ImGui.Text("[Shift] Show Hotkeys");
            ImGui.SameLine(650);
            ImGui.Text("[Ctrl] Match Block");
            ImGui.SameLine(800);
            ImGui.Text("[Ctrl] Match Shape");
            
            ImGui.Text("Number Keys 1-9: Quick select blocks");
        }
        ImGui.End();
    }
    
    /// <summary>
    /// Update ship statistics
    /// </summary>
    private void UpdateStatistics()
    {
        if (!_buildSystem.CurrentEntity.HasValue)
        {
            _currentStats = null;
            return;
        }
        
        _currentStats = _buildSystem.CalculateShipStatistics(_buildSystem.CurrentEntity.Value);
    }
    
    /// <summary>
    /// Get current ship inventory
    /// </summary>
    private Inventory? GetCurrentInventory()
    {
        if (!_buildSystem.CurrentEntity.HasValue) return null;
        
        var inventoryComp = _gameEngine.EntityManager.GetComponent<InventoryComponent>(_buildSystem.CurrentEntity.Value);
        return inventoryComp?.Inventory;
    }
    
    /// <summary>
    /// Place a block at current position
    /// </summary>
    public void PlaceBlock()
    {
        if (!_buildSystem.CurrentEntity.HasValue) return;
        
        _buildSystem.AddBlock(_placementPosition, _blockSize, _selectedBlockType, _selectedMaterial);
        UpdateStatistics();
    }
}
