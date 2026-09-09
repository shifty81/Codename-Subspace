#include "content/ShipyardCertificationSystem.h"
#include "content/ShipyardNameClassification.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <numeric>
#include <set>
#include <sstream>
#include <unordered_map>

namespace fs = std::filesystem;

namespace subspace {
namespace {

struct Face { std::array<int, 3> v{{0, 0, 0}}; };

std::string Lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string Safe(std::string value) {
    value = Lower(value);
    for (char& c : value) if (!std::isalnum(static_cast<unsigned char>(c))) c = '_';
    while (value.find("__") != std::string::npos) value.replace(value.find("__"), 2, "_");
    while (!value.empty() && value.front() == '_') value.erase(value.begin());
    while (!value.empty() && value.back() == '_') value.pop_back();
    return value.empty() ? "mesh" : value;
}

std::string InferClassFromName(const std::string& name) {
    return ShipyardNameClassifier::Classify(name).moduleClass;
}

bool ParseIndex(const std::string& token, int vertexCount, int& out) {
    const std::size_t slash = token.find('/');
    const std::string first = token.substr(0, slash);
    if (first.empty()) return false;
    try {
        int idx = std::stoi(first);
        if (idx > 0) --idx;
        else if (idx < 0) idx = vertexCount + idx;
        else return false;
        if (idx < 0 || idx >= vertexCount) return false;
        out = idx;
        return true;
    } catch (...) {
        return false;
    }
}

struct Dsu {
    std::vector<int> p, r;
    explicit Dsu(int n) : p(n), r(n, 0) { std::iota(p.begin(), p.end(), 0); }
    int find(int x) { return p[x] == x ? x : p[x] = find(p[x]); }
    void join(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return;
        if (r[a] < r[b]) std::swap(a, b);
        p[b] = a;
        if (r[a] == r[b]) ++r[a];
    }
};

struct QuantKey {
    long long x = 0, y = 0, z = 0;
    bool operator==(const QuantKey& o) const { return x == o.x && y == o.y && z == o.z; }
};
struct QuantHash {
    std::size_t operator()(const QuantKey& k) const noexcept {
        std::size_t h = std::hash<long long>{}(k.x);
        h ^= std::hash<long long>{}(k.y) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<long long>{}(k.z) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

float BBoxVolume(const ShipyardCertifiedIsland& i) {
    return std::max(1e-9f, (i.maxX - i.minX) * (i.maxY - i.minY) * (i.maxZ - i.minZ));
}

void FillBounds(ShipyardCertifiedIsland& island, const std::vector<float>& p) {
    if (island.sourceVertexIndices.empty()) return;
    const int first = island.sourceVertexIndices.front();
    island.minX = island.maxX = p[first * 3 + 0];
    island.minY = island.maxY = p[first * 3 + 1];
    island.minZ = island.maxZ = p[first * 3 + 2];
    for (int idx : island.sourceVertexIndices) {
        const float x = p[idx * 3 + 0], y = p[idx * 3 + 1], z = p[idx * 3 + 2];
        island.minX = std::min(island.minX, x); island.maxX = std::max(island.maxX, x);
        island.minY = std::min(island.minY, y); island.maxY = std::max(island.maxY, y);
        island.minZ = std::min(island.minZ, z); island.maxZ = std::max(island.maxZ, z);
    }
}

ShipyardCertifiedIsland WholeBounds(const ShipyardObjCertification& module) {
    ShipyardCertifiedIsland whole;
    whole.moduleClass = module.sourceClass;
    whole.grade = module.moduleGrade;
    whole.role = ShipyardIslandRole::StructuralRoot;
    whole.primary = true;
    whole.sourceVertexIndices.reserve(module.positions.size() / 3);
    for (std::size_t i = 0; i < module.positions.size() / 3; ++i) whole.sourceVertexIndices.push_back(static_cast<int>(i));
    whole.vertexCount = whole.sourceVertexIndices.size();
    whole.triangleCount = module.triangles.size() / 3;
    FillBounds(whole, module.positions);
    return whole;
}

std::vector<ShipyardSocketCandidate> MakeModuleSockets(const ShipyardObjCertification& module) {
    std::vector<ShipyardSocketCandidate> out;
    if (module.positions.empty()) return out;
    const auto i = WholeBounds(module);
    const float cx = (i.minX + i.maxX) * .5f, cy = (i.minY + i.maxY) * .5f, cz = (i.minZ + i.maxZ) * .5f;
    const float hx = std::max(.001f, (i.maxX - i.minX) * .5f);
    const float hy = std::max(.001f, (i.maxY - i.minY) * .5f);
    const float hz = std::max(.001f, (i.maxZ - i.minZ) * .5f);
    const float clearance = .32f * std::max({hx, hy, hz});
    auto add = [&](std::string name, std::string type, float x, float y, float z,
                   float dx, float dy, float dz, float cr) {
        out.push_back({std::move(name), std::move(type), x, y, z, dx, dy, dz, cr});
    };

    const std::string c = Lower(module.sourceClass);
    const auto classified = ShipyardNameClassifier::Classify(module.sourceName);
    if (c == "propulsion") {
        add("hull_mount", "hull_aft", cx, i.maxY, cz, 0, 1, 0, clearance);
        if (classified.semantic == "ENGINE_HOUSING") {
            add("engine_cavity", "engine_cavity", cx, cy - hy * .45f, cz, 0, -1, 0, std::max(hx, hz) * .78f);
        } else {
            add("exhaust", "exhaust", cx, i.minY, cz, 0, -1, 0, std::max(hx, hz));
        }
    } else if (classified.semantic == "STRUCTURAL_FRAME") {
        add("forward", "hull", cx, i.maxY, cz, 0, 1, 0, clearance);
        add("aft", "hull", cx, i.minY, cz, 0, -1, 0, clearance);
        add("port", "lateral", i.minX, cy, cz, -1, 0, 0, clearance);
        add("starboard", "lateral", i.maxX, cy, cz, 1, 0, 0, clearance);
    } else if (c == "command") {
        add("aft", "hull_forward", cx, i.minY, cz, 0, -1, 0, clearance);
    } else if (c == "hardpoint") {
        add("mount", "hardpoint_surface", cx, cy, i.minZ, 0, 0, -1, std::max(hx, hy));
        add("weapon_axis", "weapon_axis", cx, cy, i.maxZ, 0, 0, 1, std::max(hx, hy));
    } else if (c == "detail") {
        add("mount", "surface_detail", cx, cy, i.minZ, 0, 0, -1, std::max(hx, hy));
    } else if (c == "adapter") {
        add("forward", "hull", cx, i.maxY, cz, 0, 1, 0, clearance);
        add("aft", "hull", cx, i.minY, cz, 0, -1, 0, clearance);
    } else {
        add("forward", "hull", cx, i.maxY, cz, 0, 1, 0, clearance);
        add("aft", "hull", cx, i.minY, cz, 0, -1, 0, clearance);
        add("port", "lateral", i.minX, cy, cz, -1, 0, 0, clearance);
        add("starboard", "lateral", i.maxX, cy, cz, 1, 0, 0, clearance);
        add("dorsal", "surface", cx, cy, i.maxZ, 0, 0, 1, clearance);
        add("ventral", "surface", cx, cy, i.minZ, 0, 0, -1, clearance);
        // Every hull can expose an inset aft engine pair. If an authored hull
        // visually contains shrouds these sockets seat the drive inside that
        // rear volume instead of floating it behind the model.
        add("engine_port", "engine_cavity", cx - hx * .42f, i.minY + hy * .55f, cz, 0, -1, 0, std::max(hx, hz) * .32f);
        add("engine_starboard", "engine_cavity", cx + hx * .42f, i.minY + hy * .55f, cz, 0, -1, 0, std::max(hx, hz) * .32f);
    }
    return out;
}

std::string JsonEscape(const std::string& s) {
    std::string o;
    for (char c : s) { if (c == '\\' || c == '"') o.push_back('\\'); o.push_back(c); }
    return o;
}


std::string Trim(std::string value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) value.erase(value.begin());
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) value.pop_back();
    return value;
}

ShipyardCertifiedMaterial* FindOrAddMaterial(ShipyardObjCertification& c, const std::string& name) {
    for (auto& material : c.materials) if (material.name == name) return &material;
    c.materials.push_back({});
    c.materials.back().name = name;
    return &c.materials.back();
}

std::string MtlTexturePath(const std::string& rest) {
    std::vector<std::string> tokens;std::string current;char quote=0;
    for(char c:rest){if(quote){if(c==quote)quote=0;else current.push_back(c);continue;}if(c=='"'||c=='\''){quote=c;continue;}if(std::isspace(static_cast<unsigned char>(c))){if(!current.empty()){tokens.push_back(current);current.clear();}continue;}current.push_back(c);}
    if(!current.empty())tokens.push_back(current);return tokens.empty()?std::string{}:tokens.back();
}

void ResolveMaterialSidecars(ShipyardObjCertification& c,
                             const fs::path& objDirectory,
                             const fs::path& outputModules,
                             std::set<std::string>& copiedSidecars) {
    std::error_code ec;
    for (const auto& libName : c.materialLibraries) {
        const fs::path srcMtl = objDirectory / fs::path(libName).filename();
        if (!fs::exists(srcMtl, ec)) continue;
        const fs::path dstMtl = outputModules / srcMtl.filename();
        fs::copy_file(srcMtl, dstMtl, fs::copy_options::overwrite_existing, ec);
        if (!ec) copiedSidecars.insert(dstMtl.filename().string());

        std::ifstream mtl(srcMtl);
        std::string line;
        ShipyardCertifiedMaterial* current = nullptr;
        while (std::getline(mtl, line)) {
            std::istringstream ls(line);
            std::string op; ls >> op;
            if (op.empty() || op[0] == '#') continue;
            if (op == "newmtl") {
                std::string name; std::getline(ls, name); name = Trim(name);
                if (!name.empty()) { current = FindOrAddMaterial(c, name); current->resolvedFromMtl = true; }
                continue;
            }
            if (!current) continue;
            std::string rest; std::getline(ls, rest); rest = Trim(rest);
            const std::string texName = MtlTexturePath(rest);
            bool* flag = nullptr;
            if (op == "map_Kd" || op == "map_BaseColor" || op == "map_albedo" || op == "map_diffuse") flag = &current->hasBaseColorTexture;
            else if (op == "map_Bump" || op == "bump" || op == "map_bump" || op == "map_normal" || op == "norm") flag = &current->hasNormalTexture;
            else if (op == "map_Pm" || op == "map_metallic") flag = &current->hasMetallicTexture;
            else if (op == "map_Pr" || op == "map_roughness") flag = &current->hasRoughnessTexture;
            else if (op == "map_Ke" || op == "map_emissive") flag = &current->hasEmissiveTexture;
            if (!flag || texName.empty()) continue;
            const fs::path srcTex = objDirectory / fs::path(texName).filename();
            if (!fs::exists(srcTex, ec)) continue;
            const fs::path dstTex = outputModules / srcTex.filename();
            fs::copy_file(srcTex, dstTex, fs::copy_options::overwrite_existing, ec);
            if (!ec) { *flag = true; copiedSidecars.insert(dstTex.filename().string()); }
        }
    }
}

void WriteMetadata(const ShipyardObjCertification& c, const fs::path& path, const std::string& id) {
    std::ofstream o(path);
    if (!o) return;
    o << "{\n"
      << "  \"moduleId\": \"" << JsonEscape(id) << "\",\n"
      << "  \"sourceName\": \"" << JsonEscape(c.sourceName) << "\",\n"
      << "  \"moduleClass\": \"" << JsonEscape(c.sourceClass) << "\",\n"
      << "  \"semantic\": \"" << JsonEscape(ShipyardNameClassifier::Classify(c.sourceName).semantic) << "\",\n"
      << "  \"classificationConfidence\": " << ShipyardNameClassifier::Classify(c.sourceName).confidence << ",\n"
      << "  \"classificationRule\": \"" << JsonEscape(ShipyardNameClassifier::Classify(c.sourceName).rule) << "\",\n"
      << "  \"grade\": \"" << ShipyardCertificationSystem::GradeName(c.moduleGrade) << "\",\n"
      << "  \"authoredObjectPreserved\": true,\n"
      << "  \"diagnosticIslandCount\": " << c.islands.size() << ",\n"
      << "  \"connectivityPolicy\": \"DIAGNOSTIC_ONLY\",\n"
      << "  \"sockets\": [\n";
    for (std::size_t s = 0; s < c.sockets.size(); ++s) {
        const auto& q = c.sockets[s];
        o << "    {\"name\":\"" << q.name << "\",\"type\":\"" << q.type << "\",\"position\":["
          << q.x << ',' << q.y << ',' << q.z << "],\"direction\":[" << q.dirX << ',' << q.dirY << ',' << q.dirZ
          << "],\"clearanceRadius\":" << q.clearanceRadius << "}" << (s + 1 < c.sockets.size() ? "," : "") << "\n";
    }
    o << "  ]\n}\n";
}

} // namespace

ShipyardObjCertification ShipyardCertificationSystem::AnalyzeObj(const std::string& text,
                                                                  const std::string& sourceName,
                                                                  const std::string& sourceClass) {
    ShipyardObjCertification result;
    result.sourceName = sourceName;
    result.sourceClass = sourceClass.empty() ? InferClassFromName(sourceName) : Lower(sourceClass);
    result.preserveAuthoredObject = true;

    std::vector<Face> faces;
    std::istringstream in(text);
    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ls(line);
        std::string op;
        ls >> op;
        if (op == "v") {
            float x, y, z;
            if (!(ls >> x >> y >> z)) { result.error = "invalid vertex at line " + std::to_string(lineNo); return result; }
            result.positions.insert(result.positions.end(), {x, y, z});
        } else if (op == "vt") {
            result.hasTexcoords = true;
        } else if (op == "vn") {
            result.hasNormals = true;
        } else if (op == "mtllib") {
            std::string lib; while (ls >> lib) if (!lib.empty()) result.materialLibraries.push_back(lib);
        } else if (op == "usemtl") {
            std::string name; std::getline(ls, name); name = Trim(name);
            if (!name.empty()) FindOrAddMaterial(result, name);
        } else if (op == "f") {
            std::vector<int> poly;
            std::string tok;
            while (ls >> tok) {
                int idx = 0;
                if (!ParseIndex(tok, static_cast<int>(result.positions.size() / 3), idx)) {
                    result.error = "invalid face at line " + std::to_string(lineNo); return result;
                }
                poly.push_back(idx);
            }
            if (poly.size() < 3) continue;
            for (std::size_t k = 1; k + 1 < poly.size(); ++k) faces.push_back({{poly[0], poly[k], poly[k + 1]}});
        }
    }
    if (result.positions.empty() || faces.empty()) { result.error = "OBJ contains no usable triangle geometry"; return result; }
    for (const auto& f : faces) result.triangles.insert(result.triangles.end(), f.v.begin(), f.v.end());

