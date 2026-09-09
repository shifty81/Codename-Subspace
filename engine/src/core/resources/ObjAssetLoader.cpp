#include "core/resources/ObjAssetLoader.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace subspace {

namespace fs = std::filesystem;

static std::string Trim(std::string value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) value.erase(value.begin());
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) value.pop_back();
    if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') || (value.front() == '\'' && value.back() == '\'')))
        value = value.substr(1, value.size() - 2);
    return value;
}

static std::vector<std::string> TokenizeQuoted(const std::string& value) {
    std::vector<std::string> out; std::string current; char quote=0;
    for(char c:value){
        if(quote){ if(c==quote){quote=0;} else current.push_back(c); continue; }
        if(c=='"'||c=='\''){quote=c;continue;}
        if(std::isspace(static_cast<unsigned char>(c))){if(!current.empty()){out.push_back(current);current.clear();}continue;}
        current.push_back(c);
    }
    if(!current.empty())out.push_back(current);
    return out;
}

static std::string MtlTextureLeaf(std::string rest) {
    rest = Trim(rest); if(rest.empty()) return {};
    const auto tokens=TokenizeQuoted(rest); if(tokens.empty()) return {};
    // Shipyard exports put map options before the file. Quoted filenames with
    // spaces survive as one token; otherwise the final token is the file path.
    return Trim(tokens.back());
}

static ObjMaterialData* FindOrAddMaterial(ObjMeshData& out, const std::string& name) {
    for (auto& material : out.materials) if (material.name == name) return &material;
    out.materialNames.push_back(name);
    out.materials.push_back(ObjMaterialData{});
    out.materials.back().name = name;
    return &out.materials.back();
}

static std::string ResolveTexturePath(const fs::path& directory, const std::string& raw) {
    if (raw.empty()) return {};
    std::error_code ec;
    fs::path candidate = directory / fs::path(raw);
    if (fs::exists(candidate, ec)) return candidate.lexically_normal().string();
    candidate = directory / fs::path(raw).filename();
    if (fs::exists(candidate, ec)) return candidate.lexically_normal().string();
    return {};
}

