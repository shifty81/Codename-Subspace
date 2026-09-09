#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct ConsoleResult { bool success=false; std::string output; };

class DeveloperConsole {
public:
    using Command = std::function<ConsoleResult(const std::vector<std::string>&)>;
    DeveloperConsole();
    void Register(const std::string& name, Command command);
    bool HasCommand(const std::string& name) const;
    ConsoleResult Execute(const std::string& line);
    const std::vector<std::string>& History() const { return history_; }
    std::vector<std::string> Commands() const;

private:
    static std::vector<std::string> Tokenize(const std::string& line);
    std::unordered_map<std::string, Command> commands_;
    std::unordered_map<std::string, std::string> variables_;
    std::vector<std::string> history_;
};

} // namespace subspace
