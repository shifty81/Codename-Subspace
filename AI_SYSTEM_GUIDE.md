# AI System Guide

## Overview

The Avorion-like AI system provides sophisticated autonomous ship behavior through a state-based architecture with perception, decision-making, and movement capabilities. The system integrates seamlessly with existing game systems including Combat, Mining, Physics, and Navigation.

## Architecture

### Core Components

1. **AIComponent** - Defines AI entity properties and state
2. **AISystem** - Main system managing all AI entities
3. **AIPerceptionSystem** - Handles environmental awareness
4. **AIDecisionSystem** - Manages state transitions and prioritization
5. **AIMovementSystem** - Controls ship movement and combat maneuvering

## AI States

The AI operates in various states based on current situation and priorities:

### State Hierarchy (Priority Order)

1. **Fleeing** - Highest priority when severely damaged
2. **Combat** - Engage hostile entities
3. **ReturningToBase** - Return home when cargo full or damaged
4. **Mining/Salvaging/Trading** - Resource gathering activities
5. **Patrol** - Move between waypoints
6. **Idle** - Default state

### State Descriptions

- **Idle**: Default state, ship stays relatively still
- **Patrol**: Ship follows predefined waypoints
- **Mining**: Locate and mine asteroids for resources
- **Salvaging**: Collect resources from wreckage
- **Trading**: Dock at stations to trade resources
- **Combat**: Engage hostile entities with weapons
- **Fleeing**: Retreat from combat when damaged
- **Evasion**: Tactical evasive maneuvers
- **ReturningToBase**: Navigate back to home base
- **Repairing**: Get repairs at a station

## AI Personalities

Personalities affect decision-making priorities and behavior:

- **Balanced** - Tries all activities equally
- **Aggressive** - Seeks combat, prefers strongest targets
- **Defensive** - Cautious, only fights when threatened
- **Miner** - Prioritizes mining over other activities
- **Trader** - Focuses on trading operations
- **Salvager** - Prefers salvaging wreckage
- **Coward** - Flees easily, avoids combat

## Combat Tactics

AI ships can use different combat maneuvers:

- **Aggressive** - Close in and attack directly
- **Kiting** - Maintain distance while attacking
- **Strafing** - Circle around target
- **Broadsiding** - Position perpendicular for maximum turret coverage
- **Defensive** - Maintain maximum distance, evade frequently

## Usage Examples

### Creating a Mining AI Ship

```csharp
var engine = new GameEngine();
var entity = engine.EntityManager.CreateEntity("Miner");

// Add required components (structure, physics, mining, inventory)
var structure = new VoxelStructureComponent();
structure.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(2, 2, 2), "Iron"));
engine.EntityManager.AddComponent(entity.Id, structure);

var physics = new PhysicsComponent
{
    Position = new Vector3(100, 0, 0),
    Mass = structure.TotalMass
};
engine.EntityManager.AddComponent(entity.Id, physics);

var mining = new MiningComponent
{
    EntityId = entity.Id,
    MiningPower = 15f,
    MiningRange = 100f
};
engine.EntityManager.AddComponent(entity.Id, mining);

var inventory = new InventoryComponent
{
    EntityId = entity.Id,
    Inventory = new Inventory { MaxCapacity = 500 }
};
engine.EntityManager.AddComponent(entity.Id, inventory);

// Add AI component
var ai = new AIComponent
{
    EntityId = entity.Id,
    Personality = AIPersonality.Miner,
    CanMine = true,
    HomeBase = new Vector3(0, 0, 0)
};
engine.EntityManager.AddComponent(entity.Id, ai);
```

### Creating a Combat AI Ship

