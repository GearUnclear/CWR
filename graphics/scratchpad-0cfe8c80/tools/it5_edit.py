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
    # ring: one row
    ('''// The horizon ring: the sky's own colour just above the land in every
// azimuth, measured from the resolved frame (before the fog) and kept for
// the azimuths out of view. Two rows of kFogRingBins texels, u = the azimuth
// from north (clockwise) over a full turn: row 0 the sky just above the
// silhouette (the lowest sky in that azimuth, the limit far ground melts
// into), row 1 the sky about 8 degrees up (the Rayleigh air's light); rgb
// linear (the world's shoulder undone), a how sure (1 measured this frame,
// decaying while out of view; 0 never seen: the model's colour then).''',
     '''// The horizon ring: the sky's own colour just above the land in every
// azimuth, measured from the resolved frame (before the fog) and kept for
// the azimuths out of view. One row of kFogRingBins texels, u = the azimuth
// from north (clockwise) over a full turn: the sky a little above the
// silhouette (the lowest sky in that azimuth, the limit far ground melts
// into); rgb linear, a how sure (1 measured this frame, decaying while out of
// view; 0 never seen: the model's colour then).'''),
    ('''    int row = int(gl_FragCoord.y);
    float az = (gl_FragCoord.x / ringCtl.x) * 6.2831853;''', '''    float az = (gl_FragCoord.x / ringCtl.x) * 6.2831853;'''),
    ('''        float base = row == 0 ? max(found, radians(2.0)) : max(found + radians(1.5), radians(8.0));''',
     '''        float base = max(found, radians(2.0));'''),
    ('''vec3 ringExpand(vec3 lin) {
    float peak = max(lin.r, max(lin.g, lin.b));
    if (peak <= 0.8) return lin;
    float hdr = 0.6 + 0.04 / (1.0 - min(peak, 0.995));
    return lin * (hdr / peak);
}

''', ''),
    # injection: one ring for both lights (the air's own chroma comes from
    # the Rayleigh coefficients)
    ('''    vec3 up = vec3(d.x, 0.25 * length(d.xz), d.z);
    vec3 dispUp = litAtmo.z > 0.0 ? atmoFogColor(up, fogColorToward(up)) : classic;
    g_air = mix(toLinear(disp), fogNightAir.rgb, fogNightAir.w);
    g_ray = mix(toLinear(dispUp), fogNightAir.rgb, fogNightAir.w);''', '''    g_air = mix(toLinear(disp), fogNightAir.rgb, fogNightAir.w);'''),
    ('''    vec4 ring0 = vec4(0.0), ring1 = vec4(0.0);
    for (int k = -2; k <= 2; ++k) {
        float w = 3.0 - abs(float(k));
        float u = az + float(k) * (3.0 / 256.0);
        vec4 a = texture(fogRing, vec2(u, 0.25));
        vec4 b = texture(fogRing, vec2(u, 0.75));
        ring0 += w * vec4(a.rgb * a.a, a.a);
        ring1 += w * vec4(b.rgb * b.a, b.a);
    }
    ring0 /= 9.0;
    ring1 /= 9.0;
    ring0.rgb /= max(ring0.a, 1e-4);
    ring1.rgb /= max(ring1.a, 1e-4);
    g_air = mix(g_air, ring0.rgb, clamp(ring0.a, 0.0, 1.0)) * fogLightK.y;
    g_ray = mix(g_ray, ring1.rgb, clamp(ring1.a, 0.0, 1.0)) * fogLightK.y;''', '''    vec4 ring = vec4(0.0);
    for (int k = -2; k <= 2; ++k) {
        float w = 3.0 - abs(float(k));
        vec4 a = texture(fogRing, vec2(az + float(k) * (3.0 / 256.0), 0.5));
        ring += w * vec4(a.rgb * a.a, a.a);
    }
    ring /= 9.0;
    ring.rgb /= max(ring.a, 1e-4);
    g_air = mix(g_air, ring.rgb, clamp(ring.a, 0.0, 1.0)) * fogLightK.y;
    // The air's own blue is in its Rayleigh coefficients: its light is the
    // same (the sky's colour would count the blue twice).
    g_ray = g_air;'''),
    # beams: visibility contrast
    ('''        float T = 1.0;
        float acc = 0.0;
        float sinSun = max(litSunDir.y, 0.03);''', '''        float T = 1.0;
        float acc = 0.0;
        float accAll = 0.0;
        float sinSun = max(litSunDir.y, 0.03);'''),
    ('''            if (c.wet > 1e-6) {
                float vis = fogSunVisibility(rel);
                acc += c.wet * vis * exp(-c.tauUp / sinSun) * T * dt;
            }''', '''            if (c.wet > 1e-6) {
                float vis = fogSunVisibility(rel);
                float w = c.wet * exp(-c.tauUp / sinSun) * T * dt;
                acc += w * vis;
                accAll += w;
            }'''),
    ('''        float lobe = kFogPi * max(fogMiePhase(dot(d, litSunDir.xyz)) - kFogIso, 0.0);
        vec3 beam = fogSunRadiance() * fogNearSun.rgb * (lobe * acc);''', '''        float lobe = kFogPi * max(fogMiePhase(dot(d, litSunDir.xyz)) - kFogIso, 0.0);
        // The shafts read against the shade around them: the share of the
        // ray's wet air that sees the sun is lifted (its square root), so a
        // ray lit for a fifth of its way in a wood reads twice as bright,
        // while open air (all of it lit) and air all in shade keep theirs.
        float lit = accAll > 1e-9 ? acc / accAll : 0.0;
        vec3 beam = fogSunRadiance() * fogNearSun.rgb * (lobe * accAll * sqrt(lit));'''),
    # ground air: pools in low ground
    ('''    float sGround = fogGround.x * exp(-hAbove / max(fogGround.y, 1.0)) * nearKeep;
    float overFloor = max(p.y - floorY, 0.0);''', '''    float overFloor = max(p.y - floorY, 0.0);
    // The humid air pools in the low ground: thin on the slopes and ridges.
    float sGround = fogGround.x * exp(-hAbove / max(fogGround.y, 1.0) - overFloor / 150.0) * nearKeep;'''),
])

