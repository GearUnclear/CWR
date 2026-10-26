"""Build cues.json for render.py from trace.json and stats.json (video 6).
Every caption is pinned to the engine line or scenario note it describes and
quotes that line under the caption.

Usage: python make_cues.py trace.json stats.json cues.json
"""
import json
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


# ---- the two towns ---------------------------------------------------------------
e = F(r"Arudy rose; the occupier does not answer")
cue(e, "The same scenario as video six, on the round five build. Four fighters take Arudy and stay; the first town's forty percent roll does not answer.", rate=5, dt=-1)
e = F(r"AWAKE after 2 distinct towns")
cue(e, "Six take Sainte Marie and stay. The second town wakes the director, with a pool of forty-eight.", rate=5, dt=-0.5)
e = F(r"\[op 2\] Broken off at Arudy")
cue(e, "The first two trucks are beaten the same way as before: three of four riflemen fall, the last one breaks off, the truck follows him home.", rate=8, dt=-0.5)

# ---- the ladder climbs --------------------------------------------------------------
e = F(r"\[op 3\] the repel ladder on Arudy")
b = F(r"band STRONG -> STRETCHED \(pool 27")
cue(e, "Arudy is re-armed and the ladder buys a truck with a tank: twenty-one units, and the band drops to stretched.", rate=5, line=both(e, b), dt=-0.5)
e = F(r"massing for Sainte Marie: need 12 infantry")
cue(e, "No base can spare twelve men for Sainte Marie, so the director masses for three ticks.", rate=4, dt=-0.5)
e = F(r"band STRETCHED -> THIN \(pool 4")
o = F(r"\[op 4\] APC")
cue(e, "Then a BMP and a T72 go anyway. One band line: stretched to thin, pool four. Video six wrote two lines here in one second; the band now steps once per frame.", rate=3, line=both(o, e), dt=-0.5)
e = F(r"\[op 3\] Broken off at Arudy")
cue(e, "Arudy beats the tank column: six dead, two walk home. Second rung: a different source, or the air.", rate=8, dt=-0.5)
e = F(r"\[op 6\] escalation strike")
o = F(r"\[op 5\] the repel ladder on Arudy")
cue(e, "A helicopter insertion with an attack helicopter strike on top. The strike line now names the band its rule tested: the pool stays stretched, sixteen of forty-eight.", rate=4, line=both(o, e), dt=-0.3)
e = F(r"Arudy: third repel")
cue(e, "Third repel: Arudy is contained, and the Vigny Outpost is surged by six instead.", rate=5, dt=-0.5)
e = F(r"Sainte Marie: third repel")
cue(e, "Sainte Marie repels six men by helicopter and is contained too; La Riviere Outpost is surged by eight.", rate=8, dt=-0.5)

# ---- the rising and the wave ----------------------------------------------------------
n = F(r"^lever: La Riviere Outpost garrison 0", kind="note")
c = F(r"Arudy: containment lifted: control")
cue(n, "The scripted rising takes both generated outposts, Chapoi and Cancon. Control falls to seventy-two percent and both containments lift.", rate=25, line=text(c))
e = F(r"Wave 1 \(30\) inbound")
cue(e, "Wave one is called: thirty bodies, ten minutes out.", rate=8, dt=-0.5)
e = F(r"Wave 1: strikes on the cluster around Arudy")
o = F(r"\[op 10\] Wave 1 strike")
cue(e, "The wave's strike budget reads fifteen, but its one strike costs twenty-one: twelve for the helicopter and its crew, nine staged men. Nothing of the wave stays in the pool.", rate=3, line=both(e, o), dt=-0.3)
e = F(r"\[op 9\] the Ural carrying the force on Vigny Outpost made no progress")
cue(e, "The new stall rule fires for the first time: a truck stuck two hundred and ninety-five metres from the Vigny Outpost lets its men out after forty-five seconds.", rate=5, dt=-0.5)
e = F(r"\[op 10\] Broken off at Arudy")
cue(e, "Arudy repels the wave strike, seven dead, and is contained again.", rate=10, dt=-0.5)

