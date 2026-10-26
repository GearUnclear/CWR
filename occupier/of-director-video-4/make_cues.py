"""Build cues.json for render.py from trace.json and stats.json (video 4).
Every caption is pinned to the engine line or scenario note it describes and
quotes that line under the caption.

Usage: python make_cues.py trace.json stats.json cues.json
"""
import json
import re
import sys

from cue_lib import T, text

tr = T(sys.argv[1])
st = json.load(open(sys.argv[2]))
out_path = sys.argv[3]

cues = []
missing = []


def F(pat, after=-1.0, before=1e12, kind=None):
    e = tr.find(pat, after, before, kind)
    if e is None:
        missing.append(pat)
    return e


def cue(e, say, rate=None, line=None, dt=0.0, **kw):
    if e is None:
        return
    d = {"sim": round(e["sim"] + dt, 2), "text": say, "line": line if line is not None else text(e)}
    if rate is not None:
        d["rate"] = rate
    d.update(kw)
    cues.append(d)


def both(a, b):
    return text(a) + "   " + text(b) if a and b else text(a or b)


# ---- the opening towns ------------------------------------------------------
e = F(r"Village rose; the occupier does not answer")
cue(e, "A cell takes Houdan and moves on. The director is still dormant, so the old rule rolls a forty percent chance: no answer.", rate=14, dt=-1)
e = F(r"AWAKE after 2 distinct towns")
cue(e, "Four fighters take Le Port and stay. A second town wakes the director, with two tokens in hand.", rate=5, dt=-1)
e = F(r"\[op 1\] Motorized squad")
cue(e, "A local reports the four. To out-gun them it buys a truck with a tank escort: twenty-five units, seventy-four seconds by road.", rate=4)
e = F(r"\[op 2\] Foot squad")
cue(e, "Empty Houdan gets four walkers. Its road winds past the detour rule, so the quote is the straight line times one and a half.", rate=3)
e = F(r"\[op 1\] born for Le Port")
r = F(r"\[op 1\] Repelled at Le Port")
cue(e, "The column arrives with defenders present and goes live, five point seven kilometres from the player. Only the riflemen are born; the armour stays abstract. Repelled.", rate=9, line=both(e, r), dt=-1)

# ---- the Outpost ------------------------------------------------------------
n = F(r"lever: the Outpost hold squad", kind="note")
e = F(r"\[op 3\] arrival rule")
cue(n, "A cell takes the Outpost and its hold squad is deleted: the base is empty. Walking takes fifty-five minutes by road, so a Mi17 brings six.", rate=3, line=text(e))
e = F(r"6 attacker\(s\) walk into empty Outpost")
r = F(r"\[op 3\] Outpost retaken")
cue(e, "They hold it abstractly. The census counts nobody, the meter fills in seventeen seconds, and the base is retaken with no soldier born.", rate=2, line=both(e, r))

# ---- the double capture -----------------------------------------------------
a = F(r"Chapoi rose; the occupier director takes it")
b = F(r"Dourdan rose; the occupier director takes it")
cue(a, "Dourdan and Chapoi fall in one tick, and the count is exact: two towns before Chapoi, three before Dourdan.", rate=20, line=both(a, b), dt=-0.5)
e = F(r"\[op 4\] arrival rule")
cue(e, "Chapoi is seventy minutes away on foot, so it also gets a helicopter, and is retaken unseen.", rate=25)

# ---- Arudy -------------------------------------------------------------------
e = F(r"\[op 5\] APC")
cue(e, "Three fighters hold Arudy and a local reports them. The plan expected a helicopter; with three reported, the cheapest lethal package is a BMP, four minutes by road.", rate=20)

# ---- the Airfield and the wave ------------------------------------------------
n = F(r"lever: the Airfield hold squad", kind="note")
w = F(r"Wave 1 \(30\) inbound")
cue(n, "Raiders empty the Airfield and leave. Control drops to sixty-nine percent, and wave one, thirty strong, is called to the Harbour.", rate=8, line=text(w))
e = F(r"no operation on Airfield: no occupier base can spare the minimum")
cue(e, "For eight minutes no base can spare six men for the Airfield. The refusal names the gap: short by one.", rate=40)
e = F(r"\[op 6\] Wave 1 strike: HQ assault")
L = F(r"wave of 30 landed")
cue(L, "The wave lands and lifts the pool to ninety. Its strikes go for the cluster around Dourdan: six men walk on the headquarters, forty minutes by road.", rate=6, line=text(e))
e = F(r"\[op 9\] spare cap")
r = F(r"Airfield retaken by the holding force")
cue(e, "Now the spare cap sends six from the Harbour to the Airfield by helicopter. Nobody is reported there, so no jet opener. Retaken in three minutes.", rate=12, line=both(e, r))

