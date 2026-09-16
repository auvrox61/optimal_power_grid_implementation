#ifndef POWER_GRID_H
#define POWER_GRID_H

#include "types.h"
#include "merge_sort.h"
#include "dijkstra.h"
#include "edmonds_karp.h"
#include "greedy_scheduler.h"

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>

class PowerGrid {
public:
    std::vector<PowerPlant> plants;
    std::vector<ConsumerZone> zones;
    std::vector<TransmissionLine> lines;
    std::vector<std::string> nodeNames;

    int superSource;
    int superSink;
    int totalNodes;

    PowerGrid() = default;

    void addPlant(const PowerPlant& p) {
        plants.push_back(p);
    }

    void addZone(const ConsumerZone& z) {
        zones.push_back(z);
    }

    void addLine(const TransmissionLine& l) {
        lines.push_back(l);
    }

    void rankZonesByPriority() {
        std::cout << "\n[STEP 1] Ranking consumer zones by priority...\n";
        MergeSort::sortByPriority(zones);
        MergeSort::printRanking(zones);
    }

    void findMinLossPaths() {
        std::cout << "\n[STEP 2] Finding minimum-loss transmission paths...\n";

        int n = static_cast<int>(plants.size() + zones.size());
        DijkstraGraph dGraph(n);

        std::vector<std::string> names;

        for (const auto& p : plants)
            names.push_back(p.name);

        for (const auto& z : zones)
            names.push_back(z.name);

        for (const auto& l : lines) {
            if (l.from >= 0 && l.from < n &&
                l.to >= 0 && l.to < n) {
                dGraph.addEdge(
                    l.from,
                    l.to,
                    l.lossCoefficient,
                    l.capacityMW
                );
            }
        }

        int pCount = static_cast<int>(plants.size());
        std::vector<int> demandIds;

        for (int i = 0; i < static_cast<int>(zones.size()); i++)
            demandIds.push_back(pCount + i);

        for (int p = 0; p < pCount; p++)
            dGraph.printMinLossPaths(p, names, demandIds);
    }

    double computeMaxFlow() {
        std::cout << "\n[STEP 3] Computing maximum power flow...\n";

        int pCount = static_cast<int>(plants.size());
        int zCount = static_cast<int>(zones.size());

        totalNodes = pCount + zCount + 2;
        superSource = pCount + zCount;
        superSink = pCount + zCount + 1;

        EdmondsKarp ek(totalNodes);

        for (int i = 0; i < pCount; i++) {
            ek.addEdge(
                superSource,
                i,
                plants[i].availableOutputMW
            );
        }

        for (const auto& l : lines) {
            if (l.from >= 0 && l.from < pCount + zCount &&
                l.to >= 0 && l.to < pCount + zCount) {

                double effectiveCap =
                    l.capacityMW * (1.0 - l.lossCoefficient);

                ek.addEdge(
                    l.from,
                    l.to,
                    effectiveCap
                );
            }
        }

        for (int j = 0; j < zCount; j++) {
            ek.addEdge(
                pCount + j,
                superSink,
                zones[j].peakDemandMW
            );
        }

        double flow = ek.maxFlow(superSource, superSink);

        ek.printResult(
            flow,
            "All Generation Sources",
            "All Consumer Zones"
        );

        return flow;
    }

    void allocatePower(double availableMW) {
        std::cout << "\n[STEP 4] Allocating power by priority...\n";

        double remaining = std::max(0.0, availableMW);

        for (auto& z : zones) {
            z.allocatedMW = 0.0;

            if (remaining <= 0.0)
                continue;

            double allocation = 0.0;

            if (z.priorityTier == PriorityTier::TIER1) {
                allocation = std::min(
                    z.peakDemandMW,
                    remaining
                );
            } else {
                allocation = std::min(
                    z.peakDemandMW,
                    remaining
                );

                if (allocation < z.minGuaranteedSupplyMW) {
                    allocation = std::min(
                        z.minGuaranteedSupplyMW,
                        remaining
                    );
                }
            }

            z.allocatedMW = allocation;
            remaining -= allocation;
        }

        printAllocationTable(availableMW);
    }

