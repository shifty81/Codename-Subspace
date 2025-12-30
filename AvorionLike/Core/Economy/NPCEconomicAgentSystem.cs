using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.AI;
using AvorionLike.Core.Mining;
using AvorionLike.Core.Economy;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Economy;

/// <summary>
/// Types of NPC economic agents
/// </summary>
public enum NPCAgentType
{
    Miner,
    Trader,
    Hauler,
    Producer
}

/// <summary>
/// Component for NPC economic agents that drive market simulation
/// </summary>
public class NPCEconomicAgentComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    /// <summary>
    /// Type of economic activity this agent performs
    /// </summary>
    public NPCAgentType AgentType { get; set; }
    
    /// <summary>
    /// Current trade route or mining location
    /// </summary>
    public Vector3? CurrentDestination { get; set; }
    
    /// <summary>
    /// Goods being transported
    /// </summary>
    public Dictionary<GoodType, int> CargoHold { get; set; } = new();
    
    /// <summary>
    /// Maximum cargo capacity
    /// </summary>
    public int MaxCargo { get; set; } = 1000;
    
    /// <summary>
    /// Current cargo used
    /// </summary>
    public int CurrentCargo => CargoHold.Values.Sum();
    
    /// <summary>
    /// Credits available for trading
    /// </summary>
    public int Credits { get; set; } = 10000;
    
    /// <summary>
    /// Home station ID
    /// </summary>
    public Guid? HomeStationId { get; set; }
    
    /// <summary>
    /// Target station for current route
    /// </summary>
    public Guid? TargetStationId { get; set; }
    
    /// <summary>
    /// Resource type for miners
    /// </summary>
    public ResourceType? TargetResource { get; set; }
    
    /// <summary>
    /// Efficiency multiplier (0.5 - 2.0)
    /// </summary>
    public float Efficiency { get; set; } = 1.0f;
    
    /// <summary>
    /// Time between economic actions (seconds)
    /// </summary>
    public float ActionInterval { get; set; } = 60f;
    
    /// <summary>
    /// Time until next action
    /// </summary>
    public float TimeUntilNextAction { get; set; } = 0f;
}

