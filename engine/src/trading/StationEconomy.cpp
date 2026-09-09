#include "trading/StationEconomy.h"

#include <algorithm>
#include <map>
#include <sstream>

namespace subspace {
namespace {
int DefaultPrice(const std::string& commodity)
{
    if (commodity.find("rare") != std::string::npos) return 60;
    if (commodity == "fuel" || commodity == "hydrogen" || commodity == "helium") return 22;
    if (commodity == "salvage" || commodity == "surface-salvage") return 30;
    if (commodity == "water" || commodity == "organics" || commodity == "food") return 18;
    return 14;
}
}

StationServiceProfile BuildStationEconomyFromSurvey(const SectorResourceSurvey& survey)
{
    StationServiceProfile station;
    station.stationId = survey.sectorId + ":dev-station";
    station.services = {"repair", "trade", "refuel", "survey-data"};

    std::map<std::string, CommodityPrice> prices;
    for (const auto& resource : survey.resources) {
        CommodityPrice price;
        price.commodity = resource.tag;
        price.basePrice = DefaultPrice(resource.tag);
        price.multiplier = std::max(0.55f, 1.18f - static_cast<float>(resource.abundance) * 0.018f);
        prices[resource.tag] = price;
    }
    for (const auto& hint : survey.economyHints) {
        auto& price = prices[hint.commodity];
        price.commodity = hint.commodity;
        if (price.basePrice <= 0) {
            price.basePrice = DefaultPrice(hint.commodity);
        }
        price.multiplier = std::max(0.35f, hint.priceMultiplier * (1.0f + static_cast<float>(hint.demand - hint.supply) * 0.035f));
    }
    if (prices.empty()) {
        prices["ore"] = {"ore", 14, 1.0f};
        prices["salvage"] = {"salvage", 30, 1.0f};
    }
    for (const auto& kv : prices) {
        station.commodityPrices.push_back(kv.second);
    }
    return station;
}

TradeQuote QuoteCargoSale(const StationServiceProfile& station, const std::vector<CargoYieldItem>& cargo)
{
    TradeQuote quote;
    for (const auto& item : cargo) {
        auto price = std::find_if(station.commodityPrices.begin(), station.commodityPrices.end(), [&](const CommodityPrice& p) {
            return p.commodity == item.commodity;
        });
        if (price != station.commodityPrices.end()) {
            quote.totalCredits += static_cast<int>(static_cast<float>(price->basePrice * item.units) * price->multiplier);
        }
        else {
            quote.totalCredits += item.creditValue;
        }
    }
    std::ostringstream stream;
    stream << "sale=" << quote.totalCredits << " credits items=" << cargo.size();
    quote.summary = stream.str();
    return quote;
}

int QuoteRepairCost(const StationServiceProfile& station, float currentHull, float maxHull)
{
    const int missing = std::max(0, static_cast<int>(maxHull - currentHull));
    return missing * std::max(1, station.repairPricePerHull);
}

std::string StationEconomySummary(const StationServiceProfile& station)
{
    std::ostringstream stream;
    stream << station.stationId << " commodities=" << station.commodityPrices.size() << " services=";
    for (std::size_t i = 0; i < station.services.size(); ++i) {
        if (i > 0) stream << ",";
        stream << station.services[i];
    }
    return stream.str();
}

} // namespace subspace
