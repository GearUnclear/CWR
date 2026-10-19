#ifdef _MSC_VER
#pragma once
#endif

#ifndef __ENGINE_GL33_HPP
#define __ENGINE_GL33_HPP

#include <Poseidon/Core/Types.hpp>

using namespace Poseidon;
class TextureGL33;
class TextBankGL33;

#include <Poseidon/Graphics/Core/MatrixConversion.hpp>
#include <Poseidon/Graphics/Core/RenderState.hpp>

// GL33 has no D3D profiling scopes; the macro is a no-op here.
#define PROFILE_DX_SCOPE(name)

#include <vector>
#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Containers/StaticArray.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/Graphics/Core/TLVertex.hpp>
#include <Poseidon/Graphics/Rendering/RenderPassDescriptor.hpp>
#include <PoseidonGL33/SDLEventWindow.hpp>

enum PixelShaderSpecular
{
    PSSSpecular,
    PSSNormal,
    NPixelShaderSpecular
};

#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <Poseidon/Graphics/Rendering/Lighting/CombatLights.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/Lights.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/FrameLights.hpp>
#include <Poseidon/Graphics/Core/GpuTimings.hpp>
#include <Poseidon/Graphics/Remaster/AirMedium.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <Poseidon/Graphics/Remaster/PhotoCamera.hpp>
#include <Poseidon/Graphics/Remaster/PostStages.hpp>
#include <Poseidon/Graphics/Remaster/ScreenReflections.hpp>
#include <Poseidon/Graphics/Remaster/SurfaceMaterials.hpp>
#include <Poseidon/Graphics/Remaster/SurfaceLight.hpp>
#include <Poseidon/Graphics/Remaster/TemporalAA.hpp>
#include <Poseidon/Graphics/Remaster/TerrainHorizon.hpp>
#include <Poseidon/Graphics/Remaster/Wetness.hpp>
#include <Poseidon/Graphics/Remaster/VolumetricFog.hpp>

#include <memory>

typedef int ZFyzType;
#define zToFyz(z) toInt(z * 0x10000000)
#define MAX_FYZ_Z 0x7fffffff

enum TexLoc
{
    TexLocalVidMem,
    TexNonlocalVidMem,
    TexSysMem
};

enum
{
    MeshBufferLength = 32 * 1024,
    IndexBufferLength = 4 * 1024
};

enum PixelShaderMode
{
    PSMDay,
    PSMNight,
    NPixelShaderModes
};

enum PixelShaderID
{
    PSNormal,
    PSDetail,
    PSGrass,
    PSWater,
    PSFlat,
    PSShadow, // unlit cutout: constant black + alpha
    NPixelShaders,
    PSNone = NPixelShaders
};

struct alignas(16) PSConstants
{
    enum Slot : int
    {
        SlotFogColor = 0,
        SlotAlphaRef = 1,
        SlotConstColor = 3, // per-object IsColored tint
        SlotLightDir = 4,
        SlotGrassCoef1 = 5,
        SlotGrassCoef2 = 6,
        SlotRgbEyeCoef = 7
    };

    float fogColor[4] = {0, 0, 0, 1};
    float alphaRef[4] = {0, 0, 0, 0};
    float lightDir[4] = {0, 0, 0, 0};
    float grassCoef1[4] = {0, 0, 0, 0};
    float grassCoef2[4] = {0, 0, 0, 0};
    float rgbEyeCoef[4] = {0, 0, 0, 1};
    // Per-object IsColored tint, white = no-op.  Uploaded to SlotConstColor (3);
    // kept last so the other fields keep their tested offsets (test_gl33_rendering).
    float constColor[4] = {1, 1, 1, 1};
};

enum VertexShaderID
{
    VSScreen,
    VSTransform,
    VSShadow, // unlit transform, shadow path
    NVertexShaders,
    VSNone = NVertexShaders
};

namespace VSConst
{
// VS UBO layout — each slot is one vec4 (16 bytes).  Distinct
// uniforms occupy disjoint byte ranges (I-01).  vpScale lives at
// slot 21, disjoint from SlotProj (0), so VSScreen draws can never
// clobber VSTransform's projection matrix and vice versa (B-001).
enum : int
{
    SlotProj = 0,  // 4 vec4s
    SlotView = 4,  // 4 vec4s
    SlotWorld = 8, // 4 vec4s
    SlotSunDir = 12,
    SlotAmbient = 13,
    SlotDiffuse = 14,
    SlotEmissive = 15,
    SlotFogParam = 16,
    SlotCamPos = 17,
    SlotSpecular = 18,
    SlotSpecEn = 19,
    SlotSunEn = 20,
    SlotVpScale = 21,
    // Remastered lighting: the crown ellipsoid of the shape being drawn
    // (xyz centre, w valid; xyz inverse half extents). Unused by the classic VS.
    SlotCrown = 22,
    SlotCrownScale = 23,
    SlotTexMat0 = 24, // 4 vec4s
    SlotTexMat1 = 28, // 4 vec4s
    SlotTexCtrl = 32,
    // Local (point/spot) lights for night per-vertex illumination.
    SlotLightCount = 33,   // .x = active local light count
    SlotLightPos = 34,     // MaxLocalLights vec4: xyz world pos, w = startAtten
    SlotLightDiffuse = 42, // MaxLocalLights vec4: diffuse * nightEffect
    SlotLightAmbient = 50, // MaxLocalLights vec4: ambient * nightEffect
    SlotLightDir = 58,     // MaxLocalLights vec4: xyz beam dir (world), w = isSpot
    SlotLightVP = 66,      // 4 vec4s: light view-projection for shadow-map sampling
};

// Per-draw cap on local lights folded into the vertex shader.
static constexpr int MaxLocalLights = 8;

// I-01 tier 1: no two named ranges overlap.  Update this list
// when adding a new register.
static_assert(SlotView >= SlotProj + 4, "SlotView overlaps SlotProj");
static_assert(SlotWorld >= SlotView + 4, "SlotWorld overlaps SlotView");
static_assert(SlotSunDir >= SlotWorld + 4, "SlotSunDir overlaps SlotWorld");
static_assert(SlotVpScale >= SlotSunEn + 1, "SlotVpScale overlaps SlotSunEn");
static_assert(SlotCrown >= SlotVpScale + 1, "SlotCrown overlaps SlotVpScale");
static_assert(SlotCrownScale >= SlotCrown + 1, "SlotCrownScale overlaps SlotCrown");
static_assert(SlotTexMat0 >= SlotCrownScale + 1, "SlotTexMat0 overlaps SlotCrownScale");
static_assert(SlotTexMat1 >= SlotTexMat0 + 4, "SlotTexMat1 overlaps SlotTexMat0");
static_assert(SlotTexCtrl >= SlotTexMat1 + 4, "SlotTexCtrl overlaps SlotTexMat1");
static_assert(SlotLightCount >= SlotTexCtrl + 1, "SlotLightCount overlaps SlotTexCtrl");
static_assert(SlotLightPos >= SlotLightCount + 1, "SlotLightPos overlaps SlotLightCount");
static_assert(SlotLightDiffuse >= SlotLightPos + MaxLocalLights, "SlotLightDiffuse overlaps SlotLightPos");
static_assert(SlotLightAmbient >= SlotLightDiffuse + MaxLocalLights, "SlotLightAmbient overlaps SlotLightDiffuse");
static_assert(SlotLightDir >= SlotLightAmbient + MaxLocalLights, "SlotLightDir overlaps SlotLightAmbient");
static_assert(SlotLightVP >= SlotLightDir + MaxLocalLights, "SlotLightVP overlaps SlotLightDir");
}; // namespace VSConst

struct TriQueue
{
    StaticArray<WORD> _triangleQueue;

    TextureGL33* _texture;
    int _level;
    int _special;
    Poseidon::PassId _passId = Poseidon::PassId::Opaque;
    int _lastUsed;
};

enum
{
    MaxTriQueues = 32,
    TriQueueSize = 2048
};

struct QueueGL33
{
    int _vertexBufferUsed;
    int _indexBufferUsed;

    int _meshBase, _meshSize;

    TriQueue _tri[MaxTriQueues];
    bool _triUsed[MaxTriQueues];
    int _actTri;

    int _usedCounter;
    bool _firstVertex;
    bool _firstIndex;

    QueueGL33();
    int Allocate(TextureGL33* tex, int level, int spec, int minI, int maxI, int tip);
    void Free(int i);
};

struct SVertex
{
    Vector3P pos;
    Vector3P norm;
    Poseidon::UVPair t0;
};

// Shader override directory — when set (CWR_SHADER_OVERRIDE_DIR, read in
// Init3DState), GL33's CompileGLShader prefers `<dir>/<name>.glsl` over the
// inline source.  Empty = inline only.
void SetShaderOverrideDir(const std::string& dir);
const std::string& ShaderOverrideDir();
// Contents of `<override dir>/<name>.glsl`, if the directory is set and the
// file exists (the post pass compiles its shaders through this too).
bool LoadShaderOverride(const char* name, std::string& out);

class EngineGL33 : public Engine
{
    typedef Engine base;

  protected:
    int _portraitTargetSize = 0;
    int _w = 0, _h = 0; // back buffer dimensions
    bool _resetNeeded = false;
    TLVertexTable* _mesh = nullptr; // mesh data used during rendering

    enum RenderMode
    {
        RMLines,
        RMTris,
        RM2DLines,
        RM2DTris
    };
    RenderMode _renderMode = RMTris;

    bool _sunEnabled = false;
    Poseidon::TLMaterial _materialSet;
    int _materialSetSpec = 0;
    // Signature of the LightList last uploaded by DoSetMaterial. SetMaterial's
    // cache must re-upload when the lights change, not only when the material
    // changes — otherwise a draw sharing a material with a prior lamp-less draw
    // reuses its empty light list and renders unlit (black road under a lamp).
    uint64_t _materialSetLightsSig = 0;
#ifndef NDEBUG
    // Debug tripwire: signature of the frame-constant lighting inputs DoSetMaterial
    // folds in but leaves OUT of the per-draw cache key. Asserts on a cache hit
    // that they are unchanged — catches a cache that outlived its frame (a future
    // omitted-input bug like the black-road one, in the cross-frame direction).
    uint64_t _materialFrameInputsSig = 0;
#endif

    RString _pendingScreenshotPath;
    // The next decal's light past its colour (SetDecalGlow), cleared by it.
    PackedColor _decalGlow = PackedColor(0u);

    void DrawDecal(Vector3Par screen, float rhw, float sizeX, float sizeY, PackedColor color,
                   const Poseidon::MipInfo& mip, int specFlags) override;
    void DrawPolygon(const VertexIndex* ii, int n) override;
    void DrawSection(const FaceArray& face, Offset beg, Offset end) override;
    void DrawPoints(const TLVertex* vs, int nVertex);
    void DrawPoints(int beg, int end) override;
    bool CanGrass() const override;

