// Remastered lighting: the air pass (the Air post stage). Feature switches airBeams,
// airHaze and valleyMist; the medium and its weather and time rules are pure
// (Graphics/Remaster/AirMedium.hpp), the shaders GL-free
// (EngineGL33_PostAirShaders.cpp).
//
// The stage runs after the occlusion and before the bloom, on the resolved
// frame, when one of the three switches and its frame gate are on:
//   1. psPostAirMarch, half resolution, on the half-resolution linear depth:
//      sun beams (single scattering in a ground mist, the sun visibility from
//      the shadow map, the horizon map and the cloud field, 24 steps over the
//      first 250 m, and the same scattering as if all of the ray saw the sun),
//      headlight cones (up to four spotlights, night only), the haze's sun
//      visibility (12 samples beyond 150 m), the valley mist's top at the
//      surface each ray ends on and the view depth. Under TAA the march's start turns
//      every frame (the golden-ratio step of the TAA sequence) and the history
//      averages it, in place of a temporal accumulation of its own;
//   2. psPostAirBlur twice (across, down), depth-aware;
//   3. psPostAirMean (beams only): the depth-matched local mean the beam
//      contrast works against (for the sun, on the share of the scattering
//      that sees the sun, so open ground toward the horizon gets no band);
//   4. psPostAirApply, full resolution: haze and mist in closed form toward the
//      sky behind each pixel, then the beams, over the frame the stages before
//      it left (PostFrame::frameColor: the resolved colour, or the frame with
//      the reflections when Ssr ran), into airOutTex. The bloom and the
//      composite read airOutTex in place of that frame; with the stage off they
//      read it as before, so "off" is the frame before the feature.
// When the air pass draws beams the legacy shafts hold off (PlanPostStages).
// With the haze on, the forward fog keeps only its view-distance edge
// (UpdateAirMedium sets litHaze.x; the shaders' finishFragment).
//
// The blur and the mean read the view depth from the march's second target on
// unit 3.
//
// GL state: the stage binds its inputs to kPostUnits; unit 2 takes the cascade
// array (or the depth stand-in, never texture 0: a shader must never sample an incomplete texture) under the
// comparison sampler for the march and gets the post pass's sampler 0 back
// after it; unit 16 takes the horizon array (or its stand-in) the world passes
// bind there. Unit 8 keeps the sky table the world bound, unit 9 the sky's
// cloud density (bound again for the march) and unit 15 the canopy's
// sun-visibility map (named by the pasted piece, never read here).

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/EngineGL33_PostPass.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Remaster/AirMedium.hpp>
#include <Poseidon/Graphics/Remaster/PhotoCamera.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>

#include <algorithm>
#include <cmath>

namespace rm = Poseidon::remaster;
using namespace Poseidon::gl33post;
using namespace Poseidon::gl33lighting;