    float minX = result.positions[0], maxX = minX, minY = result.positions[1], maxY = minY, minZ = result.positions[2], maxZ = minZ;
    for (std::size_t i = 0; i < result.positions.size() / 3; ++i) {
        minX = std::min(minX, result.positions[i * 3]); maxX = std::max(maxX, result.positions[i * 3]);
        minY = std::min(minY, result.positions[i * 3 + 1]); maxY = std::max(maxY, result.positions[i * 3 + 1]);
        minZ = std::min(minZ, result.positions[i * 3 + 2]); maxZ = std::max(maxZ, result.positions[i * 3 + 2]);
    }
    const float major = std::max({maxX - minX, maxY - minY, maxZ - minZ, 1e-5f});
    const double eps = std::max(1e-7, static_cast<double>(major) * 1e-6);
    std::unordered_map<QuantKey, int, QuantHash> canonical;
    std::vector<int> cv(result.positions.size() / 3);
    for (std::size_t i = 0; i < cv.size(); ++i) {
        QuantKey k{llround(result.positions[i * 3] / eps), llround(result.positions[i * 3 + 1] / eps), llround(result.positions[i * 3 + 2] / eps)};
        auto [it, added] = canonical.emplace(k, static_cast<int>(i));
        (void)added;
        cv[i] = it->second;
    }