    void Draw2D(const Poseidon::Draw2DPars& pars, const Poseidon::Rect2DAbs& rect,
                const Poseidon::Rect2DAbs& clip) override;
    void DrawPoly(const Poseidon::MipInfo& mip, const Poseidon::Vertex2DPixel* vertices, int nVertices,
                  const Poseidon::Rect2DPixel& clip, int specFlags) override;
    void DrawPoly(const Poseidon::MipInfo& mip, const Poseidon::Vertex2DAbs* vertices, int nVertices,
                  const Poseidon::Rect2DAbs& clip, int specFlags) override;
    void DrawLine(const Poseidon::Line2DAbs& line, PackedColor c0, PackedColor c1,
                  const Poseidon::Rect2DAbs& clip) override;
    void DrawLine(int beg, int end) override;

    void DoSetMaterial(const Poseidon::TLMaterial& mat, const LightList& lights,
                       const Poseidon::render::LegacySpec& spec);
    void SetMaterial(const Poseidon::TLMaterial& mat, const LightList& lights,
                     const Poseidon::render::LegacySpec& spec) override;

    void Screenshot(RString filename) override { _pendingScreenshotPath = static_cast<const char*>(filename); }
    void FlushPendingScreenshot() override { CaptureScreenshotIfPending(); }
    bool CapturePortrait(const std::function<void()>& draw, std::vector<uint8_t>& rgb) override;
    bool CanRestore() { return false; }

    void SwitchRenderMode(RenderMode mode)
    {
        if (_renderMode == mode)
            return;
        DoSwitchRenderMode(mode);
    }

    enum TexGenMode
    {
        TGFixed,
        TGNone,
        TGDetail,
        TGGrass,
        TGWater
    };

    enum class BlendMode
    {
        Opaque,
        AlphaBlend,
        Additive,
        Shadow
    };

    enum class DepthMode
    {
        Normal,
        ReadOnly,
        Disabled,
        // Per-poly shadow accumulation: stencil EQUAL 0 / INCR_SAT for
        // within-caster exclusion; per-poly (1-srcA) blend darkens the
        // framebuffer directly.
        Shadow
    };

  protected:
    int _pixelSize;
    int _depthBpp;
    int _refreshRate;
    Poseidon::WindowMode _windowMode = Poseidon::WindowMode::Borderless;
    int _windowedRestoreW = 0;
    int _windowedRestoreH = 0;
    bool _pendingExclusiveEnter = false;

    SDL_Window* _sdlWindow = nullptr;
    SDLEventWindow _eventWindow;

    int _bias;
    float _grassParam[4];
    bool _clipANearEnabled, _clipAFarEnabled;
    Plane _clipANear, _clipAFar;

    int _minGuardX;
    int _maxGuardX;
    int _minGuardY;
    int _maxGuardY;

    bool _windowed;

  protected:
    TextBankGL33* _textBank = nullptr;

  protected:
    // GL context (SDL_GLContext, held as void* to keep SDL out of this header)
    void* _glContext = nullptr;

    int _prepSpec;
    // Most recently bound TEXTURE1 handle.  Tracks SetMultiTexturing's
    // resolved handle across its early-out path so each TL draw's
    // captured DrawItem records the *currently bound* multi-tex even
    // when the format didn't change since the previous draw.
    unsigned int _lastTexture1Handle = 0;
    bool _stencilExclusionEnabled;
    TexGenMode _texGenMode;
    Poseidon::PassId _activePassId = Poseidon::PassId::ScreenSpace;

    int _iOffset;

    QueueGL33* _lastQueueSource;

    // GL objects for dynamic 2D/queue rendering
    unsigned int _vaoScreen = 0; // TLVertex layout (vsScreen: pos,rhw,color,specular,uv0,uv1)
    unsigned int _vaoMesh = 0;   // SVertex layout (vsTransform: pos,normal,uv)
    unsigned int _vbo = 0;
    unsigned int _ibo = 0;

    // 1x1 opaque-white sentinel.  Bound to any sampler that would otherwise
    // reference a P3D face's missing texture.  GLSL has no fixed-function
    // fallback for "no texture"; sampling the GL default texture (name 0)
    // returns undefined data and triggers GL_LOW id=131204.  White makes
    // `tex.rgb * vertColor.rgb` collapse to vertColor — the GLSL equivalent
    // of D3D9 SELECTARG2(DIFFUSE), which is what the original FF pipeline
    // did for untextured faces.
    unsigned int _fallbackWhiteTex = 0;

  public:
    // Dedicated upload texture unit.  All glTexImage2D / glTexSubImage2D /
    // glCompressedTexSubImage2D calls run with this active so the engine's
    // cached binding on unit 1 (_formatSet) remains accurate.  Without this,
    // a demand-load between two draws of
    // the same texture leaves GL bound to the just-uploaded handle while
    // the cache still claims the previous draw's binding — the next
    // `ApplyPassState` skips the rebind ("nothing changed") and the draw
    // samples the wrong texture.  GL 3.3 guarantees ≥16 fragment image
    // units; the engine uses 0 and 1, leaving 2..15 free.
    static constexpr unsigned int kUploadUnit = 0x84C7; // GL_TEXTURE7
  protected:
    bool _lastClampU, _lastClampV;
    bool _pointSampling;
    bool _enableReorder;

    // GL sampler objects: 8 combos of point(4) | clampU(1) | clampV(2)
    unsigned int _samplerObjects[8] = {};
    void CreateSamplerStates();
    void DestroySamplerStates();
    void ApplySamplerState();

    TexLoc _texLoc;

    // Capability constants — fixed for GL 3.3 Core on desktop drivers (every
    // desktop GL 3.3 driver supports them), so they fold at compile time
    // instead of being queried at runtime.
    static constexpr bool _can565 = true;
    static constexpr bool _can88 = false;
    static constexpr bool _can8888 = true;
    static constexpr int _dxtFormats = 0x3E; // DXT1..DXT5
    static constexpr bool _hasStencilBuffer = true;
    static constexpr bool _canDetailTex = true;
    static constexpr bool _canZBias = true;

    // MSAA alpha-to-coverage.  _msaaActive: the default framebuffer actually
    // got multisample buffers (driver may quietly downgrade the request).
    // _alphaToCoverageCfg: the GraphicsConfig knob.  _a2cBound caches the
    // GL_SAMPLE_ALPHA_TO_COVERAGE enable so ApplyPipeline doesn't reissue it
    // per draw.
    bool _msaaActive = false;
    bool _alphaToCoverageCfg = true;
    bool _a2cBound = false;
    bool _a2oneBound = false; // GL_SAMPLE_ALPHA_TO_ONE (remastered programs with alpha-to-coverage)
    bool _debugFlatColor = false;

    // SSAA render-scale state.  _ssaaFbo != 0 == active.  The scaled target is
    // multisampled (4x — combined with the scale that beats the default FB's
    // 8x while keeping VRAM sane); _ssaaResolveFbo is the same-size
    // single-sample stage the MSAA resolve lands in before the downsample
    // blit to the window.
    float _renderScale = 1.0f;
    float _pendingRenderScale = 1.0f;
    int _msaaSamples = 0;
    int _pendingMsaaSamples = 0;
    unsigned int _ssaaFbo = 0;
    unsigned int _ssaaColorRb = 0;
    unsigned int _ssaaDepthRb = 0;
    unsigned int _ssaaResolveFbo = 0;
    unsigned int _ssaaResolveRb = 0;
    int _ssaaW = 0;
    int _ssaaH = 0;

    bool SSAAActive() const { return _ssaaFbo != 0; }
    // Pixel size of the current render target: the scaled offscreen target
    // when SSAA is active, else the window.  Every glViewport/glScissor that
    // addresses the frame target derives from this.
    void RenderTargetSize(int& w, int& h) const;
    void ApplyPendingRenderScale();
    void DestroySSAATarget();
    // Resolve + downsample the scaled target into the default framebuffer and
    // leave it bound for reading.  No-op when SSAA is off.
    void ResolveSSAAToDefault();
    // Bind the frame render target (scaled FBO or default) for drawing and
    // restore its full viewport.
    void BindFrameRenderTarget();

    // Composite the gamma curve onto the finished frame.
    void ApplyGammaPass();
    void DestroyGammaTarget();
    unsigned int _gammaTex = 0;
    unsigned int _gammaVao = 0;
    unsigned int _gammaProgram = 0;
    int _gammaInvGammaLoc = -1;
    int _gammaTexW = 0;
    int _gammaTexH = 0;
    // Latches so an unusable target is not rebuilt every frame.
    bool _gammaUnavailable = false;

    // GL shader programs.  GL33 always uses shaders — there is no
    // fixed-function path to gate against.
    unsigned int _shaderProgram[NVertexShaders][NPixelShaderSpecular][NPixelShaderModes][NPixelShaders];
    PixelShaderID _pixelShaderSel;
    PixelShaderMode _pixelShaderModeSel;
    PixelShaderSpecular _pixelShaderSpecularSel;
    PSConstants _psConstants;

    VertexShaderID _vertexShaderSel = VSNone;
    // _frameState.fogParams[] is the single source of truth for the shader fog
    // uniform; SetShaderFogEnabled mutates it in place so subsequent
    // UploadFrameConstants re-uploads (e.g. from EnableSunLight) preserve
    // whichever fog state the active pass last asked for.
    FrameState _frameState;
    std::vector<DrawItem> _drawItems;
    DrawItem _currentDrawItem;

    float _nightEye;
    int _dbgMeshDrawCalls = 0;
    int _dbgMeshTotalIndices = 0;
    int _dbgAddVerticesCalls = 0;
    int _dbgTotalVertices = 0;
    int _dbgQueueFanCalls = 0;
    int _dbgTotalFanTris = 0;

    QueueGL33 _queueNo;

    // Deferred VBO upload. AddVertices copies into this CPU mirror and bumps
    // _queueNo._vertexBufferUsed; the glBufferSubData is batched once per flush
    // (UploadPendingVertices, called from FlushQueue before each draw) instead
    // of once per primitive — the map background alone submits tens of
    // thousands of tiny polys per frame, and a driver upload each was the
    // dominant cost.  _vboUploadedVerts is the first mirror vertex not yet
    // uploaded to the GL buffer.
    std::vector<TLVertex> _vboMirror;
    int _vboUploadedVerts = 0;

    float _gamma;

    bool _frameOpen;

    Color _textColor;

    enum VFormatSet
    {
        SingleTex,
        DetailTex,
        SpecularTex,
        GrassTex
    };
    VFormatSet _formatSet;

    enum class PipelineVertexInput
    {
        ActivePass,
        Screen,
        Mesh
    };

