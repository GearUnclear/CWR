"""Wires the RESEARCH volumetric pass into EngineGL33_PostFX.cpp (scratch worktree)."""
import sys

p = sys.argv[1]
s = open(p, encoding='utf-8').read()


def rep(old, new, count=1):
    global s
    n = s.count(old)
    assert n == count, (old[:80], n)
    s = s.replace(old, new)


rep("""    delFbo(p.shaftFbo);
""", """    delFbo(p.shaftFbo);
    delFbo(p.volFbo[0]);
    delFbo(p.volFbo[1]);
""")
rep("""    delTex(p.shaftTex);
""", """    delTex(p.shaftTex);
    delTex(p.volTex[0]);
    delTex(p.volTex[1]);
""")
rep("""         {&p.progAo, &p.progAoBlur, &p.progBloomDown, &p.progBloomUp, &p.progShafts, &p.progComposite})""",
    """         {&p.progAo, &p.progAoBlur, &p.progBloomDown, &p.progBloomUp, &p.progShafts, &p.progComposite, &p.progVol,
          &p.progVolBlur})""")
rep("""        p.progComposite = LinkPostProgram(vs, s_psPostCompositeGLSL, "psPostComposite");
""", """        p.progComposite = LinkPostProgram(vs, s_psPostCompositeGLSL, "psPostComposite");
        // RESEARCH: volumetric scattering (optional: its failure only disables it).
        p.progVol = LinkPostProgram(vs, s_psPostVolumetricGLSL, "psPostVolumetric");
        p.progVolBlur = LinkPostProgram(vs, s_psPostVolBlurGLSL, "psPostVolBlur");
        if (p.progVol)
        {
            const GLuint blk = glGetUniformBlockIndex(p.progVol, "LitConstants");
            if (blk != GL_INVALID_INDEX)
                glUniformBlockBinding(p.progVol, blk, Poseidon::gl33lighting::kLitBinding);
        }
        LOG_INFO(Graphics, "GL33 post: volumetric programs {} {}", p.progVol ? "ok" : "FAILED",
                 p.progVolBlur ? "ok" : "FAILED");
""")
rep("""    p.shaftTex = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
""", """    p.shaftTex = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    p.volTex[0] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    p.volTex[1] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
""")
rep("""    p.shaftFbo = MakeFbo(GL_COLOR_ATTACHMENT0, p.shaftTex);
    check();
""", """    p.shaftFbo = MakeFbo(GL_COLOR_ATTACHMENT0, p.shaftTex);
    check();
    for (int i = 0; i < 2; ++i)
    {
        p.volFbo[i] = MakeFbo(GL_COLOR_ATTACHMENT0, p.volTex[i]);
        check();
    }
""")
rep("""    if (_lighting.ambientOcclusion <= 0.0f && _lighting.bloom <= 0.0f && _lighting.sunShafts <= 0.0f)
        return; // nothing to add: leave the frame as drawn""",
    """    if (_lighting.ambientOcclusion <= 0.0f && _lighting.bloom <= 0.0f && _lighting.sunShafts <= 0.0f &&
        _lighting.volumetric <= 0.0f)
        return; // nothing to add: leave the frame as drawn""")
rep("""    if (worldDepth && glow && _lighting.sunShafts > 0.0f && GScene)
    {""", """    const bool volumetric = worldDepth && glow && _lighting.volumetric > 0.0f && GScene && GScene->GetCamera() &&
                            p.progVol && p.progVolBlur;
    if (!volumetric && worldDepth && glow && _lighting.sunShafts > 0.0f && GScene)
    {""")
