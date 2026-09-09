#pragma once
#include <string>
#include <unordered_map>
#include <vector>
namespace subspace {
struct SubspaceUiColor{float r=0,g=0,b=0,a=1;};enum class SubspaceUiState{Normal,Hover,Pressed,Selected,Disabled,Valid,Warning,Error,Authoring};enum class SubspaceUiTextRole{Caption,Body,Label,Section,Title};
struct SubspaceUiTheme{SubspaceUiColor canvas,panel,raised,border,textPrimary,textSecondary,accent,valid,warning,error,authoring;float spacingXs=4,spacingS=8,spacingM=12,spacingL=20,radiusControl=5,radiusPanel=8,fontCaption=14,fontBody=16,fontLabel=15,fontSection=17,fontTitle=20,minControlHeight=38;static SubspaceUiTheme Dark();SubspaceUiColor StateColor(SubspaceUiState)const;};
enum class EditorPointerEventType{Move,Enter,Leave,PrimaryDown,PrimaryUp,SecondaryDown,SecondaryUp,MiddleDown,MiddleUp,Drag,Wheel,Cancel};struct EditorPointerEvent{EditorPointerEventType type=EditorPointerEventType::Move;float x=0,y=0,deltaX=0,deltaY=0,wheel=0;bool shift=false,control=false,alt=false;};
class EditorInputRouter{public:void Capture(const std::string&id){capture_=id;}void ReleaseCapture(){capture_.clear();}const std::string&CaptureId()const{return capture_;}bool IsCapturedBy(const std::string&id)const{return capture_==id;}void SetHover(const std::string&id){hover_=id;}const std::string&HoverId()const{return hover_;}private:std::string capture_,hover_;};
struct EditorHelpEntry{std::string id,displayName,shortDescription,detailedDescription,shortcut;};class EditorHelpRegistry{public:void Register(EditorHelpEntry);const EditorHelpEntry*Find(const std::string&)const;std::string Tooltip(const std::string&,bool detailed,const std::string&disabledReason={})const;private:std::unordered_map<std::string,EditorHelpEntry> entries_;};
enum class EditorPropertyType{Text,Bool,Integer,Float,Enum,Color,AssetReference,ReadOnly};struct EditorProperty{std::string id,label;EditorPropertyType type=EditorPropertyType::Text;std::string value,units,helpId;bool enabled=true,advanced=false;};struct EditorPropertySection{std::string id,title;bool collapsed=false,advanced=false;std::vector<EditorProperty>properties;};struct EditorContextAction{std::string id,label,shortcut,helpId;bool enabled=true;std::string disabledReason;};
enum class EditorKeyboardEventType{KeyDown,KeyUp,TextInput};
struct EditorKeyboardEvent{EditorKeyboardEventType type=EditorKeyboardEventType::KeyDown;int key=0;std::string text;bool shift=false,control=false,alt=false;};
class EditorFocusService{public:void Request(const std::string&id){focus_=id;}void Clear(){focus_.clear();}const std::string&FocusedId()const{return focus_;}bool HasFocus(const std::string&id)const{return focus_==id;}private:std::string focus_;};
enum class EditorValidationSeverity{Info,Valid,Warning,Error};
struct EditorValidationMessage{EditorValidationSeverity severity=EditorValidationSeverity::Info;std::string code,message,targetId;};
struct EditorContextMenuModel{float x=0,y=0;std::vector<EditorContextAction> actions;bool open=false;};

}
