# Reproducible Lobo content repairs

`repair_lobo.py` implements the static audit originally filed as CWR #69, now
tracked internally at [Lobo #1](https://github.com/GearUnclear/lobo/issues/1).
It changes installed addon members only after every repair has passed preflight.
No game is launched. Only transformation code, synthetic tests and documentation
belong in this source repository; addon data and backups stay local.

## Use

Run from the CWR repository root with Python 3.10 or newer:

```powershell
# Preview only; writes the optional report, not addon files.
python arma_CWR/tools/lobo/repair_lobo.py --mod '@LoBo' --compatibility ud --report tmp/lobo-preview.json

# Apply to the UD installation, with verified original backups.
python arma_CWR/tools/lobo/repair_lobo.py --mod '@LoBo' --compatibility ud --apply --report tmp/lobo-applied.json

# A second pass must report zero changed members and exit 0.
python arma_CWR/tools/lobo/repair_lobo.py --mod '@LoBo' --compatibility ud --check

# Broad diagnostics; candidates require review, not automatic repair.
python arma_CWR/tools/lobo/audit_lobo.py --mod '@LoBo' --report tmp/lobo-audit.json

# Synthetic offline regression suite, no installed data required.
python -m unittest discover -s arma_CWR/tests/ci -p 'test_lobo_*.py'
```

Pass `--tools PATH` for another standalone PoseidonTools build. The Windows
default is `arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe`. Compressed members
are decoded through its `pbo show` command, with archive snapshot checks around
the read. The selected repair archives in the audited installation need no
compressed-member decoding.

The repair exits 2 on incompatible input or I/O failure. `--check` exits 1
when repairs would change data, 0 when already repaired. The default preview
exits 0 for a valid plan. The audit's `--strict` exits 1 for unresolved references
**or incomplete source-parser coverage**; ordinary candidate warnings alone do
not make it fail.

Reinstalling original Lobo data requires rerunning the repair. Run the existing
`PoseidonTools mod doctor <mod> --fix` maintenance separately for scope keywords,
malformed floats and buried wreck origins. This new repair does not replace it.

## Compatibility profiles

Use **`--compatibility ud`** (default) for Uslu dur!: wreck fires use the engine's
`nearestObjects` command to discover all nearby on-foot infantry, including
remote victims. One scheduler on the wreck owner applies damage through the
engine's network-aware `setDammage` path. The local player is excluded from that
query and handled once separately. This is verified against source behavior;
multiplayer execution has not been tested.

Use **`--compatibility classic` on a separate Classic mod copy before packaging**.
Classic fire scripts discover the player's and wreck's groups, de-duplicate them
and filter for distance, living units and on-foot state. This fixes the undefined
function and invalid glow state using older commands, but cannot discover every
unrelated group. It is a deliberately limited compatibility fallback, not full
nearby-infantry parity. Neither profile creates a trigger to perform this query.

Do not redistribute the UD profile as a Classic-compatible repair: Classic 1.99
does not provide `nearestObjects`. The newer `createTrigger`/`setTriggerArea`
commands are also unsuitable as an assumed Classic PC fallback. Official remaster
versions need their own API/runtime verification. Existing package tooling does
not automatically select this repair profile; prepare its source copy explicitly.
Switching the two profiles updates only the 16 wreck-fire queries and creates a
new backup batch; switching back is reproducible.

## Repair scope and policy

| Audit item | Implemented change |
| --- | --- |
| Missing M833 projectile | Defines the existing magazine's projectile at an explicit 400-point baseline below M900; no alias or identifier rename. |
| MG engagement | JAM-derived role profiles; separates MG and rifle 5.56 use and corrects the suppressed Negev magazine. |
| Unsupported armor | Uses the six supported tank components; repairs missing model bindings and hull/turret membership. |
| Aircraft labels | Corrects MiG-21, MiG-25 and F-5 targets; preserves pylon slots. |
| Micro Uzi | Corrects the HD magazine reference. |
| Landing craft | Repairs vehicle variable, digit labels and unused parameter access in both numbering scripts. |
| Wreck fires | Initializes the query; valid sound anchor on every path, guarded glow lifetime and one damage scheduler. |
| Divers | Uses the verified bundled swimmer model for both missing outfits, retaining roles/loadouts. |
| MG splash | Explicitly removes indirect damage from ordinary ball/AP rounds and preserves explosive families. |
| Blast policy | Bounds the 60 mm and vehicle-bomb plateaus while retaining contact strength and heavy-mortar baselines. |
| Mk82 family | Shares the existing Mk82 warhead values with GBU-12, retaining guidance controls. |
| Missile helpers | Deletes 41 launch terrain helpers immediately after their final read; guards projectile ownership. |
| Display text | Repairs Assault, Missile, Gauge and 122 mm strings without renaming classes. |
| Offline checks | Reference, label, local-call and custom-hitpoint diagnostics; explicit incomplete-coverage reporting. |

See [AMMUNITION-POLICY.md](AMMUNITION-POLICY.md) for all tuning choices and the
144 mathematical blast scenarios; these are not measured lethal radii or
shots-to-kill results. See [ARMOR-MODELS.md](ARMOR-MODELS.md) for actual model
selections and preserved script-facing names. The repair implements supported
component coverage, not a new directional penetration simulator.

## Backups, integrity and recovery

Every apply batch stores complete original archives and `manifest.json` under
`<mod>/.ud-backups/issue-69/<UTC-time>-<unique-id>/`. The historical directory
name remains stable after the issue transfer. The manifest records repair
version, source-rule SHA-256, profile and before/after archive/member hashes.
Reports can be stored under the checkout's ignored `tmp/` directory.

All new archives are staged and validated before originals are backed up or
replaced. Non-target member bytes, ordering, packing and metadata are preserved.
SHA-1 trailers are verified/recomputed; legacy textual signatures are retained.
Old StuffPBO trailers can be inspected, but rewriting their unsupported checksum
format is rejected. Unknown source values, duplicate/unsafe members, incomplete
inputs and conflicting edits fail before installation.

An interrupted multi-archive install attempts to restore every file it changed.
Rollback preserves any later concurrent edit, records conflicts/failures and
keeps the originals. Replacing a whole mod directory is not one filesystem
transaction; inspect the manifest if power loss interrupts the process.

For deliberate recovery, stop using that mod copy, verify each current archive
matches the manifest's `after_sha256` and each backup matches `before_sha256`,
then restore that batch's originals. If a hash differs, review the later change
instead of overwriting it. Restore batches in reverse order when reverting
multiple repairs. Keep backups out of both GitHub repositories.

## Validation limits

The lightweight source linter intentionally fails closed on malformed legacy
syntax. The audited pack has five configs it cannot fully parse (`LoBo_FAB`,
`LoBo_IAk47Iraqi`, `LoBoChallen`, `LoBoEgypt`, `W_CombatStatics`). It reports them
and downgrades unresolved references to uncertain candidates because skipped
files may contain those definitions. PoseidonTools' legacy config exports and
the mounted static probe are separate evidence; a successful export can recover
old syntax and is not proof the source was well formed. No braces are invented
by the repair to silence this report.

Label/local-variable scans also cannot prove reachability or general dataflow.
The broad candidate list is not a list of confirmed gameplay defects. The
targeted repair validates all changed script labels and known variable state,
model selection structure, expected ammunition values and repeatability.

No game was launched for this work. Runtime combat, all missile shots, AI weapon
selection, animated hit locations, saves and multiplayer still need a separately
authorized play test. All peers should use the same repaired content.
