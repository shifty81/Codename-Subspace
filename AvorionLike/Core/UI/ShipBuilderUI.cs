using System.Numerics;
using ImGuiNET;
using Silk.NET.Input;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Resources;
using AvorionLike.Core.ECS;

namespace AvorionLike.Core.UI;

/// <summary>
/// Interactive ship builder UI for placing and removing voxel blocks
/// Provides a full ship construction interface with material selection, block types, and build controls
/// </summary>
public class ShipBuilderUI
{
    private readonly GameEngine _gameEngine;
    private readonly BuildSystem _buildSystem;
    private bool _isOpen = false;
    private bool _buildModeActive = false;
    private Guid _currentShipId = Guid.Empty;
    
    // Build settings
    private string _selectedMaterial = "Iron";
    private BlockType _selectedBlockType = BlockType.Hull;
    private Vector3 _blockSize = new(2, 2, 2);
    private int _gridSize = 2; // Grid snap size
    private bool _gridSnap = true;
    private bool _showGrid = true;
    
    // Placement state
    private Vector3 _placementPosition = Vector3.Zero;
    private bool _placementMode = true; // true = place, false = remove
    private string _statusMessage = "";
    private float _statusMessageTimer = 0f;
    
    // Blueprint save/load
    private bool _showSaveBlueprintDialog = false;
    private bool _showLoadBlueprintDialog = false;
    private string _blueprintName = "MyShip";
    private List<string> _availableBlueprints = new List<string>();
    private int _selectedBlueprintIndex = 0;
    
    // Statistics
    private int _totalBlocks = 0;
    private float _totalMass = 0f;
    private int _materialCost = 0;
    
    // Material colors for UI - matching reference image 4456.PNG (repository root)
    private readonly Dictionary<string, Vector4> _materialColors = new()
    {
        ["Iron"] = new Vector4(0.72f, 0.72f, 0.75f, 1.0f),      // #B8B8C0 - Polished steel grey
        ["Titanium"] = new Vector4(0.82f, 0.87f, 0.95f, 1.0f),   // #D0DEF2 - Brilliant silver-blue
        ["Naonite"] = new Vector4(0.15f, 0.92f, 0.35f, 1.0f),    // #26EB59 - Vivid emerald green
        ["Trinium"] = new Vector4(0.25f, 0.65f, 1.0f, 1.0f),     // #40A6FF - Brilliant sapphire blue
        ["Xanion"] = new Vector4(1.0f, 0.82f, 0.15f, 1.0f),      // #FFD126 - Brilliant gold
        ["Ogonite"] = new Vector4(1.0f, 0.40f, 0.15f, 1.0f),     // #FF6626 - Fiery orange-red
        ["Avorion"] = new Vector4(0.85f, 0.20f, 1.0f, 1.0f)      // #D933FF - Royal purple
    };
    
    public bool IsOpen => _isOpen;
    public bool IsBuildModeActive => _buildModeActive;
    public Vector3 PlacementPosition => _placementPosition;
    public Vector3 BlockSize => _blockSize;
    public bool ShowGrid => _showGrid;
    
    public ShipBuilderUI(GameEngine gameEngine)
    {
        _gameEngine = gameEngine;
        _buildSystem = new BuildSystem(gameEngine.EntityManager);
    }
    
    /// <summary>
    /// Open the ship builder for a specific ship
    /// </summary>
    public void Open(Guid shipId)
    {
        _currentShipId = shipId;
        _isOpen = true;
        
        // Start build session
        if (_buildSystem.StartBuildSession(shipId))
        {
            _buildModeActive = true;
            _buildSystem.SetSelectedMaterial(shipId, _selectedMaterial);
            _buildSystem.SetSelectedBlockType(shipId, _selectedBlockType);
            SetStatusMessage("Build mode activated", 3f);
        }
        else
        {
            SetStatusMessage("Failed to start build session", 3f);
        }
        
        UpdateStatistics();
    }
    
