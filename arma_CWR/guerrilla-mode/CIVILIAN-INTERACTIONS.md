# Civilian interactions (#41)

Approach an ambient civilian on foot, within 5 m, while within 300 m of
their town centre. Their action menu offers **Ask for a donation** and
**Extort money**. Panicked towns and a civilian undergoing a shakedown are
unavailable. Dealers, companions, disguised fighters and future NGO contacts
are excluded by population-roster membership, regardless of their model.

Each approached civilian draws opinion from town support +/-35, clamped to
0..100, and fear from 0..100. Occupier-owned towns multiply the support
baseline by 0.5 before that opinion draw; ownership does not gate interaction
or overwrite town support. This suppression factor is a tuning choice where
the issue specified suppression without a number.

| Action | Outcome | Personal cost | Default reward |
|---|---|---|---|
| Solicit | Opinion >=75: 50% donation chance; otherwise refusal | 10 opinion per attempt | 25 R on donation |
| Extort | Fear >=65: certain payment; fear <20: resistance; otherwise refusal | 30 opinion per attempt | 50 R on payment |

Every valid attempt, including refusal and resistance, starts the same 60 s
personal cooldown. Each actual opinion point lost nudges town support down
0.05 through `GM_fnSupportAdd`. Opinion recovers at 1 point/minute up to its
original baseline, evaluated lazily on the next interaction. Recovery does
not undo town support losses. Rewards, cooldowns, recovery and the intermediate
fear refusal policy are explicit implementation choices for unspecified parts
of #41. Combat retaliation and random rogues remain issue #42: resistance
currently refuses payment and emits the hook below.

## Campaign and performance contract

The shared core runs on every full-core template. It scans only the existing
bounded ambient population, once a second, and mounts at most two actions.
Personal records are allocated only on interaction, never for the whole island.
The cache retains at most 64 rows (normal ambient population is 24 bodies).
Capacity refuses a new profile without evicting/rerolling an existing one.
Leaving a town's 300 m area drops its personal records. Null, dead, removed
or converted bodies are pruned. Revisiting can draw new scores; that is the
visit-local consistency rule in #41, not persistent civilian identity.

Profile layout in `GM_CI_PROFILES`:
`[body, zoneName, baselineOpinion, opinion, fear, updatedAt, nextAsk]`.
World saves restore valid body links and the script bank restores scores and
cooldowns. The bootstrap's single `campaignLoaded` handler requests a menu
remount and appends to `campaign.sqs`'s existing event queue. Native handler
registration replaces a slot, so the interaction manager must not register
a second load handler. Null links degrade by pruning. Zone
names, not unstable registry indexes, identify retained town state. Support
and treasury use their existing native/script campaign persistence.

Validation, cooldown, support debit and payout run in one synchronous call,
without a scheduled wait or a cached treasury balance. `GM_fnResourcesAdd`
is shared with zone income; market and recruit debits retain their atomic
read/modify/write expressions. A stale menu cannot pay after walking away,
death, panic or a change of target. The menu holds one pending request only.

Existing saves embed their old SQS text: this feature starts with new campaigns
or a fresh mission bootstrap. It does not migrate campaigns saved before #41.

## Extension surface

`[body, caller, "SOLICIT"|"EXTORT"] call GM_CI_fnInteract` is the same service
the action menu uses. It returns
`[outcome, paid, zoneName, opinion, fear]`, also published as `GM_CI_LAST`.
Outcomes: `DONATED`, `EXTORTED`, `REFUSED`, `RESISTED`, `COOLDOWN`, `INVALID`,
`CAPACITY`. The latter three do not change support or money.

After `GM_CI_READY`, systems can install synchronous callbacks:

- `GM_CI_ON_RESIST`: `[body, caller, zoneName, verb]`, once for every accepted
  low-fear extortion. #42 may reserve/transfer the body through its own bounded
  assailant service. This layer never changes global civilian diplomacy.
- `GM_CI_ON_RESULT`: `[body, caller, verb, result]`, once after each accepted
  attempt commits. Useful for mission objectives or other campaign policies.

Callbacks must not wait or recursively call the interaction service; enqueue
into the consumer's bounded queue for asynchronous work. Defaults are no-ops,
so an absent consumer cannot accumulate handles or grow saves. Callback
arguments are transient; persistent consumers should keep names/scalars.
`GM_CI_fnOutcome` is the pure decision seam; `GM_CI_ROLL` defaults to
`{random 1}` and allows deterministic tests without a second effects path.

Optional numeric keys in each island's **CIV descriptor**:

| Key | Default | Accepted range |
|---|---:|---:|
| `interactionOpinionSpread` | 35 | 0..100 |
| `interactionOccupiedScale` | 0.5 | 0..1 |
| `interactionDonation` | 25 | 0..10000 |
| `interactionExtortion` | 50 | 0..10000 |
| `interactionSolicitLoss` | 10 | 0..100 |
| `interactionExtortLoss` | 30 | 0..100 |
| `interactionRecoveryPerMinute` | 1 | 0..100 |
| `interactionCooldown` | 60 | 1..86400 s |
| `interactionSupportWeight` | 0.05 | 0..1 |

Numeric values are clamped at bootstrap. Personal thresholds, visit radius
and cache capacity are shared-core constants. Island/faction packs need no
script copies, civilian classname literals or fixed occupier side.

Trident: `guerrilla_civilian_interaction.test.sqf` covers decisions and the
action-to-ledger path; `guerrilla_civilian_interaction_save.seq` checks real
save/load, cooldown preservation, restored menus and deleted-body cleanup.

The Sinai integration lane runs the same service on the real @LoBo template
with WEST occupying and EAST resisting. The save fixture disables native
support drift so persistence can be checked against exact ledger values.
