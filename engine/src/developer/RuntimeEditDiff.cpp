#include "developer/diff/RuntimeEditDiff.h"

#include <sstream>

namespace subspace {

std::string RuntimeEditDiff::Summary() const
{
    std::ostringstream out;
    out << (label.empty() ? sourceCommand.name : label) << " (" << entries.size() << " change";
    if (entries.size() != 1) {
        out << "s";
    }
    out << ")";
    return out.str();
}

RuntimeEditDiffBuilder::RuntimeEditDiffBuilder(RuntimeEditCommand command)
{
    _diff.sourceCommand = std::move(command);
}

RuntimeEditDiffBuilder& RuntimeEditDiffBuilder::SetLabel(std::string label)
{
    _diff.label = std::move(label);
    return *this;
}

RuntimeEditDiffBuilder& RuntimeEditDiffBuilder::Add(std::string target, std::string field, std::string beforeValue, std::string afterValue)
{
    _diff.entries.push_back({std::move(target), std::move(field), std::move(beforeValue), std::move(afterValue)});
    return *this;
}

RuntimeEditDiff RuntimeEditDiffBuilder::Build() const
{
    return _diff;
}

} // namespace subspace
