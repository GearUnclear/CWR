r"""Writes the one creature config (uddedushka\config.cpp) of the integrated addon, for every
creature in the build, and the per-creature data the script set reads.

    python gen_config.py --plan <plan.json> --moves moves_dedushka.hpp --out <config.cpp>

plan.json (written by build-kj-dedushka.ps1 from creatures.json):
    {"creatures": [{"key": "a", "class": "UDDedushka", "displayName": "Dedushka", "model": "dedushka",
                    "moves": "CfgMovesDedushka", "sounds": {"KJ_Roar": "KJ_RoarBear1"}, "anim": "<dir of gen_ded_anim.py output>",
                    "export": "<dedushka_export.json>", "rest": "<dedushka_rest.json>"}, ...]}

A creature is a DATA ENTRY. For each one the config gets:
  - CfgModels >> <model stem>
  - its own moves class (the shared moves_dedushka.hpp with this creature's numbers put in: clip
    lengths, turn rates, footfall edges, and its own clip files under \uddedushka\anim_<key>\).
    Move NAMES are the same in every moves class (DedIdle, DedWalk, ...), so the scripts need not
    know which creature they drive.
  - CfgVehicles >> <class>: the model, the moves class, the toughness, what the scripts read
    (kjHeight, kjReach, kjSnd_* its own voice, kjFurnacePoint and its metres, kjMouthUp) and the engine's
    `class Kaiju` trait block (features\engine\API.md).
Everything that depends on a model (height, reach, wading depth, furnace, footfall phases) is
read from that model's own build, so a new or changed model is one build command.
"""
import argparse
import json
import math
import os
import re

HEAD = r'''// uddedushka: the giant creatures as Man-class units. INTEGRATED addon: every creature of the
// build in one config (its model, its own generated clips and moves class, the brain's DedStride,
// the crush feature's kjHeight and zone armour, the engine's `class Kaiju` traits).
// Written by experiments\kaiju\integration\addon\gen_config.py. Do not edit this copy: edit the
// generator, moves_dedushka.hpp or creatures.json, and rebuild (build-kj-dedushka.ps1).
//
// Config rules learnt on the chupacabra (cryptid notes, anim\moves\NOTES.md): no forward
// declarations, base chains are re-declared empty and merge onto the originals, never
// `class X : X`, edge costs at most 2.5.

#define private   0
#define protected 1
#define public    2

class CfgPatches
{
    class uddedushka
    {
        units[] = {@UNITS@};
        weapons[] = {};
        requiredVersion = 1.30;
        // Civilian, Man, CfgModels >> Man and CfgMovesMC live in the base config (no owner
        // addon), so nothing has to be required.
        requiredAddons[] = {};
    };
};

class CfgModels
{
    class Default {};
    class Head : Default {};
    class Man : Default {};
    // Looked up by the model's file stem.
@MODELS@};

class CfgManActions {};
class CfgMovesMC
{
    class Default {};
};
'''

