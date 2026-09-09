#include "ships/ShipyardDesignLanguageSystem.h"
#include <algorithm>
#include <cmath>
namespace subspace {
const std::vector<ShipyardDesignLanguageReference>& ShipyardDesignLanguageSystem::References(){static const std::vector<ShipyardDesignLanguageReference> r={{"ScoutShip_156",7.4776f,33.8902f,15.2209f},{"BattleShip_157",13.7072f,32.9679f,10.2643f},{"CruiserShip_158",15.4893f,31.6708f,10.4426f}};return r;}
float ShipyardDesignLanguageSystem::Score(float w,float l,float h,bool cmd,bool drive,bool balanced){if(l<=.001f)return 0;const float wr=w/l,hr=h/l;float ratio=100.0f; // broad envelope learned from examples, not template matching.
    if(wr<.15f)ratio-=std::min(28.0f,(.15f-wr)*120.0f);if(wr>.62f)ratio-=std::min(28.0f,(wr-.62f)*90.0f);if(hr<.08f)ratio-=12.0f;if(hr>.58f)ratio-=std::min(32.0f,(hr-.58f)*85.0f);if(l<w*1.25f)ratio-=24.0f;if(l<h*1.30f)ratio-=30.0f;if(!cmd)ratio-=22.0f;if(!drive)ratio-=34.0f;if(!balanced)ratio-=8.0f;return std::clamp(ratio,0.0f,100.0f);}
}
