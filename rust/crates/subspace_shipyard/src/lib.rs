use serde::{Deserialize, Serialize};
use std::collections::{BTreeMap, BTreeSet};
use subspace_core::StableId;
use subspace_ship::{ModuleDefinition, ShipBlueprint};

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Workspace { Build, Interior, Systems, Appearance, Test, Dev }

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Tool { Select, Move, Rotate, Scale, Attach, Socket, Symmetry, Measure, Pivot, Snap, FrameSelection, FrameShip }

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum GeneratorCandidateState { None, Preview, Draft, Validated, Certified }

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize, Default)]
pub struct SelectionState {
    pub primary: Option<StableId>,
    pub selected: BTreeSet<StableId>,
}
impl SelectionState {
    pub fn select_single(&mut self, id: StableId) { self.primary = Some(id); self.selected.clear(); self.selected.insert(id); }
    pub fn toggle(&mut self, id: StableId) {
        if !self.selected.remove(&id) { self.selected.insert(id); self.primary = Some(id); }
        else if self.primary == Some(id) { self.primary = self.selected.iter().next().copied(); }
    }
    pub fn clear(&mut self) { self.primary = None; self.selected.clear(); }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ShipyardDocument { pub blueprint: ShipBlueprint, pub revision: u64, pub saved_revision: u64 }
impl ShipyardDocument {
    pub fn new(blueprint: ShipBlueprint) -> Self { Self { blueprint, revision: 1, saved_revision: 1 } }
    pub fn mark_modified(&mut self) { self.revision = self.revision.saturating_add(1); }
    pub fn mark_saved(&mut self) { self.saved_revision = self.revision; }
    pub fn is_dirty(&self) -> bool { self.revision != self.saved_revision }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ShipyardSession {
    pub workspace: Workspace,
    pub active_tool: Tool,
    pub selection: SelectionState,
    pub advanced_visible: bool,
    pub generator_candidate: GeneratorCandidateState,
    pub active_panel: Option<String>,
    pub maximized_panel: Option<String>,
}
impl Default for ShipyardSession {
    fn default() -> Self {
        Self {
            workspace: Workspace::Build, active_tool: Tool::Select, selection: SelectionState::default(),
            advanced_visible: false, generator_candidate: GeneratorCandidateState::None,
            active_panel: Some("ship-viewport".into()), maximized_panel: None,
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct CommandDescriptor {
    pub id: &'static str,
    pub label: &'static str,
    pub category: &'static str,
    pub shortcut: &'static str,
    pub mutates_document: bool,
}

#[derive(Debug, Default)]
pub struct CommandRegistry { commands: BTreeMap<&'static str, CommandDescriptor> }
impl CommandRegistry {
    pub fn register(&mut self, descriptor: CommandDescriptor) { self.commands.insert(descriptor.id, descriptor); }
    pub fn get(&self, id: &str) -> Option<&CommandDescriptor> { self.commands.get(id) }
    pub fn search(&self, query: &str) -> Vec<&CommandDescriptor> {
        let q = query.to_ascii_lowercase();
        self.commands.values().filter(|c| {
            c.id.to_ascii_lowercase().contains(&q) || c.label.to_ascii_lowercase().contains(&q) || c.category.to_ascii_lowercase().contains(&q)
        }).collect()
    }
    pub fn professional_defaults() -> Self {
        let mut out = Self::default();
        for command in [
            CommandDescriptor { id: "tool.select", label: "Select", category: "Tool", shortcut: "Q", mutates_document: false },
            CommandDescriptor { id: "tool.move", label: "Move", category: "Tool", shortcut: "W", mutates_document: false },
            CommandDescriptor { id: "tool.rotate", label: "Rotate", category: "Tool", shortcut: "E", mutates_document: false },
            CommandDescriptor { id: "tool.scale", label: "Scale", category: "Tool", shortcut: "R", mutates_document: false },
            CommandDescriptor { id: "tool.attach", label: "Attach", category: "Tool", shortcut: "A", mutates_document: false },
            CommandDescriptor { id: "tool.socket", label: "Socket", category: "Tool", shortcut: "", mutates_document: false },
            CommandDescriptor { id: "tool.symmetry", label: "Symmetry", category: "Tool", shortcut: "", mutates_document: false },
            CommandDescriptor { id: "tool.measure", label: "Measure", category: "Tool", shortcut: "M", mutates_document: false },
            CommandDescriptor { id: "edit.undo", label: "Undo", category: "Edit", shortcut: "Ctrl+Z", mutates_document: true },
            CommandDescriptor { id: "edit.redo", label: "Redo", category: "Edit", shortcut: "Ctrl+Shift+Z", mutates_document: true },
            CommandDescriptor { id: "generator.generate", label: "Generate", category: "Generator", shortcut: "", mutates_document: true },
            CommandDescriptor { id: "generator.new-seed-generate", label: "New Seed + Generate", category: "Generator", shortcut: "", mutates_document: true },
            CommandDescriptor { id: "generator.explain", label: "Explain", category: "Generator", shortcut: "", mutates_document: false },
            CommandDescriptor { id: "validation.run", label: "Validate Ship", category: "Validation", shortcut: "", mutates_document: false },
            CommandDescriptor { id: "search.open", label: "Universal Search", category: "View", shortcut: "F3", mutates_document: false },
            CommandDescriptor { id: "view.toggle-docks", label: "Collapse / Restore Docks", category: "View", shortcut: "F12", mutates_document: false },
            CommandDescriptor { id: "view.maximize-panel", label: "Maximize Panel", category: "View", shortcut: "Ctrl+Space", mutates_document: false },
        ] { out.register(command); }
        out
    }
}

#[derive(Debug, Clone)]
pub struct History { undo: Vec<ShipyardDocument>, redo: Vec<ShipyardDocument>, capacity: usize }
impl History {
    pub fn new(capacity: usize) -> Self { Self { undo: Vec::new(), redo: Vec::new(), capacity: capacity.max(1) } }
    pub fn commit(&mut self, before: ShipyardDocument, after: &ShipyardDocument) {
        if before.revision == after.revision { return; }
        if self.undo.len() >= self.capacity { self.undo.remove(0); }
        self.undo.push(before); self.redo.clear();
    }
    pub fn undo(&mut self, current: &mut ShipyardDocument) -> bool {
        let Some(previous) = self.undo.pop() else { return false; };
        self.redo.push(current.clone()); *current = previous; true
    }
    pub fn redo(&mut self, current: &mut ShipyardDocument) -> bool {
        let Some(next) = self.redo.pop() else { return false; };
        self.undo.push(current.clone()); *current = next; true
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum DockHint { Left, Center, RightTop, RightBottom, Bottom, Floating }

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PanelDescriptor { pub id: String, pub title: String, pub dock: DockHint, pub default_visible: bool }

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LayoutPreset { pub id: String, pub workspace: Workspace, pub visible_panels: Vec<String> }

pub fn default_layouts() -> Vec<LayoutPreset> {
    vec![
        LayoutPreset { id: "BUILD".into(), workspace: Workspace::Build, visible_panels: vec!["asset-browser".into(),"ship-viewport".into(),"outliner".into(),"inspector".into(),"activity".into()] },
        LayoutPreset { id: "INTERIOR".into(), workspace: Workspace::Interior, visible_panels: vec!["asset-browser".into(),"ship-viewport".into(),"outliner".into(),"inspector".into(),"interior-program".into(),"activity".into()] },
        LayoutPreset { id: "SYSTEMS".into(), workspace: Workspace::Systems, visible_panels: vec!["ship-viewport".into(),"outliner".into(),"inspector".into(),"systems".into(),"generator".into(),"activity".into()] },
        LayoutPreset { id: "APPEARANCE".into(), workspace: Workspace::Appearance, visible_panels: vec!["asset-browser".into(),"ship-viewport".into(),"outliner".into(),"inspector".into(),"appearance".into(),"activity".into()] },
        LayoutPreset { id: "TEST".into(), workspace: Workspace::Test, visible_panels: vec!["ship-viewport".into(),"outliner".into(),"inspector".into(),"playtest".into(),"validation".into(),"activity".into()] },
    ]
}

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct AssetBrowserQuery {
    pub text: String,
    pub required_tags: BTreeSet<String>,
    pub compatible_only: bool,
    pub certified_only: bool,
    pub favorites_only: bool,
}

pub fn filter_asset_definitions<'a>(definitions: &'a [ModuleDefinition], query: &AssetBrowserQuery) -> Vec<&'a ModuleDefinition> {
    let text = query.text.to_ascii_lowercase();
    definitions.iter().filter(|d| {
        (text.is_empty() || d.display_name.to_ascii_lowercase().contains(&text) || d.id.0.to_ascii_lowercase().contains(&text))
        && query.required_tags.iter().all(|tag| d.tags.iter().any(|t| t == tag))
    }).collect()
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum PropertyScope { Instance, Definition }

#[derive(Debug, Clone)]
pub struct PropertySection {
    pub scope: PropertyScope,
    pub id: String,
    pub title: String,
    pub rows: Vec<(String, String)>,
}

pub fn instance_definition_sections(blueprint: &ShipBlueprint, selected: StableId, definitions: &[ModuleDefinition]) -> Vec<PropertySection> {
    let Some(instance) = blueprint.modules.iter().find(|m| m.id == selected) else { return vec![]; };
    let definition = definitions.iter().find(|d| d.id == instance.definition);
    let mut out = vec![PropertySection {
        scope: PropertyScope::Instance, id: "instance".into(), title: "INSTANCE".into(),
        rows: vec![("Instance ID".into(), selected.to_string()), ("Definition".into(), instance.definition.0.clone())],
    }];
    if let Some(definition) = definition {
        out.push(PropertySection {
            scope: PropertyScope::Definition, id: "definition".into(), title: "DEFINITION".into(),
            rows: vec![
                ("Name".into(), definition.display_name.clone()),
                ("Class".into(), definition.class.clone()),
                ("Semantic".into(), definition.semantic.clone()),
                ("Size".into(), definition.size_class.clone()),
                ("Provenance".into(), definition.provenance.clone()),
            ],
        });
    }
    out
}
