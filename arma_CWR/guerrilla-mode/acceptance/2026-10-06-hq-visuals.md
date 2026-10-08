# HQ and cache visuals — 2026-10-06

The Classic HQ now uses a map desk (`stulsuplikmapa.p3d`) and single crate
(`bedna_ammo.p3d`) indoors, or the stock open `CampEmpty` tent and camouflaged
crate stack (`hromada_beden.p3d`) outdoors. The global `guerrilla-hq.hpp`
supplement creates empty cache classes derived from `WeaponHolder`; campaign
scripts supply the inventory. Empty caches persist without refilling.

Indoor siting checks supported floor, overhead cover and wall clearance near
building path points. Terrain-cell scanning finds the same houses after a
reload. Moving between layouts replaces the cache class while transferring
its weapons and actual magazine objects, preserving partial ammo counts.
Older saves receive the visuals on simulation without increasing move count.

Validation against Classic 1.99 data:

- Clang RelWithDebInfo `PoseidonGame` and `PoseidonTests` build passed.
- `[guerrilla][base]`: 67 assertions across 8 cases passed.
- Runtime installer regression passed, including both config includes and
  repeat-install idempotence. Installed HQ header matches the source hash;
  installed shared scripts match their source files.
- Trident: `guerrilla_hq_visuals.seq`, `market_reference_mission.test.sqf`
  and `guerrilla_native_save_reload.seq` all passed in the final run.
  The new sequence covers indoor/outdoor moves, fresh-process reloads,
  empty-cache persistence, inventory counts and removal of old props.
- Synthetic legacy-format save: an invisible `WeaponHolder` upgraded to the
  indoor crate and desk, retained two AK47s and three magazines (one with
  seven rounds), kept one stash registration and zero HQ moves. A subsequent
  serialized save confirmed the seven-round magazine survived.
- In-engine visual captures are available locally under
  `tmp/hq-final-preview/`: `000_indoor_1.png`, `001_indoor_2.png`, and
  `002_outdoor.png`. Integration output is under `tmp/hq-final-regression/`.

One earlier market run timed out opening the vehicle dealer menu; the final
combined run passed all three suites. Multiplayer and LoBo placement were
not exercised in this acceptance pass.
