# Occupier forces director: lever audit

Read-only audit of branch `occupier-forces` at 4b9e4b7, 2026-10-16. Nothing was
edited, built or run. The owner's question: "are there too many levers on the
director, are all impactful". Short answer: **yes, too many. 75 tuning keys
feed the director and its neighbours. 18 change what a player sees in an hour
on Malden, 22 are dead or shadowed at the defaults, and almost all of them are
set only by tests.** The shipped templates set one key: `occupierForces`
(Guerrilla.Abel 1; Market, Qrf, Undercover 0). No description.ext and no
config file authors any other director key. The faction descriptors author
only the air hull classes.

## How the evidence was gathered

- Code: `OccupierForcesTuning` (OccupierForces.hpp:313-400), its reader
  `LoadFromParams` (OccupierForces.cpp:258-333), `CounterAttackTuning`
  (CounterAttack.hpp:179-204, CounterAttack.cpp:181-222), `ZoneTuning`
  (ZoneRegistry.hpp:38-111, ZoneRegistry.cpp:747-757), the package choice
  (ForcePackages.cpp:683-790), `gmForcesTune` (OccupierForcesCommands.cpp:350-532).
- Docs: OCCUPIER-FORCES.md section 16, OFFSCREEN-CONTRACT.md section 4.
- Tests: tri tests in `tests/integration/scripting/guerrilla_forces_*` (66
  files), unit suites test_occupier_forces.cpp, test_force_packages.cpp,
  test_counter_attack.cpp, test_zone_registry.cpp.
