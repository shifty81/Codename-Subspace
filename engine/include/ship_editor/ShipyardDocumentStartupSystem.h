#pragma once

#include "rendering/ProceduralVisualVariantSystem.h"

namespace subspace {

// Distinguish a new studio DOCUMENT from a game-session ship. An empty recipe is
// deliberately invalid as a playable ship, but valid as a blank authoring page.
struct ShipyardDocumentStartupSystem {
    static ProceduralShipVisualRecipe EmptyDocument() {
        ProceduralShipVisualRecipe recipe{};
        recipe.recipeId="shipyard.untitled";
        recipe.role="INDUSTRIAL";
        recipe.seed=1u;
        recipe.forwardAuthority="FORWARD_MARKER";
        recipe.cockpitModuleIndex=-1;
        recipe.runtimePcgCertified=false;
        recipe.runtimeCertificationMessage="EMPTY_AUTHORING_DOCUMENT";
        return recipe;
    }

    static ProceduralShipVisualRecipe SelectInitialDocument(
        bool standaloneStudio,const ProceduralShipVisualRecipe& gameplayStarter) {
        return standaloneStudio?EmptyDocument():gameplayStarter;
    }
};

} // namespace subspace
