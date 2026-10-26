Third close-out pass (commits e8ef485, 8380ab2):

- **The air re-plan goes through the director ledger** (`OccupierForces::SpendOnOp` books the airframe on the operation's row; it settles like any hull). Closed.
- **Dealer markers follow the body** (`Market::SyncDealerPos`; the record and marker sit within 0.5 m of the dealer; `market_reference_mission` asserts 1 m). Closed.
- **Script player handles after a body-rebuilding load**: the save records which globals hold the player body (`playerOnVars`) and the engine repoints them after `RebuildPlayerBodyAfterLoad`, so saves made before the campaign.sqs rebind recover too (proved with the pre-change scripts installed). Closed.
- **Crew settle off the ledger one for one** (`CAOpCrewSettled`): a fully settled row reads 0, the forced prune never refunds crew, legacy rows reconcile on load. Found while closing the ledger item; closed.

Still open: alarm answer timing beside the player (3.1 step 1 by design), player messaging (D12 deferred), the Compound as an island, the occupation network (#76), the human session (12.3), `editor_wizard_finish` (needs packages/Game), `objNull distance <pos>` raising instead of returning 1e10 (upstream command), a dealer respawn starting from the last body position (drift of a few metres per death).
