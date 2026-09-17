#pragma once
// Shared geometry for the visual Asset Browser and pointer/wheel navigation.
// No renderer, inventory, or Win32 dependency: tests can certify it headlessly.
#include "ship_editor/ShipyardAssetBrowserSystem.h"
#include <algorithm>
#include <cstddef>

namespace subspace {
struct ShipyardCatalogViewport {
    float cardX=0,cardY=0,cardW=0,cardH=0,gap=0;
    float prevX=0,nextX=0,navY=0,navW=0,navH=0;
    std::size_t count=0,pageSize=1,start=0,maxStart=0;
    static ShipyardCatalogViewport Compute(float x,float y,float w,float h,float scale,
                                            ShipyardAssetBrowserDensity density,float thumbnailScale,
                                            std::size_t count,std::size_t wantedStart) {
        ShipyardCatalogViewport v;v.count=count;
        const float s=std::max(.5f,scale);
        float factor=density==ShipyardAssetBrowserDensity::Compact?.82f:
                     (density==ShipyardAssetBrowserDensity::Large?1.18f:1.0f);
        factor*=std::clamp(thumbnailScale,.70f,1.35f);
        v.gap=5*s;v.navW=28*s;v.navH=28*s;
        v.prevX=x+7*s;v.nextX=x+w-7*s-v.navW;
        v.cardX=x+40*s;v.cardY=y+89*s;v.navY=v.cardY+5*s;
        const float usable=std::max(48*s,w-80*s);
        const float desired=std::clamp(205*s*factor,156*s,270*s);
        v.pageSize=std::max<std::size_t>(1,std::min<std::size_t>(8,
            static_cast<std::size_t>(std::max(1.0f,(usable+v.gap)/(desired+v.gap)))));
        v.cardW=std::max(28*s,(usable-v.gap*static_cast<float>(v.pageSize-1))/static_cast<float>(v.pageSize));
        v.cardH=std::max(28*s,h-96*s);
        v.maxStart=count>v.pageSize?count-v.pageSize:0;
        v.start=std::min(wantedStart,v.maxStart);
        return v;
    }
    std::size_t Step(int direction) const {
        return direction<0?(start==0?0:start-1):std::min(maxStart,start+1);
    }
};
} // namespace subspace
