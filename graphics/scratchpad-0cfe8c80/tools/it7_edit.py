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
    ('''    g_air = mix(g_air, ring.rgb, clamp(ring.a, 0.0, 1.0)) * fogLightK.y;''',
     '''    // (Not at night: the lamps' heads and flares above the roofs would read
    // as sky there; the night sky is the model's dark colour.)
    g_air = mix(g_air, ring.rgb, clamp(ring.a, 0.0, 1.0) * (1.0 - fogNightAir.w)) * fogLightK.y;'''),
    ('''    // The eye's own surroundings stay legible inside the mist and the humid air.
    float nearKeep = mix(kMistNearKeep, 1.0, smoothstep(kMistNearFrom, kMistNearTo, length(rel)));
    FogMediumCell c;
    c.haze = fogHaze.x * exp(-max(p.y, -50.0) / fogHaze.y);
    c.ray = kRayleighSea * (fogHaze.z * exp(-max(p.y, 0.0) / kRayleighHeight));''',
     '''    // The eye's own surroundings stay legible: inside the mist and the humid
    // air their density near the eye is a small share, and the clear haze
    // and air keep kHazeNearKeep of theirs there (the eye reads its own
    // surroundings in full contrast; the depth is in the distance).
    float nearRamp = smoothstep(kMistNearFrom, kMistNearTo, length(rel));
    float nearKeep = mix(kMistNearKeep, 1.0, nearRamp);
    float hazeKeep = mix(kHazeNearKeep, 1.0, nearRamp);
    FogMediumCell c;
    c.haze = fogHaze.x * exp(-max(p.y, -50.0) / fogHaze.y) * hazeKeep;
    c.ray = kRayleighSea * (fogHaze.z * exp(-max(p.y, 0.0) / kRayleighHeight) * hazeKeep);'''),
    ('''const float kMistNearKeep = 0.15;
const float kMistNearFrom = 20.0;
const float kMistNearTo = 300.0;''', '''const float kMistNearKeep = 0.15;
const float kHazeNearKeep = 0.4;
const float kMistNearFrom = 20.0;
const float kMistNearTo = 300.0;'''),
])

edit('engine/Poseidon/Graphics/Remaster/VolumetricFog.hpp', [
    ('''constexpr float kHazeClear = 1.8e-4f;''', '''constexpr float kHazeClear = 1.5e-4f;'''),
    ('''constexpr float kRayleighScale = 3.5f;''', '''constexpr float kRayleighScale = 2.5f;'''),
    ('''constexpr float kWeatherFogHeight = 450.0f; // setFog: thins upward from the fog floor''',
     '''constexpr float kWeatherFogHeight = 800.0f; // setFog: thins upward from the fog floor (the stars go with it)'''),
    ('''constexpr float kMistNearKeep = 0.15f;''', '''constexpr float kMistNearKeep = 0.15f;
constexpr float kHazeNearKeep = 0.4f; // the clear haze and air near the eye'''),
])
print('ok')
