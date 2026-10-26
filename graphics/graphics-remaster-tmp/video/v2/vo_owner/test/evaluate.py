"""Compare owner_vo.py split's takes.json with the stand-in's truth.

    python evaluate.py <truth json> <takes.json>

A truth take counts as recovered when a found take of the same line and wording covers it
(speech onset..offset overlapping the truth by 80% of their union or more). Also checks that no
cut (with its padding) reaches into another take, a number or a cough, that the fragment is
reported as a fragment, the restart is repaired and the clipped take is flagged.
"""
import json
import sys


def iou(a0, a1, b0, b1):
    inter = max(0.0, min(a1, b1) - max(a0, b0))
    union = max(a1, b1) - min(a0, b0)
    return inter / union if union > 0 else 0.0


def main(truth_path, takes_path):
    truth = json.load(open(truth_path, encoding="utf-8"))
    rep = json.load(open(takes_path, encoding="utf-8"))
    found = rep["takes"]
    bad = 0
    used = set()
    print(f"{'truth':34s} {'found':8s} {'iou':>5s}  notes")
    for t in truth:
        if t["kind"] not in ("take", "fragment"):
            continue
        cands = [(iou(t["start"], t["end"], f["onset"], f["offset"]), k, f) for k, f in enumerate(found)
                 if f["id"] == t["line"]]
        cands.sort(key=lambda c: -c[0])
        name = f"{t['line']} {t['kind']}{' ' + str(t.get('take', '')) if t['kind'] == 'take' else ''}" \
               f"{' (' + t['wording'] + ')' if t.get('wording') == 'shorter' else ''}"
        if not cands or cands[0][0] < (0.6 if t["kind"] == "fragment" else 0.8):
            bad += 1
            print(f"{name:34s} {'MISSED':8s} {cands[0][0] if cands else 0:5.2f}")
            continue
        q, k, f = cands[0]
        used.add(k)
        notes = []
        ok = True
        if t["kind"] == "fragment" and f["kind"] != "fragment":
            ok = False
            notes.append(f"reported as {f['kind']}, not a fragment")
        if t["kind"] == "take" and f["kind"] == "fragment":
            ok = False
            notes.append("reported as a fragment")
        if t.get("wording") and t["wording"] != f["wording"]:
            ok = False
            notes.append(f"wording {f['wording']} (truth {t['wording']})")
        if t.get("restart") and not f["splices"]:
            ok = False
            notes.append("restart not repaired")
        if t.get("restart") and f["splices"]:
            notes.append(f"restart repaired ({f['splices']})")
        if f.get("clipped"):
            notes.append("CLIPPED flagged")
        # the padded cut must not reach into any other truth event
        for o in truth:
            if o is t or o["kind"] not in ("take", "fragment", "number", "cough"):
                continue
            if f["start"] < o["end"] - 0.02 and f["end"] > o["start"] + 0.02:
                inside_splice = any(s0 <= o["start"] and s1 >= o["end"] - 0.05 for s0, s1 in f["splices"])
                if not inside_splice:
                    ok = False
                    notes.append(f"cut reaches into a {o['kind']} at {o['start']:.2f}")
        bad += 0 if ok else 1
        print(f"{name:34s} {f['label']:8s} {q:5.2f}  {'OK' if ok else 'WRONG'} {'; '.join(notes)}")
    extra = [f for k, f in enumerate(found) if k not in used]
    for f in extra:
        print(f"EXTRA found take {f['label']} {f['kind']} at {f['onset']:.2f}-{f['offset']:.2f}: {f['heard'][:60]}")
    nums = [t for t in truth if t["kind"] == "number"]
    if nums:
        print(f"numbers in truth: {len(nums)}; found: {len(rep['numbers'])}: "
              + ", ".join(f"{n['said']}->{n['line']}" for n in rep["numbers"]))
    print(f"{'ALL RECOVERED' if not bad else str(bad) + ' PROBLEM(S)'}; {len(extra)} extra")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1], sys.argv[2]))
