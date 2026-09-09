#include "developer/reflection/ComponentReflectionRegistry.h"

#include <algorithm>

namespace subspace {

void ComponentReflectionRegistry::RegisterComponent(ReflectedComponentType type, Getter getter, Setter setter)
{
    if (type.name.empty()) {
        return;
    }
    const std::string key = type.name;
    _entries[key] = Entry{std::move(type), std::move(getter), std::move(setter)};
}

bool ComponentReflectionRegistry::HasComponent(const std::string& component) const
{
    return _entries.find(component) != _entries.end();
}

bool ComponentReflectionRegistry::HasField(const std::string& component, const std::string& field) const
{
    auto it = _entries.find(component);
    if (it == _entries.end()) {
        return false;
    }
    const auto& fields = it->second.type.fields;
    return std::any_of(fields.begin(), fields.end(), [&](const ReflectedComponentField& f) { return f.name == field; });
}

std::string ComponentReflectionRegistry::GetField(std::uint64_t entityId, const std::string& component, const std::string& field) const
{
    auto it = _entries.find(component);
    if (it == _entries.end() || !it->second.getter || !HasField(component, field)) {
        return {};
    }
    return it->second.getter(entityId, field);
}

bool ComponentReflectionRegistry::SetField(std::uint64_t entityId, const std::string& component, const std::string& field, const std::string& value, std::string& message) const
{
    auto it = _entries.find(component);
    if (it == _entries.end()) {
        message = "Unknown component: " + component;
        return false;
    }
    if (!HasField(component, field)) {
        message = "Unknown field: " + component + "." + field;
        return false;
    }
    if (!it->second.setter) {
        message = "Component is read-only: " + component;
        return false;
    }
    return it->second.setter(entityId, field, value, message);
}

std::vector<ReflectedComponentType> ComponentReflectionRegistry::GetComponents() const
{
    std::vector<ReflectedComponentType> result;
    result.reserve(_entries.size());
    for (const auto& kv : _entries) {
        result.push_back(kv.second.type);
    }
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.name < b.name; });
    return result;
}

std::vector<ComponentFieldValue> ComponentReflectionRegistry::Snapshot(std::uint64_t entityId, const std::vector<std::string>& components) const
{
    std::vector<ComponentFieldValue> values;
    auto appendComponent = [&](const Entry& entry) {
        for (const auto& field : entry.type.fields) {
            values.push_back({entry.type.name, field.name, entry.getter ? entry.getter(entityId, field.name) : std::string{}});
        }
    };

    if (components.empty()) {
        for (const auto& kv : _entries) {
            appendComponent(kv.second);
        }
    } else {
        for (const auto& name : components) {
            auto it = _entries.find(name);
            if (it != _entries.end()) {
                appendComponent(it->second);
            }
        }
    }
    return values;
}

void ComponentReflectionRegistry::Clear()
{
    _entries.clear();
}

} // namespace subspace
