"""Build cues.json for render_compare.py from the two main traces (and the
legacy and bubble-entry traces when present).  Every caption is pinned to
the engine line or scenario note it describes, on the scenario clock
(clock.py), and quotes the matching log line from each side.

Usage: python make_cues.py traceA.json traceB.json statsA.json statsB.json cues.json [statsBlegacy.json]
"""
import json
import re
import sys

from clock import Clock
from cue_lib import T, text

ta, tb = T(sys.argv[1]), T(sys.argv[2])
sa, sb = json.load(open(sys.argv[3])), json.load(open(sys.argv[4]))
out_path = sys.argv[5]
sl = json.load(open(sys.argv[6])) if len(sys.argv) > 6 else None
notes_a = [e for e in ta.info if e["kind"] == "note"]
notes_b = [e for e in tb.info if e["kind"] == "note"]
clk = Clock(notes_a, ta.end, notes_b, tb.end)


def C(side, e):
    return clk.to_c(side, e["sim"])


def short(e):
    return text(e)


cues = []
words = 0


def cue(c, say, la, lb, rate, **kw):
    global words
    d = {"sim": round(c, 2), "text": say, "lineA": la, "lineB": lb, "rate": rate}
    d.update(kw)
    cues.append(d)
    words += len(say.split())


# ---- floor and the double capture ----------------------------------------
fa = ta.find(r"band STRONG -> THIN \(pool 15 of 50\)")
fb = tb.find(r"band STRONG -> THIN \(pool 15 of 50\)")
cue(C("A", fa) - 0.5, "Both runs force the pool to fifteen of fifty, with the floor at thirty percent. Fifteen sits exactly on the floor, where the plan still lets the director act.", short(fa), short(fb), 0.4)
ka = ta.all(r"town\(s\) taken before", before=60)
kb = tb.all(r"town\(s\) taken before", before=60)
cue(C("A", ka[0]), "Then Houdan and Dourdan fall in the same census tick. A counts zero towns taken before Houdan and one before Dourdan. B counts one and two: the second capture leaks into the first.",
    " / ".join(e["text"] for e in ka), " / ".join(e["text"] for e in kb), 0.3)
la1 = ta.find(r"\[op 1\] Foot squad")
nb = tb.note(r"after 240 s at the floor")
cue(C("A", la1), "Two towns wake the director with two tokens, still at the floor. A spends both at once: two foot squads. B reads fifteen as below the floor and sits on its tokens for four minutes.",
    short(la1) + "  " + short(ta.find(r"\[op 2\] Foot squad")), short(nb), 4)
rb = tb.find(r"\[op 1\] APC")
cue(C("B", rb) - 3, "Back at fifty, B moves. Its men now report three fighters per town, so it buys an APC and a truck. A's walkers are already halfway.",
    short(ta.note(r"lever: pool restored")), short(rb) + "  " + short(tb.find(r"\[op 2\] Motorized")), 1.5)

# ---- births ------------------------------------------------------------------
pa = ta.note(r"resistance bodies parked")
pb = tb.note(r"resistance bodies parked")
c_step = clk.c[3]
cue(c_step - 1.0, "Ninety parked bodies crowd the 130-body budget: one more force fits, two do not. The player steps between the towns, so both forces are due in one step.", short(pa), short(pb), 0.15, inset="births")
da = ta.find_anom(r"deferred \(live squads")
b2 = tb.find(r"\[op 2\] born for Dourdan")
cue(c_step + 2, "A charges the first birth before admitting the second: the Dourdan force waits. B checks both against the same census and births both: seventeen bodies in one step, one hundred fifteen to one hundred thirty-two.", short(da), short(tb.find(r"\[op 1\] born for Village")) + "  " + short(b2), 0.3, inset="births")
ga = ta.find_anom(r"\[gmsim\] budget")
gb = tb.find_anom(r"\[gmsim\] budget")
cue(c_step + 14, "On both sides, roadside traffic then pushes the census past one hundred forty within seconds. The birth check holds the line; ambient traffic does not.", short(ga), short(gb), 3, inset="births")
ab = ta.find(r"leaves the player's world .* abstract again")
rpb = tb.find(r"\[op 2\] Repelled at Dourdan")
cue(C("A", ab), "The defended towns repel every attack. A's late Dourdan force goes back to abstract travel when the player leaves, and fights an hour later.", short(ab), short(rpb), 30)