- Videos: the scenario scripts in `tmp/of-director-video*/`. Videos 1 to 3 pulled
  `directorInterval 10`, `waveLead 120` and CA `birthGiveUp 60`. Video 3 also
  pulled `poolFloor 0.3`, `refillHours 48` (gmForcesForce) and CA
  `firstTownChance 0` / `chance 0`. Videos 4 and 5 ran on the shipped keys ("no
  director lever"). Their UDNOTE `lever:` lines only change garrisons, support,
  heat and squads. Video 5 (`stats.json`, 3 h 36 min of game time, 22
  operations) is the main evidence for which keys actually bind.

Malden facts the judgments rest on (Guerrilla.Abel, video 5 boot line):
R = 32 (Outpost 6, Compound 6, Harbour 8, Airfield 12), K = 14, G = 56,
poolStart = 60, refill 37.0/h. Single-base spare at full reserve, with
`counterAttackSourceKeep` 0.25: Outposts 5, Harbour 6, Airfield 9. The pool was
STRONG in 1435 of 1449 samples (99 %), with a minimum of 29 and a maximum of 69.

## The table

Legend. Read at: OF = OccupierForces.cpp, CA = CounterAttack.cpp, CAA =
CounterAttackAir.cpp, FP = ForcePackages.cpp, ZR = ZoneRegistry.cpp. Changed
by: U = a unit test sets it off default, Tn = n tri files change it through
gmForcesTune / gmCounterAttackTune, V = a video run changed it ("none" = only
default pins). Q1 feel: S = shapes an hour of play, W = felt rarely or late, N
= a log number only. Q3: A:<lever> = folds into that lever of the proposal
below; C = make it a constant; T = test lever behind gmForcesTune only; G =
engine guard; Geo = author geography key, keep. Q4: Y = the section 16 row is
accurate, P = partly, N = wrong or missing.

### OccupierForcesTuning (43 keys)

| Key | Default (range) | Read at | Changed by | Q1 | Q2 shadowed / dead at defaults | Q3 | Q4 |
|---|---|---|---|---|---|---|---|
| occupierForces | 1 (0/1) | OF:258, CA:205, ZR:757 | missions, U, T4 | S | no | Geo (the layer switch) | Y |
| forcesPoolMin | 24 (0..200) | OF:344 | U | N | dead on Malden: R + 2K = 60 > 24. Binds only on a micro-island | C | Y |
| forcesPoolPerTown | 2 (0..8) | OF:344 | U | S | no. poolStart sets wave size (0.5 x 60 = 30) and whether a strike fits the keep-STRONG rule (NEXT question 1) | A:Strength | Y |
| forcesPoolRefillHours | 4 (0.5..48; tune 0.01) | OF:730, 861 | U, T7, V3 | S | no. 37/h is 62 % of poolStart per hour, so the pool sat STRONG 99 % of video 5. This key alone decides whether the band ladder ever shows | A:Recovery | P: "exhausted to full" is right, but the row hides that the rate is (poolStart+R+G)/h, 148 units on Malden, all paid into the pool |
| forcesPoolFloor | 0.2 (0..1) | OF.hpp:756 | U, V3 | N | dead: THIN never reached in video 5 (10 samples in video 4) | C | Y |
| forcesDirectorInterval | 60 (10..600; tune 1) | OF:1307, 3229 | U, T19, V1-3 | N | tokens accrue on elapsed time, so the tick adds at most 60 s of reaction lag and no cadence | T | Y |
| forcesTokensMax | 2 (0..6) | OF:1913, 3297 | U, T2 | W | binds: the purse sat at 2 for 13 % of samples and the clock reset. Its one visible effect is a 2-op burst after a quiet spell (ops 17 and 18, 180 s apart) | C (2) | Y |
| forcesTokenInterval[] | {600,1050,1800,0} | OF:467, 1920 | U | S (STRONG entry) / N (rest) | the STRONG entry IS the cadence: ops 3 to 6 launched at 841, 1442, 2043, 2643. The other three entries are near-dead because the band stays STRONG | A:Cadence | Y |
| forcesControlMilWeight | 0.6 (0..1) | OF:806 | U | W | no. It moves when a wave fires (wave 1 at control 0.69, 44 min in) | A:Waves | Y |
| forcesWaveThresholds[] | {0.75,0.50} | OF:521, 1234 | U | W | the second threshold was not reached in 3.6 h | A:Waves | Y |
| forcesWaveWarLevel | 1 (0..3) | OF:1198 | U | W | no. After wave 1 the infantry tier rose (foot 4 = lethality 5.00 from op 19) | A:Waves | Y |
| forcesWakeTowns | 2 (2..99; tune 1) | OF:3292 | U, T18 (13 at 1, 5 at 99) | S | no. The first town gets only the legacy 40 % roll; video 5's Village got none | C (2); its value 1 is a test lever | P: the row omits that the tri suite mostly runs at 1, a value config cannot reach |
| forcesOpsMax | 2 (0..6) | OF:472 | U, T1 | N | applies only outside STRONG (1 % of video 5) | A:Concurrency | Y |
| forcesOpsMaxStrong | 3 (0..6) | OF:473 | U, T1 | N | never bound in video 5: the born-op count peaked at 2 of 3 because the token cadence binds first. It bound for 70 samples in video 4, before the re-arm fix | A:Concurrency | Y |
| forcesHeatFull | 40 (1..100) | OF:1962 | U | S | no. With heatCapSpike 40 a fresh capture is a sure candidate, then a fading one: held decay is 1.5/min, so the chance hits zero about 27 min after capture | A:Aggression | P: "candidate heat" does not say candidate chance = heat / heatFull per roll |
| forcesUrgentHeat | 50 (1..100) | OF:1896 | U | W | the scripted captures in the videos raised no alarm, so there urgency came only from a heat lever (op 14). In real play capture +40 plus RED +15 = 55 crosses it, so expect it often | A:Aggression | Y |
| forcesTownCooldown | 1800 (0..86400) | OF:3396-3425 | none | S | no. It sets the per-town cadence: Le Port was hit at 241, 2643, 4565, 6487, 8409, 10871 (rest 1800 s + the wait for a token) | A:Cadence | P: the row says "after a repel or a cancel" but not that the rest's end is the re-arm moment |
| forcesRearmHeat | 40 (0..100) | OF:1456-1505 | U | S (as on/off) | shadowed by heatFull: any value >= 40 is a sure candidate, values below 40 are a weaker roll, >= 50 makes every re-arm urgent. At the default it is a boolean | A:Aggression (on, = heatFull) | P: right about 40, but the urgent regime at >= urgentHeat is not mentioned |
| forcesPushoutQuiet | 1800 (0..86400) | OF:2072 | none | W | shadowed by intelDecay: sightings are deleted at intelDecay (OF:1829), so quiet = min(pushoutQuiet, intelDecay). Values above 1800 are dead. 3 push-outs in video 5 | C (= intelDecay) | P: it also applies to held bases |
| forcesRetakeMin | 4 (1..12) | OF:2149 | U | W | lowering it is dead on Malden: min = max(4, garrisonMax) and cityGarrison is 4. Only raising it is live | A:Minimums | Y |
| forcesLethalityMargin | 1.2 (1..3; tune 0) | OF:2144, 3896 | U | S | no. L_req = 1.2 x 3 x fighters: 4 fighters gave 14.4 and a Ural + T72 column of 25 units on Le Port five times | A:Aggression | Y |
| forcesUrgentCostShare | 1.5 (1..3) | OF:2151, FP:746 | U, FP unit | N | nearly dead. For a 4-body foot retake the next package (truck, 8) costs 2x, outside 1.5. Its one firing (op 14) traded Ural + T72 (74 s) for BMP + BMP (63 s) | C (1.5) | Y |
| forcesIntelDecay | 1800 (1..86400) | OF:1750, 1829, 3576, 3603, 3665 | U | S | no. It sets how long reported fighters drive L_req, re-arm, strikes and openers. It also caps pushoutQuiet | C (1800), or A:Aggression | P: the row gives no range and misses the pushoutQuiet cap |
| forcesSpareCap | 1 (0/1) | OF:2182, 2195 | U, T1 | W | on Malden it only ever lowers the 12-body Airfield retake to 6 from the Harbour (op 10 in videos 4 and 5) | A:Minimums (always on) | Y |
| forcesSpareCapFloor | 6 (1..12) | OF:2187, 2198, 2500 | U | N | shadowed twice. (a) The HQ assault: min(6, max(L, 6)) = 6 always, so the cap is a no-op on HQ assaults. (b) The largest spare outside the Airfield is the Harbour's 6, the floor itself. The floor only ever turns an all-Outpost spare of 5 into a refusal, which is the same as spareCap 0 | A:Minimums (= hqAssaultMin) | N: the row does not say that floor = hqAssaultMin makes the HQ branch inert |
| forcesSlowEta | 900 (0..86400) | OF:2163, FP:744 | U, FP unit, T5 (0) | N | never decided a pick. All 9 arrival-rule firings in videos 4 and 5 had the cheapest foot quote above 2000 s, so airEta alone gives the same helicopter. The log labels ops 3 and 17 "forcesSlowEta" only because FP:781 prefers that label when the price fits | A:Reach (0.6 x reach) | P: right, but the log line it describes misnames the cause |
| forcesSlowCostShare | 3.0 (1..5) | OF:2164, FP:747 | FP unit | N | binds only when the cheapest quote is 900-1500 s (foot legs of 1350-2250 m). No such leg occurred in videos 4 or 5. Above 1500 s airEta admits the helicopter anyway. 3.0 is exactly (6 + 10 + 2) / 6, a Mi17 over a 6-body foot quote | C (3) | Y |
| forcesAirEta | 1500 (0..86400) | OF:2165, FP:745 | U, FP unit, T5 (0) | S | no. 5 of 22 operations in video 5 were Mi17 insertions | A:Reach | Y |
| forcesRoadFactor | 1.5 (1..5) | OF:2156, 3904 | U, FP unit | N | nearly dead: RoadQuoteCache supplies a real route for 15 of 21 quotes in video 5. One quote used x1.5 (535 s, under slowEta) and no choice changed. It also disagrees with the march: CA:313 multiplies a route-less leg by counterAttackRoadFactor 1.3, not 1.5 | C (1.5, one shared factor) | P: the row misses the 1.3 vs 1.5 mismatch |
| forcesHqAssaultMin | 6 (1..12) | OF:2148 | none | W | no. It sizes the HQ assault (wave strike op 7, foot 6) | A:Minimums | Y |
| forcesControlDwell | 2 (1..10) | OF:2888 | none | N | a 2-tick debounce | C | Y |
| forcesWaveFinal | 1 (0/1) | OF:2884 | none | N | final stand only, never reached | A:Waves | Y |
| forcesWaveShare | 0.5 (0..2) | OF:2700 | none | W | no. Wave 1 = 30 | A:Waves | Y |
| forcesWaveMin / forcesWaveMax (2) | 12 / 48 | OF:2701-2702 | none | N | dead on Malden: 30 lies inside 12..48. They bind only for poolStart < 24 or > 96 | C | Y |
| forcesWaveLead | 600 (0..7200) | OF:2862 | U, T1, V1-3 | W | no. The warning window before a wave lands | A:Waves | Y |
| forcesWaveLeadFinal | 120 | OF:2849-2862 | none | N | final stand only | C | Y |
| forcesWaveRetakeShare | 0.5 (0..1) | OF:3009 | none | W | no ("up to 15") | A:Waves | Y |
| forcesWaveOps | 4 (0..8) | OF:3007, 3048, 3103 | T1 | W | the cluster gave 3 targets, so the cap of 4 did not bind | C | Y |
| forcesWaveCluster | 2000 (100..20000) | OF:3010-3015 | T1 | W | no. It picks which towns the wave hits | C | Y |
| forcesWaveSurge | 12 (0..48) | OF:2984 | none | N | final stand only | C | Y |
| forcesWaveHub | "" | OF:2786-2788 | U | W | no | Geo | Y |
| forcesLog | 0 (0/1) | OF:65 | U | N | a diagnostic switch | keep as a diagnostic key | Y |

### CounterAttackTuning forces keys (3) and counterAttack keys the director runs through (16)

| Key | Default | Read at | Changed by | Q1 | Q2 | Q3 | Q4 |
|---|---|---|---|---|---|---|---|
| forcesLiveHullsMax | 6 (0..20) | CA:1955, CAA:889 | U (9) | N | never bound (cap_exceeded empty). 3 born ops x at most 2 hulls each = 6, so only waves could reach it | A:Concurrency (2 x ops cap) | Y |
| forcesSortieTime | 300 (60..1800) | CAA:897 | U | W | 1 strike in video 5 | C | P: the owner column says OF, but CA reads it |
| forcesOffMapEdge | nearest | CAA:269, 314 | U | W | used only when no airfield is held | Geo | P: owner given as OF, but CA reads it |
| counterAttack | 1 | CA:181, 237 | none | W | no | Geo | Y |
| counterAttackFirstTownChance | 0.4 | CA:237 | U, T29, V3 | S | no. It decides whether the first town is answered at all | A:Aggression | Y |
| counterAttackChance | 1.0 | CA:237 | U, V3 | N | dead with the key on and wakeTowns 2: the flip of the second distinct town wakes the director, which takes it (CA:1171). It is live only if the first town re-flips first | C | Y |
| counterAttackSize / WarLevelStep | 0.5 / 0.25 | CA:243-248 | U | W | first-town answer only | C | Y |
| counterAttackDelayMin / Max | 120 / 300 | CA:1210 | U, T4 | W | first-town answer only | C | Y |
| counterAttackSourceKeep | 0.25 | CA:657 | none | S | no. It sets every base's spare and with it the package (Outpost spare 5 forces the BMP on Arudy) and the spare-cap coincidence | C (or A:Strength) | N: "source and delay" hides that it drives every director quote |
| counterAttackSpeed | 4 | CA:287 | U, T14 (speed lever) | N | dead with the key on (CA:306) except as the test lever | T | Y |
| counterAttackRoadFactor | 1.3 | CA:287, 313 | none | W | still read with the key on for any march with no road route (CA:313), at a factor different from the quote | C (merge with forcesRoadFactor) | N: the row says "legacy pacing (key off)" |
| counterAttackDefenderWeight | 3 | CA:278, 328, OF:4161 | none | S | no. Half of L_req. With the margin it forms one product (3.6) | A:Aggression (as the product) | N: the row says "Fight"; it is the director's lethality requirement |
| counterAttackRepelLoss | 0.5 | CA:1862 | none | N | dead with the key on (key-off Fight only) | C | Y |
| counterAttackLiveTimeout | 1200 | CA:2261, 2767, 2893 | none | S | no. A lone survivor held ops 12 and 15 for 18 min each (NEXT question 2) | C | Y |
| counterAttackMaxLive | 2 | CA:1948, 2136 | none | W | live ops reached 2 for 2 % of samples, with no deferral logged | A:Concurrency | Y |
| counterAttackBirthMinToTarget | 250 | CA:1658, 1982, 2956, 3077, CAA:285 | none | N | an unseen-birth guard | G | Y |
| counterAttackBirthGiveUp | 600 | CA:1997, 2377-2425, 2592 | U, T3, V1-3 | N | an unseen-birth guard | G | Y |

### ZoneTuning and script neighbours (13)

| Key | Default | Read at | Changed by | Q1 | Q2 | Q3 | Q4 |
|---|---|---|---|---|---|---|---|
| heatCapSpike | 40 | ZR:3243 | none | S | equal to heatFull: the same number twice | A:Aggression (keep = heatFull) | not in 16 |
| gmHeatDecayHeld (script) | 0.25 / 10 s | escalation.sqs:49-51 | none | S | no. It sets the ~27 min candidate window | A:Aggression | Y |
| gmHeatDecay (script) | 1 / 10 s | escalation.sqs:51 | none | N | no effect on the director: candidates are held zones, which use gmHeatDecayHeld | C | n/a |
| cityGarrison | 4 | ZR:751, 795 | none | S | it shadows retakeMin downward and sets G | island/author key | not in 16 |
| zoneArea | 150 | OF:1719, CA:1658 | none | W | no | Geo | n/a |
| baseRetake | follows occupierForces | ZR:757 | none | W | no | Geo | Y |
| captureRate / captureCrewCap / captureDecayDefended / captureDecayAbandoned (4) | 6 / 3 / 10 / 2 | ZR occCapture tick | U | W | no. How fast a base falls back | C | Y |
| budgetLiveBodies | 130 | CA:1949-1965, CAA:888, Traffic.cpp:1546, ZR:3462-3480 | U (12) | N | never bound (peak 46) | G | Y |
| budgetSideGroups | 40 | CA:2136, ZR:3656-3661 | U (13) | N | never bound (peak 8) | G | Y |
| emptyGroupRelease | 60 | ZR:3737-3807 | none | N | 3 groups released | G | Y |

gmForcesTune-only and test ranges: `meanStanding` (no config key), `wakeTowns` 1,
`directorInterval` 1..9, `refillHours` 0.01..0.49 and `lethalityMargin` 0..0.99. Then
`gmCounterAttackTune` `speed` (the speed lever, 14 tri uses) and `townsTaken`.
`forcesTokenInterval[]`, `forcesWaveThresholds[]` and `forcesWaveHub` have no
gmForcesTune entry. Not counted: the 7 faction-descriptor hull keys
(`forcesTruck` ... `forcesJet`), which are content, plus ForcePackages prices,
lethalities and speeds, which are already constants.

## The count

- **75 tuning keys**: 46 forces-family (43 OF + 3 CA), 16 counterAttack, 13
  zone/script neighbours. Every one of them is set only by tests, except
  `occupierForces`.
- **Strongly felt in a Malden hour: 18.** refillHours, tokenInterval (STRONG),
  townCooldown, heatFull, heatCapSpike, gmHeatDecayHeld, lethalityMargin,
  defenderWeight, airEta, wakeTowns, firstTownChance, rearmHeat (as on/off),
  poolPerTown, intelDecay, sourceKeep, liveTimeout, cityGarrison, occupierForces.
- **Felt rarely or late: 30**, mostly waves (first wave 44 min in), the HQ
  assault, push-outs, strikes, urgency and the first-town legacy answer.
- **Dead or shadowed at the defaults: 22.** poolMin, poolFloor,
  directorInterval, opsMax, opsMaxStrong, urgentCostShare, spareCapFloor,
  slowEta, slowCostShare, roadFactor, controlDwell, waveFinal, waveSurge,
  waveLeadFinal, waveMin, waveMax, liveHullsMax, log, counterAttackChance,
  counterAttackSpeed, counterAttackRepelLoss, gmHeatDecay.
- **Internal guards: 5** (birthMinToTarget, birthGiveUp and the three budgets).
- **Test-only by purpose: 3 keys** (directorInterval, controlDwell,
  counterAttackSpeed), plus 1 tune-only lever, 4 test ranges and 2 CA tune levers.

## The interactions that make keys dead

1. **Refill beats the band ladder.** At 4 h, 37 units/h flow back while a
   repel costs about 4. To leave STRONG the player must out-kill about 37
   occupiers an hour. Until then the STRETCHED/THIN/SPENT token intervals,
   poolFloor and opsMax do nothing (1 % of video 5 outside STRONG).
2. **Tokens beat the ops caps.** At one op per 600 s, born ops peaked at 2, so
   opsMaxStrong 3 and liveHullsMax 6 (3 x 2 hulls) never bind outside waves.
3. **airEta swallows slowEta and slowCostShare** whenever the cheapest leg is
   over 1500 s, and every slow leg seen on Malden was (2035 to 5157 s).
4. **Three 40s and two 1800s.** heatFull = heatCapSpike = rearmHeat = 40
   makes rearmHeat a boolean. intelDecay = pushoutQuiet = townCooldown = 1800,
   and intelDecay caps pushoutQuiet.
5. **Minimum coincidences.** spareCapFloor 6 = hqAssaultMin 6 = Harbour spare
   6 = Outpost garrisonMax 6. retakeMin 4 = cityGarrison 4.
6. **Two numbers for one requirement.** L_req = lethalityMargin x
   defenderWeight. Two road factors (1.5 quote, 1.3 march) for one leg.
7. **Urgent share vs prices.** A truck adds 4 units, so a foot retake under 8
   bodies can never upgrade at 1.5x; the arrival rule does that job now.

## Proposed consolidation: eight author levers

Each lever below is one `CfgGuerrillaZones` key, and a difficulty preset picks
one of its four values (Easy / Normal / Hard / Brutal; Normal = today's
behaviour). Every old field stays inside `OccupierForcesTuning`, derived from
the levers, and stays reachable through `gmForcesTune`, so tests keep working.

| Lever | Folds together | Default | Easy / Normal / Hard / Brutal |
|---|---|---|---|
| forcesCadence (s per op while STRONG) | tokenInterval[] = {c, 1.75c, 3c, 0}; townCooldown = 3c; tokensMax 2, directorInterval 60, wakeTowns 2 become constants | 600 | 1200 / 600 / 420 / 300 |
| forcesStrength (poolStart scale) | poolStart = max(24, s x (R + 2K)); waveShare stays a share of it; poolMin, poolPerTown, poolFloor become constants. The owner still has to decide whether strike prices scale with it (NEXT 1) | 1.0 | 0.75 / 1.0 / 1.25 / 1.5 |
| forcesRecovery (h, empty to full) | refillHours | 4 | 8 / 4 / 3 / 2 |
| forcesReach (s ETA before it rides or flies) | airEta = r; slowEta = 0.6r; slowCostShare 3, urgentCostShare 1.5 and one road factor 1.5 (quote and march) become constants | 1500 | 3000 / 1500 / 1200 / 900 |
| forcesAggression (a) | heatFull = heatCapSpike = 40/a; urgentHeat = 50/a (exit 0.7x); rearm on at heatFull; L_req product = 3.6a (margin x defenderWeight); firstTownChance = 0.4a; StrikeMinFighters 4/3/2/2 | 1.0 | 0.75 / 1.0 / 1.25 / 1.5 |
| forcesWaves (0..3) | thresholds, share, lead, retake share, the final stand set, warLevel step; controlMilWeight, dwell, ops, cluster, min/max and surge become constants | 1 | 0 or 1 with {0.75}, share 0.35, lead 900 / today / {0.8, 0.6}, share 0.6, lead 450 / {0.85, 0.65, 0.4}, share 0.75, lead 300 |
| forcesMinimums (bodies, m) | retakeMin = m; hqAssaultMin = spareCapFloor = m + 2; spareCap always on; alarm minimum m | 4 | 3 / 4 / 6 / 8 |
| forcesConcurrency (ops in flight while STRONG, n) | opsMaxStrong = n; opsMax = n - 1; liveHullsMax = 2n; counterAttackMaxLive = n - 1 | 3 | 2 / 3 / 4 / 5 |

These stay as author keys, outside difficulty: `occupierForces`,
`forcesWaveHub`, `forcesOffMapEdge`, the hull class keys, `forcesLog` (a
diagnostic), and the engine budgets (`budgetLiveBodies`, `budgetSideGroups`,
`emptyGroupRelease`). Everything else becomes a constant with a gmForcesTune
entry: intelDecay and pushoutQuiet as one 1800 s memory, liveTimeout, sortieTime,
sourceKeep, the birth guards. counterAttackChance, counterAttackSpeed and
counterAttackRepelLoss are key-off legacy and can go once the key-off path is
retired.

## Engineering cost

- **Code (about 1 to 2 days).** OccupierForces.hpp/.cpp: a lever reader in
  LoadFromParams that derives the fields, then reads any old key as an expert
  override with one WARN. CounterAttack.cpp: liveHullsMax and maxLive derived
  from the levers, and counterAttackRoadFactor merged with forcesRoadFactor
  (this changes how long a route-less march takes). ZoneRegistry.cpp:
  heatCapSpike tied to the Aggression lever. escalation.sqs / init.sqs:
  gmHeatDecayHeld either stays or moves native. OccupierForcesCommands.cpp:
  keep every field key and add the eight levers. A new .cpp file has to be
  listed in `Showcase.Abel/human/coverage.json`.
- **Unit tests that pin the old keys or defaults.** test_occupier_forces.cpp:
  the cases "defaults match the knob table" (line 236), "config keys are read
  and repaired" (263), "director knobs default to the knob table" (1669) and
  forcesLog (289), plus config strings naming forcesOpsMax (15 mentions),
  forcesOpsMaxStrong (5), forcesRoadFactor (5), forcesRearmHeat (4),
  forcesSlowEta (4), forcesAirEta (4), forcesTokenInterval (3).
  test_counter_attack.cpp: forcesLiveHullsMax (9) and
  counterAttackFirstTownChance (8). test_force_packages.cpp sets
  PackageRequest fields only, so it is unaffected. If the old keys keep
  reading as overrides, only the default-pin cases change.
- **Tri tests.** None change if gmForcesTune keeps the field names: there are
  19 files with directorInterval, 18 with wakeTowns, 7 with refillHours, 5 each
  with airEta and slowEta, and 29 gmCounterAttackTune firstTownChance uses.
- **Saves.** Tuning is not saved. It is re-read from the mission config on
  load (test_occupier_forces.cpp:1496), so no save compatibility is needed. No
  shipped description.ext authors an old key, so nothing has to keep reading
  for content either. Keeping the old keys as warned overrides for one release
  is cheap insurance for third-party templates.
- **Docs.** Rewrite OCCUPIER-FORCES.md section 16 into two tables (levers;
  constants and test levers). Fix OFFSCREEN-CONTRACT.md section 4, which lacks
  forcesRoadFactor and forcesLog and still says forcesControlDwell is "absent
  from [16]". Add a roadmap item.

## What to measure in the next playtest

Play on the shipped keys with `forcesLog = 1`, so every director line lands in
the RPT at INFO. Count these:

1. **Time outside STRONG** (`band STRONG ->` lines) against the player's kills
   per hour (sum of `Repelled at ... N lost` plus garrison kills) and the boot
   `refill 37.0/h`. Under 5 % outside STRONG means Recovery is the lever to
   move, and the band ladder is invisible.
2. **Launch spacing**: the gap between any two launches (expect about 600 s)
   and between two launches on the same town (expect 1800 s plus the token
   wait). Also ask: did attacks on a held town feel too frequent?
3. **Urgency in real fighting**: the share of launch lines marked `urgent`.
   The videos show none from real alarms, but capture plus RED should cross 50.
4. **Arrival rule**: every `arrival rule (...)` line with the cheapest
   seconds. Any cheapest leg of 900 to 1500 s is the only case where
   slowEta/slowCostShare matter. Note how often a helicopter answers.
5. **Packages against reported fighters**: the `lethality X of Y` pairs.
   Check that T72 escorts against 4 fighters read as right to the player, not
   as overkill (this is the Aggression lever).
6. **Born ops against the cap**: whether three operations are ever in flight
   at once (opsMaxStrong), and whether `spare cap:` fires on anything except
   the Airfield.
7. **First wave time and control** (`Wave 1 (N) inbound ... control C`):
   whether a human reaches 0.75 within the session, and whether the 600 s
   lead was noticed.
8. **Peak live AI and groups** in the `[gmsim]` lines with companions,
   traffic and civilians on, against 130 / 40.

Items 1, 2 and 5 confirm the three levers that matter most: Recovery, Cadence
and Aggression. Items 3, 4 and 6 settle whether urgentHeat, slowEta and
opsMaxStrong earn a place or become constants.
