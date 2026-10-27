#pragma once

#include <Poseidon/Graphics/Core/GpuTimings.hpp>

#include <cstdint>

// The remastered post pass as a fixed list of stages (INTEGRATION-PLAN.md
// section 3.6). EngineGL33::EndWorldScene runs the stages that a frame's plan
// names, in this order, on one PostFrame (EngineGL33_PostFX.cpp); each stage
// has its own GPU mark and section (Graphics/Core/GpuTimings.hpp). Pure, no
// GL: which stages run in a frame, and at what strength, is a function of the
// inputs below, so the early return and every stage's gate can be tested.
//
// The list is fixed. A later step lands its stage by giving it a pass and
// setting `landed` (A5 the TAA resolve and sharpen, B4 the combat scatter, B6
// the tag readout and the reflections, B8 the air pass), never by reordering
// the list. A stage that has not landed holds its place, mark and section and
// never runs. A stage that samples PostLinearDepth's output sets
// readsLinearDepth, and the planner then runs LinearDepth before it.

namespace Poseidon::remaster
{

enum class PostStage : std::uint8_t
{
    Resolve,       // colour, depth and stencil out of the multisampled frame target
    LinearDepth,   // half-resolution view depth, for the stages that read it
    TagReadout,    // SSR stencil tags (B6, only with ssr gated on)
    Ao,            // ambient occlusion and its blur
    Ssr,           // screen-space reflections (B6; before bloom, so reflected highlights bloom)
    Air,           // the volumetric fog and atmosphere (look-fog, superseding B8's air pass; replaces Shafts when on)
    Bloom,         // bloom chain
    Shafts,        // the legacy sun shafts
    CombatScatter, // combat light scattered in smoke and dust (B4)
    Composite,     // occlusion, bloom and shafts into the frame target
    Temporal,      // TAA resolve (A5)
    Sharpen,       // TAA sharpen or, with TAA off, the luma sharpen (A5, B11); never both
    DebugView,     // a diagnostic view of a post buffer over the frame (debug=5: the linear depth)
    Count
};
constexpr int kPostStageCount = static_cast<int>(PostStage::Count);

struct PostStageInfo
{
    PostStage id;
    const char* name; // its GPU section's name (GpuTimings), also its name in logs
    const char* step; // integration step that gives the stage its pass
    // False while the stage has no pass: it never runs. The step that lands it
    // sets it to true.
    bool landed;
    // The stage samples the half-resolution linear depth: LinearDepth runs
    // whenever it does.
    bool readsLinearDepth;
    gpu::Mark endMark;    // timestamp after the stage
    gpu::Section section; // from the end of the stage that ran before it to endMark
};

const PostStageInfo& GetPostStageInfo(PostStage s);

// LightingTuning::debugView value whose view the DebugView stage draws over the
// frame: the half-resolution linear depth (0 is the final image, 1 to 3 views
// of the remastered shaders, 4 the occlusion in the composite).
constexpr int kDebugViewLinearDepth = 5;
// The air pass's in-scatter alone over black (haze, mist and beams; B8). Its
// own stage draws it: the view shows only where the air pass runs. After the
// light tiles (6, FrameLights.hpp) and the albedo (7, LightRig.hpp).
constexpr int kDebugViewAir = 8;
// The shafts run only above this on-screen weight of the sun.
constexpr float kShaftMinWeight = 0.001f;

// What decides a frame's stages; the backend fills it once per post pass.
struct PostFrameInputs
{
    // The post knobs (LightingTuning ambientOcclusion, bloom, sunShafts).
    float ambientOcclusion = 0.0f;
    float bloom = 0.0f;
    float sunShafts = 0.0f;
    int debugView = 0;
    // The world depth is there to reconstruct from: false in a vehicle
    // interior, which clears it and draws with its own near plane.
    bool worldDepth = true;
    // No screen overlay was drawn inside the world pass (gunner optics): glow
    // would grey its black surround.
    bool glow = true;
    // The sun's weight on screen for the shafts: 0 when it is behind the
    // camera or under the horizon, scaled down by cloud and night.
    float shaftWeight = 0.0f;
    // Temporal anti-aliasing: the taa switch and its frame gate were on when
    // the world's passes began, so they were drawn with the sub-pixel jitter
    // (TemporalAA.hpp). The resolve and its sharpen then run unless this frame
    // turned out to be a vehicle interior or to carry a screen overlay.
    bool temporal = false;
    // The luma sharpen (B11): the sharpen switch and its frame gate are on (so
    // TAA is not running this frame) and its strength is above 0. The Sharpen
    // stage then runs alone, on the composited frame.
    bool lumaSharpen = false;
    // The air pass (B8): one of airHaze, valleyMist or airBeams and its frame
    // gate are on. It asks for the pass by itself and runs where the world
    // depth is there and no screen overlay was drawn (the haze would grey an
    // optics mask's black surround).
    bool air = false;
    // The air pass draws sun beams this frame (airBeams and its gate on):
    // they replace the legacy shafts.
    bool airBeams = false;
    // Screen-space reflections (B6, ScreenReflections.hpp): the world's passes
    // wrote the surface tags into the stencil (the ssr switch is on), and the
    // switch's frame gate is on (water or wet ground on screen).
    bool ssrTags = false;
    bool ssr = false;
    // The combat shadow mask (B4): its switch and frame gate are on and combat
    // light lit this frame. The next frame's mask traces through this frame's
    // half-resolution linear depth, so LinearDepth runs (it is not a stage of
    // its own: the mask pass runs at the start of the next world).
    bool combatMask = false;
    // The surfaces lane (SurfaceLight.hpp) wrote a vehicle interior's shares
    // into its surface target: the interior's own depth (its own near plane)
    // is what the occlusion reads there, so the Ao stage runs without the
    // world depth (the corners and the breech of a crew compartment darken).
    bool surfaceInterior = false;
    // The photoreal camera (photoLight, PhotoCamera.hpp): the world was drawn
    // as scene light this frame, and the composite is the camera that turns
    // it into the picture, so it asks for the pass by itself.
    bool camera = false;
    // The volumetric fog (FeatureSet::volFog and its frame gate, look-fog):
    // the Air stage applies the fog volume to every pixel by its world depth
    // in place of B8's air pass (whose gates are off while it runs) and
    // replaces the legacy shafts (its sun light in the air is the beams). It
    // needs the world depth (or fogWorldDepth); a screen overlay keeps it
    // (the apply leaves the overlay's mask as drawn). Held off, the frame
    // keeps the forward fog's classic edge (VolumetricFog.hpp).
    bool fog = false;
    // A vehicle interior cleared the world depth, and the world's depth and
    // colour were saved just before (EngineGL33::Clear): the fog runs on
    // them and leaves the interior as drawn, so the air does not vanish when
    // the player looks out of a cockpit. Only the fog reads the saved depth;
    // the other stages keep holding off as with no world depth.
    bool fogWorldDepth = false;
};

// A frame's stages: which run and the strengths the composite uses.
struct PostStagePlan
{
    std::uint32_t runs = 0; // bit per PostStage
    // Occlusion and bloom strength in the composite: the knob, or 0 when the
    // frame holds the stage off (no world depth, a screen overlay).
    float aoStrength = 0.0f;
    float bloomStrength = 0.0f;
    // The Air stage runs the volumetric fog (look-fog), not B8's air pass.
    bool fog = false;
    bool Runs(PostStage s) const { return ((runs >> static_cast<int>(s)) & 1u) != 0; }
    // No stage asked for the pass: the frame stays as drawn.
    bool Empty() const { return runs == 0; }
};

// The early return asks every stage: the pass runs when a landed stage asks
// for it (its knob is on, its switch and frame gate are on, or its debug view
// is selected), and then Resolve and Composite always run, so a stage that is
// on but held off this frame (AO in a vehicle interior) still gets the
// composite, which copies the resolved frame back as it always has. Each other
// stage runs when its own gate says so. Temporal and Sharpen run together: the
// composite then draws into the TAA input instead of the frame target, and
// the sharpen writes the resolved frame back into every sample of it. With TAA
// off the luma sharpen asks for the pass and runs the Sharpen stage alone, on
// the composite's output in the same TAA input; never both sharpens. The air
// pass asks by itself too; when it draws beams the legacy shafts hold off.
// Tags the world wrote ask for the pass too (TagReadout measures the screen
// shares the reflections' gate reads, so the gate can open again); Ssr runs
// only on a frame that kept its world depth and drew no screen overlay, unless
// one of its diagnostic views is selected.
PostStagePlan PlanPostStages(const PostFrameInputs& in);

// The forward fog's linear factor, (end - d) * invRange clamped to 0..1, as
// the classic programs compute it from its start and 1 / (end - start).
float ForwardFogFactor(float distance, float start, float invRange);

// The forward fog under night vision. The intensified frame is drawn by the
// classic programs, which have no litHaze.x; while the volumetric fog draws,
// the remastered programs keep only the forward fog's edge by scaling its
// factor (min(factor * scale, 1), scale = FogMedium::fogEdgeScale). The same
// edge in the classic programs' terms is the linear factor with the same end
// and a range `scale` times shorter, so the intensified frame and AI sight
// both get the volume's air plus that one edge, and the weather fog is not
// counted twice. scale <= 1 or an empty range returns the inputs.
struct ForwardFogRange
{
    float start = 0.0f;
    float invRange = 0.0f;
};
ForwardFogRange ScaleForwardFogRange(float start, float invRange, float scale);

// A vehicle interior is drawn over the world the fog saved; the fog's apply
// leaves every pixel the interior wrote depth to as drawn. Its glass writes
// depth too (the cockpit models rely on it to hide what lies behind the
// canopy), so the world seen through a canopy or a windscreen would stay
// unfogged while AI sight is cut by the air out there. A blended draw of glass
// (by its surface class) sets the stencil bit kInteriorGlassStencil where it
// is drawn; every other interior draw sets kInteriorSolidStencil and clears
// the glass bit. After the resolve the pixels with the glass bit and no solid
// bit (the world seen through glass, nothing of the vehicle behind it) get
// the far depth, and the apply fogs them by the world's depth. The interior
// cleared the stencil with its depth, the projected shadows keep the low
// bits, and no interior frame reads the reflections' tags.
constexpr unsigned kInteriorGlassStencil = 0x80u;
constexpr unsigned kInteriorSolidStencil = 0x40u;
bool InteriorGlassDraw(bool blended, bool glassClass);

} // namespace Poseidon::remaster
