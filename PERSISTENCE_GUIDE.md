# Persistence System Guide

## Overview

The AvorionLike persistence system provides comprehensive save/load functionality for game state. It allows players to save their progress and resume games later, including all entity data, component states, and game configurations.

## Architecture

### Core Components

1. **ISerializable Interface** (`Core/Persistence/ISerializable.cs`)
   - Defines the contract for serializable objects
   - Methods:
     - `Serialize()` - Converts object to dictionary format
     - `Deserialize(Dictionary<string, object> data)` - Restores object from dictionary

2. **SerializationHelper** (`Core/Persistence/SerializationHelper.cs`)
   - Provides utility methods for complex type serialization
   - Handles Vector3, enums, and nested dictionaries
   - Manages JsonElement conversion for proper deserialization

3. **SaveGameManager** (`Core/Persistence/SaveGameManager.cs`)
   - Manages save file operations
   - Handles file I/O, directory management
   - Provides save game listing and metadata

4. **GameEngine Integration** (`Core/GameEngine.cs`)
   - Implements high-level save/load methods
   - Orchestrates entity and component serialization
   - Provides convenience methods (QuickSave, QuickLoad)

## Supported Components

The following components implement `ISerializable` and are automatically saved/loaded:

### 1. PhysicsComponent
Saves:
- Position, Velocity, Acceleration
- Rotation, Angular Velocity, Angular Acceleration
- Mass, Moment of Inertia
- Drag coefficients
- Thrust and torque capabilities
- Collision properties

### 2. VoxelStructureComponent
Saves:
- All voxel blocks (position, size, material, type)
- Block damage states
- Functional properties (thrust, power, shields)
- Structural integrity

### 3. InventoryComponent
Saves:
- Resource amounts by type
- Inventory capacity
- Current capacity usage

### 4. ProgressionComponent
Saves:
- Experience points and level
- Skill points
- Experience to next level

### 5. FactionComponent
Saves:
- Faction affiliation
- Reputation with all factions

## Usage

### Basic Save/Load

```csharp
// Initialize game engine
var engine = new GameEngine(12345);
engine.Start();

// Create some entities...
var ship = engine.EntityManager.CreateEntity("Player Ship");
// ... add components ...

// Save the game
bool success = engine.SaveGame("mysave");
if (success)
{
    Console.WriteLine("Game saved!");
}

// Load the game later
bool loaded = engine.LoadGame("mysave");
if (loaded)
{
    Console.WriteLine("Game loaded!");
}
```

### Quick Save/Load

```csharp
// Quick save (uses "quicksave" as name)
engine.QuickSave();

// Quick load
engine.QuickLoad();
```

### List Available Saves

```csharp
// Get list of save game names
List<string> saveNames = engine.GetSaveGames();

// Get detailed information
List<SaveGameInfo> saves = engine.GetSaveGameInfo();
foreach (var save in saves)
{
    Console.WriteLine($"{save.SaveName} - {save.SaveTime}");
}
```

## Save File Format

Save files are stored as JSON with the `.save` extension in:
- **Windows**: `%APPDATA%\AvorionLike\Saves\`
- **Linux/Mac**: `~/.config/AvorionLike/Saves/`

### Example Save Structure

```json
{
  "SaveName": "mysave",
  "SaveTime": "2025-11-05T20:00:00Z",
  "Version": "1.0.0",
  "GalaxySeed": 12345,
  "GameState": {
    "IsRunning": true
  },
  "Entities": [
    {
      "EntityId": "c6af84f5-4796-46c1-aa14-0f425a2f83a8",
      "EntityName": "Player Ship",
      "IsActive": true,
      "Components": [
        {
          "ComponentType": "AvorionLike.Core.Physics.PhysicsComponent",
          "Data": {
            "Position": { "X": 100.0, "Y": 100.0, "Z": 100.0 },
            "Velocity": { "X": 10.0, "Y": 0.0, "Z": 0.0 },
            "Mass": 1000.0
          }
        },
        {
          "ComponentType": "AvorionLike.Core.Voxel.VoxelStructureComponent",
          "Data": {
            "Blocks": [
              {
                "Position": { "X": 0.0, "Y": 0.0, "Z": 0.0 },
                "Size": { "X": 2.0, "Y": 2.0, "Z": 2.0 },
                "MaterialType": "Iron",
                "BlockType": "Hull"
              }
            ]
          }
        }
      ]
    }
  ]
}
```

## Adding Serialization to New Components

To make a new component serializable:

1. Implement the `ISerializable` interface:

```csharp
using AvorionLike.Core.Persistence;

