#include "developer/ai/DeveloperAiCommandProtocol.h"

namespace subspace {

DeveloperAiCommandEnvelope DeveloperAiCommandProtocol::MakeEnvelope(std::string requestId, std::string actor, RuntimeEditCommand command, std::string intent)
{
    DeveloperAiCommandEnvelope envelope;
    envelope.requestId = std::move(requestId);
    envelope.actor = std::move(actor);
    envelope.intent = std::move(intent);
    envelope.requiresConfirmation = IsWriteCommand(command);
    envelope.command = std::move(command);
    return envelope;
}

bool DeveloperAiCommandProtocol::IsWriteCommand(const RuntimeEditCommand& command)
{
    if (command.name.rfind("entity.inspect", 0) == 0 ||
        command.name.rfind("asset.validate", 0) == 0 ||
        command.name.rfind("ship.validate", 0) == 0) {
        return false;
    }
    return command.name.find(".set") != std::string::npos ||
           command.name.find(".add") != std::string::npos ||
           command.name.find(".remove") != std::string::npos ||
           command.name.find(".delete") != std::string::npos ||
           command.name.find(".place") != std::string::npos ||
           command.name.find(".paint") != std::string::npos ||
           command.name.find("reload") != std::string::npos ||
           command.name.find("promote") != std::string::npos;
}

std::string DeveloperAiCommandProtocol::Describe(const DeveloperAiCommandEnvelope& envelope)
{
    std::string text = envelope.actor + " requested " + envelope.command.name;
    if (!envelope.intent.empty()) {
        text += " for " + envelope.intent;
    }
    if (envelope.requiresConfirmation) {
        text += " [confirmation required]";
    }
    return text;
}

} // namespace subspace
