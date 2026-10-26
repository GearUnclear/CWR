Fixed in repair 2026-10-11.3 (CWR commit [98bf49b](https://github.com/GearUnclear/CWR/commit/98bf49b596406796d4edc9244dc1bb88041e65a0) on main), rule module `clusters`, applied in batch `20261012T034354Z-0532e837`. `LoBo_airammo\Scripts\cbu\rbk500.sqs` now creates the defined `LoBoRBK500Bomblet`; no alias class was added, and no script still names `LoBoRBKBomblet`.

Status: runtime verified. Run `rbk500-01` (UD engine, official Remaster data, installed @LoBo): the Syrian MiG-21 CAS1 fired the RBK-500 through its installed fired handler, the dispenser opened at about 47.6 m (the script releases at 50 m), and at least 100 of the script's 105 bomblets were seen. A bomblet created directly first proved the counter could see them. Limits: script path only, aircraft held still and invulnerable at 150 m; ballistics, spread and damage are unchecked.

Ships in the [LoBo Reloaded 2026-10-11 release](https://github.com/GearUnclear/lobo/releases/tag/lobo-reloaded-2026-10-11).
