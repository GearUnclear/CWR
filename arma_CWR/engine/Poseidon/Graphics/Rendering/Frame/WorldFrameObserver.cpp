#include "WorldFrameObserver.hpp"

#include <Poseidon/Foundation/Platform/AppConfig.hpp>

#include "BuildFrame.hpp"
#include "FrameStats.hpp"
#include "RuntimeChecks.hpp"
#include "SceneExtractor.hpp"
#include "ValidateFrame.hpp"

#include <Poseidon/Graphics/Core/Engine.hpp>

namespace Poseidon
{

namespace render::frame
{

namespace
{
// Render thread only, like everything in the observer.
// Pass shape of the most recently observed frame.
std::vector<ObservedPass> s_lastShape;
// Observation cadence: frames left until the next observed one (play), and
// whether every frame is observed (tri). Read by ObserverRecordsThisFrame
// before the frame's draws; until the first observation latches the mode the
// backend records, as it always did.
int s_sampleCountdown = 0;
bool s_everyFrame = true;
// I-29: the near-black run across observed frames (W1-2).
BlackFrameTracker s_blackFrames;

void LogBlackFrames(const std::vector<BlackFrameTracker::Log>& logs)
{
    for (const auto& l : logs)
        LOG_WARN(Graphics, "frame runtime {}: {} (consecutive frames: {})", l.violation.ruleId,
                 l.violation.detail.c_str(), l.consecutive);
}

// The centre-pixel probes that have arrived (oldest first), into the tracker.
void CollectWorldProbes(Engine& engine)
{
    Engine::WorldProbe results[8];
    int n;
    do
    {
        n = engine.PollWorldProbes(results, 8);
        for (int i = 0; i < n; ++i)
            s_blackFrames.Resolve(results[i].frame, results[i].rgb);
    } while (n == 8);
    LogBlackFrames(s_blackFrames.Advance(engine.GetFrameCounter()));
}
} // namespace

const std::vector<ObservedPass>& LastObservedFrameShape()
{
    return s_lastShape;
}

bool ObserverRecordsThisFrame()
{
    // ObserveRenderedFrame observes when --s_sampleCountdown <= 0.
    return s_everyFrame || s_sampleCountdown <= 1;
}

void ObserveRenderedFrame(Engine& engine, Scene& scene)
{
    // Probe results come in on every frame, observed or not, so a result is
    // late by the GPU's latency only.
    CollectWorldProbes(engine);

    // Diagnostics, not rendering: extraction walks every draw bucket
    // (O(draw calls) copies per frame) and the validators scan the result.
    // Sample every 16th frame in normal play — a real invariant violation
    // is a persistent state, so sampling still catches it within ~0.3 s.
    // The tri harness (--autotest) keeps full per-frame coverage because
    // tests assert on observer output (pass shape, viewport, near-black)
    // at exact frames.
    {
        // POSEIDON_OBSERVER_SAMPLE=1 forces sampling under the harness so
        // the perf bench can measure normal-play behavior.
        static const bool everyFrame =
            Foundation::AppConfig::Instance().AutoTest() && getenv("POSEIDON_OBSERVER_SAMPLE") == nullptr;
        s_everyFrame = everyFrame;
        if (!s_everyFrame)
        {
            if (--s_sampleCountdown > 0)
                return;
            s_sampleCountdown = 16;
        }
    }
    // Frame-static state: tracks the GL error count across frames
    // so only *new* errors are reported; throttles violation logs
    // so a stable bug doesn't spam every frame; counts down to
    // the next stats summary line.
    static unsigned int s_lastGlErrorCount = 0;
    static int s_violationLogs = 0;
    static int s_statsCountdown = 0;
    static int s_viewportMismatchLogs = 0;
    static int s_meshHandleLogs = 0;

    SceneInputs si = ExtractSceneInputs(engine, scene);
    si.lastObservedDebugErrorCount = s_lastGlErrorCount;
    s_lastGlErrorCount = si.currentDebugErrorCount;

    const Frame f = BuildFrame(si);

    // Snapshot the pass shape for the tri harness.
    s_lastShape.clear();
    for (const auto& p : f.passes)
        s_lastShape.push_back({p.kind, static_cast<int>(p.draws.size())});

    const auto vr = ValidateFrame(f);

    if (!vr.ok() && s_violationLogs < 5)
    {
        for (const auto& v : vr.violations)
            LOG_DEBUG(Graphics, "ValidateFrame {}: {}", v.ruleId, v.detail.c_str());
        ++s_violationLogs;
    }

    // Viewport runtime check — the GL viewport at the observation seam
    // must match the rect SceneInputs carried.  Mismatch indicates
    // a resize / scissor / blit-target bug that screenshot tests
    // can mask (rendered content still looks plausible).  Throttle
    // the log so a stuck-mismatch only fires five times.
    {
        int liveVp[4] = {0, 0, 0, 0};
        if (engine.GetGLViewport(liveVp))
        {
            // SceneInputs carries window-space coords; with SSAA the live GL
            // viewport is the render-scaled frame target.  Scale expectations
            // the same way RenderTargetSize does so scale 1.5/2 stays clean.
            const float rs = engine.GetRenderScale();
            const auto scaled = [rs](int v) { return static_cast<int>(v * rs + 0.5f); };
            const auto vio =
                DetectViewportMismatch(scaled(f.camera.viewport.x), scaled(f.camera.viewport.y),
                                       scaled(f.camera.viewport.width), scaled(f.camera.viewport.height), liveVp);
            if (vio && s_viewportMismatchLogs < 5)
            {
                LOG_WARN(Graphics, "frame runtime {}: {}", vio->ruleId, vio->detail.c_str());
                ++s_viewportMismatchLogs;
            }
        }
    }

    // Mesh-handle runtime check — every TL draw must reach the frame
    // layer with a resolvable VAO — EmitDraw silently skips zero VAOs.
    // Counts across all per-pass buckets so a missing capture site
    // anywhere in the engine is visible.
    {
        unsigned int tlCount = 0, zeroVao = 0;
        auto count = [&](const std::vector<SceneDraw>& v)
        {
            for (const auto& d : v)
            {
                if (d.indexCount <= 0)
                    continue;
                ++tlCount;
                if (d.mesh.vao == 0)
                    ++zeroVao;
            }
        };
        count(si.shadowDraws);
        count(si.skyDraws);
        count(si.worldOpaqueDraws);
        count(si.worldCutoutDraws);
        count(si.surfaceOverlayDraws);
        count(si.waterDraws);
        count(si.worldTransparentDraws);
        count(si.cockpitDraws);
        const auto vio = DetectMissingMeshHandles(tlCount, zeroVao);
        if (vio && s_meshHandleLogs < 5)
        {
            LOG_WARN(Graphics, "frame runtime {}: {}", vio->ruleId, vio->detail.c_str());
            ++s_meshHandleLogs;
        }
    }

    // Near-black runtime check — a non-empty 3D scene should produce a
    // non-trivial centre pixel.  Catches catastrophic clear-too-late,
    // scene-matrix, or blit failures that the structural validators
    // can't see.  Counts opaque + transparent world buckets as the
    // "3D content was meant to be drawn" signal; HUD-only frames
    // (menus, options screen) are intentionally exempt.
    //
    // The centre pixel is read asynchronously (W1-2): a synchronous read
    // here resolved the whole multisampled frame twice and stalled the CPU
    // until the GPU had drawn it (a 100 ms stall per observed frame on long
    // views). The probe resolves one pixel into a fenced readback; the
    // tracker applies its result a few frames later, in frame order, with
    // the same 30-frame threshold (a pre-dawn cutscene such as the demo
    // intro at 5:35 AM reads about (6,6,8) for a while) and 5-log cap. A
    // probe that cannot be queued without waiting is dropped and changes
    // nothing, as a failed read did.
    const std::uint32_t frame = engine.GetFrameCounter();
    const bool had3DContent = !si.worldOpaqueDraws.empty() || !si.worldTransparentDraws.empty();
    if (had3DContent)
    {
        const int cx = f.camera.viewport.x + f.camera.viewport.width / 2;
        const int cy = f.camera.viewport.y + f.camera.viewport.height / 2;
        if (engine.QueueWorldProbe(cx, cy))
            s_blackFrames.ExpectSample(frame);
    }
    else
    {
        s_blackFrames.NoContent(frame);
    }

    // Optional per-frame stats summary — gated by AppConfig flag
    // so default gameplay logs nothing.  Fires on the first
    // observed frame (s_statsCountdown starts at 0) then every
    // 60th frame ≈ once a second at 60 Hz.
    if (AppConfig::Instance().RenderFrameLog())
    {
        if (s_statsCountdown <= 0)
        {
            const FrameStats s = CountFrameStats(f);
            LOG_INFO(Graphics, "render frame: passes={} draws={} maxDrawsInPass={} glErrorsThisFrame={}", s.passCount,
                     s.drawCount, s.maxDrawsInPass, f.newDebugErrors);
            s_statsCountdown = 60;
        }
        else
        {
            --s_statsCountdown;
        }
    }
}

} // namespace render::frame

} // namespace Poseidon
