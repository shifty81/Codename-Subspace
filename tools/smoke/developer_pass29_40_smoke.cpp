#include "developer/assets/ContentNormalizationManifest.h"
#include "developer/assets/RuntimeHotReloadQueue.h"
#include "developer/build/DeveloperBuildPreflight.h"
#include "developer/input/DeveloperInputRouter.h"
#include "developer/reflection/ComponentReflectionDefaults.h"
#include "developer/reflection/ComponentReflectionRegistry.h"
#include "developer/resources/RuntimeResourceBindingCatalog.h"
#include "developer/resources/RuntimeResourceRegistry.h"
#include "developer/safety/RuntimeEditSafetyGate.h"
#include "developer/script/DeveloperCommandScriptRunner.h"
#include "developer/ui/DeveloperCommandPalette.h"
#include "developer/ui/DeveloperPanelModel.h"

#include <cstdlib>
#include <iostream>

using namespace subspace;

static void require_true(bool value, const char* message) {
    if (!value) {
        std::cerr << "FAIL: " << message << "\n";
        std::exit(1);
    }
}

int main() {
    RuntimeResourceRegistry resources;
    RuntimeResourceBindingCatalog::RegisterDefaultBindings(resources);
    require_true(resources.HasBinding("texture"), "texture default resource binding should exist");

    ComponentReflectionRegistry reflection;
    ComponentReflectionDefaults::RegisterCommonComponentTypes(reflection);
    require_true(reflection.HasComponent("Transform"), "Transform reflection should exist");
    require_true(reflection.HasField("Transform", "position.x"), "Transform.position.x should exist");

    RuntimeHotReloadQueue queue;
    AssetReloadRequest reload;
    reload.kind = "texture";
    reload.path = "content/assets/textures/test.png";
    queue.Enqueue(reload, "smoke");
    require_true(queue.Size() == 1, "hot reload queue should contain one item");
    require_true(queue.Pop().request.kind == "texture", "hot reload pop should preserve request");

    RuntimeEditSafetyGate safety;
    RuntimeEditCommand deleteCommand;
    deleteCommand.name = "entity.delete";
    RuntimeEditSafetyContext safetyContext;
    safetyContext.allowDestructiveEntityOps = false;
    require_true(!safety.Evaluate(deleteCommand, safetyContext).allowed, "destructive entity delete should be blocked by default");
    safetyContext.allowDestructiveEntityOps = true;
    require_true(safety.Evaluate(deleteCommand, safetyContext).allowed, "destructive entity delete should be allowed after opt-in");

    DeveloperPanelModel panels;
    panels.RegisterDefaultPanels();
    require_true(panels.IsVisible("console"), "console panel should be visible by default");
    panels.TogglePanel("assets");
    require_true(panels.IsVisible("assets"), "assets panel toggle should work");

    DeveloperInputRouter input;
    input.BindDefaultActions();
    require_true(input.ResolveCommand("undo") == "dev.undo", "default undo input command should resolve");

    DeveloperCommandBridge bridge([](const RuntimeEditCommand& command) {
        return RuntimeEditResult::Success(command, "ok", command.name != "dev.mode.on");
    });
    bridge.SetSupportedCommands({"dev.mode.on", "dev.undo", "texture.reload", "entity.inspect"});

    DeveloperCommandPalette palette;
    palette.RebuildFromBridge(bridge);
    require_true(!palette.Search("texture").empty(), "palette should find texture command");

    DeveloperCommandScriptRunner runner;
    auto script = runner.RunLines({"# smoke", "dev.mode.on", "texture.reload path=foo.png"}, bridge);
    require_true(script.success, "command script should succeed");
    require_true(script.executed == 2, "command script should execute two commands");

    ContentNormalizationManifestBuilder contentBuilder;
    auto contentPlan = contentBuilder.BuildDefaultPlan();
    require_true(contentPlan.MoveCount() >= 3, "content normalization should plan core moves");
    require_true(!contentPlan.HasDestructiveMoves(), "content normalization plan should be dry-run/non-destructive");

    DeveloperBuildPreflight preflight;
    auto preflightReport = preflight.EvaluateRootInventory({"engine", "docs", "scripts", "random.tmp"});
    require_true(!preflightReport.HasErrors(), "preflight should not error with engine present");
    require_true(!preflightReport.issues.empty(), "preflight should warn on unexpected root entries");

    std::cout << "Developer pass29-40 smoke test passed.\n";
    return 0;
}
