#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct SubspaceUiColor { float r=0,g=0,b=0,a=1; };
enum class SubspaceUiState { Normal,Hover,Pressed,Selected,Disabled,Valid,Warning,Error,Authoring };
enum class SubspaceUiTextRole { Caption,Body,Label,Section,Title };

/// Project-wide UI tokens. Runtime HUD, Shipyard/dev mode, strategic command,
/// dialogs and tooling all project from this one dark theme rather than keeping
/// separate editor/game palettes.
struct SubspaceUiTheme {
    SubspaceUiColor canvas,panel,raised,border,textPrimary,textSecondary,accent,valid,warning,error,authoring;
    SubspaceUiColor input,header,scrollTrack,scrollThumb,overlay,popup,focus;
    float spacingXs=4,spacingS=8,spacingM=12,spacingL=20;
    float radiusControl=5,radiusPanel=8;
    float fontCaption=14,fontBody=16,fontLabel=15,fontSection=17,fontTitle=20;
    float minControlHeight=38;
    float splitterThickness=5;
    float scrollbarThickness=10;
    float panelOpacityDefault=.92f;
    float panelOpacityMinimum=.28f;
    float panelOpacityMaximum=1.0f;
    static SubspaceUiTheme Dark();
    SubspaceUiColor StateColor(SubspaceUiState) const;
};

enum class EditorPointerEventType { Move,Enter,Leave,PrimaryDown,PrimaryUp,SecondaryDown,SecondaryUp,MiddleDown,MiddleUp,Drag,Wheel,Cancel };
struct EditorPointerEvent { EditorPointerEventType type=EditorPointerEventType::Move; float x=0,y=0,deltaX=0,deltaY=0,wheel=0; bool shift=false,control=false,alt=false; };
class EditorInputRouter { public: void Capture(const std::string&id){capture_=id;} void ReleaseCapture(){capture_.clear();} const std::string& CaptureId()const{return capture_;} bool IsCapturedBy(const std::string&id)const{return capture_==id;} void SetHover(const std::string&id){hover_=id;} const std::string& HoverId()const{return hover_;} private: std::string capture_,hover_; };

struct EditorHelpEntry { std::string id,displayName,shortDescription,detailedDescription,shortcut; };
class EditorHelpRegistry { public: void Register(EditorHelpEntry); const EditorHelpEntry* Find(const std::string&) const; std::string Tooltip(const std::string&,bool detailed,const std::string&disabledReason={}) const; private: std::unordered_map<std::string,EditorHelpEntry> entries_; };
enum class EditorPropertyType { Text,Bool,Integer,Float,Enum,Color,AssetReference,ReadOnly };
struct EditorProperty { std::string id,label; EditorPropertyType type=EditorPropertyType::Text; std::string value,units,helpId; bool enabled=true,advanced=false; };
struct EditorPropertySection { std::string id,title; bool collapsed=false,advanced=false; std::vector<EditorProperty>properties; };
struct EditorContextAction { std::string id,label,shortcut,helpId; bool enabled=true; std::string disabledReason; };
enum class EditorKeyboardEventType { KeyDown,KeyUp,TextInput };
struct EditorKeyboardEvent { EditorKeyboardEventType type=EditorKeyboardEventType::KeyDown; int key=0; std::string text; bool shift=false,control=false,alt=false; };
class EditorFocusService { public: void Request(const std::string&id){focus_=id;} void Clear(){focus_.clear();} const std::string& FocusedId()const{return focus_;} bool HasFocus(const std::string&id)const{return focus_==id;} private: std::string focus_; };
enum class EditorValidationSeverity { Info,Valid,Warning,Error };
struct EditorValidationMessage { EditorValidationSeverity severity=EditorValidationSeverity::Info; std::string code,message,targetId; };
struct EditorContextMenuModel { float x=0,y=0; std::vector<EditorContextAction> actions; bool open=false; };

// Pass1073+: one dock model for in-game workspaces and development/editor use.
enum class SubspaceDockSplitAxis { Horizontal, Vertical };
struct SubspaceUiRect { float x=0,y=0,width=0,height=0; };
struct SubspaceDockPanel {
    std::string id;
    std::string title;
    std::string defaultLeafId;
    bool visible=true;
    bool closable=true;
    bool floatable=true;
    bool resizable=true;
    bool pinned=false;
    bool acceptsPointer=true;
    float opacity=.92f;
    float minWidth=150;
    float minHeight=90;
    float maxWidth=4096;
    float maxHeight=4096;
};
struct SubspaceDockNode {
    std::string id;
    bool split=false;
    SubspaceDockSplitAxis axis=SubspaceDockSplitAxis::Horizontal;
    float ratio=.5f;
    std::string firstChildId;
    std::string secondChildId;
    std::vector<std::string> tabs;
    std::string activeTabId;
    bool collapsed=false;
};
struct SubspaceFloatingPanel { std::string panelId; SubspaceUiRect rect{80,80,420,320}; };
struct SubspaceDockWorkspace {
    std::string id="workspace";
    std::string rootNodeId="root";
    std::vector<SubspaceDockPanel> panels;
    std::vector<SubspaceDockNode> nodes;
    std::vector<SubspaceFloatingPanel> floatingPanels;
};
struct SubspaceDockLayout {
    std::string panelId;
    std::string leafId;
    SubspaceUiRect rect{};
    float opacity=.92f;
    bool visible=false;
    bool active=false;
    bool floating=false;
};

class SubspaceDockSystem {
public:
    static SubspaceDockWorkspace CreateMinimalWorkspace(std::string id="workspace");
    static SubspaceDockPanel* FindPanel(SubspaceDockWorkspace&,const std::string& id);
    static const SubspaceDockPanel* FindPanel(const SubspaceDockWorkspace&,const std::string& id);
    static SubspaceDockNode* FindNode(SubspaceDockWorkspace&,const std::string& id);
    static const SubspaceDockNode* FindNode(const SubspaceDockWorkspace&,const std::string& id);
    static bool RegisterPanel(SubspaceDockWorkspace&,SubspaceDockPanel panel);
    static bool OpenPanel(SubspaceDockWorkspace&,const std::string& panelId);
    static bool ClosePanel(SubspaceDockWorkspace&,const std::string& panelId);
    static bool ActivatePanel(SubspaceDockWorkspace&,const std::string& panelId);
    static bool MovePanel(SubspaceDockWorkspace&,const std::string& panelId,const std::string& targetLeafId,bool activate=true);
    static bool FloatPanel(SubspaceDockWorkspace&,const std::string& panelId,SubspaceUiRect rect);
    static bool DockPanel(SubspaceDockWorkspace&,const std::string& panelId,const std::string& targetLeafId,bool activate=true);
    static bool SetPanelOpacity(SubspaceDockWorkspace&,const std::string& panelId,float opacity,const SubspaceUiTheme& theme=SubspaceUiTheme::Dark());
    static bool ResizeSplit(SubspaceDockWorkspace&,const std::string& nodeId,float ratio);
    static bool ResizeFloating(SubspaceDockWorkspace&,const std::string& panelId,SubspaceUiRect rect);
    static std::vector<SubspaceDockLayout> Materialize(const SubspaceDockWorkspace&,int width,int height,float topInset=34.0f);
    static bool Validate(const SubspaceDockWorkspace&,std::string* error=nullptr);
};

} // namespace subspace
