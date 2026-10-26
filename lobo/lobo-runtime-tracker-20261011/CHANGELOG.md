# Change log

## 2026-10-11 - Repair 2026-10-11.2 applied and offline verified

- Corrects [#6](https://github.com/GearUnclear/lobo/issues/6), AP mine pickup's nonexistent script reference; [#7](https://github.com/GearUnclear/lobo/issues/7), IDF UH-60 insertion's missing path separator; and [#10](https://github.com/GearUnclear/lobo/issues/10), MiG-23 wing-sweep clamp's variable/numeric typos.
- Source is merged in [CWR PR #74](https://github.com/GearUnclear/CWR/pull/74), source commit `bd9b7d0264c56ebb3036289dd018e80eacb2f17f`, merge commit `9c9868231ae3c008d1112fe0d5dda0d4120c1617`.
- Applied with verified backups to three members in `LoBo_Mines.pbo`, `LoboIDFhelo.pbo` and `LoBoMig.pbo`. Rule SHA-256: `1520e54c8b0c601887abcada96325a41ba4b0c361275d05090b684acaf0c0c53`. Backup batch: `20261011T201747Z-5b12c00c`.
- All 86 synthetic LoBo regression tests pass. Installed repeat check exits 0 with zero changed members; report-only doctor exits 0 and says Nothing to do, with 15 existing unreadable entries. Independent verification checked all 124 archive hashes, backups, changed member hashes and 1,360 byte-identical untouched members in the three changed archives.
- Implemented, applied and offline verified only. No game was launched for this batch, so mine pickup, insertion and wing animation are not claimed runtime verified. Issues #2-5 and #8-9 remain outside this bounded batch.
- Package pickup note written at `D:/LoBo-Reloaded/incoming/lobo-repair-update/NOTE.txt` before the 13:30 PDT cutoff. Local reports are under `D:/Arma_CWA/tmp/lobo-runtime-acceptance-20261011/repair-2-*`. This batch supersedes the earlier acceptance handoff's unchanged-PBO statement for these three archives only.

## 2026-10-11 - Runtime acceptance partially verified (UD)

Repair `2026-10-11.1` remains installed. [CWR PR #70](https://github.com/GearUnclear/CWR/pull/70) is merged; no repair rule or installed PBO was changed by this acceptance task. Original runtime probes and detailed scope/evidence are in [CWR PR #73](https://github.com/GearUnclear/CWR/pull/73), commit `3447d53fe7e2f036cc1805d6d0dcee8f4e1a7799`.

- **Runtime verified, aircraft/LCM item:** MiG-21 ic1/ic2, MiG-25 cap1 and F-5 firemissiles2/3 fired all 30 missiles plus two Mk20 bombs. The 42 close captures show missile visuals clearing; the helper positive control was observable and ended at zero. Both LCMs display three-digit numbers in the reviewed side/stern views. These were controlled invulnerable aircraft launches at 250 m, not normal-flight survival or guidance tests. LCM numbering used native init with the auxiliary server branch disabled.
- **Partial, AI/damage item remains unchecked:** native Jordanian M60A1 M833 firing, six controlled M833 target impacts, 144 blast cases across infantry/UAZ/Abrams, nominal 5/15/30 m and M113 cover, plus nine ordinary MG projectile impacts with zero nearby damage at measured 2.00349 m. Mk82/LDGB/GBU-12 measurements are broadly consistent with the common warhead policy. M240/M249/suppressed Negev fired at 100 m in the original lane. Longer-range firing remains unverified: the terrain-selected lane also failed its LoBo and stock M60 positive controls, so its zero-shot results do not establish a LoBo defect.
- **Blocked, multiplayer wreck-fire item remains unchecked:** three identical local game processes established the intended ownership. Simulation time advanced, but after host destruction the two clients still reported the Toyota alive with damage zero at 5 and 15 seconds. Expected M1Smoker count was zero on every peer. Ownership damage and natural cleanup were not verified. The separate dedicated attempt was rejected for client/server build-tag mismatch. Classic fallback was not tested.
- **Partial, tank item remains unchecked:** eight tank variants retained class names, ordered component names, named damage values and total damage through a fresh-process save/load, with no mismatch at tolerance 0.0001. Track values were explicitly seeded; this is not independent raw-slot coverage. Turret captures show no visible turn. Host-only test impacts did not appear on clients; multiplayer impacts/animations/saved missions remain open. Phase-one text was overwritten by the runner; reload logs and both phases' captures survive.

**Offline verified and unchanged for handoff:** final repair checks exit 0 with zero changed members; report-only doctor exits 0 and says Nothing to do for 124 PBOs, with 15 entries unreadable by that scanner. All 124 archive hashes, the Bin roster and both runtime binary hashes match the pre-run inventory. The earlier applied/baseline rule SHA-256 remains `54a34c7b1144259b89d4c5522795f8b65f8ffc72b43a4dfddecb386e00f2c1fb`; both final source checkouts report `e025a5228caebc861cc39a981e8b84322ffbc180de6625980ba3e0e3c43e38f3`. The source-byte fingerprint difference was not attributed, and no new repair batch was applied.

Local evidence is retained at `D:\Arma_CWA\tmp\lobo-runtime-acceptance-20261011`: `aircraft-04`, `lcm-01`, `damage-02`, `blast-variants-01`, `ordinary-mg-02`, `ai-ranges-02`, `ai-clear-lane-01`, `ai-controls-01`, `tanks-save-01`, `wreck-fire-03/04`, `wreck-diagnostic-01`, and final verification/hash reports. All game processes opened by this task were closed. Reported jet self-destruction at the original location and menu flicker are not claimed fixed. [Issue #1](https://github.com/GearUnclear/lobo/issues/1) stays open.

## 2026-10-11 — Repair 2026-10-11.1 applied and offline verified

- [Source PR #70](https://github.com/GearUnclear/CWR/pull/70), draft; source commit `ba21ff49370766f02f4d29ca6c65c5fb5e7e3eb3`.
- UD profile: 76 changed members in 25 addon archives. A second pass changes zero. Original archives and the complete manifest remain local.
- Fixes cover all 13 audit areas plus reusable diagnostics: ammunition/reference errors, role-specific AI ranges, blast policy, script state/labels/helper cleanup, supported armor/model bindings, missing divers and display text.
- 81 synthetic tests pass; 11 changed configs export; all 7,993 explicit Lobo references resolve in a 114-export index. The mounted standalone Man/Land probe improves from 1,747/1,749 to 1,749/1,749.
- [Targeted repair CI](https://github.com/GearUnclear/CWR/actions/runs/38134085171) passes on both Ubuntu and Windows. The broader engine CI was still running when this record was written.
- Independent installed verification confirms 25 archive/backups, 76 changed members, 6,129 untouched members byte-identical, and all six model reads.
- Local applied rule SHA-256: `54a34c7b1144259b89d4c5522795f8b65f8ffc72b43a4dfddecb386e00f2c1fb` (fingerprint includes local source bytes/line endings).
- Backup manifest SHA-256: `ff5985e27d26a66a105bcaf366dae6acd969fb11fe4fa466846b23b911fa8e48`; local batch `20261011T105837Z-33d55270`, under `@LoBo/.ud-backups/issue-69/`.
- No game launched. Runtime combat, multiplayer, animations and saves remain unverified; [issue #1](https://github.com/GearUnclear/lobo/issues/1) stays open for these checks. Classic fire compatibility uses a documented group-limited fallback, while UD has full nearby-infantry queries. The lightweight audit reports five legacy parse gaps; the export index excludes two unavailable AGS exports.

Reproduce from the CWR root: `python -m unittest discover -s arma_CWR/tests/ci -p 'test_lobo_*.py'`; `python arma_CWR/tools/lobo/repair_lobo.py --mod '@LoBo' --compatibility ud --check`. Full commands/policies are linked in [References](REFERENCES.md).

## 2026-10-11 — Static audit and repair work started

- Created this private reference repository and transferred [CWR #69 to Lobo #1](https://github.com/GearUnclear/lobo/issues/1).
- Audit covered configs from 124 addon archives and 904 Lobo SQS scripts. The static Classic+Lobo Man/Land probe checked 1,749 classes, with two missing-model failures.
- Started coordinated work on ammunition/AI/damage profiles, script/polish defects, supported armor components/model bindings, missing diver models, and reversible archive repairs.
- Implementation is currently in the CWR working tree. Existing unrelated packaging changes are preserved.
- No game was launched. No gameplay or multiplayer validation is claimed.

This entry records the start of work; the applied batch is recorded above.