    // Atomic pipeline bind reading from `Poseidon::render::RenderPassDescriptor` —
    // declares the full backend state for a single draw and forwards to
    // the per-state helpers (`ApplyDepthMode`, `ApplyBlendMode`,
    // `SelectVertexShader`, etc.) which each maintain their own cache.
    // Every caller (the spec-driven `ApplyPassState` path and special-
    // purpose post-processes like the shadow darken pass) builds a
    // `RenderPassDescriptor` and forwards it here.
    void ApplyPipeline(const Poseidon::render::RenderPassDescriptor& d);

  public:
    // Pass-boundary markers — forwarded to GL_KHR_debug if the
    // loader picked up the function pointers (GL 4.3 core or
    // KHR_debug extension).  Guarded for older contexts where the
    // pointer is null (defensive — debug callback init already
    // reports KHR_debug availability).  Defined in EngineGL33.cpp
    // so the gl.h include stays out of public headers.
    void BeginDebugGroup(const char* name) override;
    void EndDebugGroup() override;

    // Reads live GL viewport via glGetIntegerv(GL_VIEWPORT).
    // Implemented in EngineGL33.cpp so the gl.h dependency stays
    // contained.
    bool GetGLViewport(int outRect[4]) const override;

    // The emission seam — issues glBindVertexArray + glDrawElements
    // for the typed Draw.  Implemented in EngineGL33_VertexBuffer.cpp
    // next to its DrawSectionTL caller.
    void EmitDraw(const Poseidon::render::frame::Draw& d) override;

    // --- Properties ---
    RString GetDebugName() const override;
    RString GetRendererName() const override;
    size_t GetDrawItemCount() const override { return _drawItems.size(); }
    const std::vector<DrawItem>* GetRecordedDraws() const override { return &_drawItems; }
    unsigned int GetDebugErrorCount() const override;
    std::string GetLastDebugMessage() const override;
    int SampleBackBufferNonBlack() override;
    bool SamplePixel(int x, int y, uint8_t* outRGB) override;
    int Width() const override { return _w; }
    int Height() const override { return _h; }
    int PixelSize() const override { return _pixelSize; }
    int RefreshRate() const override { return _refreshRate; }
    bool CanBeWindowed() const override { return true; }
    bool IsWindowed() const override { return _windowed; }
    bool IsResizable() const override
    {
        return _sdlWindow && (SDL_GetWindowFlags(_sdlWindow) & SDL_WINDOW_RESIZABLE) != 0;
    }

    int Width2D() const { return _w; }
    int Height2D() const { return _h; }

    int MinGuardX() const override { return _minGuardX; }
    int MaxGuardX() const override { return _maxGuardX; }
    int MinGuardY() const override { return _minGuardY; }
    int MaxGuardY() const override { return _maxGuardY; }

    int MinSatX() const override { return _minGuardX; }
    int MaxSatX() const override { return _maxGuardX; }
    int MinSatY() const override { return _minGuardY; }
    int MaxSatY() const override { return _maxGuardY; }

  protected:
    void WorkToBack();
    void BackToFront();
    void CaptureScreenshotIfPending();

  public:
    EngineGL33(int width, int height, bool windowed, int bpp);
    ~EngineGL33() override;

    bool InitDrawDone() override;
    bool IsAbleToDraw() override;
    bool IsAbleToDrawCheckOnly();
    void InitDraw(bool clear = false, PackedColor color = PackedColor(0)) override;
    void FinishDraw() override;
    void NextFrame() override;
    void DrawTestPattern(const char* name) override;

    void Pause() override;
    void Restore() override;

    void PreReset(bool hard);
    void PostReset();

    bool Reset();
    bool ResetHard() override;
    void ResetForRemount() override; // mod re-mount: drop+rebuild GPU, keep window

    bool SwitchRes(int w, int h, int bpp) override;
    bool SwitchRefreshRate(int refresh) override;
    bool SetWindowMode(Poseidon::WindowMode mode) override;
    Poseidon::WindowMode GetCurrentWindowMode() const override;
    void OnWindowResized(int w, int h) override;
    void OnFullscreenChanged(bool windowed) override;

    void ListResolutions(FindArray<ResolutionInfo>& ret) override;
    void ListRefreshRates(FindArray<int>& ret) override;
    void ListMonitors(FindArray<MonitorInfo>& ret) override;
    int GetCurrentMonitor() const override;
    bool SwitchMonitor(int idx) override;
    bool GetDesktopDisplayMode(int& w, int& h, int& refresh) const override;
    bool GetCurrentDisplayMode(int& w, int& h, int& refresh) const override;
    bool GetRequestedFullscreenMode(int& w, int& h, int& refresh) const override;
    bool SetSwapInterval(int interval) override;
    int GetSwapInterval() const override;
    void StartTextInput() override;
    void StopTextInput() override;
    bool IsTextInputActive() const override;

    void DestroySurfaces();

    TexLoc GetTexLoc() const { return _texLoc; }

    int DXTSupport() const { return _dxtFormats; }
    bool CanDXT(int i) const { return (_dxtFormats & (1 << i)) != 0; }

    bool Can565() const { return _can565; }
    bool Can88() const { return _can88; }
    bool Can8888() const { return _can8888; }

    bool GetHWTL() const { return true; }

    void Clear(bool clearZ = true, bool clear = true, PackedColor color = PackedColor(0)) override;

    VertexBuffer* CreateVertexBuffer(const Shape& src, VBType type) override;
    int CompareBuffers(const Shape& s1, const Shape& s2) override;

    int FrameTime() const;
    int AFrameTime() const override { return FrameTime(); }

    void SetGamma(float gamma) override;
    float GetGamma() const override { return _gamma; }

    // Event-loop hooks (IGraphicsEngine) — forward to the embedded
    // SDLEventWindow helper.
    void HandleEvents() override { _eventWindow.HandleEvents(); }
    bool IsOpen() const override { return _eventWindow.IsOpen(); }
    void SetMouseGrab(bool grab) override { _eventWindow.SetMouseGrab(grab); }
    bool IsMouseGrabbed() const override { return _eventWindow.IsMouseGrabbed(); }

    void EnableReorderQueues(bool enableReorder) override;
    void FlushQueues() override;

    void BeginShadowPass() override;
    void EndShadowPass() override;

    bool ShadowDepthProbe(const float* lightVP16, const float* triXYZ, int vertCount, int res,
                          float* outDepth) override;

    void SetShadowMapsEnabled(bool enabled) override { _shadowTuning.enabled = enabled; }
    // The remastered lighting asked for but not running (its programs failed to
    // build) falls back to Classic, and Classic shadows are the projected ones.
    bool ShadowMapsEnabled() const override
    {
        return _shadowTuning.enabled && (_lightingActive || !_lighting.remastered);
    }

    // --- Remastered lighting (EngineGL33_Lighting.cpp, EngineGL33_LightingShaders.cpp,
    // EngineGL33_PostFX.cpp) ---
    void SetLightingTuning(const Poseidon::remaster::LightingTuning& tuning) override;
    Poseidon::remaster::LightingTuning GetLightingTuning() const override { return _lighting; }
    bool LightingRemasterActive() const override { return _lightingActive; }
    void PinCloudDrift(bool pinned, float x, float z) override;
    bool GetCloudDrift(float& x, float& z) const override;
    bool GetFeatureGates(Poseidon::remaster::FeatureGates& out) const override
    {
        out = _featureGates;
        out.camera = CameraReadoutNow();
        return true;
    }
    bool LocalLightsFromTiles() const override { return _lightTilesActive; }
    bool RemasterMoonKey() const override;
    bool ParticleNightLight(Vector3Par pos, float linear[3]) const override;
    bool ParticleRigLight(float linear[3]) const override;
    bool CombatLightsActive() const override { return _combat.active; }
    void SetDecalGlow(PackedColor glow) override { _decalGlow = glow; }
    void RequestShaderReload() override { _shaderReloadPending = true; }
    void MarkWorldOverlay() override
    {
        if (_worldPassOpen)
        {
            // The world as it was before the first overlay of the frame: the
            // fog applies to it, and the overlay's darkening is laid back
            // over the fogged world (EngineGL33_PostFog.cpp).
            if (!_worldOverlayDrawn)
                SnapshotWorldBeforeOverlay();
            _worldOverlayDrawn = true;
        }
    }
    std::string ShaderOverrideDirectory() const override { return ShaderOverrideDir(); }
    void SetGpuTimingEnabled(bool enabled) override;
    bool GpuTimingEnabled() const override { return _gpuTiming; }
    std::string GpuTimingSummary() const override;
    bool GpuTimingSectionMs(const char* name, float& ms) const override;
    void GpuMarkWorldPhase(WorldPhase phase) override;
    bool WantsTerrainHorizonMap() const override;
    void SetTerrainHorizonMap(std::shared_ptr<const Poseidon::remaster::TerrainHorizonMap> map) override;
    void EndWorldScene() override;
    // --- The remastered sky: the sky table (EngineGL33_SkyTable.cpp) ---
    void BeginRemasterSky(Poseidon::RemasterSky mode) override;
    void EndRemasterSky() override;
    bool RemasterSkyPending() const override;
    std::string RemasterSkyCheck(int texels) override;
    // --- The remastered sea (EngineGL33_Sea.cpp) ---
    void BeginSea(Texture* otherFrame, float otherWeight) override;
    void EndSea() override;
    // Between BeginSea and EndSea with the feature on: the landscape's sea
    // faces route to the water program (BuildContext::remasteredSea).
    bool SeaRouted() const { return _seaOpen; }
    bool _seaOpen = false;
    // --- Long tree shadows: the canopy and its sun-visibility map (EngineGL33_Canopy.cpp) ---
    void SetTerrainCanopy(std::shared_ptr<const Poseidon::remaster::CanopyField> canopy) override;
    void TerrainCanopyChanged(const Poseidon::remaster::CanopyRect* rects, int count) override;
    std::string RemasterCanopyCheck(int texels) override;

