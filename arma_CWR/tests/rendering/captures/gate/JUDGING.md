# Judging a graphics-remaster change (P0-11)

The procedure behind PERF-PLAN 3.3 to 3.6: who looks at what, with which tool, how a verdict is
written, and how the judges are calibrated before their first real verdict. Readers: the
orchestrator, the calibration agent (CAL-1), the steward and the sheet writer. **Judges never get
this file**: a judge gets `judge_brief.txt` with the placeholders filled in and its batch folder,
nothing else (section 6 explains why).

## 1. The tools (all in this folder; image work only through `squad-heavy.ps1 -Kind analysis`)

| tool | what it gives |
|---|---|
| `vgate.py compare A B --claim exact\|pit\|invisible\|look --noise <file>` | per shot: class (IDENTICAL, PI-T, WITHIN-NOISE, NEAR, CHANGED), the claim result (PASS, EXPLAIN, FAIL, PERCEPTUAL, JUDGES, REFUSED), max / px>2 / px>8 per band, the offset-compensated px>2, the shift spread, every cluster over 8 levels as a crop, SSIM and dE, the tripwires with crops, `diff_x8.png`, `blink.gif`, and the blind 320 px sheet `thumbs_blind_320.png` (its key sits in `thumbs_blind_key.json.private`, which no judge opens) |
| `vgate.py noise B1 B2 B3 --build-id X` | the boot-to-boot floor of one build: every pair's counts, the maxima per view and band |
| `motion.py screen --cand C --ref G [--vanilla V]` | the motion numbers and tripwires of the candidate against the golden clip, plus the still products: 4 x 4 contact sheets at a 1-frame step (frames and camera-compensated differences), velocity and residual maps, x-t and y-t slices |
| `owner_sheet.py spec.json` | the owner's page: 320 and 960 px pairs, three-way clips, one plain question per item, no numbers |
| `perf-bench.ps1` + `bench_report.py` | the numbers side (not a judge tool; judges only check that a faster number has a valid still, 3.3 "Validity") |

Shots come from the gate's own run folders (perf-bench `stills` jobs, `run-vgate.ps1`), each boot
with its `manifest.json` (exe sha256, build id, env, tests). Never judge from an implementer's
pictures.

## 2. The ladder and the claims (3.3)

