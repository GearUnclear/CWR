import os

os.chdir(r"D:\Arma_CWA_worktrees\look-light\arma_CWR")


def sub(p, a, b):
    s = open(p, encoding="utf-8").read()
    assert a in s, (p, a)
    s = s.replace(a, b)
    open(p, "w", encoding="utf-8", newline="").write(s)


g = "tests/unit/engine/Poseidon/Graphics/"
sub(g + "test_remaster_lighting.cpp", '        {"litCamera", LC::Camera, 1},',
    '        {"litCamera", LC::Camera, 1},\n        {"litCameraLook", LC::CameraLook, 1},')
sub(g + "test_remaster_lighting.cpp", "CHECK(static_cast<int>(LC::Count) == static_cast<int>(LC::Camera) + 1);",
    "CHECK(static_cast<int>(LC::Count) == static_cast<int>(LC::CameraLook) + 1);")
sub(g + "test_feature_switches.cpp", "constexpr int kPhotoLightKnobs = 16;", "constexpr int kPhotoLightKnobs = 18;")
sub(g + "test_air_medium.cpp",
    "shaft = max(shaft - airBeams.y * local, floorAt) + boost * max(shaft - local, vec2(0.0));",
    "shaft = max(shaft - airBeams.y * local, floorAt) + boost2 * max(shaft - local, vec2(0.0));")
sub(g + "test_air_medium.cpp",
    "glow = airSunColor.rgb * (shaft.x * air.a) + airSpotTint.rgb * shaft.y;",
    "glow = airSunColor.rgb * (shaft.x * air.a * camBeams) + airSpotTint.rgb * shaft.y;")
sub(g + "test_photo_camera.cpp", '"camScotopic", "camAdditive"})', '"camScotopic", "camAdditive", "camAerial", "camLocal"})')
sub("engine/Poseidon/Graphics/Remaster/FeatureSwitches.cpp",
    '"camNight", "Night exposure", "EV at full night", &LT::camNight, -3.0f,',
    '"camNight", "Night exposure", "EV at full night", &LT::camNight, -5.0f,')
print("ok")
