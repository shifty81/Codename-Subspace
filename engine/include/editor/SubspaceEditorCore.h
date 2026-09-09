#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
namespace subspace {
enum class EditorWorkspaceKind { Shipyard, StationBuilder, TurretLab, PlanetSector, MaterialStudio, PcgStudio, BlueprintLibrary };
enum class EditorDocumentState { Clean, Dirty, Draft, Invalid };
struct EditorDocument { std::string id,displayName; EditorWorkspaceKind workspace=EditorWorkspaceKind::Shipyard; EditorDocumentState state=EditorDocumentState::Clean; std::string sourcePath; bool readOnly=false; };
struct EditorWorkspaceDescriptor { EditorWorkspaceKind kind=EditorWorkspaceKind::Shipyard; std::string id,displayName,description; bool playerVisible=false,authoringVisible=true; };
class EditorWorkspaceRegistry { public:void Register(EditorWorkspaceDescriptor);const EditorWorkspaceDescriptor* Find(EditorWorkspaceKind) const;const std::vector<EditorWorkspaceDescriptor>& All() const{return workspaces_;} private:std::vector<EditorWorkspaceDescriptor> workspaces_;};
struct EditorSelectionItem { std::string id,type,label; };
class EditorSelectionService { public:void Select(EditorSelectionItem,bool additive=false);void Clear();bool Contains(const std::string&) const;const EditorSelectionItem* Primary() const;const std::vector<EditorSelectionItem>& Items() const{return items_;} private:std::vector<EditorSelectionItem> items_;};
struct EditorOutlinerNode { std::string id,parentId,label,type; bool visible=true,locked=false; };
class EditorOutlinerModel { public:void SetNodes(std::vector<EditorOutlinerNode> n){nodes_=std::move(n);}std::vector<EditorOutlinerNode> ChildrenOf(const std::string&) const;const EditorOutlinerNode* Find(const std::string&) const;const std::vector<EditorOutlinerNode>& Nodes() const{return nodes_;} private:std::vector<EditorOutlinerNode> nodes_;};
class EditorCommand { public:virtual ~EditorCommand()=default;virtual bool Execute()=0;virtual void Undo()=0;virtual std::string Description() const=0;};
class LambdaEditorCommand final:public EditorCommand { public:LambdaEditorCommand(std::string d,std::function<bool()> e,std::function<void()> u):d_(std::move(d)),e_(std::move(e)),u_(std::move(u)){}bool Execute()override{return e_?e_():false;}void Undo()override{if(u_)u_();}std::string Description()const override{return d_;}private:std::string d_;std::function<bool()>e_;std::function<void()>u_;};
class EditorCommandStack { public:bool Execute(std::unique_ptr<EditorCommand>);bool Undo();bool Redo();bool CanUndo()const{return cursor_>0;}bool CanRedo()const{return cursor_<history_.size();}std::string UndoDescription()const;std::string RedoDescription()const;std::size_t Size()const{return history_.size();}private:std::vector<std::unique_ptr<EditorCommand>> history_;std::size_t cursor_=0;};
struct EditorContext { EditorWorkspaceKind workspace=EditorWorkspaceKind::Shipyard;EditorDocument document;EditorSelectionService selection;EditorOutlinerModel outliner;EditorCommandStack commands;bool helpMode=true,developerAuthoring=false;};
}
