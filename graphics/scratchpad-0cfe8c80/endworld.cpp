void EngineGL33::PostFrameGeometry(PostFrame& f) const
{
    const GfxMatrix& proj = _frameState.projection;
    f.projInfo[0] = proj._33;
    f.projInfo[1] = proj._43;
    f.projInfo[2] = 1.0f / proj._11;
    f.projInfo[3] = 1.0f / proj._22;
    // The 3D scene may fill only a sub-rectangle (aspect pillarbox); uv <-> NDC
    // goes through that rectangle.
    const Poseidon::AspectSettings& as = _aspectSettings;
    const bool fullRect =
        as.worldLeft <= 0.0f && as.worldTop <= 0.0f && as.worldRight >= 1.0f && as.worldBottom >= 1.0f;
    f.rect[0] = fullRect ? 0.0f : as.worldLeft;
    f.rect[1] = fullRect ? 0.0f : 1.0f - as.worldBottom;
    f.rect[2] = fullRect ? 1.0f : as.worldRight - as.worldLeft;
    f.rect[3] = fullRect ? 1.0f : as.worldBottom - as.worldTop;
    f.depthScale = GLAD_GL_ARB_clip_control ? 1.0f : 2.0f;
    f.depthBias = GLAD_GL_ARB_clip_control ? 0.0f : -1.0f;
}

void EngineGL33::PostFrameSun(PostFrame& f) const
{
    // The sun's place and weight on screen for the shafts, when it is in front
    // of the camera and above the horizon.
    LightSun* sun = GScene->MainLight();
    Camera* cam = GScene->GetCamera();
    const Vector3 toSun = -sun->SunDirection();
    // Camera space: InverseScaled maps world directions into view space.
    const Vector3 v = cam->InverseScaled().Rotate(toSun);
    if (v.Z() > 0.05f && toSun.Y() > -0.02f)
    {
        const GfxMatrix& proj = _frameState.projection;
        const float ndcX = proj._11 * v.X() / v.Z();
        const float ndcY = proj._22 * v.Y() / v.Z();
        f.sunU = f.rect[0] + (ndcX * 0.5f + 0.5f) * f.rect[2];
        f.sunV = f.rect[1] + (ndcY * 0.5f + 0.5f) * f.rect[3];
        const float offX = std::fabs(ndcX) > 1.0f ? std::fabs(ndcX) - 1.0f : 0.0f;
        const float offY = std::fabs(ndcY) > 1.0f ? std::fabs(ndcY) - 1.0f : 0.0f;
        const float onScreen = std::exp(-3.0f * (offX + offY));
        const float lowSun = 0.55f + 0.45f * (1.0f - std::min(1.0f, toSun.Y() / 0.6f));
        Landscape* land = GScene->GetLandscape();
        const float clear = land ? land->SkyThrough() : 1.0f;
        f.shaftWeight = onScreen * lowSun * clear * (1.0f - sun->NightEffect());
        f.shaftTint = _lightRigSun;
    }
}

void EngineGL33::EndWorldScene()
{
    // Only the frame that actually drew the world through the remastered
    // shaders has a sky light share in its alpha and a world depth buffer.
    const bool worldDrawn = _worldPassOpen;
    // Queued world draws commit with the remastered programs before the HUD
    // and dialogs switch back to the classic set (and before the world's GPU
    // timestamp, so the world section holds all of them).
    if (_remasterPrograms || (_gpuTiming && worldDrawn))
        FlushAndFreeAllQueues(_queueNo);
    if (worldDrawn)
        GpuMark(Poseidon::gpu::Mark::WorldEnd);
    _worldPassOpen = false;
    _worldEffectsOn = false;
    _worldSceneEnded = true;
    UpdateRemasterPrograms();
    if (_lightingActive)
        UpdateShadowMapLitState(); // disarms the shadow test for the HUD and dialogs
    if (!worldDrawn || !_glContext || !_lightingActive || !_lighting.postEffects || !SSAAActive() ||
        _portraitTargetSize)
        return;

    // This frame's stages. A vehicle interior clears the world's depth and
    // draws itself with its own near plane (Transport::Draw): no world depth to
    // reconstruct from, so no occlusion and no depth-tested shafts this frame.
    // A screen overlay drawn inside the world pass (gunner optics mask) is in
    // the frame already: glow would grey its black surround.
    PostFrame f;
    PostFrameGeometry(f);
    rm::PostFrameInputs in;
    in.ambientOcclusion = _lighting.ambientOcclusion;
    in.bloom = _lighting.bloom;
    in.sunShafts = _lighting.sunShafts;
    in.debugView = _lighting.debugView;
    in.worldDepth = !_worldDepthCleared;
    in.glow = !_worldOverlayDrawn;
    if (in.worldDepth && in.glow && in.sunShafts > 0.0f && GScene)
        PostFrameSun(f);
    in.shaftWeight = f.shaftWeight;
    f.plan = rm::PlanPostStages(in);
    if (f.plan.Empty())
        return; // nothing to add: leave the frame as drawn

    FlushAndFreeAllQueues(_queueNo);
    RenderTargetSize(f.w, f.h);
    if (f.w <= 0 || f.h <= 0 || !EnsurePostResources(f.w, f.h))
        return;
    f.hw = (f.w + 1) / 2;
    f.hh = (f.h + 1) / 2;

    // The stages in list order, each followed by its GPU mark. The resolve
    // blits under the state the world left, as it always has; the fullscreen
    // stages after it share one state.
    for (int i = 0; i < rm::kPostStageCount; ++i)
    {
        const auto stage = static_cast<rm::PostStage>(i);
        if (!f.plan.Runs(stage))
            continue;
        RunPostStage(stage, f);
        GpuMark(rm::GetPostStageInfo(stage).endMark);
        if (stage == rm::PostStage::Resolve)
            BeginPostFullscreen();
    }
    GpuMark(Poseidon::gpu::Mark::PostEnd);
    EndPostFullscreen();
}

