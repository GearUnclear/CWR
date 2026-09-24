# Naming feature: issue #57 acceptance

Audit date: 2026-09-24. Source requirement: [Naming System: Friendly and Enemy Legends](https://github.com/GearUnclear/CWR/issues/57).

**Status: implementation fixes verified by source tests; runtime acceptance pending.**
PR #61 merged the original registry, dossier and enemy commander implementation.
This audit checks that implementation against the original issue rather than
treating the merge or the older design notes as proof of completion.

## Requirements and evidence

| Original requirement | Current implementation and verification | Remaining acceptance |
| --- | --- | --- |
| Prefix, regional first name, describer, regional surname, title in that order; optional nickname slots | `LegendNames` assembles all five slots and preserves the issue's quoted describer and title capitalization. Its unit cases verify formatting, all bank entries and deterministic selection. | None for string assembly. |
| Use the attached name bank | The downloaded `test.json` matches `tests/fixtures/legend-names/issue57-names.json` as parsed JSON. `gen_legend_names.py --check` verifies compiled tables against it. | None for the bank transcription. |
| Regional personal names | New companion rows draw both personal names from the resistance region. The all-pools registry regression checks membership for both slots across all 33 pools. Script roster keys remain stable; loaded resolved identities stay unchanged. | Exercise Classic and LoBo resistance choices in game. |
| Earn one nickname at a level threshold; max level adds a different slot and Legend status | SERGEANT / 250 XP earns the first slot; COLONEL / 1900 XP earns the second. Tests cover both thresholds, a jump across both, repeat observations, and save/load award latches. | Run `scripting/guerrilla_legend_names` and `scripting/guerrilla_legend_save.seq`. |
| Earning a name feels like an event | The native poll sends the resolved award text to the normal hint UI and writes an attributed journal entry. Tests verify both announcements, ordinary promotion announcements, and silence on repeated observations. The script no longer overwrites promotion/award hints. | Observe/capture both award hints at normal gameplay speed. |
| Names recur in game and persist after death in records/memorials | `BindRow` sets the live body's name; journal gathering and the persisted companion status line read the same registry display name. Registry tests cover awards, death, retained deeds and reload; journal compose tests cover rosters, memorials and records keyed by character id. The naming integration lane now checks the real memorial and fallen dossier. | Run that lane and inspect labels, roster and memorial rendering. |
| Three dangerous enemy Legends each playthrough | Three distinct identities; sniper, elite commander and tank commander where faction assets permit. Skill is 0.9, with guards/crew. Placement now recovers a feasible triple when preferred stands block the other two. Failed creations with no surviving actors retry after 30 ticks at their original stand. Tests cover roles, feasible placement, pending spawn persistence and no duplicate defeat. | Run `scripting/guerrilla_legends_place` for supported Classic/LoBo campaigns; exercise temporary group exhaustion. |
| Enemy personal names come from the supplied modern Western male bank; hostile nicknames | Enemy rows use the attachment's `western_evil` regions: western, british or israeli. Other faction regions fall back to western. All-pools tests verify enemy personal-name and hostile-word membership. | None for selection; observe final labels in game. |
| Fixed positions, separate from reinforcement/QRF, identifiable and killable | Named bosses and tank crew have movement disabled while retaining targeting/fire. Their own groups are not registered with garrison or QRF state. Existing integration lanes check displacement under fire, group isolation, combat and defeat; source tests cover placement, persistent positions and defeat latches. | Run `scripting/guerrilla_legends_place`, `scripting/guerrilla_legends_defeat` and `scripting/guerrilla_legends_save_reload.seq`. |
| Names and roles remain visible on the campaign map | Live labels contain name and role; defeated labels now retain both plus `(defeated)`. The marker regression exercises the actual defeat repaint and persisted identity/role. Integration expectations were updated. | Inspect live and defeated map markers before and after full game reload. |

## Verified in this checkout

- Linux `PoseidonGame` and `PoseidonTests` build successfully with Clang.
- `PoseidonTests '[legends],[journal],[guerrilla]' --reporter compact`: **411 cases, 3,954,120 assertions passed**.
- The new placement regression failed before the fix: one commander placed where three legal stands existed. It passes with the fallback search.
- `python3 -m unittest discover -s arma_CWR/tests/contracts -v`: **9 cases passed**.
- `python3 arma_CWR/tools/legend-names/gen_legend_names.py --check`: passed.
- Formatting checks on changed C++ files and `git diff --check`: passed.
- Roadmap validation passes with two existing, unrelated dependency warnings.

Several older integration lanes put assertions inside loops whose return values
were discarded by Trident. The affected naming/save lanes now collect failures
and assert their aggregate at the top level. Prior green runs of those lines
are not treated as acceptance evidence.

## Runtime gate

This Linux checkout contains no Classic 1.99 or LoBo game-data installation.
The SQF integration tests above have been updated but **have not been run on
this change**. Run them with the freshly built game and the updated `gmcore`
installed into a prepared Classic data directory; use a separate save/test
profile. Also inspect both nickname announcements, the roster, the memorial
and all three enemy labels at 800x600 and 1440x1080.

Keep issue #57 and the roadmap item open until those results are recorded.
Do not infer gameplay acceptance from the source-test totals.

## Compatibility and limits

Existing saves keep resolved names, awards and history; they are not renamed
to match the new selection policy. Saves predating the Legend registry retain
the existing derived-record behavior. A custom island with fewer than three
legal terrain positions still reports the placement shortage; the fallback
search does not put commanders underwater or inside the player's camp.
Unsupported/missing faction assets still use the existing role fallback.
These limits must be distinguished from passing the three-commander runtime
gate on supported campaign setups.
