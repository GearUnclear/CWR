#!/usr/bin/env python3
"""Work out which unit classes the portrait catalogue has to cover, and write
the shoot scripts that photograph them.

A dossier portrait is looked up by `lower(bodyClass) + "__" + lower(face)`
(GuerrillaJournalPages.cpp PortraitKeyOf), where bodyClass is the LIVE body's
own `GetNonAIType()->GetName()`.  A Legend row only ever wears one of four
bodies, and all four come off the faction descriptor:

    companionClass          the named companion (companions.sqs:43)
    companionClassCiv       the same companion on the civilian outfit
                            (companions.sqs:47; a DIFFERENT unit class, which
                            is why the outfit needs no place in the key)
    officer or top tiers[]  the elite/tank commander boss body
                            (LegendPlacement.cpp ReadLegendCapability:
                            eliteClass = officer if present else riflemanClass,
                            riflemanClass = TopRung(tiers))
    top tiersSniper[]       the sniper boss body (cap.sniperClass)

Guards are never Legend rows, so guardClass needs no photograph.

Usage:

    python portrait_roster.py list                 # print the class table
    python portrait_roster.py emit <lane> <out>    # write a shoot .test.sqf

`lane` is "vanilla" or "lobo".  The emitter exists because the vanilla lane is
36 captures and the LoBo lane is 92; hand-writing those blocks would guarantee a
typo in a class name, and a typo in a class name is a portrait filed under a key
nothing will ever ask for.

Standard library only.  tests/contracts/test_portrait_catalogue.py imports the
parser from here so the two can never drift.
"""

import os
import re
import sys

# LegendRegistry::kPortraitFaces (LegendRegistry.cpp:80).  These four tokens are
# the only ones PortraitKeyOf will ever emit; anything else degrades to
# "Default", which has no key and draws the "Photograph unavailable" treatment.
PORTRAIT_FACES = ["Face10", "Face18", "Face27", "Face33"]

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CONFIG_DIR = os.path.join(REPO_ROOT, "guerrilla-mode", "config")

LANES = {
    "vanilla": "guerrilla-factions.hpp",
    "lobo": "lobo-factions.hpp",
}


# ---------------------------------------------------------------------------
# descriptor parsing
# ---------------------------------------------------------------------------

_COMMENT_BLOCK = re.compile(r"/\*.*?\*/", re.S)
_COMMENT_LINE = re.compile(r"//[^\n]*")


def strip_comments(text):
    """Remove C comments.  The descriptors are heavily commented and several
    comments quote class names that are NOT descriptor values."""
    text = _COMMENT_BLOCK.sub("", text)
    return _COMMENT_LINE.sub("", text)


def split_factions(text):
    """Yield (name, body) for every inner `class X { ... };`.

    The outer `class CfgGuerrillaFactions` block is not a faction, so the walk
    starts inside it: brace depth 1 is where the faction classes live.
    """
    text = strip_comments(text)
    out = []
    depth = 0
    i = 0
    pending = None  # (name, body_start) for a class whose '{' we just passed
    while i < len(text):
        c = text[i]
        if c == "{":
            depth += 1
            i += 1
            continue
        if c == "}":
            depth -= 1
            if depth == 1 and pending is not None:
                out.append((pending[0], text[pending[1] : i]))
                pending = None
            i += 1
            continue
        if depth == 1 and pending is None and text.startswith("class", i):
            m = re.match(r"class\s+([A-Za-z0-9_]+)\s*(?::\s*[A-Za-z0-9_]+\s*)?\{", text[i:])
            if m:
                pending = (m.group(1), i + m.end())
                depth += 1
                i += m.end()
                continue
        i += 1
    return out


def scalar(body, key):
    """`key = "value";` -> value, or "" when absent."""
    m = re.search(r'\b' + re.escape(key) + r'\s*=\s*"([^"]*)"\s*;', body)
    return m.group(1) if m else ""


def array(body, key):
    """`key[] = {"a", "b"};` -> ["a", "b"], or [] when absent."""
    m = re.search(r'\b' + re.escape(key) + r'\s*\[\s*\]\s*=\s*\{([^}]*)\}', body)
    if not m:
        return []
    return re.findall(r'"([^"]*)"', m.group(1))


def top_rung(ladder):
    """LegendPlacement.cpp TopRung: the last non-empty entry."""
    for entry in reversed(ladder):
        if entry:
            return entry
    return ""


