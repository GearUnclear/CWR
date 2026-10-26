"""Motion strip test for the normals track (research only): does the relief swim?

Writes nrm_abel_seq.test.sqf/.toml next to the other research tests: the
lowsun_fp set, then for each of the given variants N frames with the eye
walking forward `step` metres per frame (props re-pinned, sim time pinned).

usage: python gen_seq.py [--variants cur:101.12,v9bold:91.12] [--frames 4] [--step 0.35]
"""
import argparse

import gen_tests as g

STEP = '''call {{ capOX = capOX + {step} * (sin capH); capOZ = capOZ + {step} * (cos capH) }}
triSimFrames 20
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "{label}"
"{label}" call capRecord
'''


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--variants", default="cur:101.12,v9bold:91.12")
    ap.add_argument("--frames", type=int, default=4)
    ap.add_argument("--step", type=float, default=0.35)
    a = ap.parse_args()
    world, begin, spawn = g.VIEWS["lowsun_fp"]
    body = g.HEADER.format(world=world, ox=7300, oz=10900)
    body += f"\n{begin} call capBegin\ncall {{{spawn}\n}}\ntriSimFrames 150\ncall {{ capOX0 = capOX; capOZ0 = capOZ }}\n"
    body += '"remastered" call capMode\n[] call capResetKnobs\n'
    for label, sat in (v.split(":") for v in a.variants.split(",")):
        body += f'["{label}", "saturation={sat}"] call capVariant\n'
        # back to the view origin for every variant (props and sim time stay pinned)
        body += "call { capOX = capOX0; capOZ = capOZ0 }\n"
        for f in range(a.frames):
            body += STEP.format(step=0.0 if f == 0 else a.step, label=f"lowsun_fp__seq_{label}_{f}")
    body += "[] call capResetKnobs\n"
    body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
    (g.OUT / "nrm_abel_seq.test.sqf").write_text(body, newline="\n")
    toml = (g.COMMITTED / "abel_lighting.test.toml").read_text()
    (g.OUT / "nrm_abel_seq.test.toml").write_text("# RESEARCH (normals track) motion strip\n" + toml, newline="\n")
    print("tests/rendering/research/normals/nrm_abel_seq.test.sqf")


if __name__ == "__main__":
    main()
