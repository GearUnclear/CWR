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
cue(e, "Four fighters take Arudy and stay. The first town gets the old forty percent roll, and this time nobody answers.", rate=5, dt=-1)
e = F(r"AWAKE after 2 distinct towns")
cue(e, "Six take Sainte Marie and stay. The second town wakes the director, with a pool of forty-eight: the two generated outposts are paid out of it.", rate=5, dt=-0.5)
e = F(r"\[op 2\] arrival rule")
cue(e, "From the new Vigny Outpost, walking to Arudy would take twenty-two minutes, past the fifteen-minute rule, so a truck goes. Neither attack knows of a single defender.", rate=4)
e = F(r"\[op 2\] Broken off at Arudy")
cue(e, "Three of four riflemen fall and the last one breaks off. The town is still held, so the force withdraws at once and its truck follows it home.", rate=6, dt=-0.5)
e = F(r"Sainte Marie: first repel")
cue(e, "First rung of the repel ladder: the next attack on Sainte Marie needs half again the lethality, and ten infantry, twice the fighters it has seen.", rate=10, dt=-0.3)

# ---- the ladder climbs --------------------------------------------------------------
e = F(r"\[op 3\] the repel ladder on Arudy")
b = F(r"band STRONG -> STRETCHED \(pool 27")
cue(e, "Arudy's rest ends with its four fighters reported. The ladder buys a truck and a tank: twenty-one units leave the pool, and the band drops to stretched on that spend alone.", rate=5, line=both(e, b), dt=-0.5)
e = F(r"massing for Sainte Marie: need 12 infantry")
cue(e, "No base can spare twelve men, so the director holds its token and masses. Regrowth adds one body per base every ten minutes; three minutes of massing change nothing.", rate=4, dt=-0.5)
e = F(r"band STRONG -> THIN \(pool 4")
o = F(r"\[op 4\] APC")
cue(e, "Then the strongest single-base package goes anyway: a BMP and a T72, twenty-eight units. The pool falls from thirty-two to four, strong to thin in one tick.", rate=3, line=both(o, e), dt=-0.5)
e = F(r"\[op 3\] Broken off at Arudy")
r = F(r"Arudy: second repel")
cue(e, "Arudy beats the tank column too: six dead, two walk home. Second rung: the next attack comes from a different source, or by air.", rate=6, line=both(e, r), dt=-0.5)
e = F(r"\[op 5\] the repel ladder on Arudy")
s = F(r"\[op 6\] escalation strike")
cue(e, "So the third attack on Arudy flies, with an attack helicopter strike on top. The rule tests the pool, which reads stretched; the line says strong, because both airframes count as committed.", rate=4, line=both(e, s), dt=-0.5)
e = F(r"\[op 6\] Struck Arudy")
cue(e, "The strike cuts Arudy's standing to ten. Its locals stop talking to the occupier, and that matters later.", rate=5, dt=-0.5)
e = F(r"Arudy: third repel")
cue(e, "Third repel: Arudy is contained. No more attacks on it; four bodies surge the Vigny Outpost instead.", rate=4, dt=-0.5)
e = F(r"Sainte Marie: third repel")
o = F(r"\[op 7\] Helicopter insertion")
cue(e, "Sainte Marie gets six men by helicopter, a quarter of the lethality the ladder asked for, and repels them. Contained, and La Riviere Outpost is surged by eight.", rate=8, line=both(o, e), dt=-0.5)

