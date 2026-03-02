#include "navigation/Pathfinding.h"

#include <algorithm>
#include <cmath>

namespace subspace {

const std::vector<NavEdge> NavGraph::_emptyEdges;

// ---------------------------------------------------------------------------
// NavGraph
// ---------------------------------------------------------------------------

NavNodeId NavGraph::AddNode(const Vector3& position, float cost)
{
    NavNodeId id = _nextId++;
    NavNode node;
    node.id = id;
    node.position = position;
    node.cost = cost;
    _nodes[id] = node;
    return id;
}

void NavGraph::AddEdge(NavNodeId from, NavNodeId to, float weight)
{
    AddDirectedEdge(from, to, weight);
    AddDirectedEdge(to, from, weight);
}

void NavGraph::AddDirectedEdge(NavNodeId from, NavNodeId to, float weight)
{
    if (_nodes.find(from) == _nodes.end() || _nodes.find(to) == _nodes.end())
        return;

    float w = weight < 0.0f ? CalculateEdgeWeight(from, to) : weight;

    NavEdge edge;
    edge.from = from;
    edge.to = to;
    edge.weight = w;
    _adjacency[from].push_back(edge);
}

void NavGraph::RemoveNode(NavNodeId id)
{
    _nodes.erase(id);
    _adjacency.erase(id);

    // Remove all edges pointing to this node.
    for (auto& [nodeId, edges] : _adjacency) {
        edges.erase(
            std::remove_if(edges.begin(), edges.end(),
                           [id](const NavEdge& e) { return e.to == id; }),
            edges.end());
    }
}

void NavGraph::RemoveEdge(NavNodeId from, NavNodeId to)
{
    auto removeDirected = [this](NavNodeId src, NavNodeId dst) {
        auto it = _adjacency.find(src);
        if (it == _adjacency.end()) return;
        auto& edges = it->second;
        edges.erase(
            std::remove_if(edges.begin(), edges.end(),
                           [dst](const NavEdge& e) { return e.to == dst; }),
            edges.end());
    };
    removeDirected(from, to);
    removeDirected(to, from);
}

void NavGraph::SetBlocked(NavNodeId id, bool blocked)
{
    auto it = _nodes.find(id);
    if (it != _nodes.end()) {
        it->second.blocked = blocked;
    }
}

bool NavGraph::IsBlocked(NavNodeId id) const
{
    auto it = _nodes.find(id);
    if (it == _nodes.end()) return true;
    return it->second.blocked;
}

const NavNode* NavGraph::GetNode(NavNodeId id) const
{
    auto it = _nodes.find(id);
    if (it == _nodes.end()) return nullptr;
    return &it->second;
}

const std::vector<NavEdge>& NavGraph::GetEdges(NavNodeId id) const
{
    auto it = _adjacency.find(id);
    if (it == _adjacency.end()) return _emptyEdges;
    return it->second;
}

size_t NavGraph::NodeCount() const { return _nodes.size(); }

size_t NavGraph::EdgeCount() const
{
    size_t total = 0;
    for (const auto& [id, edges] : _adjacency) {
        total += edges.size();
    }
    return total;
}

void NavGraph::Clear()
{
    _nodes.clear();
    _adjacency.clear();
    _nextId = 1;
}

NavNodeId NavGraph::FindNearest(const Vector3& position) const
{
    NavNodeId bestId = InvalidNavNodeId;
    float bestDist = std::numeric_limits<float>::max();

    for (const auto& [id, node] : _nodes) {
        if (node.blocked) continue;
        float dist = (node.position - position).length();
        if (dist < bestDist) {
            bestDist = dist;
            bestId = id;
        }
    }
    return bestId;
}

void NavGraph::BuildGrid(const Vector3& origin, float spacing,
                          int countX, int countY, int countZ)
{
    Clear();

    if (countX <= 0 || countY <= 0 || countZ <= 0) return;

    // Store node ids in a 3D flat array for neighbor lookup.
    std::vector<NavNodeId> ids(
        static_cast<size_t>(countX) * static_cast<size_t>(countY) *
        static_cast<size_t>(countZ));

    auto idx = [&](int ix, int iy, int iz) -> size_t {
        return static_cast<size_t>(ix)
             + static_cast<size_t>(iy) * static_cast<size_t>(countX)
             + static_cast<size_t>(iz) * static_cast<size_t>(countX) * static_cast<size_t>(countY);
    };

    // Create nodes.
    for (int iz = 0; iz < countZ; ++iz) {
        for (int iy = 0; iy < countY; ++iy) {
            for (int ix = 0; ix < countX; ++ix) {
                Vector3 pos = origin + Vector3(
                    static_cast<float>(ix) * spacing,
                    static_cast<float>(iy) * spacing,
                    static_cast<float>(iz) * spacing);
                ids[idx(ix, iy, iz)] = AddNode(pos);
            }
        }
    }

    // Connect 6-axis neighbors.
    for (int iz = 0; iz < countZ; ++iz) {
        for (int iy = 0; iy < countY; ++iy) {
            for (int ix = 0; ix < countX; ++ix) {
                NavNodeId current = ids[idx(ix, iy, iz)];
                if (ix + 1 < countX)
                    AddEdge(current, ids[idx(ix + 1, iy, iz)]);
                if (iy + 1 < countY)
                    AddEdge(current, ids[idx(ix, iy + 1, iz)]);
                if (iz + 1 < countZ)
                    AddEdge(current, ids[idx(ix, iy, iz + 1)]);
            }
        }
    }
}

float NavGraph::CalculateEdgeWeight(NavNodeId from, NavNodeId to) const
{
    const NavNode* a = GetNode(from);
    const NavNode* b = GetNode(to);
    if (!a || !b) return std::numeric_limits<float>::max();

    float distance = (b->position - a->position).length();
    float avgCost = (a->cost + b->cost) * 0.5f;
    return distance * avgCost;
}

// ---------------------------------------------------------------------------
// Pathfinder
// ---------------------------------------------------------------------------

Pathfinder::Pathfinder(const NavGraph& graph)
    : _graph(graph)
    , _heuristic(EuclideanHeuristic)
{
}

NavPath Pathfinder::FindPath(NavNodeId startId, NavNodeId goalId) const
{
    _lastExplored = 0;
    NavPath result;

    const NavNode* startNode = _graph.GetNode(startId);
    const NavNode* goalNode  = _graph.GetNode(goalId);
    if (!startNode || !goalNode) return result;
    if (_graph.IsBlocked(startId) || _graph.IsBlocked(goalId)) return result;

    // Trivial case.
    if (startId == goalId) {
        result.waypoints.push_back(startNode->position);
        result.totalCost = 0.0f;
        result.valid = true;
        return result;
    }

    struct OpenEntry {
        NavNodeId id;
        float fCost;
        bool operator>(const OpenEntry& o) const { return fCost > o.fCost; }
    };

    std::priority_queue<OpenEntry, std::vector<OpenEntry>, std::greater<OpenEntry>> openSet;
    std::unordered_map<NavNodeId, float> gCost;
    std::unordered_map<NavNodeId, NavNodeId> cameFrom;
    std::unordered_set<NavNodeId> closedSet;

    gCost[startId] = 0.0f;
    openSet.push({startId, _heuristic(startNode->position, goalNode->position)});

    while (!openSet.empty()) {
        OpenEntry current = openSet.top();
        openSet.pop();

        if (current.id == goalId) {
            // Reconstruct path.
            std::vector<Vector3> waypoints;
            float totalCost = gCost[goalId];
            NavNodeId cur = goalId;
            while (cur != startId) {
                const NavNode* n = _graph.GetNode(cur);
                if (n) waypoints.push_back(n->position);
                cur = cameFrom[cur];
            }
            waypoints.push_back(startNode->position);
            std::reverse(waypoints.begin(), waypoints.end());

            result.waypoints = std::move(waypoints);
            result.totalCost = totalCost;
            result.valid = true;
            return result;
        }

        if (closedSet.count(current.id)) continue;
        closedSet.insert(current.id);
        _lastExplored++;

        for (const NavEdge& edge : _graph.GetEdges(current.id)) {
            if (closedSet.count(edge.to)) continue;
            if (_graph.IsBlocked(edge.to)) continue;

            float tentativeG = gCost[current.id] + edge.weight;

            auto it = gCost.find(edge.to);
            if (it == gCost.end() || tentativeG < it->second) {
                cameFrom[edge.to] = current.id;
                gCost[edge.to] = tentativeG;

                const NavNode* neighborNode = _graph.GetNode(edge.to);
                float h = neighborNode
                    ? _heuristic(neighborNode->position, goalNode->position)
                    : 0.0f;
                openSet.push({edge.to, tentativeG + h});
            }
        }
    }

    return result; // No path found.
}

NavPath Pathfinder::FindPathByPosition(const Vector3& start, const Vector3& goal) const
{
    NavNodeId startId = _graph.FindNearest(start);
    NavNodeId goalId  = _graph.FindNearest(goal);
    if (startId == InvalidNavNodeId || goalId == InvalidNavNodeId) return NavPath();
    return FindPath(startId, goalId);
}

float Pathfinder::EuclideanHeuristic(const Vector3& a, const Vector3& b)
{
    return (b - a).length();
}

float Pathfinder::ManhattanHeuristic(const Vector3& a, const Vector3& b)
{
    return std::fabs(b.x - a.x) + std::fabs(b.y - a.y) + std::fabs(b.z - a.z);
}

void Pathfinder::SetHeuristic(std::function<float(const Vector3&, const Vector3&)> h)
{
    _heuristic = std::move(h);
}

// ---------------------------------------------------------------------------
// SmoothPath
// ---------------------------------------------------------------------------

NavPath SmoothPath(const NavPath& raw, float tolerance)
{
    if (!raw.valid || raw.waypoints.size() <= 2) return raw;

    NavPath smoothed;
    smoothed.valid = raw.valid;
    smoothed.totalCost = raw.totalCost;

    std::vector<Vector3> points = raw.waypoints;

    bool removed = true;
    while (removed) {
        removed = false;
        std::vector<Vector3> next;
        next.push_back(points[0]);

        for (size_t i = 1; i + 1 < points.size(); ++i) {
            Vector3 a = points[i - 1];
            Vector3 b = points[i];
            Vector3 c = points[i + 1];

            Vector3 ab = b - a;
            Vector3 ac = c - a;

            // Cross product of ab and ac.
            Vector3 cross(
                ab.y * ac.z - ab.z * ac.y,
                ab.z * ac.x - ab.x * ac.z,
                ab.x * ac.y - ab.y * ac.x);

            float acLen = ac.length();
            float deviation = (acLen > 0.0f) ? cross.length() / acLen : 0.0f;

            if (deviation < tolerance) {
                removed = true; // Skip this collinear waypoint.
            } else {
                next.push_back(b);
            }
        }

        next.push_back(points.back());
        points = std::move(next);
    }

    smoothed.waypoints = std::move(points);
    return smoothed;
}

} // namespace subspace
