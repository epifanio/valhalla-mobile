# Adventure-riding costing extension

This fork of `valhalla-mobile` vendors
[`epifanio/valhalla`](https://github.com/epifanio/valhalla) on branch
`feat/adventure-riding` instead of upstream
[`valhalla/valhalla`](https://github.com/valhalla/valhalla). The fork adds a
per-edge `TaggedValue::kAdventureRiding` attribute plus three per-request
costing options that let riders express preference for curated off-road
networks (Trans Euro Trail, EuroVelo, Backcountry Discovery Routes, …)
without changing the tile format. Stock-Valhalla tiles without the tag
route exactly as before — the new options are no-ops for them.

The C++ wrapper is a transparent JSON pass-through (see
`src/wrapper/valhalla_actor.cpp`), so the new options are available
immediately as request keys; no Swift/Kotlin type changes are required.

## The three options

All three live on each profile's `costing_options.<profile>` block. They
default to 1.0 (no bias, no skill modifier) and pay zero cost in the
EdgeCost hot path when unset. Currently wired into `auto`, `motorcycle`,
`bicycle`, and `pedestrian` profiles.

| key | range | default | what it does |
|---|---|---|---|
| `use_adventure_riding` | 0.0 – 1.0 | 1.0 | Cost multiplier on adventure-riding edges. 1.0 = neutral; 0.0 = "always prefer the trail if reachable"; intermediate values prefer it linearly. |
| `use_adventure_riding_curve` | 0.1 – 10.0 | 1.0 | Exponent on the bias. `effective = pow(use_adventure_riding, use_adventure_riding_curve)`. 1.0 = linear (legacy). 3 makes `bias=0.5` act like `bias≈0.125`. Useful because real OSM trail/road cost ratios are large enough that linear bias=0.5 leaves trail above road on most fixtures. |
| `adventure_riding_speed_factor` | 0.3 – 3.0 | 1.0 | Rider-skill speed multiplier on adventure-riding edges. Scales the OSM-tagged speed at routing time, affecting BOTH the cost and the reported `trip.summary.time`. 0.5 = "I take trails at half the average rider's pace"; 1.5 = "expert, 1.5×". |

## Calling from Swift

The typed `RouteRequest` (from `valhalla-openapi-models-swift`) doesn't
yet model these fields, so use the raw-JSON escape hatch:

```swift
let valhalla = try Valhalla(configPath: "/path/to/valhalla.json")

let requestJSON = """
{
  "locations": [
    {"lat": 58.50, "lon": 14.40},
    {"lat": 58.70, "lon": 14.50}
  ],
  "costing": "motorcycle",
  "costing_options": {
    "motorcycle": {
      "use_adventure_riding": 0.3,
      "use_adventure_riding_curve": 3.0,
      "adventure_riding_speed_factor": 0.7
    }
  }
}
"""

let responseJSON = valhalla.route(rawRequest: requestJSON)
```

`route(rawRequest:)` returns the standard Valhalla JSON response shape —
`trip.summary.cost`, `trip.summary.time`, `trip.legs[].maneuvers[]` etc.
The route through adventure-riding edges shows their `name`/`ref` in the
maneuver street_names (e.g. `"TET-N-01"`).

## Calling from Kotlin (Android)

Same shape — the Android JNI route entry point (`ValhallaKotlin.route` in
`src/wrapper/main.cpp`) takes the request as a `jstring` and forwards it
verbatim. Construct the JSON in Kotlin and call as usual.

## Tile-build requirement

The `kAdventureRiding` tag only appears in tiles built from a PBF whose
ways carry `source=<network-name>` AND the tile builder was given a
matching allow-list in `valhalla.json`:

```json
{
  "mjolnir": {
    "adventure_riding_sources": {"TET": 1, "EuroVelo": 2, "BDR": 3}
  }
}
```

The numeric ids correspond to `baldr::AdventureRidingClass` in
`valhalla/baldr/graphconstants.h`. Without this config the parser
ignores `source=*` and no edges get the tag — the request fields then
silently no-op.

## Reference

- Plan + implementation details (six layers + three-axis API):
  [`docs/adventure-riding/PLAN.md`](https://github.com/epifanio/valhalla/blob/feat/adventure-riding/docs/adventure-riding/PLAN.md)
  in the Valhalla fork.
- Tile-build augmentation pipeline (per-country TET overlay generation):
  `scripts/tet/` in `epifanio/FastGIS`, branch
  `feat/tet-augmentation-pipeline`.
- Gurka tests (`gurka_adventure_riding`) pin the API behaviour: cost-bias
  endpoints, curve invariants, speed-factor scaling, and untagged-edge
  no-op.