```csharp
var entity = engine.EntityManager.CreateEntity("Fighter");

// Add structure, physics, combat components...

var combat = new CombatComponent
{
    EntityId = entity.Id,
    MaxShields = 500f,
    CurrentShields = 500f
};

// Add turrets
combat.AddTurret(new Turret
{
    Name = "Laser",
    Type = WeaponType.Laser,
    Damage = 25f,
    FireRate = 2f,
    Range = 1200f
});

engine.EntityManager.AddComponent(entity.Id, combat);

// Add AI with aggressive personality
var ai = new AIComponent
{
    EntityId = entity.Id,
    Personality = AIPersonality.Aggressive,
    CombatTactic = CombatTactic.Strafing,
    MinCombatDistance = 400f,
    MaxCombatDistance = 1000f
};
engine.EntityManager.AddComponent(entity.Id, ai);
```

### Setting Patrol Waypoints

```csharp
var ai = engine.EntityManager.GetComponent<AIComponent>(entityId);
var waypoints = new List<Vector3>
{
    new Vector3(100, 0, 0),
    new Vector3(100, 100, 0),
    new Vector3(0, 100, 0),
    new Vector3(0, 0, 0)
};

engine.AISystem.SetPatrolWaypoints(entityId, waypoints);
```

### Setting Home Base

```csharp
engine.AISystem.SetHomeBase(entityId, new Vector3(0, 0, 0));
```

## Perception System

The AI perception system provides awareness of the environment:

### What AI Can Perceive

- **Nearby Entities** - Other ships within perception range (default: 2000 units)
  - Position, velocity, distance
  - Shield and hull status
  - Hostile/friendly status
  
- **Asteroids** - Mineable resources
  - Position, distance
  - Resource type and remaining amount
  
- **Stations** - Trading posts and docks
  - Position, distance
  - Station type
  
- **Threats** - Hostile entities
  - Priority level (None, Low, Medium, High, Critical)
  - Threat assessment (0-1 scale)
  - Whether actively attacking

### Perception Range

Default perception range is 2000 units, configurable when creating the AIPerceptionSystem.

## Decision Making

The AI decision system evaluates multiple factors:

### Decision Factors

1. **Hull Percentage** - Ship health determines flee/fight decisions
2. **Shield Status** - Shield strength affects combat willingness
3. **Cargo Capacity** - Full cargo triggers return to base
4. **Threat Level** - Number and strength of nearby hostiles
5. **Personality** - Affects all decision weights
6. **Resource Availability** - Presence of asteroids, stations, etc.

### State Transitions

States automatically transition based on priorities:

```
Damaged (< 20% health) → Fleeing
Under Attack → Combat
Cargo Full (> 90%) → ReturningToBase
Asteroids Nearby + Miner → Mining
No Threats + Has Waypoints → Patrol
Default → Idle
```

## Movement Behaviors

### Patrol Movement
- Moves sequentially through waypoints
- Transitions to next waypoint when within 100 units
- Loops back to first waypoint

### Combat Movement

Different tactics produce different behaviors:

**Aggressive**:
- Close to minimum combat distance
- Face target directly
- Full frontal assault

**Kiting**:
- Maintain ideal distance (average of min/max)
- Retreat if too close
- Face target while moving

**Strafing**:
- Circle around target
- Adjust distance to ideal range
- Perpendicular movement

**Broadsiding**:
- Position perpendicular to target
- Optimal for side-mounted turrets
- Maintain steady distance

**Defensive**:
- Stay at maximum combat distance
- Random evasive movements
- Face target but ready to dodge

### Mining Movement
- Approach asteroid to within mining range
- Apply braking when close
- Start mining operation when in range

### Fleeing Movement
- Move away from threats
- Head to home base if available
- Maximum speed

## Configuration Parameters

### AIComponent Properties

