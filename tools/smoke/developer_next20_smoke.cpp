#include "developer/BuildReadinessMatrix.h"
#include "developer/ContentLayoutPlan.h"
#include "developer/ProjectIdentityNormalization.h"
#include "developer/ai/DeveloperAiCommandProtocol.h"
#include "developer/diagnostics/DeveloperDiagnosticsHub.h"
#include "developer/gizmos/RuntimeGizmoDrawList.h"
#include "developer/profiles/RuntimeSystemProfile.h"
#include "developer/reflection/ComponentReflectionRegistry.h"
#include "developer/resources/RuntimeResourceRegistry.h"
#include "developer/selection/RuntimeSelectionService.h"
#include "developer/ui/DeveloperCommandAutocomplete.h"
#include "developer/ui/DeveloperWorkspaceState.h"
#include "developer/validation/DeveloperValidationReport.h"

#include <cassert>
#include <iostream>

int main()
{
    subspace::RuntimeSelectionService selection;
    selection.SetPrimary({subspace::RuntimeSelectionKind::Entity, "42", "test entity", 42});
    assert(selection.HasSelection());

    subspace::RuntimeGizmoDrawList gizmos;
    gizmos.AddSelectionBounds({0, 0, 0}, {1, 1, 1});
    assert(!gizmos.Empty());

    subspace::DeveloperCommandAutocomplete autocomplete;
    autocomplete.SetCommands({"entity.inspect", "entity.select", "asset.reload"});
    assert(!autocomplete.Suggest("entity.").empty());

    subspace::ContentLayoutPlan plan = subspace::ContentLayoutPlan::CreateDefault();
    assert(plan.ResolveCanonicalRoot("Assets") == "content/assets");

    subspace::BuildReadinessMatrix matrix;
    matrix.Add({"developer", "READY", "continue"});
    assert(!matrix.HasBlockingItems());

    std::cout << "Developer next-20 smoke test passed.\n";
}