# ---- the strike ----------------------------------------------------------------
n = F(r"lever: Le Port heat", kind="note")
cue(n, "Le Port is made urgent, four fighters reported, the director strong.", rate=8, gapmax=2.0)
e = F(r"band STRONG -> THIN")
j = F(r"\[op 12\] Jet strike over Le Port")
cue(e, "It buys two BMPs and a jet: forty-five units at once. The pool falls to seventeen and the band from strong straight to thin.", rate=1.2, line=both(j, e), dt=-0.5)
e = F(r"\[op 12\] Struck Le Port")
b = F(r"band THIN -> STRONG")
cue(e, "The abstract strike kills nobody, but Le Port loses forty support and forty standing for twenty minutes. The jet's refund restores strong thirty-eight seconds later.", rate=3, line=both(e, b), dt=-0.3)

# ---- the rising and the HQ ------------------------------------------------------
n = F(r"^control .* after the rising", kind="note")
cue(n, "The cell rises again at the Outpost, Houdan and Dourdan. Control falls to seventy-seven percent, but wave one is spent; the next waits at fifty.", rate=25)
e = F(r"stand at the undefended HQ")
d = F(r"HQ at Camp destroyed")
cue(e, "The wave's walkers reach the headquarters unseen. Nobody defends it; its meter fills in fifteen seconds and it is destroyed.", rate=1.5, line=both(e, d), dt=-1)
b = F(r"over poolStart banked at")
g = F(r"\[op 6\] @HQ retaken")
cue(b, "The six go back to the pool, five banked at the Airfield over the ceiling. In the same second the log claims they stay as the HQ's garrison.", rate=2, line=both(b, g))

# ---- free running -----------------------------------------------------------------
e = F(r"\[op 15\] Push-out")
cue(e, "Then forty-five free minutes. A push-out walks forty-one minutes to Dourdan; a foot squad dies at Le Port.", rate=60)
r = F(r"\[op 16\] Repelled at Le Port")
end = F(r"^end: pool", kind="note")
cue(r, "Then the strong director sits on two tokens for twenty-one minutes. Arudy's heat ran out during its rest, and tips keep it from ever being quiet, so nothing goes there.", rate=100,
    line=text(end))

