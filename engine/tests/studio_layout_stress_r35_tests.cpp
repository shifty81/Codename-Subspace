#include "editor/EditorDccShellLayoutSystem.h"
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
using namespace subspace;
namespace {
bool overlap(const EditorDccRect&a,const EditorDccRect&b){return a.width>0&&a.height>0&&b.width>0&&b.height>0&&a.x<b.x+b.width&&b.x<a.x+a.width&&a.y<b.y+b.height&&b.y<a.y+a.height;}
bool inside(const EditorDccRect&r,int w,int h){return r.x>=-.01f&&r.y>=-.01f&&r.width>=0&&r.height>=0&&r.x+r.width<=w+.01f&&r.y+r.height<=h+.01f;}
}
int main(){
    const std::array<std::array<int,2>,10> sizes{{{{1120,740}},{{1280,768}},{{1366,768}},{{1440,900}},{{1600,900}},{{1920,1080}},{{2560,1440}},{{3440,1440}},{{3840,1600}},{{3840,2160}}}};
    int checked=0;
    for(const auto s:sizes){
        const auto l=EditorDccShellLayoutSystem::Compute(s[0],s[1]);
        assert(l.valid);
        for(const auto r:{l.applicationMenu,l.workspaceStrip,l.viewportHeader,l.viewport,l.toolRail,l.assetShelf,l.outliner,l.properties,l.statusBar})assert(inside(r,s[0],s[1]));
        assert(!overlap(l.toolRail,l.viewport));
        assert(!overlap(l.toolRail,l.assetShelf));
        assert(!overlap(l.viewport,l.assetShelf));
        assert(!overlap(l.viewport,l.outliner));
        assert(!overlap(l.viewport,l.properties));
        assert(!overlap(l.assetShelf,l.outliner));
        assert(!overlap(l.assetShelf,l.properties));
        assert(!overlap(l.outliner,l.properties));
        assert(std::fabs(l.viewport.x-l.assetShelf.x)<.01f);
        assert(std::fabs(l.viewport.width-l.assetShelf.width)<.01f);
        assert(l.viewport.width>=300&&l.viewport.height>=250);
        ++checked;
    }
    assert(!EditorDccShellLayoutSystem::Compute(1119,740).valid);
    assert(!EditorDccShellLayoutSystem::Compute(1120,739).valid);
    std::cout<<"R35 DCC layout collision stress: "<<checked<<" resolutions PASS\n";
}
