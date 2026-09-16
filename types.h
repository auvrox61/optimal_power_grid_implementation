#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>
#include <limits>

const double INF = std::numeric_limits<double>::infinity();

enum class FuelType {
    GAS,
    COAL,
    OIL,
    SOLAR,
    NUCLEAR
};

std::string fuelTypeToString(FuelType f) {
    switch (f) {
        case FuelType::GAS:     return "Gas";
        case FuelType::COAL:    return "Coal";
        case FuelType::OIL:     return "Oil";
        case FuelType::SOLAR:   return "Solar";
        case FuelType::NUCLEAR: return "Nuclear";
        default:                return "Unknown";
    }
}

enum class PriorityTier {
    TIER1 = 1,
    TIER2 = 2,
    TIER3 = 3
};

std::string tierToString(PriorityTier t) {
    switch (t) {
        case PriorityTier::TIER1: return "Tier-1 (Critical)";
        case PriorityTier::TIER2: return "Tier-2 (Industrial/EPZ)";
        case PriorityTier::TIER3: return "Tier-3 (Residential/Commercial)";
        default:                  return "Unknown";
    }
}

struct PowerPlant {
    int         id;
    std::string name;
    double      maxOutputMW;
    double      availableOutputMW;
    FuelType    fuelType;
    double      operationalCostPerMW;
    double      rampUpTimeMinutes;

    PowerPlant(int id, const std::string& name,
               double maxOut, double availOut,
               FuelType fuel, double cost, double rampUp)
        : id(id), name(name), maxOutputMW(maxOut),
          availableOutputMW(availOut), fuelType(fuel),
          operationalCostPerMW(cost), rampUpTimeMinutes(rampUp) {}
};

struct ConsumerZone {
    int          id;
    std::string  name;
    double       peakDemandMW;
    double       minGuaranteedSupplyMW;
    PriorityTier priorityTier;
    double       allocatedMW;

    ConsumerZone(int id, const std::string& name,
                 double peakDemand, double minSupply,
                 PriorityTier tier)
        : id(id), name(name), peakDemandMW(peakDemand),
          minGuaranteedSupplyMW(minSupply), priorityTier(tier),
          allocatedMW(0.0) {}
};

struct TransmissionLine {
    int    from;
    int    to;
    double capacityMW;
    double lossCoefficient;

    TransmissionLine(int from, int to, double cap, double loss)
        : from(from), to(to), capacityMW(cap), lossCoefficient(loss) {}
};

struct LoadSheddingWindow {
    int          zoneId;
    std::string  zoneName;
    double       startHour;
    double       endHour;
    double       loadShedMW;

    LoadSheddingWindow(int zid, const std::string& zname,
                       double start, double end, double shed)
        : zoneId(zid), zoneName(zname),
          startHour(start), endHour(end), loadShedMW(shed) {}

    double duration() const { return endHour - startHour; }
};

#endif