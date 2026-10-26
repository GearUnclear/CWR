The occupier director (OccupierForces) and the counter-attack service write one INFO log line per action (OCCUPIER-FORCES.md D12: "Player messaging is deferred. Each director action writes one INFO log line, and a gm* command lists operations. No hints or journal lines from the director yet."). PR #95 lands those lines behind a toggle that silences them outside dev runs and test missions.

This issue is the player-facing side: how the journal (the map-screen diary, `Journal` / `JournalCompose*`) should surface what the occupier is doing, without breaking D9 (the player learns only what his own people could know: a column seen on the road, a helicopter heard, a town struck, a base retaken, rumours from informants).

Open questions, deliberately undecided here:
- Which events reach the journal at all (a strike on a held town, a base lost, a wave landing, an alarm answered), and through what in-world source (a resident, a fighter, a radio intercept, the market dealer).
- Tone and timing: immediate lines versus a nightly digest, hints versus facts.
- Whether `gmOccupierOps` / `gmOccupierIntel` stay dev-only or get a player view.

Related: #77, #95, #97, the `Journal` notepad work (#61).
