#pragma once

#include <algorithm>
#include <cstddef>

namespace subspace {

// Small deterministic layout rules shared by Studio's visible control surface.
// The dock tree still owns panel rectangles; this policy only subdivides an
// already-authoritative strip/panel rectangle without inventing a second one.
class ConstructionUiLayoutPolicy {
public:
    static float TabWidth(float availableWidth,std::size_t count,float gap) noexcept {
        if(count==0)return 0.0f;
        const float gaps=gap*static_cast<float>(count>0?count-1:0);
        return std::max(1.0f,(availableWidth-gaps)/static_cast<float>(count));
    }

    static bool ShowAssetDensity(float assetWidth,float uiScale) noexcept {
        return assetWidth>=520.0f*std::max(0.5f,uiScale);
    }

    static bool ShowAssetSecondaryActions(float assetWidth,float uiScale) noexcept {
        return assetWidth>=650.0f*std::max(0.5f,uiScale);
    }

    static bool Overlaps(float ax,float ay,float aw,float ah,
                         float bx,float by,float bw,float bh,
                         float epsilon=0.25f) noexcept {
        return ax+aw>bx+epsilon && bx+bw>ax+epsilon &&
               ay+ah>by+epsilon && by+bh>ay+epsilon;
    }
};

} // namespace subspace
