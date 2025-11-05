using AvorionLike.Core.ECS;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Procedural;

namespace AvorionLike.Core.Economy;

/// <summary>
/// Types of goods that can be produced and traded
/// </summary>
public enum GoodType
{
    // Raw materials
    IronOre,
    TitaniumOre,
    
    // Refined materials
    RefinedIron,
    RefinedTitanium,
    
    // Components
    Electronics,
    Machinery,
    Weapons,
    ShipParts,
    
    // Consumer goods
    Food,
    Medicine,
    Luxuries
}

/// <summary>
/// Represents a trade good with price and quantity
/// </summary>
public class TradeGood
{
    public GoodType Type { get; set; }
    public int Quantity { get; set; }
    public float BasePrice { get; set; }
    public float CurrentPrice { get; set; }
}

/// <summary>
/// Production recipe for stations
/// </summary>
public class ProductionRecipe
{
    public GoodType Output { get; set; }
    public int OutputAmount { get; set; }
    public Dictionary<GoodType, int> Inputs { get; set; } = new();
    public float ProductionTime { get; set; } // In seconds
}

/// <summary>
/// Types of stations
/// </summary>
public enum StationType
{
    Mine,
    Refinery,
    Factory,
    Shipyard,
    TradingPost,
    BlackMarket,
    SmugglerHideout
}

/// <summary>
/// Represents a station that can produce and trade goods
/// </summary>
public class Station
{
    public Guid Id { get; set; } = Guid.NewGuid();
    public string Name { get; set; } = "Station";
    public StationType Type { get; set; }
    public Dictionary<GoodType, TradeGood> Inventory { get; set; } = new();
    public List<ProductionRecipe> Recipes { get; set; } = new();
    public bool IsIllegal { get; set; } = false; // Black market or smuggler
    
    public void ProduceGoods(float deltaTime)
    {
        foreach (var recipe in Recipes)
        {
            // Check if we have inputs
            bool canProduce = true;
            foreach (var input in recipe.Inputs)
            {
                if (!Inventory.ContainsKey(input.Key) || Inventory[input.Key].Quantity < input.Value)
                {
                    canProduce = false;
                    break;
                }
            }
            
            if (canProduce)
            {
                // Consume inputs
                foreach (var input in recipe.Inputs)
                {
                    Inventory[input.Key].Quantity -= input.Value;
                }
                
                // Produce output
                if (!Inventory.ContainsKey(recipe.Output))
                {
                    Inventory[recipe.Output] = new TradeGood 
                    { 
                        Type = recipe.Output, 
                        Quantity = 0,
                        BasePrice = GetBasePrice(recipe.Output),
                        CurrentPrice = GetBasePrice(recipe.Output)
                    };
                }
                Inventory[recipe.Output].Quantity += recipe.OutputAmount;
            }
        }
        
        // Update prices based on supply/demand
        UpdatePrices();
    }
    
    private void UpdatePrices()
    {
        foreach (var good in Inventory.Values)
        {
            // Simple supply/demand: more stock = lower price
            float supplyFactor = 1.0f;
            if (good.Quantity > 1000) supplyFactor = 0.8f;
            else if (good.Quantity > 500) supplyFactor = 0.9f;
            else if (good.Quantity < 100) supplyFactor = 1.3f;
            else if (good.Quantity < 50) supplyFactor = 1.5f;
            
            good.CurrentPrice = good.BasePrice * supplyFactor;
        }
    }
    
    private float GetBasePrice(GoodType type)
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

/// <summary>
/// Component for trading with stations
/// </summary>
public class TraderComponent : IComponent
{
    public Guid EntityId { get; set; }
    public List<TradeRoute> TradeRoutes { get; set; } = new();
    public int TradingReputation { get; set; } = 0;
}

/// <summary>
/// Trade route between two stations
/// </summary>
public class TradeRoute
{
    public Guid SourceStationId { get; set; }
    public Guid DestinationStationId { get; set; }
    public GoodType GoodToTransport { get; set; }
    public int Amount { get; set; }
    public bool IsActive { get; set; } = true;
}

/// <summary>
/// Component for piracy activities
/// </summary>
public class PirateComponent : IComponent
{
    public Guid EntityId { get; set; }
    public int PirateReputation { get; set; } = 0; // Negative = hostile to authorities
    public int StolenGoods { get; set; } = 0;
    public List<GoodType> IllegalCargo { get; set; } = new(); // Smuggled goods
}

/// <summary>
/// System for managing economy, production, and trading
/// </summary>
public class EconomySystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly Dictionary<Guid, Station> _stations = new();
    private readonly Random _random = new();

    public EconomySystem(EntityManager entityManager) : base("EconomySystem")
    {
        _entityManager = entityManager;
        InitializeStationTemplates();
    }

    public override void Update(float deltaTime)
    {
        // Update all stations
        foreach (var station in _stations.Values)
        {
            station.ProduceGoods(deltaTime);
        }
        
        // Process automated trade routes
        UpdateTradeRoutes(deltaTime);
    }
    