    Poseidon::remaster::LightingTuning _lighting;
    // Remastered programs built and the setting asks for them: DoSelectPixelShader
    // picks _shaderProgramRemaster.
    bool _lightingActive = false;
    bool _lightingProgramsBuilt = false;
    // A world 3D pass began since InitDraw (EndWorldScene runs the post pass
    // only then) and the terrain/cloud lookups are valid for the draws in it
    // (not a portrait capture or a 3D preview drawn inside a dialog).
    bool _worldPassOpen = false;
    bool _worldEffectsOn = false;
    bool _worldSceneEnded = false;   // EndWorldScene ran this frame: later 3D passes are UI previews
    bool _worldDepthCleared = false; // a depth clear inside the world pass (vehicle interior view)
    bool _worldOverlayDrawn = false; // a screen overlay drawn inside the world pass (gunner optics)
    // The two flags above as the previous frame left them: the gates are
    // computed at the start of the world pass, before this frame can set them.
    bool _prevWorldDepthCleared = false;
    bool _prevWorldOverlayDrawn = false;
    // Feature gates (Graphics/Remaster/FeatureGates.hpp), computed once per
    // frame at the world's first 3D pass by UpdateFeatureGates.
    Poseidon::remaster::FeatureGates _featureGates;
    // Dev: rebuild every program from the built-in sources and the override
    // directory at the start of the next frame (EngineGL33_Shaders.cpp).
    bool _shaderReloadPending = false;
    void ReloadShadersNow();
    // GPU timestamp queries (EngineGL33_GpuTimer.cpp): kGpuFrames frames in
    // flight, each read back when its slot comes round again and only if the
    // GPU has finished it (the CPU never waits on a query).
    static constexpr int kGpuFrames = 4;
    unsigned int _gpuQueries[kGpuFrames][Poseidon::gpu::kMarkCount] = {};
    std::uint32_t _gpuRecorded[kGpuFrames] = {};
    int _gpuSlot = 0;
    bool _gpuTiming = false;
    Poseidon::gpu::GpuTimings _gpuTimings;
    void GpuMark(Poseidon::gpu::Mark mark);
    void GpuFrameBegin();
    void DestroyGpuQueries();
    // The remastered program set is in use: lighting active and inside the
    // world's 3D passes. Portraits, 3D previews in dialogs and night vision draw
    // through the classic programs (their own light setups and the green
    // intensifier are calibrated for them). Owns the unit-2 comparison sampler.
    bool _remasterPrograms = false;
    // Opaque draw in flight: the remastered shaders store one minus the sky
    // light share in alpha (EngineGL33_PostFX.cpp). Mirrors LitConstants.flags.x.
    bool _litOpaque = false;
    bool _litAdditive = false;
    // The rig's sun colour this frame (sun-shaft tint) and its exposure.
    Color _lightRigSun = Color(1, 1, 1, 1);
    // The rig's sky light on an up-facing surface (the photoreal camera's sky
    // clouds read it).
    Color _lightRigSky = Color(0, 0, 0, 1);
    float _lightRigExposure = 1.0f;
    // The air pass (B8, EngineGL33_PostAir.cpp): this frame's medium and which
    // of its parts run (their switches and gates), set with the gates at the
    // world's first pass; the forward fog's edge follows it (litHaze).
    Poseidon::remaster::AirMedium _airMedium;
    Poseidon::remaster::AirParts _airParts;
    std::uint32_t _airFrame = 0; // engine frame counter when set
    // Sets _airMedium and _airParts from the gates and the weather, and the
    // Haze slot (the classic fog's edge) for the world's draws.
    void UpdateAirMedium();
    // World position of the camera when the shadow depth pass ran: the map is
    // sampled one frame later, from wherever the camera is then.
    float _shadowCamPos[3] = {};
    // Terrain horizon map and the textures the remastered shaders sample.
    std::shared_ptr<const Poseidon::remaster::TerrainHorizonMap> _terrainMap;
    bool _terrainMapDirty = false;
    unsigned int _terrainHorizonTex = 0; // GL_TEXTURE_2D_ARRAY RGBA16, 4 layers
    unsigned int _terrainHeightTex = 0;  // GL_RG32F: height, sky visibility
    unsigned int _groundFieldTex = 0;    // GL_RGBA16: the land's own colour per vertex, a = 1 (unit 12)
    unsigned int _cloudNoiseTex = 0;     // GL_R8, tileable
    unsigned int _dummyDepthArray = 0;   // 1x1x1 depth array: units 2 and 6 when no shadow map
    unsigned int _dummyHorizonTex = 0;   // 1x1x1 RGBA16 array: unit 3 when no map
    unsigned int _dummyHeightTex = 0;    // 1x1 RG32F: unit 4 when no map
    unsigned int _dummyGroundTex = 0;    // 1x1 RGBA16 (0, 0, 0, 0): unit 12 without a ground field
    unsigned int _shadowCompareSampler = 0;
    unsigned int _shadowRawSampler = 0; // unit 6: the cascade array as raw depth (soft-shadow blocker search)
    unsigned int _shaderProgramRemaster[NVertexShaders][NPixelShaderSpecular][NPixelShaderModes][NPixelShaders] = {};

    void InitLightingResources();
    void DeinitLightingResources();
    void UploadTerrainHorizonMap();
    // The ground field's texture (unit 12) while a map with ground colours is
    // uploaded, else its stand-in (a = 0: the shaders class the land as before).
    unsigned int GroundFieldTexture() const;
    // Per-frame rig, terrain, cloud and camera uniforms; at the start of a 3D pass.
    void UploadLitFrameConstants();
    // Fills the gate inputs from the scene and computes _featureGates, once per
    // frame and only in the world's own passes (not a portrait capture or a 3D
    // preview after the world). `rig` is this frame's light rig when the
    // remastered programs computed one (its cloud cover), else nullptr.
    void UpdateFeatureGates(const Poseidon::remaster::LightRig* rig);
    // Re-derives _remasterPrograms from the lighting and world-pass flags and,
    // on a change, rebinds the unit-2 sampler and re-selects the current
    // program. Callers flush pending draws first.
    void UpdateRemasterPrograms();
    void UploadLitMaterial(const Poseidon::TLMaterial& mat);
    void UploadLitShadowDelta();
    // The draw's own light list into the per-draw light slots; while the tiled
    // list lights the pass, only the draw's night factor (litLightCount.z).
    void UploadLitLights(const LightList& lights, float nightEffect);
    void SetLitOpaque(bool opaque, bool additive = false);
    // The terrain detail (B11) runs on the land only: the land and the sea are
    // both drawn by PSDetail, the land with the DetailTexture bit, the sea with
    // SpecularTexture. Sets litDetail.w for the draw ApplyPassState prepares.
    void SetLitTerrainDraw(bool land);
    // ApplyPassState, for a PSDetail draw: the land or the sea, from the
    // draw's multitexture stage (EngineGL33_State.cpp, next to SetTexture's).
    void SelectTerrainDetailLayer(const Poseidon::render::LegacySpec& spec);
    // A detail-family draw's faces are the land's (the detail texture), not
    // the sea's (the specular layer alone); the terrain detail and the
    // reflections' surface tag both ask (EngineGL33_State.cpp).
    static bool IsLandLayer(const Poseidon::render::LegacySpec& spec);

    // The frame's tiled light list (the lightTiles switch, plan section 5 A6;
    // Graphics/Rendering/Lighting/FrameLights.hpp). Built once per frame at the
    // world's first 3D pass when its gate is on: the scene's active lights,
    // ranked, binned into kTileSize tiles over the frame target and uploaded to
    // a uniform block of lights (kLightListBinding) and a tile index texture
    // (kLightTileUnit).
    struct LightTileState
    {
        std::vector<Poseidon::lighting::FrameLight> lights; // ranked
        Poseidon::lighting::TileGrid grid;
        std::vector<float> texels;                 // the buffer contents (PackFrameLights)
        std::vector<float> uploadedTexels;         // what the buffer holds
        std::vector<std::uint8_t> uploadedIndices; // what the tile texture holds
        unsigned int dataBuf = 0;                  // uniform block (kLightListBinding), kMaxFrameLights lights
        unsigned int tileTex = 0;                  // RGBA8UI, kTileTexels texels per tile
        int texW = 0, texH = 0;                    // tile texture size
        std::uint32_t frame = 0xffffffffu;         // frame counter of the last build
    } _lightTiles;
    // The remastered programs read the tiled list in the current world pass
    // (litLightCount.y holds the tile size) instead of a draw's own lights:
    // per-draw light uploads stop and the scene may draw lit objects in
    // instanced runs (LocalLightsFromTiles). Decided at each world 3D pass from
    // the frame's gate, cleared at EndWorldScene.
    bool _lightTilesActive = false;
    // The physical lamps and low beams light this world pass (LitConst::Night
    // x, the nightLights gate with the tiled list): particles take them too
    // (ParticleNightLight). Decided with _lightTilesActive, cleared with it.
    bool _nightLightsActive = false;
    // The rig's light on a lit particle under the photoreal camera
    // (ParticleRigLight; pre-exposed linear), set with the frame's rig
    // (UploadLitFrameConstants) for the frame it names.
    float _particleRigLight[3] = {0.0f, 0.0f, 0.0f};
    std::uint32_t _particleRigFrame = 0xffffffffu;
    // Wet weather (B7, Graphics/Remaster/Wetness.hpp, SurfaceMaterials.hpp):
    // this frame's wetness (the world's state or the knob), set with the gates;
    // _wetShading while the wetWeather gate is on in the world's passes, which
    // then hand each draw its surface class (LitConst::MatClass xy).
    Poseidon::remaster::WetnessFrame _wetFrame;
    bool _wetShading = false;
    // The surface rules: the stock table and the config's CfgSurfaceMaterials,
    // built at the first wet frame. Each texture classifies itself once per
    // object kind against the table of its generation (TextureGL33).
    Poseidon::remaster::SurfaceTable _surfaceTable;
    int _surfaceTableGen = 0; // 0 = not built
    void EnsureSurfaceTable();
    // The Weather slot and the frame flags of the MatClass slot, from the gates.
    void UploadLitWeather();
    // The surface class of the draw ApplyPassState prepares (its texture and
    // object kind) into LitConst::MatClass xy, uploaded only when it changes.
    void SetLitSurface(TextureGL33* tex, const Poseidon::render::RenderPassDescriptor& d);
    void InitLightTiles();
    void DeinitLightTiles();
    // Builds, bins and uploads the list (once per frame).
    void UpdateLightTiles();

