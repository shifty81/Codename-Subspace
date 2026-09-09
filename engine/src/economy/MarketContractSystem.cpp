#include "economy/MarketContractSystem.h"

#include <algorithm>

namespace subspace {
std::uint64_t MarketContractSystem::PlaceOrder(MarketOrderSide s,const std::string&c,double p,std::uint64_t q,const std::string&o){if(c.empty()||p<=0||q==0)return 0;auto id=nextOrder_++;orders_.push_back({id,s,c,p,q,o});return id;}
std::vector<MarketFill> MarketContractSystem::Match(const std::string&c){std::vector<MarketFill>fills;for(auto&b:orders_){if(b.side!=MarketOrderSide::Buy||b.commodity!=c||b.quantity==0)continue;for(auto&s:orders_){if(s.side!=MarketOrderSide::Sell||s.commodity!=c||s.quantity==0||s.owner==b.owner||s.price>b.price)continue;auto q=std::min(b.quantity,s.quantity);if(!q)continue;double px=(b.price+s.price)*.5;b.quantity-=q;s.quantity-=q;fills.push_back({b.id,s.id,q,px});if(!b.quantity)break;}}return fills;}
std::uint64_t MarketContractSystem::AddContract(const SandboxContract&c){auto copy=c;copy.id=nextContract_++;contracts_[copy.id]=copy;return copy.id;}
std::vector<SandboxContract> MarketContractSystem::AvailableContracts(double standing)const{std::vector<SandboxContract>out;for(const auto&kv:contracts_)if(!kv.second.completed&&standing>=kv.second.minimumStanding)out.push_back(kv.second);return out;}
bool MarketContractSystem::CompleteContract(std::uint64_t id){auto it=contracts_.find(id);if(it==contracts_.end()||it->second.completed)return false;it->second.completed=true;return true;}
} // namespace subspace