    /// <summary>
    /// Close the ship builder
    /// </summary>
    public void Close()
    {
        if (_buildModeActive && _currentShipId != Guid.Empty)
        {
            _buildSystem.EndBuildSession(_currentShipId);
            _buildModeActive = false;
        }
        
        _isOpen = false;
        _currentShipId = Guid.Empty;
    }
    
    /// <summary>
    /// Toggle the ship builder UI
    /// </summary>
    public void Toggle()
    {
        if (_isOpen)
        {
            Close();
        }
        else
        {
            // Find first ship with voxel structure
            var ships = _gameEngine.EntityManager.GetAllEntities()
                .Where(e => _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(e.Id) != null)
                .ToList();
            
            if (ships.Any())
            {
                Open(ships.First().Id);
            }
            else
            {
                SetStatusMessage("No ships found to build", 3f);
            }
        }
    }
    
    /// <summary>
    /// Render the ship builder UI - Enhanced Avorion-style interface
    /// </summary>
    public void Render()
    {
        if (!_isOpen) return;
        
        // Update status message timer
        if (_statusMessageTimer > 0)
        {
            _statusMessageTimer -= ImGui.GetIO().DeltaTime;
        }
        
        // Render comprehensive builder interface
        RenderResourcePanel();
        RenderMainBuilderWindow();
        RenderStatsPanel();
        RenderDialogs();
    }
    
