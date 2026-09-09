#pragma once

#include <string>

namespace subspace {

/// Base class for systems that process entities with specific components.
class SystemBase {
public:
    explicit SystemBase(const std::string& name) : _name(name) {}
    virtual ~SystemBase() = default;

    const std::string& GetName() const { return _name; }
    bool IsEnabled() const { return _isEnabled; }
    void SetEnabled(bool enabled) { _isEnabled = enabled; }

    /// Update the system each frame.
    virtual void Update(float deltaTime) = 0;

    /// Initialize the system (called once at registration).
    virtual void Initialize() {}

    /// Clean up system resources.
    virtual void Shutdown() {}

protected:
    std::string _name;
    bool _isEnabled = true;
};

} // namespace subspace
