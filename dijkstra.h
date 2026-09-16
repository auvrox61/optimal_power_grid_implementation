#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "types.h"
#include <vector>
#include <queue>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <functional>
#include <utility>
#include <limits>

#ifndef INF
constexpr double INF = std::numeric_limits<double>::infinity();
#endif

struct DijkstraEdge {
    int to;
    double loss;
    double capacityMW;
};

class DijkstraGraph {
public:
    int numNodes;
    std::vector<std::vector<DijkstraEdge>> adjList;

    explicit DijkstraGraph(int n) : numNodes(n), adjList(n) {}

    void addEdge(int from, int to, double loss, double capacity) {
        adjList[from].push_back({to, loss, capacity});
    }

    struct Result {
        std::vector<double> dist;
        std::vector<int> parent;
    };

    Result run(int source) const {
        Result res;
        res.dist.assign(numNodes, INF);
        res.parent.assign(numNodes, -1);
        res.dist[source] = 0.0;

        using pdi = std::pair<double, int>;
        std::priority_queue<pdi, std::vector<pdi>, std::greater<pdi>> pq;

        pq.push({0.0, source});

        while (!pq.empty()) {
            pdi topPair = pq.top();
            double d = topPair.first;
            int u = topPair.second;
            pq.pop();

            if (d > res.dist[u])
                continue;

            for (const auto& e : adjList[u]) {
                double newDist = res.dist[u] + e.loss;

                if (newDist < res.dist[e.to]) {
                    res.dist[e.to] = newDist;
                    res.parent[e.to] = u;
                    pq.push({newDist, e.to});
                }
            }
        }

        return res;
    }

    std::vector<int> getPath(const Result& res, int target) const {
        std::vector<int> path;

        if (res.dist[target] == INF)
            return path;

        for (int v = target; v != -1; v = res.parent[v])
            path.push_back(v);

        std::reverse(path.begin(), path.end());

        return path;
    }

    void printMinLossPaths(
        int source,
        const std::vector<std::string>& nodeNames,
        const std::vector<int>& demandNodeIds
    ) const {
        Result res = run(source);

        std::cout << "\n========================================\n";
        std::cout << "  MINIMUM-LOSS PATHS (Dijkstra)\n";
        std::cout << "  Source: " << nodeNames[source] << "\n";
        std::cout << "========================================\n";

        for (int target : demandNodeIds) {
            std::cout << "  → " << std::left << std::setw(22)
                      << nodeNames[target];

            if (res.dist[target] == INF) {
                std::cout << "  [UNREACHABLE]\n";
                continue;
            }

            std::cout << "  Total Loss: "
                      << std::fixed << std::setprecision(4)
                      << res.dist[target] * 100.0
                      << "%  |  Path: ";

            auto path = getPath(res, target);

            for (int i = 0; i < static_cast<int>(path.size()); i++) {
                if (i)
                    std::cout << " → ";

                std::cout << nodeNames[path[i]];
            }

            std::cout << "\n";
        }

        std::cout << "========================================\n";
    }
};

#endif 