    /// <summary>
    /// Render top-left resource panel showing credits and materials
    /// </summary>
    private void RenderResourcePanel()
    {
        ImGui.SetNextWindowPos(new Vector2(10, 10));
        ImGui.SetNextWindowSize(new Vector2(400, 120));
        ImGui.SetNextWindowBgAlpha(0.9f);
        
        ImGuiWindowFlags flags = ImGuiWindowFlags.NoTitleBar | 
                                ImGuiWindowFlags.NoResize | 
                                ImGuiWindowFlags.NoMove |
                                ImGuiWindowFlags.NoCollapse;
        
        if (ImGui.Begin("##ResourcePanel", flags))
        {
            var inventory = _currentShipId != Guid.Empty ? 
                _gameEngine.EntityManager.GetComponent<InventoryComponent>(_currentShipId)?.Inventory : null;
            
            // Credits and Sector info
            int credits = inventory?.GetResourceAmount(ResourceType.Credits) ?? 0;
            ImGui.Text("Credits");
            ImGui.SameLine(120);
            ImGui.Text($"£{credits:N0}");
            ImGui.SameLine(250);
            ImGui.Text("Sector");
            ImGui.SameLine(320);
            ImGui.Text("0:0"); // Placeholder
            
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
        var color = _materialColors.GetValueOrDefault(name, new Vector4(1, 1, 1, 1));
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
    /// Render main builder window with material tabs and block selection
    /// </summary>
    private void RenderMainBuilderWindow()
    {
        var io = ImGui.GetIO();
        float panelWidth = 630;
        float panelHeight = 550;
        float centerX = (io.DisplaySize.X - panelWidth) / 2;
        float centerY = (io.DisplaySize.Y - panelHeight) / 2;
        
        ImGui.SetNextWindowPos(new Vector2(centerX, centerY));
        ImGui.SetNextWindowSize(new Vector2(panelWidth, panelHeight));
        ImGui.SetNextWindowBgAlpha(0.95f);
        
        if (ImGui.Begin("Blocks", ref _isOpen, ImGuiWindowFlags.None))
        {
            // Status bar
            if (_statusMessageTimer > 0)
            {
                ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(1f, 1f, 0f, 1f));
                ImGui.Text(_statusMessage);
                ImGui.PopStyleColor();
            }
            else
            {
                ImGui.TextColored(new Vector4(0.5f, 0.5f, 0.5f, 1f), 
                    _buildModeActive ? "Build Mode Active" : "Build Mode Inactive");
            }
            
            ImGui.Separator();
            
            // Mode selection
            ImGui.Text("Mode:");
            ImGui.SameLine();
            if (ImGui.RadioButton("Place", _placementMode))
            {
                _placementMode = true;
            }
            ImGui.SameLine();
            if (ImGui.RadioButton("Remove", !_placementMode))
            {
                _placementMode = false;
            }
            
            ImGui.Separator();
            
            // Material tabs
            if (ImGui.BeginTabBar("MaterialTabs"))
            {
                foreach (var materialName in MaterialProperties.Materials.Keys)
                {
                    var color = _materialColors.GetValueOrDefault(materialName, new Vector4(1, 1, 1, 1));
                    ImGui.PushStyleColor(ImGuiCol.Text, color);
                    
                    if (ImGui.BeginTabItem(materialName))
                    {
                        _selectedMaterial = materialName;
                        if (_buildModeActive)
                        {
                            _buildSystem.SetSelectedMaterial(_currentShipId, materialName);
                        }
                        UpdateMaterialCost();
                        
                        RenderBlockGrid(materialName);
                        ImGui.EndTabItem();
                    }
                    
                    ImGui.PopStyleColor();
                }
                
                ImGui.EndTabBar();
            }
            
            ImGui.Separator();
            
            // Grid settings
            ImGui.Checkbox("Grid Snap", ref _gridSnap);
            ImGui.SameLine();
            ImGui.Checkbox("Show Grid", ref _showGrid);
            
            if (_gridSnap)
            {
                ImGui.SliderInt("Grid Size", ref _gridSize, 1, 10);
            }
            
            // Block size
            ImGui.Text("Block Size:");
            var size = new System.Numerics.Vector3(_blockSize.X, _blockSize.Y, _blockSize.Z);
            if (ImGui.DragFloat3("Size", ref size, 0.5f, 1f, 20f))
            {
                _blockSize = new Vector3(size.X, size.Y, size.Z);
                if (_buildModeActive)
                {
                    _buildSystem.SetSelectedSize(_currentShipId, _blockSize);
                }
                UpdateMaterialCost();
            }
            
            ImGui.Separator();
            
            // Placement controls
            ImGui.Text("Placement Position:");
            var pos = new System.Numerics.Vector3(_placementPosition.X, _placementPosition.Y, _placementPosition.Z);
            if (ImGui.DragFloat3("Position", ref pos, 1f))
            {
                _placementPosition = new Vector3(pos.X, pos.Y, pos.Z);
                if (_gridSnap)
                {
                    _placementPosition = SnapToGrid(_placementPosition, _gridSize);
                }
            }
            
            ImGui.Separator();
            
            // Action buttons
            if (_placementMode)
            {
                if (ImGui.Button("Place Block", new Vector2(-1, 30)))
                {
                    PlaceBlock();
                }
            }
            else
            {
                if (ImGui.Button("Remove Block", new Vector2(-1, 30)))
                {
                    RemoveBlock();
                }
            }
            
            ImGui.Separator();
            
            // Quick actions
            if (ImGui.Button("Save Blueprint", new Vector2(-1, 25)))
            {
                _showSaveBlueprintDialog = true;
            }
            
            if (ImGui.Button("Load Blueprint", new Vector2(-1, 25)))
            {
                _availableBlueprints = ShipBlueprint.ListBlueprints();
                _showLoadBlueprintDialog = true;
            }
            
            ImGui.Separator();
            
            // Help text
            ImGui.TextDisabled("Controls:");
            ImGui.TextDisabled("  B - Toggle Builder");
            ImGui.TextDisabled("  Mouse - Move placement cursor");
            ImGui.TextDisabled("  Click - Place/Remove block");
        }
        ImGui.End();
    }
    
    /// <summary>
    /// Render block grid for material
    /// </summary>
    private void RenderBlockGrid(string materialName)
    {
        var color = _materialColors.GetValueOrDefault(materialName, new Vector4(1, 1, 1, 1));
        
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
                if (_buildModeActive)
                {
                    _buildSystem.SetSelectedBlockType(_currentShipId, blockType);
                    _buildSystem.SetSelectedMaterial(_currentShipId, materialName);
                }
            }
            
            ImGui.PopStyleColor();
            
            if (ImGui.IsItemHovered())
            {
                ImGui.SetTooltip($"{blockType}\n{GetBlockTypeDescription(blockType)}");
            }
            
            // Next column or new row
            if ((i + 1) % columns != 0)
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
    /// Render stats panel on right side
    /// </summary>
    private void RenderStatsPanel()
    {
        var io = ImGui.GetIO();
        float panelWidth = 300;
        float panelHeight = 600;
        float rightX = io.DisplaySize.X - panelWidth - 10;
        
        ImGui.SetNextWindowPos(new Vector2(rightX, 10));
        ImGui.SetNextWindowSize(new Vector2(panelWidth, panelHeight));
        ImGui.SetNextWindowBgAlpha(0.9f);
        
        ImGuiWindowFlags flags = ImGuiWindowFlags.NoTitleBar | 
                                ImGuiWindowFlags.NoResize | 
                                ImGuiWindowFlags.NoMove |
                                ImGuiWindowFlags.NoCollapse;
        
        if (ImGui.Begin("##StatsPanel", flags))
        {
            ImGui.Text("Ship Statistics");
            ImGui.Separator();
            
            ImGui.Text($"Total Blocks: {_totalBlocks}");
            ImGui.Text($"Total Mass: {_totalMass:F1} tons");
            ImGui.Text($"Next Block Cost: {_materialCost} {_selectedMaterial}");
            
            // Additional stats from voxel structure
            if (_currentShipId != Guid.Empty)
            {
                var structure = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(_currentShipId);
                if (structure != null)
                {
                    ImGui.Separator();
                    ImGui.Text($"Hull: {structure.StructuralIntegrity:F0}");
                    ImGui.Text($"Volume: {_totalMass / 100f:F1} m³");
                    
                    ImGui.Separator();
                    ImGui.Text($"Power: {structure.PowerGeneration:F1} MW");
                    ImGui.Text($"Shields: {structure.ShieldCapacity:F0}");
                    ImGui.Text($"Thrust: {structure.TotalThrust:F0} N");
                    ImGui.Text($"Torque: {structure.TotalTorque:F0} Nm");
                }
            }
        }
        ImGui.End();
    }
    
    /// <summary>
    /// Render dialogs (blueprints)
    /// </summary>
    private void RenderDialogs()
    {
        if (_showSaveBlueprintDialog)
            RenderSaveBlueprintDialog();
        
        if (_showLoadBlueprintDialog)
            RenderLoadBlueprintDialog();
        
        if (!_isOpen && _buildModeActive)
        {
            Close();
        }
    }
    
    /// <summary>
    /// Handle keyboard input
    /// </summary>
    public void HandleInput()
    {
        // Toggle ship builder with 'B' key
        if (ImGui.IsKeyPressed(ImGuiKey.B))
        {
            Toggle();
        }
    }
    
    /// <summary>
    /// Place a block at the current position
    /// </summary>
    private void PlaceBlock()
    {
        if (!_buildModeActive || _currentShipId == Guid.Empty)
        {
            SetStatusMessage("Build mode not active", 2f);
            return;
        }
        
        // Get inventory component
        var inventory = _gameEngine.EntityManager.GetComponent<InventoryComponent>(_currentShipId);
        if (inventory == null)
        {
            SetStatusMessage("Ship has no inventory", 2f);
            return;
        }
        
        // Attempt to place block
        var result = _buildSystem.PlaceBlock(_currentShipId, _placementPosition, inventory.Inventory);
        
        if (result.Success)
        {
            SetStatusMessage($"Block placed! {result.Message}", 2f);
            UpdateStatistics();
            
            // Move placement position for next block (smart positioning)
            if (_gridSnap)
            {
                _placementPosition.X += _blockSize.X + _gridSize;
            }
            else
            {
                _placementPosition.X += _blockSize.X;
            }
        }
        else
        {
            SetStatusMessage($"Failed: {result.Message}", 3f);
        }
    }
    
    /// <summary>
    /// Remove a block at the current position
    /// </summary>
    private void RemoveBlock()
    {
        if (!_buildModeActive || _currentShipId == Guid.Empty)
        {
            SetStatusMessage("Build mode not active", 2f);
            return;
        }
        
        // Get inventory component
        var inventory = _gameEngine.EntityManager.GetComponent<InventoryComponent>(_currentShipId);
        if (inventory == null)
        {
            SetStatusMessage("Ship has no inventory", 2f);
            return;
        }
        
        // Attempt to remove block
        var result = _buildSystem.RemoveBlock(_currentShipId, _placementPosition, 1f, inventory.Inventory);
        
        if (result.Success)
        {
            SetStatusMessage($"Block removed! {result.Message}", 2f);
            UpdateStatistics();
        }
        else
        {
            SetStatusMessage($"Failed: {result.Message}", 3f);
        }
    }
    
    /// <summary>
    /// Update ship statistics display
    /// </summary>
    private void UpdateStatistics()
    {
        if (_currentShipId == Guid.Empty) return;
        
        var structure = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(_currentShipId);
        if (structure != null)
        {
            _totalBlocks = structure.Blocks.Count;
            _totalMass = structure.TotalMass;
        }
        
        UpdateMaterialCost();
    }
    
    /// <summary>
    /// Update material cost for current block size
    /// </summary>
    private void UpdateMaterialCost()
    {
        float volume = _blockSize.X * _blockSize.Y * _blockSize.Z;
        _materialCost = (int)(volume * 10); // 10 units per cubic unit
    }
    
    /// <summary>
    /// Snap position to grid
    /// </summary>
    private Vector3 SnapToGrid(Vector3 position, int gridSize)
    {
        return new Vector3(
            MathF.Round(position.X / gridSize) * gridSize,
            MathF.Round(position.Y / gridSize) * gridSize,
            MathF.Round(position.Z / gridSize) * gridSize
        );
    }
    
    /// <summary>
    /// Set a status message with timer
    /// </summary>
    private void SetStatusMessage(string message, float duration)
    {
        _statusMessage = message;
        _statusMessageTimer = duration;
    }
    
    /// <summary>
    /// Get description for block type
    /// </summary>
    private string GetBlockTypeDescription(BlockType blockType)
    {
        return blockType switch
        {
            BlockType.Hull => "Basic structure",
            BlockType.Armor => "Heavy protection",
            BlockType.Engine => "Main thrust",
            BlockType.Thruster => "Maneuvering",
            BlockType.GyroArray => "Rotation control",
            BlockType.Generator => "Power generation",
            BlockType.ShieldGenerator => "Shield capacity",
            BlockType.TurretMount => "Weapon mount",
            BlockType.HyperdriveCore => "FTL travel",
            BlockType.Cargo => "Storage space",
            BlockType.CrewQuarters => "Crew housing",
            BlockType.PodDocking => "Pod docking",
            _ => ""
        };
    }
    
    private void RenderSaveBlueprintDialog()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), ImGuiCond.Always, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(400, 180), ImGuiCond.Always);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags.NoResize | 
                                       ImGuiWindowFlags.NoCollapse |
                                       ImGuiWindowFlags.NoMove;
        
        if (ImGui.Begin("Save Blueprint", ref _showSaveBlueprintDialog, windowFlags))
        {
            if (_currentShipId == Guid.Empty || !_gameEngine.EntityManager.HasComponent<VoxelStructureComponent>(_currentShipId))
            {
                ImGui.Text("No ship to save! Build a ship first.");
                ImGui.Dummy(new Vector2(0, 20));
                
                ImGui.SetCursorPosX(150);
                if (ImGui.Button("OK", new Vector2(100, 40)))
                {
                    _showSaveBlueprintDialog = false;
                }
            }
            else
            {
                ImGui.Text("Enter blueprint name:");
                ImGui.SetNextItemWidth(360);
                ImGui.InputText("##blueprintname", ref _blueprintName, 100);
                
                ImGui.Dummy(new Vector2(0, 20));
                
                ImGui.SetCursorPosX(80);
                if (ImGui.Button("Save", new Vector2(100, 40)))
                {
                    if (!string.IsNullOrWhiteSpace(_blueprintName))
                    {
                        var structure = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(_currentShipId);
                        if (structure != null)
                        {
                            var blueprint = ShipBlueprint.FromVoxelStructure(_blueprintName, structure);
                            var filePath = Path.Combine(ShipBlueprint.GetBlueprintsDirectory(), $"{_blueprintName}.blueprint");
                            
                            if (blueprint.SaveToFile(filePath))
                            {
                                SetStatusMessage($"Blueprint '{_blueprintName}' saved!", 3f);
                                _showSaveBlueprintDialog = false;
                            }
                            else
                            {
                                SetStatusMessage("Failed to save blueprint!", 3f);
                            }
                        }
                    }
                }
                
                ImGui.SameLine();
                ImGui.SetCursorPosX(220);
                if (ImGui.Button("Cancel", new Vector2(100, 40)))
                {
                    _showSaveBlueprintDialog = false;
                }
            }
            
            ImGui.End();
        }
    }
    
    private void RenderLoadBlueprintDialog()
    {
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), ImGuiCond.Always, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(400, 400), ImGuiCond.Always);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags.NoResize | 
                                       ImGuiWindowFlags.NoCollapse |
                                       ImGuiWindowFlags.NoMove;
        
        if (ImGui.Begin("Load Blueprint", ref _showLoadBlueprintDialog, windowFlags))
        {
            ImGui.Text("Select a blueprint to load:");
            ImGui.Dummy(new Vector2(0, 10));
            
            if (_availableBlueprints.Count == 0)
            {
                ImGui.Text("No blueprints found.");
            }
            else
            {
                if (ImGui.BeginChild("BlueprintsList", new Vector2(360, 250)))
                {
                    for (int i = 0; i < _availableBlueprints.Count; i++)
                    {
                        bool isSelected = (_selectedBlueprintIndex == i);
                        if (ImGui.Selectable(_availableBlueprints[i], isSelected))
                        {
                            _selectedBlueprintIndex = i;
                        }
                    }
                    
                    ImGui.EndChild();
                }
            }
            
            ImGui.Dummy(new Vector2(0, 10));
            
            ImGui.SetCursorPosX(80);
            bool canLoad = _availableBlueprints.Count > 0 && _selectedBlueprintIndex >= 0 && _selectedBlueprintIndex < _availableBlueprints.Count;
            
            if (!canLoad)
                ImGui.BeginDisabled();
                
            if (ImGui.Button("Load", new Vector2(100, 40)))
            {
                if (canLoad && _currentShipId != Guid.Empty)
                {
                    var blueprintName = _availableBlueprints[_selectedBlueprintIndex];
                    var filePath = Path.Combine(ShipBlueprint.GetBlueprintsDirectory(), $"{blueprintName}.blueprint");
                    var blueprint = ShipBlueprint.LoadFromFile(filePath);
                    
                    if (blueprint != null)
                    {
                        var structure = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(_currentShipId);
                        if (structure != null)
                        {
                            blueprint.ApplyToVoxelStructure(structure);
                            UpdateStatistics();
                            SetStatusMessage($"Blueprint '{blueprintName}' loaded!", 3f);
                            _showLoadBlueprintDialog = false;
                        }
                    }
                    else
                    {
                        SetStatusMessage("Failed to load blueprint!", 3f);
                    }
                }
            }
            
            if (!canLoad)
                ImGui.EndDisabled();
            
            ImGui.SameLine();
            ImGui.SetCursorPosX(220);
            if (ImGui.Button("Cancel", new Vector2(100, 40)))
            {
                _showLoadBlueprintDialog = false;
            }
            
            ImGui.End();
        }
    }
}
