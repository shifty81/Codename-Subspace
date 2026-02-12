#include "ships/Blueprint.h"
#include "ships/BlockPlacement.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace subspace {

// ---------------------------------------------------------------------------
// JSON helpers (minimal, no external library)
// ---------------------------------------------------------------------------
namespace {

std::string EscapeJson(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    return out;
}

// Skip whitespace in json starting at pos
void SkipWS(const std::string& json, size_t& pos) {
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' ||
           json[pos] == '\n' || json[pos] == '\r')) {
        ++pos;
    }
}

// Parse a quoted string starting at pos (pos should point to opening '"')
std::string ParseString(const std::string& json, size_t& pos) {
    if (pos >= json.size() || json[pos] != '"') return {};
    ++pos; // skip opening quote
    std::string result;
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            ++pos;
            switch (json[pos]) {
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                default:   result += json[pos]; break;
            }
        } else {
            result += json[pos];
        }
        ++pos;
    }
    if (pos < json.size()) ++pos; // skip closing quote
    return result;
}

// Parse an integer starting at pos (with overflow protection)
int ParseInt(const std::string& json, size_t& pos) {
    SkipWS(json, pos);
    size_t start = pos;
    if (pos < json.size() && json[pos] == '-') ++pos;
    while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9') ++pos;
    if (pos == start) return 0;
    try {
        return std::stoi(json.substr(start, pos - start));
    } catch (const std::out_of_range&) {
        return 0;
    } catch (const std::invalid_argument&) {
        return 0;
    }
}

