# Occupier forces: owner summary (draft, numbers filled at the final gate)

## What the occupier does in play now

- The occupier has one budget, a pool of soldier-equivalents (about 60 on Malden at the start). Garrisons regrow from it, retaken towns are refilled from it, and every operation is bought from it. It refills slowly (empty to full in about four hours) and jumps when one of three reinforcement waves lands (at 75 % control, 50 %, and a final stand). A starved occupier is visibly weaker: smaller garrisons, lower morale, no armour or air.
- A director wakes when the second distinct town falls. It only knows what its men saw (sightings, alarms, heat, flips), never where the player really is. It spends a small command budget on the hottest towns, on the player's bases and quiet towns when strong, and with a deliberate force on his HQ. For each operation it buys the cheapest package that is lethal enough, the fastest one when urgent: a foot squad, a truck, a truck or APC with one armoured escort, a helicopter insertion, a jet or attack-helicopter strike.
- Every force is born out of sight (at its base or farther back, never within 250 m of the target) and moves at its real speed. A watched road means it starts farther back and the player watches it arrive. Trucks brake and dismount at the town edge; escorts drive up and support; helicopters land 300 to 500 m out, in plain view if they must, unload and leave.
- Any enemy standing in an empty liberated town or base takes it back through the capture meter, a passing column too. A held town weakened by a passing enemy stays weak until the player or resistance fighters are present.
- Only a deliberate assault takes the HQ, through a meter at the HQ itself. The HQ is then destroyed with its cache and garage vehicles, and the player may set up a new one anywhere, even in an enemy town (which keeps its garrison).
- An air strike on a held town cuts both sides' standing there sharply, panics the civilians and halves the town's income for twenty minutes.
- Alarms are answered natively by the director (foot, truck, column or helicopter), replacing the scripted QRF. Old saves keep the scripted QRF and the old heat cooling.
- The campaign is lost only with no HQ, no base and no town, after it once had something: a lose debriefing, saves kept. The win latches only when every enemy military zone is held at once.

## What is parked

See the fork issue (follow-ups) and the plan doc's section 17: the K settlement probe, the alarm answer timing, player messaging, the top-up adoption cap, the air re-plan price in the ledger, seven LOW review items, the Southeast Compound (an island: air-only source), the occupation network per template (#76), the HQ respawn (#94), the human session.

## Human playtest on Malden

Launch (from `D:\Arma_CWA\arma_CWR`):

```
& 'C:\dev\vcpkg\downloads\tools\powershell-core-7.6.2-windows\pwsh.exe' -File .\play-guerrilla.ps1 -GameDir D:\Arma_CWA\tmp\remaster-shadow-67 -GameExe .\dist\x64-win-rwdi\PoseidonGame.exe -Dev
```

The launcher needs PowerShell 7 and pwsh is not on PATH, hence the explicit path. Keep the same flags for every launch (no `-LoBoDir`, so saves never take the body-rebuild path). Pick EAST as the occupier so the aircraft match the steps below. Do not let any agent run a gate during the session: a gate rebuilds `dist` and reinstalls shadow-67 under the running game.

New game, GUERRILLA, Malden (Guerrilla.Abel). The director is on by default. `-Dev` turns the director INFO lines on; they go to `arma_CWR\logs\game-<ts>.log`, not the screen (`gmForcesTune ["log", 1]` at run time does the same). On screen you get hints only for a lost hold, town or base; operations, waves, alarm answers, strikes and HQ loss print nothing (HQ loss: the marker vanishes and the "Set up a headquarters" objective returns). Dev console commands to watch it: `gmOccupierForces` (pool, band, control, waves, tallies), `gmOccupierOps` (every operation with kind, package, source, target, phase), `gmOccupierIntel` (what the occupier knows), `gmCounterAttacks` (records, hulls, escorts, air; the LZ is slot 25), `gmHqState`, `gmZoneSim (gmZoneIndex "<zone>")` (occCapture, standing, struckUntil), `gmZoneStruck (gmZoneIndex "<town>")`, `gmRoadRoute [from, to]`, `gmLandmass [pos]`. Steps 10 and 11 will not be reached at the real knobs in one sitting; use `gmForcesForce ["wave", n]` and `gmHqForceCapture 100` if you want to see them. Armour needs a stored spare hull at the source base, so it may not appear naturally; `gmForcesForce` can buy one. Step 9's legacy branch needs a pre-branch Abel save; without one it can only be shown with a lever.

1. **Pacing and the first town.** Take one town. About 40 % of the time an immediate answer comes from the nearest base; otherwise heat decides. Watch `gmOccupierOps` for the record and its INFO line in the log.
2. **The director wakes.** Take a second town. The pool line in `gmOccupierForces` shows tokens; operations now target the hottest town. Leave a town empty and watch an enemy squad or a passing patrol retake it (the drain, no ownership filter).
3. **Watched arrival.** Stand on the road between a base and your town with binoculars. The force is born beyond your view and walks or drives in; a truck brakes at about 250 m and the squad dismounts. Nothing appears within 250 m of the town or beside you.
4. **Armour.** Hold a town with four or more fighters so the required lethality rises; when the pool is STRONG the director buys a truck with an APC or tank escort. The escort drives to the town edge and fires; the truck stays back.
5. **Helicopter insertion.** Hold a town far from every base (or watch every road). The force re-plans as an insertion: a Mi17 from the Airfield (or off-map) lands 300 to 500 m out, unloads and leaves. `gmCounterAttacks` shows the LZ and the airframe.
6. **Base retake.** Take a base and leave it empty while the pool is STRONG; a BASE operation retakes it through `occCapture` (`gmZoneSim`). Hold it with a squad to force a live fight.
7. **The HQ.** Draw heat near the Camp. A deliberate HQ_ASSAULT force arrives and fills the HQ meter (`gmHqState` slot 3); patrols through the Camp do nothing. The HQ is destroyed, the garage jeep wrecked, the market marker gone; re-establish in another town through the usual flow.
8. **Air strike.** Hold a town with three or more fighters reported and let it go URGENT while the pool is STRONG. A Su25 strikes: support and standing in the town fall by 40, nearby towns by 10, civilians panic, the economy tick halves there (`gmZoneStruck`).
9. **Alarm answer.** Get spotted in an occupier town until it goes RED. The director answers with a package born out of sight (`gmOccupierOps` kind ALARM); no scripted QRF appears. Load a pre-branch save to see the scripted QRF still used there.
10. **Waves.** Push control below 75 %: a wave lands at the hub (airfield, seaport or the strongest base) and strikes the densest cluster of your towns, able to retake a base; no win while any enemy military zone is held. Below 50 % the second; the final stand when the occupier holds no base or town but the HQ's.
11. **Lose.** Lose the HQ while holding nothing: the lose debriefing.
12. **Frame time and landings.** During insertions and columns check the frame counter; report any landing more than 100 m from the LZ and any force that stalls.