void EngineGL33::RunPostStage(rm::PostStage stage, const PostFrame& f)
{
    switch (stage)
    {
        case rm::PostStage::Resolve:
            PostResolve(f);
            break;
        case rm::PostStage::LinearDepth:
            PostLinearDepth(f);
            break;
        case rm::PostStage::Ao:
            PostAo(f);
            break;
        case rm::PostStage::Bloom:
            PostBloom(f);
            break;
        case rm::PostStage::Shafts:
            PostShafts(f);
            break;
        case rm::PostStage::Composite:
            PostComposite(f);
            break;
        case rm::PostStage::DebugView:
            PostDebugView(f);
            break;
        default:
            break; // a stage that has not landed is never planned
    }
}

void EngineGL33::PostResolve(const PostFrame& f)
{
    // The multisampled frame target: colour (with the ambient share in alpha),
    // depth and stencil.
    const PostTargets& p = _post;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _ssaaFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, p.colorFbo);
    glBlitFramebuffer(0, 0, f.w, f.h, 0, 0, f.w, f.h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, p.depthFbo);
    glBlitFramebuffer(0, 0, f.w, f.h, 0, 0, f.w, f.h, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
}

void EngineGL33::BeginPostFullscreen()
{
    Poseidon::render::depthstencil::Disabled(/*hasStencil*/ false);
    glDisable(GL_STENCIL_TEST);
    Poseidon::render::blend::Opaque();
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    Poseidon::render::pipeline::DisableDepthClamp();
    glBindVertexArray(_post.vao);
    for (int unit : Poseidon::gl33lighting::kPostUnits)
        glBindSampler(static_cast<GLuint>(unit), 0);
}