static void LoadMtlLibraries(const fs::path& objPath, ObjMeshData& out) {
    const fs::path directory = objPath.parent_path();
    for (const auto& library : out.materialLibraries) {
        fs::path mtlPath = directory / fs::path(library);
        std::error_code ec;
        if (!fs::exists(mtlPath, ec)) mtlPath = directory / fs::path(library).filename();
        std::ifstream input(mtlPath);
        if (!input) continue;
        ObjMaterialData* current = nullptr;
        std::string line;
        while (std::getline(input, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream ls(line);
            std::string op; ls >> op;
            if (op == "newmtl") {
                std::string name; std::getline(ls, name); name = Trim(name);
                if (!name.empty()) { current = FindOrAddMaterial(out, name); current->resolvedFromMtl = true; }
                continue;
            }
            if (!current) continue;
            if (op == "Kd") {
                if (ls >> current->diffuseR >> current->diffuseG >> current->diffuseB) current->hasDiffuseColor = true;
            } else if (op == "Ke") {
                ls >> current->emissiveR >> current->emissiveG >> current->emissiveB;
            } else if (op == "Ns") {
                float ns = current->shininess; if (ls >> ns) current->shininess = std::clamp(ns * 0.096f, 0.0f, 96.0f);
            } else if (op == "d") {
                float a = 1.0f; if (ls >> a) current->alpha = std::clamp(a, 0.0f, 1.0f);
            } else if (op == "Tr") {
                float tr = 0.0f; if (ls >> tr) current->alpha = std::clamp(1.0f - tr, 0.0f, 1.0f);
            } else if (op == "Pm") {
                float value = -1.0f; if (ls >> value) current->metallic = std::clamp(value, 0.0f, 1.0f);
            } else if (op == "Pr") {
                float value = -1.0f; if (ls >> value) current->roughness = std::clamp(value, 0.0f, 1.0f);
            } else if (op == "map_Kd" || op == "map_BaseColor" || op == "map_albedo" || op == "map_diffuse" ||
                       op == "map_Bump" || op == "bump" || op == "map_bump" || op == "map_normal" || op == "norm" ||
                       op == "map_Pm" || op == "map_metallic" || op == "map_Pr" || op == "map_roughness" || op == "map_Ke" || op == "map_emissive") {
                std::string rest; std::getline(ls, rest);
                const std::string resolved = ResolveTexturePath(directory, MtlTextureLeaf(rest));
                if (op == "map_Kd" || op == "map_BaseColor" || op == "map_albedo" || op == "map_diffuse") current->baseColorTexturePath = resolved;
                else if (op == "map_Bump" || op == "bump" || op == "map_bump" || op == "map_normal" || op == "norm") current->normalTexturePath = resolved;
                else if (op == "map_Pm" || op == "map_metallic") current->metallicTexturePath = resolved;
                else if (op == "map_Pr" || op == "map_roughness") current->roughnessTexturePath = resolved;
                else if (op == "map_Ke" || op == "map_emissive") current->emissiveTexturePath = resolved;
            }
        }
    }
}

bool ObjAssetLoader::LoadFile(const std::string& path, ObjMeshData& out, std::string* error) const {
    std::ifstream f(path); if(!f){if(error)*error="could not open OBJ: "+path; return false;}
    std::ostringstream s; s<<f.rdbuf();
    if (!Parse(s.str(), out, error)) return false;
    LoadMtlLibraries(fs::path(path), out);
    return true;
}

static bool ResolveObjIndex(const std::string& text, int count, int& out) {
    if(text.empty()) { out=-1; return true; }
    try {
        int idx=std::stoi(text);
        if(idx>0) idx-=1;
        else if(idx<0) idx=count+idx;
        else return false;
        if(idx<0||idx>=count)return false;
        out=idx; return true;
    } catch(...){return false;}
}

struct FaceVertex { int p=-1,t=-1,n=-1; };
static bool ParseFaceVertex(const std::string& token, int pc, int tc, int nc, FaceVertex& out) {
    const auto s1=token.find('/');
    if(s1==std::string::npos) return ResolveObjIndex(token,pc,out.p);
    if(!ResolveObjIndex(token.substr(0,s1),pc,out.p)) return false;
    const auto s2=token.find('/',s1+1);
    if(s2==std::string::npos) return ResolveObjIndex(token.substr(s1+1),tc,out.t);
    if(!ResolveObjIndex(token.substr(s1+1,s2-s1-1),tc,out.t)) return false;
    return ResolveObjIndex(token.substr(s2+1),nc,out.n);
}

bool ObjAssetLoader::Parse(const std::string& text, ObjMeshData& out, std::string* error) const {
    out=ObjMeshData{}; std::istringstream input(text); std::string line; int lineNo=0; int currentMaterial=-1;
    auto materialIndex=[&](const std::string& name){
        for(std::size_t i=0;i<out.materialNames.size();++i) if(out.materialNames[i]==name) return static_cast<int>(i);
        out.materialNames.push_back(name);
        out.materials.push_back(ObjMaterialData{});
        out.materials.back().name = name;
        return static_cast<int>(out.materialNames.size()-1);
    };
    while(std::getline(input,line)){++lineNo; if(line.empty()||line[0]=='#')continue; std::istringstream ls(line); std::string op; ls>>op;
        if(op=="v"){float x,y,z;if(!(ls>>x>>y>>z)){if(error)*error="invalid vertex at line "+std::to_string(lineNo);return false;}out.positions.emplace_back(x,y,z);}
        else if(op=="vn"){float x,y,z;if(!(ls>>x>>y>>z)){if(error)*error="invalid normal at line "+std::to_string(lineNo);return false;}out.normals.emplace_back(x,y,z);}
        else if(op=="vt"){float u,v;if(!(ls>>u>>v)){if(error)*error="invalid texcoord at line "+std::to_string(lineNo);return false;}out.texcoords.push_back({u,v});}
        else if(op=="mtllib"){
            std::string rest;std::getline(ls,rest);for(const auto& lib:TokenizeQuoted(rest))if(!lib.empty())out.materialLibraries.push_back(lib);
        }
        else if(op=="usemtl"){std::string name;std::getline(ls,name);while(!name.empty()&&std::isspace(static_cast<unsigned char>(name.front())))name.erase(name.begin());if(!name.empty())currentMaterial=materialIndex(name);}
        else if(op=="f"){
            std::vector<FaceVertex> face;std::string tok;
            while(ls>>tok){FaceVertex fv;if(!ParseFaceVertex(tok,static_cast<int>(out.positions.size()),static_cast<int>(out.texcoords.size()),static_cast<int>(out.normals.size()),fv)){if(error)*error="invalid face at line "+std::to_string(lineNo);return false;}face.push_back(fv);}
            if(face.size()<3){if(error)*error="face has fewer than 3 vertices";return false;}
            for(size_t i=1;i+1<face.size();++i){ObjTriangle tri; const FaceVertex fv[3]={face[0],face[i],face[i+1]}; for(int c=0;c<3;++c){tri.position[c]=fv[c].p;tri.texcoord[c]=fv[c].t;tri.normal[c]=fv[c].n;}tri.materialIndex=currentMaterial;out.triangles.push_back(tri);}
        }
    }
    if(out.positions.empty()){if(error)*error="OBJ contains no vertices";return false;} return true;
}

} // namespace subspace
