#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

namespace subspace {

/// Return status of a single behavior-tree node tick.
enum class BTStatus {
    Running, ///< Node is still executing; tick again next frame.
    Success, ///< Node completed successfully.
    Failure  ///< Node completed with failure.
};

// ---------------------------------------------------------------------------
// Base node
// ---------------------------------------------------------------------------

/// Abstract base class for all behavior tree nodes.
class BTNode {
public:
    virtual ~BTNode() = default;

    /// Advance the node's logic by one tick.
    /// @param dt  Elapsed seconds since the last tick.
    virtual BTStatus Tick(float dt) = 0;

    /// Reset the node to its initial state (called when a parent restarts).
    virtual void Reset() {}
};

// ---------------------------------------------------------------------------
// Composite nodes
// ---------------------------------------------------------------------------

/// Sequence node (AND): ticks children left-to-right.
/// Returns Failure as soon as any child fails.
/// Returns Success when all children succeed.
/// Remembers the current child across ticks so long-running children work.
class BTSequence : public BTNode {
public:
    void AddChild(std::shared_ptr<BTNode> child) {
        _children.push_back(std::move(child));
    }

    BTStatus Tick(float dt) override;
    void     Reset()        override;

private:
    std::vector<std::shared_ptr<BTNode>> _children;
    std::size_t _current = 0;
};

/// Selector node (OR): ticks children left-to-right.
/// Returns Success as soon as any child succeeds.
/// Returns Failure when all children fail.
class BTSelector : public BTNode {
public:
    void AddChild(std::shared_ptr<BTNode> child) {
        _children.push_back(std::move(child));
    }

    BTStatus Tick(float dt) override;
    void     Reset()        override;

private:
    std::vector<std::shared_ptr<BTNode>> _children;
    std::size_t _current = 0;
};

/// Parallel node: ticks ALL children every frame.
/// Returns Success when successThreshold children have succeeded.
/// Returns Failure when enough children have failed to make success impossible.
class BTParallel : public BTNode {
public:
    /// @param successThreshold  Number of children that must succeed.
    explicit BTParallel(std::size_t successThreshold = 1)
        : _successThreshold(successThreshold) {}

    void AddChild(std::shared_ptr<BTNode> child) {
        _children.push_back(std::move(child));
    }

    BTStatus Tick(float dt) override;
    void     Reset()        override;

private:
    std::vector<std::shared_ptr<BTNode>> _children;
    std::size_t _successThreshold = 1;
};

/// Inverter decorator: flips Success ↔ Failure; Running passes through.
class BTInverter : public BTNode {
public:
    explicit BTInverter(std::shared_ptr<BTNode> child)
        : _child(std::move(child)) {}

    BTStatus Tick(float dt) override {
        BTStatus s = _child ? _child->Tick(dt) : BTStatus::Failure;
        if (s == BTStatus::Success) return BTStatus::Failure;
        if (s == BTStatus::Failure) return BTStatus::Success;
        return s;
    }

    void Reset() override { if (_child) _child->Reset(); }

private:
    std::shared_ptr<BTNode> _child;
};

// ---------------------------------------------------------------------------
// Leaf node
// ---------------------------------------------------------------------------

/// Leaf node whose behaviour is defined by an arbitrary callback.
class BTLeaf : public BTNode {
public:
    using Action = std::function<BTStatus(float dt)>;

    explicit BTLeaf(Action action) : _action(std::move(action)) {}

    BTStatus Tick(float dt) override {
        return _action ? _action(dt) : BTStatus::Failure;
    }

private:
    Action _action;
};

// ---------------------------------------------------------------------------
// Condition node (always leaf-level)
// ---------------------------------------------------------------------------

/// Condition node: evaluates a predicate and returns Success/Failure instantly.
class BTCondition : public BTNode {
public:
    using Predicate = std::function<bool()>;
    explicit BTCondition(Predicate pred) : _pred(std::move(pred)) {}

    BTStatus Tick(float /*dt*/) override {
        return (_pred && _pred()) ? BTStatus::Success : BTStatus::Failure;
    }

private:
    Predicate _pred;
};

// ---------------------------------------------------------------------------
// BehaviorTree driver
// ---------------------------------------------------------------------------

/// Thin driver that owns the root node and forwards ticks to it.
class BehaviorTree {
public:
    void SetRoot(std::shared_ptr<BTNode> root) { _root = std::move(root); }

    /// Advance the entire tree by one tick.
    /// @return The root node's status, or BTStatus::Failure if no root is set.
    BTStatus Tick(float dt);

    /// Reset the entire tree to its initial state.
    void Reset();

private:
    std::shared_ptr<BTNode> _root;
};

} // namespace subspace
