#pragma once

#include "developer/events/DeveloperEventFeed.h"
#include "developer/validation/DeveloperValidationReport.h"

#include <string>
#include <vector>

namespace subspace {

class DeveloperDiagnosticsHub {
public:
    DeveloperEventFeed& Events() { return _events; }
    const DeveloperEventFeed& Events() const { return _events; }

    DeveloperValidationReport& Validation() { return _validation; }
    const DeveloperValidationReport& Validation() const { return _validation; }

    void Info(std::string title, std::string message, std::string source = {});
    void Warning(std::string title, std::string message, std::string source = {});
    void Error(std::string title, std::string message, std::string source = {});
    std::string Summary() const;

private:
    DeveloperEventFeed _events;
    DeveloperValidationReport _validation;
};

} // namespace subspace
