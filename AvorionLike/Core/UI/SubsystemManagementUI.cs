using System.Numerics;
using ImGuiNET;
using AvorionLike.Core.ECS;
using AvorionLike.Core.RPG;
using AvorionLike.Core.Fleet;

namespace AvorionLike.Core.UI;

/// <summary>
/// UI for managing ship and pod subsystems
/// </summary>
public class SubsystemManagementUI
{
    private readonly GameEngine _gameEngine;
    private bool _isOpen = false;
    private Guid? _selectedEntityId = null;
    private bool _isPodMode = false;
    
    // Selected subsystem for inspection/upgrade
    private SubsystemUpgrade? _selectedSubsystem = null;
    private int _selectedSlotIndex = -1;
    
    public bool IsOpen => _isOpen;
    
    public SubsystemManagementUI(GameEngine gameEngine)
    {
        _gameEngine = gameEngine;
    }
    
    public void Show(Guid? entityId = null)
    {
        _isOpen = true;
        _selectedEntityId = entityId;
        
        // Detect if this is a pod or ship
        if (entityId.HasValue)
        {
            var podSubsystem = _gameEngine.EntityManager.GetComponent<PodSubsystemComponent>(entityId.Value);
            _isPodMode = podSubsystem != null;
        }
    }
    
    public void Hide()
    {
        _isOpen = false;
        _selectedEntityId = null;
        _selectedSubsystem = null;
        _selectedSlotIndex = -1;
    }
    
    public void Toggle()
    {
        if (_isOpen)
            Hide();
        else
            Show();
    }
    