    Dsu d(static_cast<int>(faces.size()));
    std::unordered_map<int, int> firstFace;
    for (int fi = 0; fi < static_cast<int>(faces.size()); ++fi) {
        for (int v : faces[fi].v) {
            const int key = cv[v];
            auto it = firstFace.find(key);
            if (it == firstFace.end()) firstFace[key] = fi;
            else d.join(fi, it->second);
        }
    }
    std::map<int, std::vector<int>> groups;
    for (int fi = 0; fi < static_cast<int>(faces.size()); ++fi) groups[d.find(fi)].push_back(fi);
    for (const auto& [root, fl] : groups) {
        (void)root;
        ShipyardCertifiedIsland island;
        island.islandIndex = result.islands.size();
        island.sourceTriangleIndices = fl;
        std::set<int> verts;
        for (int fi : fl) for (int v : faces[fi].v) verts.insert(v);
        island.sourceVertexIndices.assign(verts.begin(), verts.end());
        island.vertexCount = verts.size();
        island.triangleCount = fl.size();
        FillBounds(island, result.positions);
        result.islands.push_back(std::move(island));
    }
    if (result.islands.empty()) { result.error = "no connected components found"; return result; }

    auto primaryIt = std::max_element(result.islands.begin(), result.islands.end(), [](const auto& a, const auto& b) {
        if (a.triangleCount != b.triangleCount) return a.triangleCount < b.triangleCount;
        return BBoxVolume(a) < BBoxVolume(b);
    });
    const std::size_t primaryIndex = static_cast<std::size_t>(std::distance(result.islands.begin(), primaryIt));
    const float pv = BBoxVolume(*primaryIt);
    const float pt = static_cast<float>(std::max<std::size_t>(1, primaryIt->triangleCount));
    for (std::size_t idx = 0; idx < result.islands.size(); ++idx) {
        auto& island = result.islands[idx];
        island.primary = idx == primaryIndex;
        island.volumeRatio = BBoxVolume(island) / std::max(1e-9f, pv);
        island.triangleRatio = static_cast<float>(island.triangleCount) / pt;
        island.moduleClass = result.sourceClass;
        if (island.primary) {
            island.grade = ShipyardCertificationGrade::A;
            island.role = ShipyardIslandRole::StructuralRoot;
        } else if (island.volumeRatio >= .12f || island.triangleRatio >= .15f) {
            island.grade = ShipyardCertificationGrade::B;
            island.role = ShipyardIslandRole::FunctionalComponent;
        } else {
            island.grade = ShipyardCertificationGrade::C;
            island.role = ShipyardIslandRole::SurfaceDetail;
        }
    }

