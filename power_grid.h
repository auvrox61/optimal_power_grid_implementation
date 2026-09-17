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
    std::vector<PowerPlant>       plants;
    std::vector<ConsumerZone>     zones;
    std::vector<TransmissionLine> lines;

    int superSource = -1;
    int superSink   = -1;
    int totalNodes  = 0;

    PowerGrid() = default;

    void addPlant(const PowerPlant& p)      { plants.push_back(p); }
    void addZone (const ConsumerZone& z)    { zones.push_back(z);  }
    void addLine (const TransmissionLine& l){ lines.push_back(l);  }

    void rankZonesByPriority() {
        std::cout << "\n[STEP 1] Ranking consumer zones by priority "
                     "(Merge Sort)...\n";
        MergeSort::sortByPriority(zones);
        MergeSort::printRanking(zones);
    }

    void findMinLossPaths() {
        std::cout << "\n[STEP 2] Finding minimum-loss transmission "
                     "paths (Dijkstra)...\n";

        int pCount = static_cast<int>(plants.size());
        int zCount = static_cast<int>(zones.size());
        int n      = pCount + zCount;

        DijkstraGraph dGraph(n);

        std::vector<std::string> names;
        names.reserve(n);

        for (const auto& p : plants)
            names.push_back(p.name);

        for (const auto& z : zones)
            names.push_back(z.name);

        for (const auto& l : lines) {
            if (l.from < n && l.to < n)
                dGraph.addEdge(l.from, l.to,
                               l.lossCoefficient,
                               l.capacityMW);
        }

        std::vector<int> demandIds;

        for (int i = 0; i < zCount; i++)
            demandIds.push_back(pCount + i);

        for (int p = 0; p < pCount; p++)
            dGraph.printMinLossPaths(p, names, demandIds);
    }

    double computeMaxFlow() {
        std::cout << "\n[STEP 3] Computing maximum power flow "
                     "(Edmonds-Karp)...\n";

        int pCount  = static_cast<int>(plants.size());
        int zCount  = static_cast<int>(zones.size());

        totalNodes  = pCount + zCount + 2;
        superSource = pCount + zCount;
        superSink   = pCount + zCount + 1;

        EdmondsKarp ek(totalNodes);

        for (int i = 0; i < pCount; i++)
            ek.addEdge(superSource, i,
                       plants[i].availableOutputMW);

        for (const auto& l : lines) {
            double effectiveCap =
                l.capacityMW * (1.0 - l.lossCoefficient);

            ek.addEdge(l.from, l.to, effectiveCap);
        }

        for (int j = 0; j < zCount; j++)
            ek.addEdge(pCount + j, superSink,
                       zones[j].peakDemandMW);

        double flow = ek.maxFlow(superSource, superSink);

        ek.printResult(flow,
                       "All Generation Sources",
                       "All Consumer Zones");

        return flow;
    }

    void allocatePower(double availableMW) {
        std::cout << "\n[STEP 4] Allocating power by priority...\n";

        double remaining = availableMW;

        for (auto& z : zones) {
            if (remaining <= 1e-9) {
                z.allocatedMW = 0.0;
                continue;
            }

            if (z.priorityTier == PriorityTier::TIER1) {
                z.allocatedMW = z.peakDemandMW;
            } else {
                double give =
                    std::min(z.peakDemandMW, remaining);

                give = std::max(
                    give,
                    std::min(z.minGuaranteedSupplyMW,
                             remaining)
                );

                z.allocatedMW = give;
            }

            remaining -= z.allocatedMW;
        }

        printAllocationTable(availableMW);
    }

    void scheduleLoadShedding() {
        std::cout << "\n[STEP 5] Scheduling load-shedding windows "
                     "(Greedy Interval)...\n";

        std::vector<ConsumerZone> tier3;
        double deficitMW = 0.0;

        for (const auto& z : zones) {
            if (z.priorityTier == PriorityTier::TIER3) {
                tier3.push_back(z);

                double deficit =
                    z.peakDemandMW - z.allocatedMW;

                if (deficit > 1e-9)
                    deficitMW += deficit;
            }
        }

        if (deficitMW < 1e-9) {
            std::cout << "  No load shedding required!\n";
            return;
        }

        std::cout << "  Total power deficit to shed: "
                  << std::fixed << std::setprecision(2)
                  << deficitMW << " MW\n";

        std::vector<std::pair<double,double>> slots = {
            { 6.0,  8.0},
            { 8.0, 10.0},
            {10.0, 12.0},
            {12.0, 14.0},
            {14.0, 16.0},
            {16.0, 18.0},
            {18.0, 20.0},
            {20.0, 22.0}
        };

        auto sched = GreedyScheduler::fairSchedule(
                         tier3, slots, deficitMW);

        GreedyScheduler::printSchedule(sched);
    }

    void run() {
        printHeader();
        rankZonesByPriority();
        findMinLossPaths();
        double mf = computeMaxFlow();
        allocatePower(mf);
        scheduleLoadShedding();
        printSummary(mf);
    }