# ---- the rising and the wave ----------------------------------------------------------
n = F(r"^lever: La Riviere Outpost garrison 0", kind="note")
c = F(r"Arudy: containment lifted: control")
cue(n, "Now the scripted rising: raiders take both generated outposts, Chapoi and Cancon. Control falls to seventy-two percent, and both containments lift.", rate=25, line=text(c))
e = F(r"Wave 1 \(30\) inbound")
cue(e, "Wave one is called: thirty bodies, ten minutes out.", rate=8, dt=-0.5)
e = F(r"Wave 1: strikes on the cluster around Arudy")
p = F(r"Wave 1: \d+ stay in the pool")
cue(e, "It tops up the Airfield, flies six men at Arudy, and sends a two-man column on an eighty-five minute walk. Three bodies stay in the pool, below the ceiling, so no surge is ever needed.", rate=4, line=both(e, p), dt=-0.3)
e = F(r"\[op 10\] Broken off at Arudy")
cue(e, "Arudy repels the wave strike as well, and is contained again.", rate=10, dt=-0.5)

# ---- the stuck truck ----------------------------------------------------------------------
e = F(r"\[op 13\] the repel ladder on Sainte Marie")
cue(e, "Sainte Marie's rest ends. Its fourth attack, nine men in a truck with a tank, crosses country from the Outpost.", rate=12, dt=-0.5)
e = F(r"\[op 13\] born for Sainte Marie")
cue(e, "Here is this run's defect. The truck is born two hundred and ninety-seven metres out and never moves again. Its nine men never get out.", rate=25, dt=-0.5)
e = F(r"escort T72 of the force on Sainte Marie is lost")
cue(e, "The defenders' RPG kills the tank escort. The truck still sits at the edge, full.", rate=40, dt=-0.5)
e = F(r"\[op 13\] Withdrew from Sainte Marie")
cue(e, "After twenty-seven minutes the live timeout ends it: none lost, nine going home, and the director books it as a repel.", rate=3, dt=-0.5)
e = F(r"band STRONG -> STRETCHED \(pool 23")
cue(e, "In the same second the band line says stretched, then strong. The pool never left thirty-four: the line was written halfway through one booking.", rate=3, dt=0.2)

# ---- the end ---------------------------------------------------------------------------------
e = F(r"Arudy: containment lifted: its reports decayed")
cue(e, "With its locals silent, Arudy's reports decay, and push-outs creep back with no fighter reported. Sainte Marie's locals keep talking, so it stays contained to the end.", rate=40, dt=-0.5)
end = F(r"^end: pool", kind="note")
cue(end, "Three and a half hours: sixty-four occupiers lost, eleven operations on the two towns, and both still held.", rate=4, dt=-30)

# ---- cards and table ---------------------------------------------------------------------------
lever_card = ("Levers (resistance side only): forcesLog on (a diagnostic); the player and his companion invulnerable in the north-western hills; "
              "Arudy (four fighters) and Sainte Marie (six) set to garrison 0 and support 55 and held to the end by invulnerable fighters kept within 120 m, "
              "magazines topped up every 300 s; a rising at 98 min: both generated outposts, Chapoi and Cancon raided by invulnerable cells that leave once each falls; "
              "time x8, x4 while a force is live. No director key tuned. One tri run, --retries 0 -j1, strict mode, 2064 s wall.")
