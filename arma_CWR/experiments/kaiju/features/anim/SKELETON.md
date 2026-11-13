# Dedushka skeleton: what the final model must provide

The animation set is not hand-keyed for one body. `gen_ded_anim.py` reads a model's **joint file** and writes every clip for that model, so the winner of the model round only has to meet this page. Nothing here depends on the proxy's sizes.

Proven in game on the proxy (`kaiju-notes\pipeline\build\dedushka_rest.json`), and offline on two invented joint files shaped like candidates A and D (`kaiju-notes\features\anim\build\synthetic\rest_like_A.json`, `rest_like_D.json`: all offline checks pass; they are worked examples of the format, not measurements of the candidates). To rebuild for another model:

```
powershell -NoProfile -File build-kj-anim.ps1 -ModelBuild <the pipeline's build folder of that model>
```

## 1. Rest pose and axes

- The rest pose **is** the standing pose, as sculpted. An identity matrix leaves a bone where the modeller put it, so the creature needs no bind pose other than the one in the file.
- In Blender: he faces **-Y**, his left is **+X**, up is **+Z**, 1 unit is 1 m, the origin is **on the ground between the feet**. (Candidates A and D are built facing +Y: turn them 180 degrees before rigging; the pipeline's wrap mode already does this for statues.)
- In the P3D and the RTM ("file space") that is X = his left, Y = up, Z = back, so forward is -Z. A Blender point (x, y, z) is file point (x, z, y).
- Both soles flat on the ground (y = 0) in the rest pose. The two legs may have different lengths and shapes (candidate D); each leg is solved on its own.

## 2. Bones

A bone is a named selection (a Blender vertex group) in every visual LOD, and in the Geometry, FireGeometry, Memory and HitPoints LODs for the points that must follow it. The engine binds weights by selection name to the bone names it finds in the RTM headers, so the names below are the contract. Lowercase. `l` = his left, `p` = his right (the stock prefix).

The 25 stock man names are kept because the engine asks for `zbran`, `roura`, `lruka`, `pruka` and `hlava` by name and the RTM tools are built round them. Eight more are Dedushka's own; they are optional.

| Bone | Carries | Joint (pivot) it turns about | Candidate A group | Candidate D group |
|---|---|---|---|---|
| `bricho` | pelvis: the root | hip centre (`root` in the joint file) | pelvis | pelvis |
| `lzadek`, `pzadek` | pelvis halves; ride with `bricho` | none needed | (leave empty) | (leave empty) |
| `zebra` | waist and belly, lower spine | lowest spine joint | spine | belly |
| `hrudnik` | chest: the fixed part of the ribcage, the furnace bowl | mid spine joint | chest | chest, ribs |
| `krk` | neck | neck base | neck | neck |
| `hlava` | head, upper jaw | head joint | head | head |
| `lrameno`, `prameno` | shoulder girdle, shoulder blade | where it meets the chest | (leave empty, or the shoulder blades) | (leave empty) |
| `lbiceps`, `pbiceps` | upper arm | shoulder joint | upperarm_l / _r | l_upperarm / r_upperarm |
| `lloket`, `ploket` | forearm | elbow | forearm_l / _r | l_forearm / r_forearm |
| `lruka`, `pruka` | hand, claws, fingers | wrist | hand_l / _r | l_hand + l_fingers / r_hand + r_fingers |
| `lstehno`, `pstehno` | thigh | hip joint | thigh_l / _r | l_thigh / r_thigh |
| `lholen`, `pholen` | shin | knee | shin_l / _r | l_shin / r_shin |
| `lchodidlo`, `pchodidlo` | foot. On a digitigrade leg: the long hock bone | ankle. Digitigrade: the hock joint | foot_l / _r | l_foot / **r_hock** |
| `lprsty`, `pprsty` | toes: the part that stays flat on the ground when the heel lifts. Digitigrade: the paw | ball of the foot | (split the front third of the foot off, or leave empty) | l_toes / **r_foot + r_toes** |
| `zbran`, `roura` | nothing visible; memory points only | right hand, upper back | | |
| `ded_jaw` | lower jaw | jaw hinge | jaw | jaw |
| `ded_rib_l`, `ded_rib_r` | the two halves of the ribcage that open for the beam | their hinge line on each flank | gate_l / gate_r | ribs_liz / ribs_dog |
| `ded_tail1`, `ded_tail2`, `ded_tail3` | tail, root to tip | each joint | (none) | tail1 / tail2 / tail3 |
| `ded_hang1` | a heavy hanging part that should swing late (parent `hrudnik`) | where it hangs from | beard, torn skin | deadhead |
| `ded_hang2` | a second one (parent `krk`) | where it hangs from | | collar chain |

Parent chain (the generator's, the joint file may override a parent for an optional bone): `bricho` > `zebra` > `hrudnik` > `krk` > `hlava` > `ded_jaw`; `hrudnik` > `lrameno` > `lbiceps` > `lloket` > `lruka`; `bricho` > `lzadek` > `lstehno` > `lholen` > `lchodidlo` > `lprsty`; `hrudnik` > `ded_rib_l`, `ded_rib_r`, `ded_hang1`, `roura`; `krk` > `ded_hang2`; `bricho` > `ded_tail1` > `ded_tail2` > `ded_tail3`; `pruka` > `zbran`.

Weighting rules:

- Every vertex of every visual LOD is in at least one of these bones. A vertex in none stays at the rest pose while the body walks away from it.
- At most 4 bones per vertex; weights are stored in steps of 1/128 (engine limits). Two bones across each joint ring is what the clips expect: the loop at the joint half and half, the loops either side 80/20. A model weighted one bone per vertex (as A and D are now) works, with hard creases at the joints.
- The furnace faces (fullbright) go with `hrudnik`; faces on the opening rib halves go with `ded_rib_l` / `ded_rib_r`, so the light is uncovered when they swing.
- A bone left empty costs nothing. A bone with no joint in the joint file rides rigidly on its parent, so a model without a tail, jaw or opening ribs needs no change anywhere: the chest then "opens" only through the shoulders going back and the spine arching.
- Total bones in the set: 33 (limit 127). Each extra bone costs about 700 of the engine's 32000 cached animation matrices; the set uses 23661 now.

What the clips ask of each bone, so the mesh can be cut for it: spine bones bow up to 35 degrees each way; the head throws back 30 degrees in the roar; the jaw opens 30 degrees; each rib half swings 38 degrees outward about a near-vertical hinge; the swatting arm lifts above the level and sweeps 125 degrees across the front; the thigh lifts about 60 degrees in the stomp; the knee bends up to 100 degrees.

## 3. The joint file

`dedushka_rest.json`, format `ud-rtm-rest/1` (the pipeline's format, with optional keys added; the pipeline's exporter writes it from the collection property `ud_rest_json`). File space, metres.

```
{
 "format": "ud-rtm-rest/1",
 "height_m": 43.4,
 "root": [0, 21.0, 0.4],                  // hip centre: the pivot of bricho
 "pivots": {                              // each bone's joint with its parent
   "zebra": [...], "hrudnik": [...], "krk": [...], "hlava": [...],
   "lrameno": [...], "lbiceps": [...], "lloket": [...], "lruka": [...],      // and the p side
   "lstehno": [...], "lholen": [...], "lchodidlo": [...], "lprsty": [...],  // and the p side
   "ded_jaw": [...], "ded_rib_l": [...], "ded_rib_r": [...],                // optional
   "ded_tail1": [...], "ded_tail2": [...], "ded_tail3": [...], "ded_hang1": [...], "ded_hang2": [...]
 },
 "points": {                              // optional; defaults in brackets
   "foot_l": [x, 0, z], "foot_r": [x, 0, z],   // centre of each sole on the ground [under the ankle, halfway to the toe joint]
   "heel_l": [x, 0, z], "heel_r": [x, 0, z],   // back end of each sole [as far behind the centre as the toe joint is ahead of it]
   "hand_l": [...], "hand_r": [...],           // middle of the palm or claws [a quarter forearm past the wrist]
   "furnace": [...],                           // where the beam leaves the chest [halfway from chest joint to neck base]
   "mouth": [...],                             // where the roar leaves him
   "head_top": [...]                           // top of the head in the rest pose (it rides with hlava)
 },
 "parents": { "ded_hang1": "lrameno" },   // optional: move an optional bone to another parent
 "anim": { "hunch": 0, "tall": 24, "swat_arm": "l", "stomp_foot": "p" }      // optional feel overrides
}
```

- Required joints: the 20 named on the first three `pivots` lines. The generator stops with a list if one is missing.
- A joint is the point the bone turns about: the centre of the ring loop at that joint. `lrameno` may sit at the neck base if there is no real shoulder girdle.
- Knees: the generator bends each knee the way the rest pose bends it (the knee's offset from the hip to ankle line), or forward if the rest leg is straight. A leg sculpted locked straight and meant to bend backward needs a small backward bend in the rest pose.
- Digitigrade leg (D's right): `pstehno` = hip, `pholen` = knee (stifle), `pchodidlo` = hock joint, `pprsty` = ball of the paw, `foot_r` = middle of the paw on the ground. The hock bone and paw move as one unit, placed on the ground; the heel-off of the walk pitches the hock bone about the paw. A sole point ahead of the toe joint is taken to lie on the toes bone, one behind it on the foot bone, so a paw whose pad is in front of its joint needs no setting.
- The soles never go under the ground: if a pose would push the toe joint, the sole centre or the heel point below y = 0, the whole foot is lifted by that much.
- `anim` settings and their defaults are the TUNING block at the top of `gen_ded_anim.py`. The two that depend on the sculpt:
  - `hunch`: degrees of extra forward bow in his normal carriage. 30 for a model sculpted upright (the proxy), 0 for a model sculpted hunched (A, D). Chosen automatically from the rest spine's lean (under 8 degrees counts as upright).
  - `tall`: degrees he un-bows from the rest pose for the beam. Automatic: 0.8 of the rest lean, at most 30.
  - `hand_clearance`: set automatically when the rest hands are within 5 percent of his height from the ground (A's claws): the elbows bend just enough that the hands never go through the ground.
  - `swat_arm`, `stomp_foot`: `p` (right) by default. D's long lizard arm is the left one: `"swat_arm": "l"`.

## 4. Memory points

Scripts cannot read a memory point on the stock engine (there is no selection position command), so the generator publishes posed positions in `ded_anim_timing.sqf` from the joint file's `points`. The model must still carry the points, for the engine's own use and for the day a command exists (see ENGINE-WISHES.md). Each point is also in the bone selection it must follow.

| Memory point | In bone | For |
|---|---|---|
| `zamerny` | `hrudnik` | the engine's aim point and view position: the furnace, chest front |
| `osa mireni`, `osa otaceni` | `bricho`, `hrudnik` | engine aiming and head axes |
| `pilot` | `hlava` | the eye |
| `stopaL`, `stopaP` | `lchodidlo`, `pchodidlo` | under each sole: the engine's footprint and step sound points; same place as `foot_l` / `foot_r` |
| `zbran` (2 points), `roura` (2 points) | `zbran`, `roura` | required bones; right hand and upper back |
| `granat`, `granat2` | `pruka`, `lruka` | hands; same place as `hand_r` / `hand_l` |
| `ded_furnace` | `hrudnik` | beam origin; same place as `furnace` |
| `ded_furnace_dir` | `hrudnik` | one metre in front of `ded_furnace` along the way the beam leaves |
| `ded_mouth` | `hlava` | roar origin; same place as `mouth` |
| one point per bone | each trunk and leg bone | the engine's lean and hit code looks bones up through their points (the proxy has them) |

## 5. The other LODs

As the proxy has them (`proxy\build_proxy.py` is the worked example): Geometry and FireGeometry boxes each in one bone, so the collision shape follows the animation (the raised arm of the swat is what a helicopter hits); LandContact, one point at the origin in no bone; HitPoints with `hlava`, `telo`, `ruce`, `nohy` each point also in its bone; `autocenter = 0` on the Geometry LOD so the origin stays between the feet (the turn clips step round that point).

## 6. What the generator gives back

Per model: 14 RTMs, `ded_anim_gen.hpp` (seconds and turn rates for `moves_dedushka.hpp`), `ded_anim_timing.json` / `.sqf` and `FOOTFALLS.md` (footfalls and events with positions), `ded_anim_report.txt` (offline checks: planted foot slide, leg reach, soles under ground, start and end poses, key steps, speeds). `preview_clips.py` draws every clip without the game. The build stops if an offline check fails.

## 7. Not handled yet

- Hands that walk on the ground (a knuckle walk). The arms are dead weight hanging from the shoulders; a model whose arms carry weight needs arm IK like the legs.
- A third leg segment that bends on its own during the stance (the hock of a digitigrade leg flexing under load). It is rigid with the paw.
- Fingers, ears, eyes, cloth. Give them to `ded_hang1` / `ded_hang2` or leave them rigid.
- An asymmetric model has asymmetric clips, but the walk assumes two feet side by side at rest. D's left leg "thrown out sideways" is fine as long as its sole is on the ground in the rest pose.