    public void Render()
    {
        if (!_isOpen) return;
        
        var io = ImGui.GetIO();
        ImGui.SetNextWindowPos(new Vector2(io.DisplaySize.X * 0.5f, io.DisplaySize.Y * 0.5f), 
            ImGuiCond.FirstUseEver, new Vector2(0.5f, 0.5f));
        ImGui.SetNextWindowSize(new Vector2(900, 600), ImGuiCond.FirstUseEver);
        
        if (ImGui.Begin("Subsystem Management", ref _isOpen))
        {
            RenderEntitySelector();
            ImGui.Separator();
            
            if (_selectedEntityId.HasValue)
            {
                // Create two columns: left for subsystem slots, right for inventory/details
                if (ImGui.BeginTable("SubsystemTable", 2, ImGuiTableFlags.Resizable | ImGuiTableFlags.BordersInnerV))
                {
                    ImGui.TableSetupColumn("Equipped", ImGuiTableColumnFlags.WidthFixed, 400);
                    ImGui.TableSetupColumn("Storage & Details", ImGuiTableColumnFlags.WidthStretch);
                    
                    ImGui.TableNextRow();
                    
                    // Left column: Equipped subsystems
                    ImGui.TableSetColumnIndex(0);
                    RenderEquippedSubsystems();
                    
                    // Right column: Storage and details
                    ImGui.TableSetColumnIndex(1);
                    RenderSubsystemStorage();
                    
                    ImGui.EndTable();
                }
            }
            else
            {
                ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1.0f), 
                    "Select a ship or pod to manage subsystems.");
            }
        }
        ImGui.End();
    }
    
    private void RenderEntitySelector()
    {
        ImGui.Text("Select Ship/Pod:");
        ImGui.SameLine();
        
        // Get all entities with subsystems
        var entities = _gameEngine.EntityManager.GetAllEntities()
            .Where(e => _gameEngine.EntityManager.GetComponent<ShipSubsystemComponent>(e.Id) != null ||
                       _gameEngine.EntityManager.GetComponent<PodSubsystemComponent>(e.Id) != null)
            .ToList();
        
        if (entities.Count == 0)
        {
            ImGui.TextColored(new Vector4(1f, 0.5f, 0.2f, 1f), "No ships or pods with subsystem slots found.");
            return;
        }
        
        string currentLabel = _selectedEntityId.HasValue 
            ? GetEntityLabel(_selectedEntityId.Value) 
            : "Select...";
        
        if (ImGui.BeginCombo("##EntitySelector", currentLabel))
        {
            foreach (var entity in entities)
            {
                bool isSelected = _selectedEntityId == entity.Id;
                string label = GetEntityLabel(entity.Id);
                
                if (ImGui.Selectable(label, isSelected))
                {
                    _selectedEntityId = entity.Id;
                    
                    // Update pod mode
                    var podSubsystem = _gameEngine.EntityManager.GetComponent<PodSubsystemComponent>(entity.Id);
                    _isPodMode = podSubsystem != null;
                }
                
                if (isSelected)
                    ImGui.SetItemDefaultFocus();
            }
            ImGui.EndCombo();
        }
    }
    
    private string GetEntityLabel(Guid entityId)
    {
        var entity = _gameEngine.EntityManager.GetEntity(entityId);
        if (entity == null) return "Unknown";
        
        var podComponent = _gameEngine.EntityManager.GetComponent<PlayerPodComponent>(entityId);
        if (podComponent != null)
        {
            return $"[POD] {entity.Name}";
        }
        
        var shipClass = _gameEngine.EntityManager.GetComponent<ShipClassComponent>(entityId);
        if (shipClass != null)
        {
            return $"[{shipClass.ShipClass}] {entity.Name}";
        }
        
        return entity.Name;
    }
    
    private void RenderEquippedSubsystems()
    {
        if (!_selectedEntityId.HasValue) return;
        
        ImGui.TextColored(new Vector4(0.4f, 0.8f, 1f, 1f), "Equipped Subsystems");
        ImGui.Separator();
        
        if (_isPodMode)
        {
            var podSubsystem = _gameEngine.EntityManager.GetComponent<PodSubsystemComponent>(_selectedEntityId.Value);
            if (podSubsystem != null)
            {
                RenderSubsystemSlots(podSubsystem);
            }
        }
        else
        {
            var shipSubsystem = _gameEngine.EntityManager.GetComponent<ShipSubsystemComponent>(_selectedEntityId.Value);
            if (shipSubsystem != null)
            {
                RenderSubsystemSlots(shipSubsystem);
            }
        }
    }
    
    private void RenderSubsystemSlots(ShipSubsystemComponent shipSubsystem)
    {
        ImGui.Text($"Slots: {shipSubsystem.EquippedSubsystems.Count(s => s != null)} / {shipSubsystem.MaxSubsystemSlots}");
        ImGui.Spacing();
        
        for (int i = 0; i < shipSubsystem.MaxSubsystemSlots; i++)
        {
            var subsystem = shipSubsystem.EquippedSubsystems[i];
            RenderSubsystemSlot(i, subsystem, () => shipSubsystem.UnequipSubsystem(i));
        }
    }
    
    private void RenderSubsystemSlots(PodSubsystemComponent podSubsystem)
    {
        ImGui.Text($"Slots: {podSubsystem.EquippedSubsystems.Count(s => s != null)} / {podSubsystem.MaxSubsystemSlots}");
        ImGui.Spacing();
        
        for (int i = 0; i < podSubsystem.MaxSubsystemSlots; i++)
        {
            var subsystem = podSubsystem.EquippedSubsystems[i];
            RenderSubsystemSlot(i, subsystem, () => podSubsystem.UnequipSubsystem(i));
        }
    }
    
    private void RenderSubsystemSlot(int slotIndex, SubsystemUpgrade? subsystem, Func<SubsystemUpgrade?> unequipAction)
    {
        ImGui.PushID($"slot_{slotIndex}");
        
        if (subsystem == null)
        {
            // Empty slot
            ImGui.BeginDisabled();
            ImGui.Button($"[EMPTY SLOT {slotIndex + 1}]", new Vector2(-1, 40));
            ImGui.EndDisabled();
        }
        else
        {
            // Filled slot - show subsystem
            var color = GetRarityColor(subsystem.Rarity);
            ImGui.PushStyleColor(ImGuiCol.Button, color);
            ImGui.PushStyleColor(ImGuiCol.ButtonHovered, color * 1.2f);
            ImGui.PushStyleColor(ImGuiCol.ButtonActive, color * 0.8f);
            
            if (ImGui.Button($"{subsystem.Name}##slot{slotIndex}", new Vector2(-200, 40)))
            {
                _selectedSubsystem = subsystem;
                _selectedSlotIndex = slotIndex;
            }
            
            ImGui.PopStyleColor(3);
            
            // Unequip button
            ImGui.SameLine();
            if (ImGui.Button($"Unequip##unequip{slotIndex}", new Vector2(90, 40)))
            {
                var unequipped = unequipAction();
                if (unequipped != null)
                {
                    // Add back to storage
                    var storage = _gameEngine.EntityManager.GetComponent<SubsystemInventoryComponent>(_selectedEntityId.Value);
                    storage?.AddSubsystem(unequipped);
                }
            }
            
            // Show upgrade level
            if (subsystem.UpgradeLevel > 0)
            {
                ImGui.SameLine();
                ImGui.TextColored(new Vector4(1f, 0.8f, 0f, 1f), $"+{subsystem.UpgradeLevel}");
            }
            
            // Show bonus on next line
            ImGui.Indent();
            ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1f), 
                $"Bonus: +{subsystem.GetTotalBonus() * 100f:F1}%");
            ImGui.Unindent();
        }
        
        ImGui.PopID();
        ImGui.Spacing();
    }
    
    private void RenderSubsystemStorage()
    {
        if (!_selectedEntityId.HasValue) return;
        
        ImGui.TextColored(new Vector4(0.4f, 0.8f, 1f, 1f), "Subsystem Storage");
        ImGui.Separator();
        
        var storage = _gameEngine.EntityManager.GetComponent<SubsystemInventoryComponent>(_selectedEntityId.Value);
        if (storage == null)
        {
            ImGui.TextColored(new Vector4(1f, 0.5f, 0.2f, 1f), "No subsystem storage available.");
            return;
        }
        
        ImGui.Text($"Stored: {storage.StoredSubsystems.Count} / {storage.MaxStorage}");
        ImGui.Spacing();
        
        // Filter options
        ImGui.Text("Filter:");
        ImGui.SameLine();
        
        if (ImGui.BeginCombo("##RarityFilter", "All Rarities"))
        {
            // TODO: Add filtering
            ImGui.EndCombo();
        }
        
        ImGui.Separator();
        
        if (ImGui.BeginChild("StorageList", new Vector2(0, -100)))
        {
            foreach (var subsystem in storage.StoredSubsystems)
            {
                ImGui.PushID(subsystem.Id.ToString());
                
                var color = GetRarityColor(subsystem.Rarity);
                ImGui.PushStyleColor(ImGuiCol.Button, color);
                ImGui.PushStyleColor(ImGuiCol.ButtonHovered, color * 1.2f);
                ImGui.PushStyleColor(ImGuiCol.ButtonActive, color * 0.8f);
                
                if (ImGui.Button(subsystem.Name, new Vector2(-100, 30)))
                {
                    _selectedSubsystem = subsystem;
                    _selectedSlotIndex = -1; // Not equipped
                }
                
                ImGui.PopStyleColor(3);
                
                ImGui.SameLine();
                if (ImGui.Button("Equip", new Vector2(90, 30)))
                {
                    EquipSubsystemFromStorage(subsystem);
                }
                
                // Show details
                ImGui.Indent();
                ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1f), 
                    $"{subsystem.Type} | +{subsystem.GetTotalBonus() * 100f:F1}%");
                ImGui.Unindent();
                
                ImGui.PopID();
                ImGui.Spacing();
            }
        }
        ImGui.EndChild();
        
        // Selected subsystem details
        if (_selectedSubsystem != null)
        {
            ImGui.Separator();
            RenderSubsystemDetails(_selectedSubsystem);
        }
    }
    
    private void EquipSubsystemFromStorage(SubsystemUpgrade subsystem)
    {
        if (!_selectedEntityId.HasValue) return;
        
        bool equipped = false;
        int freeSlot = -1;
        
        if (_isPodMode)
        {
            var podSubsystem = _gameEngine.EntityManager.GetComponent<PodSubsystemComponent>(_selectedEntityId.Value);
            if (podSubsystem != null)
            {
                // Find first free slot
                for (int i = 0; i < podSubsystem.MaxSubsystemSlots; i++)
                {
                    if (podSubsystem.EquippedSubsystems[i] == null)
                    {
                        freeSlot = i;
                        break;
                    }
                }
                
                if (freeSlot >= 0)
                {
                    equipped = podSubsystem.EquipSubsystem(subsystem, freeSlot);
                }
            }
        }
        else
        {
            var shipSubsystem = _gameEngine.EntityManager.GetComponent<ShipSubsystemComponent>(_selectedEntityId.Value);
            if (shipSubsystem != null)
            {
                // Find first free slot
                for (int i = 0; i < shipSubsystem.MaxSubsystemSlots; i++)
                {
                    if (shipSubsystem.EquippedSubsystems[i] == null)
                    {
                        freeSlot = i;
                        break;
                    }
                }
                
                if (freeSlot >= 0)
                {
                    equipped = shipSubsystem.EquipSubsystem(subsystem, freeSlot);
                }
            }
        }
        
        if (equipped)
        {
            // Remove from storage
            var storage = _gameEngine.EntityManager.GetComponent<SubsystemInventoryComponent>(_selectedEntityId.Value);
            storage?.RemoveSubsystem(subsystem.Id);
        }
    }
    
    private void RenderSubsystemDetails(SubsystemUpgrade subsystem)
    {
        ImGui.TextColored(new Vector4(1f, 1f, 0.4f, 1f), "Subsystem Details");
        
        var rarityColor = GetRarityColor(subsystem.Rarity);
        ImGui.TextColored(rarityColor, subsystem.Name);
        ImGui.Text($"Type: {subsystem.Type}");
        ImGui.Text($"Rarity: {subsystem.Rarity}");
        ImGui.Text($"Quality: {subsystem.Quality}");
        ImGui.Text($"Level: {subsystem.UpgradeLevel} / {(int)subsystem.Quality}");
        ImGui.Text($"Total Bonus: +{subsystem.GetTotalBonus() * 100f:F1}%");
        ImGui.TextWrapped($"Description: {subsystem.Description}");
        
        ImGui.Spacing();
        
        // Upgrade button
        if (subsystem.CanUpgrade())
        {
            var upgradeCost = subsystem.GetUpgradeCost();
            
            if (ImGui.Button($"Upgrade to +{subsystem.UpgradeLevel + 1}", new Vector2(-1, 30)))
            {
                // TODO: Check if player has resources and perform upgrade
                if (subsystem.Upgrade())
                {
                    // Success
                }
            }
            
            ImGui.Text("Cost:");
            foreach (var cost in upgradeCost)
            {
                ImGui.BulletText($"{cost.Value} {cost.Key}");
            }
        }
        else
        {
            ImGui.TextColored(new Vector4(0.7f, 0.7f, 0.7f, 1f), "Max upgrade level reached");
        }
    }
    
    private Vector4 GetRarityColor(SubsystemRarity rarity)
    {
        return rarity switch
        {
            SubsystemRarity.Common => new Vector4(0.7f, 0.7f, 0.7f, 1f),       // Gray
            SubsystemRarity.Uncommon => new Vector4(0.2f, 0.8f, 0.2f, 1f),     // Green
            SubsystemRarity.Rare => new Vector4(0.2f, 0.5f, 1f, 1f),           // Blue
            SubsystemRarity.Epic => new Vector4(0.8f, 0.2f, 0.8f, 1f),         // Purple
            SubsystemRarity.Legendary => new Vector4(1f, 0.6f, 0.1f, 1f),      // Orange
            _ => new Vector4(1f, 1f, 1f, 1f)
        };
    }
    
    public void HandleInput()
    {
        // Toggle with U key (for Upgrades/sUbsystems)
        if (ImGui.IsKeyPressed(ImGuiKey.U))
        {
            Toggle();
        }
    }
}