# ---- the Outpost -------------------------------------------------------------
oa = ta.find(r"\[op 3\] Base retake")
ob_rule = tb.find(r"\[op 3\] arrival rule")
ob = tb.find(r"\[op 3\] Base retake")
cue(C("B", ob_rule) - 2, "A cell takes the Outpost and leaves it empty. B's arrival rule sees a thirty-minute walk and buys a Mi17 insertion. A sends six men on foot. Both add a jet opener on an empty base.",
    short(oa), short(ob_rule) + "  " + short(ob), 6)
ar_a = ta.find(r"\[op 3\] arrived abstract at Outpost")
ar_b = tb.find(r"\[op 3\] arrived abstract at Outpost")
cue(C("B", ar_b), "B holds the base two minutes after launch. A's walkers need almost an hour; the quote said thirty minutes, measured in a straight line.", short(ar_a), short(ar_b), 40)
c_ent = clk.c[6]
cue(c_ent - 0.5, "Now the bubble test, from a second short run per branch. The player steps into the bubble a fifth of a second before a census tick.", "", "", 0.2, inset="entry")
sua = json.load(open("suppA_trace.json"))
sub = json.load(open("suppB_trace.json"))
ent_a = [r for r in sua["fine"] if r[1] == "enter"][0]
ent_b = [r for r in sub["fine"] if r[1] == "enter"][0]
rt_b = [e for e in sub["info"] if "Outpost retaken" in e["text"]][0]
nx_a = [r for r in sua["fine"] if r[1] == "base" and r[0] > ent_a[0] and r[2] != ent_a[2]][0]
cue(c_ent + 0.5, "On B the abstract force still counts at that tick: the meter fills and the base changes hands with the player seven hundred metres away. On A the force is already off the census: the meter only decays.",
    "bubble_entry run, A: entry at %.2f s with the meter at %d; next tick %.2f s: meter %d, owner %s" % (ent_a[0], ent_a[2], nx_a[0], nx_a[2], nx_a[3]),
    "bubble_entry run, B: entry at %.2f s with the meter at %d; next tick: %s" % (ent_b[0], ent_b[2], rt_b["text"]), 0.2, inset="entry")
cue(c_ent + 8, "In the main runs the step came just after a tick, so neither side drained. The window is one second in three, which is why the fix matters only sometimes.",
    short(ta.note(r"30 s after the entry")), short(tb.note(r"30 s after the entry")), 6)

# ---- the free hour -----------------------------------------------------------
c_fr = clk.c[8]
ka2 = [e for e in ta.all(r"town\(s\) taken before") if e["sim"] > 1000]
kb2 = [e for e in tb.all(r"town\(s\) taken before") if e["sim"] > 1000]
cue(c_fr - 2, "Five more towns rise in one tick. The same leak: A counts two to six towns before, B six to ten.",
    "k: " + ", ".join(re.search(r"\((\d+) town", e["text"]).group(1) for e in ka2) + "  (" + ka2[0]["text"] + ")",
    "k: " + ", ".join(re.search(r"\((\d+) town", e["text"]).group(1) for e in kb2) + "  (" + kb2[0]["text"] + ")", 3)
hb = tb.find(r"Helicopter insertion .* to Sainte Marie")
cue(C("B", hb), "Then a free hour. B's tokens come every ten minutes: an APC to Larche, a helicopter that retakes Sainte Marie, then a push-out every ten minutes.",
    short(ta.find(r"\[op 7\] Foot squad")), short(hb), 25)
p8 = ta.find(r"\[op 8\] Push-out")
cue(C("A", p8), "A's tokens come every thirty: two operations in the hour, both on foot. Sainte Marie, Chapoi and Arudy are still the resistance's when the hour ends.", short(p8), short(tb.find(r"\[op 11\] Push-out")), 25)
lg = ""
if sl:
    n_leg = len(sl.get("stretch_launches", []))
    lg = " A third run, B with the old cadence keys, launched %d in that hour and kept B's floor, k and birth behaviour." % n_leg
cue(clk.end - 300, "Push-outs keep the cheapest package on both branches by design, so quiet towns always get walkers." + lg,
    short(ta.note(r"^end: pool")), short(tb.note(r"^end: pool")), 25)

