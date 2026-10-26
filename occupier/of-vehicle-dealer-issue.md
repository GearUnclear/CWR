Owner request from the 2026-10-14 Malden playtest (Cancon start, occupier-forces branch): the campaign needs a vehicle dealer the player can actually find and use. Filed as the tracking issue; the shape of the fix is open.

## What exists today

- `Market` seeds WEAPON and VEHICLE dealers across the island's CITY zones, each kind on an independent draw over `dealerShare` (0.34) of the cities, so a given town hosts a vehicle dealer roughly one time in three and the player has no way to know which (`engine/Poseidon/Game/Guerrilla/Market.hpp`, `Market.cpp` PlanDealers). A vehicle dealer stands on a diagonal off-road spot inside the town and owns a LOT spot nearby where a bought hull is dropped. A killed dealer respawns after 600 s.
- Guerrilla.Abel's market block lists vehicle rows (Skoda, Rapid and the rest of `class Vehicles`, `guerrilla-mode/mission/Guerrilla.Abel/description.ext` line 287) and `vehicleDealers[]` can pin authored towns.
- The script side (`guerrilla-mode/core/scripts/market.sqs`) handles the buy, the debit, the createVehicle at the LOT spot and the garage lock; `Market.Abel` is the reference slice. Dealer markers follow the body since e8ef485.
- Known gaps already on file: dealer respawn drift (#97), Sinai dealer cap (#75), the Southeast Compound is an island with no boat package (#97 content items), and the owner's note that the vehicle system is not fully developed yet.

## What the playtest showed

In a Cancon start the owner could commandeer a passerby's car and lock it in the garage, but did not come across a vehicle dealer and asked for one as a tracked item.

## Questions to settle before building

1. Discoverability: should every campaign guarantee at least one vehicle dealer within reach of the start (a pinned dealer in the start town or the nearest city), should dealers be marked on the map once a town is visited, or should the journal or a civilian tip point to the nearest one?
2. Placement: one dealer per island region, one in every city, or the current random third? The weapon dealer has the same question.
3. Stock and delivery: is the current list (civilian cars) the intended stock, or should captured military hulls, trucks for the forces, and boats (the Compound) be on offer? Delivery at the LOT spot versus to the garage.
4. Whether this folds into the vehicle system work the owner has planned, or lands first as a small discoverability change (pin a dealer to the start town, mark it on the map).

Related: #97, #75, #94 (HQ respawn), the garage in market.sqs.
