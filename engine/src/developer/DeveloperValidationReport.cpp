#include "developer/validation/DeveloperValidationReport.h"

#include <sstream>

namespace subspace {

void DeveloperValidationReport::AddInfo(std::string code, std::string path, std::string message)
{
    Add(DeveloperValidationSeverity::Info, std::move(code), std::move(path), std::move(message));
}

void DeveloperValidationReport::AddWarning(std::string code, std::string path, std::string message)
{
    Add(DeveloperValidationSeverity::Warning, std::move(code), std::move(path), std::move(message));
}

void DeveloperValidationReport::AddError(std::string code, std::string path, std::string message)
{
    Add(DeveloperValidationSeverity::Error, std::move(code), std::move(path), std::move(message));
}

bool DeveloperValidationReport::HasErrors() const
{
    return Count(DeveloperValidationSeverity::Error) > 0;
}

std::size_t DeveloperValidationReport::Count(DeveloperValidationSeverity severity) const
{
    std::size_t total = 0;
    for (const auto& issue : _issues) {
        if (issue.severity == severity) {
            ++total;
        }
    }
    return total;
}

std::string DeveloperValidationReport::Summary() const
{
    std::ostringstream out;
    out << Count(DeveloperValidationSeverity::Error) << " error(s), "
        << Count(DeveloperValidationSeverity::Warning) << " warning(s), "
        << Count(DeveloperValidationSeverity::Info) << " info item(s)";
    return out.str();
}

void DeveloperValidationReport::Clear()
{
    _issues.clear();
}

void DeveloperValidationReport::Add(DeveloperValidationSeverity severity, std::string code, std::string path, std::string message)
{
    _issues.push_back({severity, std::move(code), std::move(path), std::move(message)});
}

} // namespace subspace
