#pragma once
#include <string>
#include <unordered_map>
#include <vector>
namespace subspace {
enum class ResourceTier { Raw, Refined, Industrial, Advanced, Strategic };
struct MaterialDefinition { std::string id; std::string name; ResourceTier tier=ResourceTier::Raw; double massPerUnit=1; double structuralStrength=1; double thermalResistance=0; double radiationResistance=0; double rarity=0; };
struct MaterialRecipe { std::string id; std::unordered_map<std::string,double> inputs; std::string output; double outputAmount=1; };
class ResourceMaterialSystem {
public:
 ResourceMaterialSystem();
 bool RegisterMaterial(const MaterialDefinition& material);
 bool RegisterRecipe(const MaterialRecipe& recipe);
 const MaterialDefinition* Get(const std::string& id) const;
 bool CanProduce(const std::string& recipeId,const std::unordered_map<std::string,double>& inventory) const;
 bool Produce(const std::string& recipeId,std::unordered_map<std::string,double>& inventory) const;
 std::vector<std::string> MaterialsAtLeast(ResourceTier tier) const;
private:std::unordered_map<std::string,MaterialDefinition> materials_;std::unordered_map<std::string,MaterialRecipe> recipes_;
};
}
