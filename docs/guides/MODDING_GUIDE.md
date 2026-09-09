# Modding Guide for Codename:Subspace
## Creating Lua Mods for the Game Engine

Welcome to modding! Codename:Subspace has a powerful Lua-based modding system that lets you extend and customize the game.

## Table of Contents
1. [Getting Started](#getting-started)
2. [Mod Structure](#mod-structure)
3. [Lua API Reference](#lua-api-reference)
4. [Example Mods](#example-mods)
5. [Best Practices](#best-practices)
6. [Troubleshooting](#troubleshooting)

## Getting Started

### Prerequisites
- Codename:Subspace installed
- Basic knowledge of Lua programming
- Text editor (VS Code, Notepad++, etc.)

### Mods Directory
Mods are automatically discovered in:
```
Windows: %APPDATA%/Codename-Subspace/Mods/
Linux: ~/.config/Codename-Subspace/Mods/
macOS: ~/Library/Application Support/Codename-Subspace/Mods/
```

## Mod Structure

### Basic Mod Layout
```
MyMod/
├── mod.json          # Mod metadata
└── main.lua          # Main script file
```

### mod.json Format
```json
{
  "Id": "my_awesome_mod",
  "Name": "My Awesome Mod",
  "Version": "1.0.0",
  "Author": "YourName",
  "Description": "Adds cool new features to the game",
  "MainScript": "main.lua",
  "Dependencies": []
}
```

### Creating Your First Mod

1. **Create mod directory**:
   ```
   Mods/my_first_mod/
   ```

2. **Create mod.json**:
   ```json
   {
     "Id": "my_first_mod",
     "Name": "My First Mod",
     "Version": "1.0.0",
     "Author": "You",
     "Description": "My first modding attempt!",
     "MainScript": "main.lua"
   }
   ```

3. **Create main.lua**:
   ```lua
   -- My First Mod
   log("Hello from my first mod!")
   
   function OnModLoad()
       log("Mod loaded successfully!")
   end
   
   OnModLoad()
   ```

4. **Launch the game** - Your mod will be automatically loaded!

## Lua API Reference

### Built-in Functions

#### Logging
```lua
-- Log a message
log("This is a message")
API:Log("Info message")
API:LogWarning("Warning message")
API:LogError("Error message")
```

#### Safe API Calls
```lua
-- Safely call API functions with error handling
local result = SafeAPICall("CreateEntity", "MyShip")
```

### Entity Management

#### Create Entity
```lua
-- Returns entity ID as string
local entityId = API:CreateEntity("My Custom Ship")
log("Created entity: " .. entityId)
```

#### Destroy Entity
```lua
local success = API:DestroyEntity(entityId)
if success then
    log("Entity destroyed")
end
```

#### Get Entity Count
```lua
local count = API:GetEntityCount()
log("Total entities: " .. count)
```

### Voxel System

#### Add Voxel Structure
```lua
-- Add voxel component to entity
local success = API:AddVoxelStructure(entityId)
```

#### Add Voxel Block
```lua
-- Add a block at position with size and material
local success = API:AddVoxelBlock(
    entityId,
    0, 0, 0,        -- x, y, z position
    2, 2, 2,        -- sizeX, sizeY, sizeZ
    "Iron"          -- material type
)
```

**Available Materials**:
- "Iron" - Basic hull material
- "Titanium" - Stronger, lighter
- "Naonite" - Advanced material
- "Trinium" - Lightweight, strong
- "Xanion" - Very strong
- "Ogonite" - Extremely dense
- "Avorion" - Legendary material

#### Get Voxel Mass
```lua
local mass = API:GetVoxelMass(entityId)
log("Ship mass: " .. mass .. " kg")
```

### Physics System

#### Add Physics Component
```lua
-- Add physics at position with mass
local success = API:AddPhysics(
    entityId,
    100, 100, 100,  -- x, y, z position
    1000            -- mass in kg
)
```

#### Apply Force
```lua
-- Apply force vector to entity
API:ApplyForce(entityId, 0, 0, 1000)  -- Push forward
```

#### Set Velocity
```lua
-- Set velocity directly
API:SetVelocity(entityId, 10, 0, 0)  -- Move in X direction
```

#### Get Position
```lua
local pos = API:GetPosition(entityId)
log("Position: " .. pos.x .. ", " .. pos.y .. ", " .. pos.z)
```

### Resource Management

#### Add Inventory
```lua
-- Add inventory component to entity
local success = API:AddInventory(entityId, 1000)
```

#### Add Resources
```lua
-- Add resources to inventory
local success = API:AddResource(entityId, "Iron", 100)
```

**Resource Types**:
- "Iron"
- "Titanium"
- "Naonite"
- "Trinium"
- "Xanion"
- "Ogonite"
- "Avorion"
- "Credits"

#### Get Resource Amount
```lua
local amount = API:GetResourceAmount(entityId, "Iron")
log("Iron: " .. amount)
```

### Event System

#### Subscribe to Events
```lua
-- Subscribe to game events
API:SubscribeToEvent("EntityCreated", function(event)
    log("An entity was created!")
end)
```

#### Publish Events
```lua
-- Publish custom events
API:PublishEvent("MyCustomEvent", "Some data")
```

### Galaxy Generation

#### Generate Sector
```lua
-- Generate a galaxy sector
local sector = API:GenerateSector(0, 0, 0)
log("Sector has " .. sector.AsteroidCount .. " asteroids")
if sector.HasStation then
    log("Sector has a station!")
end
```

### Utility Functions

#### Get Statistics
```lua
local stats = API:GetStatistics()
log("Total entities: " .. stats.TotalEntities)
```

#### Get Game Time
```lua
local time = API:GetGameTime()
log("Current game time: " .. time)
```

## Example Mods

### Example 1: Custom Ship Spawner
```lua
-- custom_ships.lua
log("=== Custom Ship Spawner Loaded ===")

function CreateBattleship(name, x, y, z)
    log("Creating battleship: " .. name)
    
    -- Create entity
    local entityId = API:CreateEntity(name)
    
    -- Add voxel structure
    API:AddVoxelStructure(entityId)
    
    -- Build hull (larger central blocks)
    for i = -2, 2 do
        for j = -1, 1 do
            API:AddVoxelBlock(entityId, i*3, j*3, 0, 3, 3, 3, "Titanium")
        end
    end
    
    -- Add engines (rear blocks)
    API:AddVoxelBlock(entityId, -9, 0, 0, 2, 2, 2, "Iron")
    API:AddVoxelBlock(entityId, -9, 0, 3, 2, 2, 2, "Iron")
    
    -- Add physics
    local mass = API:GetVoxelMass(entityId)
    API:AddPhysics(entityId, x, y, z, mass)
    
    -- Add inventory
    API:AddInventory(entityId, 5000)
    API:AddResource(entityId, "Iron", 100)
    API:AddResource(entityId, "Titanium", 50)
    
    log("Battleship created with mass: " .. mass)
    return entityId
end

-- Create a test battleship
local ship = CreateBattleship("USS Enterprise", 0, 0, 0)
```

### Example 2: Resource Generator
```lua
-- resource_generator.lua
log("=== Resource Generator Loaded ===")

-- Configuration
local GENERATION_INTERVAL = 10  -- seconds
local RESOURCE_AMOUNT = 50

function GenerateResourcesForEntity(entityId)
    local resources = {"Iron", "Titanium", "Naonite"}
    
    for _, resource in ipairs(resources) do
        local success = API:AddResource(entityId, resource, RESOURCE_AMOUNT)
        if success then
            log("Generated " .. RESOURCE_AMOUNT .. " " .. resource)
        end
    end
end

-- Subscribe to game time events
local lastGeneration = 0
function OnUpdate()
    local currentTime = API:GetGameTime()
    
    if currentTime - lastGeneration >= GENERATION_INTERVAL then
        -- Generate resources for all entities with inventories
        local entityCount = API:GetEntityCount()
        log("Generating resources for entities...")
        lastGeneration = currentTime
    end
end

log("Resource Generator active - generates resources every " .. GENERATION_INTERVAL .. " seconds")
```

### Example 3: Event Logger
```lua
-- event_logger.lua
log("=== Event Logger Loaded ===")

-- Subscribe to various events
API:SubscribeToEvent("EntityCreated", function(event)
    log("[EVENT] Entity was created")
end)

API:SubscribeToEvent("EntityDestroyed", function(event)
    log("[EVENT] Entity was destroyed")
end)

API:SubscribeToEvent("ComponentAdded", function(event)
    log("[EVENT] Component was added to entity")
end)

log("Event Logger is monitoring game events")
```

### Example 4: Auto-Miner
```lua
-- auto_miner.lua
log("=== Auto-Miner Mod Loaded ===")

function CreateMiningDrone(x, y, z)
    local droneId = API:CreateEntity("Mining Drone")
    
    -- Small, efficient design
    API:AddVoxelStructure(droneId)
    API:AddVoxelBlock(droneId, 0, 0, 0, 1, 1, 1, "Iron")
    API:AddVoxelBlock(droneId, 1, 0, 0, 1, 1, 1, "Iron")
    
    local mass = API:GetVoxelMass(droneId)
    API:AddPhysics(droneId, x, y, z, mass)
    API:AddInventory(droneId, 1000)
    
    log("Mining drone created at (" .. x .. ", " .. y .. ", " .. z .. ")")
    return droneId
end

-- Create a fleet of mining drones
for i = 1, 5 do
    CreateMiningDrone(i * 10, 0, 0)
end

log("Mining fleet deployed!")
```

## Best Practices

### 1. Error Handling
Always use `SafeAPICall` for critical operations:
```lua
local result = SafeAPICall("CreateEntity", "MyShip")
if result == nil then
    log("ERROR: Failed to create entity")
    return
end
```

### 2. Performance
- Avoid creating entities in tight loops
- Cache API results when possible
- Use events instead of polling

### 3. Naming Conventions
- Use descriptive entity names
- Prefix custom events with your mod name
- Use UPPERCASE for constants

### 4. Mod Dependencies
If your mod requires another mod:
```json
{
  "Dependencies": ["core_utilities", "advanced_building"]
}
```

### 5. Documentation
Add comments to your code:
```lua
-- Creates a custom ship with specific layout
-- @param name: Ship name
-- @param x, y, z: Starting position
-- @return entityId: ID of created ship
function CreateCustomShip(name, x, y, z)
    -- Implementation
end
```

## Troubleshooting

### Mod Not Loading
1. Check mod.json syntax (use JSON validator)
2. Verify MainScript file exists
3. Check game logs for errors
4. Ensure mod folder is in correct location

### Lua Errors
- Check console for error messages
- Use `log()` for debugging
- Test API calls individually
- Verify entity IDs are valid

### Performance Issues
- Reduce entity creation frequency
- Use batching for bulk operations
- Profile your mod code
- Check for infinite loops

### Common Errors

**"API not initialized"**
- Called API before it was ready
- Use SafeAPICall wrapper

**"Entity not found"**
- Entity ID is invalid or entity was destroyed
- Always check return values

**"Invalid resource type"**
- Check spelling of resource names
- Use exact capitalization

## Advanced Topics

### Custom Material Properties
```lua
-- Define custom block properties
local customBlock = {
    material = "Titanium",
    position = {x = 0, y = 0, z = 0},
    size = {x = 2, y = 2, z = 2}
}
```

### Working with Vectors
```lua
-- Vector math for positions
function CalculateDistance(pos1, pos2)
    local dx = pos2.x - pos1.x
    local dy = pos2.y - pos1.y
    local dz = pos2.z - pos1.z
    return math.sqrt(dx*dx + dy*dy + dz*dz)
end
```

### State Persistence
```lua
-- Save mod state between sessions
ModState = {
    droneCount = 0,
    resourcesGathered = 0
}

function SaveState()
    -- Implement save logic
end

function LoadState()
    -- Implement load logic
end
```

## Community Resources

- **Discord**: Join our modding community
- **GitHub**: Contribute examples and templates
- **Wiki**: Comprehensive API documentation
- **Forums**: Get help and share your mods

## Mod Submission

Want to share your mod?
1. Test thoroughly
2. Write clear documentation
3. Create a README.md
4. Submit to mod repository
5. Share on community channels

## Version Compatibility

This guide is for **Codename:Subspace v0.1.x**

Always check mod compatibility with your game version!

---

Happy modding! 🚀

*For more examples, check the `/Mods/Examples/` directory in your game installation.*