def legend_classes(body):
    """The bodies a LegendRow can wear for one faction, in a stable order."""
    rifleman = top_rung(array(body, "tiers"))
    officer = scalar(body, "officer")
    elite = officer if officer else rifleman
    sniper = top_rung(array(body, "tiersSniper"))
    out = []
    for cls in (
        scalar(body, "companionClass"),
        scalar(body, "companionClassCiv"),
        elite,
        sniper,
    ):
        if cls and cls not in out:
            out.append(cls)
    return out


def lane_roster(lane):
    """[(faction, [class, ...]), ...] for one descriptor file."""
    path = os.path.join(CONFIG_DIR, LANES[lane])
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        text = fh.read()
    return [(name, legend_classes(body)) for name, body in split_factions(text)]


def lane_classes(lane):
    """Deduplicated, order-preserving class list for one lane."""
    out = []
    for _, classes in lane_roster(lane):
        for cls in classes:
            if cls not in out:
                out.append(cls)
    return out


def required_keys(lane):
    """Every `<class>__<face>` key the lane must ship, lowercase."""
    return [
        "%s__%s" % (cls.lower(), face.lower())
        for cls in lane_classes(lane)
        for face in PORTRAIT_FACES
    ]


# ---------------------------------------------------------------------------
# shoot emission
# ---------------------------------------------------------------------------

# Frozen camera and staging constants.  Tuned over three calibration passes (the
# working is written up in guerrilla-mode/core/portraits/README.md); changing any
# of them means the whole catalogue has to be reshot, because a portrait's job is
# to look like every other portrait beside it on the page.
#
#   anchor    where every subject in the lane stands.  Picked by shooting the
#             same head at four candidate spots and keeping the one whose
#             backdrop is open ground and sky: no wall, no tree trunk, no
#             rock face directly behind the head.
#   dir       the subject's heading, which is also the camera's axis, because
#             TriSetPersonFaceView puts the eye at head + facing * distance.
#             120 deg at 10:30 on 15 June puts the sun three-quarters onto the
#             face and leaves an empty pasture and a distant ridge behind it.
#   park      where the player is sent.  This one is not cosmetic: with the
#             player 180 m away every subject's head tracked HIM and every
#             frame came back in profile.  Three kilometres away there is
#             nothing to look at and the heads stay square to the camera.
#   watch     a point 100 m ahead along `dir`, given to doWatch as a second
#             belt on the same problem.
LANE_RIG = {
    "vanilla": {
        "anchor": "[3892.7, 6980.0, 0]",
        "park": "[3892.7, 4000.0, 0]",
        "watch": "[3979.3, 6930.0, 1.6]",
        "camobj": "[3892.7, 6980.0, 30]",
        "camtgt": "[3892.7, 7180.0, 30]",
        "world": "Nogova (tools/screenshots/missions/Stories.Noe)",
    },
    "lobo": {
        "anchor": "[9060.0, 12180.0, 0]",
        "park": "[9060.0, 9200.0, 0]",
        "watch": "[9146.6, 12130.0, 1.6]",
        "camobj": "[9060.0, 12180.0, 30]",
        "camtgt": "[9060.0, 12380.0, 30]",
        "world": "Sinai (guerrilla-mode/mission/Guerrilla.Sinai)",
    },
}

SUBJECT_DIR = 120
CAM_DISTANCE = 3.5

# camSetFov is INERT until the camera has a target: CameraVehicle::CamEffectFOV
# only reaches the saturate() that applies _minFov/_maxFov when
# _lastTgtPos.SquareSize() > 0.5 (CameraHold.cpp:262-277), and a camCreate'd
# camera with no camSetTarget returns its default _lastFov of 0.7 for ever.  The
# first calibration pass swept five lenses and got five identical frames because
# of it.  The shoot therefore always camSetTargets before it camSetFovs.
CAM_FOV = 0.30