    void scheduleLoadShedding() {
        std::cout << "\n[STEP 5] Scheduling load-shedding windows...\n";

        std::vector<ConsumerZone> tier3;
        double deficitMW = 0.0;

        for (const auto& z : zones) {
            if (z.priorityTier == PriorityTier::TIER3) {
                tier3.push_back(z);

                double deficit =
                    z.peakDemandMW - z.allocatedMW;

                if (deficit > 0.0)
                    deficitMW += deficit;
            }
        }

        if (deficitMW < 1e-9) {
            std::cout << "  No load shedding required!\n";
            return;
        }

        std::cout << "  Total deficit to shed: "
                  << std::fixed << std::setprecision(2)
                  << deficitMW << " MW\n";

        std::vector<std::pair<double, double>> slots = {
            {6.0, 8.0},
            {8.0, 10.0},
            {10.0, 12.0},
            {12.0, 14.0},
            {14.0, 16.0},
            {16.0, 18.0},
            {18.0, 20.0},
            {20.0, 22.0}
        };

        auto schedule = GreedyScheduler::fairSchedule(
            tier3,
            slots,
            deficitMW
        );

        GreedyScheduler::printSchedule(schedule);
    }

    void run() {
        printHeader();
        rankZonesByPriority();
        findMinLossPaths();
        double maxFlow = computeMaxFlow();
        allocatePower(maxFlow);
        scheduleLoadShedding();
        printSummary(maxFlow);
    }

private:
    void printHeader() const {
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════════╗\n";
        std::cout << "║  OPTIMAL POWER GRID LOAD DISTRIBUTION SYSTEM ║\n";
        std::cout << "║  Bangladesh National Power Grid Optimizer     ║\n";
        std::cout << "╚══════════════════════════════════════════════╝\n";
    }

    void printAllocationTable(double totalAvailable) const {
        double totalDemand = 0.0;
        double totalAllocated = 0.0;
        double totalMin = 0.0;

        std::cout << "\n  Power Allocation Results:\n";
        std::cout << "  " << std::string(88, '-') << "\n";

        std::cout << "  " << std::left
                  << std::setw(25) << "Zone"
                  << std::setw(20) << "Tier"
                  << std::setw(14) << "Demand(MW)"
                  << std::setw(14) << "Min(MW)"
                  << std::setw(14) << "Allocated(MW)"
                  << std::setw(10) << "Status"
                  << "\n";

        std::cout << "  " << std::string(88, '-') << "\n";

        for (const auto& z : zones) {
            std::string status;

            if (z.allocatedMW >= z.peakDemandMW)
                status = "FULL";
            else if (z.allocatedMW >= z.minGuaranteedSupplyMW)
                status = "PARTIAL";
            else
                status = "DEFICIT";

            std::cout << "  " << std::left
                      << std::setw(25) << z.name
                      << std::setw(20) << tierToString(z.priorityTier)
                      << std::setw(14) << z.peakDemandMW
                      << std::setw(14) << z.minGuaranteedSupplyMW
                      << std::setw(14) << std::fixed
                      << std::setprecision(2)
                      << z.allocatedMW
                      << std::setw(10) << status
                      << "\n";

            totalDemand += z.peakDemandMW;
            totalAllocated += z.allocatedMW;
            totalMin += z.minGuaranteedSupplyMW;
        }

        std::cout << "  " << std::string(88, '-') << "\n";

        std::cout << "  " << std::left
                  << std::setw(25) << "TOTAL"
                  << std::setw(20) << ""
                  << std::setw(14) << totalDemand
                  << std::setw(14) << totalMin
                  << std::setw(14) << totalAllocated
                  << "\n";

        std::cout << "\n  Available Supply : "
                  << totalAvailable << " MW\n";

        std::cout << "  Total Demand     : "
                  << totalDemand << " MW\n";

        std::cout << "  Total Allocated  : "
                  << totalAllocated << " MW\n";

        std::cout << "  Supply Gap       : "
                  << std::max(
                         0.0,
                         totalDemand - totalAllocated
                     )
                  << " MW\n";
    }

    void printSummary(double maxFlow) const {
        double totalDemand = 0.0;
        double totalGen = 0.0;

        for (const auto& z : zones)
            totalDemand += z.peakDemandMW;

        for (const auto& p : plants)
            totalGen += p.availableOutputMW;

        std::cout << "\n╔══════════════════════════════════════════════╗\n";
        std::cout << "║                SYSTEM SUMMARY                ║\n";
        std::cout << "╠══════════════════════════════════════════════╣\n";

        std::cout << "║  Total Generation Capacity : "
                  << std::setw(8)
                  << std::fixed
                  << std::setprecision(1)
                  << totalGen
                  << " MW         ║\n";

        std::cout << "║  Maximum Deliverable Flow  : "
                  << std::setw(8)
                  << maxFlow
                  << " MW         ║\n";

        std::cout << "║  Total Consumer Demand     : "
                  << std::setw(8)
                  << totalDemand
                  << " MW         ║\n";

        std::cout << "║  Deficit                   : "
                  << std::setw(8)
                  << std::max(
                         0.0,
                         totalDemand - maxFlow
                     )
                  << " MW         ║\n";

        std::cout << "╚══════════════════════════════════════════════╝\n\n";
    }
};

#endif