namespace
{

constexpr float kAirMeanRadius = 0.12f;    // the beams' local mean: a disc this share of the frame height
constexpr float kAirMistFullFrom = 600.0f; // m: the valley mist shows from the near distance, in full from here
constexpr float kAirVisMaxSunDeg = 50.0f;  // the horizon map stores angles up to 45 deg: no ridge shades the air above
constexpr float kAirSunMinSin = -0.035f;   // the sun's beams fade in from 2 deg under the horizon
constexpr float kAirSunFullSin = 0.0175f;  // to 1 deg above it

void SetMat3(GLuint prog, const char* name, const float* columnMajor)
{
    const GLint loc = glGetUniformLocation(prog, name);
    if (loc >= 0)
        glUniformMatrix3fv(loc, 1, GL_FALSE, columnMajor);
}

// The view matrix's rotation, inverted: view-space directions (x right, y up,
// z forward) to camera-relative world. View x = _11 X + _21 Y + _31 Z (the
// vertex shaders' `view * p` with the matrix uploaded from &_11), so the rows
// of the rotation are (_11, _21, _31), (_12, _22, _32), (_13, _23, _33).
void ViewToWorld(const GfxMatrix& v, float out[9])
{
    const double r[3][3] = {{v._11, v._21, v._31}, {v._12, v._22, v._32}, {v._13, v._23, v._33}};
    const double c00 = r[1][1] * r[2][2] - r[1][2] * r[2][1];
    const double c01 = r[1][2] * r[2][0] - r[1][0] * r[2][2];
    const double c02 = r[1][0] * r[2][1] - r[1][1] * r[2][0];
    double det = r[0][0] * c00 + r[0][1] * c01 + r[0][2] * c02;
    if (std::fabs(det) < 1e-12)
        det = 1.0;
    const double inv[3][3] = {
        {c00 / det, (r[0][2] * r[2][1] - r[0][1] * r[2][2]) / det, (r[0][1] * r[1][2] - r[0][2] * r[1][1]) / det},
        {c01 / det, (r[0][0] * r[2][2] - r[0][2] * r[2][0]) / det, (r[0][2] * r[1][0] - r[0][0] * r[1][2]) / det},
        {c02 / det, (r[0][1] * r[2][0] - r[0][0] * r[2][1]) / det, (r[0][0] * r[1][1] - r[0][1] * r[1][0]) / det},
    };
    for (int col = 0; col < 3; ++col)
        for (int row = 0; row < 3; ++row)
            out[col * 3 + row] = static_cast<float>(inv[row][col]);
}

float ToLinear(float display)
{
    return std::pow(std::max(display, 0.0f), 2.2f);
}

} // namespace

void EngineGL33::UpdateAirMedium()
{
    // Once per frame, with the gates (the world's first pass computes them).
    const std::uint32_t frame = GetFrameCounter();
    if (_featureGates.computed && _featureGates.frame == frame && _airFrame != frame && GScene && GScene->MainLight())
    {
        _airFrame = frame;
        // The pass runs where the post effects run and, as the previous frame
        // showed, on a world with its depth and no screen overlay; anywhere
        // else the forward fog keeps its classic edge.
        const bool post = _lighting.postEffects && SSAAActive() && !_post.unavailable && !_post.airUnavailable &&
                          !_prevWorldDepthCleared && !_prevWorldOverlayDrawn;
        rm::AirParts parts;
        parts.haze = post && _featureGates[rm::Feature::AirHaze].on;
        parts.valleyMist = post && _featureGates[rm::Feature::ValleyMist].on;
        parts.beams = post && _featureGates[rm::Feature::AirBeams].on;
        const rm::GateInputs& g = _featureGates.inputs;
        rm::AirInputs in;
        in.sunElevationDeg = g.sunElevationDeg;
        in.sunEast = -GScene->MainLight()->Direction().X();
        in.fog = g.fog;
        in.rain = g.rainDensity;
        in.skyThrough = g.skyThrough;
        in.fogStart = GScene->GetFogMinRange();
        in.fogEnd = GScene->GetFogMaxRange();
        _airMedium = rm::ComputeAirMedium(in, parts, _lighting.volGain);
        _airParts = parts;
    }
    const float scale = _airFrame == frame ? _airMedium.fogEdgeScale : 1.0f;
    SetLitSlot(LitConst::Haze, scale - 1.0f, 0.0f, 0.0f, 0.0f);
}

