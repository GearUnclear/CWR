"""v9 research test for the ssgi track: override_v9 (v8 trace; hue albedo now
keeps the texture's light/dark detail). headlights_bounce waits 1200 frames
before its first shot so the far town's lamps (auto mode, 12 s simulation
step) are on for every shot (in v7/v8 they switched on between shots).
A gi=0 Remastered shot is repeated at the end of each view (rem_end).

python gen_tests_v9.py <out dir>
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import gen_tests as g  # noqa: E402
import gen_tests_v7 as v7  # noqa: E402

BASE = v7.BASE
CH = ["giFallbackShare=2", "giDebug=0"] + BASE
NIGHT = [
    ("gi_m1s1_chroma", ["giMode=1", "gi=1"] + CH),
    ("gi_m1s2_chroma", ["giMode=1", "gi=2"] + CH),
    ("gi_m1s1_chroma_dbg", ["giMode=1", "gi=1", "giFallbackShare=2", "giDebug=1"] + BASE),
    ("rem_end", g.GI_OFF),
]
DAY = [
    ("gi_m1s1_chroma", ["giMode=1", "gi=1"] + CH),
    ("gi_m1s2_chroma", ["giMode=1", "gi=2"] + CH),
    ("gi_m6s1_chroma", ["giMode=6", "gi=1"] + CH),
    ("rem_end", g.GI_OFF),
]
# Both switchLight tries (every lamp within 3 km ON, then within 1 km OFF)
# crashed the game in StreetLamp::SimulateSwitch -> EntityHit::GetHit (access
# violation; runs/v9_crash, runs/v9_crash2): some map lamp near the spot has no
# valid bulb hit. So instead the view waits 1200 frames (well past the lamps'
# 12 s simulation step) before its first shot.
HL = v7.VIEWS["headlights_bounce"][0] + "triSimFrames 1200\n"
VIEWS = dict(v7.VIEWS)
VIEWS["headlights_bounce"] = (HL, False)
PLAN = [("headlights_bounce", NIGHT), ("town_dark_fire", NIGHT), ("lowsun_fp", DAY), ("noon_fields", DAY)]


def main():
    out_dir = Path(sys.argv[1])
    body = g.HEADER.format(world="Abel", px=7300, pz=10900).replace("gen_tests.py", "gen_tests_v9.py")
    body = body.replace("capFire = objNull\n", "capFire = objNull\ncapJeep = objNull\ncapDrv = objNull\n")
    for v, variants in PLAN:
        setup, fire = VIEWS[v]
        body += f"\n// ---- {v}\n" + setup + "triSimFrames 150\n"
        body += g.shot(v, "classic", "classic", g.GI_OFF, fire)
        body += g.shot(v, "remastered", "remastered", g.GI_OFF, fire)
        for label, knobs in variants:
            body += g.shot(v, label, "remastered", knobs, fire)
        body += '"gi=0" call capKnob\n"giFallbackShare=0.5" call capKnob\n'
        body += "call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull } }\n"
        body += "[] call capClear\n"
    body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "abel_ssgi_v9.test.sqf").write_text(body, encoding="utf-8", newline="\n")
    toml = (
        'binary = "PoseidonGame"\n'
        'mission = "tests/rendering/captures/missions/udcap_abel.Abel"\n'
        'tags = ["lighting_capture", "headful", "exclusive", "research"]\n'
        "timeout = 900\nassert_timeout = 180\n"
        'extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]\n'
    )
    (out_dir / "abel_ssgi_v9.test.toml").write_text(toml, encoding="utf-8", newline="\n")
    print("wrote", out_dir / "abel_ssgi_v9.test.sqf")


if __name__ == "__main__":
    main()
