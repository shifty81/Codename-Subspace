#include "editor/ForgeWorkspaceSystem.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace subspace {
namespace {

std::string ReadAll(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    std::ostringstream out;
    out << in.rdbuf();
    return out.str();
}

std::size_t SkipWs(const std::string& s, std::size_t p) {
    while (p < s.size() && std::isspace(static_cast<unsigned char>(s[p]))) ++p;
    return p;
}

std::string Unescape(std::string value) {
    std::string out; out.reserve(value.size());
    bool escape = false;
    for (char c : value) {
        if (escape) {
            switch (c) { case 'n': out.push_back('\n'); break; case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break; default: out.push_back(c); break; }
            escape = false;
        } else if (c == '\\') escape = true;
        else out.push_back(c);
    }
    return out;
}

std::string StringField(const std::string& object, const std::string& key) {
    const std::string needle = "\"" + key + "\"";
    std::size_t p = object.find(needle); if (p == std::string::npos) return {};
    p = object.find(':', p + needle.size()); if (p == std::string::npos) return {};
    p = SkipWs(object, p + 1); if (p >= object.size() || object[p] != '"') return {};
    ++p; std::string raw; bool escape = false;
    for (; p < object.size(); ++p) {
        const char c = object[p];
        if (!escape && c == '"') break;
        if (!escape && c == '\\') { escape = true; raw.push_back(c); continue; }
        escape = false; raw.push_back(c);
    }
    return Unescape(raw);
}

bool BoolField(const std::string& object, const std::string& key, bool fallback = false) {
    const std::string needle = "\"" + key + "\"";
    std::size_t p = object.find(needle); if (p == std::string::npos) return fallback;
    p = object.find(':', p + needle.size()); if (p == std::string::npos) return fallback;
    p = SkipWs(object, p + 1);
    if (object.compare(p, 4, "true") == 0) return true;
    if (object.compare(p, 5, "false") == 0) return false;
    return fallback;
}

std::string ObjectField(const std::string& text, const std::string& key) {
    const std::string needle = "\"" + key + "\"";
    std::size_t p = text.find(needle); if (p == std::string::npos) return {};
    p = text.find('{', p + needle.size()); if (p == std::string::npos) return {};
    const std::size_t start = p; int depth = 0; bool quoted = false, escape = false;
    for (; p < text.size(); ++p) {
        const char c = text[p];
        if (quoted) { if (!escape && c == '"') quoted = false; escape = !escape && c == '\\'; if (c != '\\') escape = false; continue; }
        if (c == '"') { quoted = true; escape = false; continue; }
        if (c == '{') ++depth;
        else if (c == '}' && --depth == 0) return text.substr(start, p - start + 1);
    }
    return {};
}

std::string ArrayField(const std::string& text, const std::string& key) {
    const std::string needle = "\"" + key + "\"";
    std::size_t p = text.find(needle); if (p == std::string::npos) return {};
    p = text.find('[', p + needle.size()); if (p == std::string::npos) return {};
    const std::size_t start = p; int depth = 0; bool quoted = false, escape = false;
    for (; p < text.size(); ++p) {
        const char c = text[p];
        if (quoted) { if (!escape && c == '"') quoted = false; escape = !escape && c == '\\'; if (c != '\\') escape = false; continue; }
        if (c == '"') { quoted = true; escape = false; continue; }
        if (c == '[') ++depth;
        else if (c == ']' && --depth == 0) return text.substr(start, p - start + 1);
    }
    return {};
}

std::vector<std::string> StringArray(const std::string& object, const std::string& key) {
    const auto array = ArrayField(object, key); std::vector<std::string> out;
    bool quoted = false, escape = false; std::string raw;
    for (char c : array) {
        if (!quoted) { if (c == '"') { quoted = true; raw.clear(); } continue; }
        if (!escape && c == '"') { out.push_back(Unescape(raw)); quoted = false; continue; }
        if (!escape && c == '\\') { escape = true; raw.push_back(c); continue; }
        escape = false; raw.push_back(c);
    }
    return out;
}

std::vector<std::string> ObjectsInArray(const std::string& array) {
    std::vector<std::string> out; int depth = 0; bool quoted = false, escape = false; std::size_t start = std::string::npos;
    for (std::size_t p = 0; p < array.size(); ++p) {
        const char c = array[p];
        if (quoted) { if (!escape && c == '"') quoted = false; escape = !escape && c == '\\'; if (c != '\\') escape = false; continue; }
        if (c == '"') { quoted = true; escape = false; continue; }
        if (c == '{') { if (depth++ == 0) start = p; }
        else if (c == '}' && depth > 0 && --depth == 0 && start != std::string::npos) {
            out.push_back(array.substr(start, p - start + 1)); start = std::string::npos;
        }
    }
    return out;
}

std::string Lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return value;
}