edit('engine/PoseidonGL33/EngineGL33_PostFog.cpp', [
    ('''                _fog.ringTex[i] = Make2D(GL_RGBA16F, GL_RGBA, GL_FLOAT, kFogRingBins, 2, zero.data());''',
     '''                _fog.ringTex[i] = Make2D(GL_RGBA16F, GL_RGBA, GL_FLOAT, kFogRingBins, 1, zero.data());'''),
    ('''            const std::vector<float> zero(static_cast<size_t>(kFogRingBins) * 2 * 4, 0.0f);''',
     '''            const std::vector<float> zero(static_cast<size_t>(kFogRingBins) * 4, 0.0f);'''),
    ('''        DrawTo(_fog.ringFbo[next], kFogRingBins, 2);''', '''        DrawTo(_fog.ringFbo[next], kFogRingBins, 1);'''),
])
edit('engine/PoseidonGL33/EngineGL33.hpp', [
    ('''        // The horizon ring (RGBA16F, kFogRingBins x 2, ping-pong): the sky's''',
     '''        // The horizon ring (RGBA16F, kFogRingBins x 1, ping-pong): the sky's'''),
])

edit('engine/Poseidon/Graphics/Remaster/VolumetricFog.hpp', [
    ('''constexpr float kHazeClear = 2.2e-4f;''', '''constexpr float kHazeClear = 1.8e-4f;'''),
    ('''constexpr float kFogBeamGain = 5.0f;''', '''constexpr float kFogBeamGain = 8.0f;'''),
    ('''constexpr float kGroundHazeHeight = 40.0f;  // the humid air near the ground: its scale height above the ground''',
     '''constexpr float kGroundHazeHeight = 40.0f;  // the humid air near the ground: its scale height above the ground
constexpr float kGroundHazePool = 150.0f;   // and over the fog floor: it pools in the low ground'''),
])
edit('engine/Poseidon/Graphics/Remaster/VolumetricFog.cpp', [
    ('''    float sigma = m.groundSigma * std::exp(-std::max(aboveGround, 0.0f) / std::max(m.groundHeight, 1.0f));''',
     '''    float sigma = m.groundSigma * std::exp(-std::max(aboveGround, 0.0f) / std::max(m.groundHeight, 1.0f) -
                                           std::max(y - floorY, 0.0f) / kGroundHazePool);'''),
])
print('ok')
