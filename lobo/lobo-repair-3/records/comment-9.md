Implemented in repair 2026-10-11.3 (CWR commit [98bf49b](https://github.com/GearUnclear/CWR/commit/98bf49b596406796d4edc9244dc1bb88041e65a0) on main), rule module `effects`, applied in batch `20261012T034354Z-0532e837`, with follow-up guards in batch `20261012T042613Z-b68ba5de`.

- The question is answered: `M1Smoker`, `M60Smoker`, `Smoker` and the helicopter scripts' `KioGlowFire` were defined nowhere (all @LoBo configs, @LoBo_UD, @LoBo_Deps, the Classic and Remaster base configs).
- LoBo_airammo's config now defines `LoBo_WreckGlow`, an invisible vanilla `Fire` (model `empty`), with all four classes under it.
- Consequence: defining `KioGlowFire` makes the BAS helicopter wreck burn live for the first time. As authored it would hit any player within 10 m, seated or not, with +0.04 every 0.25 s. The second batch gives the five helicopter `fire.sqs` copies (LoBoEghelo `Engine`, `Engine2`, `Engine3`; LoboIDFhelo `CH53\Engine`, `UH1\Engine`) the ground scripts' policy: the wreck owner runs the burn, only a local, living player on foot is hit, at most 0.04 per 10 s.
- Known side effect: the glow is a vanilla Fireplace, so a soldier on foot within 5 m who faces it is offered "Put out fire". No config key suppresses it.

Status: partly runtime verified. Run `glow-01` (UD engine, official Remaster data, installed @LoBo): all four classes create objects, and the killed handlers of a Toyota (`M1Smoker`), a Merkava2 (`M60Smoker`) and a Mi-8 (`KioGlowFire`) each made exactly one burning glow 1 to 3 m from the wreck. Run `glow-unlisted-01`: in a mission that does not activate `lobo_airammo`, each glow logs one `Access denied` warning and is still created.

Remaining runtime checks before closing:
1. A player seated in a vehicle next to a burning helicopter wreck takes no burn.
2. A player on foot within 10 m of it gets one MEDIC line and at most 0.04 damage per 10 s.
3. The "Put out fire" action at a wreck glow: confirm it appears, and decide whether it is acceptable.
4. The glow is removed by the 150 s cleanup.
5. The `say "fire"` sound and the light are present, and nothing visible (no campfire mesh) sits at the glow. The wreck screenshots so far show only smoke.
6. Optional: decide whether UD should preload `lobo_airammo` to silence the `Access denied` warning.

Ships in the [LoBo Reloaded 2026-10-11 release](https://github.com/GearUnclear/lobo/releases/tag/lobo-reloaded-2026-10-11).
