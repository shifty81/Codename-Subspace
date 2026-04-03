#include "voxel/VoxelSystem.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

namespace subspace {

// ---------------------------------------------------------------------------
// MaterialProperties
// ---------------------------------------------------------------------------

const MaterialProperties& MaterialProperties::Get(const std::string& mat) {
    static const std::unordered_map<std::string, MaterialProperties> table = {
        { "Iron",     { 1.0f, 1.0f, 1.0f, 1.0f, 0xB2B2B2 } },
        { "Titanium", { 0.9f, 1.4f, 1.1f, 1.1f, 0xC8DCF0 } },
        { "Naonite",  { 0.8f, 1.8f, 1.3f, 1.3f, 0x33CC4D } },
        { "Trinium",  { 0.7f, 2.2f, 1.5f, 1.5f, 0x4D99E6 } },
        { "Xanion",   { 0.6f, 2.8f, 1.7f, 1.8f, 0xE6B233 } },
        { "Ogonite",  { 0.5f, 3.5f, 1.9f, 2.0f, 0xE64D4D } },
        { "Avorion",  { 0.4f, 4.5f, 2.2f, 2.5f, 0xCC33E6 } },
    };
    auto it = table.find(mat);
    if (it != table.end()) return it->second;
    static const MaterialProperties fallback{};
    return fallback;
}

// ---------------------------------------------------------------------------
// VoxelBlock
// ---------------------------------------------------------------------------

VoxelBlock VoxelBlock::Make(float px, float py, float pz,
                             float sx, float sy, float sz,
                             const std::string& material,
                             BlockType type,
                             BlockShape shape,
                             BlockOrientation orient) {
    VoxelBlock b;
    b.px = px; b.py = py; b.pz = pz;
    b.sx = sx; b.sy = sy; b.sz = sz;
    b.material    = material;
    b.blockType   = type;
    b.shape       = shape;
    b.orientation = orient;

    const auto& mp = MaterialProperties::Get(material);
    b.colorRGB = mp.color;

    float volume = sx * sy * sz;
    if (shape == BlockShape::Wedge || shape == BlockShape::HalfBlock)
        volume *= 0.5f;
    else if (shape == BlockShape::Corner || shape == BlockShape::Tetrahedron)
        volume *= 0.25f;
    else if (shape == BlockShape::InnerCorner)
        volume *= 0.75f;
    else if (shape == BlockShape::SlopedPlate)
        volume *= 0.3f;

    b.mass          = volume * mp.massMultiplier;
    b.maxDurability = 100.0f * mp.durabilityMultiplier * volume;
    b.durability    = b.maxDurability;

    switch (type) {
        case BlockType::Armor:
            b.maxDurability *= 5.0f;
            b.durability     = b.maxDurability;
            b.mass          *= 1.5f;
            break;
        case BlockType::Framework:
            b.maxDurability *= 0.2f;
            b.durability     = b.maxDurability;
            b.mass          *= 0.1f;
            break;
        case BlockType::Engine:
            b.thrustPower = 50.0f * volume * mp.energyEfficiency;
            break;
        case BlockType::Thruster:
            b.thrustPower = 30.0f * volume * mp.energyEfficiency;
            break;
        case BlockType::GyroArray:
            b.thrustPower = 20.0f * volume * mp.energyEfficiency;
            break;
        case BlockType::Generator:
            b.powerGen = 100.0f * volume * mp.energyEfficiency;
            break;
        case BlockType::ShieldGenerator:
            b.shieldCap = 200.0f * volume * mp.shieldMultiplier;
            break;
        default:
            break;
    }

    return b;
}

void VoxelBlock::TakeDamage(float damage) {
    durability -= damage;
    if (durability <= 0.0f) {
        durability  = 0.0f;
        isDestroyed = true;
    }
}

bool VoxelBlock::Intersects(const VoxelBlock& o) const {
    return (px < o.px + o.sx && px + sx > o.px) &&
           (py < o.py + o.sy && py + sy > o.py) &&
           (pz < o.pz + o.sz && pz + sz > o.pz);
}

void VoxelBlock::Serialize(ComponentData& out) const {
    out.data["px"] = std::to_string(px);
    out.data["py"] = std::to_string(py);
    out.data["pz"] = std::to_string(pz);
    out.data["sx"] = std::to_string(sx);
    out.data["sy"] = std::to_string(sy);
    out.data["sz"] = std::to_string(sz);
    out.data["mat"] = material;
    out.data["bt"]  = std::to_string(static_cast<int>(blockType));
    out.data["bs"]  = std::to_string(static_cast<int>(shape));
    out.data["bo"]  = std::to_string(static_cast<int>(orientation));
    out.data["dur"] = std::to_string(durability);
    out.data["mxd"] = std::to_string(maxDurability);
    out.data["col"] = std::to_string(colorRGB);
    out.data["dst"] = isDestroyed ? "1" : "0";
}

VoxelBlock VoxelBlock::Deserialize(const ComponentData& in) {
    auto get = [&](const std::string& k, float def = 0.0f) -> float {
        auto it = in.data.find(k);
        return (it != in.data.end()) ? std::stof(it->second) : def;
    };
    auto gets = [&](const std::string& k, const std::string& def = "") -> const std::string& {
        auto it = in.data.find(k);
        return (it != in.data.end()) ? it->second : def;
    };

    VoxelBlock b;
    b.px = get("px"); b.py = get("py"); b.pz = get("pz");
    b.sx = get("sx", 1.0f); b.sy = get("sy", 1.0f); b.sz = get("sz", 1.0f);
    b.material    = gets("mat", "Iron");
    b.blockType   = static_cast<BlockType>(std::stoi(gets("bt", "0")));
    b.shape       = static_cast<BlockShape>(std::stoi(gets("bs", "0")));
    b.orientation = static_cast<BlockOrientation>(std::stoi(gets("bo", "0")));
    b.durability  = get("dur", 100.0f);
    b.maxDurability = get("mxd", 100.0f);
    b.colorRGB    = static_cast<uint32_t>(std::stoul(gets("col", "0xB2B2B2")));
    b.isDestroyed = (gets("dst", "0") == "1");
    return b;
}

// ---------------------------------------------------------------------------
// VoxelStructureComponent
// ---------------------------------------------------------------------------

VoxelBlock& VoxelStructureComponent::AddBlock(VoxelBlock block) {
    blocks.push_back(std::move(block));
    return blocks.back();
}

bool VoxelStructureComponent::RemoveBlock(size_t index) {
    if (index >= blocks.size()) return false;
    blocks.erase(blocks.begin() + static_cast<ptrdiff_t>(index));
    return true;
}

size_t VoxelStructureComponent::ActiveBlockCount() const {
    size_t count = 0;
    for (const auto& b : blocks)
        if (!b.isDestroyed) ++count;
    return count;
}

float VoxelStructureComponent::TotalMass() const {
    float m = 0.0f;
    for (const auto& b : blocks)
        if (!b.isDestroyed) m += b.mass;
    return m;
}

float VoxelStructureComponent::TotalThrust() const {
    float t = 0.0f;
    for (const auto& b : blocks)
        if (!b.isDestroyed) t += b.thrustPower;
    return t;
}

float VoxelStructureComponent::TotalPowerGen() const {
    float p = 0.0f;
    for (const auto& b : blocks)
        if (!b.isDestroyed) p += b.powerGen;
    return p;
}

float VoxelStructureComponent::TotalShieldCap() const {
    float s = 0.0f;
    for (const auto& b : blocks)
        if (!b.isDestroyed) s += b.shieldCap;
    return s;
}

ComponentData VoxelStructureComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "VoxelStructureComponent";
    cd.data["blockCount"] = std::to_string(blocks.size());
    for (size_t i = 0; i < blocks.size(); ++i) {
        ComponentData bd;
        blocks[i].Serialize(bd);
        for (auto& [k, v] : bd.data)
            cd.data["b" + std::to_string(i) + "_" + k] = v;
    }
    return cd;
}

