# Occupier director: does it read as a competent commander under strain?

Read-only assessment, 2026-10-16, branch `occupier-forces` at 4b9e4b7. Evidence: the five video
runs (`tmp/of-director-video`, `-2`, `-3` A and B, `-4`, `-5`: stats, traces, launch lines), the
progress log (owner playtest of 2026-10-14), `OCCUPIER-FORCES.md` sections 0 to 2, 5.6, 6.3 to 6.5,
7 and 16, `tmp/of-generated-bases-design.md` section 1, acf2311 and `OCCUPIER-FORCES-NEXT.md`.
Key tuning values are the sibling audit's; they are named here only where a recommendation needs one.

**The owner's question, as clarified.** Not "is it soft enough" but: from the ground, do its choices
look like a capable commander's under strain? Is it intelligent (right target, right force,
learns from a repel, concentrates, uses air where it pays, retakes what matters first)? Is it
reasonable (proportionate, not omniscient, no absurd moves)? Difficulty stays where the owner set
it (acf2311). "Strained and far from home" is the fiction the behaviour should read through, not a
reason to weaken it.

**Caveat on the evidence.** The scenario runs use levers: the player and the cell's fighters are
invulnerable, time runs x8 (x4 while a force is live), captures are scripted. So the repel counts
and "4 lost" per fight are staged. The director's *decisions* in response are the real shipped code,
and those are what this note judges.

## Verdict in one paragraph

On empty ground the director already reads well. Every empty town or base it went for was
reoccupied within 2 to 50 minutes, by the right sized package, from a base it could actually reach.
It honours D9 (no omniscience was seen in any run), and since round 3.5 it never idles while it
has a target. It reads badly against resistance. It feeds the same minimum force along the same
road from the same base into the same defended town every 32 minutes. It never escalates, never
switches approach, never gives up, and never concentrates. Meanwhile its helicopters go to empty
towns. Its band machinery, the part meant to show strain, is almost never seen: in about 14 hours
of game time across five runs the band sat outside STRONG for 11.7 minutes (1.4 %), and every one
of those dips was a transient airframe or strike debit, not a loss. A company of infantry died at
two towns in video 5 and the occupier's posture did not change at all. The fix is not a weaker
occupier. It is a director that remembers what failed, and a band that measures losses.

## 1. Strain: short of men, or a bottomless machine?

**Numbers (video 5, the shipped tree: 3 h 36 min, 22 operations).**

| Measure | Value |
|---|---|
| Operations per game hour | 9, 4, 6, 3 (about 6 an hour; the 6.3 STRONG table says "up to 6") |
| Package mix | TRUCK 6, APC 5, TRANSPORT_HELI 5, FOOT 5, JET 1 (the jet was the test lever) |
| Origins | director 14 retakes + 2 base, wave 2, push-out 3 |
| Sources | Le Port Harbour 12, Outpost 8, Airfield 1, Compound 1 |
| Targets | Le Port 8, Arudy 5, everything else 1 or 2 |
| Repels | 11 of 22 (every one at Le Port or Arudy) |
| Infantry lost at the two defended towns | about 47 (Arudy 23 of 25 sent, Le Port 24 of 28) |
| Pool, min/max per 30 min | 39/60, 37/69, 41/69, 37/63, 41/63, 29/63, 40/62, 50/60 |
| Time outside STRONG | 2.3 min (two insertion and strike debits) |

The earlier runs agree. Video 4: 16 ops in 2 h 18 min, 0.6 min THIN (a strike opener). Video 3 B:
11 ops in 1 h 31 min, 4.1 min THIN. Video 3 A, the pre-retune tempo: 8 ops in 3 h 8 min, 7 of them
on foot. Video 2: 10 ops in 3.5 h, never left STRONG.

