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
    
    // Statistics
    private int _totalBlocks = 0;
    private float _totalMass = 0f;
    private int _materialCost = 0;
    
    // Material colors for UI
    private readonly Dictionary<string, Vector4> _materialColors = new()
    {
        ["Iron"] = new Vector4(0.5f, 0.5f, 0.5f, 1.0f),
        ["Titanium"] = new Vector4(0.25f, 0.41f, 0.88f, 1.0f),
        ["Naonite"] = new Vector4(0.2f, 0.8f, 0.2f, 1.0f),
        ["Trinium"] = new Vector4(0.0f, 0.8f, 0.82f, 1.0f),
        ["Xanion"] = new Vector4(1.0f, 0.84f, 0.0f, 1.0f),
        ["Ogonite"] = new Vector4(1.0f, 0.27f, 0.0f, 1.0f),
        ["Avorion"] = new Vector4(0.58f, 0.44f, 0.86f, 1.0f)
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
    /// Render the ship builder UI
    /// </summary>
    public void Render()
    {
        if (!_isOpen) return;
        
        // Update status message timer
        if (_statusMessageTimer > 0)
        {
            _statusMessageTimer -= ImGui.GetIO().DeltaTime;
        }
        
        // Main ship builder window
        ImGui.SetNextWindowSize(new Vector2(400, 600), ImGuiCond.FirstUseEver);
        if (ImGui.Begin("Ship Builder", ref _isOpen, ImGuiWindowFlags.None))
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
            
            // Material selection
            ImGui.Text("Material:");
            foreach (var material in MaterialProperties.Materials.Keys)
            {
                var color = _materialColors.GetValueOrDefault(material, new Vector4(1, 1, 1, 1));
                ImGui.PushStyleColor(ImGuiCol.Text, color);
                
                if (ImGui.RadioButton(material, _selectedMaterial == material))
                {
                    _selectedMaterial = material;
                    if (_buildModeActive)
                    {
                        _buildSystem.SetSelectedMaterial(_currentShipId, material);
                    }
                    UpdateMaterialCost();
                }
                
                ImGui.PopStyleColor();
                
                // Show material stats
                var props = MaterialProperties.GetMaterial(material);
                ImGui.SameLine();
                ImGui.TextDisabled($"(Tech {props.TechLevel}, Dur: {props.DurabilityMultiplier:F1}x)");
            }
            
            ImGui.Separator();
            
            // Block type selection
            ImGui.Text("Block Type:");
            var blockTypes = Enum.GetValues<BlockType>();
            foreach (var blockType in blockTypes)
            {
                if (ImGui.RadioButton(blockType.ToString(), _selectedBlockType == blockType))
                {
                    _selectedBlockType = blockType;
                    if (_buildModeActive)
                    {
                        _buildSystem.SetSelectedBlockType(_currentShipId, blockType);
                    }
                }
                
                // Add descriptions
                var description = GetBlockTypeDescription(blockType);
                if (!string.IsNullOrEmpty(description))
                {
                    ImGui.SameLine();
                    ImGui.TextDisabled($"({description})");
                }
            }
            
            ImGui.Separator();
            
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
            
            // Grid settings
            ImGui.Checkbox("Grid Snap", ref _gridSnap);
            ImGui.SameLine();
            ImGui.Checkbox("Show Grid", ref _showGrid);
            
            if (_gridSnap)
            {
                ImGui.SliderInt("Grid Size", ref _gridSize, 1, 10);
            }
            
            ImGui.Separator();
            
            // Statistics
            ImGui.Text("Ship Statistics:");
            ImGui.Indent();
            ImGui.Text($"Total Blocks: {_totalBlocks}");
            ImGui.Text($"Total Mass: {_totalMass:F1} tons");
            ImGui.Text($"Next Block Cost: {_materialCost} {_selectedMaterial}");
            ImGui.Unindent();
            
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
            if (ImGui.Button("Center View", new Vector2(-1, 25)))
            {
                // Signal to center camera on ship (handled by GraphicsWindow)
                SetStatusMessage("Center view not yet implemented", 2f);
            }
            
            if (ImGui.Button("Save Blueprint", new Vector2(-1, 25)))
            {
                // TODO: Implement blueprint saving
                SetStatusMessage("Blueprint saving coming soon", 2f);
            }
            
            if (ImGui.Button("Load Blueprint", new Vector2(-1, 25)))
            {
                // TODO: Implement blueprint loading
                SetStatusMessage("Blueprint loading coming soon", 2f);
            }
            
            ImGui.Separator();
            
            // Help text
            ImGui.TextDisabled("Controls:");
            ImGui.TextDisabled("  B - Toggle Builder");
            ImGui.TextDisabled("  Mouse - Move placement cursor");
            ImGui.TextDisabled("  Click - Place/Remove block");
            ImGui.TextDisabled("  Shift+Click - Multi-place");
        }
        ImGui.End();
        
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
}
