from pathlib import Path
T = Path(r"D:\Arma_CWA_worktrees\gr-B8\arma_CWR\tests\unit\engine\Poseidon\Graphics")
def rep(path, old, new):
    s = path.read_text(encoding="utf-8")
    assert s.count(old) == 1, (path.name, old[:70], s.count(old))
    path.write_text(s.replace(old, new), encoding="utf-8", newline="\n")
a = T / "test_air_medium.cpp"
rep(a, """TEST_CASE("Air medium: morning mist with an eastern sun, gone by 15 degrees", "[remaster][AirMedium]")
{
    // Full from sunrise to 5 degrees (Abel's valley at 05:15 in July, the
    // ridge at 05:08), half way at 10, gone by 15 (06:30 in July, 08:00 in
    // March): a dawn feature.
    CHECK(MorningWeight(2.0f, 0.9f) == Catch::Approx(1.0f));
    CHECK(MorningWeight(5.0f, 0.6f) == Catch::Approx(1.0f));
    CHECK(MorningWeight(10.0f, 0.9f) == Catch::Approx(0.5f));
    CHECK(MorningWeight(12.0f, 0.9f) > 0.0f);
    CHECK(MorningWeight(12.0f, 0.9f) < 0.25f);
    CHECK(MorningWeight(15.0f, 0.9f) == 0.0f);
    CHECK(MorningWeight(-5.0f, 0.9f) == 0.0f);  // still night
    CHECK(MorningWeight(10.0f, -0.6f) == 0.0f); // evening: a western sun
""", """TEST_CASE("Air medium: morning mist with an eastern sun, gone by 10 degrees", "[remaster][AirMedium]")
{
    // Full from sunrise to 6 degrees (the ridge at 05:08 in July has the sun
    // at 5.1 degrees, Abel's valley at 05:15 at 6.3), half way at 8, gone by
    // 10 (about an hour after sunrise): a dawn feature. lowsun_fp (08:00 in
    // March) has the sun at 12.7 degrees in game: no mist there.
    CHECK(MorningWeight(2.0f, 0.9f) == Catch::Approx(1.0f));
    CHECK(MorningWeight(5.1f, 0.6f) == Catch::Approx(1.0f));
    CHECK(MorningWeight(6.3f, 0.9f) > 0.95f);
    CHECK(MorningWeight(8.0f, 0.9f) == Catch::Approx(0.5f));
    CHECK(MorningWeight(9.5f, 0.9f) > 0.0f);
    CHECK(MorningWeight(9.5f, 0.9f) < 0.05f);
    CHECK(MorningWeight(10.0f, 0.9f) == 0.0f);
    CHECK(MorningWeight(12.7f, 0.9f) == 0.0f); // lowsun_fp
    CHECK(MorningWeight(-5.0f, 0.9f) == 0.0f); // still night
    CHECK(MorningWeight(8.0f, -0.6f) == 0.0f); // evening: a western sun
""")
rep(a, """    // A clear morning an hour or two after sunrise (lowsun_fp, 08:00 in March,
    // 15 degrees): the mist is gone and the beams stay clear.
    in.skyThrough = 1.0f;
    in.sunElevationDeg = 15.0f;""", """    // A clear morning two hours after sunrise (lowsun_fp, 08:00 in March: the
    // sun at 12.7 degrees in game): the mist is gone and the beams stay clear.
    in.skyThrough = 1.0f;
    in.sunElevationDeg = 12.7f;""")
f = T / "test_feature_switches.cpp"
rep(f, """    in.sunAzimuthDeg = 90.0f; // the same sun in the morning
    CHECK(G(in, Feature::ValleyMist).on);
    CHECK(G(in, Feature::AirBeams).on);
    CHECK_FALSE(G(in, Feature::AirBeams, Apply("remastered,valleyMist=0")).on);
    in.sunElevationDeg = 40.0f; // mist burned off
    CHECK_FALSE(G(in, Feature::ValleyMist).on);""", """    in.sunAzimuthDeg = 90.0f; // a morning sun, an hour after sunrise
    in.sunElevationDeg = 8.0f;
    CHECK(G(in, Feature::ValleyMist).on);
    CHECK(G(in, Feature::AirBeams).on);
    CHECK_FALSE(G(in, Feature::AirBeams, Apply("remastered,valleyMist=0")).on);
    in.sunElevationDeg = 12.7f; // lowsun_fp, 08:00 in March: the mist burned off
    CHECK_FALSE(G(in, Feature::ValleyMist).on);""")
print("ok")
