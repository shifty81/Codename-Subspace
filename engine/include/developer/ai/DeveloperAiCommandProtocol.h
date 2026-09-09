#pragma once

#include "developer/RuntimeEditCommand.h"

#include <string>
#include <vector>

namespace subspace {

struct DeveloperAiCommandEnvelope {
    std::string requestId;
    std::string actor;
    std::string intent;
    RuntimeEditCommand command;
    bool requiresConfirmation = true;
};

struct DeveloperAiCommandPlan {
    std::string title;
    std::string summary;
    std::vector<DeveloperAiCommandEnvelope> commands;
};

class DeveloperAiCommandProtocol {
public:
    static DeveloperAiCommandEnvelope MakeEnvelope(std::string requestId, std::string actor, RuntimeEditCommand command, std::string intent = {});
    static bool IsWriteCommand(const RuntimeEditCommand& command);
    static std::string Describe(const DeveloperAiCommandEnvelope& envelope);
};

} // namespace subspace