cards = [
    {"lines": [("h", "Operation Stone Lantern"),
               ("s", "a fictional campaign on Guerrilla.Abel, one engine run, %d s of game time (Day 1, 07:30 to %s)" % (round(tr.end), "%02d:%02d" % divmod(int((7 * 3600 + 30 * 60 + tr.end) // 60), 60))),
               ("p", "occupier-forces at e9698d8 (the eight levers a5b40f0, generated bases step 3), built in build/agent67-rwdi, on the Remaster shadow of6, stock Guerrilla.Abel at forcesDifficulty Normal (poolStart 48). Two towns taken and held by their garrisons."),
               ("d", lever_card)],
     "voice": "Operation Stone Lantern: one engine run on the stock Abel campaign, the eight levers at Normal. Two towns are taken and held by real garrisons. The script plays only the resistance; every occupier move is the engine's own.",
     "dur": 18},
]

ml = st["max_live"]
mg = st["max_groups"]
bt = st["band_time"]
gs = st["launch_gap_stats"]
lad = st["ladder"]


def rungs(z):
    return ", ".join("%s %d" % (x[1], x[0]) for x in lad.get(z, []) if x[1] not in ("applied",))


table = [
    ["1. Outside STRONG vs kills", "%.1f %% outside STRONG (STRETCHED %d s, THIN %d s); occupier lost %d in %.1f h, %.0f an hour, the worst hour %d (refill %.0f an hour). Every demotion followed a launch debit; losses held the strength under the pool for 86 min but changed the band for only 9 min" % (
        100 * st["outside_strong_share"], bt.get("STRETCHED", 0), bt.get("THIN", 0), st["lost_end"], st["sim_end"] / 3600.0, st["lost_per_hour"], st["lost_peak_hour"]["lost"], st["refill_per_hour"])],
    ["2. Launch spacing", "%d gaps, median %d s, min %d (the wave's strike and column in one tick), max %d; per town: Arudy %s s, Sainte Marie %s s" % (
        gs["n"], gs["median"], gs["min"], gs["max"], ", ".join(str(x) for x in st["town_gaps"]["Arudy"]), ", ".join(str(x) for x in st["town_gaps"]["Sainte Marie"]))],
    ["3. Urgent share", "0 of %d ground launches urgent; the one urgent line is the escalation strike (strikes are always urgent)" % len(st["launch_lines"])],
    ["4. Slow legs", "%d arrival-rule lines, two in the 900 to 1500 s window (FOOT 1308 s from Vigny Outpost to Arudy, a truck went); 3 helicopters (two ladder rung 2, one wave strike)" % len(st["slow_legs"])],
    ["5. Lethality vs reported", "first attacks 5.0 against 0 reported; rung 1 on Arudy 22.0 of 21.6; every later ladder plan short: 25.5, 11.25, 7.5 and 23.25 against 21.6 to 32.4 (the most lethal affordable)"],
    ["6. Three in flight", "born operations peaked at %d of 3; never three" % st["born_max"][1]],
    ["7. First wave", "staged: fired at %d s at control 0.72 after the rising, landed 600 s later; 1 strike target, 1 reinforce column, 3 left in the pool" % st["wave1"][0]],
    ["8. Peaks", "live AI %d of 130 (occupier %d, resistance %d, civilian %d); groups on one side %d of 40" % (ml["total"], ml["occ"], ml["res"], ml["civ"], mg["maxSide"])],
    ["Repel ladder", "Arudy: %s. Sainte Marie: %s" % (rungs("Arudy"), rungs("Sainte Marie"))],
    ["Massing, break-off", "%d massing holds, all ended by the 3-tick limit, none could staff; %d break-offs, %d live-timeout withdrawals; hulls left after their infantry each time" % (
        sum(1 for m in st["massing"] if " ends " in m[1]), len(st["breakoffs"]), len(st["withdrawals"]))],
    ["Surge, spare hulls", "wave surge: none (the wave was spent at landing, the pool never at the ceiling); stored spare hulls: 1 (an APC at Le Port Harbour, back in the pool 96 s later); containment surges %d" % len(st["contained_lines"])],
    ["WARN / ERRR", "%d WARN (sound and texture), %d ERRR; none from the director" % (st["warn_count"], st["err_count"])],
    ["Verdict", "Three findings contradict the docs: a carrier stuck at 297 m with its infantry seated for 27 min (6.5); band lines written on a half-booked ledger (5.6); Malden's pool figures still 60 (5.6, 6.3). The loop does not stop."],
]
spec = {
    "cards": cards,
    "cues": sorted(cues, key=lambda c: c["sim"]),
    "table": table,
    "table_title": "The audit's eight numbers, and what else the run counted",
    "outro_voice": "Three findings for the next round: a truck that sits at the edge with its riflemen inside, band lines written halfway through a booking, and the plan's Malden numbers, which still assume a pool of sixty. The loop goes on.",
    "foot": "Scripts, trace and logs: tmp/of-director-video-6. Narration: OpenRouter gpt-audio-mini via arma_CWR/voice-lines/generate_voice.py.",
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
