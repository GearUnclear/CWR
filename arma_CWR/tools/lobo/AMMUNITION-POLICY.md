# Lobo ammunition policy for issue #69

This is a reproducible Uslu dur! gameplay baseline, implemented by
`repair/ammo.py`. It fixes broken references and inconsistent engine inputs; it
is not a claim that legacy damage points measure real penetration, explosive
mass or lethal distance. The repair preserves existing public class identifiers,
magazine capacity, muzzle velocity, dispersion, reload time, cost and unrelated
values, except for the explicitly listed changes below. Review unknown locally
modified values instead of silently replacing them.

## Missing M833 projectile

The existing `CfgWeapons/LoBo_105Sabot2` magazine continues to select
`CfgAmmo/LoBo_105Sabot2`. The repair defines that previously absent class directly
under `Shell105`, with hit 400, indirect hit/range 0/0, explosive 0, cost 300, and
AI distance/probability points (10/.5, 1000/.85, 2000/.65). No model or sound
payload is copied into the repository; normal Shell105 inheritance supplies the
projectile's base behavior.

The 400-point choice is an explicit conservative balance baseline below the
pack's existing M900 projectile at 450. The AI envelope follows that existing
105 mm family. This is neither an alias to the M900 projectile nor a conversion
of an asserted historical penetration measurement. The existing M900 and other
tank ammunition values remain unchanged; balancing the entire armor/penetration
ladder is outside this repair. Compare the two rounds against identical tanks
and impact locations before claiming a shots-to-kill result.

## AI engagement profiles by role

Each tuple below is distance in metres / authored AI hit probability. These are
weapon evaluation inputs, not ballistic range limits. `HitProbability` in
`engine/Poseidon/World/Detection/TargetFire.cpp` interpolates between these points
and returns zero beyond the maximum.

| Projectile / use | Minimum | Midpoint | Maximum | Bundled JAM comparator |
| --- | ---: | ---: | ---: | --- |
| Original affected Lobo bullets | 1/.80 | 50/.35 | 109/.08 | None; pistol-like profile |
| `LoBo762Bullet`, FN MAG | 1/.90 | 100/.50 | 450/.05 | `JAM_Large_Mgun_Bullet` |
| New `LoBo556MGBullet`, Minimi/Negev | 1/.90 | 100/.50 | 450/.05 | `JAM_Medium_Mgun_Bullet` |
| `LoBo556Bullet`, CAR-15/M16/Galil rifle uses | .5/.10 | 50/.38 | 250/.04 | `JAM_Medium_Rifle_Single_Bullet` |
| `LoBo556Bullet_SD`, suppressed Negev | .5/.25 | 70/.50 | 250/.08 | `JAM_Medium_Silenced_Rifle_Burst_Bullet` (also inherited by Auto) |

The bundled `JAM_West_762x51_Mgun_Bullet` and
`JAM_West_556x45_MGun_Bullet` inherit the respective MG profiles. JAM's suppressed
200-round 5.56 magazine selects `JAM_West_556x45_Silenced_Auto_Bullet`, which
inherits the suppressed profile above. This deliberately retains that shorter
250 m envelope for the suppressed magazine.

`LoBo556Bullet` also serves ordinary rifles, so changing it globally to the MG
profile would erase a useful role distinction. A new derived MG class overrides
only the six range/probability keys and is selected by `LoBoM249Mag` and
`LoBoNegevMag`. Existing rifle references and their stable class names remain.
Their shared projectile cannot represent JAM's separate per-fire-mode profiles;
the rifle single-shot baseline is the conservative common profile. Lobo's
existing hit, noise, flash and cartridge values are preserved.

`LoBoNegevSDMag` previously selected the unsuppressed projectile. It now selects
the existing `LoBo556Bullet_SD`, preserving that class's authored suppressed
noise/visibility. At 300 m, the portable MG curve changes from zero to about
0.242857; at 451 m it is still zero. Rifle and suppressed curves remain zero at
300 m. These tests do not claim actual accuracy or that AI always chooses to fire.

## Ordinary projectiles and explosive projectiles

Both indirect hit and radius become zero on these ordinary ball/AP classes:

- M240 coax (including its inheriting M240C variant), .50 cal, DTM, PK, DShK, KPVT;
- the duplicated `loBo_DShK_Ter` projectile;
- ZPU-4 14.5 mm, 25 mm AP and 30 mm APFSDS.

The last 30 mm class already had zero indirect damage; zeroing its residual
radius makes the intent explicit. M240 and .50 cal gain explicit zero overrides
so inherited parent settings cannot reintroduce splash. Direct `hit` values,
tracer colors, ammunition IDs and AI ranges in this group remain unchanged.

Selection is by the authored ammunition role, not merely by a `Bullet` base class
or `explosive=0`. The engine's indirect damage path does not require the explosive
flag. Actual HE/HEI cannon ammunition, the M40 spotting round's intended flash,
and shotgun pellet approximations keep their authored effects. JAM itself uses
small indirect effects for bullets; matching its AI profile does not import those
effects. Existing tank AP shell splash/spall approximations were not swept into
this targeted ordinary-MG fix.

## Blast policy and numerical comparisons

`indirectHitRange` is the full-strength plateau input. `CalcDammage` in
`engine/Poseidon/World/Entities/Weapons/Dammage.cpp` is flat within that range and
falls as `(range / distance)^4` beyond it. `ExplosionDammage` in
`engine/Poseidon/World/Simulation/Collisions.cpp` obtains candidate objects using
four times that range, scales by an unshielded fraction, and dispatches damage.
Finite object bounds, hitpoints, armor and structural damage subsequently matter.
The spatial search is not a promised lethal radius; a curve sample is not the
result for a whole vehicle or human target.

