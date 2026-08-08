# Uslu dur! progress, week of Jul 21 - Aug 4, 2026

| Work item | Issue | Status | Commits | Notes / remaining |
|---|---|---|---|---|
| Deep undercover follow-ups: vehicle undercover test, exposure-map seam fix | #24 | Done | `56a55c6`, `a94430a` | Vehicle test covers mounting a stolen UAZ unobserved; #24 fixed exposure being handed back to the wrong side's map |
| Undercover.Abel playable sandbox mission (Houdan) + run-game.ps1 launch fix | - | Done | `283574a` | Companion piece to the undercover system, playable reference mission |
| Site: undercover field-dossier page + uc-* captures | - | Done | `3eea713` | Linked from the gameplay index |
| Character select: warrior vs civilian outfit family (M1-M3) | #25 | Done, closing pass uncommitted | `ff5ff41`, `eacda4a` | Player + recruit matching + AI civTier ladder; 2121 assertions green. Closing pass adds M2.3 integration coverage: real companion/recruit spawns in the civilian e2e, deterministic `guerrilla_outfit_hold_spawn` test, cross-process `guerrilla_outfit_save_reload.seq`. Still uncommitted |
| Showcase fix: unstarve scripted force-fire, stage alert chapter into witness range | - | Done | `eacda4a` | Fixed pre-existing player_fire/showcase red tests |
| Guerrilla.Lebanon80 playable template (IDF occupation vs Hizballah) + boot test | - | Done | `4ff55be` | Known gaps: menu fixture lacks Hizballah; installer needs `&`-invoked `-IncludeWorld Sinai,Lebanon80` |
| Site marketing pass 1: homepage as landing page (hero + tagline + CTAs) | - | Done | `306cd37` | |
| Site marketing pass 2: Field Photos media gallery + MEDIA nav | - | Done | `a619b68` | |
| Site: homepage Field Reviews strip (in-universe box quotes) | - | Done | `812a6ff` | |
| Main menu: direct-launch buttons for Showcase and Undercover reference missions | - | Done | `6adcf63` | Menu e2e tests need `triEndTest` |
| Roadmap bookkeeping (log lines, hash backfills) | - | Ongoing | `cde2162`, `85d0f35`, `106f4c0`, `d08ec8f`, `56b03f5`, `f64dc8d` | |

## Follow-ups surfaced this week

- The #25 landing comment's "M3.2 needs no new code" claim was wrong: nothing native consumes `FactionCivTierClass`, so garrison use of the civTier ladder is real #16 work. Correction posted on the issue.
- Still in progress per the roadmap and untouched this week: `gate-zero-playtest`, `camera-mouselook`, `dual-package-compat`, `faction-class-resolution`.
- Immediate loose end: commit the #25 closing pass sitting uncommitted on `guerrilla-mode`.
