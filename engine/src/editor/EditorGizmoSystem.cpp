#include "editor/EditorGizmoSystem.h"
#include <cmath>
namespace subspace {EditorScreenDirection EditorGizmoSystem::ProjectPlanarDirection(const Vector3&d,float deg,float v){constexpr float pi=3.14159265358979323846f;float y=deg*pi/180,c=std::cos(y),s=std::sin(y);float x=d.x*c+d.y*s,sy=-(d.x*(-s)+d.y*c)*v,l=std::sqrt(x*x+sy*sy);if(l<1e-5f)return{0,-1,false};return{x/l,sy/l,true};}}
