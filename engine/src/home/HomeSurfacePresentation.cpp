#include "home/HomeSurfacePresentation.h"

#include <algorithm>

namespace subspace {

const char* HomeSurfaceTileKindName(HomeSurfaceTileKind kind) {
    switch (kind) {
    case HomeSurfaceTileKind::Plain: return "Plain";
    case HomeSurfaceTileKind::ResourcePatch: return "ResourcePatch";
    case HomeSurfaceTileKind::Foundation: return "Foundation";
    case HomeSurfaceTileKind::Road: return "Road";
    case HomeSurfaceTileKind::Conveyor: return "Conveyor";
    case HomeSurfaceTileKind::PowerLine: return "PowerLine";
    case HomeSurfaceTileKind::Blocked: return "Blocked";
    case HomeSurfaceTileKind::Water: return "Water";
    }
    return "Unknown";
}

HomeSurfaceTileVisual MakeHomeSurfaceTile(HomeSurfaceTileKind kind, float richness) {
    HomeSurfaceTileVisual tile;
    tile.kind = kind;
    tile.resourceRichness = std::max(0.0f, richness);
    switch (kind) {
    case HomeSurfaceTileKind::Plain: tile.fillColor = 0x18262Fu; tile.edgeColor = 0x253A45u; tile.glyph = '.'; break;
    case HomeSurfaceTileKind::ResourcePatch: tile.fillColor = 0x514224u; tile.edgeColor = 0xA7792Eu; tile.glyph = '*'; break;
    case HomeSurfaceTileKind::Foundation: tile.fillColor = 0x2F3B46u; tile.edgeColor = 0x7E91A0u; tile.glyph = '#'; break;
    case HomeSurfaceTileKind::Road: tile.fillColor = 0x31302Fu; tile.edgeColor = 0x6C6761u; tile.glyph = '='; break;
    case HomeSurfaceTileKind::Conveyor: tile.fillColor = 0x3A2D1Au; tile.edgeColor = 0xE0B15Au; tile.glyph = '>'; break;
    case HomeSurfaceTileKind::PowerLine: tile.fillColor = 0x232735u; tile.edgeColor = 0x73B8FFu; tile.glyph = '+'; break;
    case HomeSurfaceTileKind::Blocked: tile.fillColor = 0x171A1Du; tile.edgeColor = 0x3A3F45u; tile.glyph = 'x'; break;
    case HomeSurfaceTileKind::Water: tile.fillColor = 0x12394Cu; tile.edgeColor = 0x2CAAD9u; tile.glyph = '~'; break;
    }
    return tile;
}

HomeSurfaceFrameVisual CreateStarterHomeSurfaceFrame(int width, int height) {
    HomeSurfaceFrameVisual frame;
    frame.width = std::max(1, width);
    frame.height = std::max(1, height);
    frame.tiles.resize(static_cast<std::size_t>(frame.width * frame.height));
    for (int y = 0; y < frame.height; ++y) {
        for (int x = 0; x < frame.width; ++x) {
            HomeSurfaceTileKind kind = HomeSurfaceTileKind::Plain;
            if (x < 2 || y < 2 || x >= frame.width - 2 || y >= frame.height - 2) kind = HomeSurfaceTileKind::Blocked;
            if ((x - frame.width / 4) * (x - frame.width / 4) + (y - frame.height / 2) * (y - frame.height / 2) < 18) kind = HomeSurfaceTileKind::ResourcePatch;
            if (x > frame.width / 2 - 3 && x < frame.width / 2 + 3 && y > frame.height / 2 - 3 && y < frame.height / 2 + 3) kind = HomeSurfaceTileKind::Foundation;
            frame.tiles[static_cast<std::size_t>(y * frame.width + x)] = MakeHomeSurfaceTile(kind, kind == HomeSurfaceTileKind::ResourcePatch ? 0.75f : 0.0f);
        }
    }
    return frame;
}

void AddStructureMarker(HomeSurfaceFrameVisual& frame, const HomeStructureMarkerVisual& marker) {
    if (marker.width <= 0 || marker.height <= 0) {
        frame.warnings.push_back("Rejected structure marker with invalid dimensions: " + marker.structureId);
        return;
    }
    frame.structures.push_back(marker);
}

int CountTilesOfKind(const HomeSurfaceFrameVisual& frame, HomeSurfaceTileKind kind) {
    return static_cast<int>(std::count_if(frame.tiles.begin(), frame.tiles.end(), [kind](const HomeSurfaceTileVisual& tile) { return tile.kind == kind; }));
}

} // namespace subspace