# ---- cards and table ------------------------------------------------------------
cards = [
    {"lines": [("h", "Operation Long Harvest"),
               ("s", "the occupier director on Guerrilla.Abel, one engine run, %d s of game time (Day 1, 07:30 to %s)" % (round(tr.end), "%02d:%02d" % divmod(int((7 * 3600 + 30 * 60 + tr.end) // 60), 60))),
               ("p", "occupier-forces at b59db00 (df9457e plus the roadmap commit), built in build/agent2-rwdi, on the Remaster shadow of2, stock Guerrilla.Abel (poolStart 60). Every occupier decision is the engine's own; no director key was tuned."),
               ("d", "Levers (resistance side only): companions off and the legacy script QRF parked; the player invulnerable and parked in the north-west hills; each captured point set to garrison 0 and support 55 and taken by two to four invulnerable fighters; the hold squads of the Outpost and the Airfield deleted; Le Port heat raised once to 60; time x8, x4 while a force is live. One tri run, --retries 0 -j1, strict mode, 1145 s wall.")],
     "voice": "Operation Long Harvest: one engine run on the stock Abel campaign, the occupier director at its shipped settings. The script plays only the resistance. Every occupier move is the engine's own.",
     "dur": 14},
]


def kinds(s):
    return ", ".join("%s %d" % (k.lower().replace("/", " "), v) for k, v in sorted(s["ops"]["by_kind"].items(), key=lambda kv: -kv[1]))


def pk(s):
    return ", ".join("%s %d" % (k.lower().replace("transport_heli", "helicopter"), v) for k, v in sorted(s["ops"]["by_package"].items(), key=lambda kv: -kv[1]))


kd = st["k_double"]
kk = ", ".join(re.search(r"(\w+) rose.*\((\d+) town", k[1]).group(1) + " " + re.search(r"\((\d+) town", k[1]).group(1) for k in kd)
ml = st["max_live"]
mg = st["max_groups"]
warn_kinds = {}
for w in st["warn_error"]:
    key = "sound" if "SetStopValue" in w[3] else ("texture" if "texture" in w[3] else ("config" if "Config value" in w[3] else "other"))
    warn_kinds[key] = warn_kinds.get(key, 0) + w[1]
table = [
    ["Operations launched", "%d by kind: %s" % (st["ops"]["count"], kinds(st))],
    ["By package at launch", pk(st) + "; escorts: T72 (op 1), BMP (op 11); none re-planned"],
    ["Arrivals", "%d at the target, all abstract first: %d held an empty target abstractly, %d found defenders and went live (all repelled)" % (st["arrivals"].get("abstract", 0), st["arrivals"].get("abstract", 0) - st["arrivals"].get("live", 0), st["arrivals"].get("live", 0))],
    ["Births; deferred by the budget", "%d, all at route points 1520 to 5716 m from the player; deferred %d" % (sum(st["births"].values()), len(st["deferred"]))],
    ["Cancelled unseen", "%d" % len(st["cancels"])],
    ["Most bodies and groups", "live AI %d of 130 (occupier %d, resistance %d, civilian %d); groups on one side %d (resistance, empty groups of deleted squads counted); far live groups %d; traffic skippedBudget %d" % (ml["total"], ml["occ"], ml["res"], ml["civ"], mg["maxSide"], st["max_far"], st["skipped_end"])],
    ["k after the double capture", kk + " (Houdan and Le Port before): right"],
    ["Pool against poolStart 60", "never above 60 before the wave; %d of 60 after the wave landed (headroom); low %d of 60 after the strike" % (st["pool_max"][1], st["pool_min"][1])],
    ["Banked / dropped", "%d banked at the Airfield (HQ assault survivors) / %d dropped" % (st["banked_total"], st["dropped_total"])],
    ["WARN / ERRR, strict checks", "%d WARN (%s), %d ERRR; no check fired in strict mode" % (st["warn_count"], ", ".join("%s %d" % kv for kv in sorted(warn_kinds.items(), key=lambda kv: -kv[1])), st["err_count"])],
    ["Frame p95", "median %.1f ms, worst sample %.1f ms" % (st["frame_p95"]["median"], st["frame_p95"]["max"])],
    ["Verdict", "Contradicts 6.3: a defended town that repels one attack drops out of reach (its +15 heat decays inside the 30 min rest, tips keep it from being quiet), so the STRONG free 45 minutes saw 2 launches and 21 idle minutes on two tokens. The HQ fall logs '@HQ retaken: 6 stay as its garrison' after refunding them. The loop does not stop."],
]
spec = {
    "cards": cards,
    "cues": sorted(cues, key=lambda c: c["sim"]),
    "table": table,
    "table_title": "What the run counted",
    "outro_voice": "No engine error and no strict check fired, and the census never passed forty-one of a hundred and thirty. But a defended town that repels one attack drops out of the director's reach. The loop goes on.",
    "foot": "Scripts, trace and logs: tmp/of-director-video-4. Narration: OpenRouter gpt-audio-mini via arma_CWR/voice-lines/generate_voice.py.",
}
words = sum(len(c["text"].split()) for c in cues) + sum(len(c["voice"].split()) for c in cards) + len(spec["outro_voice"].split())
blob = json.dumps(spec, ensure_ascii=False)
if chr(0x2014) in blob:
    raise SystemExit("em dash in the cues")
json.dump(spec, open(out_path, "w", encoding="utf-8"), indent=1, ensure_ascii=False)
print("cues %d, spoken words %d, missing %s" % (len(cues), words, missing))
for c in spec["cues"]:
    print(round(c["sim"]), c.get("rate"), c["text"][:70], "|", c["line"][:90])
