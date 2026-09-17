#include "editor/EditorDccShellLayoutSystem.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace subspace;
namespace {
int assertions=0;
void Check(bool valid,const char* name){
    if(!valid){std::cerr<<"[FAIL] "<<name<<'\n';std::exit(1);}
    ++assertions;
}
bool Near(float a,float b){return std::fabs(a-b)<.11f;}
}
int main(){
    Check(!EditorDccShellLayoutSystem::Compute(900,600).valid,"unsupported tiny canvas rejected");
    for(const auto dims:{1280,1600,1920,3840}){
        const int height=dims==1280?768:(dims==1600?900:(dims==1920?1080:2160));
        const auto l=EditorDccShellLayoutSystem::Compute(dims,height);
        Check(l.valid,"desktop layout materializes");
        Check(Near(l.toolRail.y+l.toolRail.height,l.statusBar.y),"fixed rail extends to status independently of Assets");
        Check(l.assetShelf.x>=l.toolRail.x+l.toolRail.width,"Asset Browser cannot start under fixed rail");
        Check(Near(l.assetShelf.x,l.viewport.x),"shelf shares viewport content column left edge");
        Check(Near(l.assetShelf.width,l.viewport.width),"shelf and viewport have one content-column width");
        Check(Near(l.viewport.y+l.viewport.height,l.assetShelf.y),"shelf owns bottom leaf below viewport");
        Check(Near(l.assetShelf.y+l.assetShelf.height,l.statusBar.y),"shelf reaches status boundary");
        Check(l.toolRail.height>l.viewport.height,"rail outlives docked shelf");
    }
    std::cout<<"PASS1508R4: "<<assertions<<" DCC shell geometry assertions passed\n";
}
