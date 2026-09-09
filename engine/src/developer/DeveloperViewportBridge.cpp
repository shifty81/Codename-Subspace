#include "developer/viewport/DeveloperViewportBridge.h"

namespace subspace {

void DeveloperViewportBridge::SetPickCallback(PickCallback callback)
{
    _pickCallback = std::move(callback);
}

ViewportPickResult DeveloperViewportBridge::Pick(const ViewportRay& ray) const
{
    if (!_pickCallback) {
        return {false, {}, {}, "No viewport pick callback registered."};
    }
    return _pickCallback(ray);
}

bool DeveloperViewportBridge::SelectFromRay(const ViewportRay& ray)
{
    auto result = Pick(ray);
    if (!result.hit || !_selectionService) {
        return false;
    }
    _selectionService->SetPrimary(result.item);
    return true;
}

} // namespace subspace