rep("""        GpuMark(Poseidon::gpu::Mark::ShaftsEnd);
    }
""", """        GpuMark(Poseidon::gpu::Mark::ShaftsEnd);
    }

    // 3b. RESEARCH: ray-marched volumetric scattering (replaces the shafts).
    if (volumetric)
    {
        LightSun* sun = GScene->MainLight();
        Camera* cam = GScene->GetCamera();
        const GLuint prog = p.progVol;
        glUseProgram(prog);
        u1i(prog, "depthTex", 0);
        u1i(prog, "shadowArr", 2);
        u1i(prog, "terrainHorizon", 3);
        u1i(prog, "terrainHeight", 4);
        u1i(prog, "cloudNoise", 5);
        bindTex(0, p.depthTex);
        const bool shadowValid = ShadowMapsEnabled() && _shadowMapActive && _shadowMapTex && _shadowCascades > 0;
        const bool map = _terrainHorizonTex && _terrainHeightTex;
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, shadowValid ? _shadowMapTex : _dummyDepthArray);
        glBindSampler(2, _shadowCompareSampler);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, map ? _terrainHorizonTex : _dummyHorizonTex);
        bindTex(4, map ? _terrainHeightTex : _dummyHeightTex);
        bindTex(5, _cloudNoiseTex);
        u4f(prog, "projInfo", proj._33, proj._43, 1.0f / proj._11, 1.0f / proj._22);
        u4f(prog, "depthMap", depthScale, depthBias, 0.0f, 0.0f);
        u4f(prog, "worldRect", rectX, rectY, rectW, rectH);
        {
            const GLint loc = glGetUniformLocation(prog, "viewRot");
            if (loc >= 0)
                glUniformMatrix4fv(loc, 1, GL_FALSE, &_frameState.view._11);
        }
        if (shadowValid)
        {
            const GLint loc = glGetUniformLocation(prog, "cascadeVP");
            if (loc >= 0)
                glUniformMatrix4fv(loc, _shadowCascades, GL_FALSE, _shadowMapVP);
            u4f(prog, "cascadeSplits", _shadowSplits[0], _shadowSplits[1], _shadowSplits[2], _shadowSplits[3]);
            u4f(prog, "cascadeCtl", static_cast<float>(_shadowCascades), _shadowTuning.fadeRange,
                _shadowTuning.biasBase, static_cast<float>(_shadowOmniCount));
            u4f(prog, "shadowCam", _shadowCamFwd[0], _shadowCamFwd[1], _shadowCamFwd[2], 1.0f);
        }
        else
        {
            u4f(prog, "shadowCam", 0.0f, 0.0f, 1.0f, 0.0f);
        }
        const Vector3 camPos = cam->Position();
        u4f(prog, "shadowDelta", camPos.X() - _shadowCamPos[0], camPos.Y() - _shadowCamPos[1],
            camPos.Z() - _shadowCamPos[2], 0.0f);
        // Medium: a haze whose density follows the weather's visibility (fog
        // range), and a ground mist that thickens as the fog closes in.
        const float fogMax = std::max(50.0f, GScene->GetFogMaxRange());
        const float viewDist = std::max(fogMax, GScene->GetPreferredViewDistance());
        const float fogness = std::clamp(1.0f - fogMax / viewDist, 0.0f, 1.0f);
        const float hazeSigma = _lighting.volHaze * 0.6f / fogMax;
        const float mistSigma = _lighting.volMist * 0.004f * (1.0f + 6.0f * fogness);
        u4f(prog, "volParams", _lighting.volumetric, hazeSigma, mistSigma, std::max(1.0f, _lighting.volMistHeight));
        u4f(prog, "volParams2", std::clamp(_lighting.volAniso, -0.95f, 0.95f), std::max(10.0f, _lighting.volDistance),
            std::clamp(_lighting.volSteps, 4.0f, 128.0f), _lighting.volCarve);
        // Local lights: the scene's active point and spot lights at night.
        const float night = sun->NightEffect();
        float lpos[16 * 4] = {}, lcol[16 * 4] = {}, ldir[16 * 4] = {};
        int nLights = 0;
        if (night > 0.0f && _lighting.volLocal > 0.0f)
        {
            const LightList& lights = GScene->ActiveLights();
            for (int i = 0; i < lights.Size() && nLights < 16; i++)
            {
                Light* light = lights[i];
                if (!light)
                    continue;
                LightDescription desc;
                light->GetDescription(desc);
                const bool isSpot = desc.type == LTSpotLight;
                if (desc.type != LTPoint && !isSpot)
                    continue;
                Vector3 beam = desc.dir;
                beam.Normalize();
                const Color dif = desc.diffuse * night;
                float* pp = lpos + nLights * 4;
                pp[0] = desc.pos.X() - camPos.X();
                pp[1] = desc.pos.Y() - camPos.Y();
                pp[2] = desc.pos.Z() - camPos.Z();
                pp[3] = desc.startAtten;
                float* pc = lcol + nLights * 4;
                pc[0] = std::pow(std::max(dif.R(), 0.0f), 2.2f);
                pc[1] = std::pow(std::max(dif.G(), 0.0f), 2.2f);
                pc[2] = std::pow(std::max(dif.B(), 0.0f), 2.2f);
                pc[3] = isSpot ? 1.0f : 0.0f;
                float* pd = ldir + nLights * 4;
                pd[0] = beam.X();
                pd[1] = beam.Y();
                pd[2] = beam.Z();
                nLights++;
            }
        }
        if (nLights > 0)
        {
            const GLint lp = glGetUniformLocation(prog, "volLightPos");
            const GLint lc = glGetUniformLocation(prog, "volLightColor");
            const GLint ld = glGetUniformLocation(prog, "volLightDir");
            if (lp >= 0)
                glUniform4fv(lp, nLights, lpos);
            if (lc >= 0)
                glUniform4fv(lc, nLights, lcol);
            if (ld >= 0)
                glUniform4fv(ld, nLights, ldir);
        }
        static int s_volLogFrame = 0;
        if ((s_volLogFrame++ % 300) == 0)
            LOG_INFO(Graphics, "GL33 post: volumetric fogMax {} vd {} haze {} mist {} lights {} shadow {} map {}",
                     fogMax, viewDist, hazeSigma, mistSigma, nLights, shadowValid ? 1 : 0, map ? 1 : 0);
        u4f(prog, "volParams3", _sunEnabled ? 1.0f - night : 0.0f, _lighting.volLocal, static_cast<float>(nLights),
            0.0f);
        drawTo(p.volFbo[0], hw, hh);
        glBindSampler(2, 0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        glUseProgram(p.progVolBlur);
        u1i(p.progVolBlur, "volTex", 0);
        bindTex(0, p.volTex[0]);
        u2f(p.progVolBlur, "stepUV", 1.0f / hw, 0.0f);
        drawTo(p.volFbo[1], hw, hh);
        bindTex(0, p.volTex[1]);
        u2f(p.progVolBlur, "stepUV", 0.0f, 1.0f / hh);
        drawTo(p.volFbo[0], hw, hh);
        GpuMark(Poseidon::gpu::Mark::ShaftsEnd);
    }
""")
rep("""    u1i(p.progComposite, "shaftTex", 4);
""", """    u1i(p.progComposite, "shaftTex", 4);
    u1i(p.progComposite, "fullDepthTex", 5);
""")
rep("""    bindTex(4, shaftWeight > 0.001f ? p.shaftTex : p.blackTex);
    u4f(p.progComposite, "compParams", aoStrength, bloomStrength * 0.25f, _lighting.sunShafts * 0.6f, 1.0f);""",
    """    bindTex(4, volumetric ? p.volTex[0] : (shaftWeight > 0.001f ? p.shaftTex : p.blackTex));
    bindTex(5, p.depthTex);
    u4f(p.progComposite, "volUp", volumetric ? 1.0f : 0.0f, _lighting.volDebug > 0.5f ? 1.0f : 0.0f,
        static_cast<float>(hw), static_cast<float>(hh));
    u4f(p.progComposite, "volProj", proj._33, proj._43, depthScale, depthBias);
    u4f(p.progComposite, "compParams", aoStrength, bloomStrength * 0.25f,
        volumetric ? 1.0f : _lighting.sunShafts * 0.6f, 1.0f);""")
open(p, 'w', encoding='utf-8', newline='').write(s)
print("patched")