# ---- cards and table ---------------------------------------------------------
cards = [
    {"lines": [("h", "Two branches, one scenario"),
               ("s", "the occupier director on Guerrilla.Abel, Remaster data, one tri run of the same script per branch"),
               ("A", "origin/reviewable_forces at ba2aa5c: 197a7d3 plus five engine fixes (384b283 k in a capture batch, 93cd5c9 presence in the current bubble, b6901d6 births charged per step and the pool floor ratio, bad1530 and 44a8254 cleared bindings) and the Remaster-only setup guard (d773dd5). Old cadence: a token every 1800 / 2700 / 3600 s, two operations."),
               ("B", "occupier-forces at fb9dde1: 197a7d3 plus acf2311, the retune: a token every 600 / 1050 / 1800 s, three operations while STRONG, the spare cap, the arrival rule. Without the five fixes."),
               ("d", "Levers, identical on both: a scratch Guerrilla.Abel with forcesPoolMin 50 and forcesPoolPerTown 1 (poolStart 50); director tick 10 s, wave lead 120 s, birth give-up 60 s, first-town answer chances 0; pool forced to 15 at forcesPoolFloor 0.3 for four minutes; invulnerable resistance cells; 86 to 94 parked bodies against the 130-body budget; the player teleports; time x4 to x8; --no-strict. Only commands both branches have.")],
     "voice": "Two builds of the same engine play the same scripted campaign. On the left, A: reviewable forces, five engine fixes without the retune. On the right, B: occupier forces, the retune without the fixes. The script plays only the resistance; every occupier decision is the engine's own.",
     "dur": 20, "y": 70},
]


def ops_line(s):
    bk = s["ops"]["by_package"]
    return "%d: %s" % (s["ops"]["count"], ", ".join("%s %d" % (k.lower().replace("transport_heli", "heli"), v) for k, v in sorted(bk.items())))


def arr(s):
    a = s["arrivals"]
    return "abstract %d, live %d" % (a.get("abstract", 0), a.get("live", 0))


def stretch(s):
    sp = s.get("stretch_launches", [])
    pk = s.get("stretch_packages", {})
    return "%d (%s)" % (len(sp), ", ".join("%s %d" % (k.lower(), v) for k, v in sorted(pk.items())))


def kk(t, after, before):
    return ", ".join(re.search(r"\((?:chance [0-9.]+, )?(\d+) town", e["text"]).group(1) for e in t.all(r"town\(s\) taken before", after, before))


table = [
    ["Operations (by package at launch)", ops_line(sa), ops_line(sb)],
    ["Launches in the free hour", stretch(sa), stretch(sb)],
    ["Arrivals", arr(sa), arr(sb)],
    ["Births refused by the body budget", "%d (Dourdan, live AI 128+4 of 130)" % len(sa["deferred_budget"]), "%d" % len(sb["deferred_budget"])],
    ["Live AI at the birth step / run peak (traffic included); max occupier groups", "128 / %d; %d" % (sa["max_live"]["total"], sa["max_groups"]["occ"]), "138 / %d; %d" % (sb["max_live"]["total"], sb["max_groups"]["occ"])],
    ["k after the double capture / the five", "%s / %s" % (kk(ta, 0, 60), kk(ta, 1000, 1e9)), "%s / %s" % (kk(tb, 0, 60), kk(tb, 1000, 1e9))],
    ["Pool 15 of 50 at floor 0.3", "accepted: 2 operations at the floor", "rejected: 0 operations in 240 s"],
    ["Bubble entry 0.2 s before a census tick", "no drain (72 to 70)", "drained: base retaken, player 720 m away"],
    ["Engine ERRR lines (AIUnitImpl.cpp 748)", "1", "0 (1 in the strict attempt that stopped)"],
]
if sl:
    table.append(["Third run: B with the legacy cadence keys", "(not run)", "free hour: %s; floor, k, births as B" % stretch(sl)])
spec = {
    "title": "Branch comparison: the occupier director",
    "sub": {"A": "origin/reviewable_forces  ba2aa5c  (fixes, old cadence)", "B": "occupier-forces  fb9dde1  (retune, no fixes)"},
    "cards": cards,
    "cues": sorted(cues, key=lambda c: c["sim"]),
    "table": table,
    "table_title": "What the two runs counted",
    "outro_voice": "The fixes change what happens at the edges: a floor, a batch, a step, a tick. The retune changes the tempo. They do not overlap, and B wants both.",
    "foot": "Scripts, traces and logs: tmp/of-director-video-3. Narration: OpenRouter gpt-audio-mini via arma_CWR/voice-lines/generate_voice.py.",
}
for c in cards:
    words += len(c["voice"].split())
words += len(spec["outro_voice"].split())
json.dump(spec, open(out_path, "w", encoding="utf-8"), indent=1)
print("cues %d, spoken words %d, scenario clock end %.0f s, anchors A %s B %s" % (len(cues), words, clk.end, [round(x) for x in clk.a], [round(x) for x in clk.b]))
for c in spec["cues"]:
    print(round(c["sim"]), c.get("inset", ""), c["text"][:70], "|", c["lineA"][:60], "|", c["lineB"][:60])
