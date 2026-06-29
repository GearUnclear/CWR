# Bullet penetration through soft cover (wallbang)

*Generalize the existing glass pass-through into a material-driven penetration system so rifle rounds punch through wood, foliage, and thin metal for reduced damage behind cover.*

## Summary

Today a bullet (`ShotShell` / `ShotBullet`) stops dead at the first solid object it hits, with one hard-coded exception: glass, which it flies through while still damaging (shattering) the pane. This mod **generalizes that exception** into a real penetration model:

1. When a round hits a surface, classify the impacted material as **penetrable** or **solid** using the surface material data already attached to every `Texture` (`Roughness()` / `Dustness()`).
2. For a penetrable hit, apply (reduced) damage to that object, **subtract a per-material energy cost** from the round's remaining hit budget, keep the projectile alive, and continue the trace.
3. Stop (and apply a final hit + explosion as today) when the round meets a *solid* surface, or when its remaining hit budget drops below a configurable threshold.

Player-visible / observable result: fire a rifle at a soldier crouched behind a wooden fence, plank wall, or bush and the target takes (reduced) damage instead of being perfectly safe; fire at a concrete wall or a tank and the round still stops exactly as before. AI behind soft cover becomes vulnerable, which materially changes firefight dynamics around hedgerows and wooden structures in Everon/Malden.

## Why it's interesting

- It is a **pure-sim, AI-relevant** change: suppressing or killing enemies through light cover is a behaviour the AI already "wants" (it shoots at last-known positions), and this makes those shots actually connect. No new art, no new models.
- It reuses two fields (`_roughness`, `_dustness`) that are **already parsed from `CfgSurfaces` and already stored on every texture** but are currently only consumed by terrain physics — so the data plumbing exists and is free.
- The hook point is tiny and well isolated: the glass special-case is a ~40-line block inside one function.

## Difficulty & prerequisites

**Intermediate.** You need to understand the collision-resolution loop in `ShotShell::Simulate` and add a small amount of per-instance state to the projectile. No threading, no networking redesign (damage is already gated on `IsLocal()`).

**No art assets are required.** This is C++ engine work plus optional `CfgSurfaces` / `CfgAmmo` config keys that reuse existing classes. No new textures, models, P3D, PAA, or sounds. Existing surface material values ship with the game data.

## Key files & entry points

- `engine/Poseidon/World/Entities/Weapons/Shots.cpp`
  - `ShotShell::Simulate(float, SimulationImportance)` — **lines 647-791**, the core loop. The glass special-case lives at **lines 680-748**:
    - `Texture* glass = GPreloadedTextures.New(TextureBlack);` (680) and `bool detectGlass = dyn_cast<ShotBullet>(this) != nullptr;` (681)
    - first loop (683-697) picks `minI` = nearest **non-glass** colliding object;
    - second loop (700-720) applies `info.object->DirectLocalHit(info.component, Type()->hit)` to glass-only surfaces (717);
    - `minI >= 0` block (722-748) does the terminal `GLandscape->ExplosionDammage(...)` and `_delete = true`.
  - `ShotBullet::Simulate` — **lines 793-797** (calls `base::Simulate`, then `SetEnd`). `ShotBullet` is the rifle/MG round subclass; `detectGlass` is gated to it.
  - `ShotShell` ctor — **lines 602-627** shows the established pattern for reading optional per-ammo config: `type->GetParamEntry()` then `pars.FindEntry("coefGravity")` / `"timeToLive"`. Reuse this for a `penetration`/`penetrationPower` key.
- `engine/Poseidon/World/Entities/Weapons/Shots.hpp`
  - `class ShotShell` — **lines 121-139** (add penetration state here).
  - `class Shot` — **lines 12-59** (add a virtual `HitCoefficient()` accessor here so the damage path can read it).
- `engine/Poseidon/World/Scene/Object.hpp`
  - `struct CollisionInfo` — **lines 867-880**: fields used are `Texture *texture` (869), `Ref<Object> object` (872), `float under` (877, immersion depth ≈ traversed thickness), `int component` (879), `Point3 pos` (870), `Vector3 dirOut` (871).
- `engine/Poseidon/Graphics/Textures/TextureBank.hpp`
  - `Texture::Roughness()` / `SetRoughness()` — **lines 82-83**; `Texture::Dustness()` / `SetDustness()` — **lines 85-86** (the `// for physical simulation` fields at 23-24).
  - `struct SurfaceInfo` — **lines 109-119** (`_roughness`, `_dustness`).
