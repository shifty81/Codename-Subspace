#pragma once

#include <string>
#include <utility>

namespace subspace {

class PlayerController {
public:
    void SetControlledEntity(std::string entityId) { _controlledEntityId = std::move(entityId); }
    const std::string& GetControlledEntity() const { return _controlledEntityId; }
    bool HasControlledEntity() const { return !_controlledEntityId.empty(); }

private:
    std::string _controlledEntityId;
};

} // namespace subspace
