#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#include "types.h"
#include <vector>
#include <iostream>
#include <iomanip>

class MergeSort {
public:
    static void sortByPriority(std::vector<ConsumerZone>& zones) {
        if (zones.size() <= 1) return;
        mergeSort(zones, 0, static_cast<int>(zones.size()) - 1);
    }

    static void printRanking(const std::vector<ConsumerZone>& zones) {
        std::cout << "\n========================================\n";
        std::cout << "  CONSUMER PRIORITY RANKING (Merge Sort)\n";
        std::cout << "========================================\n";
        std::cout << std::left
                  << std::setw(5)  << "Rank"
                  << std::setw(25) << "Zone Name"
                  << std::setw(25) << "Priority Tier"
                  << std::setw(15) << "Peak (MW)"
                  << std::setw(15) << "Min Supply (MW)"
                  << "\n";
        std::cout << std::string(85, '-') << "\n";

        int rank = 1;
        for (const auto& z : zones) {
            std::cout << std::left
                      << std::setw(5)  << rank++
                      << std::setw(25) << z.name
                      << std::setw(25) << tierToString(z.priorityTier)
                      << std::setw(15) << z.peakDemandMW
                      << std::setw(15) << z.minGuaranteedSupplyMW
                      << "\n";
        }
        std::cout << "========================================\n";
    }

private:
    static bool hasHigherPriority(const ConsumerZone& a,
                                  const ConsumerZone& b) {
        if (static_cast<int>(a.priorityTier) != static_cast<int>(b.priorityTier))
            return static_cast<int>(a.priorityTier) <
                   static_cast<int>(b.priorityTier);

        return a.peakDemandMW > b.peakDemandMW;
    }

    static void mergeSort(std::vector<ConsumerZone>& zones,
                          int left, int right) {
        if (left >= right) return;

        int mid = left + (right - left) / 2;
        mergeSort(zones, left, mid);
        mergeSort(zones, mid + 1, right);
        merge(zones, left, mid, right);
    }

    static void merge(std::vector<ConsumerZone>& zones,
                      int left, int mid, int right) {
        std::vector<ConsumerZone> leftArr(zones.begin() + left,
                                          zones.begin() + mid + 1);
        std::vector<ConsumerZone> rightArr(zones.begin() + mid + 1,
                                          zones.begin() + right + 1);

        int i = 0, j = 0, k = left;

        while (i < (int)leftArr.size() && j < (int)rightArr.size()) {
            if (hasHigherPriority(leftArr[i], rightArr[j]) || 
               (!hasHigherPriority(rightArr[j], leftArr[i]) && true)) {
                // If leftArr[i] >= rightArr[j] in priority, prefer leftArr[i]
                if (hasHigherPriority(rightArr[j], leftArr[i])) {
                    zones[k++] = rightArr[j++];
                } else {
                    zones[k++] = leftArr[i++];
                }
            }
        }

        while (i < (int)leftArr.size())
            zones[k++] = leftArr[i++];

        while (j < (int)rightArr.size())
            zones[k++] = rightArr[j++];
    }
};

#endif