#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class ConversionGateStatus { Pending, Ported, Replaced, ReferenceOnly, Deferred, Rejected };

struct ConversionGateEntry {
    std::string sourcePath;
    std::string cppOwner;
    ConversionGateStatus status = ConversionGateStatus::Pending;
    std::string note;
};

struct ConversionGateReport {
    int pending = 0;
    int ported = 0;
    int replaced = 0;
    int referenceOnly = 0;
    int deferred = 0;
    int rejected = 0;
    std::vector<std::string> warnings;
};

const char* ConversionGateStatusName(ConversionGateStatus status);
ConversionGateReport BuildConversionGateReport(const std::vector<ConversionGateEntry>& entries);
bool IsConversionGateCleanEnoughForArchive(const ConversionGateReport& report);

} // namespace subspace