    // Combat light (plan section 5 B4; Graphics/Rendering/Lighting/CombatLights.hpp,
    // EngineGL33_PostCombat.cpp). The game's combat lights join the tiled list
    // as HDR entries while their gate is on; up to four of them are shadowed
    // through a half-resolution mask traced at the start of the world from the
    // previous frame's linear depth; the strongest scatter in the air of the
    // composite; a burst's flash raises the exposure.
    struct CombatState
    {
        // This frame's combat lights light the world (UpdateLightTiles), and
        // the classic lights of the same sources light nothing.
        bool active = false;
        int count = 0;          // combat lights at the head of the tiled list
        float flashGain = 1.0f; // the flash exposure response this frame
        float flashHeld = 0.0f; // the flash irradiance the response follows (FlashHeldIrradiance)
        float exposure = 1.0f;  // the rig's exposure times the flash gain (the forward shaders' litZenith.w)
        float night = 0.0f;     // the night the day scale and the flash response follow (CombatNight)
        Poseidon::lighting::ShadowSelection selection;
        double selectionTime = 0.0; // CombatLightSet::SimTime at the last selection
        // The frame the mask is built in, and whether it was.
        std::uint32_t maskFrame = 0xffffffffu;
        bool maskBuilt = false;
        int shadowed = 0;         // channels in use this frame
        float maskLight[16] = {}; // per channel: xyz in the mask camera's view, w core radius
        float maskRange[4] = {};  // per channel: window end
        // The camera of the last linear depth (EndWorldScene), which the mask
        // is traced in, and the frame it was drawn in.
        Poseidon::lighting::CombatMaskCamera depthCamera;
        std::uint32_t depthFrame = 0xffffffffu;
        bool depthValid = false;
        // The forward shaders' mapping of this frame's fragments into the mask.
        float rows[3][4] = {};
        // The FrameLights block's combat members as uploaded (4 vec4).
        float uploaded[16] = {};
        bool uploadedValid = false;
        // The strongest combat lights for the composite's in-scatter: view
        // space position and core radius, linear radiance and window end, the
        // radius of the lit medium.
        int scatterCount = 0;
        float scatterPos[32] = {}, scatterRgb[32] = {}, scatterMed[32] = {};
        // GL: the mask and its blur ping-pong (RGBA8, half resolution), the
        // white stand-in, the programs.
        unsigned int maskTex[2] = {}, maskFbo[2] = {};
        int maskW = 0, maskH = 0;
        unsigned int standInTex = 0;
        unsigned int progMask = 0, progBlur = 0, vao = 0;
        bool programsTried = false, unavailable = false;
    } _combat;
    void InitCombatResources();
    void DeinitCombatResources();
    // The combat lights of this frame for the tiled list (camera-relative,
    // ranked, the shadowed ones given a channel) and the scatter list; called
    // by UpdateLightTiles while the combat gate is on.
    void GatherCombatLights(const Poseidon::lighting::TileView& view, std::vector<Poseidon::lighting::FrameLight>& out);
    // The FrameLights block's combat members (count, mask strength, rows).
    void UploadCombatFrame();
    // The shadow mask for this frame (once per frame, at the world's first 3D
    // pass, after the tiled list): the trace and its blur into unit 14.
    void UpdateCombatMask();
    void BindCombatMask();
    // The post pass: remember the camera of the linear depth the next frame's
    // mask traces through; the composite's in-scatter uniforms.
    void SaveCombatMaskCamera(int targetW, int targetH, const float rect[4]);
    void SetCombatScatterUniforms(unsigned int program, float exposure);
    // Writes slots [first, first + count) of the CPU LitConstants block to its
    // buffer and records them in the buffer's mirror: every upload of the
    // block, whole or partial, goes through here (plan section 5 A4).
    void UploadLitSlots(int first, int count);
    // Binds units 2..6, 8, 9, 13 and 15 for the remastered shaders (compare sampler on 2).
    void BindLightingTextures();
    // The sky table (EngineGL33_SkyTable.cpp): the stand-in and GL objects, the
    // worker build of the atmosphere tables (true once they exist for the
    // tuning's haze; starts the build when they do not), the GPU build of the
    // sky-view table, the sky slots of LitConstants and the unit-8 binding.
    void InitSkyTable();
    void DeinitSkyTable();
    bool PollSkyTables();
    bool SkyTablesReady() const;
    bool BuildSkyViewTableGpu(float sunElevationRad);
    void UploadLitSky(const Vector3& toSun, float skyThrough);
    void BindSkyTable();
    // The sky's own clouds (EngineGL33_SkyClouds.cpp): the cloud-density map
    // on unit 9 (built on a worker thread when the weather sky changes, a
    // stand-in until the first one), the shadow anchor and the clouds slots.
    void InitSkyClouds();
    void DeinitSkyClouds();
    bool SkyCloudsWanted() const;   // remastered, and litClouds or skyCloudShadows switched on
    bool SkyDensityReady() const;   // a map is built (the last one while a new weather sky builds)
    bool SkyDensityPending() const; // a build runs, or the wanted weather sky has none yet
    void PollSkyDensity();          // collects a finished build; starts one when the weather sky changed
    // The shadowed share of the ground at this sky-through and sun elevation
    // (SkyShadowCoverage), or negative without a map.
    float SkyShadowCoverageAt(float skyThrough, float sunElevationDeg) const;
    void BindSkyDensity();
    // Captures: the anchor at the camera, held while the clouds are pinned.
    void PinSkyShadowAnchor(bool pinned);
    // The Sky and SkyShadow slots; `cloudField` = the rig casts cloud shadows.
    void UploadLitClouds(float skyThrough, bool cloudField);
    // The canopy (EngineGL33_Canopy.cpp): the stand-ins on units 10 and 15, the
    // canopy texture (uploaded whole, or the rectangles that changed), the GPU
    // build of the sun-visibility map when the sun or the camera moved, the
    // Canopy slot of LitConstants and the unit-15 binding.
    void InitCanopy();
    void DeinitCanopy();
    void UploadCanopyField();
    bool BuildCanopySunGpu(const Vector3& toSun, float originX, float originZ, float texel);
    void UploadLitCanopy(const Vector3& toSun, float sunElevationDeg, float skyThrough);
    void BindCanopy();
    // The canopy heightfield as the world programs read it for the sky it
    // hides (the surfaces lane): height base, range, 1 / extent, 1 when there
    // is an uploaded canopy (else all zero).
    void SurfaceCanopyConstants(float out[4]) const;

    // Surfaces and ambient light (the look lane "surface",
    // Graphics/Remaster/SurfaceLight.hpp; EngineGL33_Surface.cpp).
    //   - The surface target: a second multisampled colour attachment of the
    //     frame target (RGBA8), drawn only while a remastered world pass runs
    //     with the switch on: the share of each pixel's light the horizon
    //     occlusion may darken, the albedo and the sun's share. Draw buffers 0
    //     and 1 from the world's first pass to EndWorldScene; the write mask of
    //     attachment 1 follows the bound program (the remastered lit programs
    //     write it) and the blend mode (opaque and alpha-blended draws).
    //   - The sky light's spherical harmonics, projected on a worker thread from
    //     the atmosphere tables whenever the sky table is rebuilt.
    //   - The per-draw surface class (the lit programs' uniform surfClass)
    //     from the section's texture and the drawn object's kind.
    unsigned int _surfRb = 0;       // the surface target's renderbuffer (attachment 1 of _ssaaFbo)
    int _surfRbSamples = 0;         // its sample count
    bool _surfTargetOn = false;     // draw buffers 0 and 1 are on (this world pass)
    bool _surfMaskOn = true;        // attachment 1's colour mask
    bool _surfProgram = false;      // the bound program writes fragSurf
    bool _surfFrameWritten = false; // this frame's world wrote the target (EndWorldScene reads it)
    bool SurfaceWanted() const;     // the switch and its gate, this frame, in the world's passes
    void BeginSurfaceTarget();
    void EndSurfaceTarget();
    void DestroySurfaceTarget();
    void UpdateSurfaceMask(bool force = false);
    void UploadLitSurface(const Poseidon::remaster::LightRig& rig, float skyThrough);
    // Sets surfClass on the bound lit program when it changed.
    void ApplySurfaceClass(TextureGL33* tex, const Poseidon::render::RenderPassDescriptor& d);
    // The occlusion accumulated over frames (EngineGL33_PostFX.cpp): which
    // target holds the last one and for which frame and size; _surfAoOut is
    // this frame's result for the composite (0: the blurred aoTex[0]).
    struct SurfAoHistory
    {
        bool exists = false;
        std::uint32_t frame = 0;
        int w = 0, h = 0;
        int idx = 0;
    } _surfAoHist;
    unsigned int _surfAoOut = 0;
    void ResetSurfaceClassPrograms();   // the lit programs were deleted (their uniform cache goes)
    unsigned int _surfBoundProgram = 0; // the bound program when it writes fragSurf, else 0
    void SurfaceEnterInterior();        // a depth clear inside the world pass: a vehicle interior draws next
    // The sky light (EngineGL33_SkyTable.cpp): starts a projection when the
    // sky table was rebuilt or the haze changed, collects a finished one.
    void PollSkyLightSH();
    const Poseidon::remaster::SkyLightSH& SkyLightSHNow() const;
    // One LitConstants slot (flushed with the next FlushLitConstants), and the
    // sky mode (litSkyFwd.w, flushed at once).
    void SetLitSlot(int slot, float x, float y, float z, float w);
    void SetLitSkyMode(int mode);
    void FlushLitConstants();
    unsigned int SelectProgram(int vs, int spec, int mode, int ps) const;

