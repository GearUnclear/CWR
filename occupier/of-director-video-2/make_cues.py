"""Build cues.json from trace.json: every commentary line is pinned to the first
engine log line (or scenario UDNOTE) matching its pattern after the previous
cue, so the captions are timed to the events they describe.

Usage: python make_cues.py <trace.json> <cues.json>
"""
import json
import re
import sys

CARDS = [
    {"card": [
        ("h", "Operation Low Tide"),
        ("s", "the occupier director, decision by decision"),
        ("p", "A fictional campaign on the shipped Guerrilla.Abel template, Remaster data. This is ENGINE OUTPUT: one tri test run of the real game. Every occupier choice, number and position on screen comes from the engine, sampled every 10 s of game time, and the green log lines are quoted from the game log."),
    ], "dur": 9.0},
    {"card": [
        ("h", "What the scenario script did"),
        ("p", "It plays only the resistance: which cell takes which zone, and where the player stands. Test levers, each named on screen when used: director tick 10 s (60 in play), wave lead 120 s (600), birth give-up 60 s (600), time acceleration x4 to x8, garrison reserves set to 0 or 2 to stand in for raids, one heat raise to stand in for an ambush, and invulnerable resistance fighters so the held town stays held."),
        ("d", "The occupier side was never scripted."),
    ], "dur": 10.0},
]

# (pattern on the event text, caption = voice line, optional engine-line pattern, rate)
SPEC = [
    (r"lever: Houdan garrison", "Day one. A resistance cell takes Houdan, beside the player's camp. The director still sleeps.", r"Village taken", 4),
    (r"Village rose; the occupier does not answer", "A first town only gets the old forty percent roll. This time, no answer.", None, 4),
    (r"lever: Le Port garrison", "A second cell takes Le Port on the south coast. Four fighters stay to hold it.", r"Le Port taken", 3),
    (r"AWAKE after 2", "Two towns wake the director. It holds two command tokens and spends them on the hottest towns.", None, 3),
    (r"\[op 1\] Foot squad", "Four men leave the harbour for Le Port. Nothing spawns: the force is a record with a clock.", None, 6),
    (r"\[op 2\] Foot squad", "Four more walk from the Outpost to Houdan. No fighters reported, so it buys the minimum.", None, 10),
    (r"a local in Le Port talked", "Its intel comes only from its own men or from locals. A local in Le Port reports four fighters.", None, 10),
    (r"\[op 2\] arrived abstract at Village", "Houdan was left empty. The squad arrives without ever being born and drains the town.", None, 2),
    (r"\[op 1\] arrived abstract at Le Port", "Le Port has defenders, so that fight must be real: the squad is born on its road, unseen, four kilometres away.", r"\[op 1\] born for Le Port", 3),
    (r"\[op 1\] Repelled", "Houdan has reverted, but Le Port holds. It rests for thirty minutes and its heat rises.", None, 6),
    (r"lever: Outpost reserve 0", "Another cell takes the Outpost, then leaves it empty.", None, 6),
    (r"Push-out: Base retake", "A token returns. With no hot target, a strong occupier pushes out: six men head for the quiet Outpost.", None, 30),
    (r"lever: Airfield reserve 2", "Scripted: a raid leaves the Airfield two men in reserve, and an ambush makes Le Port urgent.", r"lever: Le Port heat", 3),
    (r"\[op \d+\] Motorized squad", "Four reported fighters call for lethality fourteen point four. Urgent, it buys the fastest fit: a truck.", None, 3),
    (r"the player watches the road", "But the player watches the harbour road. No point behind the truck is unseen, so it cannot be born.", r"deferred: no point of its road", 3),
    (r"Southeast Compound passed over", "The next base back? The Compound is on another island, and the Airfield cannot spare the men.", None, 3),
    (r"re-planned as a Mi17 insertion", "So the operation is re-planned as a helicopter insertion.", None, 3),
    (r"insertion #1 Mi17 born airborne", "The Mi17 is born airborne, unseen, and lands four hundred metres out, away from the reported fighters.", r"born airborne at .* for Le Port", 12),
    (r"Mi17 down", "Four soldiers get out, the airframe goes home, and the cell repels them too.", r"Repelled at Le Port", 6),
    (r"lever: Airfield reserve 0", "A raid takes the Airfield. Control, sixty percent bases, forty percent towns, drops below three quarters.", r"Wave 1 \(\d+\) inbound", 4),
    (r"Wave 1 \(\d+\) inbound", "Wave one fires: thirty soldiers for the pool, landing at the harbour.", None, 6),
    (r"Wave 1: strikes on the cluster", "It strikes the densest cluster: the player's headquarters. Six men march on the camp, no token needed.", r"Wave 1 strike: HQ assault", 6),
    (r"\[op 3\] arrived abstract at Outpost", "The push-out reaches the Outpost and retakes it.", r"\[op 3\] Outpost retaken", 8),
    (r"no operation on Airfield", "The director wants the Airfield back, but no base can spare the men.", None, 40),
    (r"stand at the undefended HQ", "The assault finds the headquarters empty. Only a deliberate HQ assault fills this meter.", None, 6),
    (r"OccupierForces: HQ at .* destroyed", "At one hundred the headquarters falls, cache and garage with it.", None, 4),
]


def main(src, dst):
    trace = json.load(open(src))
    info = sorted(trace["info"], key=lambda e: e["sim"])
    cues = list(CARDS)
    last = -1.0
    words = 0
    for pat, text, linepat, rate in SPEC:
        hit = None
        for e in info:
            if e["sim"] >= last and re.search(pat, e["text"]):
                hit = e
                break
        if hit is None:
            print("MISSING:", pat)
            continue
        line = hit["text"] if hit["kind"] == "info" else ""
        if linepat:
            for e in info:
                if e["sim"] >= hit["sim"] - 1 and e["kind"] == "info" and re.search(linepat, e["text"]):
                    line = e["text"]
                    break
        if hit["kind"] == "note" and not line:
            line = "scenario note: " + hit["text"]
        cues.append({"sim": hit["sim"], "text": text, "line": line, "rate": rate})
        last = hit["sim"]
        words += len(text.split())
        print("%7.0f  %s" % (hit["sim"], text[:70]))
    for c in cues:
        for k in ("text", "line"):
            if chr(0x2014) in c.get(k, ""):
                raise SystemExit("em dash in cue")
    print("spoken words:", words)
    json.dump(cues, open(dst, "w"), indent=1)


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
