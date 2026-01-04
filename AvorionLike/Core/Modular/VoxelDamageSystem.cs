using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Component that holds voxel damage visualization data for modular ships
/// Voxels are used ONLY for showing damage, not for ship construction
/// </summary>
public class VoxelDamageComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    /// <summary>
    /// Voxel blocks used to visualize damage on ship modules
    /// These are overlays on top of the actual 3D models
    /// </summary>
    public List<VoxelBlock> DamageVoxels { get; set; } = new();
    
    /// <summary>
    /// Maps module ID to its damage voxels
    /// </summary>
    public Dictionary<Guid, List<VoxelBlock>> ModuleDamageMap { get; set; } = new();
    
    /// <summary>
    /// Whether damage visualization is enabled
    /// </summary>
    public bool ShowDamage { get; set; } = true;
}

/// <summary>
/// System that generates and updates voxel-based damage visualization for modular ships
/// Voxels are used to show damage on ship modules by creating "broken" or "missing" sections
/// </summary>
public class VoxelDamageSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly Logger _logger = Logger.Instance;
    
    public VoxelDamageSystem(EntityManager entityManager) : base("VoxelDamageSystem")
    {
        _entityManager = entityManager;
    }
    
    public override void Update(float deltaTime)
    {
        // Update damage visualization for all modular ships
        var ships = _entityManager.GetEntitiesWithComponents(typeof(ModularShipComponent));
        
        foreach (var shipId in ships)
        {
            UpdateShipDamageVisualization(shipId);
        }
    }
    
    /// <summary>
    /// Update damage visualization for a ship
    /// </summary>
    private void UpdateShipDamageVisualization(Guid shipId)
    {
        var modularShip = _entityManager.GetComponent<ModularShipComponent>(shipId);
        if (modularShip == null) return;
        
        var damageComponent = _entityManager.GetComponent<VoxelDamageComponent>(shipId);
        if (damageComponent == null)
        {
            // Create damage component if it doesn't exist
            damageComponent = new VoxelDamageComponent { EntityId = shipId };
            _entityManager.AddComponent(shipId, damageComponent);
        }
        
        if (!damageComponent.ShowDamage) return;
        
        // Update damage voxels for each damaged module
        foreach (var module in modularShip.Modules)
        {
            if (module.DamageLevel > 0.1f) // Only show damage if > 10%
            {
                UpdateModuleDamageVoxels(module, damageComponent);
            }
        }
    }
    
    /// <summary>
    /// Generate or update damage voxels for a specific module
    /// </summary>
    private void UpdateModuleDamageVoxels(ShipModulePart module, VoxelDamageComponent damageComponent)
    {
        // Remove old damage voxels for this module
        if (damageComponent.ModuleDamageMap.ContainsKey(module.Id))
        {
            var oldVoxels = damageComponent.ModuleDamageMap[module.Id];
            foreach (var voxel in oldVoxels)
            {
                damageComponent.DamageVoxels.Remove(voxel);
            }
            damageComponent.ModuleDamageMap.Remove(module.Id);
        }
        
        // Generate new damage voxels based on damage level
        var damageVoxels = GenerateDamageVoxelsForModule(module);
        damageComponent.ModuleDamageMap[module.Id] = damageVoxels;
        damageComponent.DamageVoxels.AddRange(damageVoxels);
    }
    
    /// <summary>
    /// Generate voxel blocks that represent damage on a module
    /// Creates "holes" and "broken sections" using destroyed/damaged voxels
    /// </summary>
    private List<VoxelBlock> GenerateDamageVoxelsForModule(ShipModulePart module)
    {
        var damageVoxels = new List<VoxelBlock>();
        
        // Don't create damage voxels for completely destroyed modules
        if (module.IsDestroyed) return damageVoxels;
        
        // Create a grid of voxels over the module's bounding box
        // The number of voxels depends on the module's size
        int voxelsX = Math.Max(2, (int)(module.Scale.X * 2));
        int voxelsY = Math.Max(2, (int)(module.Scale.Y * 2));
        int voxelsZ = Math.Max(2, (int)(module.Scale.Z * 2));
        
        float voxelSizeX = module.Scale.X / voxelsX;
        float voxelSizeY = module.Scale.Y / voxelsY;
        float voxelSizeZ = module.Scale.Z / voxelsZ;
        
        var random = new Random(module.Id.GetHashCode());
        
        // Generate voxels based on damage level
        for (int x = 0; x < voxelsX; x++)
        {
            for (int y = 0; y < voxelsY; y++)
            {
                for (int z = 0; z < voxelsZ; z++)
                {
                    // Randomly remove voxels based on damage level
                    if (random.NextDouble() < module.DamageLevel)
                    {
                        // Calculate voxel position relative to module
                        Vector3 localPos = new Vector3(
                            (x - voxelsX / 2f) * voxelSizeX,
                            (y - voxelsY / 2f) * voxelSizeY,
                            (z - voxelsZ / 2f) * voxelSizeZ
                        );
                        
                        Vector3 worldPos = module.Position + localPos;
                        Vector3 voxelSize = new Vector3(voxelSizeX, voxelSizeY, voxelSizeZ);
                        
                        // Create a damaged/destroyed voxel
                        var voxel = new VoxelBlock(
                            worldPos, 
                            voxelSize, 
                            module.MaterialType, 
                            BlockType.Hull,
                            BlockShape.Cube,
                            BlockOrientation.PosY
                        );
                        
                        // Mark as destroyed to show as "missing" or "damaged"
                        voxel.TakeDamage(voxel.MaxDurability);
                        
                        damageVoxels.Add(voxel);
                    }
                }
            }
        }
        
        return damageVoxels;
    }
    
    /// <summary>
    /// Apply damage to a ship module and update visualization
    /// </summary>
    public void ApplyDamageToModule(Guid shipId, Guid moduleId, float damage)
    {
        var modularShip = _entityManager.GetComponent<ModularShipComponent>(shipId);
        if (modularShip == null) return;
        
        var module = modularShip.GetModule(moduleId);
        if (module == null) return;
        
        // Apply damage to module
        float oldHealth = module.Health;
        module.TakeDamage(damage);
        
        _logger.Info("VoxelDamageSystem", 
            $"Module {moduleId} took {damage} damage: {oldHealth} -> {module.Health}");
        
        // Update damage visualization
        var damageComponent = _entityManager.GetComponent<VoxelDamageComponent>(shipId);
        if (damageComponent != null)
        {
            UpdateModuleDamageVoxels(module, damageComponent);
        }
        
        // If module is destroyed, check for detached modules
        if (module.IsDestroyed)
        {
            HandleDestroyedModule(shipId, moduleId, modularShip);
        }
        
        // Recalculate ship stats
        modularShip.RecalculateStats();
    }
    
    /// <summary>
    /// Handle a destroyed module - detach connected modules if necessary
    /// </summary>
    private void HandleDestroyedModule(Guid shipId, Guid moduleId, ModularShipComponent ship)
    {
        _logger.Warning("VoxelDamageSystem", $"Module {moduleId} destroyed!");
        
        // Check if this was a critical module (core/cockpit)
        if (ship.CoreModuleId == moduleId)
        {
            _logger.Critical("VoxelDamageSystem", "Core module destroyed! Ship is lost!");
        }
        
        // Optionally detach modules that were connected to this one
        var module = ship.GetModule(moduleId);
        if (module != null)
        {
            foreach (var attachedId in module.AttachedModules)
            {
                var attached = ship.GetModule(attachedId);
                if (attached != null)
                {
                    attached.AttachedToModules.Remove(moduleId);
                    
                    // Check if the attached module is still connected to the ship
                    if (!IsModuleConnectedToCore(attached, ship))
                    {
                        _logger.Warning("VoxelDamageSystem", 
                            $"Module {attachedId} detached from ship due to destroyed connection!");
                        // Module is floating free - could spawn it as debris
                    }
                }
            }
        }
    }
    
    /// <summary>
    /// Check if a module is still connected to the ship's core
    /// </summary>
    private bool IsModuleConnectedToCore(ShipModulePart module, ModularShipComponent ship)
    {
        if (!ship.CoreModuleId.HasValue) return true;
        if (module.Id == ship.CoreModuleId.Value) return true;
        
        // BFS to find path to core
        var visited = new HashSet<Guid>();
        var queue = new Queue<Guid>();
        queue.Enqueue(module.Id);
        visited.Add(module.Id);
        
        while (queue.Count > 0)
        {
            var currentId = queue.Dequeue();
            if (currentId == ship.CoreModuleId.Value) return true;
            
            var current = ship.GetModule(currentId);
            if (current == null || current.IsDestroyed) continue;
            
            foreach (var connectedId in current.AttachedToModules.Concat(current.AttachedModules))
            {
                if (!visited.Contains(connectedId))
                {
                    visited.Add(connectedId);
                    queue.Enqueue(connectedId);
                }
            }
        }
        
        return false;
    }
    
    /// <summary>
    /// Clear all damage visualization for a ship
    /// </summary>
    public void ClearDamageVisualization(Guid shipId)
    {
        var damageComponent = _entityManager.GetComponent<VoxelDamageComponent>(shipId);
        if (damageComponent != null)
        {
            damageComponent.DamageVoxels.Clear();
            damageComponent.ModuleDamageMap.Clear();
        }
    }
    
    /// <summary>
    /// Repair a module and update damage visualization
    /// </summary>
    public void RepairModule(Guid shipId, Guid moduleId, float repairAmount)
    {
        var modularShip = _entityManager.GetComponent<ModularShipComponent>(shipId);
        if (modularShip == null) return;
        
        var module = modularShip.GetModule(moduleId);
        if (module == null) return;
        
        float oldHealth = module.Health;
        module.Repair(repairAmount);
        
        _logger.Info("VoxelDamageSystem", 
            $"Module {moduleId} repaired by {repairAmount}: {oldHealth} -> {module.Health}");
        
        // Update damage visualization
        var damageComponent = _entityManager.GetComponent<VoxelDamageComponent>(shipId);
        if (damageComponent != null)
        {
            if (module.DamageLevel < 0.1f)
            {
                // Remove damage visualization if module is nearly pristine
                if (damageComponent.ModuleDamageMap.ContainsKey(moduleId))
                {
                    var oldVoxels = damageComponent.ModuleDamageMap[moduleId];
                    foreach (var voxel in oldVoxels)
                    {
                        damageComponent.DamageVoxels.Remove(voxel);
                    }
                    damageComponent.ModuleDamageMap.Remove(moduleId);
                }
            }
            else
            {
                UpdateModuleDamageVoxels(module, damageComponent);
            }
        }
        
        // Recalculate ship stats
        modularShip.RecalculateStats();
    }
}