**Why the band never moves.** The pool, as built, mostly measures hulls that are out. Foot
infantry is paid from a base's stored reserve, and the pool repays that through regrowth (at most
1 body per base per 600 s, 18 an hour across three ground bases). The refill is 37 an hour (5.2).
Hulls are refunded when they get home. So the pool falls only while airframes are aloft, or when a
vehicle is destroyed (a T72 column is 21 units, a third of the pool). Under video 5's losses it
cannot fall at all. That is the 6.3 arithmetic working as designed: "a STRONG pool survives a worst
hour at 55 of 60". But it means a three-fighter cell that kills 47 soldiers changes nothing the
player can see. To the man on the ground that reads as a vending machine, whatever the pool bar
says.

**Where acf2311 made it richer.** It fixed the dead air. Before the retune (video 3 A), most
answers were 4-man walks and there were 20-minute silences. After it there are trucks, APCs, the
T72 escort and insertions, and with re-arm and the arrival rule (round 3.5), no idle STRONG
minutes. Variety of hardware went up a lot: a player now sees motorized columns, not files of
walkers.

**Where it made it a vending machine.** Le Port got the same Motorized squad (4, Ural) with T72
from the Harbour, born at the same point ([8275, 3397], 62 % along a 739 m road), seven times. The
launches came at 241, 2643, 4575, 6495, 8410, 10871 and 12792 s: about every 32 minutes, because
the rest is 1800 s and re-arm launches the instant it ends (7 of 9 re-arms had a 0 s delay).
Arudy: the same APC (4, BMP) from the Outpost, born at [5812, 7037], five times, lethality 11.00
against a required 10.80 every time. The tempo is fine. What makes it a machine is the sameness,
and the zero cost of failure.

**What already reads as strain.** Spare is thin (the three ground bases hold about 20 spare bodies
when full), and the spare cap sends 6 to a base that asks for 12, which reads as a commander making
do. The Compound is air-only. The wave lands at a harbour after a 10-minute lead. Strikes cut
standing and informants dry up after a bombing. These are good bones. None of them reacts to the
player's success.

## 2. Far from home: what says expeditionary, what is missing

**Present.** The theatre pool and its slow refill (full in about 4 hours). Waves that a parent
nation sends when control drops, landing at the harbour or airfield hub. Off-map air at 1.5x price.
Bases on one landmass and an air-only island. Per-town standing that bombing burns. An informant
network that works only where the occupier is tolerated. The director knowing only what its men saw.

**Missing, in order of how much it would show on the ground.**
1. *Memory of failure.* A repel adds +15 heat and a 30-minute rest (`OccupierForces.cpp`, the
   `CAOpRepelled` case near line 3405). Nothing else. No repel count, no escalation, no change of
   approach, no point at which it gives up.
2. *A cost for losses.* Dead men refill from the theatre at the same rate whatever the cause. There
   is no political or morale price. That is why 47 dead change nothing.
3. *Posture changes.* Garrisons never hunker, bases never consolidate, a far town is never
   abandoned. The occupier never reinforces what it holds instead of attacking. Its only verbs are
   "retake" and "regrow".
4. *Lines of supply.* Regrowth is an unseen deposit. There is no convoy the player can cut to
   starve a base (Traffic already has military convoys, but they are scenery).

**Real occupations, only where they suggest a mechanic.**
- *Soviet motor-rifle regiment in an Afghan province (1980s).* Platoons hunkered in fixed posts
  (zastavas); the regiment mounted a few large combined-arms sweeps with heliborne blocking groups
  rather than a steady trickle; road convoys were the ambush target; airframe losses made aircrews
  cautious. *Texture:* mass, then one big operation; the convoy as a target; caution after an
  airframe loss.
- *IDF in the south Lebanon security zone (1985 to 2000).* Hilltop outposts, raids and a retaliatory
  strike after casualties, and a casualty-sensitive public at home that finally forced withdrawal.
  *Texture:* losses carry a political cost that, over hours, contracts posture (fewer far towns,
  more bases). It fits the repo's own Lebanon80 template.
- *Rhodesian Fireforce.* Few helicopters. They went on a fresh, confirmed sighting from an
  observation post, as a quick vertical envelopment, never to reoccupy an empty village. *Texture:*
  insertions go where fighters were just seen.
- *US forces in a Vietnamese district.* Firebases. Air and artillery on contact. Villages held by
  day and contested by night. *Texture:* fire support after contact, not as an opener; a day and
  night posture.