    struct PostTargets
    {
        static constexpr int kBloomLevels = 5;
        int w = 0, h = 0;
        unsigned int colorTex = 0, colorFbo = 0;
        unsigned int depthTex = 0, depthFbo = 0;
        unsigned int aoTex[2] = {}, aoFbo[2] = {};
        unsigned int shaftTex = 0, shaftFbo = 0;
        unsigned int linDepthTex = 0, linDepthFbo = 0; // R32F half res: view depth in metres
        unsigned int bloomTex[kBloomLevels] = {}, bloomFbo[kBloomLevels] = {};
        int bloomW[kBloomLevels] = {}, bloomH[kBloomLevels] = {};
        unsigned int whiteTex = 0, blackTex = 0; // 1x1 stand-ins for skipped passes
        unsigned int vao = 0;
        unsigned int progAo = 0, progAoBlur = 0, progBloomDown = 0, progBloomUp = 0, progShafts = 0, progComposite = 0;
        unsigned int progLinearDepth = 0, progDebugDepth = 0;
        // TAA (EngineGL33_PostTemporal.cpp): the composite's output when the
        // Sharpen stage runs, after the resolve or alone (RGBA8), the history ping-pong (RGBA16F: rgb colour, a
        // view depth in metres) and the opaque world copied before the
        // translucent objects (RGBA8, the particle mask).
        unsigned int taaInTex = 0, taaInFbo = 0;
        unsigned int histTex[2] = {}, histFbo[2] = {};
        unsigned int opaqueTex = 0, opaqueFbo = 0;
        unsigned int progTaa = 0, progTaaSharpen = 0;
        unsigned int progLumaSharpen = 0; // the luma sharpen with TAA off (B11)
        // The air pass (B8, EngineGL33_PostAir.cpp), half resolution: the
        // march's two targets (RGBA16F: beams, headlights, haze sun
        // visibility, view depth; RG16F: valley mist top and density), the
        // blur's ping-pong and the beams' local mean (RG16F); full resolution:
        // the frame with the air applied (RGBA8, its alpha kept), which the
        // stages after it read in place of colorTex.
        unsigned int airTex[2] = {}, airMistTex = 0, airMeanTex = 0, airOutTex = 0;
        unsigned int airMarchFbo = 0, airFbo[2] = {}, airMeanFbo = 0, airOutFbo = 0;
        unsigned int progAirMarch = 0, progAirBlur = 0, progAirMean = 0, progAirApply = 0;
        bool airUnavailable = false; // its programs failed: the air pass never runs, the rest does
        // The surfaces lane: the surface target resolved (RGBA8, full res) and
        // the horizon occlusion's program (writes aoTex like progAo).
        unsigned int surfTex = 0, surfFbo = 0;
        unsigned int progGtao = 0;
        // Its occlusion accumulated over TAA frames (RG16F half res, ping-pong).
        unsigned int gtaoHistTex[2] = {}, gtaoHistFbo[2] = {};
        unsigned int progGtaoAccum = 0;
        // The photoreal camera's meter (EngineGL33_Camera.cpp): kMeterWidth x
        // kMeterHeight RGBA16F, read back through _camera's ring.
        unsigned int meterTex = 0, meterFbo = 0, progMeter = 0;
        bool programsTried = false;
        bool unavailable = false;
    } _post;
    // TAA frame state (EngineGL33_PostTemporal.cpp, Graphics/Remaster/TemporalAA.hpp).
    struct TaaState
    {
        std::uint32_t sequence = 0; // jitter index: advances once per resolved frame
        // The frame whose world passes are jittered (engine frame counter), its
        // offset in render-target pixels and its camera; set at its first world
        // pass, when the frame gates are known.
        bool frameOn = false;
        std::uint32_t frame = 0;
        Poseidon::remaster::TaaOffset jitterPx;
        Poseidon::remaster::TaaCamera camera;
        // The offset the vertex shaders hold now (vpScale.zw, NDC).
        Poseidon::remaster::TaaOffset ndcApplied;
        float mipBiasApplied = 0.0f;
        // The opaque world was copied this frame (BeginTranslucentObjects).
        bool opaqueCopied = false;
        // The last resolve: its history, target and camera.
        Poseidon::remaster::TaaHistory history;
        int historyIdx = 0;
        Poseidon::remaster::TaaCamera prevCamera;
    } _taa;
    // TAA runs this frame: the switch and its frame gate are on, the frame
    // gets a post pass, and the world's passes are this frame's.
    bool TaaWantedThisFrame() const;
    // At each world pass: fixes the frame's jitter and camera on its first
    // pass, then hands the jitter to the vertex shaders and the texture LOD
    // bias to the material samplers (zero and none outside a TAA frame).
    void ApplyTaaJitter();
    // Writes `ndc` into vpScale.zw (and flushes) and `bias` into the material
    // samplers, each only when it changes.
    void SetTaaJitterState(Poseidon::remaster::TaaOffset ndc, float bias);
    void BeginTranslucentObjects() override;
    // The photoreal camera (look lane "light", feature switch photoLight;
    // EngineGL33_Camera.cpp, Graphics/Remaster/PhotoCamera.hpp): the physical
    // rig's inputs, the metered and adapted exposure, the white balance, and
    // the meter pass with its readback (a ring of pixel-pack buffers read a few
    // frames later; the CPU never waits on the GPU).
    struct CameraState
    {
        // Four slots: the driver runs up to three frames ahead; a slot whose
        // fence has not passed is never reused (that frame's meter is skipped).
        static constexpr int kRing = 4;
        // This frame's world passes write scene light (decided with the gates,
        // read by the post pass).
        bool frameOn = false;
        std::uint32_t frame = 0;
        // Exposure (log2 of the gain before the curve): in use and adapting
        // toward the target of the last meter.
        float exposureLog2 = 0.0f;
        float targetLog2 = 0.0f;
        bool adapted = false;
        bool snap = true; // take the next meter at once (a cut, a mode change)
        double lastStep = -1.0;
        float lastCam[3] = {};
        float lastDir[3] = {};
        bool haveCam = false;
        Poseidon::remaster::MeterResult meter;
        float nightEffect = 0.0f;
        float duskEffect = 0.0f; // CameraDusk: the dusk's exposure share (look director)
        float duskKeep = 0.0f;   // CameraDuskKeep: the dusk's dark keep share (look director round 2)
        float golden = 0.0f;     // CameraGolden: the golden hour's warmth (look director round 2)
        float timeEv = 0.0f;     // CameraTimeOfDayEv: the golden hour's and the high sun's EV (look director round 2)
        float veil = 0.0f;       // CameraWeatherVeil: the grey day's highlight limit (look fix round 5)
        // Colour: white balance times the scene saturation.
        Poseidon::remaster::Mat3 color;
        // The golden hour's warm gain (GoldenWarmGain), laid by the composite
        // on the light that is not blue (camGolden).
        Poseidon::remaster::Rgb goldenGain{1.0f, 1.0f, 1.0f};
        float shadeCool = 0.0f; // the cool shade of a low sun (composite camShade.w)
        float whiteBalance = 0.0f;
        float skyScale = 0.0f;
        float sunLum = 0.0f, skyUpLum = 0.0f;
        // Pre-exposure: the exposure (log2) the world's passes of this frame
        // stored their light at (litCamera.w), set once per frame from the
        // exposure in use when the frame began; the composite applies the
        // rest (exposureLog2 - preExposureLog2).
        float preExposureLog2 = 0.0f;
        std::uint32_t preExposureFrame = 0xffffffffu;
        // The physical sky, recomputed when the sun moved, and the reference.
        Poseidon::remaster::PhysicalSky physical;
        float physicalHaze = -1.0f;
        float referenceIrradiance = 0.0f;
        Poseidon::remaster::Rgb referenceIlluminant;
        float referenceHaze = -1.0f;
        // GL: the readback ring of the meter pass (its target and program
        // are post targets: _post.meterTex, _post.progMeter), each slot with
        // the frame it metered.
        unsigned int pbo[kRing] = {};
        void* fence[kRing] = {};
        std::uint32_t pboFrame[kRing] = {};
        // Each slot's sun on screen (uv, v up), the frame's aspect and the
        // sun-ring guard's weight (0: no low sun in front of the camera).
        float pboSun[kRing][4] = {};
        bool pboSunFront[kRing] = {};
        int next = 0;
        std::uint32_t snapFrame = 0; // a snap takes a meter from this frame on
        bool wasOn = false;
        std::vector<std::uint16_t> halfTexels; // readback scratch (the meter target's own half floats)
        std::vector<float> logs, weights, peaks, skyPeaks;
        std::uint32_t readbacks = 0;
        // The camera's CPU time on the render thread (ms, smoothed) and the
        // meters skipped because their slot was still in flight.
        float cpuCollectMs = 0.0f, cpuQueueMs = 0.0f;
        std::uint32_t skippedMeters = 0;
    } _camera;
    // The camera can run this frame (switch, post effects, no debug view, no
    // night vision, a frame target with a post pass, not a portrait).
    bool CameraWanted() const;
    // The physical sky for the sun at this elevation from the sky table's
    // atmosphere tables (EngineGL33_SkyTable.cpp); false while they build.
    bool PhysicalSkyFor(float sunElevationRad, Poseidon::remaster::PhysicalSky& out);
    // After the gates: this frame's camera decision, colour matrix and the
    // Camera slot of LitConstants.
    void UpdateCameraFrame(const Poseidon::remaster::LightRig& rig, bool atmosphereOn);
    // At the post pass: collect finished meter readbacks and adapt.
    void CameraAdapt();
    void DestroyCamera();
    Poseidon::remaster::CameraReadout CameraReadoutNow() const;
    void DestroyPostTargets();
    void DestroyPostPrograms();
    bool EnsurePostResources(int w, int h);

