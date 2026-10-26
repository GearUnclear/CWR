Fixed in repair 2026-10-11.3 (CWR commit [98bf49b](https://github.com/GearUnclear/CWR/commit/98bf49b596406796d4edc9244dc1bb88041e65a0) on main), rule module `clusters`, applied in batch `20261012T034354Z-0532e837`.

- The A-4R and A-4RN (`firemissiles_A2.sqs`, `firemissiles_A3.sqs`), F-4 WW (`firemissiles_WW.sqs`) and IL-28 AS (`fire_mk20.sqs`) now call the consolidated `\LoBo_airammo\Scripts\cbu\rockeye.sqs`. No script still calls the old LoBoF16 or LoBoF15 path. The IL-28 fired script also gains the .1 projectile owner guard.
- The F-5 and IL-28 rockeye copies now create the defined `LoBo_mk20Deployed`. No alias class was added.

Status: runtime verified. Run `mk20-03` (UD engine, official Remaster data, installed @LoBo): the A-4R, F-4 WW and IL-28 AS each opened one `LoBo_mk20Deployed` and released at least 194, 194 and 187 of 197 bomblets (the probe samples about every 10 frames, so these are lower bounds). Limits: only the script path was tested, with the aircraft held still and invulnerable at 150 m, so ballistics, spread and damage are unchecked. The A-4RN and F-5 paths were not flown; the F-5 class rename is offline verified only.

Ships in the [LoBo Reloaded 2026-10-11 release](https://github.com/GearUnclear/lobo/releases/tag/lobo-reloaded-2026-10-11).