void EngineGL33::PostLinearDepth(const PostFrame& f)
{
    // Half-resolution view depth (psPostLinearDepth), for the stages that trace
    // or upsample at half resolution.
    const PostTargets& p = _post;
    glUseProgram(p.progLinearDepth);
    SetInt(p.progLinearDepth, "depthTex", 0);
    Set4(p.progLinearDepth, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
    Set4(p.progLinearDepth, "depthMap", f.depthScale, f.depthBias, 0.0f, 0.0f);
    Set4(p.progLinearDepth, "screen", static_cast<float>(f.w), static_cast<float>(f.h), 1.0f / f.w, 1.0f / f.h);
    Set4(p.progLinearDepth, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    BindTex(0, p.depthTex);
    DrawTo(p.linDepthFbo, f.hw, f.hh);
}

void EngineGL33::PostAo(const PostFrame& f)
{
    // Ambient occlusion at half resolution, then the depth-aware blur across
    // and down.
    const PostTargets& p = _post;
    glUseProgram(p.progAo);
    SetInt(p.progAo, "depthTex", 0);
    Set4(p.progAo, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
    Set4(p.progAo, "depthMap", f.depthScale, f.depthBias, 0.0f, 0.0f);
    Set4(p.progAo, "aoParams", kAoRadius, 1.0f, kAoFadeStart, kAoFadeEnd);
    Set4(p.progAo, "screen", static_cast<float>(f.w), static_cast<float>(f.h), 1.0f / f.w, 1.0f / f.h);
    Set4(p.progAo, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    BindTex(0, p.depthTex);
    DrawTo(p.aoFbo[0], f.hw, f.hh);
    glUseProgram(p.progAoBlur);
    SetInt(p.progAoBlur, "aoTex", 0);
    BindTex(0, p.aoTex[0]);
    Set2(p.progAoBlur, "stepUV", 1.0f / f.hw, 0.0f);
    DrawTo(p.aoFbo[1], f.hw, f.hh);
    BindTex(0, p.aoTex[1]);
    Set2(p.progAoBlur, "stepUV", 0.0f, 1.0f / f.hh);
    DrawTo(p.aoFbo[0], f.hw, f.hh);
}

void EngineGL33::PostBloom(const PostFrame& f)
{
    const PostTargets& p = _post;
    glUseProgram(p.progBloomDown);
    SetInt(p.progBloomDown, "srcTex", 0);
    for (int i = 0; i < PostTargets::kBloomLevels; ++i)
    {
        const bool first = i == 0;
        const GLuint src = first ? p.colorTex : p.bloomTex[i - 1];
        const int sw = first ? f.w : p.bloomW[i - 1];
        const int sh = first ? f.h : p.bloomH[i - 1];
        BindTex(0, src);
        Set2(p.progBloomDown, "srcTexel", 1.0f / sw, 1.0f / sh);
        Set4(p.progBloomDown, "bloomParams", kBloomThreshold, kBloomKnee, first ? 1.0f : 0.0f, 0.0f);
        DrawTo(p.bloomFbo[i], p.bloomW[i], p.bloomH[i]);
    }
    glUseProgram(p.progBloomUp);
    SetInt(p.progBloomUp, "srcTex", 0);
    Poseidon::render::blend::Additive(); // (SRC_ALPHA, ONE) with alpha 1 = add
    for (int i = PostTargets::kBloomLevels - 1; i > 0; --i)
    {
        BindTex(0, p.bloomTex[i]);
        Set2(p.progBloomUp, "srcTexel", 1.0f / p.bloomW[i], 1.0f / p.bloomH[i]);
        DrawTo(p.bloomFbo[i - 1], p.bloomW[i - 1], p.bloomH[i - 1]);
    }
    Poseidon::render::blend::Opaque();
}

void EngineGL33::PostShafts(const PostFrame& f)
{
    const PostTargets& p = _post;
    glUseProgram(p.progShafts);
    SetInt(p.progShafts, "depthTex", 0);
    SetInt(p.progShafts, "colorTex", 2);
    BindTex(0, p.depthTex);
    BindTex(2, p.colorTex);
    Set4(p.progShafts, "sunScreen", f.sunU, f.sunV, f.shaftWeight, static_cast<float>(f.w) / static_cast<float>(f.h));
    DrawTo(p.shaftFbo, f.hw, f.hh);
}

void EngineGL33::PostComposite(const PostFrame& f)
{
    // Into the frame target (every sample gets the resolved value). A stage
    // that did not run this frame reads as its 1x1 stand-in.
    const PostTargets& p = _post;
    const rm::PostStagePlan& plan = f.plan;
    glUseProgram(p.progComposite);
    SetInt(p.progComposite, "colorTex", 0);
    SetInt(p.progComposite, "aoTex", 2);
    SetInt(p.progComposite, "bloomTex", 3);
    SetInt(p.progComposite, "shaftTex", 4);
    BindTex(0, p.colorTex);
    BindTex(2, plan.Runs(rm::PostStage::Ao) ? p.aoTex[0] : p.whiteTex);
    BindTex(3, plan.Runs(rm::PostStage::Bloom) ? p.bloomTex[0] : p.blackTex);
    BindTex(4, plan.Runs(rm::PostStage::Shafts) ? p.shaftTex : p.blackTex);
    Set4(p.progComposite, "compParams", plan.aoStrength, plan.bloomStrength * 0.25f, _lighting.sunShafts * 0.6f, 1.0f);
    const Color& tint = f.shaftTint;
    const float lum = std::max(1e-4f, 0.2126f * tint.R() + 0.7152f * tint.G() + 0.0722f * tint.B());
    Set4(p.progComposite, "shaftColor", tint.R() / lum, tint.G() / lum, tint.B() / lum, 1.0f);
    // The shafts keep only what the air in front of each pixel scatters; the
    // depth is read only where there is a shaft term.
    SetInt(p.progComposite, "depthTex", 5);
    BindTex(5, p.depthTex);
    Set4(p.progComposite, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
    const float scatterLength = Poseidon::remaster::ShaftScatterLength(GScene ? GScene->GetFogMaxRange() : 0.0f);
    Set4(p.progComposite, "depthMap", f.depthScale, f.depthBias, 1.0f / scatterLength, 0.0f);
    Set4(p.progComposite, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    Set4(p.progComposite, "debugView", _lighting.debugView == 4 ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
    DrawTo(_ssaaFbo, f.w, f.h);
}

void EngineGL33::PostDebugView(const PostFrame& f)
{
    // Debug view 5: the linear depth over the whole frame target.
    const PostTargets& p = _post;
    glUseProgram(p.progDebugDepth);
    SetInt(p.progDebugDepth, "linDepthTex", 0);
    BindTex(0, p.linDepthTex);
    DrawTo(_ssaaFbo, f.w, f.h);
}

void EngineGL33::EndPostFullscreen()
{
    // Hand the GL state back: frame target bound with its full viewport, unit 0
    // sampler restored, caches invalidated so the next draw re-applies state.
    for (int unit : Poseidon::gl33lighting::kPostUnits)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindSampler(0, _samplerObjects[0]);
    glBindVertexArray(0);
    glUseProgram(0);
    BindFrameRenderTarget();
    _pixelShaderSel = PSNone;
    GL33Bind::Invalidate();
    InvalidatePipelineCache();
}