    // One post pass (INTEGRATION-PLAN.md section 3.6): what every stage reads.
    // EndWorldScene fills it, then runs the stages its plan names in
    // PostStage order (Graphics/Remaster/PostStages.hpp).
    struct PostFrame
    {
        int w = 0, h = 0;   // frame target
        int hw = 0, hh = 0; // half resolution: AO, shafts, linear depth
        // x = _33, y = _43 (viewZ = y / (ndcZ - x)), z = 1/_11, w = 1/_22
        float projInfo[4] = {};
        // The 3D viewport in uv (x, y, w, h): the scene may fill only a
        // sub-rectangle (aspect pillarbox).
        float rect[4] = {0.0f, 0.0f, 1.0f, 1.0f};
        float depthScale = 1.0f, depthBias = 0.0f; // ndcZ = depth * scale + bias
        // The sun on screen, for the shafts.
        float sunU = 0.5f, sunV = 0.5f, shaftWeight = 0.0f;
        bool sunInFront = false; // the sun is in front of the camera and above the horizon (sunU, sunV valid)
        Color shaftTint = Color(1, 1, 1, 1);
        // TAA: this frame's shift of the AO and shaft sample patterns (0 when
        // the resolve does not run: the fixed patterns of before).
        float noiseShift = 0.0f;
        // This frame's world was lit by combat lights: the composite scatters
        // them in the air (B4).
        bool combatScatter = false;
        // The surfaces lane: this frame's world wrote the surface target, so
        // the resolve copies it, the Ao stage runs the horizon occlusion and
        // the composite reads its shares.
        bool surfaces = false;
        // ... in a vehicle interior (its own depth, no world behind it).
        bool surfaceInterior = false;
        // Which stages run this frame and the strengths the composite uses.
        Poseidon::remaster::PostStagePlan plan;
        // The frame's colour the air pass, the glow stages and the composite
        // read: the resolved world (_post.colorTex), or the reflections' output
        // once the Ssr stage has run. When the air pass runs, the bloom and the
        // composite read its output (_post.airOutTex) instead.
        unsigned int frameColor = 0;
        // The photoreal camera runs this frame: the frame holds scene light
        // (camEncode) and the composite is the camera; its exposure (linear).
        bool camera = false;
        float exposure = 1.0f;
    };
    void PostFrameGeometry(PostFrame& f) const;
    void PostFrameSun(PostFrame& f) const;
    void RunPostStage(Poseidon::remaster::PostStage stage, PostFrame& f);
    // The fullscreen stages' shared state, set after the resolve; and its
    // teardown, after the last stage.
    void BeginPostFullscreen();
    void EndPostFullscreen();
    // The stages (EngineGL33_PostFX.cpp). Each leaves the shared state as it
    // found it, apart from the program and the kPostUnits bindings.
    void PostResolve(const PostFrame& f);
    void PostLinearDepth(const PostFrame& f);
    void PostAo(const PostFrame& f);
    void PostSurfaceAccumulate(const PostFrame& f); // the surfaces lane's occlusion over TAA frames
    void PostAir(const PostFrame& f); // EngineGL33_PostAir.cpp
    void PostBloom(const PostFrame& f);
    void PostShafts(const PostFrame& f);
    void PostComposite(const PostFrame& f);
    void PostTemporal(const PostFrame& f);
    void PostSharpen(const PostFrame& f);
    void PostDebugView(const PostFrame& f);
    // Screen-space reflections (EngineGL33_PostSsr.cpp; the math is
    // Graphics/Remaster/ScreenReflections.hpp, the shaders
    // EngineGL33_PostSsrShaders.cpp): the tag readout and share measurement,
    // and the reflections (half-resolution trace, the rough blur, full-resolution
    // composite into _ssr.outTex, which becomes the frame's colour for the
    // later stages).
    void PostTagReadout(const PostFrame& f);
    void PostSsr(PostFrame& f);
    struct SsrTargets
    {
        int w = 0, h = 0;
        unsigned int tagTex = 0, tagFbo = 0; // R8 full res: each pixel's stencil tag (the FBO holds the depth-stencil)
        unsigned int shareTex = 0, shareFbo = 0; // RGBA8 kSsrShareW x kSsrShareH: water and wettable ground per cell
        unsigned int mipTex = 0, mipFbo = 0;     // RGBA8 full res with mips: the frame for the reflection's blur
        int mipLevels = 1;
        unsigned int traceTex = 0, traceFbo = 0; // RGBA16F half res: the reflection as a multiply and an add
        unsigned int spreadTex = 0;              // R16F half res: its spread (the trace's second target)
        unsigned int blurTex = 0, blurFbo = 0;   // RGBA16F half res: the trace after the rough blur
        unsigned int outTex = 0, outFbo = 0;     // RGBA8 full res: the frame with the reflections
        unsigned int progTag = 0, progShare = 0, progTrace = 0, progBlur = 0, progComposite = 0;
        bool programsTried = false;
        bool unavailable = false;
        // The share readback: a ring of pixel-pack buffers, each with its fence
        // (GLsync), read once the GPU is done with it (never waited on).
        static constexpr int kRing = 3;
        unsigned int pbo[kRing] = {};
        GLsync fence[kRing] = {};
        std::uint32_t pboFrame[kRing] = {};
        int ringNext = 0;
        // The last share read back, and the frame it was measured on.
        float waterShare = -1.0f;
        float groundShare = -1.0f;
        std::uint32_t shareFrame = 0;
        bool shareValid = false;
    } _ssr;
    bool EnsureSsrResources(int w, int h);
    void DestroySsrTargets();
    void DestroySsrPrograms();
    // Reads back the oldest share measurement the GPU has finished (no wait)
    // and queues this frame's.
    void ReadSsrShares();
    // The stencil state the draw being prepared wants (its surface tag,
    // Poseidon::remaster::SsrStencilForDraw) and the one ApplyDepthMode last
    // gave the GL (valid once applied; a stencil clear invalidates it).
    Poseidon::remaster::SsrStencil _ssrStencil;
    Poseidon::remaster::SsrStencil _ssrStencilApplied;
    bool _ssrStencilValid = false;
    // The frame whose world passes wrote the tags (UploadLitFrameConstants).
    std::uint32_t _ssrTagFrame = 0xffffffffu;
    // The world's distance fog (start in metres, 1 / range), copied when
    // BeginPass builds the world pass's frame state. The trace redoes that fog
    // with it: _frameState.fogParams[2] is the fog flag of whichever draw came
    // last (SetShaderFogEnabled runs per draw, and shadows, lights, alpha-fog
    // and unfogged draws set it to 0), not the world's.
    float _ssrWorldFog[2] = {0.0f, 0.0f};
    // The world's passes write the surface tags: the ssr switch is on, the
    // remastered programs draw the world (_remasterPrograms is off for 3D
    // previews in dialogs and under night vision), inside the world pass and
    // not into a portrait.
    bool SsrTagging() const;
    // The photoreal camera's meter pass on the resolved frame, its readback
    // queued (EngineGL33_Camera.cpp); run inside the Composite stage.
    void PostCameraMeter(const PostFrame& f);
    // The volumetric fog (the Air stage, EngineGL33_PostFog.cpp; feature switch
    // volFog, Graphics/Remaster/VolumetricFog.hpp): a froxel volume lit and
    // integrated every frame and applied to every pixel by its depth, into
    // fog.outTex, which the bloom and the composite then read in place of the
    // resolved frame.
    void PostFog(const PostFrame& f);
    // The frame's medium and the forward fog's edge (LitConst::Haze), once per
    // frame from UploadLitFrameConstants, with the gates.
    void UpdateFogMedium();
    bool EnsureFogResources(int w, int h, const Poseidon::remaster::FogGrid& grid);
    void DestroyFogResources();
    void UpdateFogFloor();
    // The colour the stages after the Air stage read: the fogged frame when
    // the fog ran this frame, else the resolved frame.
    unsigned int PostSceneTex(const PostFrame& f) const;
    // The surface's share of each pixel's light after the fog
    // (FogState::veilTex) when the fog ran this frame, else 0.
    unsigned int PostFogVeilTex(const PostFrame& f) const;
    // A screen overlay is about to be drawn into the world pass (a rifle's
    // iron sight, a gunner's optic): the frame so far, resolved, for the fog.
    void SnapshotWorldBeforeOverlay();
    struct FogState
    {
        // The volumes: the injection (ping-pong, last frame's is the history)
        // and the integration, RGBA16F, cells x cells x slices; one FBO per
        // eight slices of each (eight colour attachments, one slice each).
        Poseidon::remaster::FogGrid grid;
        unsigned int injectTex[2] = {}, integTex = 0;
        std::vector<unsigned int> injectFbo[2], integFbo;
        int current = 0;
        // The fogged frame (RGBA16F, full resolution), and on the same FBO's
        // second attachment the surface's share of each pixel's light after
        // the fog (R8, veilTex): the composite's occlusion and contact
        // shadows act on the surface's own light, not on the air in front.
        unsigned int outTex = 0, outFbo = 0, veilTex = 0;
        int outW = 0, outH = 0;
        // The world before this frame's screen overlay (RGBA8, resolved;
        // SnapshotWorldBeforeOverlay) and the frame it holds.
        unsigned int snapTex = 0, snapFbo = 0;
        int snapW = 0, snapH = 0;
        std::uint32_t snapFrame = 0xffffffffu;
        // The near field (RGBA16F, half resolution: the sun's beams and the
        // lamps' light in the air, linear).
        unsigned int nearTex = 0, nearFbo = 0;
        int nearW = 0, nearH = 0;
        // The horizon ring (RGBA16F, kFogRingBins x 1, ping-pong): the sky's
        // colour above the land in every azimuth, measured from the frame.
        unsigned int ringTex[2] = {}, ringFbo[2] = {};
        int ringCur = 0;
        // The night glow (RGBA16F): two targets at a quarter of the frame and
        // two at a sixteenth (psFogGlow's ping-pong).
        unsigned int glowTex[4] = {}, glowFbo[4] = {};
        int glowW[2] = {}, glowH[2] = {};
        // The fog floor and the mist layer (RG32F, AI sight's map's grid), the
        // terrain map the dust's aridity was taken from, and AI sight's map
        // (the simulation's, TerrainLighting.hpp SimulateFogSight) they show.
        unsigned int floorTex = 0;
        const Poseidon::remaster::TerrainHorizonMap* floorFor = nullptr;
        float floorCell = 0.0f;
        int floorSize = 0;
        std::shared_ptr<const Poseidon::remaster::FogSightMap> sightMap;
        unsigned int progInject = 0, progIntegrate = 0, progNear = 0, progApply = 0, progRing = 0, progGlow = 0;
        // The map's aridity (FogAridity, with the fog floor) and the share of
        // the stars the wet air over the eye leaves (DrawPoints).
        float aridity = 0.0f;
        float starKeep = 1.0f;
        bool programsTried = false;
        bool unavailable = false;
        // The frame's medium (UpdateFogMedium) and whether the fog applies.
        Poseidon::remaster::FogMedium medium;
        std::uint32_t mediumFrame = 0;
        bool on = false;
        // The last applied frame (PostSceneTex) and its camera and volume, for
        // the next frame's reprojection.
        std::uint32_t appliedFrame = 0xffffffffu;
        bool historyValid = false;
        float prevRot[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1}; // world to view, rows
        float prevPos[3] = {0, 0, 0};
        float prevP11 = 1.0f, prevP22 = 1.0f, prevFar = 1.0f, prevK = 1.0f;
        std::uint32_t sequence = 0;
        // The mist banks' drift (m) and the time it was last advanced.
        float drift[2] = {0.0f, 0.0f};
        float driftTime = -1.0f;
    } _fog;
    ShadowMapTuning GetShadowMapTuning() const override { return _shadowTuning; }
    void SetShadowMapTuning(const ShadowMapTuning& tuning) override { _shadowTuning = tuning; }
    void SetShadowMapSunFactor(float f) override { _shadowSunFactor = f < 0.0f ? 0.0f : (f > 1.0f ? 1.0f : f); }
    static constexpr int kShadowCascades = 4;
    void RenderShadowDepthScene(const float* lightVPs, const float* splitViewDist, const float* camFwd3,
                                int numCascades, int omniCount, int res, const ShadowCasterSet& casters) override;
    bool DumpShadowMap(const char* path) override;
    bool ShadowMapCacheSelfTest() override;
    // Per-pass: bind the cascade depth-map array to unit 2 and upload the per-
    // cascade light-VPs + the shadow control vec4 so the lit shaders darken
    // shadowed fragments.  Defined in EngineGL33_Shaders.cpp (next to the UBO
    // arrays); called from BeginPass.  No-op unless a depth pass ran this frame.
    void UpdateShadowMapLitState();

    ShadowMapTuning _shadowTuning;                 // runtime knobs (FP parameter set)
    float _shadowSunFactor = 1.0f;                 // day/night fade [0,1]; 0 at night (no sun shadow)
    bool _shadowMapActive = false;                 // a depth pass ran this frame
    unsigned int _shadowMapTex = 0;                // GL depth texture ARRAY to sample
    int _shadowMapRes = 0;                         // its resolution
    int _shadowCascades = 0;                       // active cascade count this frame
    int _shadowOmniCount = 0;                      // leading omni (camera-sphere) tiers — distance-selected
    float _shadowMapVP[kShadowCascades * 16] = {}; // per-cascade light view-projections (column-major)
    float _shadowSplits[kShadowCascades] = {};     // per-tier select distance (omni: 3D radius; frustum: eye far)
    float _shadowCamFwd[3] = {};                   // camera forward (for eye-depth cascade select)

    void GetZCoefs(float& zAdd, float& zMult) override;
    void SetBias(int bias) override;
    int GetBias() override { return _bias; }

    void SetGrassParams(float a1, float a2, float a3 = 0, float a4 = 0) override;

    bool CanZBias() const override;
    bool ZBiasExclusion() const override { return !_hasStencilBuffer; }

    AbstractTextBank* TextBank() override;
    TextBankGL33* TextBankDD() const { return _textBank; }

    void CreateTextBank();
    void ReportGRAM(const char* name);

  protected:
    void DoSetGrassParamsPS();
    void UploadPSConstant(int reg, const float* data);
    void UploadPSFogColor(const Color& fogColor);
    void FlushVSConstants();
    void FlushPSConstants();

    BlendMode _currentBlendMode = BlendMode::Opaque;
    DepthMode _currentDepthMode = DepthMode::Normal;

    void StencilExclusion(bool enable)
    {
        if (enable == _stencilExclusionEnabled)
            return;
        DoStencilExclusion(enable, true);
    }
    void DoStencilExclusion(bool enable, bool optimize);