public class MyComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }
    public string MyProperty { get; set; }
    
    public Dictionary<string, object> Serialize()
    {
        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["MyProperty"] = MyProperty
        };
    }
    
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(SerializationHelper.GetValue(data, "EntityId", Guid.Empty.ToString()));
        MyProperty = SerializationHelper.GetValue(data, "MyProperty", "default");
    }
}
```

2. Add serialization support in `GameEngine.cs`:

```csharp
// In SaveGame method, add:
SerializeComponent<MyComponent>(entity, entityData);

// In DeserializeComponent method, add case:
case "AvorionLike.Core.MyNamespace.MyComponent":
    var myComponent = new MyComponent();
    myComponent.Deserialize(componentData.Data);
    EntityManager.AddComponent(entityId, myComponent);
    break;
```

## Best Practices

### 1. Version Compatibility
- Save files include a version number
- Future versions should handle migration from older formats
- Consider backward compatibility when changing data structures

### 2. Error Handling
- Always check return values from save/load operations
- Log errors for debugging
- Provide user feedback on success/failure

### 3. Save File Management
- Use descriptive save names
- Implement auto-save at regular intervals
- Provide UI for managing multiple save slots

### 4. Performance
- Save operations are I/O bound
- Consider background saves for large game states
- Compress save files for large games (future enhancement)

### 5. Data Integrity
- Validate data during deserialization
- Use default values for missing data
- Handle corrupted save files gracefully

## Testing

Test the persistence system using the demo:

1. Run the application
2. Select option 1 to create test entities
3. Select option 11 for Persistence Demo
4. Save the game (option 1)
5. Exit and restart the application
6. Load the saved game (option 2)
7. Verify all entities and components are restored correctly

## Limitations

Current limitations:
- No save file compression
- No encryption for save files
- Limited to local file system storage
- No cloud save support
- Component types must be explicitly registered in GameEngine

## Future Enhancements

Potential improvements:
- Automatic component discovery and registration
- Save file compression (gzip)
- Encryption for multiplayer/competitive games
- Cloud storage integration
- Auto-save functionality with configurable intervals
- Multiple save slots with thumbnails
- Save file migration system for version updates
- Incremental saves (delta encoding)

## Troubleshooting

### Save Failed
- Check disk space
- Verify write permissions for save directory
- Check log files for detailed error messages

### Load Failed
- Verify save file exists and is not corrupted
- Check version compatibility
- Ensure all required components are registered
- Review log files for missing component types

### Data Loss
- Save files are in JSON format - can be manually edited if needed
- Always test save/load after adding new components
- Keep backups of important save files

## API Reference

### GameEngine Methods

```csharp
// Save current game state
bool SaveGame(string saveName)

// Load saved game state
bool LoadGame(string saveName)

// Quick save (uses "quicksave" as name)
bool QuickSave()

// Quick load most recent quicksave
bool QuickLoad()

// Get list of save game names
List<string> GetSaveGames()

// Get detailed save game information
List<SaveGameInfo> GetSaveGameInfo()
```

### SerializationHelper Methods

```csharp
// Serialize/deserialize Vector3
Dictionary<string, object> SerializeVector3(Vector3 vector)
Vector3 DeserializeVector3(object obj)

// Serialize/deserialize dictionaries
Dictionary<string, object> SerializeDictionary<TKey, TValue>(Dictionary<TKey, TValue> dict)
Dictionary<TKey, TValue> DeserializeDictionary<TKey, TValue>(Dictionary<string, object> data)

// Safely get values with defaults
T GetValue<T>(Dictionary<string, object> data, string key, T defaultValue)
```

## Related Documentation

- [Architecture Guide](ARCHITECTURE.md) - Overall system architecture
- [Next Steps](NEXT_STEPS.md) - Future development priorities
- [Implementation Roadmap](IMPLEMENTATION_ROADMAP.md) - Development timeline

## Support

For issues or questions about the persistence system:
1. Check the log files in `%APPDATA%\AvorionLike\Logs\`
2. Review this guide and related documentation
3. Open an issue on GitHub with log excerpts and save file samples
