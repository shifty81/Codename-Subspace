#include "factions/FactionProfile.h"

namespace subspace {

FactionProfile FactionDefinitions::IronDominion() {
    FactionProfile fp;
    fp.id = "iron_dominion";
    fp.displayName = "Iron Dominion";

    fp.silhouette.length    = LengthBias::Short;
    fp.silhouette.thickness = ThicknessBias::Chunky;
    fp.silhouette.symmetry  = SymmetryBias::Axial;
    fp.silhouette.mass      = MassBias::Central;
    fp.silhouette.voidType  = VoidBias::Solid;

    fp.shapeLanguage.allowedShapes = {BlockShape::Cube, BlockShape::Rect};
    fp.shapeLanguage.wedgeChance  = 0.0f;
    fp.shapeLanguage.cornerChance = 0.0f;

    fp.palette.hull  = MaterialType::Titanium;
    fp.palette.armor = MaterialType::Trinium;
    fp.palette.accent = MaterialType::Ogonite;
    fp.palette.primaryColor[0] = 0.3f;
    fp.palette.primaryColor[1] = 0.1f;
    fp.palette.primaryColor[2] = 0.1f;
    fp.palette.primaryColor[3] = 1.0f;
    fp.palette.accentColor[0] = 0.5f;
    fp.palette.accentColor[1] = 0.5f;
    fp.palette.accentColor[2] = 0.5f;
    fp.palette.accentColor[3] = 1.0f;
    fp.palette.engineColor[0] = 0.9f;
    fp.palette.engineColor[1] = 0.3f;
    fp.palette.engineColor[2] = 0.1f;
    fp.palette.engineColor[3] = 1.0f;

    fp.armorBias  = 1.4f;
    fp.engineBias = 0.6f;
    fp.weaponBias = 1.2f;

    fp.preferredHull = HullArchetype::Brick;

    return fp;
}

FactionProfile FactionDefinitions::NomadContinuum() {
    FactionProfile fp;
    fp.id = "nomad_continuum";
    fp.displayName = "Nomad Continuum";

    fp.silhouette.length    = LengthBias::Long;
    fp.silhouette.thickness = ThicknessBias::Thin;
    fp.silhouette.symmetry  = SymmetryBias::Axial;
    fp.silhouette.mass      = MassBias::Spine;
    fp.silhouette.voidType  = VoidBias::Channelled;

    fp.shapeLanguage.allowedShapes = {BlockShape::Rect, BlockShape::Slope};
    fp.shapeLanguage.wedgeChance  = 0.1f;
    fp.shapeLanguage.cornerChance = 0.0f;

    fp.palette.hull  = MaterialType::Naonite;
    fp.palette.armor = MaterialType::Titanium;
    fp.palette.accent = MaterialType::Xanion;
    fp.palette.primaryColor[0] = 0.2f;
    fp.palette.primaryColor[1] = 0.4f;
    fp.palette.primaryColor[2] = 0.8f;
    fp.palette.primaryColor[3] = 1.0f;
    fp.palette.accentColor[0] = 0.7f;
    fp.palette.accentColor[1] = 0.7f;
    fp.palette.accentColor[2] = 0.8f;
    fp.palette.accentColor[3] = 1.0f;
    fp.palette.engineColor[0] = 0.3f;
    fp.palette.engineColor[1] = 0.6f;
    fp.palette.engineColor[2] = 1.0f;
    fp.palette.engineColor[3] = 1.0f;

    fp.armorBias  = 0.8f;
    fp.engineBias = 1.4f;
    fp.weaponBias = 1.0f;

    fp.preferredHull = HullArchetype::Needle;

    return fp;
}

FactionProfile FactionDefinitions::HelixCovenant() {
    FactionProfile fp;
    fp.id = "helix_covenant";
    fp.displayName = "Helix Covenant";

    fp.silhouette.length    = LengthBias::Balanced;
    fp.silhouette.thickness = ThicknessBias::Medium;
    fp.silhouette.symmetry  = SymmetryBias::QuadRadial;
    fp.silhouette.mass      = MassBias::Winged;
    fp.silhouette.voidType  = VoidBias::Ringed;

    fp.shapeLanguage.allowedShapes = {BlockShape::Corner, BlockShape::Wedge};
    fp.shapeLanguage.wedgeChance  = 0.4f;
    fp.shapeLanguage.cornerChance = 0.4f;

    fp.palette.hull  = MaterialType::Xanion;
    fp.palette.armor = MaterialType::Ogonite;
    fp.palette.accent = MaterialType::Avorion;
    fp.palette.primaryColor[0] = 0.85f;
    fp.palette.primaryColor[1] = 0.7f;
    fp.palette.primaryColor[2] = 0.2f;
    fp.palette.primaryColor[3] = 1.0f;
    fp.palette.accentColor[0] = 0.5f;
    fp.palette.accentColor[1] = 0.2f;
    fp.palette.accentColor[2] = 0.7f;
    fp.palette.accentColor[3] = 1.0f;
    fp.palette.engineColor[0] = 0.8f;
    fp.palette.engineColor[1] = 0.4f;
    fp.palette.engineColor[2] = 1.0f;
    fp.palette.engineColor[3] = 1.0f;

    fp.armorBias  = 1.0f;
    fp.engineBias = 1.0f;
    fp.weaponBias = 1.0f;

    fp.preferredHull = HullArchetype::Disk;

    return fp;
}

FactionProfile FactionDefinitions::AshenClades() {
    FactionProfile fp;
    fp.id = "ashen_clades";
    fp.displayName = "Ashen Clades";

    fp.silhouette.length    = LengthBias::Balanced;
    fp.silhouette.thickness = ThicknessBias::Medium;
    fp.silhouette.symmetry  = SymmetryBias::Axial;
    fp.silhouette.mass      = MassBias::Distributed;
    fp.silhouette.voidType  = VoidBias::Channelled;

    fp.shapeLanguage.allowedShapes = {BlockShape::Wedge, BlockShape::Corner};
    fp.shapeLanguage.wedgeChance  = 0.35f;
    fp.shapeLanguage.cornerChance = 0.25f;

    fp.palette.hull  = MaterialType::Trinium;
    fp.palette.armor = MaterialType::Naonite;
    fp.palette.accent = MaterialType::Iron;
    fp.palette.primaryColor[0] = 0.15f;
    fp.palette.primaryColor[1] = 0.4f;
    fp.palette.primaryColor[2] = 0.15f;
    fp.palette.primaryColor[3] = 1.0f;
    fp.palette.accentColor[0] = 0.1f;
    fp.palette.accentColor[1] = 0.1f;
    fp.palette.accentColor[2] = 0.1f;
    fp.palette.accentColor[3] = 1.0f;
    fp.palette.engineColor[0] = 0.2f;
    fp.palette.engineColor[1] = 0.9f;
    fp.palette.engineColor[2] = 0.3f;
    fp.palette.engineColor[3] = 1.0f;

    fp.armorBias  = 0.9f;
    fp.engineBias = 1.1f;
    fp.weaponBias = 1.1f;

    fp.preferredHull = HullArchetype::Hammerhead;

    return fp;
}

FactionProfile FactionDefinitions::AscendedArchive() {
    FactionProfile fp;
    fp.id = "ascended_archive";
    fp.displayName = "Ascended Archive";

    fp.silhouette.length    = LengthBias::Long;
    fp.silhouette.thickness = ThicknessBias::Thin;
    fp.silhouette.symmetry  = SymmetryBias::TriRadial;
    fp.silhouette.mass      = MassBias::Central;
    fp.silhouette.voidType  = VoidBias::Solid;

    fp.shapeLanguage.allowedShapes = {BlockShape::Slope, BlockShape::Rect};
    fp.shapeLanguage.wedgeChance  = 0.2f;
    fp.shapeLanguage.cornerChance = 0.1f;

    fp.palette.hull  = MaterialType::Avorion;
    fp.palette.armor = MaterialType::Xanion;
    fp.palette.accent = MaterialType::Avorion;
    fp.palette.primaryColor[0] = 0.9f;
    fp.palette.primaryColor[1] = 0.9f;
    fp.palette.primaryColor[2] = 0.95f;
    fp.palette.primaryColor[3] = 1.0f;
    fp.palette.accentColor[0] = 0.2f;
    fp.palette.accentColor[1] = 0.8f;
    fp.palette.accentColor[2] = 0.9f;
    fp.palette.accentColor[3] = 1.0f;
    fp.palette.engineColor[0] = 0.4f;
    fp.palette.engineColor[1] = 0.9f;
    fp.palette.engineColor[2] = 1.0f;
    fp.palette.engineColor[3] = 1.0f;

    fp.armorBias  = 1.0f;
    fp.engineBias = 1.2f;
    fp.weaponBias = 1.0f;

    fp.preferredHull = HullArchetype::Needle;

    return fp;
}

std::vector<FactionProfile> FactionDefinitions::GetAllFactions() {
    return {
        IronDominion(),
        NomadContinuum(),
        HelixCovenant(),
        AshenClades(),
        AscendedArchive()
    };
}

} // namespace subspace
