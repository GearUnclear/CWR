import os
os.chdir(r'D:\Arma_CWA_worktrees\look-fog\arma_CWR')


def edit(p, pairs):
    s = open(p, encoding='utf-8', newline='').read()
    for a, b in pairs:
        assert s.count(a) == 1, (p, a[:80], s.count(a))
        s = s.replace(a, b)
    open(p, 'w', encoding='utf-8', newline='').write(s)


sh = 'engine/PoseidonGL33/EngineGL33_PostFogShaders.cpp'
edit(sh, [
    ('''    float nearRamp = smoothstep(kMistNearFrom, kMistNearTo, length(rel));
    float nearKeep = mix(kMistNearKeep, 1.0, nearRamp);
    float hazeKeep = mix(kHazeNearKeep, 1.0, nearRamp);''',
     '''    float dist = length(rel);
    float nearKeep = mix(kMistNearKeep, 1.0, smoothstep(kMistNearFrom, kMistNearTo, dist));
    float hazeKeep = mix(kHazeNearKeep, 1.0, smoothstep(kHazeNearFrom, kHazeNearTo, dist));'''),
    ('''const float kHazeNearKeep = 0.4;''', '''const float kHazeNearKeep = 0.25;
const float kHazeNearFrom = 30.0;
const float kHazeNearTo = 900.0;'''),
    ('''    float sGround = fogGround.x * exp(-hAbove / max(fogGround.y, 1.0) - overFloor / 150.0) * nearKeep;''',
     '''    float sGround = fogGround.x * exp(-hAbove / max(fogGround.y, 1.0) - overFloor / 80.0) * nearKeep;'''),
])
edit('engine/Poseidon/Graphics/Remaster/VolumetricFog.hpp', [
    ('''constexpr float kHazeClear = 1.5e-4f;''', '''constexpr float kHazeClear = 2.8e-4f;'''),
    ('''constexpr float kHazeNearKeep = 0.4f; // the clear haze and air near the eye''',
     '''// The clear haze and air keep this share near the eye, the whole from
// kHazeNearFrom to kHazeNearTo away: the depth is in the distance, the eye's
// own surroundings stay crisp.
constexpr float kHazeNearKeep = 0.25f;
constexpr float kHazeNearFrom = 30.0f;
constexpr float kHazeNearTo = 900.0f;'''),
    ('''constexpr float kGroundHazePool = 150.0f;   // and over the fog floor: it pools in the low ground''',
     '''constexpr float kGroundHazePool = 80.0f;    // and over the fog floor: it pools in the low ground'''),
])
p = 'tests/unit/engine/Poseidon/Graphics/test_volumetric_fog.cpp'
edit(p, [
    ('''    CHECK(noonLeft > 0.4f);
    CHECK(noonLeft < 0.75f);''', '''    CHECK(noonLeft > 0.25f);
    CHECK(noonLeft < 0.6f);'''),
    ('''    // The eye's near thinning in the shaders is the C++ one.''', '''    // The eye's near thinning and the ground air's pooling in the shaders are
    // the C++ ones.
    CHECK(CountOf(inject, "const float kHazeNearKeep = 0.25;") == 1);
    CHECK(kHazeNearKeep == 0.25f);
    CHECK(CountOf(inject, "overFloor / 80.0") == 1);
    CHECK(kGroundHazePool == 80.0f);'''),
])
print('ok')