private:
    void printHeader() const {
        std::cout << "\n";
        std::cout << "================================================\n";
        std::cout << "  OPTIMAL POWER GRID LOAD DISTRIBUTION SYSTEM\n";
        std::cout << "  Bangladesh National Power Grid Optimizer\n";
        std::cout << "  CSE 4403 | Algorithms | Quiz 3\n";
        std::cout << "================================================\n";
    }

    void printAllocationTable(double totalAvail) const {
        double totDemand = 0;
        double totAlloc  = 0;
        double totMin    = 0;

        std::cout << "\n  Allocation Results:\n";
        std::cout << "  " << std::string(95, '-') << "\n";

        std::cout << "  " << std::left
                  << std::setw(28) << "Zone"
                  << std::setw(26) << "Tier"
                  << std::setw(12) << "Demand(MW)"
                  << std::setw(12) << "Min(MW)"
                  << std::setw(14) << "Allocated(MW)"
                  << std::setw(10) << "Status"
                  << "\n";

        std::cout << "  " << std::string(95, '-') << "\n";

        for (const auto& z : zones) {
            std::string status;

            if (z.allocatedMW >= z.peakDemandMW)
                status = "FULL";
            else if (z.allocatedMW >= z.minGuaranteedSupplyMW)
                status = "PARTIAL";
            else
                status = "DEFICIT";

            std::cout << "  " << std::left
                      << std::setw(28) << z.name
                      << std::setw(26) << tierToString(z.priorityTier)
                      << std::setw(12) << z.peakDemandMW
                      << std::setw(12) << z.minGuaranteedSupplyMW
                      << std::setw(14) << std::fixed
                      << std::setprecision(2) << z.allocatedMW
                      << std::setw(10) << status
                      << "\n";

            totDemand += z.peakDemandMW;
            totAlloc  += z.allocatedMW;
            totMin    += z.minGuaranteedSupplyMW;
        }

        std::cout << "  " << std::string(95, '-') << "\n";

        std::cout << "  " << std::left
                  << std::setw(28) << "TOTALS"
                  << std::setw(26) << ""
                  << std::setw(12) << totDemand
                  << std::setw(12) << totMin
                  << std::setw(14) << totAlloc
                  << "\n\n";

        std::cout << "  Available Supply  : "
                  << std::fixed << std::setprecision(2)
                  << totalAvail << " MW\n";

        std::cout << "  Total Demand      : "
                  << totDemand << " MW\n";

        std::cout << "  Total Allocated   : "
                  << totAlloc << " MW\n";

        std::cout << "  Remaining Deficit : "
                  << std::max(0.0, totDemand - totAlloc)
                  << " MW\n";
    }

    void printSummary(double maxFlow) const {
        double totalGen    = 0.0;
        double totalDemand = 0.0;

        for (const auto& p : plants)
            totalGen += p.availableOutputMW;

        for (const auto& z : zones)
            totalDemand += z.peakDemandMW;

        std::cout << "\n================================================\n";
        std::cout << "                  SYSTEM SUMMARY\n";
        std::cout << "================================================\n";

        std::cout << std::fixed << std::setprecision(2);

        std::cout << "  Total Available Generation : "
                  << totalGen << " MW\n";

        std::cout << "  Max Deliverable Flow       : "
                  << maxFlow << " MW\n";

        std::cout << "  Total Consumer Demand      : "
                  << totalDemand << " MW\n";

        std::cout << "  Overall Deficit            : "
                  << std::max(0.0, totalDemand - maxFlow)
                  << " MW\n";

        std::cout << "================================================\n\n";
    }
};

#endif