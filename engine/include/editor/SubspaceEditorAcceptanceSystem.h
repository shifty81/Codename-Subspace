#pragma once
#include "editor/SubspaceEditorCore.h"
#include <string>
#include <vector>
namespace subspace {struct EditorAcceptanceResult{bool passed=false;std::vector<std::string>failures;};class SubspaceEditorAcceptanceSystem{public:static EditorWorkspaceRegistry BuildDefaultWorkspaceRegistry();static EditorAcceptanceResult ValidateProjectWideEditor(const EditorWorkspaceRegistry&,bool,bool,bool,bool,bool,bool);};}
