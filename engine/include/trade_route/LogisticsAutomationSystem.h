#pragma once
#include <string>
#include <unordered_map>
#include <vector>
namespace subspace {
struct AutomatedLogisticsNode { std::string id; std::unordered_map<std::string,double> inventory; std::unordered_map<std::string,double> minimum; std::unordered_map<std::string,double> target; double throughputPerHour=100; };
struct AutomatedRoute { std::string id; std::string from; std::string to; std::string commodity; double maxPerRun=50; double priority=1; bool enabled=true; double movedTotal=0; };
class LogisticsAutomationSystem {
public:
 bool RegisterNode(const AutomatedLogisticsNode& node);
 bool RegisterRoute(const AutomatedRoute& route);
 double AdvanceRoute(const std::string& routeId,double hours);
 const AutomatedLogisticsNode* Node(const std::string& id) const;
 const AutomatedRoute* Route(const std::string& id) const;
private:std::unordered_map<std::string,AutomatedLogisticsNode> nodes_;std::unordered_map<std::string,AutomatedRoute> routes_;
};
}
