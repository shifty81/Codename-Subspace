#include "ships/HazardCertificationSystem.h"

#include <algorithm>
namespace subspace {
HazardCertificationReport HazardCertificationSystem::Certify(const std::vector<HazardProtectionModule>&s,const std::vector<HazardProtectionModule>&f,const std::vector<HazardRegionRequirement>&req)const{HazardCertificationReport r;for(const auto&m:s)r.ratings[static_cast<int>(m.hazard)]=std::max(r.ratings[static_cast<int>(m.hazard)],static_cast<int>(m.rating*m.coverage));for(const auto&m:f)r.ratings[static_cast<int>(m.hazard)]=std::max(r.ratings[static_cast<int>(m.hazard)],std::max(0,static_cast<int>(m.rating*m.coverage*.75)));for(const auto&q:req)if(r.ratings[static_cast<int>(q.hazard)]<q.minimumRating){r.suitable=false;r.deficiencies.push_back("hazard "+std::to_string(static_cast<int>(q.hazard))+" requires rating "+std::to_string(q.minimumRating));}return r;}
} // namespace subspace
