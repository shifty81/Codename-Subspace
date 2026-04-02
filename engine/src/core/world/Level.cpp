#include "core/world/Level.h"

namespace subspace {

Level::Level(const std::string& name) : _name(name) {}

void Level::Load() {
    if (_loaded) return;
    if (_onLoad) _onLoad(_em);
    _loaded = true;
}

void Level::Unload() {
    if (!_loaded) return;
    if (_onUnload) _onUnload(_em);
    // Destroy all entities in the scene's EntityManager.
    auto entities = _em.GetAllEntities();
    for (Entity* e : entities) {
        if (e) _em.DestroyEntity(e->id);
    }
    _loaded = false;
}

} // namespace subspace