```csharp
// State control
CurrentState          // Current AI state
Personality          // AI personality type
IsEnabled           // Whether AI is active

// Combat settings
FleeThreshold       // Hull % to flee (default: 0.2 = 20%)
ReturnToCombatThreshold // Hull % to stop fleeing (default: 0.6 = 60%)
MinCombatDistance   // Minimum distance from target (default: 300)
MaxCombatDistance   // Maximum distance from target (default: 800)
CombatTactic        // Preferred combat maneuver

// Resource gathering
CanMine             // Whether AI can mine
CanSalvage          // Whether AI can salvage
CanTrade            // Whether AI can trade
CargoReturnThreshold // Cargo % to return home (default: 0.9 = 90%)

// Navigation
PatrolWaypoints     // List of patrol positions
HomeBase           // Home station position

// Timing
IdleTimeout         // Seconds before switching to patrol (default: 10)
EvaluationInterval  // Seconds between state evaluations (default: 1)
```

## Integration with Game Systems

### Physics System
- AI movement commands translate to physics forces
- Thrust and torque applied through PhysicsComponent
- Collision avoidance (future enhancement)

### Combat System
- AI automatically aims and fires turrets
- Target prioritization based on threats
- Energy management for weapons

### Mining System
- AI locates and approaches asteroids
- Starts/stops mining operations
- Manages inventory capacity

### Event System
- Publishes "AI.StateChanged" events
- Can subscribe to game events for reactive behavior

## Best Practices

### Performance
- Adjust `EvaluationInterval` to balance responsiveness vs. CPU usage
- Higher intervals (2-3 seconds) for non-critical NPCs
- Lower intervals (0.5-1 second) for important enemies

### Balancing
- Adjust flee thresholds based on AI aggressiveness
- Scale combat distances with ship size and weapon ranges
- Match cargo capacity with home base availability

### Design Patterns
- Use personality types to create diverse behaviors
- Combine patrol with other capabilities for dynamic AI
- Set appropriate home bases for resource gatherers
- Configure combat tactics based on ship design

## Troubleshooting

### AI Not Moving
- Check that entity has PhysicsComponent
- Verify MaxThrust > 0
- Ensure AI is enabled (IsEnabled = true)
- Check that state is not Idle

### AI Not Engaging in Combat
- Verify CombatComponent exists
- Check that weapons have ammo/energy
- Ensure AI personality allows combat
- Verify targets are within perception range

### AI Not Mining
- Check that CanMine = true
- Verify MiningComponent exists
- Ensure asteroids are within perception range
- Check that inventory isn't full

### AI Constantly Fleeing
- Adjust FleeThreshold (increase it)
- Check ship's hull status
- Verify personality isn't Coward
- Ensure threats aren't overwhelming

## Future Enhancements

Planned improvements:
- Advanced pathfinding with obstacle avoidance
- Formation flying for fleet coordination
- Learning AI that adapts to player tactics
- Trade route optimization
- Dynamic threat assessment
- Cooperative behaviors (flanking, supporting)
- More sophisticated evasion patterns

## API Reference

### AISystem Methods

```csharp
void AddAI(Guid entityId, AIPersonality personality)
void SetPatrolWaypoints(Guid entityId, List<Vector3> waypoints)
void SetHomeBase(Guid entityId, Vector3 homePosition)
```

### AIPerceptionSystem Methods

```csharp
AIPerception UpdatePerception(Guid entityId, MiningSystem miningSystem)
PerceivedAsteroid? FindBestAsteroid(AIPerception perception, Vector3 currentPosition)
ThreatInfo? FindBestTarget(AIPerception perception, AIPersonality personality)
```

### AIDecisionSystem Methods

```csharp
AIState EvaluateState(AIComponent ai, AIPerception perception)
Guid? SelectTarget(AIComponent ai, AIPerception perception, AIPerceptionSystem perceptionSystem)
float CalculateActionPriority(AIComponent ai, AIState state, AIPerception perception)
```

### AIMovementSystem Methods

```csharp
void ExecuteMovement(AIComponent ai, float deltaTime)
```

## Example: Complete AI Ship Setup

See `Examples/AISystemExample.cs` for complete working examples including:
- Mining AI ships with resource gathering
- Combat AI ships with multiple tactics
- Patrol AI ships with waypoint navigation
- Full demonstration with multiple AI entities

Run the demo with:
```csharp
AISystemExample.RunAIDemo();
```
