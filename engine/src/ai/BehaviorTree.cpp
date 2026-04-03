#include "ai/BehaviorTree.h"

namespace subspace {

// ---------------------------------------------------------------------------
// BTSequence
// ---------------------------------------------------------------------------

BTStatus BTSequence::Tick(float dt) {
    while (_current < _children.size()) {
        BTStatus s = _children[_current]->Tick(dt);
        if (s == BTStatus::Running) return BTStatus::Running;
        if (s == BTStatus::Failure) {
            _current = 0;
            return BTStatus::Failure;
        }
        ++_current; // Success: advance to next child
    }
    _current = 0;
    return BTStatus::Success;
}

void BTSequence::Reset() {
    _current = 0;
    for (auto& c : _children) c->Reset();
}

// ---------------------------------------------------------------------------
// BTSelector
// ---------------------------------------------------------------------------

BTStatus BTSelector::Tick(float dt) {
    while (_current < _children.size()) {
        BTStatus s = _children[_current]->Tick(dt);
        if (s == BTStatus::Running) return BTStatus::Running;
        if (s == BTStatus::Success) {
            _current = 0;
            return BTStatus::Success;
        }
        ++_current; // Failure: try next child
    }
    _current = 0;
    return BTStatus::Failure;
}

void BTSelector::Reset() {
    _current = 0;
    for (auto& c : _children) c->Reset();
}

// ---------------------------------------------------------------------------
// BTParallel
// ---------------------------------------------------------------------------

BTStatus BTParallel::Tick(float dt) {
    std::size_t successes = 0;
    std::size_t failures  = 0;

    for (auto& child : _children) {
        BTStatus s = child->Tick(dt);
        if (s == BTStatus::Success) ++successes;
        else if (s == BTStatus::Failure) ++failures;
    }

    if (successes >= _successThreshold) return BTStatus::Success;
    std::size_t needed = _successThreshold - successes;
    std::size_t possible = _children.size() - failures;
    if (possible < needed) return BTStatus::Failure;
    return BTStatus::Running;
}

void BTParallel::Reset() {
    for (auto& c : _children) c->Reset();
}

// ---------------------------------------------------------------------------
// BehaviorTree driver
// ---------------------------------------------------------------------------

BTStatus BehaviorTree::Tick(float dt) {
    return _root ? _root->Tick(dt) : BTStatus::Failure;
}

void BehaviorTree::Reset() {
    if (_root) _root->Reset();
}

} // namespace subspace
