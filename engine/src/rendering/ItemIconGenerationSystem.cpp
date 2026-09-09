#include "rendering/ItemIconGenerationSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace subspace {
namespace {
struct P{float x=0,y=0,z=0;};
P Project(const Vector3&v){
    // Fixed three-quarter studio view of OBJ source space (+Y up,+Z forward).
    return {v.x*.7071067f-v.z*.7071067f,
            v.y*.8164966f-(v.x+v.z)*.4082483f,
            (v.x+v.y+v.z)*.5773502f};
}
float Edge(float ax,float ay,float bx,float by,float x,float y){return (x-ax)*(by-ay)-(y-ay)*(bx-ax);}
void RarityTint(ItemRarity r,float&rr,float&gg,float&bb){switch(r){case ItemRarity::Common:rr=.62f;gg=.68f;bb=.70f;break;case ItemRarity::Uncommon:rr=.34f;gg=.78f;bb=.46f;break;case ItemRarity::Rare:rr=.32f;gg=.58f;bb=.92f;break;case ItemRarity::Epic:rr=.68f;gg=.36f;bb=.88f;break;case ItemRarity::Legendary:rr=.96f;gg=.63f;bb=.18f;break;}}
}

ItemIconBitmap ItemIconGenerationSystem::Generate(const GeneratedItem& item,const ObjMeshData&mesh,int requested){
    ItemIconBitmap out;out.width=out.height=std::clamp(requested,32,256);out.cacheKey=item.iconKey;
    const int w=out.width,h=out.height;out.rgba.assign(static_cast<std::size_t>(w*h*4),0);if(mesh.positions.empty()||mesh.triangles.empty())return out;
    std::vector<P> pts;pts.reserve(mesh.positions.size());float minX=1e9f,maxX=-1e9f,minY=1e9f,maxY=-1e9f;
    for(const auto&v:mesh.positions){auto p=Project(v);pts.push_back(p);minX=std::min(minX,p.x);maxX=std::max(maxX,p.x);minY=std::min(minY,p.y);maxY=std::max(maxY,p.y);}const float span=std::max(.001f,std::max(maxX-minX,maxY-minY));const float scale=(w*.76f)/span;const float cx=(minX+maxX)*.5f,cy=(minY+maxY)*.5f;
    for(auto&p:pts){p.x=(p.x-cx)*scale+w*.5f;p.y=h*.5f-(p.y-cy)*scale;}
    std::vector<float> depth(static_cast<std::size_t>(w*h),-std::numeric_limits<float>::infinity());float tr,tg,tb;RarityTint(item.rarity,tr,tg,tb);
    for(const auto&t:mesh.triangles){if(t.position[0]<0||t.position[1]<0||t.position[2]<0||t.position[0]>=static_cast<int>(pts.size())||t.position[1]>=static_cast<int>(pts.size())||t.position[2]>=static_cast<int>(pts.size()))continue;const P&a=pts[t.position[0]],&b=pts[t.position[1]],&c=pts[t.position[2]];const float area=Edge(a.x,a.y,b.x,b.y,c.x,c.y);if(std::fabs(area)<1e-5f)continue;const int x0=std::max(1,static_cast<int>(std::floor(std::min({a.x,b.x,c.x}))));const int x1=std::min(w-2,static_cast<int>(std::ceil(std::max({a.x,b.x,c.x}))));const int y0=std::max(1,static_cast<int>(std::floor(std::min({a.y,b.y,c.y}))));const int y1=std::min(h-2,static_cast<int>(std::ceil(std::max({a.y,b.y,c.y}))));
        const float ux=b.x-a.x,uy=b.y-a.y,uz=b.z-a.z,vx=c.x-a.x,vy=c.y-a.y,vz=c.z-a.z;float nx=uy*vz-uz*vy,ny=uz*vx-ux*vz,nz=ux*vy-uy*vx;const float nl=std::sqrt(nx*nx+ny*ny+nz*nz);if(nl>1e-4f){nx/=nl;ny/=nl;nz/=nl;}const float light=std::clamp(.42f+.58f*std::fabs(nz),.28f,1.0f);
        for(int y=y0;y<=y1;++y)for(int x=x0;x<=x1;++x){const float px=x+.5f,py=y+.5f;float wa=Edge(b.x,b.y,c.x,c.y,px,py)/area,wb=Edge(c.x,c.y,a.x,a.y,px,py)/area,wc=1.0f-wa-wb;if(wa<0||wb<0||wc<0)continue;const float z=wa*a.z+wb*b.z+wc*c.z;const std::size_t di=static_cast<std::size_t>(y*w+x);if(z<=depth[di])continue;depth[di]=z;const std::size_t o=di*4;out.rgba[o]=static_cast<std::uint8_t>(std::clamp(tr*light,0.f,1.f)*255);out.rgba[o+1]=static_cast<std::uint8_t>(std::clamp(tg*light,0.f,1.f)*255);out.rgba[o+2]=static_cast<std::uint8_t>(std::clamp(tb*light,0.f,1.f)*255);out.rgba[o+3]=255;}
    }
    // One-pixel rarity frame is part of the generated icon identity.
    for(int x=0;x<w;++x)for(int y:{0,h-1}){auto o=static_cast<std::size_t>((y*w+x)*4);out.rgba[o]=static_cast<std::uint8_t>(tr*255);out.rgba[o+1]=static_cast<std::uint8_t>(tg*255);out.rgba[o+2]=static_cast<std::uint8_t>(tb*255);out.rgba[o+3]=255;}for(int y=0;y<h;++y)for(int x:{0,w-1}){auto o=static_cast<std::size_t>((y*w+x)*4);out.rgba[o]=static_cast<std::uint8_t>(tr*255);out.rgba[o+1]=static_cast<std::uint8_t>(tg*255);out.rgba[o+2]=static_cast<std::uint8_t>(tb*255);out.rgba[o+3]=255;}
    return out;
}

} // namespace subspace