- `engine/Poseidon/Graphics/Textures/TextureBank.cpp`
  - `AbstractTextBank::AbstractTextBank()` — **lines 361-389** parses `CfgSurfaces` entries: `info._roughness = entry >> "rough";` (376), `info._dustness = entry >> "dust";` (377).
  - `Texture::SetName` — **lines ~232-238** copies `surface._roughness` / `_dustness` onto each texture from `GetSurface(name)`.
  - `AbstractTextBank::GetSurface` — **lines 547-579**, falls back to a `"default"` surface, else a zeroed `SurfaceInfo`.
- `engine/Poseidon/World/Entities/Weapons/Dammage.cpp`
  - `Object::DirectLocalHit(int component, float val)` — **lines 186-189** (base returns `1`; overridden per object type).
- `engine/Poseidon/World/Simulation/Collisions.cpp`
  - `Landscape::ExplosionDammage(...)` — **lines 1068-1177**. Direct hit damage is `float hitVal = type->hit - type->indirectHit;` then `directHit->DirectDammage(shot, owner, pos, hitVal);` (**1081-1083**); indirect ring uses `type->indirectHit` (1108). This is the single place to scale damage by the round's remaining budget.
- `engine/Poseidon/World/Entities/Weapons/Weapons.hpp`
  - `AmmoType::hit` — **line 46** (`float hit,indirectHit,indirectHitRange;`). This is the base energy; it is `const` on the live `Shot` (accessed via `Type()`), so the running budget must live on the projectile instance, not the type.

## How it works today

In `ShotShell::Simulate` (Shots.cpp:670-749), after integrating motion, the code calls `GLandscape->ObjectCollision(collision, this, _parent, Position(), position, 0)` to gather every object the segment crosses this tick. It then:

1. Builds a sentinel `glass` texture (`GPreloadedTextures.New(TextureBlack)`) and sets `detectGlass` true only for `ShotBullet`.
2. Loops to find `minI`, the nearest collision **whose texture is not the glass sentinel** (glass is skipped, so it never becomes the stopping point).
3. In a second loop (local-only), for collisions that **are** glass it calls `DirectLocalHit(info.component, Type()->hit)` — i.e. the pane gets damaged/shattered — and the bullet is allowed to keep going.
4. If `minI >= 0`, it treats that nearest non-glass object as the terminal hit: optionally spawns an `Explosion` (when `Type()->hit > 50`), calls `ExplosionDammage(...)` with the **full** `Type()`, sets `_delete = true`, and returns.

So penetration is currently **binary and texture-identity-based**: exactly one material (the glass sentinel texture) is penetrable, the round loses no energy passing through it, and everything else is an instant wall. The surface `Roughness`/`Dustness` material data exists on every texture but is not consulted here at all.

## Implementation approach

The strategy: replace the "is this the glass texture?" test with a "how much does this material cost to penetrate?" function, and carry a running energy budget on the projectile.

1. **Add per-instance penetration state to `ShotShell`** (Shots.hpp, class at 121-139):
   - `float _hitBudget;` — remaining hit, initialized in the ctor (602-627) to `type->hit`.
   - `float _hitCoef;` — convenience = `_hitBudget / type->hit`, used by the damage path; or compute on demand.
   - Optionally `float _penetrationPower;` read from config (see next section). Initialize alongside the existing `FindEntry("coefGravity")` block.

2. **Expose the coefficient to the damage path.** In `Shot` (Shots.hpp:12-59) add:
   ```cpp
   virtual float HitCoefficient() const { return 1.0f; }
   ```
   Override in `ShotShell` to return `_hitCoef` (clamped to `[0,1]`). This keeps `ExplosionDammage`'s signature unchanged.

3. **Scale damage in `Landscape::ExplosionDammage`** (Collisions.cpp:1068). The function already has `shot`. Change:
   ```cpp
   float hitVal = type->hit - type->indirectHit;
   ```
   to multiply by `shot ? shot->HitCoefficient() : 1.0f`, and likewise scale the indirect ring `hit` (line 1108). For non-penetrating shots the coef is `1.0` so behaviour is byte-for-byte identical.

4. **Write a material classifier** (free function in Shots.cpp, near the top of the anonymous logic). Signature roughly:
   ```cpp
   // returns energy cost as a fraction of full hit, or <0 if the surface is solid (stops the round)
   static float PenetrationCost(const CollisionInfo& info, float penetrationPower);
   ```
   - Pull `Texture* tex = info.texture;` then `float rough = tex ? tex->Roughness() : 1e6f;` and `float dust = tex ? tex->Dustness() : 0;`.
   - Heuristic to start (tune later): treat **low roughness AND/OR high dustness** as soft/penetrable (wood, cloth, foliage, glass), **high roughness** as hard (rock, concrete, metal armour). Glass keeps working because the existing glass sentinel still has its surface entry; you can also keep the explicit `info.texture == glass` check as a guaranteed-penetrable fast path during migration.
   - Scale the base cost by `info.under` (immersion depth ≈ traversed thickness) so a thick beam costs more than a thin plank, and divide by `penetrationPower` so high-power rounds (e.g. `.50`, AP) punch deeper.
   - Return the per-hit fractional cost; return a sentinel (e.g. `-1`) for solid materials.