// Find the next occurrence of key "key": in json starting at pos
bool FindKey(const std::string& json, size_t& pos, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t found = json.find(search, pos);
    if (found == std::string::npos) return false;
    pos = found + search.size();
    SkipWS(json, pos);
    if (pos < json.size() && json[pos] == ':') ++pos;
    SkipWS(json, pos);
    return true;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Blueprint
// ---------------------------------------------------------------------------
std::string Blueprint::ToJson() const {
    std::ostringstream ss;
    ss << "{\"name\":\"" << EscapeJson(name) << "\","
       << "\"author\":\"" << EscapeJson(author) << "\","
       << "\"faction\":\"" << EscapeJson(faction) << "\","
       << "\"seed\":" << seed << ","
       << "\"blocks\":[";

    for (size_t i = 0; i < blocks.size(); ++i) {
        const auto& b = blocks[i];
        if (i > 0) ss << ',';
        ss << "{\"pos\":[" << b.posX << ',' << b.posY << ',' << b.posZ << "],"
           << "\"size\":[" << b.sizeX << ',' << b.sizeY << ',' << b.sizeZ << "],"
           << "\"shape\":" << b.shape << ","
           << "\"type\":" << b.type << ","
           << "\"material\":" << b.material << ","
           << "\"rot\":" << b.rotationIndex << '}';
    }
    ss << "]}";
    return ss.str();
}

Blueprint Blueprint::FromJson(const std::string& json) {
    Blueprint bp;
    size_t pos = 0;

    // Parse top-level fields
    pos = 0;
    if (FindKey(json, pos, "name"))    bp.name    = ParseString(json, pos);
    pos = 0;
    if (FindKey(json, pos, "author"))  bp.author  = ParseString(json, pos);
    pos = 0;
    if (FindKey(json, pos, "faction")) bp.faction = ParseString(json, pos);
    pos = 0;
    if (FindKey(json, pos, "seed"))    bp.seed    = ParseInt(json, pos);

    // Parse blocks array
    pos = 0;
    if (!FindKey(json, pos, "blocks")) return bp;
    if (pos >= json.size() || json[pos] != '[') return bp;
    ++pos; // skip '['

    while (pos < json.size()) {
        SkipWS(json, pos);
        if (pos >= json.size() || json[pos] == ']') break;
        if (json[pos] == ',') { ++pos; continue; }
        if (json[pos] != '{') break;

        // Find the end of this block object
        size_t objStart = pos;
        int depth = 0;
        size_t objEnd = pos;
        for (size_t i = pos; i < json.size(); ++i) {
            if (json[i] == '{') ++depth;
            if (json[i] == '}') { --depth; if (depth == 0) { objEnd = i + 1; break; } }
        }

        BlueprintBlockData bd{};

        // Parse "pos" array
        size_t bpos = objStart;
        if (FindKey(json, bpos, "pos") && bpos < objEnd && json[bpos] == '[') {
            ++bpos;
            bd.posX = ParseInt(json, bpos);
            SkipWS(json, bpos); if (bpos < objEnd && json[bpos] == ',') ++bpos;
            bd.posY = ParseInt(json, bpos);
            SkipWS(json, bpos); if (bpos < objEnd && json[bpos] == ',') ++bpos;
            bd.posZ = ParseInt(json, bpos);
        }

        // Parse "size" array
        bpos = objStart;
        if (FindKey(json, bpos, "size") && bpos < objEnd && json[bpos] == '[') {
            ++bpos;
            bd.sizeX = ParseInt(json, bpos);
            SkipWS(json, bpos); if (bpos < objEnd && json[bpos] == ',') ++bpos;
            bd.sizeY = ParseInt(json, bpos);
            SkipWS(json, bpos); if (bpos < objEnd && json[bpos] == ',') ++bpos;
            bd.sizeZ = ParseInt(json, bpos);
        }

        // Parse scalar fields
        bpos = objStart;
        if (FindKey(json, bpos, "shape") && bpos < objEnd)    bd.shape = ParseInt(json, bpos);
        bpos = objStart;
        if (FindKey(json, bpos, "type") && bpos < objEnd)     bd.type = ParseInt(json, bpos);
        bpos = objStart;
        if (FindKey(json, bpos, "material") && bpos < objEnd) bd.material = ParseInt(json, bpos);
        bpos = objStart;
        if (FindKey(json, bpos, "rot") && bpos < objEnd)      bd.rotationIndex = ParseInt(json, bpos);

        bp.blocks.push_back(bd);
        pos = objEnd;
    }

    return bp;
}

Blueprint Blueprint::FromShip(const Ship& ship, const std::string& bpName,
                              const std::string& bpAuthor) {
    Blueprint bp;
    bp.name    = bpName;
    bp.author  = bpAuthor;
    bp.faction = ship.faction;
    bp.seed    = ship.seed;

    bp.blocks.reserve(ship.blocks.size());
    for (const auto& block : ship.blocks) {
        BlueprintBlockData bd{};
        bd.posX  = block->gridPos.x;
        bd.posY  = block->gridPos.y;
        bd.posZ  = block->gridPos.z;
        bd.sizeX = block->size.x;
        bd.sizeY = block->size.y;
        bd.sizeZ = block->size.z;
        bd.rotationIndex = block->rotationIndex;
        bd.shape    = static_cast<int>(block->shape);
        bd.type     = static_cast<int>(block->type);
        bd.material = static_cast<int>(block->material);
        bp.blocks.push_back(bd);
    }

    return bp;
}

Ship Blueprint::ToShip() const {
    Ship ship;
    ship.name    = name;
    ship.faction = faction;
    ship.seed    = seed;

    for (const auto& bd : blocks) {
        // Validate enum ranges before casting
        if (bd.shape < 0 || bd.shape > static_cast<int>(BlockShape::Slope))
            continue;
        if (bd.type < 0 || bd.type > static_cast<int>(BlockType::WeaponMount))
            continue;
        if (bd.material < 0 || bd.material > static_cast<int>(MaterialType::Avorion))
            continue;
        // Validate block dimensions are positive
        if (bd.sizeX <= 0 || bd.sizeY <= 0 || bd.sizeZ <= 0)
            continue;

        auto block = std::make_shared<Block>();
        block->gridPos       = { bd.posX, bd.posY, bd.posZ };
        block->size          = { bd.sizeX, bd.sizeY, bd.sizeZ };
        block->rotationIndex = bd.rotationIndex % 4;
        block->shape         = static_cast<BlockShape>(bd.shape);
        block->type          = static_cast<BlockType>(bd.type);
        block->material      = static_cast<MaterialType>(bd.material);

        const MaterialStats& mats = MaterialDatabase::Get(block->material);
        block->maxHP     = GetBlockBaseHP(block->type) * mats.hpMultiplier;
        block->currentHP = block->maxHP;

        BlockPlacement::Place(ship, block);
    }

    return ship;
}

bool Blueprint::Validate() const {
    if (blocks.empty()) return false;

    for (const auto& bd : blocks) {
        if (bd.shape < 0 || bd.shape > static_cast<int>(BlockShape::Slope))
            return false;
        if (bd.type < 0 || bd.type > static_cast<int>(BlockType::WeaponMount))
            return false;
        if (bd.material < 0 || bd.material > static_cast<int>(MaterialType::Avorion))
            return false;
    }
    return true;
}

} // namespace subspace