void EngineGL33::PostAir(const PostFrame& f)
{
    const PostTargets& p = _post;
    const rm::AirParts& parts = _airParts;
    const rm::AirMedium& m = _airMedium;
    LightSun* sun = GScene ? GScene->MainLight() : nullptr;
    Camera* camera = GScene ? GScene->GetCamera() : nullptr;
    const rm::GateInputs& gates = _featureGates.inputs;
    float viewToWorld[9];
    ViewToWorld(_frameState.view, viewToWorld);

    // Sun beams with the sun up; headlight cones at night.
    float sunBeams = 0.0f;
    if (parts.beams && sun && _sunEnabled)
    {
        const float up = -sun->Direction().Y();
        sunBeams = rm::SmoothStep(kAirSunMinSin, kAirSunFullSin, up) * (1.0f - sun->NightEffect());
    }
    float spotPos[rm::kAirMaxSpots * 4] = {};
    float spotDir[rm::kAirMaxSpots * 4] = {};
    float tint[3] = {0.0f, 0.0f, 0.0f};
    float tintLum = 0.0f;
    int spots = 0;
    if (parts.beams && sun && camera && sun->NightEffect() > rm::kGateNightMinEffect)
    {
        // The nearest switched-on spotlights (headlights) within reach. The
        // scene's active list is already cleared here: walk its lights.
        const float night = sun->NightEffect();
        const Vector3 cam = camera->Position();
        Light* nearest[rm::kAirMaxSpots] = {};
        float nearestD2[rm::kAirMaxSpots] = {};
        for (int i = 0; i < GScene->NLights(); ++i)
        {
            Light* light = GScene->GetLight(i);
            if (!light || !light->IsOn())
                continue;
            LightDescription desc;
            light->GetDescription(desc);
            if (desc.type != LTSpotLight)
                continue;
            const float d2 = (desc.pos - cam).SquareSize();
            if (d2 > rm::kAirSpotRange * rm::kAirSpotRange)
                continue;
            int at = spots < rm::kAirMaxSpots ? spots++ : rm::kAirMaxSpots;
            if (at == rm::kAirMaxSpots)
            {
                if (d2 >= nearestD2[rm::kAirMaxSpots - 1])
                    continue;
                at = rm::kAirMaxSpots - 1;
            }
            while (at > 0 && nearestD2[at - 1] > d2)
            {
                nearest[at] = nearest[at - 1];
                nearestD2[at] = nearestD2[at - 1];
                --at;
            }
            nearest[at] = light;
            nearestD2[at] = d2;
        }
        for (int i = 0; i < spots; ++i)
        {
            LightDescription desc;
            nearest[i]->GetDescription(desc);
            Vector3 beam = desc.dir;
            beam.Normalize();
            // The light's colour as the shading takes it (scaled by the night,
            // display space), in linear light.
            const Color dif = desc.diffuse * night;
            const float lin[3] = {ToLinear(dif.R()), ToLinear(dif.G()), ToLinear(dif.B())};
            const float lum = rm::Luminance(rm::Rgb(lin[0], lin[1], lin[2]));
            for (int c = 0; c < 3; ++c)
                tint[c] += lin[c];
            tintLum += lum;
            float* pos = spotPos + i * 4;
            pos[0] = desc.pos.X() - cam.X();
            pos[1] = desc.pos.Y() - cam.Y();
            pos[2] = desc.pos.Z() - cam.Z();
            pos[3] = desc.startAtten;
            float* dir = spotDir + i * 4;
            dir[0] = beam.X();
            dir[1] = beam.Y();
            dir[2] = beam.Z();
            dir[3] = lum * rm::kAirSpotStrength;
        }
    }
    const bool beams = sunBeams > 0.0f || spots > 0;

    // The terrain maps, as the shading's litTerrain sees them (airGround reads
    // the heights whenever litTerrain.x is set: terrain shadows or terrain
    // occlusion on). The haze's sun visibility matters where hills (terrain
    // shadows on) or clouds can shade the air and the sun is up.
    const bool terrain = _terrainHorizonTex && _terrainHeightTex && _terrainMap && _terrainMap->Valid() &&
                         (_lighting.terrainShadows > 0.0f || _lighting.terrainOcclusion > 0.0f);
    const bool hills = terrain && _lighting.terrainShadows > 0.0f;
    const bool clouds = gates.cloudCover > 0.0f && _lighting.cloudShadows > 0.0f;
    const bool vis = (m.hazeSigma > 0.0f || m.valleyMist > 0.0f) && gates.sunElevationDeg > -2.0f &&
                     ((hills && gates.sunElevationDeg < kAirVisMaxSunDeg) || clouds);
    const bool march = beams || m.valleyMist > 0.0f || vis;

    if (march)
    {
        const GLuint prog = p.progAirMarch;
        glUseProgram(prog);
        SetInt(prog, "linDepthTex", 0);
        SetInt(prog, "shadowMap", kShadowMapUnit);
        SetInt(prog, "terrainHorizon", kTerrainHorizonUnit);
        SetInt(prog, "terrainHeight", kTerrainHeightUnit);
        SetInt(prog, "cloudNoise", kCloudNoiseUnit);
        SetInt(prog, "skyDensity", kSkyDensityUnit);
        SetInt(prog, "canopySun", kCanopyVisUnit);
        BindTex(0, p.linDepthTex);
        // Unit 2: the cascade array (or the depth stand-in) under the
        // comparison sampler, as the world's passes read it.
        const bool shadowMap = ShadowMapsEnabled() && _shadowMapActive && _shadowMapTex && _shadowCascades > 0;
        glActiveTexture(GL_TEXTURE0 + kShadowMapUnit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMap ? _shadowMapTex : _dummyDepthArray);
        glBindSampler(kShadowMapUnit, _shadowCompareSampler);
        glActiveTexture(GL_TEXTURE0 + kTerrainHorizonUnit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, terrain ? _terrainHorizonTex : _dummyHorizonTex);
        BindTex(kTerrainHeightUnit, terrain ? _terrainHeightTex : _dummyHeightTex);
        BindTex(kCloudNoiseUnit, _cloudNoiseTex);
        // Unit 9: the sky's cloud density, for the sky-matched cloud shadows
        // (cloudSunVisibility with litSky.y on), as the world read it.
        BindSkyDensity();
        Set4(prog, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
        Set4(prog, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
        SetMat3(prog, "viewToWorld", viewToWorld);
        if (shadowMap)
        {
            const GLint loc = glGetUniformLocation(prog, "cascadeVP");
            if (loc >= 0)
                glUniformMatrix4fv(loc, std::min(_shadowCascades, kShadowCascades), GL_FALSE, _shadowMapVP);
            Set4(prog, "cascadeSplits", _shadowSplits[0], _shadowSplits[1], _shadowSplits[2], _shadowSplits[3]);
            Set4(prog, "cascadeCtl", static_cast<float>(_shadowCascades), _shadowTuning.fadeRange,
                 _shadowTuning.biasBase, static_cast<float>(_shadowOmniCount));
            Set4(prog, "shadowCam", _shadowCamFwd[0], _shadowCamFwd[1], _shadowCamFwd[2], 1.0f);
        }
        else
        {
            Set4(prog, "shadowCam", 0.0f, 0.0f, 1.0f, 0.0f);
        }
        const Vector3 cam = camera ? camera->Position() : VZero;
        Set4(prog, "shadowDelta", cam.X() - _shadowCamPos[0], cam.Y() - _shadowCamPos[1], cam.Z() - _shadowCamPos[2],
             0.0f);
        Set4(prog, "airMedium", m.hazeSigma, m.valleyMist, m.beamMist, rm::kBeamMistHeight);
        Set4(prog, "airMarch", rm::kBeamDistance, static_cast<float>(rm::kBeamSteps),
             vis ? static_cast<float>(rm::kHazeVisSteps) : 0.0f, f.noiseShift);
        Set4(prog, "airSun", sunBeams, m.beamAir, rm::kBeamG, rm::kHazeNearDistance);
        Set4(prog, "airSpots", static_cast<float>(spots), static_cast<float>(rm::kAirSpotSteps), 0.0f, 0.0f);
        if (spots > 0)
        {
            const GLint lp = glGetUniformLocation(prog, "airSpotPos");
            const GLint ld = glGetUniformLocation(prog, "airSpotDir");
            if (lp >= 0)
                glUniform4fv(lp, spots, spotPos);
            if (ld >= 0)
                glUniform4fv(ld, spots, spotDir);
        }
        DrawTo(p.airMarchFbo, f.hw, f.hh);
        // The post pass's unit 2: no sampler object (BeginPostFullscreen).
        glBindSampler(kShadowMapUnit, 0);

        glUseProgram(p.progAirBlur);
        SetInt(p.progAirBlur, "airTex", 0);
        SetInt(p.progAirBlur, "airDepthTex", 3);
        BindTex(3, p.airMistTex);
        BindTex(0, p.airTex[0]);
        Set2(p.progAirBlur, "stepUV", 1.0f / f.hw, 0.0f);
        DrawTo(p.airFbo[1], f.hw, f.hh);
        BindTex(0, p.airTex[1]);
        Set2(p.progAirBlur, "stepUV", 0.0f, 1.0f / f.hh);
        DrawTo(p.airFbo[0], f.hw, f.hh);

        if (beams)
        {
            glUseProgram(p.progAirMean);
            SetInt(p.progAirMean, "airTex", 0);
            SetInt(p.progAirMean, "airDepthTex", 3);
            BindTex(3, p.airMistTex);
            BindTex(0, p.airTex[0]);
            Set4(p.progAirMean, "meanParams", kAirMeanRadius, static_cast<float>(f.w) / static_cast<float>(f.h), 0.0f,
                 0.0f);
            DrawTo(p.airMeanFbo, f.hw, f.hh);
        }
    }

    // The apply, full resolution.
    const GLuint prog = p.progAirApply;
    glUseProgram(prog);
    SetInt(prog, "colorTex", 0);
    SetInt(prog, "airTex", 2);
    SetInt(prog, "airMistTex", 3);
    SetInt(prog, "airMeanTex", 4);
    SetInt(prog, "depthTex", 5);
    SetInt(prog, "skyLut", kSkyLutUnit);
    BindTex(0, f.frameColor);
    BindTex(2, march ? p.airTex[0] : p.blackTex);
    BindTex(3, march ? p.airMistTex : p.blackTex);
    BindTex(4, beams ? p.airMeanTex : p.blackTex);
    BindTex(5, p.depthTex);
    // (The raw binds above left the bind cache's active unit stale: without
    // this the sky table could land on unit 5, the depth's.)
    GL33Bind::Invalidate();
    BindSkyTable(); // unit 8: the table the world read, or its stand-in
    Set4(prog, "fogColor", _fogColor.R(), _fogColor.G(), _fogColor.B(), 1.0f);
    Set4(prog, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
    Set4(prog, "depthMap", f.depthScale, f.depthBias, 0.0f, 0.0f);
    Set4(prog, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    SetMat3(prog, "viewToWorld", viewToWorld);
    Set4(prog, "airHalf", static_cast<float>(f.hw), static_cast<float>(f.hh), march ? 1.0f : 0.0f,
         _lighting.debugView == rm::kDebugViewAir ? 1.0f : 0.0f);
    Set4(prog, "airMedium", m.hazeSigma, m.valleyMist, rm::kHazeNearDistance, kAirMistFullFrom);
    Set4(prog, "airBeams", beams ? 1.0f : 0.0f, m.beamHighpass, m.beamGain, 1.0f);
    const float exposure = _lightRigExposure;
    Set4(prog, "airSunColor", _lightRigSun.R() * exposure, _lightRigSun.G() * exposure, _lightRigSun.B() * exposure,
         0.0f);
    const float fogRange = m.fogEnd - m.fogStart;
    Set4(prog, "airFog", m.fogStart, fogRange > 0.0f ? 1.0f / fogRange : 0.0f, fogRange > 0.0f ? 1.0f : 0.0f, 0.0f);
    // The photoreal camera's air (read only while litCamera.x is on).
    Set4(prog, "airCamera", _lighting.camAerial, rm::kCameraNearHazeCap, rm::kCameraSkyBeamsBySun,
         rm::kCameraBeamFullDistance);
    if (tintLum > 1e-6f)
        Set4(prog, "airSpotTint", tint[0] / tintLum, tint[1] / tintLum, tint[2] / tintLum, 0.0f);
    else
        Set4(prog, "airSpotTint", 1.0f, 1.0f, 1.0f, 0.0f);
    DrawTo(p.airOutFbo, f.w, f.h);
    glActiveTexture(GL_TEXTURE0);
}
