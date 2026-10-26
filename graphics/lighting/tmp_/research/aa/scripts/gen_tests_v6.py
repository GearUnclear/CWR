"""v6 for track aa: the cheaper default candidate M = 4x MSAA (alpha-to-coverage)
at 100% render scale, against D (shipped: no AA) and H (capture: 4x MSAA, 1.5x),
all in one session, stills and sub-pixel pans. forest_edge_golden and town_night.

Variants (selector knobs as gen_tests.py): rem = k 0; best = j3 coverage +
present p2 (Lanczos2, identity at 1.0x, + RCAS 0.5); fxaa = composite FXAA.
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import gen_tests as g  # noqa: E402
import gen_tests_v4 as v4  # noqa: E402

OUT = Path(sys.argv[1]) if len(sys.argv) > 1 else v4.OUT

SETTING_LIB = '''
// "D" | "M" | "H" call aaSetting3
aaSetting3 = {
    if (_this == "H") then { aaM = triSetMsaa 4; aaS = triSetRenderScale 1.5 };
    if (_this == "M") then { aaM = triSetMsaa 4; aaS = triSetRenderScale 1.0 };
    if (_this == "D") then { aaM = triSetMsaa 0; aaS = triSetRenderScale 1.0 };
    format ["view=%1;event=setting;setting=%2;msaa=%3;scale=%4", capView, _this, aaM, aaS] call capLog;
    true
}
'''

PLAN = {
    "D": ([("D-classic", "classic", 0, 0), ("D-rem", "remastered", 0, 0)],
          [("panD-rem", "remastered", 0, 0)]),
    "M": ([("M-classic", "classic", 0, 0), ("M-rem", "remastered", 0, 0), ("M-best", "remastered", 3, 20),
           ("M-fxaa", "remastered", 0, 1)],
          [("panM-rem", "remastered", 0, 0), ("panM-best", "remastered", 3, 20)]),
    "H": ([("H-classic", "classic", 0, 0), ("H-rem", "remastered", 0, 0), ("H-best", "remastered", 3, 20)],
          [("panH-rem", "remastered", 0, 0), ("panH-best", "remastered", 3, 20)]),
}


def block(view, begin, props, extra_pin=None, pans=True):
    s = f'''
// ----------------------------------------------------------------------------
//  {view}
// ----------------------------------------------------------------------------
{begin} call capBegin
call {{ capH0 = capH; {props} true }}
aaExtraPin = {{ {extra_pin + ';' if extra_pin else ''} true }}
triSimFrames 150
'''
    for setting, (stills, pan_list) in PLAN.items():
        s += f'call {{ "{setting}" call aaSetting3 }}\ntriSimFrames 30\n'
        for lab, mode, j, k in stills:
            s += g.shot(view, lab, mode, j, k)
        if pans:
            for lab, mode, j, k in pan_list:
                s += g.pan(view, lab, mode, j, k)
    s += 'call { "H" call aaSetting3 }\ntriSimFrames 30\n[] call capClear\n'
    return s


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    body = SETTING_LIB
    body += block("forest_edge_golden", *v4.FOREST)
    body += block("town_night",
                  '["town_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800]',
                  'capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn; if (not (isNull capFire)) then { capFire inflame true };',
                  extra_pin='if (not (isNull capFire)) then { capFire inflame true }')
    sqf = g.header("Abel", 4674, 10156) + body + g.footer()
    (OUT / "aa_msaa_m.test.sqf").write_text(sqf, encoding="utf-8", newline="\n")
    win = '"--display-mode", "windowed", "--width", "960", "--height", "640"'
    (OUT / "aa_msaa_m.test.toml").write_text(
        g.toml("tests/rendering/captures/missions/udcap_abel.Abel", 1500, f'["--no-strict", {win}]'),
        encoding="utf-8", newline="\n")
    print("wrote", OUT)


if __name__ == "__main__":
    main()