5. **Rewrite the collision block (680-748)** so it is energy-driven instead of glass-driven:
   - Iterate collisions ordered by `info.under` (ascending = nearest first). Reuse/extend the existing `minT`/`minI` scan to produce an ordered traversal rather than a single minimum.
   - For each hit in order:
     - `float cost = PenetrationCost(info, penetrationPower);`
     - If `cost < 0` (**solid**): this is the terminal hit. Do exactly what the current `minI >= 0` block does (explosion if `Type()->hit > 50`, `ExplosionDammage`, `_delete = true`), then `return`.
     - Else (**penetrable**): if `IsLocal()`, apply damage to this object — reuse the existing `info.object->DirectLocalHit(info.component, Type()->hit * _hitCoef)` call and/or a localized `ExplosionDammage` with `directHit = info.object`. Then `_hitBudget -= cost * Type()->hit; _hitCoef = _hitBudget / Type()->hit;`.
     - After subtracting, if `_hitBudget < kStopThreshold` (config; e.g. a few hit-points or a fraction of base), spend the last of the energy on this object as a terminal hit and `_delete = true; return;` — the round dies inside the cover.
   - If the loop finishes with budget remaining, **do not set `_delete`**: fall through to the existing ground-intersection test (751-782) and `Move(position)` (790) so the round continues along its trajectory into the next tick. The penetrated objects are now behind it.

6. **Keep the `ShotBullet`-only gate.** The current `detectGlass` is `dyn_cast<ShotBullet>(this) != nullptr`. Preserve that idea: only enable generalized penetration for `ShotBullet` (rifle/MG rounds), so grenades/shells (`ShotShell` non-bullet, `Missile`) keep stopping-and-exploding behaviour. Cleanest: make the penetration branch conditional on `dyn_cast<ShotBullet>(this)`, exactly mirroring the existing guard.

7. **Network correctness.** All damage is already inside `if (IsLocal())`. The projectile is simulated identically on the owner; remote clients only see the tracer. Continuing the trajectory does not add new network messages, so no `NetworkMessageFormat` changes are needed.

## Config / data / SQF touchpoints

All optional — the mod works with sensible hard-coded defaults — but these make it tunable without recompiling:

- **`CfgAmmo` per-ammo key (reuse existing parse pattern).** In the `ShotShell` ctor (Shots.cpp:611-626) add a `pars.FindEntry("penetrationPower")` read, defaulting to `1.0`. Modders then set e.g. `penetrationPower = 3;` on `B_762x51_Ball`-style classes to make AP/MG rounds punch deeper. No new config file — just a new key on existing ammo classes.
- **`CfgSurfaces` reuse.** Penetrability is derived from the existing `rough` / `dust` keys already on every surface (TextureBank.cpp:376-377). A modder can make a specific material softer simply by lowering its `rough` — no engine change needed once the classifier is in. Document the mapping you choose (roughness→hardness).
- **Global toggle.** Add a `bool` to the engine/user config (see `EngineConfig` / `UserConfig` already included at the top of Shots.cpp) such as `bulletPenetration`, so the whole feature can be disabled for vanilla-accurate servers. Default on for the mod.
- **SQF observability.** No new script commands required, but the change is directly testable from SQF/Trident by spawning a target behind a known object and reading `getDammage` / `damage` before and after firing (see Testing).

## Risks & gotchas