    // Valid extracted Shipyard objects remain intact regardless of how many
    // authored loose parts they contain. Connectivity is recorded for review,
    // not used as an automatic destructive edit.
    result.moduleGrade = ShipyardCertificationGrade::A;
    result.valid = true;
    result.sockets = MakeModuleSockets(result);
    return result;
}

std::vector<ShipyardSocketCandidate> ShipyardCertificationSystem::SuggestModuleSockets(const ShipyardObjCertification& module) {
    return MakeModuleSockets(module);
}

const char* ShipyardCertificationSystem::GradeName(ShipyardCertificationGrade g) {
    switch (g) {
    case ShipyardCertificationGrade::A: return "A";
    case ShipyardCertificationGrade::B: return "B";
    case ShipyardCertificationGrade::C: return "C";
    case ShipyardCertificationGrade::Reject: return "REJECT";
    }
    return "REJECT";
}

const char* ShipyardCertificationSystem::RoleName(ShipyardIslandRole r) {
    switch (r) {
    case ShipyardIslandRole::StructuralRoot: return "STRUCTURAL_ROOT";
    case ShipyardIslandRole::FunctionalComponent: return "FUNCTIONAL_COMPONENT";
    case ShipyardIslandRole::SurfaceDetail: return "SURFACE_DETAIL";
    case ShipyardIslandRole::ReviewRequired: return "REVIEW_REQUIRED";
    }
    return "REVIEW_REQUIRED";
}

