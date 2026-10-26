"""v8 research test for the ssgi track: override_v8 (hit-normal test, capped
crossing tolerance and mip LOD, receiver distance fade) on the v7 night views,
noon_fields, and a short strafe on headlights_bounce for the temporal path.

python gen_tests_v8.py <out dir>
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import gen_tests as g  # noqa: E402
import gen_tests_v7 as v7  # noqa: E402

BASE = v7.BASE
NIGHT = [
    ("gi_m1s1_chroma", ["giMode=1", "gi=1", "giFallbackShare=2", "giDebug=0"] + BASE),
    ("gi_m1s2_chroma", ["giMode=1", "gi=2", "giFallbackShare=2", "giDebug=0"] + BASE),
    ("gi_m1s1_share1", ["giMode=1", "gi=1", "giFallbackShare=1", "giDebug=0"] + BASE),
    ("gi_m1s1_chroma_dbg", ["giMode=1", "gi=1", "giFallbackShare=2", "giDebug=1"] + BASE),
    ("gi_m1s1_chroma_notemporal", ["giMode=1", "gi=1", "giFallbackShare=2", "giDebug=0", "giRadius=1.5", "giRays=6",
                                   "giSteps=20", "giAlbedoCap=0.8", "giTemporal=0", "bounce=1"]),
]
DAY = [
    ("gi_m1s1_fb1", ["giMode=1", "gi=1", "giFallbackShare=1", "giDebug=0"] + BASE),
    ("gi_m6s1_fb1", ["giMode=6", "gi=1", "giFallbackShare=1", "giDebug=0"] + BASE),
    ("gi_m1s1_chroma", ["giMode=1", "gi=1", "giFallbackShare=2", "giDebug=0"] + BASE),
]
STRAFE = ("strafe_m1s1_chroma", ["giMode=1", "gi=1", "giFallbackShare=2", "giDebug=0"] + BASE)
PLAN = [("headlights_bounce", NIGHT, True), ("town_dark_fire", NIGHT[:2] + NIGHT[3:4], False), ("noon_fields", DAY, False)]


def main():
    out_dir = Path(sys.argv[1])
    body = g.HEADER.format(world="Abel", px=7300, pz=10900).replace("gen_tests.py", "gen_tests_v8.py")
    body = body.replace("capFire = objNull\n", "capFire = objNull\ncapJeep = objNull\ncapDrv = objNull\n")
    for v, variants, strafe in PLAN:
        setup, fire = v7.VIEWS[v]
        body += f"\n// ---- {v}\n" + setup + "triSimFrames 150\n"
        body += g.shot(v, "classic", "classic", g.GI_OFF, fire)
        body += g.shot(v, "remastered", "remastered", g.GI_OFF, fire)
        for label, knobs in variants:
            body += g.shot(v, label, "remastered", knobs, fire)
        if strafe:
            # Camera strafes 0.25 m right per frame for 8 frames, GI temporal on;
            # the remastered (gi=0) strafe follows for the same frames.
            for label, knobs in (STRAFE, ("strafe_rem", g.GI_OFF)):
                body += g.shot(v, label + "0", "remastered", knobs, fire)
                body += "capOX0 = capOX\ncapOZ0 = capOZ\n"
                for i in range(1, 9):
                    body += "call { capOX = capOX + 0.25 * (cos capH); capOZ = capOZ - 0.25 * (sin capH); [] call capPinView }\n"
                    body += "triSimFrames 1\n"
                    body += f'triScreenshot "{v}__{label}{i}"\n'
                    body += f'"{v}__{label}{i}" call capRecord\n'
                body += "capOX = capOX0\ncapOZ = capOZ0\n"
        body += '"gi=0" call capKnob\n"giFallbackShare=0.5" call capKnob\n'
        body += "call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull } }\n"
        body += "[] call capClear\n"
    body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "abel_ssgi_v8.test.sqf").write_text(body, encoding="utf-8", newline="\n")
    toml = (
        'binary = "PoseidonGame"\n'
        'mission = "tests/rendering/captures/missions/udcap_abel.Abel"\n'
        'tags = ["lighting_capture", "headful", "exclusive", "research"]\n'
        "timeout = 900\nassert_timeout = 180\n"
        'extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]\n'
    )
    (out_dir / "abel_ssgi_v8.test.toml").write_text(toml, encoding="utf-8", newline="\n")
    print("wrote", out_dir / "abel_ssgi_v8.test.sqf")


if __name__ == "__main__":
    main()
