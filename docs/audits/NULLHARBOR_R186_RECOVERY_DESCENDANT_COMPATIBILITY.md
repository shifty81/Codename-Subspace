# R186 — One-Time Recovery / Governed Descendant Compatibility

## Failure resolved

After R185 legitimately evolved `ShipEmbodimentSystem.h`, the R33–R42 prebuild normalization stopped with:

`partial R178/R179 overlay regression detected ... Known postimages: 2/7`

This was not a new overlay regression. R182 had already completed successfully and written its transactional receipt. The defect was that `receipt_matches()` required every recovered file to remain byte-identical to the original R182 after-image forever. Any later governed feature patch therefore invalidated the receipt and reactivated the pre-recovery mixed-state classifier.

## Authority rule

R182 is a one-time certified-baseline recovery migration. Its receipt proves that reconstruction happened. After that receipt exists, later governed milestones may evolve the seven recovered files provided:

1. the receipt still names the certified `8484a08` baseline and R182 strategy;
2. all seven current files still satisfy their R182 gameplay semantic postconditions; and
3. the current standalone Studio/Shipyard external authorities still satisfy their transform contracts.

The original `afterSha256` values remain provenance for the historical R182 transaction; they are not permanent locks on all future source development.

Unknown or partial R178/R179 states without a valid R182 receipt continue to use the exact historical-postimage classifier and fail closed.

## Scope

R186 changes only recovery/proof tooling. It does not modify gameplay, Studio, renderer, application, PCC, Planetary Command, or R185 native compatibility source.
