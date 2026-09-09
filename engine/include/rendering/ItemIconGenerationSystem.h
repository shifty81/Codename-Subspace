#pragma once

#include "core/resources/ObjAssetLoader.h"
#include "inventory/ItemizationSystem.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct ItemIconBitmap {
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> rgba;
    std::string cacheKey;

    bool Valid() const { return width>0&&height>0&&rgba.size()==static_cast<std::size_t>(width*height*4); }
};

/// Deterministic CPU thumbnail renderer. It projects the actual source module
/// geometry into a compact icon bitmap, so dropped/inventory items never need
/// separately hand-authored icons that can drift from the 3D part.
class ItemIconGenerationSystem {
public:
    static ItemIconBitmap Generate(const GeneratedItem& item,const ObjMeshData& mesh,int size=96);
};

} // namespace subspace
