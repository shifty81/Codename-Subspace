#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

enum class TacticalContactKind { HostileShip, FriendlyShip, NeutralShip, Station, Planet, Moon, Asteroid, Wreck, Site, Fleet };
enum class TacticalContactPreset { Combat, Mining, Travel, Salvage, Exploration, Fleet, All };

struct TacticalContactRow {
    std::uint64_t id = 0;
    TacticalContactKind kind = TacticalContactKind::NeutralShip;
    std::string name;
    std::string type;
    float range = 0.0f;
    float velocity = 0.0f;
    float angular = 0.0f;
    int threat = 0;
    bool selected = false;
    bool locked = false;
    // Opaque application source identity. Keeping this in the tactical model
    // makes a Contacts row a real selection/targeting surface instead of a
    // painted copy that must repick the 3D scene behind the window.
    int sourceKind = 0;
    std::size_t sourceIndex = 0;
    std::string sourceId;
};

struct TacticalContactsLayout {
    float x=0.0f;
    float y=0.0f;
    float width=350.0f;
    float titleHeight=28.0f;
    float tabHeight=22.0f;
    float columnHeight=23.0f;
    float rowHeight=25.0f;
    float RowsY() const { return y+titleHeight+tabHeight+columnHeight; }
    float Height(int rows) const { return titleHeight+tabHeight+columnHeight+rowHeight*rows+8.0f; }
    bool Contains(float px,float py,int rows) const { return px>=x&&px<=x+width&&py>=y&&py<=y+Height(rows); }
};

struct TacticalContactsModel {
    TacticalContactPreset preset = TacticalContactPreset::All;
    std::vector<TacticalContactRow> rows;
    std::string sortColumn = "RANGE";
    bool sortAscending = true;
    bool freezeSort = false;
    int maxVisibleRows = 10;
};

class TacticalContactsSystem {
public:
    static bool VisibleInPreset(TacticalContactKind kind,TacticalContactPreset preset);
    static void Sort(TacticalContactsModel& model);
    static const char* PresetName(TacticalContactPreset preset);
    static const char* PresetShortName(TacticalContactPreset preset);
    static TacticalContactsLayout Layout(int viewportWidth,int viewportHeight,int visibleRows);
    static int HitTestRow(const TacticalContactsModel& model,int viewportWidth,int viewportHeight,float x,float y);
    static int HitTestPreset(const TacticalContactsModel& model,int viewportWidth,int viewportHeight,float x,float y);
    static TacticalContactPreset PresetFromIndex(int index);
};

} // namespace subspace