- **IDENTICAL**: 0 differing pixels. The only pass for a PI claim once `triDeterministic` exists.
- **PI-T** (until then, for perf fixes): pixels over 8 within max(2 x the view's floor, 30), pixels
  over 2 (after removing one global per-channel offset when the shift is spread) within
  max(1.25 x floor, 2,000), any small shift spread over the frame or no larger than between two
  boots, and no cluster over 8 levels bigger than the floor's largest. Clusters are cropped; a
  claim whose counts are inside the floor but which has a bigger cluster or a concentrated shift
  reads EXPLAIN: the implementer explains every crop in `items/<id>/notes.md` before it lands.
- **WITHIN-NOISE**: every band's max, px>2 and px>8 inside the floor. The bar an INV item meets
  before the judges look; never a PI pass.
- **NEAR**: SSIM >= 0.995 full size, dE mean <= 0.5 and p99 <= 3 at 320 px (estimated).
- **CHANGED**: everything else.

Rain views (rain or wet in the shot, or a rain view name) are perceptual only until P0-6 seeds the
rain path: their claim result is PERCEPTUAL and the rain-rule tripwire runs on the wet ground box.

**Refusals** (the comparator refuses, the judge never sees the set): UNSETTLED, lit not OK, the two
sides in different lighting modes, a non-remastered shot under a judged claim, objects= off by
more than 5% undeclared (rain views: a note), a missing manifest (bootstrap sets need
`--allow-no-manifest`, written in the verdict), equal build ids or an identical file where the job
says the builds differ, a dry shot after the session drew rain (P0-17), and with `--strict-counts`
any objects= or draws= difference (items that cull nothing: W1-9, INFRA-C1).

## 3. The rubric (3.4, short form; the plan's table of perf-lane risks applies in full)

1. **Provenance first.** Build ids differ as the job says; `lit=OK` and `mode=remastered` on every
   judged shot; no UNSETTLED; `objects=` within 5%; the candidate file is not the golden file;
   above noise where a change is claimed, IDENTICAL where "identical" is claimed, within noise where
   "invisible" is claimed; an effect proven by pixels, never by a probe that only says it drew; the
   right side really is the candidate. Any failure: refuse, write why, stop.
2. **Thumbnail test** (Realism judge): the blind 320 px sheet. "Can you tell the two apart? If so,
   which looks more real, and why?" A visible difference on an "invisible" change fails.
3. **Rejected looks**, yes or no each, with the crop: blue haze or wash; mustard or khaki ground;
   crushed shade on vehicles and soldiers (shade about 0.6 of lit); flattened or veiled crowns; only
   darker; evening cooler than the original; shadow-map darkening; flat Lambert look; reflections or
   streaks on non-puddle wet ground; the open items (tan fog wall, white low beams, olive fire-lit
   ground) must not get worse.
4. **Full size** (Artefacts judge): texture sharpness, silhouettes and tree lines at 1 to 3 km,
   aliasing on wires and fences, shadow acne or peter-panning, banding (12x luma high-pass).
5. **Motion** (screener, T items): the still products of `motion.py` and its numbers. No flicker or
   pumping, no motion the camera does not explain, no grain, boil or fizz, no popping or ghosting,
   the same look in motion as in stills. A screener may FAIL a T item; it never approves one.
6. **Fog and AI link**: any fog or air change passes the `[remaster][fog][aisight]` unit cases and
   `cockpit_fog_abel`; a change to drawn transmittance or density without the AI side is a blocker.
7. **Perf-lane checklist**: tick every row of PERF-PLAN 3.4 item 7 that matches the item's files.
8. **Verdict**: `PASS-IDENTICAL`, `PASS-INVISIBLE`, `LOOK-CHANGE`, `FAIL` or `UNSURE`, each with the
   crops that justify it, in `items/<id>/judge.md`.

**Unsure means UNSURE.** A judge that cannot decide writes UNSURE with its crops; it never guesses.

**Escalation** (3.4, 3.6): non-T item, both judges PASS: land (until the owner confirms the
calibration, O-24, an INV item also goes on the owner sheet). T item: both judges PASS and the
screener passes: the owner's motion sheet; it lands only on the owner's yes. Either FAIL: back to
the implementer with the crops. Disagreement, UNSURE, LOOK-CHANGE, or a tripwire against the anchor:
the owner queue. Never show the owner an unproven picture.

**Tripwires** (P0-9, `vgate.py`; motion ones in `motion.py`) force a look and never pass anything.
Thresholds are estimates until section 5 has run:

| tripwire | fires when (candidate against golden) | box source |
|---|---|---|
| only darker | frame mean L* down > 2 with no chroma or contrast rise | whole frame |
| blue wash | frame mean b* down > 1.5, or the ground box's b* down > 1.0 | ground |
| mustard ground | ground hue > 4 deg toward yellow (95 deg), or b* up > 3 in the yellow sector | ground |
| crushed shade | hull shade/lit (Otsu split in the hull box) drops > 0.03 | hull |
| flattened crowns | crown L* stddev down > 10% | crown |
| evening warmth | lit ground or sky CCT up > 150 K at a golden-hour view | sky, ground |
| sharpness loss | Laplacian energy over the box's own variance down > 10% | texture |
| rain rule | wet-ground pixels above the golden's 99.5th percentile up > 20%, or vertical streak energy up > 15% | wet |
| travel, flicker, boil, grain, shimmer, ghost, popping | `motion.py` docstring (bars TRAVEL <= 0.05 as rho_off, LOW <= 0.08 as lowd2) | per clip regions |

The boxes in `golden_views.py` were placed by eye on the 1080p overhaul shots of
`tmp\look\director\final` and must be re-checked on the afdb4d60 goldens (CAL-1, first task). A
missing box reports "no box", never "quiet". The absolute shade band 0.58 to 0.62 needs boxes on
the measured hull faces; until CAL-1 sets them only the relative drop fires.

## 4. Self-tests that exist (tooling, not calibration)

`selftest.py floor` reproduces M2's boot-to-boot table exactly; `selftest.py tripwires` checks that
synthetic edits of real shots fire their tripwire and the identity fires none; `selftest.py text`
checks the generators and the bench report. These prove the code paths, not the thresholds.

## 5. Calibration (blind, held out; PERF-PLAN 3.4 "Judge calibration", O-24)

Run once before the first INV or LOOK verdict, and again for any new judge brief.

**Cases and the split** (proposed by INFRA-B; the orchestrator may swap cases between halves before
CAL-1 starts, never after):

| case | owner verdict | half |
|---|---|---|
| 026f8b7c fog (the accepted fix) | accepted | **held out** (the plan requires it) |
| d91a08e0 fog ("wind") | rejected | **held out** (the plan requires it) |
| round-5 overhaul shots (`tmp\look\director\final`) | accepted | held out |
| `taaMipBias=0` against the shipped -0.5 (sharpness loss) | rejected | held out |
| puddle option 4 (`tmp\look\puddle\4\final`) | accepted | tuning |
| round-3 shiny wet ground (archive lane patches, confirm with one still first) | rejected | tuning |
| fec1075 against vanilla ("nearly no difference") | rejected | tuning |
| F1 valley mist pump (`tmp\video\mist\backup_ud\valley_mist_before_mistfix` against `backup_van`) | rejected (the pump the F1 fix removed) | tuning (extra case, not in the plan's list) |

**Controls**: the same build against itself on two boots (M2's `p5-stills1..3` for stills; two
recordings of one build for clips, e.g. `tmp\video\v2\vanilla\valley_mist` against
`tmp\video\mist\backup_van\valley_mist_before_mistfix`).

**Procedure**
1. CAL-1 tunes thresholds (NEAR, tripwires, `motion.py` bars) on the tuning half only, and writes
   every change with its reason into `items\CAL-1\notes.md`. It does not open the held-out cases,
   their clips, or any document that reports their numbers.
2. The orchestrator builds the blind set: every held-out case and at least as many controls,
   shuffled, labels and build names stripped, each as a neutral folder name (`case-01` ...), with a
   private key file only the orchestrator reads.
3. Two fresh judge agents (Realism and Artefacts) and one motion screener get `judge_brief.txt`
   and the blind folders. Their briefs never name the cases or past verdicts.
4. Pass: every held-out rejected case rejected (FAIL or, for T cases, the screener's reject), every
   control passed, no accepted case rejected. UNSURE on a rejected case counts as caught only if
   the judge's crops show the defect.
5. The orchestrator writes one plain line onto the next owner sheet ("the judges caught N of M known
   bad changes; they missed X; here is X") and asks O-24 once.

**Contamination note (2026-10-31).** INFRA-B, which wrote these tools, read d91a08e0's measured
numbers in `tmp\video\fogwind\lead\DECISION.txt` and `tmp\video\fogwind\BOARD.txt` while looking
up the plan's TRAVEL and LOW bars. INFRA-B did not tune anything on them: `motion.py` uses the
plan's two bars as written and relative factors set before reading those files, and its self-test
ran only on the F1 tuning case and controls. Still, CAL-1 should treat INFRA-B's defaults as
unvalidated, must not be INFRA-B, and should not read those two files before the blind scoring.

## 6. Why judges never see this file

It names the calibration cases and their verdicts. A judge that knows "d91a08e0 had wind" is no
longer blind, and the held-out test measures nothing. Judges get `judge_brief.txt` only.
