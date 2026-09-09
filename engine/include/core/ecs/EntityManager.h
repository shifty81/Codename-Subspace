#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"

#include <memory>
#include <mutex>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace subspace {

/// Manages entities and their components in the ECS architecture.
class EntityManager {
public:
    /// Create a new entity with a unique ID.
    Entity& CreateEntity(const std::string& name = "Entity");

    /// Destroy an entity and remove all its components.
    void DestroyEntity(EntityId entityId);

    /// Get entity by ID (returns nullptr if not found).
    Entity* GetEntity(EntityId entityId);
    const Entity* GetEntity(EntityId entityId) const;

    /// Get all entities.
    std::vector<Entity*> GetAllEntities();

    /// Add a component to an entity (takes ownership).
    template<typename T>
    T* AddComponent(EntityId entityId, std::unique_ptr<T> component);

    /// Get a component from an entity (returns nullptr if not found).
    template<typename T>
    T* GetComponent(EntityId entityId);

    /// Check if an entity has a specific component type.
    template<typename T>
    bool HasComponent(EntityId entityId) const;

    /// Remove a component from an entity.
    template<typename T>
    void RemoveComponent(EntityId entityId);

    /// Get all components of a specific type.
    template<typename T>
    std::vector<T*> GetAllComponents();

    /// Register a system.
    void RegisterSystem(std::unique_ptr<SystemBase> system);

    /// Update all enabled systems.
    void UpdateSystems(float deltaTime);

    /// Shutdown all systems.
    void Shutdown();

    /// Get entity count.
    size_t GetEntityCount() const;

private:
    EntityId NextId();

    mutable std::mutex _mutex;
    EntityId _nextId = 1;
    std::unordered_map<EntityId, Entity> _entities;
    // type_index -> (entityId -> component)
    std::unordered_map<std::type_index,
        std::unordered_map<EntityId, std::unique_ptr<IComponent>>> _components;
    std::vector<std::unique_ptr<SystemBase>> _systems;
};

// ---- template implementations ----

template<typename T>
T* EntityManager::AddComponent(EntityId entityId, std::unique_ptr<T> component)
{
    static_assert(std::is_base_of<IComponent, T>::value, "T must derive from IComponent");
    std::lock_guard<std::mutex> lock(_mutex);

    if (_entities.find(entityId) == _entities.end()) return nullptr;

    component->entityId = entityId;
    auto* ptr = component.get();
    _components[std::type_index(typeid(T))][entityId] = std::move(component);
    return ptr;
}

template<typename T>
T* EntityManager::GetComponent(EntityId entityId)
{
    static_assert(std::is_base_of<IComponent, T>::value, "T must derive from IComponent");
    std::lock_guard<std::mutex> lock(_mutex);

    auto typeIt = _components.find(std::type_index(typeid(T)));
    if (typeIt == _components.end()) return nullptr;

    auto entIt = typeIt->second.find(entityId);
    if (entIt == typeIt->second.end()) return nullptr;

    return static_cast<T*>(entIt->second.get());
}

template<typename T>
bool EntityManager::HasComponent(EntityId entityId) const
{
    static_assert(std::is_base_of<IComponent, T>::value, "T must derive from IComponent");
    std::lock_guard<std::mutex> lock(_mutex);

    auto typeIt = _components.find(std::type_index(typeid(T)));
    if (typeIt == _components.end()) return false;
    return typeIt->second.find(entityId) != typeIt->second.end();
}

template<typename T>
void EntityManager::RemoveComponent(EntityId entityId)
{
    static_assert(std::is_base_of<IComponent, T>::value, "T must derive from IComponent");
    std::lock_guard<std::mutex> lock(_mutex);

    auto typeIt = _components.find(std::type_index(typeid(T)));
    if (typeIt != _components.end()) {
        typeIt->second.erase(entityId);
    }
}

template<typename T>
std::vector<T*> EntityManager::GetAllComponents()
{
    static_assert(std::is_base_of<IComponent, T>::value, "T must derive from IComponent");
    std::lock_guard<std::mutex> lock(_mutex);

    std::vector<T*> result;
    auto typeIt = _components.find(std::type_index(typeid(T)));
    if (typeIt != _components.end()) {
        result.reserve(typeIt->second.size());
        for (auto& [id, comp] : typeIt->second) {
            result.push_back(static_cast<T*>(comp.get()));
        }
    }
    return result;
}

} // namespace subspace
