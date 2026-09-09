#include "developer/resources/RuntimeResourceRegistry.h"

#include <cassert>
#include <iostream>

int main()
{
    subspace::RuntimeResourceRegistry registry;
    registry.RegisterBinding({"texture", "TestTextureSystem", "Smoke texture handler"},
        [](const subspace::AssetReloadRequest& request, const subspace::AssetReloadReport& staged) {
            subspace::RuntimeResourceCommitReport report;
            report.success = staged.success;
            report.committed = staged.success;
            report.message = "committed " + request.path;
            return report;
        });

    subspace::AssetReloadRequest request;
    request.kind = "texture";
    request.path = "content/assets/textures/test.png";

    subspace::AssetReloadReport staged;
    staged.success = true;
    staged.staged = true;

    auto report = registry.Commit(request, staged);
    assert(report.handled);
    assert(report.success);
    assert(report.committed);
    std::cout << "RuntimeResourceRegistry smoke test passed.\n";
}