    void ChangeClipPlanes();
    void PrepareDetailTex(bool water, bool grass);
    void PrepareSingleTexModulateA();
    void PrepareSingleTexDiffuseA();
    void SetTexture(const TextureGL33* tex, const Poseidon::render::LegacySpec& spec);
    void SetMultiTexturing(VFormatSet format);

    void EnableDetailTexGen(TexGenMode mode, bool optimize)
    {
        if (_texGenMode != mode)
            DoEnableDetailTexGen(mode, optimize);
    }
    void DoEnableDetailTexGen(TexGenMode mode, bool optimize);

    WORD* QueueAdd(QueueGL33& queue, int n);
    void QueueFan(const VertexIndex* ii, int n);
    void Queue2DPoly(const TLVertex* v, int n);

    void FlushQueue(QueueGL33& queue, int index);
    void FlushAndFreeQueue(QueueGL33& queue, int index);

    int AllocateQueue(QueueGL33& queue, TextureGL33* tex, int level, int spec);
    void FreeQueue(QueueGL33& queue, int index);
    void FreeAllQueues(QueueGL33& queue);
    void FlushAndFreeAllQueues(QueueGL33& queue, bool nonEmptyOnly = false);
    void FlushAllQueues(QueueGL33& queue, int skip = -1);

    void CloseAllQueues(QueueGL33& queue);

    void DoSwitchRenderMode(RenderMode mode);

    void D3DPreparePoint();
    void D3DPrepare3DLine();

    void ApplyPassState(TextureGL33* tex, int level, const Poseidon::render::LegacySpec& spec, Poseidon::PassId passId,
                        PipelineVertexInput vertexInput);

    // Pass-state dedup (perf effort 06): ApplyPipeline short-circuits when
    // the descriptor and its pass context match the last applied state.
    // Any GL pipeline-state write outside ApplyPipeline must call
    // InvalidatePipelineCache() (B-007).
    struct
    {
        Poseidon::render::RenderPassDescriptor d;
        bool in3d = false;
        bool a2c = false;
        PipelineVertexInput vertexInput = PipelineVertexInput::ActivePass;
        bool valid = false;
    } _lastApplied;
    PipelineVertexInput _pipelineVertexInput = PipelineVertexInput::ActivePass;
    void InvalidatePipelineCache() { _lastApplied.valid = false; }

    // Instanced-run mode (perf effort 08): Scene wraps one Shape::Draw in
    // Begin/EndInstancedRun after uploading K world matrices to the
    // WorldInstances UBO; every TL EmitDraw inside the run issues
    // glDrawElementsInstanced(K) instead. Non-TL emission (vertex-soup
    // queue) inside the run marks it impure -> caller falls back to scalar
    // draws for the remaining instances.
    void BeginInstancedRun(int count)
    {
        _instCount = count;
        _instImpure = false;
    }
    bool EndInstancedRun() override
    {
        const bool pure = !_instImpure;
        _instCount = 0;
        return pure;
    }
    void UploadWorldInstances(const float* matrices, int count);
    // Run accumulation: Scene adds model-to-world transforms; the engine
    // converts (camera-relative GfxMatrix) and uploads on BeginInstancedRunUpload.
    void InstancedRunReset() override { _instPending = 0; }
    bool InstancedRunAdd(const Matrix4& modelToWorld) override;
    int InstancedRunPending() const { return _instPending; }
    void BeginInstancedRunUpload() override;
    int _instCount = 0;
    bool _instImpure = false;
    int _instPending = 0;
    GfxMatrix _instArray[256];
    void QueuePrepareTriangle(const Poseidon::MipInfo& absMip, int specFlags);

    void PrepareTriangle(const Poseidon::MipInfo& absMip, int specFlags) override;
    void PrepareTriangleTL(const Poseidon::MipInfo& mip, const Poseidon::render::LegacySpec& spec) override;

    bool GetTL() const override { return true; }
    bool GetTLOnSurface() const override { return true; }

    // MSAA alpha-to-coverage (config knob; effective only when the default
    // framebuffer actually got MSAA samples — see _msaaActive).
    void SetAlphaToCoverage(bool enable) override { _alphaToCoverageCfg = enable; }
    bool GetAlphaToCoverage() const override { return _alphaToCoverageCfg && _msaaActive; }

    // Diagnostic flat shading: object draws output solid red (alpha-test
    // silhouette + cutouts preserved) so a shading/texture highlight can be told
    // apart from a geometry one.  Uploaded via alphaRef.w (see SetAlphaTest).
    void SetDebugFlatColor(bool enable) override
    {
        _debugFlatColor = enable;
        InvalidatePipelineCache();
    }
    bool GetDebugFlatColor() const override { return _debugFlatColor; }

    // SSAA render scale.  Applied at the next frame boundary (swap) — an
    // offscreen multisampled target at scale x window size receives the whole
    // frame (3D + HUD), resolved + downsampled to the window before swap and
    // before any framebuffer readback.
    void SetRenderScale(float scale) override;
    float GetRenderScale() const override { return _renderScale; }
    void SetMsaaSamples(int samples) override;
    int GetMsaaSamples() const override { return _msaaSamples; }
    bool HasWBuffer() const override { return false; }

    bool IsWBuffer() const override { return false; }
    bool CanWBuffer() const override { return false; }
    void SetWBuffer(bool) override {}

    void BeginPass(Poseidon::PassId passId);
    void BeginScreenPass();

    // KHR_debug pass groups — one open group per active pass, switched at
    // the real pass transitions so captures bracket the draws they name.
    void SwitchPassDebugGroup(const char* name);
    void ClosePassDebugGroup();
    bool _passDebugGroupOpen = false;

    void DiscardVB();
    void AddVertices(const TLVertex* v, int n);
    void UploadPendingVertices(); // flush the deferred _vboMirror range to the GL buffer

  public:
    bool IsIn3DPass() const { return _activePassId != Poseidon::PassId::ScreenSpace; }
    void EnableSunLight(bool enable) override;

    int AddLight(Light* light);
    void ClearLights();

    void UpdateProjection() override;

    // World-viewport crop (aspect pillarbox / manual noodle).  The 3D
    // scene renders into the AspectSettings world rect; the periphery is
    // filled black on the 3D->2D transition.  No-op when the rect is full.
    void ApplyWorldViewport();
    void EndWorldViewport();
    bool _worldViewportActive = false;
    void PrepareMeshTL(const LightList& lights, const Matrix4& modelToWorld,
                       const Poseidon::render::LegacySpec& spec) override;
    void PrepareMeshTLImpl(const FrameState& frame, const Matrix4& modelToWorld,
                           const Poseidon::render::LegacySpec& spec);
    void BeginMeshTL(const Shape& sMesh, int spec, bool dynamic = false) override;
    void EndMeshTL(const Shape& sMesh) override;
    void DrawSectionTL(const Shape& sMesh, int beg, int end) override;

    void InitGL();
    void ShutdownGL();
    void TextureDestroyed(Texture* tex) override;

  public:
    void CreateVB();
    void DestroyVB();

    void CreateVBTL();
    void DestroyVBTL();

    void RestoreVB();

    void FogColorChanged(ColorVal fogColor) override { SetFogColor(fogColor); }
    void EnableNightEye(float night) override;

    void PrepareMesh(const Poseidon::render::LegacySpec& spec) override;
    void BeginMesh(TLVertexTable& mesh, const Poseidon::render::LegacySpec& spec) override;
    void EndMesh(TLVertexTable& mesh) override;

    void InitPixelShaders();
    void DeinitPixelShaders();

    void SelectPixelShaderMode(PixelShaderMode mode)
    {
        if (_pixelShaderModeSel == mode)
            return;
        DoSelectPixelShader(_pixelShaderSel, mode, _pixelShaderSpecularSel);
    }
    void SelectPixelShaderSpecular(PixelShaderSpecular spec)
    {
        if (_pixelShaderSpecularSel == spec)
            return;
        DoSelectPixelShader(_pixelShaderSel, _pixelShaderModeSel, spec);
    }

    void SelectPixelShader(PixelShaderID ps)
    {
        if (_pixelShaderSel == ps)
            return;
        DoSelectPixelShader(ps, _pixelShaderModeSel, _pixelShaderSpecularSel);
    }
    void DoSelectPixelShader(PixelShaderID ps, PixelShaderMode mode, PixelShaderSpecular spec);

    void InitVertexShaders();
    void DeinitVertexShaders();
    void SelectVertexShader(VertexShaderID vs);
    void UploadVSScreenConstants();
    // vpScale.zw: the TAA jitter the remastered vertex shaders add (NDC).
    void UploadVSJitter(float x, float y);
    void UploadVSProjection(const FrameState& frame);
    void UploadVSViewConstants(const FrameState& frame);
    FrameState BuildFrameState(Camera* camera, LightSun* sun, int bias, const Color& fogColor, bool sunEnabled);
    PassState BuildPassState(const FrameState& frame, Poseidon::PassId passId);
    void UploadVSWorldMatrix(const float worldMatrix[16]);
    // Remastered foliage: crown ellipsoid of the shape about to draw. Writes VS
    // slots 22-23 only when the value changes.
    void UploadVSCrown(const float center[3], const float invHalf[3]);
    void UploadVSMaterialConstants(const Poseidon::TLMaterial& mat, bool sunEnabled);
    void UploadVSLights(const LightList& lights, const Poseidon::TLMaterial& mat, float nightEffect);
    void UploadVSTexGenConstants(TexGenMode mode);
    void SetShaderFogEnabled(bool enabled);

    void UploadFrameConstants(const FrameState& frame);
    void UploadPassConstants(const PassState& pass);
    void UploadObjectConstants(const DrawItem& item);

    void ApplyBlendMode(BlendMode mode);
    void ApplyDepthMode(DepthMode mode);
    void SetAlphaTest(bool enable, DWORD ref = 0xc0, bool alphaToCoverage = false);

    void Init3DState();
    void Init3D();
    void SetFogColor(ColorVal fog);

    float ZShadowEpsilon() const override { return 0.01f; }
    float ZRoadEpsilon() const override { return 0.005f; }

    float ObjMipmapCoef() const override { return 1.5f; }
    float LandMipmapCoef() const { return 1.0f; }

  private:
    // I-05 / B-019 RAII shutdown guard.  Declared LAST so its
    // destructor runs FIRST in the EngineGL33 teardown chain —
    // before `_textBank` (declared earlier) is destroyed, which
    // is when the base class's `_fonts` FontCache still holds
    // valid `Ref<Texture>` slots into the bank.  The guard clears
    // the font cache while the bank is still alive; subsequent
    // member destruction then sees an empty `_fonts` so the
    // natural base-class destructor is a no-op for that field.
    // Member order is the enforcement: C++ destroys members in
    // reverse declaration order, then base members.
    struct ShutdownGuard
    {
        EngineGL33* engine;
        ~ShutdownGuard();
    };
    ShutdownGuard _shutdownGuard{this};
};

#endif