## 3. Intelligence and reasonableness from the ground

**Intelligent, today.**
- *Right package for empty ground.* Village, Chapoi, Outpost, Dourdan and Airfield were reoccupied
  by foot, truck or insertion sized to garrisonMax, with no strike wasted. The opener rule
  (3c9947b) stopped jets on empty bases.
- *No omniscience.* Arrivals counted real defenders and the director's targets came from
  sightings and tips. The HQ fell to a force that "arrived abstract at the HQ (holds the empty
  target)", 3676 m from the player, which is fair if the player was told a wave had landed.
- *Proportion.* Lethality is sized to reported fighters (`L_req = 1.2 x 3 x known`), so it does
  not send a tank company at two riflemen.

**Not intelligent, today (ranked by how visible it is).**
1. *Piecemeal into a defended town.* See section 1. A commander who loses a squad at Arudy does not
   send the same squad down the same road 30 minutes later. He sends more, or comes another way, or
   calls fire, or stops. The director does none of these, five times in a row.
2. *Thin assault infantry.* A package meets `L_req` mostly through hulls (a T72 is +12), so 4
   riflemen assault 4 dug-in fighters with a tank parked at the edge. When the riflemen die the
   tank drives home intact ("4 lost, 0 going home"; the hulls are refunded). From the ground this
   is the strangest picture in the traces: armour that runs from four men with rifles.
3. *Air goes to empty places.* All five insertions (ops 3, 4, 10, 17, 18) flew to targets with
   "0 fighter(s) reported". They went because the foot quote was slow and the arrival rule takes
   the *fastest* package within 3x the cheapest (`ForcePackages.cpp`, the choice near 720 to 800).
   Every defended target got ground packages. That is the Fireforce rule inverted: the scarcest
   asset reoccupies empty villages, while the stubborn town gets the cheap answer again.
4. *No concentration.* One record per target (`FindByTarget`) and the spare cap mean every
   operation is a one-base force. With three born slots it spreads across three targets rather than
   massing on the one that matters. The wave did the same: 30 men landed, then 6 men went on foot
   to the HQ, 9 by truck to Arudy, 4 to Dourdan, and 11 stayed in the pool, 9 of them later dropped.
5. *Priority by heat, not value.* The hottest candidate wins. Re-arm holds a repelled town at heat
   40, so the stubborn town wins again at the end of each rest, ahead of bases or the insurgent
   HQ. Le Port next to the Harbour deserves priority. Arudy, 2.2 km out and repelled four times,
   does not.

**Unreasonable, today (absurdities a player would notice).**
- The wave's HQ assault walked 41 minutes (2450 s) from the harbour where trucks had just
  unloaded it (Q5).
- A lone survivor stood 47 m from Arudy for 18 minutes, twice (ops 12 and 15), until the 1200 s live
  timeout (Q2).
- Clockwork: identical launches 32 minutes apart from identical birth points. A player will find
  the spot on the second visit and farm it.
