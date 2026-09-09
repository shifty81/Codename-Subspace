#include "cargo/CargoSystem.h"

#include <algorithm>

namespace subspace {

CargoHold::CargoHold(double maxMass, double maxVolume)
    : maxMass_(std::max(0.0, maxMass)), maxVolume_(std::max(0.0, maxVolume)) {}

bool CargoHold::CanAdd(const CargoStack& item, std::uint64_t quantity) const {
    if (quantity == 0 || item.commodityId.empty() || item.unitMass < 0.0 || item.unitVolume < 0.0) return false;
    const double nextMass = GetUsedMass() + item.unitMass * static_cast<double>(quantity);
    const double nextVolume = GetUsedVolume() + item.unitVolume * static_cast<double>(quantity);
    return nextMass <= maxMass_ + 1e-9 && nextVolume <= maxVolume_ + 1e-9;
}

bool CargoHold::Add(const CargoStack& item, std::uint64_t quantity) {
    if (!CanAdd(item, quantity)) return false;
    auto it = stacks_.find(item.commodityId);
    if (it == stacks_.end()) {
        CargoStack copy = item;
        copy.quantity = quantity;
        stacks_.emplace(copy.commodityId, std::move(copy));
    } else {
        if (it->second.unitMass != item.unitMass || it->second.unitVolume != item.unitVolume) return false;
        it->second.quantity += quantity;
    }
    return true;
}

bool CargoHold::Remove(const std::string& commodityId, std::uint64_t quantity) {
    auto it = stacks_.find(commodityId);
    if (it == stacks_.end() || quantity == 0 || it->second.quantity < quantity) return false;
    it->second.quantity -= quantity;
    if (it->second.quantity == 0) stacks_.erase(it);
    return true;
}

bool CargoHold::TransferTo(CargoHold& destination, const std::string& commodityId, std::uint64_t quantity) {
    auto it = stacks_.find(commodityId);
    if (it == stacks_.end() || quantity == 0 || it->second.quantity < quantity) return false;
    CargoStack copy = it->second;
    if (!destination.Add(copy, quantity)) return false;
    return Remove(commodityId, quantity);
}

std::uint64_t CargoHold::GetQuantity(const std::string& commodityId) const {
    auto it = stacks_.find(commodityId);
    return it == stacks_.end() ? 0 : it->second.quantity;
}

const CargoStack* CargoHold::Get(const std::string& commodityId) const {
    auto it = stacks_.find(commodityId);
    return it == stacks_.end() ? nullptr : &it->second;
}

std::vector<CargoStack> CargoHold::GetStacks() const {
    std::vector<CargoStack> out;
    out.reserve(stacks_.size());
    for (const auto& pair : stacks_) out.push_back(pair.second);
    std::sort(out.begin(), out.end(), [](const CargoStack& a, const CargoStack& b) { return a.commodityId < b.commodityId; });
    return out;
}

double CargoHold::GetUsedMass() const {
    double total = 0.0;
    for (const auto& pair : stacks_) total += pair.second.unitMass * static_cast<double>(pair.second.quantity);
    return total;
}

double CargoHold::GetUsedVolume() const {
    double total = 0.0;
    for (const auto& pair : stacks_) total += pair.second.unitVolume * static_cast<double>(pair.second.quantity);
    return total;
}

double CargoHold::GetMassUtilization() const { return maxMass_ > 0.0 ? GetUsedMass() / maxMass_ : 0.0; }
double CargoHold::GetVolumeUtilization() const { return maxVolume_ > 0.0 ? GetUsedVolume() / maxVolume_ : 0.0; }
bool CargoHold::IsOverloaded() const { return GetUsedMass() > maxMass_ + 1e-9 || GetUsedVolume() > maxVolume_ + 1e-9; }

void CargoHold::SetCapacity(double maxMass, double maxVolume) {
    maxMass_ = std::max(0.0, maxMass);
    maxVolume_ = std::max(0.0, maxVolume);
}

void CargoHold::Clear() { stacks_.clear(); }

} // namespace subspace
