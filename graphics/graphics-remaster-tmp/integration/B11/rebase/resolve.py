"""Resolve the conflicts of B11.patch applied with --3way on 5d7e7ae (A7 landed).

Each conflicted file gets one resolution per hunk, in order: "both" (ours, the
A7 tip, then theirs, B11), or explicit text. Run from anywhere.
"""
import re
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gr-B11-rebase\arma_CWR")
HUNK = re.compile(r"<<<<<<< ours\n(.*?)=======\n(.*?)>>>>>>> theirs\n", re.S)


def both(o, t):
    return o + t


def ours(o, t):
    return o


def text(s):
    return lambda o, t: s


def theirs(o, t):
    return t


R = {
    "engine/Poseidon/Graphics/Remaster/FeatureSwitches.cpp": [both],
    "engine/Poseidon/Graphics/Remaster/LightRig.hpp": [both],
    "engine/Poseidon/Graphics/Remaster/README.md": [both],
    "tests/unit/engine/Poseidon/CMakeLists.txt": [both],
    "engine/PoseidonGL33/EngineGL33_PostShaders.cpp": [both],
    "engine/PoseidonGL33/EngineGL33_Lighting.cpp": [both, lambda o, t: o + "}\n\n" + t],
    "engine/PoseidonGL33/EngineGL33_Lighting.hpp": [
        both,
        text("""    // 58-60: allocated to features still to land (LitConstPlanned MatClass,
    // Taa, Pcss); one padding member in the block until they do.
    Reserved = 58,
    // The sky pass (the sky table; clouds read the camera basis too).
    SkyRight = 61,     // xyz camera right * tan(fov x / 2), w Mie asymmetry g
    SkyUp = 62,        // xyz camera up * tan(fov y / 2)
    SkyFwd = 63,       // xyz camera forward, w sky mode of the draws that follow (RemasterSky; 0 = world)
    Atmo = 64,         // x dome blend, y overcast cloud floor, z aerial perspective blend, w cloud mask dead zone
    AtmoCloudSun = 65, // rgb sun light on the sky clouds (linear), w sky light on them (linear; the table's colour)
    AtmoGlow = 66,     // rgb classic dome sun glow (display, SunSkyColor), w sky saturation
    AtmoDome = 67,     // rgb classic dome light (display, SkyColor), w hue (0 the original hue relit, 1 physical)
    // 68-76: allocated to features still to land (LitConstPlanned Sky to
    // Canopy); one padding member in the block until they do.
    Planned = 68,
    Detail = 77, // terrain detail (B11): x self-detail strength, y detail kept, z anti-tiling, w land draw
    Count = 78,
};
constexpr int MaxLights = 8;
constexpr int ReservedCount = 3;
constexpr int PlannedCount = Detail - Planned;
"""),
        text("""static_assert(Reserved == LightDir + MaxLights, "LitConst reserved slots follow the lights");
static_assert(SkyRight == Reserved + ReservedCount, "LitConst sky slots follow the reserved ones");
static_assert(Planned == AtmoDome + 1, "the planned padding follows the sky slots");
static_assert(Count == Detail + 1, "LitConst size out of sync");
"""),
        text("""// 77: texsharp's terrain detail landed (LitConst::Detail, B11).
constexpr int Haze = 78; // haze: the analytic fallback for blended surfaces
constexpr int End = 79;  // one past the last planned slot
static_assert(MatClass == LitConst::Reserved && Pcss == LitConst::Reserved + LitConst::ReservedCount - 1,
              "the planned slots below the sky pass are the reserved padding");
static_assert(Sky == LitConst::Planned && Canopy == LitConst::Detail - 1,
              "the planned slots between the sky pass and the terrain detail are the planned padding");
static_assert(Haze == LitConst::Count, "the other planned slots follow the declared ones");
"""),
    ],
    "engine/PoseidonGL33/EngineGL33_LightingShaders.cpp": [
        lambda o, t: o + """    vec4 litPlanned[9];   // 68-76: allocated to features still to land (clouds .. hfrt)
    vec4 litDetail;       // 77: terrain detail: x self-detail strength, y detail kept, z anti-tiling, w land draw
"""],
    "tests/unit/engine/Poseidon/Graphics/test_feature_switches.cpp": [
        text("""    // Every knob with a parent sits under a landed feature switch, in its
    // track's group: TAA's three, the sky table's three (A7, test_atmosphere.cpp)
    // and texsharp's two (B11).
"""),
        lambda o, t: o.replace('CHECK((parent == "taa" || parent == "atmosphere"));',
                               'CHECK((parent == "taa" || parent == "atmosphere" || parent == "terrainDetail" ||\n'
                               '               parent == "sharpen"));'),
        text('TEST_CASE("Feature switches: the frame-cost switches, TAA, the sky table and texsharp have landed, '
             'nothing else has",\n'),
        text("""    // shows the opaque objects' GPU section for them. A5 lands taa, A7
    // atmosphere, B11 terrainDetail and sharpen.
"""),
        text("""        const bool atmosphere = info.id == Feature::Atmosphere;
        const bool texsharp = info.id == Feature::TerrainDetail || info.id == Feature::Sharpen;
        CHECK(info.landed == (perf || taa || atmosphere || texsharp));
        if (info.landed)
        {
            CHECK(std::string_view(info.step) == (perf ? "A4" : taa ? "A5" : atmosphere ? "A7" : "B11"));
"""),
        text('    CHECK(s.find("landed=6/26") != std::string::npos);\n'),
    ],
    "tests/unit/engine/Poseidon/Graphics/test_remaster_lighting.cpp": [
        text("""    // and its debug view, the TAA resolve and its sharpen, the sky table's
    // two passes, and the luma sharpen
    CHECK(post == 14);
"""),
        lambda o, t: o + """        {"litPlanned", LC::Planned, LC::PlannedCount},
        {"litDetail", LC::Detail, 1},
""",
        lambda o, t: o.replace("    CHECK(gl::LitConstPlanned::Sky == static_cast<int>(LC::Count));\n",
                               "    CHECK(gl::LitConstPlanned::Sky == static_cast<int>(LC::Planned));\n"
                               "    // B11 landed the terrain detail at 77; 68 to 76 are the planned padding.\n"
                               "    CHECK(static_cast<int>(LC::Detail) == 77);\n"
                               "    CHECK(gl::LitConstPlanned::Canopy == static_cast<int>(LC::Detail) - 1);\n"
                               "    CHECK(gl::LitConstPlanned::Haze == static_cast<int>(LC::Count));\n"),
    ],
    "engine/Poseidon/Graphics/Remaster/research/INTEGRATION-PLAN.md": [
        lambda o, t: o + "\n" + t.replace(
            "`Planned` = 58 (`PlannedCount` = 19 slots, declared in the block as one padding array\n"
            "`vec4 litPlanned[19]`), `Detail` = 77 and `Count` = 78;",
            "`Planned` = 58 (`PlannedCount` = 19 slots, declared in the block as one padding array\n"
            "`vec4 litPlanned[19]`), `Detail` = 77 and `Count` = 78; on top of A7 the padding is 68 to 76\n"
            "(`Planned` = 68, `litPlanned[9]`, after A7's `litReserved[3]` and sky slots);"),
    ],
}


def main():
    for rel, fixes in R.items():
        p = ROOT / rel
        s = p.read_bytes().decode("utf-8")
        hunks = HUNK.findall(s)
        assert len(hunks) == len(fixes), (rel, len(hunks), len(fixes))
        i = iter(fixes)
        s = HUNK.sub(lambda m: next(i)(m.group(1), m.group(2)), s)
        assert "<<<<<<<" not in s and ">>>>>>>" not in s, rel
        p.write_bytes(s.encode("utf-8"))
        print("resolved", rel, len(hunks))


main()
