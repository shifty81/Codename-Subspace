#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class RuntimeWindowDock { Floating, Left, Right, Bottom, Top };

enum class RuntimeWindowMode { Normal, Compact, Minimized };

struct RuntimeWindowState {
    std::string id;
    std::string title;
    float x=0,y=0,width=320,height=240;
    RuntimeWindowDock dock=RuntimeWindowDock::Floating;
    RuntimeWindowMode mode=RuntimeWindowMode::Normal;
    bool pinned=false;
    bool locked=false;
    float opacity=.88f;
    int zOrder=0;
};

struct RuntimeWindowLayout {
    std::vector<RuntimeWindowState> windows;
};

class RuntimeWindowLayoutSystem {
public:
    static RuntimeWindowLayout DefaultFlightLayout(int viewportWidth,int viewportHeight);
    static bool Move(RuntimeWindowLayout& layout,const std::string& id,float x,float y);
    static bool Resize(RuntimeWindowLayout& layout,const std::string& id,float width,float height);
    static bool SetMode(RuntimeWindowLayout& layout,const std::string& id,RuntimeWindowMode mode);
};

} // namespace subspace