| Family | Direct hit retained | Indirect strength retained | Plateau before -> after | Search input before -> after |
| --- | ---: | ---: | ---: | ---: |
| 60 mm HVMS HE/HEAT | 255 | 15 | 30 -> 4 m | 120 -> 16 m |
| 120 mm mortar | 230 | 45 | 8 -> 8 m | 32 -> 32 m |
| 160 mm mortar | 260 | 45 | 8 -> 8 m | 32 -> 32 m |
| `LoBo_C4` vehicle bomb | 1000 | 900 | 50 -> 10 m | 200 -> 40 m |
| `LoBo_Carbomb` vehicle bomb | 1000 | 500 | 50 -> 10 m | 200 -> 40 m |

**`LoBo_C4` is fired by `LoBo_C4Gun`, whose displayed role is Car Bomb. It is not
a handheld demolition charge.** The policy retains its strong contact damage,
uses a bounded 10 m plateau for both vehicle-bomb implementations, and leaves
their differently authored strengths intact. No explosive mass is given in the
source, so this is a gameplay bound, not a physical equivalence claim.

The 60 mm round has a smaller blast envelope and lower indirect strength than the
existing heavy mortars; its separately authored direct-hit/HEAT role is retained.
The original 120/160 mm mortar settings are deliberately retained as the reference
family, with validation that they still match 45/8. Changing them solely to make
numbers look different by caliber would add another unmeasured balance change.
Other tank/artillery HE families retain their authored profiles pending a separate
pack-wide study; issue #69's four named comparison rounds and the duplicated
vehicle-bomb implementation define this repair's blast scope.

For a mathematical point sample with armor 1 and no cover:

| Scenario | Before | After |
| --- | ---: | ---: |
| 60 mm at 0 m | 15 | 15 |
| 60 mm at 5 m | 15 | 6.144 |
| 60 mm at 10 m | 15 | .384 |
| Mortar at 10 m | 18.432 | 18.432 |
| C4 vehicle bomb at 20 m | 900 | 56.25 |
| Other vehicle bomb at 20 m | 500 | 31.25 |

`blast_comparisons()` produces 144 reproducible rows across distances
0/5/10/20/40/50 m, unshielded factors 1/.25/0, and armor factors 1/10. For example,
the C4 point sample at 20 m with .25 unshielded and armor 10 changes from 22.5 to
1.40625. Fully shielded samples return zero. The pure mathematical curve is also
reported beyond the search input for inspection; those rows do **not** imply
that the engine will visit a point-sized object there. Candidate search radii are
separate fields. No game process is used to generate these comparisons.

The module and tests deliberately avoid claiming a structural-damage result from
`hit / armor` alone. `armorStructural`, component geometry, impact placement and
cover are not encoded in these point samples. Later in-game testing should use
identical infantry, soft vehicles and tanks at fixed distances, both exposed and
behind the same cover, with identical impact points. That is a validation task,
not a prerequisite for reproducing this explicitly documented baseline.

## Mk82 / GBU-12 family

The GBU-12's warhead changes from 500/150/10 (direct/indirect/range) to
1000/500/15, matching both `LoBo_MK82` and `LoBo_MK82_LDGB`. These three values
come from one policy mapping in the repair; both reference variants are checked
before transforming the GBU-12. Guidance, accuracy/cost controls, flight behavior,
models and magazine counts remain unchanged. The shared Mk82 bomb-body rationale
was documented in issue #69 using the [USAF description](https://www.holloman.af.mil/News/Features/Display/Article/663837/quarterly-load-comp-april-10-2015/).

## Cluster delivery (2026-10-11.3)

Repair 2026-10-11.3 changes no CfgAmmo value. The A-4R, A-4RN, F-4 WW and
IL-28 AS Mk-20 scripts called a `rockeye.sqs` that the Dec 2006 consolidation
removed from LoBoF16 and LoBoF15, so those Mk-20s landed as one point HE bomb;
they now run the existing LoBo_airammo bomblet script (197 bomblets), like the
nine post-consolidation callers. The Syrian MiG-21bis RBK-500 script created the
undefined `LoBoRBKBomblet`; it now creates the existing `LoBoRBK500Bomblet`
(105 bomblets). The F-5 and IL-28 dispenser model is the defined
`LoBo_mk20Deployed`. These are class-name and path corrections, not aliases.

The KSVK ammunition's building-hit sounds named two files that never shipped.
They now point at the vanilla `weapons\bulletricochet4` and `bulletricochet5`
the class would inherit without its override, at the authored volume.

## Compatibility and verification

The Micro Uzi compatibility array names the existing `LoBoMUZIHDmag` instead of
the nonexistent `LoBoMUZIDDmag`. Display-only `12 Guage Shells` and `1225mm AP`
become `12 Gauge Shells` and `122mm AP`. Historical identifiers such as
`LoBo12GuageShell` are not renamed.

Run `python arma_CWR/tests/ci/test_lobo_ammo.py` from the repository root. It tests
reference resolution, role-specific AI behavior, non-explosive versus explosive
families, sibling warhead equivalence, bounded blast curves under cover/armor
factors, unchanged unrelated content, rejection of unknown values, and a no-op
second repair pass with LF and CRLF input. The full repair runner additionally
validates extracted real configs. Do not commit extracted addon configs or PBOs;
only the original transformation code, synthetic tests and documentation belong
in this repository.