- Wave surplus vanishes ("7 unit(s) over poolStart dropped ... every occupier base at
  garrisonMax") in the middle of a fight the occupier is losing at Arudy (Q3).

**Fairness and legibility.** The rhythm is learnable, even too learnable (above). Answers arrive
at a pace a player can meet: road 74 s from the Harbour to Le Port, 256 s from the Outpost to
Arudy, insertions in 1 to 2 minutes. Repels are rewarding only as kills; they buy no lasting
change. Where a player would feel cheated:
- an HQ lost to a wave nobody announced (D12 defers messaging, #98 journal);
- a town struck the moment it was taken (the rule needs URGENT and 3 reported, so this is rare and
  fair);
- a garrison that regrows unseen (D2 deposits are invisible by contract: correct, but the player
  never learns that a base he hit is refilling).

Where bored: the same Ural and T72 for the seventh time. The owner's own playtest adds two items
that are not the director's but read as "the world is dumb": a traffic car ramming a sign, and no
findable vehicle dealer (#99), so the player walks while the occupier drives. The Camp marker on a
Cancon start is confusing but harmless (`IsMilitaryType` excludes CAMP, so it is never a push-out
target).

## 4. The bands as a feel

| Band | What the code changes (5.6, 6.3, 7.3) | What the player sees today | What it should read as |
|---|---|---|---|
| STRONG | 600 s tokens, 3 born ops, push-out on towns and bases, base ops, strikes | everything above, 98.6 % of the time | can afford mistakes: probes, push-outs, the odd strike |
| STRETCHED | 1050 s tokens, 2 ops, no push-out, no base ops, no strikes | about 2 minutes per run, during a sortie | hoards: fewer and bigger operations, near targets first, no far-town answers |
| THIN | 1800 s tokens, floor 20 %, cheapest alarm answers, garrisons stop chasing YELLOW | 0.6 to 4 minutes, after a strike opener | turtles: pulls far garrisons back, defends bases, no air |
| SPENT | nothing launches; regrowth first | never seen | about to lose: consolidates to the hub, checkpoints empty |

The bands exist as tempo, not as posture. And the input is wrong for a feel: the pool dips
while airframes are aloft, and does not dip when men die. Two changes are needed. The band should
read what was *lost* (bodies and hulls destroyed, over a window), not what is committed. And each
band should change something the player can see on the map: patrol density, garrison posture,
which towns get answered, air use. The tempo key alone is not enough.

## 5. Recommendations, ranked by perceived intelligence per day of work

Difficulty note. Items 1 to 3 make each attempt on a defended town harder and the attempts fewer.
Items 7 and 8 shift effort, not totals. None lowers the refill, the token cadence or poolStart.

### Defaults and small code conditions (hours each)

1. **Wave strikes take the arrival rule** (Q5). Add `CAOWave` to the origins the rule covers
   (`OccupierForces.cpp` near 2161, doc 6.5 and 7.1). Any band. The player sees the wave's HQ
   assault arrive in trucks within minutes of the landing, instead of a 41-minute walk.
2. **The slow rule takes the cheapest package that meets the ETA, not the fastest** (with the
   sibling audit's numbers). Ground first under `forcesSlowEta`, and an insertion only past
   `forcesAirEta` by road, or on a target with fighters reported, or across landmasses
   (`ForcePackages.cpp` choice, 6.5). Any band. Helicopters stop landing on empty villages and
   start showing up where the fighting is. A sortie also stops demoting the band.
3. **Jitter the rest and the re-arm launch.** A hashed +-25 % on the 1800 s rest per zone and
   repel, and a 0 to 300 s delay after re-arm (6.3). Any band. Kills the 32-minute clockwork. The
   rhythm stays learnable, but not to the second.
4. **Housekeeping (Q6).** `deleteGroup` dead hold groups when the zone is lost (`capture.sqs`). No
   feel change; frees ids.

### Behaviours to add (about a day each)

5. **Repel memory and an escalation ladder** (largest single gain). Keep per zone a saved repel
   count and the last package (like the `Rearm` name list). Reset it on a capture, or after about
   2 hours with no repel. The ladder:
   - *Repel 1:* the next answer needs `L_req x 1.5` and at least `2 x knownFighters` infantry,
     not just hull lethality.
   - *Repel 2:* change approach: another source, or an insertion on a reported target, or the
     next-best unseen route point. A strike is allowed if it keeps STRETCHED (Q1).
   - *Repel 3:* contain. Rest x3, push-out skips the town, and the nearest occupier base takes a
     temporary surge above garrisonMax (item 8).

   This lives in the `CAOpRepelled` case (`OccupierForces.cpp` about 3405), `PlanStrike`, the
   package choice, and new 6.3 and 16 rows. Any band (STRETCHED and THIN skip repel 2's air). The
   player sees Arudy hit once, then harder from another direction, then, if it holds, left alone
   behind a reinforced Outpost: a commander who learns.
6. **Mass instead of trickling.** When the escalated minimum exceeds every base's spare, hold the
   token and log "massing at Outpost" until regrowth covers it. Don't launch the under-strength
   package (`TryLaunch`, 6.3). STRETCHED especially. Fewer, bigger, visibly prepared assaults. A
   player who scouts the Outpost sees it fill.
7. **Break off a failed assault** (Q2). When a live operation's living infantry is below a third
   of its launch bodies, and the town is still held 120 s after the last casualty, the survivors
   withdraw toward the source and go abstract once unseen (the CounterAttack live step). The hulls
   then withdraw with them, never before them. Any band. Fixes both the 18-minute lone survivor
   and the tank that leaves before its infantry.
8. **Wave surplus as a decaying surge** (Q3). Bank the excess over garrisonMax, up to +50 %, at the
   bases nearest the resistance cluster. Let it decay back over about an hour (the "went home"
   fiction becomes a rotation the player can watch). Drop only what remains (the ledger banking of
   df9457e). The player sees bases visibly thicker after a wave, which is "reinforce what it holds".
9. **Value-weighted candidates.** Score heat x value (own lost base 3, town within 1 km of an
   occupier base 2, HQ 2, far town 1) / road minutes (6.3 candidates). Any band. Bases and
   base-adjacent towns get answered first; far towns wait or get contained.
10. **The band reads losses, not sorties** (Q4). Hold the band over an airframe's sortie, as
    `BandHold` does for a strike: committed is not lost. A hull destroyed, or bodies killed in the
    last hour above the hour's refill, do step it (5.6). The player sees the band change only
    because of what he did.

### Systems to design (round 4 or later)

11. **Band postures.** STRETCHED: no answers to towns beyond about 2.5 km by road unless urgent,
    and patrols halved. THIN: the farthest held town garrisons are evacuated by truck in view (the
    town becomes empty, "the occupier left Goisse"), no air, alarm answers only at bases. SPENT:
    pull in to the hubs, checkpoints empty. This needs Traffic patrol density and garrison posture
    hooks.
12. **Home pressure** (the Lebanon texture). A theatre-wide will value that falls with casualties
    and lost hulls and recovers slowly. Low will narrows the set of towns the director bothers
    with (contraction), and a wave lifts it. This gives the strained fiction a visible arc: early
    the occupier contests everything, late it holds a core.
13. **Supply by convoy.** Make D2 base regrowth travel as a hub-to-base truck run (Traffic's convoy
    already exists). A cut convoy starves that base for a refill period. Ties to generated bases
    (round 4).
14. **Legibility before #98 lands.** Resistance-side intel symmetric to the occupier's informants:
    "a ship at Le Port" during a wave's 10-minute lead, "trucks leaving the Harbour", "the Outpost
    is massing". Most perceived intelligence is lost today because the player never sees the
    reasoning. D12 defers this; recommend it is the first journal work in #98.
15. **Two-source operations** (rewrite of the one record per target rule, 7.2): a pincer from two
    bases. Only after items 5 and 6 show that one-source massing is not enough.

## 6. The six tuning questions, resolved from this view

1. **Strike on a 60 pool.** Keep the keep-STRONG rule for an opener. Allow a strike that keeps
   STRETCHED only as escalation on a town with 2 or more repels and 3 or more reported (item 5).
   Do not scale prices down: air is the expensive, far-from-home asset, and its rarity is in
   character. A commander calls it after ground has failed, not as routine.
2. **Lone survivor.** Withdraw. A broken assault falling back is the most in-character thing it
   can do (item 7).
3. **Wave surplus.** Dropping is acceptable fiction, but it wastes the best visual a wave has.
   Prefer the decaying surge (item 8).
4. **Insertions demote the band.** No. Committed assets should not move the band. Hold it over the
   sortie, and let a shot-down airframe demote it (item 10). Item 2 also makes this rarer.
5. **Wave strike walking 41 minutes.** Lift the exemption (item 1).
6. **Dead hold groups.** Delete them (item 4); no effect on feel.

## 7. What this view does not change

Keep the playable rate (acf2311), the pool size, the refill, D9's intel model, the unseen-birth
rule and the arrival rule. The occupier is already as dangerous as the owner wants. What it lacks
is a commander's memory and posture. Items 1 to 3 cost hours. Items 5 to 7 cost about three days
and remove most of what currently reads as unintelligent in the traces. Item 14 is what lets a
player see any of it.
