from pathlib import Path

E = Path(r"D:\Arma_CWA_worktrees\gr-B8\arma_CWR\engine\PoseidonGL33")


def rep(path, old, new):
    s = path.read_text(encoding="utf-8")
    assert s.count(old) == 1, (path.name, old[:80], s.count(old))
    path.write_text(s.replace(old, new), encoding="utf-8", newline="\n")


fx = E / "EngineGL33_PostFX.cpp"
rep(fx, """    // The air pass: the march's two targets and the blur's ping-pong (filtered:
    // the blur and the mean tap between texels), the mean, and the frame with
    // the air applied (filtered as colorTex is, for the bloom's taps).
    p.airTex[0] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    p.airTex[1] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    p.airMistTex = MakeTexture(GL_RG16F, GL_RG, GL_HALF_FLOAT, hw, hh, false);
    p.airMeanTex = MakeTexture(GL_RG16F, GL_RG, GL_HALF_FLOAT, hw, hh, false);""",
    """    // The air pass: the march's two targets and the blur's ping-pong (filtered:
    // the blur and the mean tap between texels, the second target holds the
    // view depth they match), the mean, and the frame with the air applied
    // (filtered as colorTex is, for the bloom's taps).
    p.airTex[0] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    p.airTex[1] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    p.airMistTex = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    p.airMeanTex = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, false);""")

pa = E / "EngineGL33_PostAir.cpp"
rep(pa, """//   1. psPostAirMarch, half resolution, on the half-resolution linear depth:
//      sun beams (single scattering in a ground mist, the sun visibility from
//      the shadow map, the horizon map and the cloud field, 24 steps over the
//      first 250 m), headlight cones (up to four spotlights, night only), the
//      haze's sun visibility (12 samples beyond 150 m) and the valley mist's
//      top at the surface each ray ends on. Under TAA the march's start turns""",
    """//   1. psPostAirMarch, half resolution, on the half-resolution linear depth:
//      sun beams (single scattering in a ground mist, the sun visibility from
//      the shadow map, the horizon map and the cloud field, 24 steps over the
//      first 250 m, and the same scattering as if all of the ray saw the sun),
//      headlight cones (up to four spotlights, night only), the haze's sun
//      visibility (12 samples beyond 150 m), the valley mist's top at the
//      surface each ray ends on and the view depth. Under TAA the march's start turns""")
rep(pa, """//   3. psPostAirMean (beams only): the depth-matched local mean the beam
//      contrast works against;""", """//   3. psPostAirMean (beams only): the depth-matched local mean the beam
//      contrast works against (for the sun, on the share of the scattering
//      that sees the sun, so open ground toward the horizon gets no band);""")
rep(pa, """// GL state: the stage binds its inputs to kPostUnits; unit 2 takes the cascade""",
    """// The blur and the mean read the view depth from the march's second target on
// unit 1.
//
// GL state: the stage binds its inputs to kPostUnits; unit 2 takes the cascade""")
# Reviewer finding (c): bind the terrain maps whenever the shading's litTerrain
# says they are valid (terrain shadows or terrain occlusion), as airGround reads
# the height map then; the horizon only shades the air with terrain shadows on
# (litTerrainLift.z), so only then is the haze's visibility worth marching.
rep(pa, """    // The haze's sun visibility matters where hills or clouds can shade the air
    // and the sun is up.
    const bool terrain = _terrainHorizonTex && _terrainHeightTex && _terrainMap && _terrainMap->Valid() &&
                         _lighting.terrainShadows > 0.0f;
    const bool clouds = gates.cloudCover > 0.0f && _lighting.cloudShadows > 0.0f;
    const bool vis = (m.hazeSigma > 0.0f || m.valleyMist > 0.0f) && gates.sunElevationDeg > -2.0f &&
                     ((terrain && gates.sunElevationDeg < kAirVisMaxSunDeg) || clouds);""",
    """    // The terrain maps, as the shading's litTerrain sees them (airGround reads
    // the heights whenever litTerrain.x is set: terrain shadows or terrain
    // occlusion on). The haze's sun visibility matters where hills (terrain
    // shadows on) or clouds can shade the air and the sun is up.
    const bool terrain = _terrainHorizonTex && _terrainHeightTex && _terrainMap && _terrainMap->Valid() &&
                         (_lighting.terrainShadows > 0.0f || _lighting.terrainOcclusion > 0.0f);
    const bool hills = terrain && _lighting.terrainShadows > 0.0f;
    const bool clouds = gates.cloudCover > 0.0f && _lighting.cloudShadows > 0.0f;
    const bool vis = (m.hazeSigma > 0.0f || m.valleyMist > 0.0f) && gates.sunElevationDeg > -2.0f &&
                     ((hills && gates.sunElevationDeg < kAirVisMaxSunDeg) || clouds);""")
rep(pa, """        glUseProgram(p.progAirBlur);
        SetInt(p.progAirBlur, "airTex", 0);
        BindTex(0, p.airTex[0]);""", """        glUseProgram(p.progAirBlur);
        SetInt(p.progAirBlur, "airTex", 0);
        SetInt(p.progAirBlur, "airDepthTex", 1);
        BindTex(1, p.airMistTex);
        BindTex(0, p.airTex[0]);""")
rep(pa, """            glUseProgram(p.progAirMean);
            SetInt(p.progAirMean, "airTex", 0);
            BindTex(0, p.airTex[0]);""", """            glUseProgram(p.progAirMean);
            SetInt(p.progAirMean, "airTex", 0);
            SetInt(p.progAirMean, "airDepthTex", 1);
            BindTex(0, p.airTex[0]);""")
print("ok")
