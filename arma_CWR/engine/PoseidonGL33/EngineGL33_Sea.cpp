// Remastered sea: the feature switch `seaWaves` (the shading model is in
// Graphics/Remaster/SeaWaves.hpp and the water program s_psWaterRemasterMain).
//
// The sea is the landscape's own faces (Landscape::DrawWater), which carry the
// specular layer alone and so drew through the detail program (PSDetail) like
// the terrain. The landscape now opens the sea's draws with BeginSea and closes
// them with EndSea. In between, with the switch on and gated on in the
// remastered world pass, the descriptor routes those faces to the water
// program (BuildContext::remasteredSea), the animation frame the faces do not
// carry is bound on kSeaNextFrameUnit for the cross-fade, and the water slots
// of LitConstants (Water0, Water1) hold this frame's constants.
//
// Off (the switch, Classic, a portrait, a 3D preview after the world, night
// vision) the scope does nothing: the sea draws through the detail program as
// before the feature, and the water slots are not read.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <PoseidonGL33/TextureGL33.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/SeaWaves.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>
#include <Poseidon/World/Terrain/LandscapeShared.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>

using namespace Poseidon::gl33lighting;
namespace rm = Poseidon::remaster;

void EngineGL33::BeginSea(Texture* otherFrame, float otherWeight)
{
    if (_seaOpen)
        EndSea();
    if (!_glContext || !_remasterPrograms || !GScene || !GScene->MainLight() || _portraitTargetSize ||
        _worldSceneEnded || _nightVision || !rm::FeatureOn(_lighting.features, rm::Feature::SeaWaves))
        return;
    // The gates of this frame (computed at the world's first 3D pass).
    if (!_featureGates.computed || !_featureGates[rm::Feature::SeaWaves].on)
        return;
    // Draws queued so far keep their programs.
    FlushQueues();

    // The frame to cross-fade into, at its finest level (the faces' own frame
    // is loaded for the nearest sea; every frame comes round every 2 s). Until
    // it has a texture, the faces' frame shows alone.
    TextureGL33* other = otherWeight > 0.0f ? static_cast<TextureGL33*>(otherFrame) : nullptr;
    unsigned int handle = 0;
    if (other && _textBank)
    {
        _textBank->UseMipmap(other, 0, 0);
        handle = other->GetHandle();
    }
    const rm::SeaFrameBlend blend{0, 0, handle ? otherWeight : 0.0f};
    GL33Bind::Tex2D(kSeaNextFrameUnit, handle ? handle : _post.whiteTex);
    // The faces' frame samples through unit 0's sampler object (linear, wrap,
    // anisotropic, the TAA texture LOD bias): the other frame gets the same.
    glBindSampler(kSeaNextFrameUnit, MaterialSampler(0));
    GL33Bind::ActiveUnit(0);

    LightSun* sun = GScene->MainLight();
    const float toSunY = -sun->Direction().Y();
    const float sunElevDeg = std::asin(std::max(-1.0f, std::min(1.0f, toSunY))) * 57.29578f;
    const rm::SeaWaveConstants c = rm::ComputeSeaWaveConstants(sunElevDeg, _lighting, blend);
    const Color classic = sun->Diffuse();
    const rm::Rgb sheen = rm::SeaSheenLight(rm::Rgb(_lightRigSun.R(), _lightRigSun.G(), _lightRigSun.B()),
                                            rm::Rgb(classic.R(), classic.G(), classic.B()), c.sheen);
    SetLitSlot(LitConst::Water0, c.relief, c.crossFade, c.skyMean, c.skyTint);
    SetLitSlot(LitConst::Water1, sheen.r, sheen.g, sheen.b, c.sheenFlat);
    FlushLitConstants();
    _seaOpen = true;
}

void EngineGL33::EndSea()
{
    if (!_seaOpen)
        return;
    // The sea's draws commit with the water program.
    FlushQueues();
    _seaOpen = false;
    GL33Bind::Tex2D(kSeaNextFrameUnit, _post.whiteTex);
    glBindSampler(kSeaNextFrameUnit, 0);
    GL33Bind::ActiveUnit(0);
}

// ---- Experiment POSEIDON_XP_WATER: the shore ---------------------------------
//
// The sea program knows the ground under each water fragment (the terrain
// height map on unit 4, read on the landscape's own triangles), so it knows the
// water's depth there. With the switch on:
//   1 shallow water: the sea floor (the ground field's colour, wet) shows
//     through water a few metres deep, dimmed per channel by the water's
//     absorption along the refracted path, and the waves calm at the waterline;
//   2 surf: a broken foam fringe along the waterline (it follows the sea
//     level's own 12.5 s wave) and foam lines rolling in over the shallows;
//   4 wet sand: the land darkens below the swash line (the tide plus the
//     highest wave), on the land program;
//   8 wave masking: a wave facet whose mirror ray points below the horizon
//     sees the next wave instead of the horizon sky (the open sea too).
// POSEIDON_XP_WATER=1 turns on the shore (1, 2, 4), the look the lane
// recommends; POSEIDON_XP_WATER_PARTS=<mask> picks parts (15 adds the wave
// masking). Off, litWater2 stays zero and every fragment is as before.
namespace
{
int XpWaterParts()
{
    static const int parts = []
    {
        const char* on = std::getenv("POSEIDON_XP_WATER");
        if (!on || !*on || std::atoi(on) == 0)
            return 0;
        if (const char* m = std::getenv("POSEIDON_XP_WATER_PARTS"))
            return std::atoi(m) & 15;
        return 7;
    }();
    return parts;
}
} // namespace

void EngineGL33::UploadLitSeaShore()
{
    const int parts = XpWaterParts();
    Landscape* land = GScene ? GScene->GetLandscape() : nullptr;
    const bool on = parts != 0 && land && _remasterPrograms && !_nightVision &&
                    rm::FeatureOn(_lighting.features, rm::Feature::SeaWaves) && _featureGates.computed &&
                    _featureGates[rm::Feature::SeaWaves].on;
    if (!on)
    {
        SetLitSlot(LitConst::Water2, 0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }
    // The swash line: the tide's level plus the highest the sea level's wave
    // reaches (Landscape::Simulate), and a little run-up above it.
    const float swash = land->GetSeaLevelNoWave() + maxWave + 0.15f;
    SetLitSlot(LitConst::Water2, static_cast<float>(parts), std::fmod(Glob.time.toFloat(), 4096.0f),
               land->GetSeaLevel(), swash);
}
