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
    glBindSampler(kSeaNextFrameUnit, _samplerObjects[0]);
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
