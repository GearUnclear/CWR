Owner decisions of 2026-10-14 (commit 8909838):

- **Alarm answers beside the player**: kept as built. Enemy countermeasures spawn at their base and travel at real speed (plan 3.1 step 1). Closed.
- **Player messaging (D12)**: stays deferred. The director's and counter-attack's INFO lines are now dev-only (`--dev`, or `forcesLog = 1` in the mission; `gmForcesTune "log"`), DEBUG otherwise. Journal integration is tracked in #98 with the how left open. Closed here.
- **The Southeast Compound**: stays an air-only base until the vehicle system work (boats are a later item). Parked on that work.
- **Occupation network per template (#76)**: the owner asked whether it can be auto-detected. Answer: largely yes. Airfields from the world's ILS table (what landing aircraft already use), seaports from the coastline plus pier and dock objects inside the zone radius, hubs already derived (the strongest base, airfield or seaport); an authored override stays for intent (which of several candidates is the wave hub, outpost ladders). Awaiting the owner's go to build it as the #76 slice.

Remaining here: the human session (plan 12.3), `editor_wizard_finish` (needs packages/Game), `objNull distance <pos>` raising (upstream), dealer respawn drift.
