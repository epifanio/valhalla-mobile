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

## Recommended presets

The three knobs aren't intuitive in isolation — the relationship between cost
ratio and route choice is geometry-dependent. The presets below come from a
5-fixture sweep on real Norway + Sweden TET data (commit log
[`5b8c4ca0b…71d166d9f`](https://github.com/epifanio/valhalla/commits/feat/adventure-riding)
in the Valhalla fork, server-validated 2026-05-26):

| preset | `use_adventure_riding` | `use_adventure_riding_curve` | route behaviour |
|---|---|---|---|
| **Off** | unset (or `1.0`) | unset (or `1.0`) | Stock routing. Trail edges are neither preferred nor avoided beyond their `highway=track` defaults. |
| **Light** | `0.5` | `3` | Trail wins when it's naturally convenient (similar length to the road alternative). Stays on the road for big detours. |
| **Moderate** | `0.3` | `3` | "I want some trail today" — flips onto the trail on every fixture in the sweep, including 100+ km routes. |
| **Strong** | `0.1` | `2` | Trail dominates the route choice whenever the trail is reachable from both endpoints. Use when the rider wants maximum off-road. |
| **Trail-only** | `0.0` | `1` | Cost on trail edges drops to zero — the router prefers trail wherever there's a valid path. Legacy "binary" mode. |

For `adventure_riding_speed_factor` (the rider-skill axis) the default `1.0`
is calibrated assuming the augmentation pipeline's per-country `maxspeed`
tag is realistic. Rider-side adjustments:

| skill | meaning |
|---|---|
| `2.0` | Expert / racer pace on tracks |
| `1.5` | Above-average — quick on grade2-3 |
| `1.0` | **Default** — average tour rider at the tagged speed |
| `0.7` | Cautious; technical terrain or heavier bike |
| `0.5` | Beginner / loaded touring bike |

Skill scales both wall-clock time and cost on trail edges, so the routing
choice shifts too — a cautious rider may avoid a trail detour the average
rider would take. Setting skill = `0.5` plus a Moderate preset is a useful
way to bias *toward* trail while still accepting "this section is too rough
for me" trade-offs.

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

## The dirt-first axis (`use_dirt_first`, 0.11.0+)

A fourth option, independent of the three above: **route on unpaved
surfaces, tarmac only as a connector**. Unlike the adventure-riding trio it
keys on the per-edge `Surface` field every tile already carries — **it works
on stock tiles, no augmented rebuild needed**, including every offline pack
already on devices.

| option | range | default | effect |
|---|---|---|---|
| `use_dirt_first` | 0.0 – 1.0 | 0.0 (off) | Strength of the paved/unpaved preference inversion. At 1.0 paved edges cost ~6x, gravel/dirt ~0.3x, and motor profiles floor unpaved edge speeds (30 km/h dirt/gravel, 15 path) so 2 km/h default-speed tracks become routable with honest ETAs. Out-of-range snaps to off. |

Always send the companions — without them stock track-avoidance suppresses
the flip: `use_tracks: 1.0` (all costings) and `use_trails: 1.0`
(motorcycle). Calibrated presets (NO+SE fixtures, 2026-08-24): Light 0.35,
Moderate 0.6, Strong 1.0. Composes multiplicatively with
`use_adventure_riding` — "ride the TET, dirt-first" gets both discounts.
Details: [`docs/adventure-riding/DIRT_FIRST.md`](https://github.com/epifanio/valhalla/blob/feat/adventure-riding/docs/adventure-riding/DIRT_FIRST.md)
in the fork.

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
