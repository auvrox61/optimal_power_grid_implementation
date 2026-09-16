#ifndef EDMONDS_KARP_H
#define EDMONDS_KARP_H

#include "types.h"
#include <vector>
#include <queue>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>

class EdmondsKarp {
public:
    int numNodes;
    std::vector<std::vector<double>> cap;
    std::vector<std::vector<int>> graph;

    explicit EdmondsKarp(int n)
        : numNodes(n),
          cap(n, std::vector<double>(n, 0.0)),
          graph(n) {}

    void addEdge(int u, int v, double capacity) {
        cap[u][v] += capacity;
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    bool bfs(int source, int sink,
             std::vector<int>& parent) const {
        std::vector<bool> visited(numNodes, false);
        std::queue<int> q;

        q.push(source);
        visited[source] = true;
        parent[source] = -1;

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            for (int v : graph[u]) {
                if (!visited[v] && cap[u][v] > 1e-9) {
                    parent[v] = u;
                    visited[v] = true;

                    if (v == sink)
                        return true;

                    q.push(v);
                }
            }
        }

        return false;
    }

    double maxFlow(int source, int sink) {
        double totalFlow = 0.0;
        std::vector<int> parent(numNodes, -1);

        while (bfs(source, sink, parent)) {
            double pathFlow = INF;

            for (int v = sink; v != source; v = parent[v]) {
                int u = parent[v];
                pathFlow = std::min(pathFlow, cap[u][v]);
            }

            for (int v = sink; v != source; v = parent[v]) {
                int u = parent[v];
                cap[u][v] -= pathFlow;
                cap[v][u] += pathFlow;
            }

            totalFlow += pathFlow;
        }

        return totalFlow;
    }

    void printResult(double flow,
                     const std::string& sourceName,
                     const std::string& sinkName) const {
        std::cout << "\n========================================\n";
        std::cout << "  MAXIMUM POWER FLOW (Edmonds-Karp)\n";
        std::cout << "========================================\n";
        std::cout << "  Source : " << sourceName << "\n";
        std::cout << "  Sink   : " << sinkName << "\n";
        std::cout << "  Max Flow: "
                  << std::fixed << std::setprecision(2)
                  << flow << " MW\n";
        std::cout << "========================================\n";
    }
};

#endif