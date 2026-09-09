#include "developer/script/DeveloperCommandScriptRunner.h"

#include <cctype>

namespace subspace {

std::string DeveloperCommandScriptRunner::Trim(const std::string& value) {
    std::size_t begin = 0;
    while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin]))) {
        ++begin;
    }
    std::size_t end = value.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(begin, end - begin);
}

bool DeveloperCommandScriptRunner::IsCommentOrEmpty(const std::string& value) {
    const std::string trimmed = Trim(value);
    return trimmed.empty() || trimmed.rfind("#", 0) == 0 || trimmed.rfind("//", 0) == 0;
}

DeveloperCommandScriptReport DeveloperCommandScriptRunner::RunLines(const std::vector<std::string>& lines, const DeveloperCommandBridge& bridge) const {
    DeveloperCommandScriptReport report;
    for (std::size_t index = 0; index < lines.size(); ++index) {
        const std::string text = Trim(lines[index]);
        if (IsCommentOrEmpty(text)) {
            ++report.skipped;
            continue;
        }

        DeveloperCommandExecution execution = bridge.ExecuteLine(text);
        report.executed++;
        report.executions.push_back(execution);
        if (!execution.success) {
            report.success = false;
            report.messages.push_back("Line " + std::to_string(index + 1) + " failed: " + execution.message);
            if (!_continueOnError) {
                break;
            }
        }
    }
    if (report.success) {
        report.messages.push_back("Developer command script completed successfully.");
    }
    return report;
}

} // namespace subspace
