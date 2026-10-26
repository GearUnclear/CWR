Fixed in repair 2026-10-11.3 (CWR commit [98bf49b](https://github.com/GearUnclear/CWR/commit/98bf49b596406796d4edc9244dc1bb88041e65a0) on main), rule module `mines`, applied in batch `20261012T034354Z-0532e837`.

- All 17 mine scripts in LoBo_Mines and JOF_Objects1 poll every 0.1 s instead of every frame. By the spec's estimate that still catches about 99% of top-speed AP crossings and keeps AT detonation lag under 2 m at 70 km/h.
- The flare tripwire is only about 1 m wide, so it polls every 0.03 s (about the frame rate its author tuned for). The suggested 0.25 s would make it unreliable.
- Each loop exits when its mine is deleted (`isNull`), so a deleted mine can no longer fire a ghost explosion.
- The flare mine deletes its `_Mess` helper on trigger, pickup and deletion.

Status: implemented, applied and offline verified, with the same offline checks as #2. Not runtime verified: no game run placed or triggered mines. Still unchecked in game: AP mine walk, run and sprint; AT mine drive-over at speed; naval mine; flare tripwire on both sides; claymore; no ghost explosion after deletion; flare helper gone. Not part of this fix: in multiplayer every client still runs every mine (a locality redesign, separate from the polling cost).

Ships in the [LoBo Reloaded 2026-10-11 release](https://github.com/GearUnclear/lobo/releases/tag/lobo-reloaded-2026-10-11).