    /// <summary>
    /// Initialize station templates with production recipes
    /// </summary>
    private void InitializeStationTemplates()
    {
        // Templates will be used when generating new stations
    }
    
    /// <summary>
    /// Create a station in a sector
    /// </summary>
    public Station CreateStation(StationData stationData)
    {
        var station = new Station
        {
            Name = stationData.Name,
            Type = Enum.Parse<StationType>(stationData.StationType)
        };
        
        // Set up production based on type
        switch (station.Type)
        {
            case StationType.Mine:
                station.Recipes.Add(new ProductionRecipe
                {
                    Output = GoodType.IronOre,
                    OutputAmount = 100,
                    Inputs = new Dictionary<GoodType, int>(),
                    ProductionTime = 10f
                });
                break;
                
            case StationType.Refinery:
                station.Recipes.Add(new ProductionRecipe
                {
                    Output = GoodType.RefinedIron,
                    OutputAmount = 50,
                    Inputs = new Dictionary<GoodType, int> { { GoodType.IronOre, 100 } },
                    ProductionTime = 15f
                });
                break;
                
            case StationType.Factory:
                station.Recipes.Add(new ProductionRecipe
                {
                    Output = GoodType.ShipParts,
                    OutputAmount = 10,
                    Inputs = new Dictionary<GoodType, int> 
                    { 
                        { GoodType.RefinedIron, 50 },
                        { GoodType.Electronics, 20 }
                    },
                    ProductionTime = 30f
                });
                break;
        }
        
        _stations[station.Id] = station;
        return station;
    }
    
    /// <summary>
    /// Buy goods from a station
    /// </summary>
    public bool BuyGoods(Guid stationId, GoodType goodType, int amount, Inventory playerInventory)
    {
        if (!_stations.TryGetValue(stationId, out var station))
        {
            return false;
        }
        
        if (!station.Inventory.ContainsKey(goodType) || station.Inventory[goodType].Quantity < amount)
        {
            return false;
        }
        
        int cost = (int)(station.Inventory[goodType].CurrentPrice * amount);
        
        if (!playerInventory.HasResource(ResourceType.Credits, cost))
        {
            return false;
        }
        
        playerInventory.RemoveResource(ResourceType.Credits, cost);
        station.Inventory[goodType].Quantity -= amount;
        
        // Add to player's cargo (simplified - would need cargo system)
        return true;
    }
    
    /// <summary>
    /// Sell goods to a station
    /// </summary>
    public bool SellGoods(Guid stationId, GoodType goodType, int amount, Inventory playerInventory)
    {
        if (!_stations.TryGetValue(stationId, out var station))
        {
            return false;
        }
        
        // Check if station accepts this good (black markets accept illegal goods)
        if (!station.IsIllegal && IsIllegalGood(goodType))
        {
            return false;
        }
        
        int price = (int)(station.Inventory.GetValueOrDefault(goodType, 
            new TradeGood { CurrentPrice = GetGoodBasePrice(goodType) }).CurrentPrice * amount * 0.9f);
        
        playerInventory.AddResource(ResourceType.Credits, price);
        
        if (!station.Inventory.ContainsKey(goodType))
        {
            station.Inventory[goodType] = new TradeGood 
            { 
                Type = goodType, 
                Quantity = 0,
                BasePrice = GetGoodBasePrice(goodType),
                CurrentPrice = GetGoodBasePrice(goodType)
            };
        }
        station.Inventory[goodType].Quantity += amount;
        
        return true;
    }
    
    /// <summary>
    /// Attack a freighter for piracy
    /// </summary>
    public Dictionary<GoodType, int> AttackFreighter(PirateComponent pirate)
    {
        var loot = new Dictionary<GoodType, int>();
        
        // Random loot
        int lootCount = _random.Next(1, 4);
        for (int i = 0; i < lootCount; i++)
        {
            var goodType = (GoodType)_random.Next(Enum.GetValues<GoodType>().Length);
            int amount = _random.Next(10, 100);
            loot[goodType] = amount;
        }
        
        pirate.StolenGoods += loot.Values.Sum();
        pirate.PirateReputation -= 10; // Become more hostile
        
        return loot;
    }
    
    /// <summary>
    /// Update automated trade routes
    /// </summary>
    private void UpdateTradeRoutes(float deltaTime)
    {
        var traders = _entityManager.GetAllComponents<TraderComponent>();
        
        foreach (var trader in traders)
        {
            foreach (var route in trader.TradeRoutes.Where(r => r.IsActive))
            {
                // Simplified: would actually move goods over time
            }
        }
    }
    
    private bool IsIllegalGood(GoodType type)
    {
        // Weapons could be illegal in some sectors
        return type == GoodType.Weapons;
    }
    
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
    
    /// <summary>
    /// Get all stations in the economy
    /// </summary>
    public IEnumerable<Station> GetAllStations()
    {
        return _stations.Values;
    }
}
