#include "content/ShipyardModuleSystem.h"
#include "ship_editor/KitbashShipBuilderSystem.h"
#include "ship_editor/ShipyardEquipmentSystem.h"
#include "ship_editor/ShipBlueprintLibrarySystem.h"
#include "inventory/ItemizationSystem.h"
#include "inventory/InventorySystem.h"
#include "crafting/ManufacturingQualitySystem.h"
#include "crafting/RecyclingSystem.h"
#include "combat/ScoopableLootSystem.h"
#include "rendering/ItemIconGenerationSystem.h"
#include "ui/FrontendFlowSystem.h"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
static int passed=0,failed=0;
#define TEST(name,expr) do{if(expr){++passed;std::cout<<"[PASS] "<<name<<"\n";}else{++failed;std::cout<<"[FAIL] "<<name<<"\n";}}while(0)

static std::vector<VisualModuleSource> Fixture(){
    return {
        {"shipyard_a_hull_001_hullCompact",2.2f,3.4f,1.1f},
        {"shipyard_a_command_002_cockpit",1.2f,1.6f,.7f},
        {"shipyard_a_propulsion_003_engineBodyFish",1.0f,1.6f,.8f},
        {"shipyard_a_propulsion_004_engineTrumpet",.8f,1.2f,.7f},
        {"shipyard_a_component_005_cargoBox",.8f,1.0f,.7f},
        {"shipyard_a_component_006_instrumentMast",.25f,.45f,.9f},
        {"shipyard_a_hardpoint_007_hardpointBigGun",.6f,.8f,.35f}
    };
}

static ObjMeshData IconMesh(){
    ObjMeshData m;
    m.positions={{-1,-1,0},{1,-1,0},{0,1,0},{0,0,1.2f}};
    m.triangles={{{0,1,2},{-1,-1,-1},{-1,-1,-1},-1},{{0,1,3},{-1,-1,-1},{-1,-1,-1},-1},{{1,2,3},{-1,-1,-1},{-1,-1,-1},-1},{{2,0,3},{-1,-1,-1},{-1,-1,-1},-1}};
    return m;
}

