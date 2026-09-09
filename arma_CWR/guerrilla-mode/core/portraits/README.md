# Dossier portraits

The photographs the resistance dossier shows on a character page. One `.paa` per
`(unit class, face)` pair, named exactly the key the journal builds:

    lower(bodyClass) + "__" + lower(face) + ".paa"

`bodyClass` is the LIVE body's own `GetNonAIType()->GetName()`; `face` is the
token `LegendRegistry::BindRow` validated against `CfgFaces`. The renderer never
searches, guesses or falls back to a similar file: `PortraitKeyOf`
(`GuerrillaJournalPages.cpp`) builds that string, `Gather` probes
`gmcore\portraits\<key>.paa` with `QIFStreamB::FileExist`, and `Compose` hands
`\gmcore\portraits\<key>.paa` to `AddImage`, whose leading backslash skips
`FindPicture` and goes straight to `GlobLoadTexture`. Nothing here is looked up
by faction, so a portrait can only ever depict the uniform of the class it is
named after.

Missing file, or a face outside the four below, and the page draws its reserved
box plus the pencil "Photograph unavailable" line. That is a designed state, not
a failure.

## How these ship

`guerrilla-mode/install-missions.ps1` already mirrors `guerrilla-mode/core` into
`<GameDir>\gmcore` with `robocopy /MIR`, so this folder installs with no change
to the installer. **Never hand-copy a portrait into a game directory:** `/MIR`
means the next installer run deletes anything not in the repo copy. For a shadow
data dir, the Trident workflow does the same mirror by hand:

    robocopy guerrilla-mode\core <shadow>\gmcore /MIR /NJH /NJS /NDL /NFL /NC /NS /NP

## What is in the catalogue

The four face tokens are `LegendRegistry::kPortraitFaces`: **Face10, Face18,
Face27, Face33**. They are the only tokens `PortraitKeyOf` will ever emit;
anything else (a woman body, a package that refuses all four) degrades to
"Default", which has no key at all.

The classes are the four bodies a Legend row can wear, all named by the faction
descriptor: `companionClass`, `companionClassCiv`, the elite/commander body
(`officer`, else the top rung of `tiers[]`) and the sniper body (top rung of
`tiersSniper[]`). Guards are never Legend rows and need no photograph.

| lane | descriptor | classes | files |
|---|---|---|---|
| vanilla | `guerrilla-mode/config/guerrilla-factions.hpp` | 9 | 36, committed |
| @LoBo | `guerrilla-mode/config/lobo-factions.hpp` | 23 | 92, **not** committed |

The @LoBo half is excluded by one line in `arma_CWR/.gitignore`
(`/guerrilla-mode/core/portraits/lobo*.paa` - every @LoBo Legend class
lowercases to a `lobo` prefix, so that glob is exactly the @LoBo-derived set and
none of the vanilla set). Those frames are derived from APL-SA third-party art
and this repo is GPL. The tracked regeneration artifact is
`tools/screenshots/portraits/portrait_lobo.test.{sqf,toml}`. This is a
maintainer call and it is reversible by deleting that one line.

`catalogue.json` records one entry per key: class, face, lane, source frame,
capture size, the background-patch mean that proves the lighting, the `.paa`
size and format, and how much the four faces of that class actually differ.

## Regenerating

    python tools/screenshots/portrait_roster.py list          # what has to be covered
    python tools/screenshots/portrait_roster.py emit vanilla tools/screenshots/portraits/portrait_vanilla.test.sqf
    tri test tools/screenshots/portraits/portrait_vanilla.test.sqf
    python tools/screenshots/portrait_process.py tmp/tri/<ts>/portrait_vanilla vanilla

The @LoBo lane is the same, with `lobo` for `vanilla` and one extra flag: it must
run against the real Classic install, because `--mods-dir ".."` resolves against
the game's own working directory and the `classic-shadow` junction farm has no
`@LoBo` sibling.

    tri test --data-dir "D:\Arma_CWA\ARMA Cold War Assault [Classic]" tools/screenshots/portraits/portrait_lobo.test.sqf

Do not hand-edit the `.test.sqf` files. They are generated; edit
`portrait_roster.py` and re-emit.

## The frozen rig

Four faces of one appearance have to be interchangeable side by side on a page,
so every subject in a lane stands on ONE anchor with ONE heading under ONE sky,
one alive at a time, and the camera never moves. Change any of these and the
whole catalogue has to be reshot.

|  | vanilla | @LoBo |
|---|---|---|
| world | Nogova, `tools/screenshots/missions/Stories.Noe` | Sinai, `guerrilla-mode/mission/Guerrilla.Sinai` |
| anchor | `[3892.7, 6980.0, 0]` | `[9060.0, 12180.0, 0]` |
| player parked at | `[3892.7, 4000.0, 0]` | `[9060.0, 9200.0, 0]` |
| camera object / target | `[3892.7, 6980.0, 30]` / `[3892.7, 7180.0, 30]` | `[9060.0, 12180.0, 30]` / `[9060.0, 12380.0, 30]` |

Shared by both lanes:

    setDate [1985, 6, 15, 10, 30]      setDir 120        distance 3.5 m
    0 setOvercast 0.25                 camSetFov 0.30    view distance 1200
    0 setFog 0.05                      capture 1280x960  triCaptureAtPresent true

Post-process, all fractions of the capture HEIGHT (read from the file, never
from the toml: the capture is `SDL_GetWindowSizeInPixels`, not the requested
size):

    crop    side 0.40   dx +0.005   top 0.345
    tone    saturation 0.40, warm (1.06, 1.00, 0.92), levels 0..255 -> 18..245, contrast 1.08
    card    236 px portrait centred on a 256x256 card, paper (218, 214, 201),
            one-pixel inner rule (194, 190, 177), DXT1, opaque

