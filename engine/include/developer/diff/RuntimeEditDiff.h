#pragma once

#include "developer/RuntimeEditCommand.h"

#include <string>
#include <vector>

namespace subspace {

struct RuntimeEditDiffEntry {
    std::string target;
    std::string field;
    std::string beforeValue;
    std::string afterValue;
};

struct RuntimeEditDiff {
    std::string label;
    RuntimeEditCommand sourceCommand;
    std::vector<RuntimeEditDiffEntry> entries;

    bool Empty() const { return entries.empty(); }
    std::string Summary() const;
};

class RuntimeEditDiffBuilder {
public:
    explicit RuntimeEditDiffBuilder(RuntimeEditCommand command = {});
    RuntimeEditDiffBuilder& SetLabel(std::string label);
    RuntimeEditDiffBuilder& Add(std::string target, std::string field, std::string beforeValue, std::string afterValue);
    RuntimeEditDiff Build() const;

private:
    RuntimeEditDiff _diff;
};

} // namespace subspace
