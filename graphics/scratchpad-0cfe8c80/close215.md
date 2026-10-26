Landed on `graphics-remaster` as fec1075, "graphics: fires, flares and shell bursts light the men and cast shadows".

- The shelved work was rebased onto cb3dd0d in a scratch copy, passed independent review, and was committed by the serialized committer (52 files).
- Time-acceleration blocker: combat lights now age by the simulation step, capped at `kCombatMaxTickStep` (0.25 s) per tick, instead of skipping long steps. Under `setAccTime 4` at a low frame rate, bursts still end and the list no longer grows.
- Not yet tested in game: the time-acceleration fix during a 4x Showcase run. The final full check on the branch and the comparison video are running next.
