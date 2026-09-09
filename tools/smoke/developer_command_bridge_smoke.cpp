#include "developer/DeveloperCommandBridge.h"

#include <cassert>
#include <iostream>

using namespace subspace;

int main()
{
    DeveloperCommandBridge bridge([](const RuntimeEditCommand& command) {
        assert(command.name == "texture.reload");
        assert(command.GetArg("path") == "content/assets/textures/hull.png");
        return RuntimeEditResult::Success(command, "ok", false);
    });

    DeveloperCommandExecution execution = bridge.ExecuteLine("texture.reload path=content/assets/textures/hull.png");
    assert(execution.accepted);
    assert(execution.success);
    std::cout << "DeveloperCommandBridge smoke test passed." << std::endl;
    return 0;
}
