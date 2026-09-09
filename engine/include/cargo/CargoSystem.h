#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct CargoStack {
    std::string commodityId;
    std::string displayName;
    double unitMass = 0.0;
    double unitVolume = 0.0;
    std::uint64_t quantity = 0;
};

class CargoHold {
public:
    explicit CargoHold(double maxMass = 1000.0, double maxVolume = 1000.0);

    bool CanAdd(const CargoStack& item, std::uint64_t quantity) const;
    bool Add(const CargoStack& item, std::uint64_t quantity);
    bool Remove(const std::string& commodityId, std::uint64_t quantity);
    bool TransferTo(CargoHold& destination, const std::string& commodityId, std::uint64_t quantity);

    std::uint64_t GetQuantity(const std::string& commodityId) const;
    const CargoStack* Get(const std::string& commodityId) const;
    std::vector<CargoStack> GetStacks() const;

    double GetUsedMass() const;
    double GetUsedVolume() const;
    double GetMaxMass() const { return maxMass_; }
    double GetMaxVolume() const { return maxVolume_; }
    double GetMassUtilization() const;
    double GetVolumeUtilization() const;
    bool IsOverloaded() const;

    void SetCapacity(double maxMass, double maxVolume);
    void Clear();

private:
    double maxMass_;
    double maxVolume_;
    std::unordered_map<std::string, CargoStack> stacks_;
};

} // namespace subspace
