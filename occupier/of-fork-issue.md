Follow-ups left open by the occupier forces work (PR #95, `arma_CWR/guerrilla-mode/OCCUPIER-FORCES.md` section 17). Each is small on its own; none blocks the PR. The plan doc's section 17 and the status table at its top are the source of truth; this issue is the tracking list.

## Design items for the owner

- **The K count.** The pool sizes K from every administered non-exempt CITY; the plan's settlement probe (5.1) is not run by the engine. Decide whether the probe returns or the doc drops it (the doc now says what the code does, with an open note).
- **Alarm answer timing.** An alarm answer to a zone inside the player's bubble is born at its base at 0 % progress (3.1 step 1, D3) and may take 30 minutes on foot. If 8.2 wants a faster answer, the launch-time birth could prefer the hidden route point nearest the target; that is a design change, not a bug.
- **Player messaging (D12).** The director writes one INFO line per action and `gmOccupierOps` lists operations; no hints or journal lines yet.
- **Respawn at the HQ** is #94.

## Engine items

- A live top-up squad that reaches its town is adopted whole, with no `garrisonMax` cap (an AI group cannot be split).
- The air re-plan's helicopter price is paid to and refunded from the pool directly, so the director's ledger (`gmOccupierOps`) does not show it.
- Review LOW items still open: L6 (PruneOps past 32 rows), L8 (phase-2 save with empty `_townsEver`), L10 (`LastTickPlayer` after a load), L11 (the 3 x 3 release grid sampling), L13 (routing traps), L14 (test-lever gaps), L15 (8f9482a does not build alone, breaks bisect). Details in the phase 1 to 3 review that produced them (recorded in the plan doc's section 18).
- A slow native alarm answer can still be travelling when a later showcase capture chapter runs; `showcase/chapters/capture.sqs` only checks `GM_QRF_ACTIVE` for a leftover QRF.
- `ZoneRegistry`'s descriptor lint now lists the `forces*` keys (phase 7); confirm the WARN path on a descriptor that names a class missing from the Remaster or @LoBo.

## Content items

- Malden's Southeast Compound stands on its own island (about 800 m of deep sea to Le Port). It is an air-only source now; its west and south gates are closed wire gates (the template comment says open). Decide whether it stays a base, moves to the main island, or gets a boat package.
- The occupation network per template (#76): airfields, harbours and outpost ladders decide where air comes from and which bases are hubs for waves.

## Tests

- `guerrilla_forces_watched_route` asserts the joined route, the re-source and the birth, not the arrival.
- The armoured-column mid-route save/load `.seq` was skipped for time.
- Not run locally: the LoBo-lane civilian consumers and the Demo-world `guerrilla_save_reload.seq`.
- Human session (12.3): pacing, watched arrivals, helicopter landings and frame time, whether HQ loss reads as a setback and the loss as a defeat, air strikes on towns.

Related: #77, #35, #72, #79, #86, #94, #76.
