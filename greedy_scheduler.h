#ifndef GREEDY_SCHEDULER_H
#define GREEDY_SCHEDULER_H

#include "types.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>

class GreedyScheduler {
public:
    static std::vector<LoadSheddingWindow>
    schedule(std::vector<LoadSheddingWindow>& windows) {
        if (windows.empty()) return {};

        std::sort(windows.begin(), windows.end(),
                  [](const LoadSheddingWindow& a,
                     const LoadSheddingWindow& b) {
                      return a.endHour < b.endHour;
                  });

        std::vector<LoadSheddingWindow> selected;
        double lastEnd = -1.0;

        for (const auto& w : windows) {
            if (w.startHour >= lastEnd) {
                selected.push_back(w);
                lastEnd = w.endHour;
            }
        }

        return selected;
    }

    static std::vector<LoadSheddingWindow>
    fairSchedule(
        const std::vector<ConsumerZone>& tier3Zones,
        const std::vector<std::pair<double,double>>& timeSlots,
        double deficitMW) {

        std::vector<LoadSheddingWindow> candidates;

        if (tier3Zones.empty() || timeSlots.empty())
            return candidates;

        double remainingDeficit = deficitMW;
        int zoneIdx  = 0;
        int numZones = static_cast<int>(tier3Zones.size());

        for (const auto& slot : timeSlots) {
            if (remainingDeficit <= 1e-9)
                break;

            const ConsumerZone& zone =
                tier3Zones[zoneIdx % numZones];

            double shedable =
                zone.peakDemandMW - zone.minGuaranteedSupplyMW;

            double toShed =
                std::min(shedable, remainingDeficit);

            if (toShed > 1e-9) {
                candidates.emplace_back(
                    zone.id,
                    zone.name,
                    slot.first,
                    slot.second,
                    toShed
                );

                remainingDeficit -= toShed;
            }

            zoneIdx++;
        }

        return schedule(candidates);
    }

    static void printSchedule(
            const std::vector<LoadSheddingWindow>& windows) {

        std::cout << "\n========================================\n";
        std::cout << "  LOAD-SHEDDING SCHEDULE (Greedy)\n";
        std::cout << "========================================\n";

        if (windows.empty()) {
            std::cout << "  No load shedding required.\n";
            std::cout << "========================================\n";
            return;
        }

        std::cout << std::left
                  << std::setw(28) << "Zone"
                  << std::setw(12) << "Start(h)"
                  << std::setw(12) << "End(h)"
                  << std::setw(12) << "Dur(h)"
                  << std::setw(12) << "Shed(MW)"
                  << "\n";

        std::cout << std::string(76, '-') << "\n";

        double totalShed = 0.0;
        double totalDur = 0.0;

        for (const auto& w : windows) {
            std::cout << std::left
                      << std::setw(28) << w.zoneName
                      << std::setw(12) << std::fixed
                      << std::setprecision(1) << w.startHour
                      << std::setw(12) << w.endHour
                      << std::setw(12) << w.duration()
                      << std::setw(12) << w.loadShedMW
                      << "\n";

            totalShed += w.loadShedMW;
            totalDur  += w.duration();
        }

        std::cout << std::string(76, '-') << "\n";

        std::cout << std::left
                  << std::setw(28) << "TOTAL"
                  << std::setw(12) << ""
                  << std::setw(12) << ""
                  << std::setw(12) << totalDur
                  << std::setw(12) << totalShed
                  << "\n";

        std::cout << "========================================\n";
    }
};

#endif