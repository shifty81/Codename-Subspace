#pragma once

#include "developer/DeveloperCommandBridge.h"

#include <string>
#include <vector>

namespace subspace {

struct DeveloperCommandScriptLine {
    std::size_t lineNumber = 0;
    std::string text;
    bool skipped = false;
};

struct DeveloperCommandScriptReport {
    bool success = true;
    std::size_t executed = 0;
    std::size_t skipped = 0;
    std::vector<DeveloperCommandExecution> executions;
    std::vector<std::string> messages;
};

class DeveloperCommandScriptRunner {
public:
    void SetContinueOnError(bool value) { _continueOnError = value; }
    bool GetContinueOnError() const { return _continueOnError; }

    DeveloperCommandScriptReport RunLines(const std::vector<std::string>& lines, const DeveloperCommandBridge& bridge) const;

private:
    static std::string Trim(const std::string& value);
    static bool IsCommentOrEmpty(const std::string& value);

    bool _continueOnError = false;
};

} // namespace subspace