int main(){
    const auto catalog=ShipyardModuleSystem::BuildCatalog(Fixture());
    auto find=[&](const std::string&id){return KitbashShipBuilderSystem::FindRecord(catalog,id);};
    const auto* engine=find("shipyard_a_propulsion_004_engineTrumpet");
    const auto* cargo=find("shipyard_a_component_005_cargoBox");
    TEST("Pass431 item definitions are derived from the certified Shipyard catalog",engine&&cargo);

    const auto engineDef=ItemizationSystem::BuildShipyardDefinition(*engine);
    const auto common=ItemizationSystem::Generate(engineDef,77,ItemRarity::Common,1.0f,1.0f);
    const auto legendary=ItemizationSystem::Generate(engineDef,77,ItemRarity::Legendary,1.0f,1.0f);
    TEST("Pass431 resolved items preserve the physical source module identity",common.sourceModuleId==engine->source.moduleId&&common.instanceId!=common.definitionId);
    TEST("Pass432 rarity scales functional item stats",legendary.Stat(ItemStatKind::Thrust)>common.Stat(ItemStatKind::Thrust));
    TEST("Pass432 higher rarity receives a larger affix budget",legendary.affixes.size()>common.affixes.size());
    TEST("Pass432 item icon identity is deterministic from the resolved item",common.iconKey==ItemizationSystem::Generate(engineDef,77,ItemRarity::Common,1.0f,1.0f).iconKey);

    InventoryComponent inventory(8,10000.0f);
    const auto invItem=ItemizationSystem::ToInventoryItem(legendary);
    TEST("Pass433 manufactured/loot modules remain unique inventory instances",invItem.uniqueInstance&&invItem.maxStackSize==1&&inventory.AddItem(invItem));
    auto duplicate=invItem;duplicate.instanceId+="_other";
    TEST("Pass433 unique equipment does not stack into one slot",inventory.AddItem(duplicate)&&inventory.GetUsedSlotCount()==2);
    const auto serialized=inventory.Serialize();InventoryComponent restored;restored.Deserialize(serialized);
    TEST("Pass433 generated item rarity/quality metadata survives inventory persistence",restored.GetUsedSlotCount()==2&&restored.GetSlot(0)&&restored.GetSlot(0)->item.quality>0.9f);

    ManufacturingContext low;low.rarityCap=ItemRarity::Common;low.seed=445;low.stationTier=2;low.materialPurity=1.0f;low.skills.propulsion=0;
    ManufacturingContext high=low;high.skills.propulsion=100;high.stationTier=5;high.blueprintMastery=1.0f;
    const auto lowCraft=ManufacturingQualitySystem::Manufacture(engineDef,low);
    const auto highCraft=ManufacturingQualitySystem::Manufacture(engineDef,high);
    TEST("Pass434 manufacturing skill materially increases craftsmanship",highCraft.craftsmanship>lowCraft.craftsmanship);
    TEST("Pass434 manufacturing skill scales propulsion performance",highCraft.item.Stat(ItemStatKind::Thrust)>lowCraft.item.Stat(ItemStatKind::Thrust));
    TEST("Pass434 manufacturing quality never changes authored construction identity",highCraft.item.sourceModuleId==lowCraft.item.sourceModuleId);
    TEST("Pass434 blueprint/material rarity cap remains authoritative",highCraft.item.rarity==ItemRarity::Common);

    RecyclingContext field;field.recyclingSkill=0;field.facilityTier=1;
    RecyclingContext station;station.recyclingSkill=100;station.facilityTier=5;
    const auto fieldRecycle=RecyclingSystem::Recycle(legendary,field);
    const auto stationRecycle=RecyclingSystem::Recycle(legendary,station);
    TEST("Pass435 skilled industrial recycling improves recovery efficiency",stationRecycle.efficiency>fieldRecycle.efficiency);
    station.reverseEngineer=true;const auto reverse=RecyclingSystem::Recycle(legendary,station);
    TEST("Pass435 reverse engineering trades material yield for blueprint research",reverse.blueprintResearch>0.0f&&reverse.efficiency<stationRecycle.efficiency);

    auto builder=KitbashShipBuilderSystem::CreateStarter(catalog,"INDUSTRIAL");
    TEST("Pass436 builder root receives physical equipment slots",!builder.equipmentSlots.empty());
    std::string error;
    const bool started=KitbashShipBuilderSystem::BeginDrag(builder,catalog,"shipyard_a_command_002_cockpit",&error);
    const auto preview=started?KitbashShipBuilderSystem::PreviewDrop(builder,catalog,0,"forward"):KitbashShipBuilderPreview{};
    const bool committed=preview.valid&&KitbashShipBuilderSystem::CommitDrop(builder,catalog,preview,&error);
    TEST("Pass436 builder command history records a committed module placement",committed&&KitbashShipBuilderSystem::CanUndo(builder));
    const auto modulesAfter=builder.blueprint.modules.size();
    TEST("Pass436 Ctrl-Z/Ctrl-Y command semantics are backed by real undo/redo state",KitbashShipBuilderSystem::Undo(builder)&&builder.blueprint.modules.size()+1==modulesAfter&&KitbashShipBuilderSystem::Redo(builder)&&builder.blueprint.modules.size()==modulesAfter);
    bool cockpitSlots=false;for(const auto&s:builder.equipmentSlots)if(s.moduleIndex==1&&s.type==ShipEquipmentSlotType::Command)cockpitSlots=true;
    TEST("Pass437 each functional physical part exposes typed equipment slots",cockpitSlots);

    ShipEquipmentSlot engineSlot;engineSlot.type=ShipEquipmentSlotType::MainEngine;engineSlot.size=engine->size;
    TEST("Pass437 generated engine item can install into a compatible equipment slot",ShipyardEquipmentSystem::Install(engineSlot,legendary,&error)&&!engineSlot.installedItemInstanceId.empty());
    const auto cargoDef=ItemizationSystem::BuildShipyardDefinition(*cargo);const auto cargoItem=ItemizationSystem::Generate(cargoDef,91,ItemRarity::Rare,1.0f,1.0f);
    ShipEquipmentSlot sensorSlot;sensorSlot.type=ShipEquipmentSlotType::Sensor;
    TEST("Pass437 incompatible equipment is rejected instead of silently fitting",!ShipyardEquipmentSystem::Install(sensorSlot,cargoItem,&error));

    const auto icon=ItemIconGenerationSystem::Generate(legendary,IconMesh(),64);
    std::size_t opaque=0;for(std::size_t i=3;i<icon.rgba.size();i+=4)if(icon.rgba[i])++opaque;
    TEST("Pass438 item icons are rendered from actual item geometry",icon.width==64&&icon.height==64&&opaque>64);
    TEST("Pass438 generated icon cache identity matches resolved item identity",icon.cacheKey==legendary.iconKey);

    ProceduralShipVisualRecipe destroyed=builder.blueprint;destroyed.recipeId="test_raider";destroyed.manufacturerFamily="TEST_FLEET";
    auto drops=ScoopableLootSystem::BuildDestroyedShipDrops(destroyed,catalog,{10,4,0},1234,1.0f,1.0f);
    bool hasFragment=false,allCompact=!drops.empty(),hasModule=false;
    for(const auto&d:drops){allCompact&=d.compactScale<=.20f;if(d.item.kind==ItemKind::BlueprintFragment){hasFragment=true;TEST("Pass440 blueprint fragment loot preserves design identity",d.item.blueprintId==destroyed.recipeId&&d.item.blueprintFragmentsRequired==5);}if(d.item.kind==ItemKind::ShipModule)hasModule=true;}
    TEST("Pass439 destroyed ships produce compact scoopable physical module loot",hasModule&&allCompact);
    TEST("Pass440 destroyed ships can produce blueprint-fragment item drops",hasFragment);
    std::vector<GeneratedItem> collected;for(auto&d:drops){d.position={0.1f,0,0};d.pickupRadius=1.0f;}ScoopableLootSystem::Update(drops,{0,0,0},5.0f,.016f,&collected);
    TEST("Pass439 scoopable loot retains its resolved instance when collected",!collected.empty()&&!collected.front().instanceId.empty());

    ShipBlueprintDocument doc;doc.name="Forge Test";doc.author="TEST";doc.recipe=builder.blueprint;doc.equipmentSlots=builder.equipmentSlots;doc.appearance.primary.r=.22f;doc.appearance.factoryWear=.13f;ShipDecalLayer decal;decal.id="mark";decal.decalAsset="test_decal";doc.appearance.decals.push_back(decal);doc.blueprintId=ShipBlueprintLibrarySystem::CanonicalId(doc);
    const auto temp=std::filesystem::temp_directory_path()/"subspace_pass431_450_test.subspace_blueprint";
    const bool saved=ShipBlueprintLibrarySystem::Save(doc,temp.string(),&error);ShipBlueprintDocument loaded;const bool loadedOk=saved&&ShipBlueprintLibrarySystem::Load(temp.string(),loaded,&error);std::error_code ec;std::filesystem::remove(temp,ec);
    TEST("Pass441 blueprint save/load preserves complete assembly",loadedOk&&loaded.recipe.modules.size()==doc.recipe.modules.size()&&loaded.recipe.attachments.size()==doc.recipe.attachments.size());
    TEST("Pass441 blueprint save/load preserves equipment and appearance",loadedOk&&loaded.equipmentSlots.size()==doc.equipmentSlots.size()&&std::fabs(loaded.appearance.primary.r-.22f)<.001f&&loaded.appearance.decals.size()==1);

    FrontendFlowSystem frontend;frontend.MoveMainMenu(+1);
    TEST("Pass442 standalone main-menu Shipyard is a first-class frontend choice",frontend.MainMenuIndex()==1);

    std::cout<<"\n=== Pass431-450 Shipyard Itemization / Manufacturing / Loot: "<<passed<<" passed, "<<failed<<" failed ===\n";
    return failed?1:0;
}
