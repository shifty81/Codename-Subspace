#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class CarbonIntakeDisposition { ImportCandidate, Adapt, Reference, RejectDirectUse, Defer };
struct CarbonComponentAssessment { std::string component; CarbonIntakeDisposition disposition=CarbonIntakeDisposition::Reference; std::string reason; };
class CarbonIntakeSystem {
public:
    std::vector<CarbonComponentAssessment> DefaultMatrix() const;
    const CarbonComponentAssessment* Find(const std::vector<CarbonComponentAssessment>& matrix,const std::string& component) const;
};

} // namespace subspace
