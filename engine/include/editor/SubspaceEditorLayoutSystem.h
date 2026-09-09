#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class EditorPanelSlot { AssetBrowser, Viewport, Inspector, Outliner, Validation, Console, Pcg, Jobs, History };
struct EditorPanelLayout { EditorPanelSlot slot=EditorPanelSlot::Viewport; std::string id; float x=0,y=0,width=0,height=0; bool visible=true; bool collapsible=true; };
struct EditorWorkspaceLayout { int viewportWidth=0,viewportHeight=0; float topBarHeight=42; float leftWidth=300; float rightWidth=340; float bottomHeight=180; std::vector<EditorPanelLayout> panels; };
class SubspaceEditorLayoutSystem {
public:
    static EditorWorkspaceLayout Build(int width,int height,bool showOutliner=true,bool showBottom=false);
    static bool Validate(const EditorWorkspaceLayout& layout,std::string* error=nullptr);
};

} // namespace subspace