# ---- the stuck truck, fixed ---------------------------------------------------------------
e = F(r"\[op 13\] the repel ladder on Sainte Marie")
cue(e, "Sainte Marie's fourth attack: nine men in a truck with a tank, across country from the Outpost, the attack that stalled in video six.", rate=12, dt=-0.5)
e = F(r"\[op 13\] born for Sainte Marie")
cue(e, "The truck is born at the same spot, two hundred and ninety-seven metres out, and again it does not move.", rate=6, dt=-0.5)
e = F(r"\[op 13\] the Ural carrying the force on Sainte Marie made no progress")
cue(e, "After forty-five seconds the stall rule puts the nine men out where they stand, and they assault the town.", rate=6, dt=-0.5)
e = F(r"\[op 13\] Broken off at Sainte Marie")
cue(e, "Seven fall and two break off, inside four minutes instead of twenty-seven. Contained again.", rate=8, dt=-0.5)
e = F(r"Arudy: containment lifted: its reports decayed")
cue(e, "Arudy's locals, silent since the strike, never report it again, so its containment decays and push-outs come back with no fighter reported.", rate=40, dt=-0.5)
e = F(r"Le Port Harbour reinforced")
cue(e, "The wave's two-man column finally arrives, and its line now says where the two went: to the pool, in the wave headroom.", rate=20, dt=-0.5)
end = F(r"^end: pool", kind="note")
cue(end, "Three and a half hours: seventy-four occupiers lost, eleven operations on the two towns, both still held.", rate=4, dt=-30)

# ---- cards and table ---------------------------------------------------------------------------
lever_card = ("Levers (resistance side only): forcesLog on (a diagnostic); the player and his companion invulnerable in the north-western hills; "
              "Arudy (four fighters) and Sainte Marie (six) set to garrison 0 and support 55 and held to the end by invulnerable fighters kept within 120 m, "
              "magazines topped up every 300 s; a rising at 100 min: both generated outposts, Chapoi and Cancon raided by invulnerable cells that leave once each falls; "
              "time x8, x4 while a force is live. No director key tuned. The same scenario as video 6. One tri run, --retries 0 -j1, strict mode, 1905 s wall.")