bool ShipyardCertificationSystem::CertifyCorpus(const std::string& inputRoot,
                                                const std::string& outputRoot,
                                                ShipyardCorpusCertificationSummary* summary,
                                                std::string* error) {
    ShipyardCorpusCertificationSummary s;
    std::error_code ec;
    const fs::path in(inputRoot), out(outputRoot), modules = out / "modules", metadata = out / "metadata";
    const bool stableR5Source = fs::exists(in / "SHIPYARD_STRIKES_BACK_R5_SOURCE.txt", ec);
    fs::remove_all(out, ec);
    fs::create_directories(modules, ec);
    fs::create_directories(metadata, ec);
    if (!fs::exists(in)) { if (error) *error = "input root does not exist: " + in.string(); return false; }

    std::ofstream catalog(out / "certified_module_catalog.csv");
    std::ofstream audit(out / "connectivity_diagnostics.csv");
    std::ofstream materialAudit(out / "material_diagnostics.csv");
    if (!catalog || !audit || !materialAudit) { if (error) *error = "could not create certification catalogs"; return false; }
    catalog << "module_id,class,grade,source_obj,diagnostic_islands,relative_path,socket_count,preserved_authored_object,semantic,classification_confidence,classification_rule,source_pack,source_license\n";
    audit << "source_obj,island_index,role,vertices,triangles,volume_ratio,triangle_ratio,primary\n";
    materialAudit << "source_obj,material,resolved_from_mtl,base_color,normal,metallic,roughness,emissive\n";
    std::set<std::string> copiedSidecars;

    // Certification IDs are part of the persistent runtime/module contract.
    // Filesystem directory iteration order is not portable (Windows and Linux
    // can return different orders), so collect and sort candidate OBJ paths
    // before assigning serial identities.
    std::vector<fs::path> sourceObjects;
    for (const auto& entry : fs::recursive_directory_iterator(in, ec)) {
        if (ec) break;
        if (entry.is_regular_file() && Lower(entry.path().extension().string()) == ".obj") sourceObjects.push_back(entry.path());
    }
    if(ec){if(error)*error="could not enumerate certification input: "+ec.message();return false;}
    std::sort(sourceObjects.begin(),sourceObjects.end(),[](const fs::path& a,const fs::path& b){
        return a.generic_string()<b.generic_string();
    });

    int serial = 0;
    for (const auto& sourcePath : sourceObjects) {
        const std::string stem=sourcePath.stem().string();
        if(stableR5Source && stem.rfind("shipyard_a_",0)!=0) continue;
        ++s.sourceObjects;
        std::ifstream f(sourcePath);
        std::ostringstream buf;
        buf << f.rdbuf();
        const std::string sourceClass = InferClassFromName(sourcePath.stem().string());
        auto c = AnalyzeObj(buf.str(), sourcePath.stem().string(), sourceClass);
        if (!c.valid) { ++s.rejected; continue; }
        ResolveMaterialSidecars(c, sourcePath.parent_path(), modules, copiedSidecars);
        if (!c.materials.empty()) ++s.modulesWithMaterialSlots;
        s.materialSlots += static_cast<int>(c.materials.size());
        bool unresolved = false;
        for (const auto& material : c.materials) {
            if (material.resolvedFromMtl) ++s.resolvedMaterials; else unresolved = true;
            materialAudit << sourcePath.filename().string() << ',' << material.name << ','
                          << (material.resolvedFromMtl?1:0) << ',' << (material.hasBaseColorTexture?1:0) << ','
                          << (material.hasNormalTexture?1:0) << ',' << (material.hasMetallicTexture?1:0) << ','
                          << (material.hasRoughnessTexture?1:0) << ',' << (material.hasEmissiveTexture?1:0) << "\n";
        }
        if (unresolved) ++s.modulesWithUnresolvedMaterialAuthority;
        if (c.islands.size() > 1) ++s.multiIslandSourceObjects;
        for (const auto& island : c.islands) {
            audit << sourcePath.filename().string() << ',' << island.islandIndex << ',' << RoleName(island.role) << ','
                  << island.vertexCount << ',' << island.triangleCount << ',' << island.volumeRatio << ',' << island.triangleRatio << ','
                  << (island.primary ? 1 : 0) << "\n";
        }

        ++s.gradeA;
        ++serial;
        std::string id;
        if(stem.rfind("shipyard_a_",0)==0) id=stem; // R5 migration preserves stable Subspace identities.
        else { std::ostringstream n; n << "shipyard_a_" << Safe(c.sourceClass) << '_' << std::setw(3) << std::setfill('0') << serial << '_' << Safe(c.sourceName); id=n.str(); }
        const fs::path obj = modules / (id + ".obj");
        fs::copy_file(sourcePath, obj, fs::copy_options::overwrite_existing, ec);
        if (ec) { if (error) *error = "could not preserve authored module " + sourcePath.string() + ": " + ec.message(); return false; }
        WriteMetadata(c, metadata / (id + ".json"), id);
        const auto nameClassification = ShipyardNameClassifier::Classify(c.sourceName);
        catalog << id << ',' << c.sourceClass << ",A," << sourcePath.filename().string() << ',' << c.islands.size()
                << ",content/derived/greyoxide_shipyard_v07/certified/modules/" << obj.filename().string() << ',' << c.sockets.size() << ",1,"
                << nameClassification.semantic << ',' << nameClassification.confidence << ',' << nameClassification.rule << ','
                << (stableR5Source?"SHIPYARD_STRIKES_BACK":"GREYOXIDE_V07") << ',' << (stableR5Source?"CC-BY-SA-4.0":"CC0-1.0") << "\n";
    }

    std::ofstream marker(out / "SHIPYARD_CERTIFIED_R5.txt");
    marker << "Shipyard Canonical Authority Certification R5\n"
           << "policy=PRESERVE_AUTHORED_OBJECTS\n"
           << "visual_authority=SHIPYARD_ONLY\n"
           << "source_pack=" << (stableR5Source?"SHIPYARD_STRIKES_BACK":"GREYOXIDE_V07") << "\n"
           << "source_license=" << (stableR5Source?"CC-BY-SA-4.0":"CC0-1.0") << "\n"
           << "source_objects=" << s.sourceObjects << "\n"
           << "grade_a=" << s.gradeA << "\n"
           << "rejected=" << s.rejected << "\n"
           << "multi_island_diagnostics=" << s.multiIslandSourceObjects << "\n";
    // Retain the historic R3V1 marker as a compatibility breadcrumb for old
    // diagnostics while R5 remains the authoritative certification marker.
    std::ofstream legacyMarker(out / "SHIPYARD_CERTIFIED_R3V1.txt");
    legacyMarker << "Shipyard compatibility certification R3V1 -> R5\n"
                 << "authoritative_marker=SHIPYARD_CERTIFIED_R5.txt\n";
    if (summary) *summary = s;
    if (s.gradeA <= 0) { if (error) *error = "certification produced no Grade-A authored modules"; return false; }
    return true;
}

} // namespace subspace
