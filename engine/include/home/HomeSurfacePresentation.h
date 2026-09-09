#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class HomeSurfaceTileKind {
    Plain,
    ResourcePatch,
    Foundation,
    Road,
    Conveyor,
    PowerLine,
    Blocked,
    Water
};

struct HomeSurfaceTileVisual {
    HomeSurfaceTileKind kind = HomeSurfaceTileKind::Plain;
    std::uint32_t fillColor = 0x1D2A35u;
    std::uint32_t edgeColor = 0x263846u;
    char glyph = '.';
    float resourceRichness = 0.0f;
};

struct HomeStructureMarkerVisual {
    std::string structureId;
    std::string label;
    int x = 0;
    int y = 0;
    int width = 1;
    int height = 1;
    std::uint32_t color = 0x66D9FFu;
    bool powered = true;
    bool connectedToLogistics = true;
};

struct HomeSurfaceFrameVisual {
    int width = 0;
    int height = 0;
    std::vector<HomeSurfaceTileVisual> tiles;
    std::vector<HomeStructureMarkerVisual> structures;
    std::vector<std::string> warnings;
};

const char* HomeSurfaceTileKindName(HomeSurfaceTileKind kind);
HomeSurfaceTileVisual MakeHomeSurfaceTile(HomeSurfaceTileKind kind, float richness = 0.0f);
HomeSurfaceFrameVisual CreateStarterHomeSurfaceFrame(int width, int height);
void AddStructureMarker(HomeSurfaceFrameVisual& frame, const HomeStructureMarkerVisual& marker);
int CountTilesOfKind(const HomeSurfaceFrameVisual& frame, HomeSurfaceTileKind kind);

} // namespace subspace
