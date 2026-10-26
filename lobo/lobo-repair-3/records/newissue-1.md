The IL-28 tail gun script builds its gunner from the pilot's group. When the plane has someone aboard but no pilot at the moment the script runs, the script stops on a script error and the tail gun setup never finishes.

**Where**
- `LoBoIL28\script\tailgun.sqs`, started by `script\common_init.sqs:10` 0.1 s after spawn. `common_init.sqs` runs from the init handlers of four IL-28 classes (`LoBoIL28\config.cpp:295`, `:405`, `:458`, `:510`).
- The script skips gunner creation only when `count (crew _plane) == 0`. With crew but no driver, `_pilot = driver _plane` is objNull and its group is null, so `createunit` adds nobody and `_tunits = units _pilot_grp` is empty. The next line, `_gunner = _tunits select (count(_tunits) -1)`, then selects index -1.

**Consequence**
- The engine reports `Script error at '_gunner = _tunits select (count(_tunits) -1)|#|': Zero divisor`. Under `--autotest` that aborts the game (run `mk20-02`, `game_stdout.log` line 450). What the script does after the error in normal play (whether the turret is still created and manned) was not checked.
- The probe only hit this because it seated the player as gunner with no pilot. Whether a normal mission reaches it is unverified: an editor-placed IL-28 with a gunner but no pilot, or a script that seats the gunner before the pilot within the first 0.1 s, would.
- An empty IL-28 spawns without errors, and with an AI pilot seated first the final probe (`mk20-03`) ran clean.
- Context for a fix: by design the main loop ejects the turret gunner and the plane's gunner once the plane is above 20 m without a living pilot (a dead-pilot bailout). A pilotless IL-28 that survives the error would take that path too.

**Fix options**
- Skip gunner creation when there is no pilot, the way the empty-plane path already does: replace `? count (crew _plane) ==0: goto "crtGun"` with `? isNull (driver _plane): goto "crtGun"`.
- Or guard the select only: add `? count _tunits == 0: goto "crtGun"` before the `_gunner = ...` line.
- Either is plain OFP 1.x SQS, the same under the ud and classic profiles, and needs a new fail-closed rule in `repair_lobo.py` (the member is repacked, as in .3).
- Add a runtime case: IL-28 with a gunner and no pilot, on the ground and above 20 m.

Found during the 2026-10-11 runtime probes for repair 2026-10-11.3 (UD engine, official Remaster data, installed @LoBo). Not caused by .3: no repair batch touches `tailgun.sqs` or `common_init.sqs`.
