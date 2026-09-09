#include "core/ecs/EntityManager.h"
#include "core/events/EventSystem.h"
#include "core/events/GameEvents.h"
#include "core/logging/Logger.h"

namespace subspace {

EntityId EntityManager::NextId()
{
    return _nextId++;
}

Entity& EntityManager::CreateEntity(const std::string& name)
{
    std::lock_guard<std::mutex> lock(_mutex);

    EntityId id = NextId();
    Entity entity;
    entity.id = id;
    entity.name = name;
    entity.isActive = true;

    auto [it, _] = _entities.emplace(id, std::move(entity));

    Logger::Instance().Debug("EntityManager",
        "Created entity: " + it->second.name + " (" + std::to_string(id) + ")");

    return it->second;
}

void EntityManager::DestroyEntity(EntityId entityId)
{
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _entities.find(entityId);
    if (it == _entities.end()) return;

    std::string entityName = it->second.name;

    // Remove all components belonging to this entity
    for (auto& [typeIdx, componentMap] : _components) {
        componentMap.erase(entityId);
    }

    _entities.erase(it);

    Logger::Instance().Debug("EntityManager",
        "Destroyed entity: " + entityName + " (" + std::to_string(entityId) + ")");
}

Entity* EntityManager::GetEntity(EntityId entityId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _entities.find(entityId);
    return it != _entities.end() ? &it->second : nullptr;
}

const Entity* EntityManager::GetEntity(EntityId entityId) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _entities.find(entityId);
    return it != _entities.end() ? &it->second : nullptr;
}

std::vector<Entity*> EntityManager::GetAllEntities()
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::vector<Entity*> result;
    result.reserve(_entities.size());
    for (auto& [id, entity] : _entities) {
        result.push_back(&entity);
    }
    return result;
}

void EntityManager::RegisterSystem(std::unique_ptr<SystemBase> system)
{
    system->Initialize();
    Logger::Instance().Info("EntityManager",
        "Registered system: " + system->GetName());
    _systems.push_back(std::move(system));
}

void EntityManager::UpdateSystems(float deltaTime)
{
    for (auto& system : _systems) {
        if (system->IsEnabled()) {
            system->Update(deltaTime);
        }
    }
}

void EntityManager::Shutdown()
{
    for (auto& system : _systems) {
        system->Shutdown();
    }
}

size_t EntityManager::GetEntityCount() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _entities.size();
}

} // namespace subspace
