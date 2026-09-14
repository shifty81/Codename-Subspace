# Pass1268-1292 R3 — Static Gate Literal Match Repair

The R2 runtime/build repair is correct. The authoritative Windows run reached
95/96 CTest targets with every native Shipyard regression target passing.

The only failure was `SubspaceProjectOpsStaticCertification`: the R2 gate used
`MATCHES "compatibilityIndex++"`. CMake treats `MATCHES` as a regular expression,
and `++` is invalid in that regex dialect.

R3 changes only the static certification gate. Source-marker assertions now use
literal `string(FIND)` checks. No engine C++, renderer, Shipyard behavior, layout,
input, generator, or persistence code changes in this repair.
