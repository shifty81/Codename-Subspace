#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct ReflectedComponentField {
    std::string name;
    std::string type;
    std::string displayName;
    bool editable = true;
};

struct ReflectedComponentType {
    std::string name;
    std::string displayName;
    std::vector<ReflectedComponentField> fields;
};

struct ComponentFieldValue {
    std::string component;
    std::string field;
    std::string value;
};

class ComponentReflectionRegistry {
public:
    using Getter = std::function<std::string(std::uint64_t entityId, const std::string& field)>;
    using Setter = std::function<bool(std::uint64_t entityId, const std::string& field, const std::string& value, std::string& message)>;

    void RegisterComponent(ReflectedComponentType type, Getter getter = {}, Setter setter = {});
    bool HasComponent(const std::string& component) const;
    bool HasField(const std::string& component, const std::string& field) const;

    std::string GetField(std::uint64_t entityId, const std::string& component, const std::string& field) const;
    bool SetField(std::uint64_t entityId, const std::string& component, const std::string& field, const std::string& value, std::string& message) const;

    std::vector<ReflectedComponentType> GetComponents() const;
    std::vector<ComponentFieldValue> Snapshot(std::uint64_t entityId, const std::vector<std::string>& components = {}) const;
    void Clear();

private:
    struct Entry {
        ReflectedComponentType type;
        Getter getter;
        Setter setter;
    };

    std::unordered_map<std::string, Entry> _entries;
};

} // namespace subspace
