use subspace_shipyard::{DockHint, PanelDescriptor, Tool, Workspace};

#[derive(Debug, Clone)]
pub struct ToolDescriptor {
    pub id: &'static str,
    pub label: &'static str,
    pub tool: Tool,
    pub shortcut: &'static str,
}

#[derive(Debug, Clone)]
pub struct EditorContribution {
    pub workspaces: Vec<Workspace>,
    pub panels: Vec<PanelDescriptor>,
    pub tools: Vec<ToolDescriptor>,
}

pub trait EmberHostBridge {
    fn register_workspace(&mut self, workspace: Workspace);
    fn register_panel(&mut self, panel: PanelDescriptor);
    fn register_tool(&mut self, tool: ToolDescriptor);
}

pub fn contribution() -> EditorContribution {
    EditorContribution {
        workspaces: vec![Workspace::Build, Workspace::Interior, Workspace::Systems, Workspace::Appearance, Workspace::Test, Workspace::Dev],
        panels: vec![
            panel("asset-browser", "Asset Browser", DockHint::Left, true),
            panel("ship-viewport", "Ship Viewport", DockHint::Center, true),
            panel("outliner", "Outliner", DockHint::RightTop, true),
            panel("inspector", "Inspector", DockHint::RightBottom, true),
            panel("generator", "Generator", DockHint::Left, false),
            panel("interior-program", "Interior Program", DockHint::Left, false),
            panel("apertures-hangars", "Apertures & Hangars", DockHint::Left, false),
            panel("systems", "Systems", DockHint::RightBottom, false),
            panel("appearance", "Appearance", DockHint::RightBottom, false),
            panel("playtest", "Play / Test", DockHint::RightBottom, false),
            panel("validation", "Validation", DockHint::Bottom, false),
            panel("history", "History", DockHint::Bottom, false),
            panel("activity", "Activity / Jobs", DockHint::Bottom, true),
            panel("console", "Console", DockHint::Bottom, false),
            panel("search", "Search", DockHint::Bottom, false),
            panel("blueprints", "Blueprints", DockHint::Left, false),
            panel("forge-project", "Forge Project", DockHint::Bottom, false),
            panel("cortex", "Cortex", DockHint::RightBottom, false),
        ],
        tools: vec![
            tool("select","Select",Tool::Select,"Q"),
            tool("move","Move",Tool::Move,"W"),
            tool("rotate","Rotate",Tool::Rotate,"E"),
            tool("scale","Scale",Tool::Scale,"R"),
            tool("attach","Attach",Tool::Attach,"A"),
            tool("socket","Socket",Tool::Socket,""),
            tool("symmetry","Symmetry",Tool::Symmetry,""),
            tool("measure","Measure",Tool::Measure,"M"),
            tool("pivot","Pivot",Tool::Pivot,""),
            tool("snap","Snap",Tool::Snap,""),
            tool("frame-selection","Frame Selection",Tool::FrameSelection,"F"),
            tool("frame-ship","Frame Ship",Tool::FrameShip,""),
        ],
    }
}

fn panel(id: &str, title: &str, dock: DockHint, default_visible: bool) -> PanelDescriptor {
    PanelDescriptor { id: id.into(), title: title.into(), dock, default_visible }
}
fn tool(id: &'static str, label: &'static str, tool: Tool, shortcut: &'static str) -> ToolDescriptor {
    ToolDescriptor { id, label, tool, shortcut }
}
pub fn register_all(host: &mut dyn EmberHostBridge) {
    let contribution = contribution();
    for workspace in contribution.workspaces { host.register_workspace(workspace); }
    for panel in contribution.panels { host.register_panel(panel); }
    for tool in contribution.tools { host.register_tool(tool); }
}
