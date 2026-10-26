"""Build cues.json for render.py from trace.json and stats.json (video 5).
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


# ---- Houdan, Le Port, the column in a hull ------------------------------------
e = F(r"Village rose; the occupier does not answer")
r = F(r"\[gmsim\] released 1 empty group")
cue(e, "A cell takes Houdan and leaves. Its empty group is outside the budget, and a minute later the zone tick releases it.", rate=12, line=both(e, r), dt=-1)
e = F(r"AWAKE after 2 distinct towns")
cue(e, "Four fighters take Le Port and stay. The second town wakes the director.", rate=5, dt=-1)
e = F(r"\[op 1\] Motorized squad")
cue(e, "It buys a truck with a tank escort, and the launch line now names the escort.", rate=4)
e = F(r"from Le Port Harbour appear at .* in a hull")
cue(e, "The column arrives at defended Le Port and is born whole, in its hull, five point seven kilometres from the player.", rate=2, dt=-0.5)
e = F(r"the force on Le Port reached the town edge")
r = F(r"\[op 1\] Repelled at Le Port")
cue(e, "The truck stops two hundred and forty metres out and the squad dismounts. The tank drives on to the edge and supports. Repelled.", rate=7, line=both(e, r), dt=-0.5)

# ---- the Outpost and the double capture ----------------------------------------
n = F(r"lever: the Outpost hold squad", kind="note")
e = F(r"\[op 3\] arrival rule")
cue(n, "The Outpost is raided and emptied. Walking takes fifty-five minutes, so a helicopter retakes it.", rate=4, line=text(e))
a = F(r"Chapoi rose; the occupier director takes it")
r = F(r"\[gmsim\] released 2 empty group")
cue(a, "Dourdan and Chapoi fall in one tick, counted two and three. Both cells leave their groups behind; the two are released a minute later.", rate=14, line=both(a, r), dt=-0.5)

# ---- Arudy and Le Port: repels, rests, re-arms ----------------------------------
e = F(r"\[op 5\] APC")
r = F(r"\[op 5\] Repelled at Arudy")
cue(e, "Three fighters hold Arudy for the rest of the run. A BMP comes, born in its hull, and is repelled.", rate=12, line=both(e, r))
e = F(r"Le Port rested with 4 fighter\(s\) reported: re-armed")
o = F(r"\[op 6\] Motorized squad")
cue(e, "Le Port's rest ends with its fighters still reported. In video four such a town dropped out of reach; now it is re-armed and attacked with the next token.", rate=10, line=both(e, o))
n = F(r"lever: the Airfield hold squad", kind="note")
cue(n, "The Airfield's hold squad is deleted too. The capture script still holds its group, so the sweep leaves it alone.", rate=8)
w = F(r"Wave 1 \(30\) inbound")
cue(w, "Raiders empty the Airfield, control falls to sixty-nine percent, and wave one is called.", rate=12)
e = F(r"\[op 7\] Wave 1 strike: HQ assault")
r = F(r"\[op 8\] Repelled at Arudy")
cue(e, "Its strikes keep the minimal package: six men march on the headquarters, forty minutes by road, and Arudy repels nine more, its second repel.", rate=25, line=both(e, r))
e = F(r"\[op 10\] arrival rule")
cue(e, "A push-out on the empty Airfield would walk eighty-six minutes; push-outs now take the arrival rule, so it flies.", rate=14)
e = F(r"HQ at Camp destroyed")
h = F(r"\[op 7\] HQ assault done")
cue(e, "The six reach the empty headquarters and it falls. The settled line now says where they went: four to the pool, two dropped.", rate=4, line=both(e, h), dt=-1)
e = F(r"Arudy rested with 3 fighter\(s\) reported: re-armed")
o = F(r"\[op 12\] APC")
cue(e, "Arudy's rest ends in the same minute. Re-armed, and the third attack leaves in the same tick.", rate=3, line=both(e, o), dt=-0.5)
e = F(r"\[op 12\] Repelled at Arudy")
cue(e, "One attacker lives on, forty-seven metres from the centre, for eighteen minutes, until the live timeout ends the operation.", rate=60, dt=-1100)

# ---- the strike ---------------------------------------------------------------
e = F(r"no strike with the operation on Le Port")
cue(e, "Le Port is made urgent and gets two BMPs. A jet or a helicopter on top would leave the pool below strong, so the director buys no strike.", rate=6, dt=-0.5)
e = F(r"the force on Le Port reached the town edge", after=8500)
cue(e, "Both BMPs drive to the edge, and the escort supports from there.", rate=5, dt=-0.5)
n = F(r"lever: FALLBACK, no director-bought strike", kind="note")
b = F(r"band STRONG -> STRETCHED \(pool 29")
cue(n, "The test lever forces the jet anyway, exempt from the rule. The pool falls to twenty-nine for thirty-seven seconds.", rate=4, line=both(F(r"\[op 16\] Jet strike over Le Port"), b))

# ---- the rising and the free stretch ---------------------------------------------
n = F(r"^control .* after the rising", kind="note")
b = F(r"band STRONG -> STRETCHED \(pool 31")
cue(n, "The cell rises at three points. Two helicopters retake the Outpost and Houdan at twelve units each, and the pool dips to stretched for a hundred seconds.", rate=25, line=text(b), dt=-170)
n = F(r"^free running from here", kind="note")
o = F(r"\[op 19\] Push-out")
cue(n, "Then fifty minutes with no levers. A push-out walks to Dourdan, eleven minutes, under the fifteen-minute rule.", rate=40, line=text(o))
e = F(r"Le Port rested with 4 fighter\(s\) reported: re-armed", after=10000)
o = F(r"\[op 20\]")
cue(e, "Every rest that ends on a reported town is re-armed and attacked within one token interval. Between attacks both towns rest and nothing is quiet, so no token waits on a candidate.", rate=110, line=both(e, o))
e = F(r"\[op 20\] Repelled at Le Port")
cue(e, "On the left, the census: live AI never passes forty-six of a hundred and thirty, no spawn waits on the budget, and frames stay near eleven milliseconds.", rate=60, dt=60)
e = F(r"\[op 21\] APC")
end = F(r"^end: pool", kind="note")
cue(e, "Arudy's fifth attack goes as its rest ends. Five operations, five repels: the director never stopped coming.", rate=40, line=text(end), dt=-0.5)

# ---- cards and table ------------------------------------------------------------
lever_card = ("Levers (resistance side only): companions off and the legacy script QRF parked; the player invulnerable in the north-west hills; "
              "each captured point set to garrison 0 and support 55 and taken by two to four invulnerable fighters (Le Port four and Arudy three stay to the end); "
              "the Outpost and Airfield hold squads deleted; Le Port heat raised once to 60 and one jet strike forced by the test lever; "
              "spent cells removed with deleteGroup, the Houdan, Dourdan and Chapoi cells with deleteVehicle only; time x8, x4 while a force is live; "
              "the last 50 minutes free. One tri run, --retries 0 -j1, strict mode, 2090 s wall.")
cards = [
    {"lines": [("h", "Operation Grey Orchard"),
               ("s", "a fictional campaign on Guerrilla.Abel, one engine run, %d s of game time (Day 1, 07:30 to %s)" % (round(tr.end), "%02d:%02d" % divmod(int((7 * 3600 + 30 * 60 + tr.end) // 60), 60))),
               ("p", "occupier-forces at f197544 (round 3.5: 06414a4, 0c43877, 1ae2dc9), built in build/agent2-rwdi, on the Remaster shadow of2, stock Guerrilla.Abel (poolStart 60). Every occupier decision is the engine's own; no director key was tuned."),
               ("d", lever_card)],
     "voice": "Operation Grey Orchard: one engine run on the stock Abel campaign, after round three and a half. The script plays only the resistance; every occupier move is the engine's own.",
     "dur": 18},
]


def kinds(s):
    return ", ".join("%s %d" % (k.lower().replace("/", " "), v) for k, v in sorted(s["ops"]["by_kind"].items(), key=lambda kv: -kv[1]))


def pk(s):
    return ", ".join("%s %d" % (k.lower().replace("transport_heli", "helicopter"), v) for k, v in sorted(s["ops"]["by_package"].items(), key=lambda kv: -kv[1]))


ml = st["max_live"]
mg = st["max_groups"]
rf = st["rearm_follow"]
same = sum(1 for r in rf if r[3] is not None and r[3] <= 60)
waited = [r[3] for r in rf if r[3] is not None and r[3] > 60]
esc = {}
for o, e in st["ops"]["escorts"]:
    esc[e] = esc.get(e, 0) + 1
warn_kinds = {}
for w in st["warn_error"]:
    key = "sound" if "SetStopValue" in w[3] else ("texture" if "texture" in w[3] else ("config" if "Config value" in w[3] else "other"))
    warn_kinds[key] = warn_kinds.get(key, 0) + w[1]
hull_births = len(st["hull_births"])
table = [
    ["Operations launched", "%d: %s" % (st["ops"]["count"], kinds(st))],
    ["By package; escorts", pk(st) + "; escorts " + ", ".join("%s %d" % kv for kv in sorted(esc.items())) + ", each named in its launch line"],
    ["Arrivals; births", "%d at the target, %d of them against defenders went live; %d births, %d in a hull (the other an insertion's infantry, on foot by rule); deferred %d, cancelled %d" % (
        st["arrivals"].get("abstract", 0), st["arrivals"].get("live", 0), sum(st["births"].values()), hull_births, len(st["deferred"]), len(st["cancels"]))],
    ["Re-arms", "%d (Le Port %d, Arudy %d); %d launched in the same tick, %s after a token wait; Arudy: %d operations, %d repelled" % (
        len(rf), sum(1 for r in rf if r[1] == "Le Port"), sum(1 for r in rf if r[1] == "Arudy"), same, " and ".join("%d s" % x for x in waited) or "none",
        len(st["arudy_ops"]), len(st["arudy_repels"]))],
    ["Idle while STRONG", "%.0f minutes with a token, a free slot and a reported, rested, unattacked held town" % st["idle_strong_min"]],
    ["Strikes", "%d refused by the band rule (pool 41 of 60 after the ground debit); %d forced by the test lever (exempt): STRONG to STRETCHED for 37 s" % (len(st["strike_refusals"]), len(st["forced_strike"]))],
    ["Push-outs", "%d: foot 652 s twice (under 900 s), one Mi17 base retake (foot would take 5157 s)" % len(st["pushouts"])],
    ["HQ settled line", "'HQ assault done: 4 back to the pool, 2 dropped (every base full)'; the retake line never appears"],
    ["Banked / dropped", "%d banked / %d dropped, every base at garrisonMax" % (st["banked_total"], st["dropped_total"])],
    ["Empty groups", "peak %d, released %d (Houdan 1, Dourdan and Chapoi 2); %d at the end, held by capture.sqs hold-group handles" % (st["empty_peak"][1], st["released_total"], st["empty_end"])],
    ["Census", "live AI at most %d of 130 (occupier %d, resistance %d, civilian %d); groups on one side %d; far live groups %d; live operations %d of 3; skippedBudget %d" % (
        ml["total"], ml["occ"], ml["res"], ml["civ"], mg["maxSide"], st["max_far"], st["max_live_ops"], st["skipped_end"])],
    ["WARN / ERRR", "%d WARN (%s), %d ERRR; none from the director, the census or a strict check" % (st["warn_count"], ", ".join("%s %d" % kv for kv in sorted(warn_kinds.items(), key=lambda kv: -kv[1])), st["err_count"])],
    ["Frame p95", "median %.1f ms, worst sample %.1f ms" % (st["frame_p95"]["median"], st["frame_p95"]["max"])],
    ["Verdict", "No finding contradicts OCCUPIER-FORCES.md or the contract: no director or engine WARN or ERRR, no operation stuck past the live timeout, no idling while STRONG with a candidate. The loop can stop; the remaining items are tuning."],
]
spec = {
    "cards": cards,
    "cues": sorted(cues, key=lambda c: c["sim"]),
    "table": table,
    "table_title": "What the run counted",
    "outro_voice": "No director warning, no engine error, no strict check, and the census never passed forty-six of a hundred and thirty. Every re-armed town was attacked within one token interval. Nothing in this run contradicts the design: the loop can stop.",
    "foot": "Scripts, trace and logs: tmp/of-director-video-5. Narration: OpenRouter gpt-audio-mini via arma_CWR/voice-lines/generate_voice.py.",
}
words = sum(len(c["text"].split()) for c in cues) + sum(len(c["voice"].split()) for c in cards) + len(spec["outro_voice"].split())
blob = json.dumps(spec, ensure_ascii=False)
if chr(0x2014) in blob:
    raise SystemExit("em dash in the cues")
json.dump(spec, open(out_path, "w", encoding="utf-8"), indent=1, ensure_ascii=False)
print("cues %d, spoken words %d, missing %s" % (len(cues), words, missing))
for c in spec["cues"]:
    print(round(c["sim"]), c.get("rate"), len(c["text"]), c["text"][:70], "|", c["line"][:90])
for r in table:
    print(" ", r[0], ":", r[1])
