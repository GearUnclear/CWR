# LoBo component armor and bundled diver repairs

The issue 69 repair chooses the issue's supported-component option. It repairs
real model bindings and makes the existing damage controls explicit. It does
**not** add a directional penetration simulator or independently persistent
front, rear, side, or roof armor states.

## Why registering the old classes would not suffice

`TankType::InitShape` registers hull, turret, gun and two tracks;
`TankOrCarType::InitShape` registers the engine. `EntityAI::LocalHit` looks up
their named selections in the Hitpoints LOD and accumulates component damage
from its points. `HitPoint` multiplies component armor by selection point count;
`LocalHit` still returns the existing structural damage coefficient. Merely
registering more classes would change the saved/networked damage-array layout,
would not automatically route their damage to operational tank components, and
would not provide directional structural protection.

Inspection of the actual bundled P3Ds found problems beyond the original audit:

| Model family | Verified Hitpoints selections | Old content problem |
| --- | --- | --- |
| Four Egyptian M1 variants | `Hull`, directional hull sets, `Turret`, directional turret sets, `LTrack`, `RTrack` | Config used `turet`, `pasL`, `pasP`; all three bindings were missing. Directional classes were ignored. |
| Merkava Mk1 and all three Mk4 models | `hull`, `turet`, `pasL`, `pasP` | None of the configured `fhull`, `turetf`, `turett`, `turetb` selections exists. |
| Merkava Mk2 and Mk3 | `hull`, `fhull`, `turetf`, `turett`, `turetb`, `pasL`, `pasP` | No whole-turret `turet` selection exists, and the directional classes were ignored. |

The configured front/top/rear armor classes and their redundant scalar fields
are removed from the repaired source. The inherited Mk1/Mk2/Mk2C definitions
are repaired too, so the Mk4 cannot inherit misleading unused settings.

## Supported baseline and region coverage

The repair preserves each vehicle's overall `armor`, `armorStructural`, engine,
gun and track component coefficients. It preserves the M1's existing hull and
turret coefficients, both **0.8**, and its track coefficients of **0.6**. It
supplies both missing track selection aliases. Merkava hull armor remains **0.80**, while all
Merkava variants receive an explicit registered `HitTurret` coefficient of
**1.0**. This is a documented gameplay baseline within the old authored turret
range (rear **0.8**, top **1.0**, front **1.25**), not a measured armor thickness
or a promise to reproduce those three directional values. The actual model
geometry and the original spatial damage falloff still affect impacts.

For M1 and Mk2/Mk3 models, the regeneration script repairs selections in the
**Hitpoints LOD only**, retaining the configured legacy component names:

- Existing `Hull`/`hull` membership becomes the union of the existing whole-hull
  and directional hull sets; its original tag name and index do not change.
- A `turet` alias is appended containing the union of the existing whole-turret
  set, when present, and all authored directional turret sets.
- M1 additionally gains `pasL`/`pasP` aliases of its `LTrack`/`RTrack` selections.

Shared vertices are selected once, never duplicated. The M1 union contains
30 raw hull points and 41 turret points. Mk2/Mk3 contain 17 raw hull points and
12 turret points; the standalone asset reader expands the hull to 24 mesh
points because of its face-vertex representation. These counts are evidence of
region coverage, not independent hitpoint-state counts.

All old selection names and order stay intact. The only existing bytes changed
are the intended hull selection's membership payload; missing aliases are
appended. The script preserves all other original bytes, including vertex
positions, normals, faces, materials, original directional/turret/track
selections, animation tags, collision LODs, resolution values, and the legacy
32-byte texture-path trailer.
Existing turret animations transform the same original point coordinates.
The other LODs need no new names: `LocalHit` consumes the Hitpoints LOD, while
collision and rendering retain their original material and animation bindings.

There are still the same six registered tank components in the same order.
The configured script-facing labels (`hull`, `turet`, `pasL`, `pasP`) are
preserved: `HitpointName` exposes the model selection name to `setHit` and
`Dammaged` scripts, so new prefixed names would have broken external scripts.
No engine source, save format, network messages, class identifiers, loadouts,
or persistent damage-array shape changes. The intended behavior change is that
previously ignored hull/turret regions now feed real component damage, and M1
turret/track damage binds to real selections. Existing saved damage values keep
their component meaning. All multiplayer peers should install the same content
repair because damage response depends on these local models/configs.

## CoC combat swimmers

`CoC_Diverdes` and `CoC_DiverSaboteurj` now use the parent swimmer's bundled
`\CoC_Diver\CoC_Diver.p3d`, replacing unavailable `\seals\KMMwood*.p3d` assets.
The replacement is verified to contain the infantry body/injury selections,
`divemask`, and the two-point `divemask_axis` in its Memory LOD. Both retain
their class identities, displayed role names, movement configuration, swim
handlers, weapons and magazines; the demolition variant keeps its pipe bomb.
They share the parent swimmer appearance. Missing external woodland outfits
are not recreated or silently claimed to be present.

## Offline validation and remaining limits

`python tests/ci/test_lobo_models.py` uses synthetic MLOD/config data to verify
union membership, shared-point de-duplication, preservation of non-target bytes,
unchanged non-Hitpoints LODs/trailer, idempotence, and rejection of missing,
duplicate, conflicting, malformed or unsupported model data. It also verifies
correct component names, unchanged identities/loadouts, removal of unsupported
armor definitions, and Mk1/Mk2/Mk4-specific bindings.

During development, all six repaired models were separately read by
`PoseidonTools model inspect --selections` and the four configs were repaired
and reprocessed without a second change. All four repaired configs also parsed
successfully with `PoseidonTools config tojson`. This tool does not launch the game.
The installer must validate that config and paired model repairs are present
together. Runtime combat, animated impacts, save loading and multiplayer tests
have not been run; no game was launched for this work. The preserved layout is
a compatibility argument, not a claim of runtime verification.
