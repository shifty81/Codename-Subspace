#include "developer/diagnostics/DeveloperDiagnosticsHub.h"

#include <sstream>

namespace subspace {

void DeveloperDiagnosticsHub::Info(std::string title, std::string message, std::string source)
{
    _events.Push({DeveloperEventKind::Info, std::move(title), std::move(message), std::move(source), 0.0});
}

void DeveloperDiagnosticsHub::Warning(std::string title, std::string message, std::string source)
{
    _events.Push({DeveloperEventKind::Warning, std::move(title), std::move(message), std::move(source), 0.0});
}

void DeveloperDiagnosticsHub::Error(std::string title, std::string message, std::string source)
{
    _events.Push({DeveloperEventKind::Error, std::move(title), std::move(message), std::move(source), 0.0});
}

std::string DeveloperDiagnosticsHub::Summary() const
{
    std::ostringstream out;
    out << "events=" << _events.GetEvents().size() << ", validation=" << _validation.Summary();
    return out.str();
}

} // namespace subspace
