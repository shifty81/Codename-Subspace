#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class DeveloperValidationSeverity {
    Info,
    Warning,
    Error,
};

struct DeveloperValidationIssue {
    DeveloperValidationSeverity severity = DeveloperValidationSeverity::Info;
    std::string code;
    std::string path;
    std::string message;
};

class DeveloperValidationReport {
public:
    void AddInfo(std::string code, std::string path, std::string message);
    void AddWarning(std::string code, std::string path, std::string message);
    void AddError(std::string code, std::string path, std::string message);

    bool HasErrors() const;
    std::size_t Count(DeveloperValidationSeverity severity) const;
    const std::vector<DeveloperValidationIssue>& GetIssues() const { return _issues; }
    std::string Summary() const;
    void Clear();

private:
    void Add(DeveloperValidationSeverity severity, std::string code, std::string path, std::string message);
    std::vector<DeveloperValidationIssue> _issues;
};

} // namespace subspace
