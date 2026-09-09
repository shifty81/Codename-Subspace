#pragma once

#include "celestial/SectorResourceModel.h"
#include "mining/MiningSalvageModel.h"

#include <string>
#include <vector>

namespace subspace {

struct CommodityPrice {
    std::string commodity;
    int basePrice = 10;
    float multiplier = 1.0f;
};

struct StationServiceProfile {
    std::string stationId = "dev-station";
    std::vector<CommodityPrice> commodityPrices;
    int repairPricePerHull = 3;
    int refuelPrice = 20;
    std::vector<std::string> services;
};

struct TradeQuote {
    int totalCredits = 0;
    std::string summary;
};

StationServiceProfile BuildStationEconomyFromSurvey(const SectorResourceSurvey& survey);
TradeQuote QuoteCargoSale(const StationServiceProfile& station, const std::vector<CargoYieldItem>& cargo);
int QuoteRepairCost(const StationServiceProfile& station, float currentHull, float maxHull = 100.0f);
std::string StationEconomySummary(const StationServiceProfile& station);

} // namespace subspace