HEADER = '''// ============================================================================
//  PORTRAIT SHOOT - {lane_upper} lane.  GENERATED by tools/screenshots/portrait_roster.py;
//  edit that script and re-emit, never this file.
//
//  WHAT THIS PRODUCES.  One capture per (unit class x face) pair, named
//  "<class>__<face>" in lowercase, which is exactly the key the dossier looks
//  up: GuerrillaJournalPages.cpp PortraitKeyOf builds
//  lower(bodyClass) + "__" + lower(face) and probes
//  gmcore\\portraits\\<key>.paa.  tools/screenshots/portrait_process.py turns
//  these PNGs into those .paa.
//
//  WHY THESE CLASSES.  They are the only bodies a Legend row can wear:
//  companionClass, companionClassCiv, the elite/tank commander body
//  (officer, else the top rung of tiers[]) and the sniper body (top rung of
//  tiersSniper[]).  Guards are never Legend rows.  {nclass} classes x
//  {nface} faces = {ncap} captures.
//
//  WHY THE RIG IS FROZEN.  Four faces of one appearance have to be
//  interchangeable on the page, so every subject in this lane stands on ONE
//  anchor with ONE heading under ONE sky, one alive at a time, and the camera
//  never moves.  The background pixels are then literally the same image in
//  every frame, which portrait_process.py verifies by measuring a background
//  patch and refusing a lane whose patches drifted.
//      date      1985-06-15 10:30      anchor  {anchor}
//      setDir    {sdir}                   distance {dist} m
//      overcast  0.25                  fog      0.05
//      camSetFov {fov}                  player parked at {park}
//
//  HARNESS RULES THIS FILE OBEYS (tools/screenshots/README.md):
//    * triScreenshot labels are BARE LOWERCASE STRING LITERALS.  A format[]
//      label desyncs the runner's own capture counter and the run dies on
//      FAIL:screenshot_not_written (integration.rs:790-795).
//    * Trident splits on newlines and on semicolons at brace depth 0 and
//      evaluates each statement separately, so nothing may be a _local.
//      Everything shared is a global (ss*).
//    * A scene change and its capture never share a statement.  triSimFrames
//      sits between them so the pose has settled.
//    * triCaptureAtPresent true, so each frame is captured at the swap, after
//      the gamma pass.  The default mid-frame capture is pre-gamma and darker
//      than what a player sees, which would calibrate the tone recipe on the
//      wrong pixels.
// ============================================================================

triSimUntil {{ alive player }}

triCaptureAtPresent true

setDate [1985, 6, 15, 10, 30]
0 setOvercast 0.25
0 setFog 0.05
setViewDistance 1200
showCinemaBorder false

// Three kilometres away, and that is load-bearing.  A subject's HEAD tracks
// whatever it can see, and with the player parked 180 m off every calibration
// frame came back in hard profile.
player setCaptive true
player setPos {park}

// The four tokens this catalogue is built on, enumerated from the loaded
// package rather than assumed.  triListFaces filters CfgFaces down to the
// entries a MAN body can actually wear (no `disabled`, no `woman`, no
// "Custom"), so a token missing here would mean the file names a key the
// engine can never produce.
ssFaces = triListFaces
triAssert [("Face10" in ssFaces)]
triAssert [("Face18" in ssFaces)]
triAssert [("Face27" in ssFaces)]
triAssert [("Face33" in ssFaces)]

createCenter west
createCenter east
createCenter resistance
createCenter civilian
ssGrpW = createGroup west
ssGrpE = createGroup east
ssGrpG = createGroup resistance

// The cutscene camera exists only to suppress the HUD: World.cpp:1603 gates
// the HUD draw on `!_cameraEffect`.  Its own transform is irrelevant because
// triSetUnitFaceView pins the render camera, but its FOV is NOT - World.cpp:
// 906-919 takes fov from the active camera effect and the triSetView override
// at :1251-1254 replaces the transform only.  So this is the portrait lens -
// and camSetTarget has to come first or camSetFov is silently ignored.
ssCam = "camera" camCreate {camobj}
ssCam cameraEffect ["internal", "back"]
ssCam camSetTarget {camtgt}
ssCam camSetFov {fov}
ssCam camCommit 0

// A long warm-up, and it is not padding.  The first run of this shoot came back
// with its first four frames measurably brighter than the other thirty-two: the
// sky is still settling to the requested date and overcast for the first second
// or so of a booted world, and portrait_process.py's lighting check refused the
// whole lane over it.  Two hundred frames is well past where the patch means go
// flat.
triSimFrames 200

// Spawn one subject at the anchor, pose him, and leave him standing.  ssU is
// the live body; ssMade says whether the class materialised at all (an
// unresolvable class is a no-show, not a crash, so the shoot skips it rather
// than dying 90 captures in).
ssMake = {{
    ssU = objNull;
    ssBefore = count (units ssGrp);
    ssCls createUnit [{anchor}, ssGrp, "", 0.4, "PRIVATE"];
    ssMade = count (units ssGrp) > ssBefore;
    if (ssMade) then {{ ssU = (units ssGrp) select (count (units ssGrp) - 1) }}
}}

// setSkill 0.1 rather than 0: a zero-skill unit's AI does not run at all and
// the engine logs "check failed: ability > 0" once a second for as long as it
// lives.  disableAI TARGET + doWatch keep the head square to the camera.
ssPose = {{
    ssU setCaptive true;
    ssU disableAI "MOVE";
    ssU disableAI "AUTOTARGET";
    ssU disableAI "TARGET";
    ssU setBehaviour "CARELESS";
    ssU setSkill 0.1;
    ssU setDir {sdir};
    ssU setPos {anchor};
    ssU doWatch {watch}
}}

// Twice, with frames between: the first call is what forces EffectStandStill
// and the Default mimic (TriSetPersonFaceView), and the pose needs frames to
// settle before the camera is worth trusting.
ssFrame = {{
    triSetUnitFaceView [ssU, {dist}];
    triSimFrames 6
}}
'''

