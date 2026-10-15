# reviewable_forces

`reviewable_forces` is the review and delivery branch for a focused reliability
batch in **Uslu dur! (UD)**: town capture, occupier counterattacks, saved controls,
and stronger dealer/portrait regression tests. The accepted batch is integrated,
independently reviewed, tested headlessly, and published.

The target is the **full official Arma: Cold War Assault Remaster** and its
`remaster.cpp` contract. Classic 1.99 compatibility is being dropped. Remaining
Classic references or files are legacy material, not a support promise; the
Remaster Demo's smaller asset set is not a gameplay target.

## Branch and review checkpoints

| Checkpoint | Revision |
| --- | --- |
| Branch created from `origin/occupier-forces` | [`887ce31`](https://github.com/GearUnclear/CWR/commit/887ce31) |
| Upstream explicitly merged before integration | [`197a7d3`](https://github.com/GearUnclear/CWR/commit/197a7d3), through merge [`ee9699a`](https://github.com/GearUnclear/CWR/commit/ee9699a) |
| Final tested code and test content | [`6c3dcde`](https://github.com/GearUnclear/CWR/commit/6c3dcde) |
| Documented and published batch checkpoint | [`a6fe940`](https://github.com/GearUnclear/CWR/commit/a6fe940) |

The upstream branch is **`origin/occupier-forces`**. There was no remote branch
named `forces` when this batch began. Publication went only to
`origin/reviewable_forces`; neither upstream nor `main` was pushed, and no PR or
issue was published as part of this batch. Later documentation commits do not,
by themselves, establish that later code has passed the same checks.

## Inherited upstream changes

These upstream changes arrived on **2026-10-15** through the explicit merge.
They are separate from the branch fixes below.

| Commit | Before → after |
| --- | --- |
| [`fe228a0`](https://github.com/GearUnclear/CWR/commit/fe228a0) | Abstract-arrival logs said only that defenders were present → diagnostics identify what the census actually counted. |
| [`2921bcf`](https://github.com/GearUnclear/CWR/commit/2921bcf) | Repeated group creation could exhaust increasing group IDs → new groups reuse the lowest free ID. |
| [`197a7d3`](https://github.com/GearUnclear/CWR/commit/197a7d3) | An air replan could change the pool band during debit and again during refund → one band update reflects the net ledger movement. |

Independent integration review checked that both upstream and worker regression
tests survived the merge. Previously landed reserve normalization and corrected
SPENT/garrison prose were not reapplied as new fixes. The old checkout's include
fix was also redundant with upstream.

## Running change list

Entries follow integration order; linked commits retain the implementation history.

### 2026-10-15 — Establish the baseline and refresh upstream

[`d2b6050`](https://github.com/GearUnclear/CWR/commit/d2b6050) and
[`8bb7b88`](https://github.com/GearUnclear/CWR/commit/8bb7b88) recorded the campaign,
baseline, and failing regressions. Upstream then advanced from `887ce31` to
`197a7d3`; merge [`ee9699a`](https://github.com/GearUnclear/CWR/commit/ee9699a)
brought those changes in before the reviewed fixes were integrated.

### 2026-10-15 — Count simultaneous town captures once each

Fix [`384b283`](https://github.com/GearUnclear/CWR/commit/384b283), integrated in
[`82a57cf`](https://github.com/GearUnclear/CWR/commit/82a57cf).
Before, two towns captured in one registry tick could count as three captures.
On the legacy response path, that could also apply the later-town response
chance; the active director does not use that count argument for dispatch.
After, capture history excludes later unprocessed events in that batch, then
counts each eligible town once. Existing holdings and saved capture history
still contribute; exempt towns stay excluded.

### 2026-10-15 — Use the current player position for town/base pressure

Fix [`93cd5c9`](https://github.com/GearUnclear/CWR/commit/93cd5c9), also integrated
in [`82a57cf`](https://github.com/GearUnclear/CWR/commit/82a57cf).
Before, the zone tick read the force's previous suspension flag, so entering its
bubble could allow an invisible force to drain support or prematurely retake a
town. After, the census uses the current player sample and the force destination;
leaving resumes pressure on the next zone tick. Tests include bases and an offset HQ.

### 2026-10-15 — Share admission budgets across one counterattack pass

Fix [`b6901d6`](https://github.com/GearUnclear/CWR/commit/b6901d6), integrated in
[`155e2c9`](https://github.com/GearUnclear/CWR/commit/155e2c9).
Before, multiple operations could each see the same remaining body/group budget
and collectively exceed it. After, successful births consume the pass's remaining
allowance using actual bodies and groups, including crews. Failed births do not
consume it, and garrison detachments do not charge existing bodies again.

### 2026-10-15 — Honor the exact 30% pool boundary

Also fixed by [`b6901d6`](https://github.com/GearUnclear/CWR/commit/b6901d6).
Before, floating-point multiplication could reject an operation with exactly
15 of 50 pool units remaining. After, the comparison respects the configured
share at that boundary. Tests cover below, at, and above the threshold.

### 2026-10-15 — Preserve intentionally cleared controls during migration

Fixes [`bad1530`](https://github.com/GearUnclear/CWR/commit/bad1530) and
[`44a8254`](https://github.com/GearUnclear/CWR/commit/44a8254), integrated in
[`d09ce3b`](https://github.com/GearUnclear/CWR/commit/d09ce3b).
Before, saving/migrating bindings could restore actions the player had cleared;
empty slots could also block the stand-up fallback after moving Q to lean.
After, explicit clears and known v3/v4 omitted rows retain their meaning, and
fallback decisions count usable bindings. Earlier/unknown layouts keep their
previous recovery behavior. Already overwritten preferences cannot be recovered.

### 2026-10-15 — Strengthen dealer and portrait tests

[`6c3dcde`](https://github.com/GearUnclear/CWR/commit/6c3dcde) changes tests only.
Before, some checks could pass with stale dealer rows, the wrong cancelled
portrait, or a cache-hit counter that was never meaningfully reset. After, they
check exact identities, retained pixels, replacement output, and a real warm
cache hit. Three actual-source mutations passed the old selection and failed
the strengthened one. Production dealer/portrait behavior is unchanged.

### 2026-10-15 — Record the completed gate and publish the batch

[`a6fe940`](https://github.com/GearUnclear/CWR/commit/a6fe940) records the final
results, review evidence, and remaining runtime checks. The published code/test
content is the tested `6c3dcde` content; this checkpoint adds documentation.

## Executed verification

The final Linux Clang/Ninja build passed. Direct binaries used the CI selection
`~[GameData] ~[gamescan] ~[external-data]` without proprietary game data.

| Final check | Result |
| --- | --- |
| Foundation | 496 passed |
| Core | 826 passed |
| Poseidon | 3,250 passed; 12 skipped because no OpenAL device was available |
| Total unit cases | **4,572 passed; 12 audio skips** |
| Synthetic mod-intake checks | **8 passed** |
| Python contracts | **9 passed** |

Original-production regressions reproduced capture, admission-budget, pool-floor,
and controls defects. The presence red run instead restored the old suspension
predicate behind the new callable signature: three cases failed with ten failed
assertions. It was a disclosed compatibility mutation, not an unchanged-old-header
build. Accepted source was restored before the final gate.

Direct binaries avoid the existing CTest discovery problem for unbuilt targets.
Successful runs needed normal filesystem and loopback access. The [progress log](roadmap/forces-improvement-progress.md),
[independent review](roadmap/forces-independent-review.txt), and
[test audit](roadmap/regression-test-audit.md) retain evidence and caveats.
Local full logs are under `tmp/forces-evidence/final-*`; they are not committed.

## Remaining work and limits

No graphical gameplay, real-save compatibility, Windows behavior, or performance
verification is claimed for this batch. Full Remaster data was unavailable here.
Presence tests inject current samples; actual World/GatherInputs execution was
source-reviewed, not exercised in a running game. Budget changes are not universal
cap enforcement: the pre-existing strike body-cap omission is a separate issue.

| Unlanded follow-up | Scope and status |
| --- | --- |
| F008 | Companion death/immediate-save/reload race: source mechanism reviewed, no runtime reproduction or fix landed. Running SQS text saved in old campaigns needs a deliberate compatibility strategy. |
| F062 | Traffic event queues with no consumer: reviewed lead only; no fix or measured slowdown claim. |
| F078 | Stored traffic-alert stimulus after a zone changes owner: reviewed lead only; no fix or demonstrated downstream QRF launch. |
| Separate Remaster-only task | Runtime data selection, launch/setup guards, associated tests/docs, and default Trident environment. Not merged or validated in this batch. |

Real-game follow-up should use a setup-prepared full Remaster shadow: simultaneous
town captures, bubble crossings near retake, concurrent foot/mounted/air births,
save/reload, and authentic cleared controls on Windows remain to be checked.

## Maintaining this guide

Append dated logical entries only when changes land. Include commit links,
a plain before/after, the exact tested source hash, checks
actually run, results, and limitations. Keep inherited upstream work separate.
Update the checkpoint table only from verified integration/publication evidence;
do not transfer an older green result to newer source. For documentation-only
updates, retain the tested code hash. Correct old claims with a dated amendment
rather than silently replacing history. Move unlanded work into the running list
only when a landed commit and its evidence justify it.