- **`Type()->hit` is const.** You cannot decrement it; the running budget must live on the `Shot` instance (`_hitBudget`). Do not try to mutate the shared `AmmoType`.
- **`ExplosionDammage` signature.** Prefer the `Shot::HitCoefficient()` virtual approach over adding a parameter, to avoid touching the network twin `GetNetworkManager().ExplosionDammageEffects(...)` (Collisions.cpp:1177) and `ExplosionDammageEffects` (877).
- **Heavy intentional warning suppression.** Per `CLAUDE.md`, ~50 warning classes are disabled on purpose. Do not "fix" surrounding patterns (e.g. the tautological/defensive compares or the intentionally incomplete `switch`es in this file) while you are in here — only touch the penetration path.
- **`ClassIsMovableZeroed` / memcpy-movable types.** `CollisionInfo` holds a `Ref<Object>` (Object.hpp:872) and `CollisionBuffer` is a `StaticArray<CollisionInfo>`. If you reorder or copy collisions into a local array for ordered traversal, copy by value through the normal copy ctor — do **not** `memcpy` a `Ref<Object>` (it would corrupt the refcount). The bitwise-movable optimization applies only to types tagged for it; `CollisionInfo` carries a smart pointer, so treat it as a real object.
- **clang-cl vs GNU `__FILE__` asymmetry.** Not directly relevant here, but if you add asserts/logs that print paths, remember `__FILE__` is absolute on clang-cl and repo-relative on GNU-driver clang.
- **Per-target PRIVATE PCH.** `Shots.cpp` is compiled into `Poseidon.lib`, a `POSEIDON_PCH_TARGETS` member. Adding includes is fine; if you build with `POSEIDON_DISABLE_PCH=ON` to audit, make sure the new header (`TextureBank.hpp` for `Roughness()`/`Dustness()`) is included explicitly in Shots.cpp — currently it relies on transitive includes.
- **`FileSize` target.** `Shots.cpp` is ~1600 lines; the `FileSize` target warns at >3000 and errors at >5000, so this addition is well within budget, but keep the new classifier compact (or put it in a small helper) rather than inlining a huge tuning table.
- **Tuning trap.** Roughness/Dustness values were authored for footstep/dust physics, not ballistics — expect to spend most of the effort tuning the classifier thresholds and the per-thickness cost so concrete still stops rounds and bushes do not. Start conservative (only the softest materials penetrable) and widen.
- **Performance.** The loop already runs per-tick over `collision.Size()`, which is small. Ordered traversal adds a sort of a tiny array — negligible. Do not call `GPreloadedTextures.New` every tick if you drop the glass sentinel; cache it as today.

## Testing

- **Build:** `cmake --preset win-x64-clang-rwdi` then `cmake --build build/win-x64-clang-rwdi` (link target: `Poseidon.lib`, then a client app such as `cwr/GameDemo`).
- **Unit (Catch2, `tests/unit/`):** the natural home is `PoseidonTests`. Add a focused test for the classifier function (`PenetrationCost`) and the budget arithmetic in isolation — feed it synthetic `Texture` objects with `SetRoughness`/`SetDustness` (TextureBank.hpp:83/86) and assert soft→positive-cost, hard→negative sentinel, and that successive penetrable hits monotonically reduce `_hitBudget` until it crosses the stop threshold. Run with `ctest --test-dir build/win-x64-clang-rwdi -R PoseidonTests --output-on-failure`. Keep the classifier free of global state so it is unit-testable without a `World`.
- **Integration (Trident, `tests/integration/`):** build the runner (`cargo build --manifest-path engine/Trident/Cargo.toml`), configure `.trident.env` with `OFPR_GAME_DIR` / `OFPR_DATA_DIR`, then `tri test -j6 --retries 2 tests/integration`. Author an SQF scenario: place a unit behind a wooden object (penetrable) and behind a concrete wall (solid), have a second unit fire a burst, then assert via `damage`/`getDammage` that the wood-covered unit took damage and the concrete-covered one did not. This is the definitive "wallbang works, walls still work" check and is exactly the kind of sim assertion Trident exists for.
- **In-game manual:** run the `win-x64-clang-rwdi` build, spawn the same two-cover setup in the editor, and fire. Expect: visible (reduced) hit/kill through fences and bushes, unchanged stop-and-spark on rock/concrete/armour. Toggle the global config flag off to confirm vanilla behaviour returns.

## Scope estimate

- **Core engine change:** ~0.5-1 day. The edited region is one function (~70 lines) plus a small classifier, two new `ShotShell` members, one `Shot` virtual, and a two-line scale in `ExplosionDammage`.
- **Tuning the classifier against real surface data:** ~1-2 days of iteration in-game — this is the bulk of the work and the part that decides whether it feels good.
- **Config + tests:** ~0.5 day.

**Suggested minimal first slice (fast visible win):** keep `detectGlass`'s exact structure but replace the single `info.texture == glass` predicate with `IsPenetrable(info.texture)` where `IsPenetrable` returns true for the glass sentinel **plus** any texture below a roughness threshold. Apply a flat 50% damage reduction (hard-coded `_hitCoef = 0.5f` for any round that has penetrated at least once) via the new `HitCoefficient()` virtual, and skip the energy-budget bookkeeping entirely. That already produces an observable wallbang through wood/foliage with reduced damage in a single afternoon; the per-material cost and stop-threshold can follow once the basic effect is proven on screen.