WARMUP_NOTE = '''
// ---- ONE THROWAWAY SUBJECT, and it earns its place ----
// The first capture taken on the pinned portrait view comes back PRE-GAMMA even
// with triCaptureAtPresent on: its background patch measures 138.6 where every
// later frame measures 153.6, and 255*(138.6/255)^(1/1.2) = 152.8 - the 1.2
// gamma pass, missing, on all three channels to within 0.2/255.  Every frame
// after it is correct.  So the shoot burns one full block, identical to the
// real ones, before it starts.  portrait_process.py drops any capture whose
// label carries no "__", which is what keeps this frame out of the catalogue.
'''

BLOCK = '''
// ---- {cls} / {face} ----
ssGrp = {grp}
ssCls = "{cls}"
call ssMake
if (ssMade) then {{ call ssPose }}
if (ssMade) then {{ triAssertEq [(typeOf ssU), "{cls}"] }}
if (ssMade) then {{ ssU setFace "{face}" }}
triSimFrames 8
if (ssMade) then {{ triAssertNe [(triFaceTexture ssU), ""] }}
if (ssMade) then {{ call ssFrame }}
if (ssMade) then {{ call ssFrame }}
triSimFrames 4
triScreenshot "{key}"
if (ssMade) then {{ deleteVehicle ssU }}
triSimFrames 4
'''

# Which center a class spawns on.  Irrelevant to the photograph (a unit's side
# comes from the center it is created on, and every subject is captive and
# careless anyway), but createGroup returns grpNull for a side with no center,
# and a group that already holds a corpse would confuse ssMake's count test.
GROUP_OF = {
    "vanilla": {"W": "ssGrpW", "E": "ssGrpE", "G": "ssGrpG"},
}


def group_for(cls):
    """A center to hang the subject on.  Nothing depends on it being the
    'right' side, only on it existing."""
    low = cls.lower()
    if low.startswith("soldierw") or low.startswith("officerw"):
        return "ssGrpW"
    if low.startswith("soldiere") or low.startswith("officere"):
        return "ssGrpE"
    return "ssGrpG"


def emit(lane, out_path):
    rig = LANE_RIG[lane]
    classes = lane_classes(lane)
    text = HEADER.format(
        lane_upper=lane.upper(),
        nclass=len(classes),
        nface=len(PORTRAIT_FACES),
        ncap=len(classes) * len(PORTRAIT_FACES),
        anchor=rig["anchor"],
        park=rig["park"],
        watch=rig["watch"],
        camobj=rig["camobj"],
        camtgt=rig["camtgt"],
        sdir=SUBJECT_DIR,
        dist=CAM_DISTANCE,
        fov=CAM_FOV,
    )
    text += WARMUP_NOTE
    text += BLOCK.format(
        cls=classes[0],
        face=PORTRAIT_FACES[0],
        key="warmup_discard",
        grp=group_for(classes[0]),
    )
    for cls in classes:
        for face in PORTRAIT_FACES:
            text += BLOCK.format(
                cls=cls,
                face=face,
                key="%s__%s" % (cls.lower(), face.lower()),
                grp=group_for(cls),
            )
    text += "\ntriEndTest\n"
    with open(out_path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(text)
    return len(classes) * len(PORTRAIT_FACES)


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    cmd = sys.argv[1]
    if cmd == "list":
        for lane in LANES:
            classes = lane_classes(lane)
            print("%s: %d classes, %d captures" % (lane, len(classes), len(classes) * len(PORTRAIT_FACES)))
            for faction, per in lane_roster(lane):
                print("    %-16s %s" % (faction, ", ".join(per) if per else "(none)"))
            print("  unique: %s" % ", ".join(classes))
        return 0
    if cmd == "emit":
        if len(sys.argv) < 4:
            print(__doc__)
            return 2
        n = emit(sys.argv[2], sys.argv[3])
        print("wrote %s (%d captures)" % (sys.argv[3], n))
        return 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
