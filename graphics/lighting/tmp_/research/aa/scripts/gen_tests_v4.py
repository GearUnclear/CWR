"""Resume step (v4) for track aa: present-pass cost and FXAA-only pans.

Writes into <worktree>/arma_CWR/tests/rendering/research/aa/ (git-excluded):
  aa_cost960.test.*   forest_edge_golden at 960x640: present variants at H twice
                      (ABAB, for the timer spread), D stills, D pans rem vs FXAA
  aa_cost1080.test.*  the same H present variants at a 1920x1080 window (cost)
  aa_fxaa_eden.test.* overcast_broken: D pans rem vs FXAA (far forest shimmer)

Selector knobs as in gen_tests.py: ao = 0.7 + 0.001*k, k % 10 = composite
variant (1 = FXAA), k / 10 = present variant (0 bilinear shader, 1 Lanczos2 2D,
2 Lanczos2 2D + RCAS 0.5, 6 separable Lanczos2 + RCAS 0.5, 7 separable Lanczos2).
The scratch build appends "| present X.XXX" (ms) to the triGpuTimes summary.
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import gen_tests as g  # noqa: E402

OUT = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(
    r"D:\Arma_CWA_worktrees\gfx-aa\arma_CWR\tests\rendering\research\aa")

COST_H = [
    ("classic", "classic", 0, 0),
    ("p0", "remastered", 0, 0),
    ("lz", "remastered", 0, 10),
    ("lzrcas05", "remastered", 0, 20),
    ("seplz", "remastered", 0, 70),
    ("seplzrcas05", "remastered", 0, 60),
]
STILLS_D = [
    ("D-classic", "classic", 0, 0),
    ("D-rem", "remastered", 0, 0),
    ("D-fxaa", "remastered", 0, 1),
    ("D-fxaarcas05", "remastered", 0, 21),
]
PANS_D = [("panD-rem", "remastered", 0, 0), ("panD-fxaa", "remastered", 0, 1), ("panD-cov3fxaa", "remastered", 3, 1)]

FOREST = ('["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500]',
          '[["SoldierWB"], 12, 2, 215, "man"] call capSpawn;')
EDEN = ('["overcast_broken", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500]',
        '[["Jeep"], 12, -5, 70, "veh"] call capSpawn;')


def block(view, begin, props, h_rounds, stills_d, pans_d):
    s = f'''
// ----------------------------------------------------------------------------
//  {view}
// ----------------------------------------------------------------------------
{begin} call capBegin
call {{ capH0 = capH; {props} true }}
aaExtraPin = {{ true }}
triSimFrames 150
'''
    if h_rounds:
        s += 'call { "H" call aaSetting }\ntriSimFrames 30\n'
        for rnd in h_rounds:
            for lab, mode, j, k in COST_H:
                s += g.shot(view, f"H-{lab}-{rnd}", mode, j, k)
    if stills_d or pans_d:
        s += 'call { "D" call aaSetting }\ntriSimFrames 30\n'
        for lab, mode, j, k in stills_d:
            s += g.shot(view, lab, mode, j, k)
        for lab, mode, j, k in pans_d:
            s += g.pan(view, lab, mode, j, k)
    s += 'call { "H" call aaSetting }\ntriSimFrames 30\n[] call capClear\n'
    return s


def write(name, world, ox, oz, body, mission, mods, w, h, timeout):
    sqf = g.header(world, ox, oz) + body + g.footer()
    (OUT / f"{name}.test.sqf").write_text(sqf, encoding="utf-8", newline="\n")
    win = f'"--display-mode", "windowed", "--width", "{w}", "--height", "{h}"'
    extra = f'["--no-strict", {mods}{win}]'
    (OUT / f"{name}.test.toml").write_text(g.toml(mission, timeout, extra), encoding="utf-8", newline="\n")


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    abel_mission = "tests/rendering/captures/missions/udcap_abel.Abel"
    eden_mission = "tests/rendering/captures/missions/udcap_eden.Eden"
    write("aa_cost960", "Abel", 4674, 10156,
          block("forest_edge_golden", *FOREST, ["a", "b"], STILLS_D, PANS_D),
          abel_mission, "", 960, 640, 1500)
    write("aa_cost1080", "Abel", 4674, 10156,
          block("forest_edge_golden", *FOREST, ["a", "b"], [], []),
          abel_mission, "", 1920, 1080, 1500)
    write("aa_fxaa_eden", "Eden", 9200, 3700,
          block("overcast_broken", *EDEN, [], [("D-rem", "remastered", 0, 0), ("D-fxaa", "remastered", 0, 1)],
                PANS_D),
          eden_mission, "", 960, 640, 1200)
    print("wrote", OUT)


if __name__ == "__main__":
    main()
