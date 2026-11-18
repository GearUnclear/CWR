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
#include <Poseidon/Graphics/Remaster/SeaFlow.hpp>
#include <Poseidon/Graphics/Remaster/SeaWaves.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>
#include <Poseidon/World/Terrain/LandscapeShared.hpp>

#include <algorithm>
#include <cmath>

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
    // XP sea flow: the other frame is a wave layer of its own, at any share.
    const bool flow = rm::SeaFlowXpOn();
    TextureGL33* other = (otherWeight > 0.0f || flow) ? static_cast<TextureGL33*>(otherFrame) : nullptr;
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
    // XP sea mirror: the sea's draws also write their wave slope (off: nothing).
    SeaWaveBegin();
}

void EngineGL33::EndSea()
{
    if (!_seaOpen)
        return;
    // The sea's draws commit with the water program.
    FlushQueues();
    SeaWaveEnd();
    _seaOpen = false;
    // The shore ribbons lie on the sea and the sand just drawn.
    DrawShoreRibbons();
    GL33Bind::Tex2D(kSeaNextFrameUnit, _post.whiteTex);
    glBindSampler(kSeaNextFrameUnit, 0);
    GL33Bind::ActiveUnit(0);
}

// ---- Shore water (the Graphics page's option, first POSEIDON_XP_WATER) ------
//
// The sea program knows the ground under each water fragment (the terrain
// height map on unit 4, read on the landscape's own triangles), so it knows the
// water's depth there. While the option is on, the parts mask
// (_shoreWaterParts, SetLookOptions) picks:
//   1 shallow water: the sea floor (the ground field's colour, wet) shows
//     through water a few metres deep, dimmed per channel by the water's
//     absorption along the refracted path, and the waves calm at the waterline;
//   2 surf: a broken foam fringe along the waterline (it follows the drawn
//     sea level's wave) and the waves: painted by the sea and land
//     programs (waves rolling in, their run-up on the sand), or with the
//     Surf option at Ribbons drawn by the shore ribbons;
//   4 wet sand: the land darkens below the swash line (the tide plus the
//     highest wave), on the land program;
//   8 wave masking: a wave facet whose mirror ray points below the horizon
//     sees the next wave instead of the horizon sky (the open sea too).
//  16 (set here, not an option part) the shore ribbons draw this frame: the
//     sea and land programs leave the waves to them (the fringe stays).
// The option runs 1, 2 and 4. POSEIDON_XP_WATER (0 off, else on) overrides the
// option and POSEIDON_XP_WATER_PARTS=<mask> picks parts (15 adds the wave
// masking; Graphics/Remaster/LookOptions.hpp). Off, litWater2 stays zero and
// every fragment is as it was before the option.
void EngineGL33::UploadLitSeaShore()
{
    const int parts = _shoreWaterParts;
    Landscape* land = GScene ? GScene->GetLandscape() : nullptr;
    const bool on = parts != 0 && land && _remasterPrograms && !_nightVision &&
                    rm::FeatureOn(_lighting.features, rm::Feature::SeaWaves) && _featureGates.computed &&
                    _featureGates[rm::Feature::SeaWaves].on;
    if (!on)
    {
        // XP sea flow: the water program's clock runs without the shore too
        // (y is read only by the shore's foam, which parts 0 leaves out).
        const float clock = rm::SeaFlowXpOn() ? std::fmod(Glob.time.toFloat(), rm::kSeaFlowTimeWrap) : 0.0f;
        SetLitSlot(LitConst::Water2, 0.0f, clock, 0.0f, 0.0f);
        return;
    }
    static_assert(rm::kSeaFlowTimeWrap == 4096.0f, "the sea flow's rates are whole numbers of 1 / this clock's wrap");
    // The swash line: the tide's level plus the highest the drawn sea level's
    // wave reaches (Landscape::Simulate; XP sea flow draws a share of it), and
    // a little run-up above it.
    const float swash = land->GetSeaLevelNoWave() + maxWave * rm::SeaFlowXpHeave() + 0.15f;
    // 16: the shore ribbons draw the waves (EngineGL33_ShoreRibbons.cpp), so
    // the sea and land programs leave out their own painted surf.
    const int drawn = parts | (ShoreRibbonsActive() ? 16 : 0);
    SetLitSlot(LitConst::Water2, static_cast<float>(drawn), std::fmod(Glob.time.toFloat(), 4096.0f),
               land->GetSeaLevelDrawn(), swash);
}
