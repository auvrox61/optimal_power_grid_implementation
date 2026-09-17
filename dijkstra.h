#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#define DIJKSTRA_H

#include "types.h"
#include <vector>
#include <queue>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <utility>

struct DijkstraEdge {
    int    to;
    double loss;
    double capacityMW;
};

class DijkstraGraph {
public:
    int numNodes;
    std::vector<std::vector<DijkstraEdge>> adjList;

    explicit DijkstraGraph(int n)
        : numNodes(n), adjList(n) {}

    void addEdge(int from, int to, double loss, double capacity) {
        DijkstraEdge edge;
        edge.to         = to;
        edge.loss       = loss;
        edge.capacityMW = capacity;
        adjList[from].push_back(edge);
    }

    struct Result {
        std::vector<double> dist;
        std::vector<int>    parent;
    };

    Result run(int source) const {
        Result res;
        res.dist.assign(numNodes, INF);
        res.parent.assign(numNodes, -1);
        res.dist[source] = 0.0;

        typedef std::pair<double, int> pdi;
        std::priority_queue<pdi,
                            std::vector<pdi>,
                            std::greater<pdi> > pq;

        pq.push(std::make_pair(0.0, source));

        while (!pq.empty()) {
            pdi top = pq.top();
            pq.pop();

            double d = top.first;
            int    u = top.second;

            if (d > res.dist[u]) continue;

            for (int i = 0; i < (int)adjList[u].size(); i++) {
                const DijkstraEdge& e = adjList[u][i];

                double newDist = res.dist[u] + e.loss;

                if (newDist < res.dist[e.to]) {
                    res.dist[e.to]   = newDist;
                    res.parent[e.to] = u;
                    pq.push(std::make_pair(newDist, e.to));
                }
            }
        }

        return res;
    }

    std::vector<int> getPath(const Result& res, int target) const {
        std::vector<int> path;

        if (res.dist[target] == INF) return path;

        for (int v = target; v != -1; v = res.parent[v])
            path.push_back(v);

        std::reverse(path.begin(), path.end());
        return path;
    }

    void printMinLossPaths(
            int source,
            const std::vector<std::string>& nodeNames,
            const std::vector<int>& demandNodeIds) const {

        Result res = run(source);

        std::cout << "\n  [Dijkstra] Source: "
                  << nodeNames[source] << "\n";
        std::cout << "  "
                  << std::string(70, '-') << "\n";

        for (int i = 0; i < (int)demandNodeIds.size(); i++) {
            int target = demandNodeIds[i];

            std::cout << "  -> "
                      << std::left
                      << std::setw(28)
                      << nodeNames[target];

            if (res.dist[target] == INF) {
                std::cout << "[UNREACHABLE]\n";
                continue;
            }

            std::cout << "Loss: "
                      << std::fixed
                      << std::setprecision(2)
                      << res.dist[target] * 100.0
                      << "%   Path: ";

            std::vector<int> path = getPath(res, target);

            for (int j = 0; j < (int)path.size(); j++) {
                if (j > 0) std::cout << " -> ";
                std::cout << nodeNames[path[j]];
            }

            std::cout << "\n";
        }

        std::cout << "  "
                  << std::string(70, '-') << "\n";
    }
};

#endif