VEHICLE = r'''
    // ---- @DISPLAY@ (@KEY@): @SOURCE@
    class @CLASS@ : Civilian
    {
        scope = public;
        // Resistance (the combat feature, measured with live fire): observers judge it by its CONFIG
        // side until they have confirmed it, so it must be a side a garrison can be hostile to. The
        // quiet demo holds it captive in an EAST group; the live-fire take puts it in a Resistance group.
        side = 2;
        displayName = "@DISPLAY@";
        model = "\uddedushka\@MODEL@";
        moves = "@MOVES@";
        hiddenSelections[] = {};
        weapons[] = {};
        magazines[] = {};

        // ---- read by the kj\ script set (GetVehicleParam; kj\init.sqf, KJ_fnSetup) -------
        kjHeight = @HEIGHT@;            // metres, from the model's own bounds: every script size is a fraction of it
        kjReach = @REACH@;              // metres: how far any part is from its origin in plan (rest pose)
@SOUNDS@        kjFurnacePoint = "@FURNACE_PT@";  // memory point the furnace light, the embers and the beam leave from
        kjFurnaceFwd = @FURNACE_FWD@;   // the same point as [forward, up] metres from its feet (rest pose), for a
        kjFurnaceUp = @FURNACE_UP@;     // binary that cannot read a memory point
        kjMouthUp = @MOUTH_UP@;         // metres above its feet the roar comes from (rest pose)
        kjGeoRadius = @GEO_R@;          // metres: radius of its collision shape as built here (it sets how hard a hit counts)
        kjGeoFloor = @GEO_FLOOR@;       // metres: its collision shape starts this high (nothing on the ground can shove it); 0 = at the ground
@REGIONS@
        // ---- toughness: the combat feature's values, chosen from live-fire runs (its NOTES, "Who
        // shoots him"). armor decides WHO FIRES: the AI's two gates read it and the geometry radius;
        // at 700 tank guns, launchers, missiles and rockets fire, and rifles, machine guns, grenades
        // and the 30 mm cannon stay silent (with the cannon let in, gunships used nothing else).
        // armorStructural is the real toughness (armor x it): no stock round moves the engine's
        // damage value by more than 0.3. Zone factors 150: no head, body or leg rule can fire.
        // His health is the script's (kj\combat); the engine's damage value is only the messenger.
        armor = 700;
        armorStructural = 17;
        armorHead = 150;
        armorBody = 150;
        armorHands = 150;
        armorLegs = 150;

        cost = 10000000;
        irTarget = 1;
        audible = 8;
        camouflage = 8;
        accuracy = 0.1;

        maxSpeed = 11;
        precision = 10;
        brakeDistance = 20;
        formationX = 60;
        formationZ = 60;
        mapSize = 30;
        extCameraPosition[] = {0, 15, -90};

        // ---- engine traits (features\engine\API.md). Read when the unit is created, on the
        // kaiju binary only; the stock binary never reads this class. kj\init.sqf checks each one
        // with kjGet and sets it with kjSet when it is missing.
        class Kaiju
        {
            farDraw = 6000;             // drawn to 6 km, past the object and the view distance
            farContrast = 0.5;          // strength of the silhouette at the edge of the fog
            wadeDepth = @WADE@;         // water it stands and walks in without drowning (0.75 of its height)
            obstacleBlind = 1;          // houses and trees are open ground to the path planner
            noSpeedDamage = 1;          // a long stride is not a crash
            damageCeiling = 0.6;        // held under this while it is unkillable; lifted by kj\start.sqs when it can die
            hitReport = 1;              // every hit is queued for kjHits (who, what, where)
            stepShake = 0;              // the footfall tremor is started by kj\roar\step.sqs, with its own reach
            gridRadius = @GRID@;        // 0 = stock 25 m; only as wide as the model reaches
            nearMip = 1;                // texture mips by the nearest point of the body, not by its feet
            // the engine agent's three of the finishing round (finishing/engine/FOR-MODELS.md item 3; the
            // scripts set them too, and hold them apart from the nine above in KJ_traits2)
            glow = 0.7;                 // the furnace on the body is lit from the moment he is made
            farGlow = 0.9;              // by night the far draw keeps this much of his own colour
            plantBlend = 1;             // the standing foot stays planted while a walk blends in from a stand
        };
    };
'''


def fmt(x):
    s = "%.3f" % x
    return s.rstrip("0").rstrip(".") if "." in s else s


def parse_defines(text):
    defs = {}
    for line in text.split("\n"):
        m = re.match(r"\s*#define\s+(\w+)\s+(.*)$", line)
        if m:
            val = m.group(2)
            # a trailing // comment is not part of the value (none of the values holds //)
            val = re.sub(r"\s+//.*$", "", val).strip()
            defs[m.group(1)] = val
    return defs


