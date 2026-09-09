#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

class DeveloperCommandHistory {
public:
    explicit DeveloperCommandHistory(std::size_t capacity = 128);

    void Push(std::string commandLine);
    void Clear();

    bool Empty() const { return _entries.empty(); }
    std::size_t Size() const { return _entries.size(); }
    const std::vector<std::string>& Entries() const { return _entries; }

    std::string Previous();
    std::string Next();
    void ResetNavigation();

private:
    std::size_t _capacity = 128;
    std::vector<std::string> _entries;
    std::size_t _cursor = 0;
};

} // namespace subspace
