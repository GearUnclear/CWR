Fixed in repair 2026-10-11.3 (CWR commit [98bf49b](https://github.com/GearUnclear/CWR/commit/98bf49b596406796d4edc9244dc1bb88041e65a0) on main), rule module `loops`, applied to the installed @LoBo in backup batch `20261012T034354Z-0532e837`.

- Sea King `float.sqs` waits `~0.1`. When the helicopter dies or is deleted, the float plate is deleted and the script exits, so a destroyed Sea King is no longer held on the water.
- Dolphin arm waits `~1` and exits with its vehicle. MiG-25 gear, Syrian BMP-1 doors and F-5 canopy wait `~0.5`. The A-4 canopy waits `~0.5` instead of `~0.0001` and exits with its vehicle. The BMP-1 script gains its missing `#Exit` label.
- The same module also slows the LoBo_FAB flag loops, the CSJ airliner lights and the CSJ ILS helper, and fixes the CoC artillery MaxWind guard (audit items AG-1, AG-3, AG-4, AG-10).

Status: implemented, applied and offline verified. Synthetic tests with literal rule pins pass, the installed repeat check exits 0 with zero changes, and every changed member hash was checked. Not runtime verified: no game run exercised these vehicles. Still unchecked in game: the Sea King resting on water with the plate following, plate removal on death, the door, canopy, gear and arm animations, and a flat script count over time.

Ships in the [LoBo Reloaded 2026-10-11 release](https://github.com/GearUnclear/lobo/releases/tag/lobo-reloaded-2026-10-11).
