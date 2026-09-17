#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#define MERGE_SORT_H

#include "types.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>

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
                  << std::setw(6)  << "Rank"
                  << std::setw(30) << "Zone Name"
                  << std::setw(28) << "Priority Tier"
                  << std::setw(12) << "Peak (MW)"
                  << std::setw(16) << "Min Supply (MW)"
                  << "\n";
        std::cout << std::string(92, '-') << "\n";

        int rank = 1;
        for (const auto& z : zones) {
            std::cout << std::left
                      << std::setw(6)  << rank++
                      << std::setw(30) << z.name
                      << std::setw(28) << tierToString(z.priorityTier)
                      << std::setw(12) << z.peakDemandMW
                      << std::setw(16) << z.minGuaranteedSupplyMW
                      << "\n";
        }
        std::cout << "========================================\n";
    }

private:
    static bool hasHigherPriority(const ConsumerZone& a,
                                   const ConsumerZone& b) {
        if (static_cast<int>(a.priorityTier) !=
            static_cast<int>(b.priorityTier)) {
            return static_cast<int>(a.priorityTier) <
                   static_cast<int>(b.priorityTier);
        }
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
            if (hasHigherPriority(leftArr[i], rightArr[j]))
                zones[k++] = leftArr[i++];
            else
                zones[k++] = rightArr[j++];
        }

        while (i < (int)leftArr.size())
            zones[k++] = leftArr[i++];

        while (j < (int)rightArr.size())
            zones[k++] = rightArr[j++];
    }
};

#endif