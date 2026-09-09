#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class ConversionDisposition {
    NeedsPort,
    PartiallyPorted,
    PortedToCpp,
    ReplacedByCppDesign,
    ReferenceOnly,
    Deferred
};

struct ConversionBacklogItem {
    std::string sourcePath;
    std::string cppTargetArea;
    ConversionDisposition disposition = ConversionDisposition::NeedsPort;
    std::string note;
};

const char* ConversionDispositionName(ConversionDisposition disposition);
std::vector<ConversionBacklogItem> BuildHighLevelCppConversionBacklog();
std::vector<ConversionBacklogItem> FilterConversionBacklog(const std::vector<ConversionBacklogItem>& items,
                                                           ConversionDisposition disposition);
std::string CppConversionBacklogSummary(const std::vector<ConversionBacklogItem>& items);

} // namespace subspace