/// <summary>
/// System managing NPC economic agents for background simulation
/// Creates a living economy driven by AI behavior
/// </summary>
public class NPCEconomicAgentSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly EconomySystem _economySystem;
    private readonly Random _random;
    
    private const int MaxNPCAgents = 200; // Adjust based on performance
    private float _timeSinceLastSpawn = 0f;
    private const float AgentSpawnInterval = 120f; // Spawn agents every 2 minutes
    
    public NPCEconomicAgentSystem(EntityManager entityManager, EconomySystem economySystem, int seed = 0) 
        : base("NPCEconomicAgentSystem")
    {
        _entityManager = entityManager;
        _economySystem = economySystem;
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    public override void Update(float deltaTime)
    {
        // Spawn new agents if below max
        _timeSinceLastSpawn += deltaTime;
        if (_timeSinceLastSpawn >= AgentSpawnInterval)
        {
            _timeSinceLastSpawn = 0f;
            SpawnAgentsIfNeeded();
        }
        
        // Update all agents
        var agents = _entityManager.GetAllComponents<NPCEconomicAgentComponent>();
        foreach (var agent in agents)
        {
            UpdateAgent(agent, deltaTime);
        }
    }
    
    /// <summary>
    /// Spawn new agents if below maximum
    /// </summary>
    private void SpawnAgentsIfNeeded()
    {
        var currentAgents = _entityManager.GetAllComponents<NPCEconomicAgentComponent>().Count();
        
        if (currentAgents >= MaxNPCAgents)
            return;
            
        int agentsToSpawn = Math.Min(5, MaxNPCAgents - currentAgents);
        
        for (int i = 0; i < agentsToSpawn; i++)
        {
            SpawnRandomAgent();
        }
    }
    
    /// <summary>
    /// Spawn a random NPC economic agent
    /// </summary>
    private void SpawnRandomAgent()
    {
        var agentType = (NPCAgentType)_random.Next(Enum.GetValues<NPCAgentType>().Length);
        
        var entity = _entityManager.CreateEntity($"NPC {agentType}");
        
        var agent = new NPCEconomicAgentComponent
        {
            EntityId = entity.Id,
            AgentType = agentType,
            Efficiency = 0.5f + (float)_random.NextDouble() * 1.5f, // 0.5 to 2.0
            Credits = _random.Next(5000, 50000),
            MaxCargo = _random.Next(500, 2000)
        };
        
        // Set up based on type
        switch (agentType)
        {
            case NPCAgentType.Miner:
                agent.TargetResource = (ResourceType)_random.Next(Enum.GetValues<ResourceType>().Length);
                agent.ActionInterval = 90f; // Mine every 90 seconds
                break;
                
            case NPCAgentType.Trader:
            case NPCAgentType.Hauler:
                agent.ActionInterval = 180f; // Trade every 3 minutes
                break;
                
            case NPCAgentType.Producer:
                agent.ActionInterval = 120f; // Produce every 2 minutes
                break;
        }
        
        _entityManager.AddComponent(entity.Id, agent);
        
        // Add AI component for movement
        var aiComponent = new AIComponent
        {
            EntityId = entity.Id,
            Personality = AIPersonality.Trader,
            CurrentState = AIState.Trading
        };
        _entityManager.AddComponent(entity.Id, aiComponent);
        
        Logger.Instance.Info("NPCEconomicAgentSystem", $"Spawned NPC {agentType}");
    }
    
    /// <summary>
    /// Update a single agent's economic behavior
    /// </summary>
    private void UpdateAgent(NPCEconomicAgentComponent agent, float deltaTime)
    {
        agent.TimeUntilNextAction -= deltaTime;
        
        if (agent.TimeUntilNextAction > 0)
            return;
            
        // Reset timer with some randomness
        agent.TimeUntilNextAction = agent.ActionInterval * (0.8f + (float)_random.NextDouble() * 0.4f);
        
        // Perform action based on type
        switch (agent.AgentType)
        {
            case NPCAgentType.Miner:
                PerformMining(agent);
                break;
                
            case NPCAgentType.Trader:
                PerformTrading(agent);
                break;
                
            case NPCAgentType.Hauler:
                PerformHauling(agent);
                break;
                
            case NPCAgentType.Producer:
                PerformProduction(agent);
                break;
        }
    }
    
    /// <summary>
    /// NPC mining behavior - extracts resources and sells to stations
    /// </summary>
    private void PerformMining(NPCEconomicAgentComponent agent)
    {
        if (agent.TargetResource == null)
            return;
            
        // Mine resources
        int amountMined = (int)(100 * agent.Efficiency);
        
        // Convert resource to trade good
        var goodType = ResourceToGoodType(agent.TargetResource.Value);
        
        if (agent.CurrentCargo + amountMined <= agent.MaxCargo)
        {
            if (!agent.CargoHold.ContainsKey(goodType))
                agent.CargoHold[goodType] = 0;
                
            agent.CargoHold[goodType] += amountMined;
            
            Logger.Instance.Debug("NPCEconomicAgentSystem", 
                $"NPC miner extracted {amountMined} {goodType}");
        }
        
        // If cargo is full, sell to nearest station
        if (agent.CurrentCargo >= agent.MaxCargo * 0.8f)
        {
            SellCargoToStation(agent);
        }
    }
    
    /// <summary>
    /// NPC trading behavior - buys low, sells high
    /// </summary>
    private void PerformTrading(NPCEconomicAgentComponent agent)
    {
        var stations = _economySystem.GetAllStations().ToList();
        if (stations.Count < 2)
            return;
            
        // Find profitable trade route
        var bestRoute = FindBestTradeRoute(agent, stations);
        
        if (bestRoute.profit > 0)
        {
            ExecuteTradeRoute(agent, bestRoute.buyStation, bestRoute.sellStation, bestRoute.goodType, bestRoute.amount);
        }
    }
    
    /// <summary>
    /// NPC hauling behavior - transports goods between stations
    /// </summary>
    private void PerformHauling(NPCEconomicAgentComponent agent)
    {
        // Similar to trading but focuses on high-volume, low-margin goods
        PerformTrading(agent);
    }
    
    /// <summary>
    /// NPC production behavior - consumes resources and produces goods
    /// </summary>
    private void PerformProduction(NPCEconomicAgentComponent agent)
    {
        // Produce goods and add to market
        var producedGood = (GoodType)_random.Next(Enum.GetValues<GoodType>().Length);
        int amount = (int)(50 * agent.Efficiency);
        
        if (!agent.CargoHold.ContainsKey(producedGood))
            agent.CargoHold[producedGood] = 0;
            
        agent.CargoHold[producedGood] += amount;
        
        // Sell to station
        if (agent.CurrentCargo >= agent.MaxCargo * 0.5f)
        {
            SellCargoToStation(agent);
        }
    }
    
    /// <summary>
    /// Sell all cargo to the nearest station
    /// </summary>
    private void SellCargoToStation(NPCEconomicAgentComponent agent)
    {
        var stations = _economySystem.GetAllStations().ToList();
        if (stations.Count == 0)
            return;
            
        var targetStation = stations[_random.Next(stations.Count)];
        
        int totalRevenue = 0;
        
        foreach (var cargo in agent.CargoHold.ToList())
        {
            // Simplified: add to station inventory and get paid
            if (!targetStation.Inventory.ContainsKey(cargo.Key))
            {
                targetStation.Inventory[cargo.Key] = new TradeGood
                {
                    Type = cargo.Key,
                    Quantity = 0,
                    BasePrice = GetGoodBasePrice(cargo.Key),
                    CurrentPrice = GetGoodBasePrice(cargo.Key)
                };
            }
            
            int price = (int)(targetStation.Inventory[cargo.Key].CurrentPrice * cargo.Value);
            agent.Credits += price;
            totalRevenue += price;
            
            targetStation.Inventory[cargo.Key].Quantity += cargo.Value;
        }
        
        agent.CargoHold.Clear();
        
        Logger.Instance.Debug("NPCEconomicAgentSystem", 
            $"NPC sold cargo for {totalRevenue} credits");
    }
    
    /// <summary>
    /// Find the best trade route for profit
    /// </summary>
    private (Station buyStation, Station sellStation, GoodType goodType, int amount, int profit) FindBestTradeRoute(
        NPCEconomicAgentComponent agent, List<Station> stations)
    {
        int bestProfit = 0;
        Station? bestBuyStation = null;
        Station? bestSellStation = null;
        GoodType bestGood = GoodType.IronOre;
        int bestAmount = 0;
        
        foreach (var buyStation in stations)
        {
            foreach (var sellStation in stations)
            {
                if (buyStation.Id == sellStation.Id)
                    continue;
                    
                foreach (var good in buyStation.Inventory)
                {
                    if (good.Value.Quantity < 100)
                        continue;
                        
                    if (!sellStation.Inventory.ContainsKey(good.Key))
                        continue;
                        
                    float buyPrice = good.Value.CurrentPrice;
                    float sellPrice = sellStation.Inventory[good.Key].CurrentPrice;
                    
                    if (sellPrice <= buyPrice)
                        continue;
                        
                    int maxAmount = Math.Min(agent.MaxCargo, good.Value.Quantity);
                    int profit = (int)((sellPrice - buyPrice) * maxAmount);
                    
                    if (profit > bestProfit && agent.Credits >= buyPrice * maxAmount)
                    {
                        bestProfit = profit;
                        bestBuyStation = buyStation;
                        bestSellStation = sellStation;
                        bestGood = good.Key;
                        bestAmount = maxAmount;
                    }
                }
            }
        }
        
        return (bestBuyStation!, bestSellStation!, bestGood, bestAmount, bestProfit);
    }
    
    /// <summary>
    /// Execute a trade route
    /// </summary>
    private void ExecuteTradeRoute(NPCEconomicAgentComponent agent, Station buyStation, Station sellStation, 
        GoodType goodType, int amount)
    {
        // Buy from source
        float buyPrice = buyStation.Inventory[goodType].CurrentPrice;
        int cost = (int)(buyPrice * amount);
        
        if (agent.Credits < cost)
            return;
            
        agent.Credits -= cost;
        buyStation.Inventory[goodType].Quantity -= amount;
        
        if (!agent.CargoHold.ContainsKey(goodType))
            agent.CargoHold[goodType] = 0;
        agent.CargoHold[goodType] += amount;
        
        // Sell to destination
        float sellPrice = sellStation.Inventory[goodType].CurrentPrice;
        int revenue = (int)(sellPrice * amount);
        
        agent.Credits += revenue;
        agent.CargoHold[goodType] -= amount;
        
        if (agent.CargoHold[goodType] <= 0)
            agent.CargoHold.Remove(goodType);
            
        sellStation.Inventory[goodType].Quantity += amount;
        
        int profit = revenue - cost;
        Logger.Instance.Debug("NPCEconomicAgentSystem", 
            $"NPC trader made {profit} credits profit");
    }
    
    /// <summary>
    /// Convert resource type to trade good type
    /// </summary>
    private GoodType ResourceToGoodType(ResourceType resource)
    {
        return resource switch
        {
            ResourceType.Iron => GoodType.IronOre,
            ResourceType.Titanium => GoodType.TitaniumOre,
            _ => GoodType.IronOre
        };
    }
    
    /// <summary>
    /// Get base price for a good
    /// </summary>
    private float GetGoodBasePrice(GoodType type)
    {
        return type switch
        {
            GoodType.IronOre => 5f,
            GoodType.TitaniumOre => 10f,
            GoodType.RefinedIron => 15f,
            GoodType.RefinedTitanium => 30f,
            GoodType.Electronics => 50f,
            GoodType.Machinery => 75f,
            GoodType.Weapons => 100f,
            GoodType.ShipParts => 150f,
            GoodType.Food => 20f,
            GoodType.Medicine => 40f,
            GoodType.Luxuries => 80f,
            _ => 10f
        };
    }
}
