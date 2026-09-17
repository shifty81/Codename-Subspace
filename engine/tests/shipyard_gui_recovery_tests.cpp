#include "ship_editor/ShipyardCatalogViewport.h"
#include <cstdlib>
#include <iostream>
#include <string>
using namespace subspace;
namespace {
int assertions=0;
void Check(bool ok,const char* description){
    if(!ok){std::cerr<<"[FAIL] "<<description<<'\n';std::exit(1);}++assertions;
}
}
int main(){
    for(const float width:{380.0f,780.0f,1280.0f}){
        for(const auto density:{ShipyardAssetBrowserDensity::Compact,
                                ShipyardAssetBrowserDensity::Comfortable,
                                ShipyardAssetBrowserDensity::Large}){
            auto v=ShipyardCatalogViewport::Compute(65,480,width,228,1,density,1,26,0);
            Check(v.pageSize>=1&&v.pageSize<=8,"page count responds to width/density");
            Check(v.start==0&&v.maxStart==26-v.pageSize,"initial bounds");
            Check(v.cardX>=65+v.navW,"left arrow separate from cards");
            Check(v.cardX+(v.cardW+v.gap)*(v.pageSize-1)+v.cardW <=65+width-v.navW,
                "right arrow separate from cards");
            Check(v.Step(-1)==0,"cannot underflow scroll");
            std::size_t last=0;
            for(int step=0;step<64;++step){
                v=ShipyardCatalogViewport::Compute(65,480,width,228,1,density,1,26,last);
                last=v.Step(1);
            }
            v=ShipyardCatalogViewport::Compute(65,480,width,228,1,density,1,26,last);
            Check(v.start==v.maxStart,"last possible item reachable by wheel/page button");
            Check(v.start+v.pageSize==26,"the final asset appears on the last page");
            Check(v.Step(1)==v.maxStart,"scroll boundary does not exceed catalog");
        }
    }
    auto empty=ShipyardCatalogViewport::Compute(0,0,400,180,1,ShipyardAssetBrowserDensity::Comfortable,1,0,999);
    Check(empty.start==0&&empty.maxStart==0,"empty search result scrolls safely");
    auto single=ShipyardCatalogViewport::Compute(0,0,400,180,1,ShipyardAssetBrowserDensity::Comfortable,1,1,999);
    Check(single.start==0&&single.Step(1)==0,"single result scrolls safely");
    auto resized=ShipyardCatalogViewport::Compute(100,110,320,178,1,ShipyardAssetBrowserDensity::Large,1.35f,26,25);
    Check(resized.start==resized.maxStart,"floating resize clamps to new maximum");
    Check(resized.cardH>0&&resized.cardW>0,"small floating window creates positive card geometry");
    std::cout<<"SHIPYARD GUI RECOVERY: "<<assertions<<" checks passed\n";
}
