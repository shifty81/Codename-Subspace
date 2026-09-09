#include "navigation/VectorTravelSystem.h"

#include <algorithm>

namespace subspace {

bool VectorTravelSystem::Begin(VectorTravelSession& s,const WarpPlan& plan) const {
    if(!plan.valid)return false;
    s={};s.plan=plan;s.stage=VectorTravelStage::Aligning;s.status="ALIGNING";
    s.plannedSeconds=1.15+std::max(0.5,plan.chargeSeconds)+std::max(1.8,plan.cruiseSeconds)+1.85;
    s.remainingSeconds=s.plannedSeconds;
    return true;
}
void VectorTravelSystem::Update(VectorTravelSession& s,double dt) const {
    if(dt<=0||!InTransit(s))return;s.elapsedSeconds+=dt;s.stageSeconds+=dt;s.visualPhase+=dt;
    const double align=1.15, charge=std::max(0.5,s.plan.chargeSeconds), cruise=std::max(1.8,s.plan.cruiseSeconds), decel=1.85;
    s.remainingSeconds=std::max(0.0,s.plannedSeconds-s.elapsedSeconds);
    switch(s.stage){
        case VectorTravelStage::Aligning: s.stageProgress=std::clamp(s.stageSeconds/align,0.0,1.0);s.progress=s.stageProgress*0.1;if(s.stageSeconds>=align){s.stage=VectorTravelStage::Charging;s.stageSeconds=0;s.stageProgress=0;s.status="VECTOR CHARGE";}break;
        case VectorTravelStage::Charging: s.stageProgress=std::clamp(s.stageSeconds/charge,0.0,1.0);s.progress=0.1+s.stageProgress*0.15;if(s.stageSeconds>=charge){s.stage=VectorTravelStage::Cruise;s.stageSeconds=0;s.stageProgress=0;s.status="VECTOR SLIPSTREAM";}break;
        case VectorTravelStage::Cruise: s.stageProgress=std::clamp(s.stageSeconds/cruise,0.0,1.0);s.progress=0.25+s.stageProgress*0.65;if(s.stageSeconds>=cruise){s.stage=VectorTravelStage::Decelerating;s.stageSeconds=0;s.stageProgress=0;s.status="VECTOR EXIT";}break;
        case VectorTravelStage::Decelerating: s.stageProgress=std::clamp(s.stageSeconds/decel,0.0,1.0);s.progress=0.90+s.stageProgress*0.10;if(s.stageSeconds>=decel){s.stage=VectorTravelStage::Complete;s.progress=1.0;s.stageProgress=1.0;s.remainingSeconds=0.0;s.status="ARRIVED";}break;
        default: break;
    }
}
void VectorTravelSystem::Cancel(VectorTravelSession& s) const { if(InTransit(s)){s.stage=VectorTravelStage::Failed;s.status="VECTOR ABORTED";} }
bool VectorTravelSystem::InTransit(const VectorTravelSession& s) const { return s.stage==VectorTravelStage::Aligning||s.stage==VectorTravelStage::Charging||s.stage==VectorTravelStage::Cruise||s.stage==VectorTravelStage::Decelerating; }

} // namespace subspace