void VoxelStructureComponent::Deserialize(const ComponentData& data) {
    blocks.clear();
    int count = 0;
    auto it = data.data.find("blockCount");
    if (it != data.data.end()) count = std::stoi(it->second);
    for (int i = 0; i < count; ++i) {
        ComponentData bd;
        bd.componentType = "VoxelBlock";
        std::string prefix = "b" + std::to_string(i) + "_";
        for (const auto& [k, v] : data.data) {
            if (k.rfind(prefix, 0) == 0)
                bd.data[k.substr(prefix.size())] = v;
        }
        blocks.push_back(VoxelBlock::Deserialize(bd));
    }
}

// ---------------------------------------------------------------------------
// VoxelDamageComponent
// ---------------------------------------------------------------------------

ComponentData VoxelDamageComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "VoxelDamageComponent";
    cd.data["showDamage"] = showDamage ? "1" : "0";
    cd.data["count"] = std::to_string(damageVoxels.size());
    return cd;
}

void VoxelDamageComponent::Deserialize(const ComponentData& data) {
    auto it = data.data.find("showDamage");
    showDamage = (it != data.data.end()) ? (it->second == "1") : true;
    damageVoxels.clear();
    moduleDamageMap.clear();
}

// ---------------------------------------------------------------------------
// VoxelDamageSystem
// ---------------------------------------------------------------------------

