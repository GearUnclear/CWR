"""Generate the quiet-GPU timing tests (plan section 8 item 7; the A0 baseline).

    python gen_timing.py [--rounds 3] [--out <dir>]

Writes timing_abel, timing_eden and timing_sinai (.test.sqf + .test.toml) next to
this file. Every test runs at 1920x1080 windowed, render scale 1.0, 4x MSAA, the
presented frame, and visits its views (views.TIMING_*) in `rounds` interleaved
rounds (view order A B C .. A B C ..). Per view and round it shoots Classic then
Remastered (capMode, 90 frames, capReadTimers, capPin, 3 frames, triScreenshot,
capRecord), so make_pairs.py reads the output; the round is part of the view
label (<view>_r<k>). summarize_timing.py reduces the rounds; run-timing.ps1 runs
the set with the GPU clock sampler.

These are the views and the shot sequence of the A0 baseline
(tmp/integration/A0/timing/gen_timing.py), so a later run is comparable view for
view; each shot line now also carries feat= (triFeatureState), which shows the
gate states on the gate-off references (noon_fields, sea_noon).
"""
import argparse
from pathlib import Path

import capgen
import views

TESTS = {
    "timing_abel": ("Abel", views.TIMING_ABEL),
    "timing_eden": ("Eden", views.TIMING_EDEN),
    "timing_sinai": ("Sinai", views.TIMING_SINAI),
}


def view_block(v, rnd, first_round):
    label = f"{v['name']}_r{rnd}"
    out = [f"\n// ---- {v['name']} round {rnd} ({v['feature']})"]
    out += capgen.begin(v, label)
    out.append("triSimFrames 150")
    for mode in ("classic", "remastered"):
        out += capgen.shot(v, label, mode, f"{label}__{mode}")
    if first_round and v.get("f10"):
        # A view with trees and no spawned unit: the static casters are in the
        # shadow map from the start (F10). Wrapped in a call: a top-level FAIL
        # would abort the test.
        out.append(f'call {{ capDump = triShadowSceneDump "f10_{v["name"]}"; '
                   f'format ["view=%1;event=f10;dump=%2", capView, capDump] call capLog }}')
    out += capgen.end(v)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rounds", type=int, default=3)
    ap.add_argument("--out", default=str(capgen.HERE))
    a = ap.parse_args()
    out = Path(a.out)
    for name, (world, vs) in TESTS.items():
        title = f"Quiet-GPU timing on {world}: gen_timing.py, {a.rounds} interleaved rounds."
        lines = [capgen.header(world, vs[0], title)]
        for rnd in range(1, a.rounds + 1):
            for v in vs:
                lines += view_block(v, rnd, rnd == 1)
        lines.append('\ncall { "view=done;event=done" call capLog }')
        lines.append("triEndTest")
        capgen.write(out / f"{name}.test.sqf", "\n".join(lines) + "\n")
        capgen.write(out / f"{name}.test.toml", capgen.toml(world, title, views.WORLDS))
        print(f"{name}: {len(vs)} views x {a.rounds} rounds")


if __name__ == "__main__":
    main()
