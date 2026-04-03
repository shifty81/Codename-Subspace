#include "ships/ModularShipFactory.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// ModularShipComponent
// ---------------------------------------------------------------------------

void ModularShipComponent::RecalculateStats() {
    mass = shieldCap = thrust = powerGen = 0.0f;
    for (const auto& slot : slots) {
        if (!slot.occupied) continue;
        // Slot contributions are handled externally (e.g. WeaponSystem);
        // here we just clear dirty flag after traversal.
    }
    statsDirty = false;
}

ComponentData ModularShipComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "ModularShipComponent";
    cd.data["class"]    = shipClass;
    cd.data["hull"]     = hullMaterial;
    cd.data["mass"]     = std::to_string(mass);
    cd.data["thrust"]   = std::to_string(thrust);
    cd.data["shield"]   = std::to_string(shieldCap);
    cd.data["power"]    = std::to_string(powerGen);
    cd.data["slots"]    = std::to_string(slots.size());
    for (size_t i = 0; i < slots.size(); ++i) {
        std::string p = "s" + std::to_string(i) + "_";
        cd.data[p + "id"]       = slots[i].slotId;
        cd.data[p + "type"]     = slots[i].slotType;
        cd.data[p + "occ"]      = slots[i].occupied ? "1" : "0";
        cd.data[p + "module"]   = slots[i].occupiedBy;
    }
    return cd;
}

void ModularShipComponent::Deserialize(const ComponentData& data) {
    auto f = [&](const std::string& k, const std::string& def = "") {
        auto it = data.data.find(k);
        return (it != data.data.end()) ? it->second : def;
    };
    shipClass    = f("class");
    hullMaterial = f("hull", "Iron");
    mass         = std::stof(f("mass", "0"));
    thrust       = std::stof(f("thrust", "0"));
    shieldCap    = std::stof(f("shield", "0"));
    powerGen     = std::stof(f("power", "0"));
    int n = std::stoi(f("slots", "0"));
    slots.clear();
    for (int i = 0; i < n; ++i) {
        std::string p = "s" + std::to_string(i) + "_";
        ShipModuleSlot s;
        s.slotId     = f(p + "id");
        s.slotType   = f(p + "type");
        s.occupied   = (f(p + "occ", "0") == "1");
        s.occupiedBy = f(p + "module");
        slots.push_back(s);
    }
}

// ---------------------------------------------------------------------------
// ModularShipFactory
// ---------------------------------------------------------------------------

ModularShipFactory::ModularShipFactory(EntityManager& em) : _em(em) {}

uint64_t ModularShipFactory::CreateShip(const ModularShipBlueprint& bp,
                                         float spawnX, float spawnY, float spawnZ) {
    auto& entity = _em.CreateEntity(bp.name);

    auto comp = std::make_unique<ModularShipComponent>();
    comp->shipClass    = bp.shipClass;
    comp->hullMaterial = bp.hullMaterial;

    PopulateSlots(*comp, bp);
    comp->RecalculateStats();

    _em.AddComponent<ModularShipComponent>(entity.id, std::move(comp));

    // Add a minimal VoxelStructureComponent for the hull
    auto hull = std::make_unique<VoxelStructureComponent>();
    hull->AddBlock(VoxelBlock::Make(spawnX, spawnY, spawnZ,
                                    bp.hullLength, bp.hullHeight, bp.hullWidth,
                                    bp.hullMaterial, BlockType::Hull));
    _em.AddComponent<VoxelStructureComponent>(entity.id, std::move(hull));

    return entity.id;
}

void ModularShipFactory::PopulateSlots(ModularShipComponent& comp,
                                        const ModularShipBlueprint& bp) {
    for (int i = 0; i < bp.weaponSlots; ++i) {
        ShipModuleSlot s;
        s.slotId   = "weapon_" + std::to_string(i);
        s.slotType = "Weapon";
        if (i < static_cast<int>(bp.defaultWeapons.size())) {
            s.occupied   = true;
            s.occupiedBy = bp.defaultWeapons[static_cast<size_t>(i)];
        }
        comp.slots.push_back(s);
    }
    for (int i = 0; i < bp.engineSlots; ++i) {
        ShipModuleSlot s;
        s.slotId   = "engine_" + std::to_string(i);
        s.slotType = "Engine";
        comp.slots.push_back(s);
    }
    for (int i = 0; i < bp.utilitySlots; ++i) {
        ShipModuleSlot s;
        s.slotId   = "utility_" + std::to_string(i);
        s.slotType = "Utility";
        if (i < static_cast<int>(bp.defaultUtilities.size())) {
            s.occupied   = true;
            s.occupiedBy = bp.defaultUtilities[static_cast<size_t>(i)];
        }
        comp.slots.push_back(s);
    }
    comp.statsDirty = false;
}

ModularShipBlueprint ModularShipFactory::GetPreset(const std::string& name) {
    if (name == "Fighter") {
        return { "Fighter", "Fighter", "Iron", 15.0f, 6.0f, 4.0f,
                 2, 1, 1, { "Pulse Cannon", "Missile Pod" }, { "Scanner" } };
    }
    if (name == "Cruiser") {
        return { "Cruiser", "Cruiser", "Titanium", 50.0f, 15.0f, 10.0f,
                 4, 2, 3, { "Heavy Turret", "Heavy Turret", "Torpedo Bay", "Flak Battery" },
                 { "Shield Booster", "Repair Drone Bay", "Advanced Sensors" } };
    }
    if (name == "Freighter") {
        return { "Freighter", "Freighter", "Iron", 60.0f, 20.0f, 12.0f,
                 1, 2, 4, { "Defensive Turret" },
                 { "Cargo Expander", "Fuel Efficiency", "Navigation Suite", "Docking Aid" } };
    }
    // Default fallback
    return { name, name, "Iron", 20.0f, 8.0f, 5.0f, 2, 1, 2 };
}

std::vector<std::string> ModularShipFactory::ListPresets() {
    return { "Fighter", "Cruiser", "Freighter" };
}

// ---------------------------------------------------------------------------
// ModularShipSyncSystem
// ---------------------------------------------------------------------------

ModularShipSyncSystem::ModularShipSyncSystem() : SystemBase("ModularShipSyncSystem") {}
ModularShipSyncSystem::ModularShipSyncSystem(EntityManager& em)
    : SystemBase("ModularShipSyncSystem"), _em(&em) {}

void ModularShipSyncSystem::SetEntityManager(EntityManager* em) { _em = em; }

void ModularShipSyncSystem::Update(float /*deltaTime*/) {
    if (!_em) return;
    auto ships = _em->GetAllComponents<ModularShipComponent>();
    for (auto* ship : ships) {
        if (ship && ship->statsDirty)
            ship->RecalculateStats();
    }
}

} // namespace subspace