VoxelDamageSystem::VoxelDamageSystem() : SystemBase("VoxelDamageSystem") {}
VoxelDamageSystem::VoxelDamageSystem(EntityManager& em)
    : SystemBase("VoxelDamageSystem"), _em(&em) {}

void VoxelDamageSystem::SetEntityManager(EntityManager* em) { _em = em; }

void VoxelDamageSystem::Update(float /*deltaTime*/) {
    // Damage visualization is rebuilt on-demand via ApplyDamageToBlock /
    // RepairBlock; nothing to do each frame unless we want fade effects.
}

void VoxelDamageSystem::ApplyDamageToBlock(uint64_t entityId, size_t blockIndex,
                                           float damage) {
    if (!_em) return;
    auto* structure = _em->GetComponent<VoxelStructureComponent>(entityId);
    if (!structure || blockIndex >= structure->blocks.size()) return;

    structure->blocks[blockIndex].TakeDamage(damage);

    auto* dmgComp = _em->GetComponent<VoxelDamageComponent>(entityId);
    if (!dmgComp) return;
    if (dmgComp->showDamage)
        RebuildDamageOverlay(entityId, *structure, *dmgComp);
}

void VoxelDamageSystem::RepairBlock(uint64_t entityId, size_t blockIndex,
                                    float repairAmount) {
    if (!_em) return;
    auto* structure = _em->GetComponent<VoxelStructureComponent>(entityId);
    if (!structure || blockIndex >= structure->blocks.size()) return;

    auto& b = structure->blocks[blockIndex];
    b.durability = std::min(b.durability + repairAmount, b.maxDurability);
    b.isDestroyed = (b.durability <= 0.0f);

    auto* dmgComp = _em->GetComponent<VoxelDamageComponent>(entityId);
    if (dmgComp && dmgComp->showDamage)
        RebuildDamageOverlay(entityId, *structure, *dmgComp);
}

void VoxelDamageSystem::ClearDamageVisualization(uint64_t entityId) {
    if (!_em) return;
    auto* dmgComp = _em->GetComponent<VoxelDamageComponent>(entityId);
    if (!dmgComp) return;
    dmgComp->damageVoxels.clear();
    dmgComp->moduleDamageMap.clear();
}

void VoxelDamageSystem::RebuildDamageOverlay(uint64_t entityId,
                                              VoxelStructureComponent& structure,
                                              VoxelDamageComponent& damage) {
    damage.damageVoxels.clear();
    damage.moduleDamageMap.clear();

    for (size_t i = 0; i < structure.blocks.size(); ++i) {
        const auto& b = structure.blocks[i];
        if (b.isDestroyed) {
            // Represent each destroyed block as a damage overlay voxel
            VoxelBlock overlay = b;
            damage.damageVoxels.push_back(overlay);
            damage.moduleDamageMap[entityId].push_back(overlay);
        }
    }
}

} // namespace subspace