The card is a shade warmer and lighter than the notepad's own paper, which
measures (209, 210, 207) on a clean stretch of a real 800x600 dossier capture.
Matching it exactly makes the border vanish and the photograph reads as a hole
cut in the page.

## Why each number is what it is

Four calibration passes, and every one of them changed something.

**The anchor.** The same head was shot at four candidate spots and the winner is
the one whose backdrop is open ground and sky. The other three put a tree trunk,
a barn wall or a pale rock face directly behind the head, all of which read as
clutter once the crop is that tight.

**setDir 120.** The camera sits in front of the subject (`TriSetPersonFaceView`
puts the eye at `head + facing * distance` and looks back), so the heading is
what decides where the sun lands. At 10:30 on 15 June, 120 degrees puts a
three-quarter key on the face. The other four headings tried gave flat frontal
light or a half-shadowed face.

**camSetFov 0.30, and camSetTarget before it.** `camSetFov` is INERT until the
camera has a target: `CameraVehicle::CamEffectFOV` only reaches the `saturate()`
that applies `_minFov`/`_maxFov` when `_lastTgtPos.SquareSize() > 0.5`
(`CameraHold.cpp:262-277`), and a `camCreate`d camera with no `camSetTarget`
returns its default `_lastFov` of 0.7 for ever. The first calibration pass swept
five lenses and got five identical frames.

**The player parked 3 km away.** With the player 180 m off, every subject's head
tracked HIM and every frame came back in hard profile. Three kilometres away
there is nothing to look at. `disableAI "TARGET"` plus a `doWatch` at a point
100 m ahead are the belt to that braces.

**setSkill 0.1, not 0.** A zero-skill unit's AI does not run at all and the
engine logs `check failed: ability > 0` once a second for as long as it lives.

**crop top 0.345.** A first guess of 0.39 clipped the helmet of every helmeted
class.

**One throwaway frame at the top of each shoot.** The first capture taken on the
pinned portrait view comes back PRE-GAMMA even with `triCaptureAtPresent` on:
its background patch measures 138.6 where every later frame measures 153.6, and
`255*(138.6/255)^(1/1.2) = 152.8`, i.e. exactly the 1.2 gamma pass, missing, on
all three channels to within 0.2/255. The shoot burns one full block before it
starts; `portrait_process.py` drops any capture whose label has no `__` in it.

## The lighting proof

"We set the same date" is not evidence. `portrait_process.py` measures the mean
RGB of two fixed sky patches in the top corners of every UNTOUCHED capture and
refuses the lane unless the four faces of a class agree within 2/255 per channel
and the whole lane within 6/255. Those patches are the same sky in every frame
by construction, so any drift means the sun, the weather or the anchor moved.

Measured on the shipped runs: **vanilla 0.08 / 0.09 / 0.11** across 36 frames,
**@LoBo 1.63 / 1.19 / 0.52** across 92. The @LoBo lane is wider only because it
is two and a half times longer and the sun really does move.

The check has already earned its keep twice: it caught the pre-gamma first frame
and, before that, a whole lane whose first four frames were shot before the sky
had settled after boot.

## Honest limits

**Nine classes cover the face, and their four portraits are near enough the same
picture.** `portrait_process.py` measures this: the mean pixel difference
between any two of a class's four portraits is 1.0 to 1.8 for these nine, and
5.2 or more for every other class.

    soldierwsniper, soldieresniper    ghillie hood; the face is not visible at all
    lobo_sniper_jor                   ghillie hood
    lobo_terror_01e, lobo_terror_01r  keffiyeh face wrap and sunglasses
    lobo_terror_03e, lobo_terror_03r  keffiyeh face wrap
    lobo_terror_svde, lobo_terror_svdr  balaclava

All four keys still ship for each of them. The registry rolls a face regardless
of whether the model shows it, and the dossier has to resolve something; a
photograph of a hooded marksman is an honest photograph of that character.

**Not covered, by design:**

- any third-party `CfgGuerrillaFactions` pack. `tests/contracts/test_portrait_catalogue.py`
  is what turns that from a silent gap into a red test;
- any woman body, and any package that refuses all four tokens: no key is
  generated at all, so the dossier draws its unavailable treatment;
- a garrison, guard or civilian body that is not a Legend row;
- the Demo package's roster, per the 2026-07-14 asset-targeting policy.

The full retail Remaster needs nothing extra: its data payload is byte-identical
to Classic 1.99, so the vanilla nine resolve there unchanged.

**One wrinkle worth stating plainly.** Three @LoBo descriptors name the same
`companionClassCiv = LoBo_Terror_01E`, so an IDF companion on the civilian
outfit is photographed in that body. That is not a mismatch; it is what the
descriptor says the live body is.

## What checks this

| layer | file | what it can see |
|---|---|---|
| Catch2 | `tests/unit/.../UI/Guerrilla/test_portrait_assets.cpp` | every shipped `.paa` is a 256x256 opaque DXT1 card with a key-shaped lowercase name |
| Python | `tests/contracts/test_portrait_catalogue.py` | a portrait exists for every class every faction descriptor can produce (this is the one that goes red when somebody adds a faction) |
| Trident | `tests/integration/scripting/guerrilla_portrait_dossier.test.sqf` | the loose `.paa` really resolves through the bank-then-CWD chain in a booted game, and the texture the notepad holds is the one the live body's own class names |
| Trident | `tests/integration/ui/guerrilla_journal_capture_800.test.sqf` | the dossier still fits one physical page at 800x600 with the photograph on it |
