#pragma once

#include "developer/gizmos/RuntimeGizmoDrawList.h"
#include "developer/selection/RuntimeSelectionService.h"

#include <functional>
#include <string>

namespace subspace {

struct ViewportRay {
    GizmoVec3 origin;
    GizmoVec3 direction;
};

struct ViewportPickResult {
    bool hit = false;
    RuntimeSelectionItem item;
    GizmoVec3 worldPosition;
    std::string message;
};

class DeveloperViewportBridge {
public:
    using PickCallback = std::function<ViewportPickResult(const ViewportRay&)>;

    void SetPickCallback(PickCallback callback);
    ViewportPickResult Pick(const ViewportRay& ray) const;
    void SetSelectionService(RuntimeSelectionService* service) { _selectionService = service; }
    bool SelectFromRay(const ViewportRay& ray);

    RuntimeGizmoDrawList& GetGizmos() { return _gizmos; }
    const RuntimeGizmoDrawList& GetGizmos() const { return _gizmos; }

private:
    PickCallback _pickCallback;
    RuntimeSelectionService* _selectionService = nullptr;
    RuntimeGizmoDrawList _gizmos;
};

} // namespace subspace
