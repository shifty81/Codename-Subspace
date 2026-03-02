#pragma once

#include "core/Math.h"

#include <cstdint>
#include <functional>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace subspace {

/// Unique identifier for a navigation node.
using NavNodeId = uint32_t;
constexpr NavNodeId InvalidNavNodeId = 0;

/// A single node in the navigation graph.
struct NavNode {
    NavNodeId id = InvalidNavNodeId;
    Vector3 position;
    bool blocked = false;   ///< If true, node cannot be traversed.
    float cost = 1.0f;      ///< Movement cost multiplier (e.g., hazardous area = 2.0).
};

/// A connection between two navigation nodes.
struct NavEdge {
    NavNodeId from = InvalidNavNodeId;
    NavNodeId to = InvalidNavNodeId;
    float weight = 1.0f;    ///< Travel cost along this edge.
};

/// A completed path from start to goal.
struct NavPath {
    std::vector<Vector3> waypoints;  ///< Ordered positions from start to goal.
    float totalCost = 0.0f;          ///< Sum of edge costs along the path.
    bool valid = false;              ///< True if a path was found.

    bool IsEmpty() const { return waypoints.empty(); }
    size_t Length() const { return waypoints.size(); }
};

/// 3D navigation graph supporting A* pathfinding.
/// Can be used as a sparse graph or a regular 3D grid.
class NavGraph {
public:
    NavGraph() = default;

    /// Add a node to the graph. Returns the node id.
    NavNodeId AddNode(const Vector3& position, float cost = 1.0f);

    /// Add a bidirectional edge between two nodes.
    /// Weight defaults to the Euclidean distance between nodes multiplied by
    /// the average cost of the two nodes.
    void AddEdge(NavNodeId from, NavNodeId to, float weight = -1.0f);

    /// Add a one-way (directed) edge.
    void AddDirectedEdge(NavNodeId from, NavNodeId to, float weight = -1.0f);

    /// Remove a node and all its edges.
    void RemoveNode(NavNodeId id);

    /// Remove the edge(s) between two nodes.
    void RemoveEdge(NavNodeId from, NavNodeId to);

    /// Block / unblock a node (blocked nodes are not traversable).
    void SetBlocked(NavNodeId id, bool blocked);

    /// Check whether a node is blocked.
    bool IsBlocked(NavNodeId id) const;

    /// Get a node by id (returns nullptr if not found).
    const NavNode* GetNode(NavNodeId id) const;

    /// Get all edges from a given node.
    const std::vector<NavEdge>& GetEdges(NavNodeId id) const;

    /// Total number of nodes in the graph.
    size_t NodeCount() const;

    /// Total number of edges in the graph.
    size_t EdgeCount() const;

    /// Clear the entire graph.
    void Clear();

    /// Find the nearest node to a world position.
    NavNodeId FindNearest(const Vector3& position) const;

    /// Build a uniform 3D grid of nodes.
    /// Creates a grid of (countX * countY * countZ) nodes centered at origin,
    /// with the given spacing. Neighboring nodes are connected automatically.
    void BuildGrid(const Vector3& origin, float spacing,
                   int countX, int countY, int countZ);

private:
    NavNodeId _nextId = 1;
    std::unordered_map<NavNodeId, NavNode> _nodes;
    std::unordered_map<NavNodeId, std::vector<NavEdge>> _adjacency;
    static const std::vector<NavEdge> _emptyEdges;

    float CalculateEdgeWeight(NavNodeId from, NavNodeId to) const;
};

/// A* pathfinding algorithm operating on a NavGraph.
class Pathfinder {
public:
    explicit Pathfinder(const NavGraph& graph);

    /// Find a path from startId to goalId using A*.
    NavPath FindPath(NavNodeId startId, NavNodeId goalId) const;

    /// Find a path from world positions (snaps to nearest nodes).
    NavPath FindPathByPosition(const Vector3& start, const Vector3& goal) const;

    /// Heuristic function: Euclidean distance (default).
    static float EuclideanHeuristic(const Vector3& a, const Vector3& b);

    /// Heuristic function: Manhattan distance.
    static float ManhattanHeuristic(const Vector3& a, const Vector3& b);

    /// Set a custom heuristic function (defaults to Euclidean).
    void SetHeuristic(std::function<float(const Vector3&, const Vector3&)> h);

    /// Get the number of nodes explored in the last search.
    size_t LastNodesExplored() const { return _lastExplored; }

private:
    const NavGraph& _graph;
    std::function<float(const Vector3&, const Vector3&)> _heuristic;
    mutable size_t _lastExplored = 0;
};

/// Smooth a raw path by removing collinear waypoints.
NavPath SmoothPath(const NavPath& raw, float tolerance = 0.01f);

} // namespace subspace