def expand(text, defs):
    """Removes the #define lines and puts every macro's value in place (whole words, nested)."""
    body = "\n".join(l for l in text.split("\n") if not re.match(r"\s*#define\b", l))
    names = sorted(defs, key=len, reverse=True)
    pat = re.compile(r"\b(" + "|".join(map(re.escape, names)) + r")\b")
    for _ in range(5):
        new = pat.sub(lambda m: defs[m.group(1)], body)
        if new == body:
            break
        body = new
    return body


def creature_numbers(c):
    timing = json.load(open(os.path.join(c["anim"], "ded_anim_timing.json"), encoding="utf-8"))
    export = json.load(open(c["export"], encoding="utf-8"))
    rest = json.load(open(c["rest"], encoding="utf-8"))
    bmin, bmax = export["bounds"]["min"], export["bounds"]["max"]
    height = float(bmax[2])
    reach = max(math.hypot(x, y) for x in (bmin[0], bmax[0]) for y in (bmin[1], bmax[1]))
    grid = 0 if reach <= 24 else math.ceil(reach + 2)
    pts = rest.get("points") or {}
    # joint-file space: X left, Y up, Z back
    if "furnace" in pts:
        fpt, f_fwd, f_up = "ded_furnace", -float(pts["furnace"][2]), float(pts["furnace"][1])
    else:
        # the pipeline's contract: zamerny is the aim point in the chest; these metres are only the
        # stock binary's last fallback (the beam measures the point in game)
        fpt, f_fwd, f_up = "zamerny", abs(float(bmin[1])) * 0.88, float(timing["poses"]["normal"]["furnace"][2])
    mouth_up = float(pts["mouth"][1]) if "mouth" in pts else float(timing["poses"]["normal"]["mouth"][2])
    clips = {k["move"]: k for k in timing["clips"]}

    def edge(move, name):
        for e in clips[move]["events"]:
            if e["name"] == name:
                return e["phase"]
        return 2.0

    stomp = clips["DedStomp"]
    impact = edge("DedStomp", "impact")
    left = any(e["name"] == "foot_l_up" for e in stomp["events"])
    edges = {
        "DED_TURNL_EDGE1": "%.3f" % edge("DedTurnL", "foot_l_down"),
        "DED_TURNL_EDGE2": "%.3f" % edge("DedTurnL", "foot_r_down"),
        "DED_TURNR_EDGE1": "%.3f" % edge("DedTurnR", "foot_l_down"),
        "DED_TURNR_EDGE2": "%.3f" % edge("DedTurnR", "foot_r_down"),
        "DED_STOMP_EDGE1": "%.3f" % (impact if left else 2.0),
        "DED_STOMP_EDGE2": "%.3f" % (2.0 if left else impact),
    }
    # The model's own hit regions (finishing round: the "combat" block of a finished model's export
    # summary). A model build without the block gets no entries and the combat feature's old rule.
    regions = ""
    cb = export.get("combat") or {}
    if cb:
        lines = ["// ---- hit regions of this model (models-tests/tools/patch_zone.py is the rule that reads them; a script"
                 " set without that patch ignores them)"]
        if "legs_up_m" in cb:
            lines.append("kjLegsUp = %s;                 // metres: a hit under this is legs (its hip joints)" % fmt(cb["legs_up_m"]))
        if "head_point" in cb:
            r, f, u = cb["head_right_fwd_up"]
            lines.append('kjHeadPoint = "%s";       // memory point at the middle of its skull' % cb["head_point"])
            lines.append("kjHeadR = %s;                  // metres round that point that is head" % fmt(cb["head_r_m"]))
            lines.append("kjHeadRight = %s;" % fmt(r))
            lines.append("kjHeadFwd = %s;" % fmt(f))
            lines.append("kjHeadUp = %s;              // the same point in the rest pose" % fmt(u))
        if "furnace_r_m" in cb:
            lines.append("kjFurnaceR = %s;               // metres round the furnace point that is furnace" % fmt(cb["furnace_r_m"]))
        regions = "".join("        " + ln + chr(10) for ln in lines)
    return dict(height=height, reach=reach, grid=grid, fpt=fpt, f_fwd=f_fwd, f_up=f_up, mouth_up=mouth_up, edges=edges,
                regions=regions, source=str(export.get("blend") or export.get("obj") or c["export"]))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--plan", required=True)
    ap.add_argument("--moves", required=True)
    ap.add_argument("--out", required=True)
    a = ap.parse_args()
    plan = json.load(open(a.plan, encoding="utf-8-sig"))
    creatures = plan["creatures"]
    moves_src = open(a.moves, encoding="utf-8").read()
    marker = "class CfgMovesDedushka : CfgMovesMC"
    cut = moves_src.index(marker)
    moves_head, moves_body = moves_src[:cut], moves_src[cut:]
    shared_defs = parse_defines(moves_head)

    units, models, moves_out, vehicles = [], [], [], []
    for c in creatures:
        n = creature_numbers(c)
        gen = open(os.path.join(c["anim"], "ded_anim_gen.hpp"), encoding="utf-8").read()
        defs = dict(shared_defs)
        defs.update(parse_defines(gen))
        defs.update(n["edges"])
        body = expand(moves_body, defs)
        body = body.replace(marker, "class %s : CfgMovesMC" % c["moves"])
        body = body.replace("\\uddedushka\\anim\\", "\\uddedushka\\anim_%s\\" % c["key"])
        moves_out.append("// ==== %s: the moves class of %s (%s). Clips: \\uddedushka\\anim_%s\\ ====\n%s"
                         % (c["moves"], c["class"], c["displayName"], c["key"], body.rstrip()))
        # its own voice: kjSnd_<shared class without KJ_> = "<its class>"
        sounds = "".join('        kjSnd_%s = "%s";%s// its own take of %s\n' % (k[3:], v, " " * max(1, 22 - len(k) - len(v)), k)
                         for k, v in sorted((c.get("sounds") or {}).items()))
        units.append('"%s"' % c["class"])
        models.append("    class %s : Man {};\n" % c["model"])
        vehicles.append(VEHICLE.replace("@CLASS@", c["class"]).replace("@DISPLAY@", c["displayName"])
                        .replace("@KEY@", c["key"]).replace("@SOURCE@", n["source"].replace("\\", "/"))
                        .replace("@MODEL@", c["model"]).replace("@MOVES@", c["moves"])
                        .replace("@HEIGHT@", fmt(n["height"])).replace("@REACH@", fmt(n["reach"]))
                        .replace("@SOUNDS@", sounds).replace("@FURNACE_PT@", n["fpt"])
                        .replace("@FURNACE_FWD@", fmt(n["f_fwd"])).replace("@FURNACE_UP@", fmt(n["f_up"]))
                        .replace("@MOUTH_UP@", fmt(n["mouth_up"])).replace("@GEO_R@", fmt(float(c.get("geoRadius") or 0)))
                        .replace("@GEO_FLOOR@", fmt(float(c.get("geoFloor") or 0))).replace("@WADE@", fmt(0.75 * n["height"]))
                        .replace("@REGIONS@", n["regions"])
                        .replace("@GRID@", fmt(n["grid"])))
        print("%-6s %-18s height %s m, reach %s m, gridRadius %s, furnace %s fwd %s up %s, mouth up %s, sounds %s"
              % (c["key"], c["class"], fmt(n["height"]), fmt(n["reach"]), fmt(n["grid"]), n["fpt"], fmt(n["f_fwd"]),
                 fmt(n["f_up"]), fmt(n["mouth_up"]), c.get("sounds") or {}))

    text = (HEAD.replace("@UNITS@", ", ".join(units)).replace("@MODELS@", "".join(models))
            + "\n" + "\n\n".join(moves_out) + "\n\n"
            + "class CfgVehicles\n{\n    class All {};\n    class AllVehicles : All {};\n    class Land : AllVehicles {};\n"
            + "    class Man : Land {};\n    class Civilian : Man {};\n" + "".join(vehicles) + "};\n")
    with open(a.out, "w", encoding="utf-8", newline="\r\n") as f:
        f.write(text)


if __name__ == "__main__":
    main()
