#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class MarketOrderSide { Buy, Sell };
enum class ContractCareer { Combat, Mining, Salvage, Industry, Logistics, Exploration, Survey, Defense };
struct MarketOrderNative { std::uint64_t id=0; MarketOrderSide side=MarketOrderSide::Sell; std::string commodity; double price=0; std::uint64_t quantity=0; std::string owner; };
struct MarketFill { std::uint64_t buyOrder=0; std::uint64_t sellOrder=0; std::uint64_t quantity=0; double price=0; };
struct SandboxContract { std::uint64_t id=0; ContractCareer career=ContractCareer::Logistics; std::string title; std::string faction; double minimumStanding=-1.0; double rewardCredits=0; bool completed=false; };

class MarketContractSystem {
public:
    std::uint64_t PlaceOrder(MarketOrderSide side,const std::string& commodity,double price,std::uint64_t quantity,const std::string& owner);
    std::vector<MarketFill> Match(const std::string& commodity);
    std::uint64_t AddContract(const SandboxContract& contract);
    std::vector<SandboxContract> AvailableContracts(double standing) const;
    bool CompleteContract(std::uint64_t id);
private:
    std::uint64_t nextOrder_=1,nextContract_=1;
    std::vector<MarketOrderNative> orders_;
    std::unordered_map<std::uint64_t,SandboxContract> contracts_;
};

} // namespace subspace
