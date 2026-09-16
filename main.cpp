#include "powerGrid.h"
#include <iostream>

int main() {
    PowerGrid grid;

    grid.addPlant({0, "Ashuganj Gas Plant", 600.0, 500.0, FuelType::GAS, 4.5, 15.0});
    grid.addPlant({1, "Barapukuria Coal", 250.0, 200.0, FuelType::COAL, 3.2, 45.0});
    grid.addPlant({2, "Kaptai Hydro", 230.0, 180.0, FuelType::OIL, 2.8, 10.0});
    grid.addPlant({3, "Rooppur Nuclear", 160.0, 160.0, FuelType::NUCLEAR, 2.0, 60.0});
    grid.addPlant({4, "Teknaf Solar Farm", 80.0, 60.0, FuelType::SOLAR, 1.5, 5.0});

    grid.addZone({5, "Dhaka Hospitals & ER", 120.0, 120.0, PriorityTier::TIER1});
    grid.addZone({6, "Chittagong Port & EPZ", 200.0, 200.0, PriorityTier::TIER1});
    grid.addZone({7, "Water Treatment Plants", 80.0, 80.0, PriorityTier::TIER1});
    grid.addZone({8, "Gazipur Industrial Zone", 300.0, 150.0, PriorityTier::TIER2});
    grid.addZone({9, "Comilla EPZ", 180.0, 90.0, PriorityTier::TIER2});
    grid.addZone({10, "Dhaka Residential North", 250.0, 80.0, PriorityTier::TIER3});
    grid.addZone({11, "Dhaka Residential South", 220.0, 70.0, PriorityTier::TIER3});
    grid.addZone({12, "Sylhet Commercial Zone", 160.0, 50.0, PriorityTier::TIER3});

    grid.addLine({0, 5, 200.0, 0.03});
    grid.addLine({0, 8, 300.0, 0.04});
    grid.addLine({0, 10, 250.0, 0.05});
    grid.addLine({0, 11, 200.0, 0.05});
    grid.addLine({1, 6, 200.0, 0.06});
    grid.addLine({1, 9, 180.0, 0.07});
    grid.addLine({2, 7, 80.0, 0.02});
    grid.addLine({2, 12, 160.0, 0.04});
    grid.addLine({3, 5, 120.0, 0.01});
    grid.addLine({3, 6, 200.0, 0.02});
    grid.addLine({4, 7, 60.0, 0.01});
    grid.addLine({4, 12, 60.0, 0.02});

    grid.run();

    return 0;
}