cards = [
    {"lines": [("h", "Operation Stone Lantern, run two"),
               ("s", "a fictional campaign on Guerrilla.Abel, one engine run, %d s of game time (Day 1, 07:30 to %s)" % (round(tr.end), "%02d:%02d" % divmod(int((7 * 3600 + 30 * 60 + tr.end) // 60), 60))),
               ("p", "occupier-forces at c688fa9 (round 5: e130e2a the band once per frame, honest strike and top-up lines; c688fa9 the stall rule), built in build/agent67-rwdi, on the Remaster shadow of6, stock Guerrilla.Abel at forcesDifficulty Normal (poolStart 48). Two towns taken and held by their garrisons."),
               ("d", lever_card)],
     "voice": "Operation Stone Lantern, run two: the same scenario as video six, on the build that fixed its five findings. Two towns are taken and held by real garrisons; every occupier move is the engine's own.",
     "dur": 18},
]

ml = st["max_live"]
mg = st["max_groups"]
bt = st["band_time"]
gs = st["launch_gap_stats"]
lad = st["ladder"]


def rungs(z):
    return ", ".join("%s %d" % (x[1], x[0]) for x in lad.get(z, []) if x[1] not in ("applied",))


v6 = json.load(open(r"D:\Arma_CWA\tmp\of-director-video-6\stats.json"))


def nums(x):
    bt = x["band_time"]
    return ("%.1f %% (STRETCHED %d s, THIN %d s); lost %d, %.1f an hour, worst hour %d" % (
        100 * x["outside_strong_share"], bt.get("STRETCHED", 0), bt.get("THIN", 0), x["lost_end"], x["lost_per_hour"], x["lost_peak_hour"]["lost"]))


def gaps(x):
    g = x["launch_gap_stats"]
    return "median %d s, max %d; Arudy %s; Sainte Marie %s" % (g["median"], g["max"], ", ".join(str(v) for v in x["town_gaps"]["Arudy"]), ", ".join(str(v) for v in x["town_gaps"]["Sainte Marie"]))


def peaks(x):
    return "live AI %d of 130, groups %d of 40" % (x["max_live"]["total"], x["max_groups"]["maxSide"])


table = [
    ["", "video 6 (e9698d8)  |  video 7 (c688fa9)"],
    ["1. Outside STRONG vs kills", nums(v6) + "  |  " + nums(st) + " (refill 37 an hour)"],
    ["2. Launch spacing", gaps(v6) + "  |  " + gaps(st)],
    ["3. Urgent share", "0 of %d  |  0 of %d ground launches (only the escalation strike, always urgent)" % (len(v6["launch_lines"]), len(st["launch_lines"]))],
    ["4. Slow legs", "%d arrival-rule lines, 2 in 900 to 1500 s  |  %d, %d in 900 to 1500 s (FOOT 1308 s, a truck went)" % (len(v6["slow_legs"]), len(st["slow_legs"]), len(st["slow_legs_900_1500"]))],
    ["5. Lethality vs reported", "both: first attacks 5.0 against 0; rung 1 on Arudy 22.0 of 21.6; later plans short (25.5, 11.25, 7.5, 23.25 against 21.6 to 32.4)"],
    ["6. Three in flight", "peak %d  |  peak %d born operations of 3" % (v6["born_max"][1], st["born_max"][1])],
    ["7. First wave", "fired %d s, control 0.72  |  fired %d s, control 0.72 (staged by the rising)" % (v6["wave1"][0], st["wave1"][0])],
    ["8. Peaks", peaks(v6) + "  |  " + peaks(st)],
    ["Ladder, massing, break-off", "both: 7 operations on Arudy, 4 on Sainte Marie, all held; 4 massing holds, none could staff  |  break-offs %d / %d, live-timeout withdrawals %d / %d" % (len(v6["breakoffs"]), len(st["breakoffs"]), len(v6["withdrawals"]), len(st["withdrawals"]))],
    ["Video 6 findings", "stuck carrier: fixed (two stall-rule lines, op 13 out after 45 s); same-second band pairs: fixed (0); escalation strike line: fixed; reinforce line: fixed; Abel figures: fixed in the docs"],
    ["New finding (low)", "7.1 and 16 say strikes take 'up to 0.5 of the wave'; the share counts staged infantry only (6.5 puts hull and crew on the pool), so the one strike took 21 of the 30-unit wave against 'up to 15' (video 6: 18). A wording fix or a code change"],
    ["WARN / ERRR", "%d / %d WARN (sound, texture, config), 0 ERRR, none from the director" % (v6["warn_count"], st["warn_count"])],
    ["Verdict", "Every video 6 finding is fixed, and nothing else in the run contradicts the docs. One low item: 7.1 and 16 describe the wave strike share in units, the code counts infantry. Fix that wording or the code and the loop can stop."],
]
spec = {
    "cards": cards,
    "cues": sorted(cues, key=lambda c: c["sim"]),
    "table": table,
    "table_title": "Video 6 and video 7 side by side",
    "outro_voice": "All five findings of video six are fixed: the stuck truck now lets its men out, the band steps once, and the lines say what the rule tested. One low item is left: the wave's strike share counts only infantry, so its strike spent twenty-one of thirty units while the plan says half. Settle that wording, and the loop can stop.",
    "foot": "Scripts, trace and logs: tmp/of-director-video-7. Narration: OpenRouter gpt-audio-mini via arma_CWR/voice-lines/generate_voice.py.",
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