bool HasPs1(const ForgeWorkspaceCommand& command) {
    for (const auto& arg : command.arguments) {
        const auto lower = Lower(arg);
        if (lower.size() >= 4 && lower.substr(lower.size() - 4) == ".ps1") return true;
    }
    return false;
}

} // namespace

ForgeWorkspaceSnapshot ForgeWorkspaceSystem::Discover(const std::filesystem::path& start) {
    std::error_code ec;
    auto current = std::filesystem::absolute(start, ec);
    if (ec) current = start;
    if (std::filesystem::is_regular_file(current, ec)) current = current.parent_path();
    for (int i = 0; i < 12 && !current.empty(); ++i) {
        const auto candidate = current / "project.control.json";
        if (std::filesystem::exists(candidate, ec) && !ec) return Load(candidate);
        const auto parent = current.parent_path();
        if (parent == current) break;
        current = parent;
    }
    ForgeWorkspaceSnapshot out; out.status = "project.control.json not found from editor working path"; return out;
}

ForgeWorkspaceSnapshot ForgeWorkspaceSystem::Load(const std::filesystem::path& projectControlPath) {
    ForgeWorkspaceSnapshot out; out.controlPath = projectControlPath; out.projectRoot = projectControlPath.parent_path();
    const auto text = ReadAll(projectControlPath);
    if (text.empty()) { out.status = "project control file is empty or unreadable"; return out; }

    out.schema = StringField(text, "schema");
    const auto project = ObjectField(text, "project");
    out.projectId = StringField(project, "id");
    out.projectName = StringField(project, "name");
    out.version = StringField(project, "version");
    out.build = StringField(project, "build");
    if (out.projectId.empty()) out.projectId = StringField(text, "id");
    if (out.projectName.empty()) out.projectName = StringField(text, "name");

    const auto commands = ArrayField(text, "commands");
    for (const auto& object : ObjectsInArray(commands)) {
        ForgeWorkspaceCommand command;
        command.key = StringField(object, "key");
        command.label = StringField(object, "label");
        command.category = StringField(object, "category");
        command.mutates = BoolField(object, "mutates");
        command.requiresConfirmation = BoolField(object, "requiresConfirmation");
        command.executable = StringField(object, "executable");
        command.arguments = StringArray(object, "arguments");
        if (!command.key.empty() && !command.executable.empty()) out.commands.push_back(std::move(command));
    }

    out.valid = out.schema == "forge.project.v1" && !out.projectId.empty() && !out.commands.empty();
    out.status = out.valid ? ("Forge/PCC provider ready: " + out.projectName + " / " + std::to_string(out.commands.size()) + " commands")
                           : "project control contract is incomplete";
    return out;
}

const ForgeWorkspaceCommand* ForgeWorkspaceSystem::Find(const ForgeWorkspaceSnapshot& snapshot,
                                                         const std::string& key) {
    for (const auto& command : snapshot.commands) if (command.key == key) return &command;
    return nullptr;
}

bool ForgeWorkspaceSystem::InterpreterMatchesScript(const ForgeWorkspaceCommand& command,
                                                     std::string* error) {
    if (!HasPs1(command)) return true;
    const auto executable = Lower(command.executable);
    if (executable.find("pwsh") != std::string::npos || executable.find("powershell") != std::string::npos) return true;
    if (error) *error = "PowerShell script command is not assigned to pwsh/PowerShell";
    return false;
}

std::string ForgeWorkspaceSystem::CommandPreview(const ForgeWorkspaceCommand& command) {
    auto quote = [](const std::string& token) {
        if (token.find_first_of(" \t\"") == std::string::npos) return token;
        std::string escaped; escaped.reserve(token.size() + 2); escaped.push_back('"');
        for (char c : token) { if (c == '"') escaped.push_back('\\'); escaped.push_back(c); }
        escaped.push_back('"'); return escaped;
    };
    std::string out = quote(command.executable);
    for (const auto& arg : command.arguments) out += " " + quote(arg);
    return out;
}

} // namespace subspace
