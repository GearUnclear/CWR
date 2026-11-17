#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Rendering/Frame/RuntimeChecks.hpp>

#include <string>
#include <utility>
#include <vector>

// Phase E.11.c — DetectBlackFrame is the pure decision function the
// production observer uses to fire I-29.  Tested standalone so its
// thresholding and "had 3D content" gating are locked down without
// any framebuffer / GL state.

namespace v2 = Poseidon::render::frame;

TEST_CASE("Frame/E.11.c: DetectBlackFrame skips check when no 3D content", "[render-frame][runtime][I-29][E.11.c]")
{
    const std::uint8_t black[3] = {0, 0, 0};
    REQUIRE_FALSE(Poseidon::render::frame::DetectBlackFrame(/*had3DContent=*/false, black).has_value());
}

TEST_CASE("Frame/E.11.c: DetectBlackFrame flags pure-black centre when 3D expected",
          "[render-frame][runtime][I-29][E.11.c]")
{
    const std::uint8_t black[3] = {0, 0, 0};
    const auto v = Poseidon::render::frame::DetectBlackFrame(true, black);
    REQUIRE(v.has_value());
    REQUIRE(std::string(v->ruleId) == "I-29");
}

TEST_CASE("Frame/E.11.c: DetectBlackFrame respects threshold boundary", "[render-frame][runtime][I-29][E.11.c]")
{
    // At the default threshold (8), (8,8,8) is still considered
    // black; (9,8,8) is not.  Asserts the inequality direction so
    // dim-but-non-black scenes pass.
    const std::uint8_t atBoundary[3] = {8, 8, 8};
    REQUIRE(Poseidon::render::frame::DetectBlackFrame(true, atBoundary).has_value());

    const std::uint8_t justOver[3] = {9, 8, 8};
    REQUIRE_FALSE(Poseidon::render::frame::DetectBlackFrame(true, justOver).has_value());
}

TEST_CASE("Frame/E.11.c: DetectBlackFrame passes lit scenes", "[render-frame][runtime][I-29][E.11.c]")
{
    // The forest-weapon-visible center pixel reads ~(120,122,118)
    // in production.  A lit world should never flag.
    const std::uint8_t litForest[3] = {120, 122, 118};
    REQUIRE_FALSE(Poseidon::render::frame::DetectBlackFrame(true, litForest).has_value());
}

TEST_CASE("Frame/E.11.c: DetectBlackFrame uses custom threshold", "[render-frame][runtime][I-29][E.11.c]")
{
    // A stricter threshold of 4 lets (5,5,5) through; a looser
    // threshold of 32 still flags (16,16,16) as dark enough to be
    // suspicious.  Tests that the parameter actually drives the
    // decision.
    const std::uint8_t dim[3] = {5, 5, 5};
    REQUIRE_FALSE(Poseidon::render::frame::DetectBlackFrame(true, dim, /*threshold=*/4).has_value());

    const std::uint8_t darker[3] = {16, 16, 16};
    REQUIRE(Poseidon::render::frame::DetectBlackFrame(true, darker, /*threshold=*/32).has_value());
}

TEST_CASE("Frame/E.11.c: DetectBlackFrame detail mentions sampled values", "[render-frame][runtime][I-29][E.11.c]")
{
    // Detail string is what an operator sees in the log; it must
    // include the actual readback so the bug class (depth-clear vs
    // matrix vs blit) is diagnosable from the line alone.
    const std::uint8_t black[3] = {1, 2, 3};
    const auto v = Poseidon::render::frame::DetectBlackFrame(true, black);
    REQUIRE(v.has_value());
    REQUIRE(v->detail.find('1') != std::string::npos);
    REQUIRE(v->detail.find('2') != std::string::npos);
    REQUIRE(v->detail.find('3') != std::string::npos);
}

// I-24 — viewport parity at the observation seam.

TEST_CASE("Frame/E.11.d: DetectViewportMismatch passes when flat_quads align", "[render-frame][runtime][I-24][E.11.d]")
{
    const int live[4] = {0, 0, 800, 600};
    REQUIRE_FALSE(Poseidon::render::frame::DetectViewportMismatch(0, 0, 800, 600, live).has_value());
}

TEST_CASE("Frame/E.11.d: DetectViewportMismatch catches width drift", "[render-frame][runtime][I-24][E.11.d]")
{
    // Window resized to 1024 wide but SceneInputs still carries 800
    // — the bug class fullscreen-transition or scissor-leak produces.
    const int live[4] = {0, 0, 1024, 600};
    const auto v = Poseidon::render::frame::DetectViewportMismatch(0, 0, 800, 600, live);
    REQUIRE(v.has_value());
    REQUIRE(std::string(v->ruleId) == "I-24");
    REQUIRE(v->detail.find("1024") != std::string::npos);
    REQUIRE(v->detail.find("800") != std::string::npos);
}

TEST_CASE("Frame/E.11.d: DetectViewportMismatch catches origin drift", "[render-frame][runtime][I-24][E.11.d]")
{
    // A scissor leak typically shifts the flat_quad origin; size matches
    // but x/y differ.  Defaults to zero tolerance so a single-pixel
    // drift is loud.
    const int live[4] = {16, 32, 800, 600};
    REQUIRE(Poseidon::render::frame::DetectViewportMismatch(0, 0, 800, 600, live).has_value());
}

TEST_CASE("Frame/E.11.d: DetectViewportMismatch tolerance window", "[render-frame][runtime][I-24][E.11.d]")
{
    // GL spec allows minor implementation slack on some drivers,
    // and HiDPI scaling can introduce 1-pixel rounding.  A
    // tolerance of 1 should swallow that without losing real
    // mismatches.
    const int liveTight[4] = {0, 0, 801, 600};
    REQUIRE_FALSE(Poseidon::render::frame::DetectViewportMismatch(0, 0, 800, 600, liveTight, 1).has_value());

    const int liveLoose[4] = {0, 0, 803, 600};
    REQUIRE(Poseidon::render::frame::DetectViewportMismatch(0, 0, 800, 600, liveLoose, 1).has_value());
}

// I-22 — every TL draw must carry a non-zero backend mesh handle.

TEST_CASE("Frame/E.11.h: DetectMissingMeshHandles passes when every TL has a VAO",
          "[render-frame][runtime][I-22][E.11.h]")
{
    // 198 forest-frame TL draws, all with backendMeshHandle set
    // by EngineGL33_VertexBuffer.cpp's DrawSectionTL.
    REQUIRE_FALSE(Poseidon::render::frame::DetectMissingMeshHandles(198, 0).has_value());
}

TEST_CASE("Frame/E.11.h: DetectMissingMeshHandles flags any zero VAO", "[render-frame][runtime][I-22][E.11.h]")
{
    // Even one bad draw is structurally wrong — a TL draw arrived
    // at the frame layer without a resolvable mesh, which means the capture
    // site (DrawSectionTL or any future TL-path) is missing the
    // backendMeshHandle assignment.
    const auto v = Poseidon::render::frame::DetectMissingMeshHandles(100, 1);
    REQUIRE(v.has_value());
    REQUIRE(std::string(v->ruleId) == "I-22");
    REQUIRE(v->detail.find("1 of 100") != std::string::npos);
}

TEST_CASE("Frame/E.11.h: DetectMissingMeshHandles passes when no TL draws at all",
          "[render-frame][runtime][I-22][E.11.h]")
{
    // HUD-only frame: all draws went through the queued path, no
    // TL draws.  The check vacuously passes — nothing to validate.
    REQUIRE_FALSE(Poseidon::render::frame::DetectMissingMeshHandles(0, 0).has_value());
}

// B-007 — GL bind-cache skip must agree with the live GL binding.
// This is the pure decision the debug tripwire in GL33Bind::Tex2D uses:
// when the cache skips a glBindTexture, it reads back GL_TEXTURE_BINDING_2D
// and feeds (cachedHandle, liveGLHandle) here.  Catches the
// divergence class — a deleted/recycled handle still matching a stale skip.

TEST_CASE("Frame/B-007: DetectBindCacheDivergence passes when cache matches GL", "[render-frame][runtime][B-007]")
{
    // The common case: the skip is legitimate — GL really holds what the
    // cache claims, so no glBindTexture was needed.
    REQUIRE_FALSE(Poseidon::render::frame::DetectBindCacheDivergence(0, 42, 42).has_value());
}

TEST_CASE("Frame/B-007: DetectBindCacheDivergence flags a stale cache entry", "[render-frame][runtime][B-007]")
{
    // Cache claims unit 0 holds texture 42 but GL has 99 — something rebound
    // the unit outside the cache without invalidating.  The skip would leave 99
    // sampled instead of 42.
    const auto v = Poseidon::render::frame::DetectBindCacheDivergence(0, 42, 99);
    REQUIRE(v.has_value());
    REQUIRE(std::string(v->ruleId) == "B-007");
    REQUIRE(v->detail.find("unit 0") != std::string::npos);
    REQUIRE(v->detail.find("42") != std::string::npos);
    REQUIRE(v->detail.find("99") != std::string::npos);
}

TEST_CASE("Frame/B-007: DetectBindCacheDivergence catches the recycled-handle shape", "[render-frame][runtime][B-007]")
{
    // The recycled-handle manifestation: the overview texture (GL id 7) was deleted and
    // its id recycled for a terrain tile, but the engine's stale skip-gate still
    // matched 7 while the unit actually sat on the GL default (id 0, the 1x1
    // white). The cache claims 7; GL reads back 0 → divergence.
    const auto v = Poseidon::render::frame::DetectBindCacheDivergence(0, 7, 0);
    REQUIRE(v.has_value());
    REQUIRE(std::string(v->ruleId) == "B-007");
    REQUIRE(v->detail.find("7") != std::string::npos);
}

// I-29 over time (W1-2): BlackFrameTracker. The observer's centre pixel now
// arrives through a fenced readback a few frames late, or not at all when no
// slot is free; the tracker must give the counts and log lines the old
// synchronous read gave.

namespace
{
using Tracker = Poseidon::render::frame::BlackFrameTracker;
constexpr std::uint8_t kBlack[3] = {0, 0, 0};
constexpr std::uint8_t kLit[3] = {120, 122, 118};

// One observed frame with its result in hand at once (the old synchronous read).
std::vector<Tracker::Log> SyncSample(Tracker& t, std::uint32_t frame, const std::uint8_t rgb[3])
{
    t.ExpectSample(frame);
    t.Resolve(frame, rgb);
    return t.Advance(frame);
}
} // namespace

TEST_CASE("Frame/W1-2: BlackFrameTracker logs at the 30th consecutive black frame",
          "[render-frame][runtime][I-29][W1-2]")
{
    Tracker t;
    std::uint32_t f = 100;
    for (int i = 1; i < Tracker::kThreshold; ++i)
        REQUIRE(SyncSample(t, f++, kBlack).empty());
    REQUIRE(t.Consecutive() == Tracker::kThreshold - 1);
    const auto logs = SyncSample(t, f++, kBlack);
    REQUIRE(logs.size() == 1);
    CHECK(std::string(logs[0].violation.ruleId) == "I-29");
    CHECK(logs[0].consecutive == Tracker::kThreshold);
    CHECK(logs[0].violation.detail.find("RGB=(0,0,0)") != std::string::npos);
}

TEST_CASE("Frame/W1-2: BlackFrameTracker resets on a lit frame and on a frame without 3D",
          "[render-frame][runtime][I-29][W1-2]")
{
    Tracker t;
    std::uint32_t f = 1;
    for (int i = 0; i < 20; ++i)
        SyncSample(t, f++, kBlack);
    SyncSample(t, f++, kLit);
    REQUIRE(t.Consecutive() == 0);
    for (int i = 0; i < 20; ++i)
        SyncSample(t, f++, kBlack);
    REQUIRE(t.Consecutive() == 20);
    t.NoContent(f);
    REQUIRE(t.Advance(f++).empty());
    REQUIRE(t.Consecutive() == 0);
    // Neither 20-frame run reached the threshold.
    REQUIRE(t.LogsEmitted() == 0);
}

TEST_CASE("Frame/W1-2: BlackFrameTracker caps the log at five lines", "[render-frame][runtime][I-29][W1-2]")
{
    Tracker t;
    std::vector<int> counts;
    for (std::uint32_t f = 1; f <= 60; ++f)
        for (const auto& l : SyncSample(t, f, kBlack))
            counts.push_back(l.consecutive);
    REQUIRE(counts == std::vector<int>{30, 31, 32, 33, 34});
    REQUIRE(t.LogsEmitted() == Tracker::kLogCap);
    // The cap holds for the session: a new run after a reset logs nothing more.
    SyncSample(t, 61, kLit);
    for (std::uint32_t f = 62; f <= 100; ++f)
        REQUIRE(SyncSample(t, f, kBlack).empty());
    CHECK(t.Consecutive() == 39);
}

TEST_CASE("Frame/W1-2: BlackFrameTracker applies late results in frame order", "[render-frame][runtime][I-29][W1-2]")
{
    // Frames 1..30 are black and each result arrives 3 frames late; frame 31
    // has no 3D content, so its reset is queued before the results of frames
    // 28..30 arrive. Synchronously, frame 30 logged (run 30) and frame 31 then
    // reset the run. The reset must neither wipe the late results nor be lost.
    Tracker t;
    for (std::uint32_t f = 1; f <= 30; ++f)
    {
        t.ExpectSample(f);
        if (f > 3)
            t.Resolve(f - 3, kBlack);
        REQUIRE(t.Advance(f).empty());
    }
    REQUIRE(t.Consecutive() == 27);
    t.NoContent(31);
    t.Resolve(28, kBlack);
    REQUIRE(t.Advance(31).empty());
    t.Resolve(29, kBlack);
    REQUIRE(t.Advance(32).empty());
    REQUIRE(t.Consecutive() == 29);
    REQUIRE(t.Pending() == 2); // frame 30's sample, then frame 31's reset
    t.Resolve(30, kBlack);
    const auto logs = t.Advance(33);
    REQUIRE(logs.size() == 1);
    CHECK(logs[0].consecutive == 30);
    CHECK(t.Consecutive() == 0);
    CHECK(t.Pending() == 0);
}

TEST_CASE("Frame/W1-2: BlackFrameTracker matches the synchronous read with a three-frame delay",
          "[render-frame][runtime][I-29][W1-2]")
{
    // A scripted 300-frame session: black stretches, lit frames, frames without
    // 3D content. The delayed tracker must emit the same log lines (same run
    // lengths, same order) as the synchronous one, each at most 3 frames late,
    // and end on the same run.
    auto kind = [](std::uint32_t f) -> int // 0 black, 1 lit, 2 no 3D content
    {
        if (f == 33 || f == 270)
            return 2;
        if (f % 41 == 0)
            return 1;
        return (f / 50) % 2 == 0 ? 0 : (f % 3 == 0 ? 1 : 0);
    };
    Tracker sync, late;
    std::vector<std::pair<int, std::uint32_t>> syncLogs, lateLogs; // (run, frame logged)
    constexpr std::uint32_t kDelay = 3;
    constexpr std::uint32_t kFrames = 300;
    for (std::uint32_t f = 1; f <= kFrames + kDelay; ++f)
    {
        if (f <= kFrames)
        {
            const int k = kind(f);
            if (k == 2)
            {
                sync.NoContent(f);
                late.NoContent(f);
            }
            else
            {
                sync.ExpectSample(f);
                sync.Resolve(f, k == 0 ? kBlack : kLit);
                late.ExpectSample(f);
            }
            for (const auto& l : sync.Advance(f))
                syncLogs.emplace_back(l.consecutive, f);
        }
        // The result of frame f - kDelay arrives now.
        if (f > kDelay && f - kDelay <= kFrames && kind(f - kDelay) != 2)
            late.Resolve(f - kDelay, kind(f - kDelay) == 0 ? kBlack : kLit);
        for (const auto& l : late.Advance(f))
            lateLogs.emplace_back(l.consecutive, f);
    }
    REQUIRE(syncLogs.size() == 5);
    REQUIRE(syncLogs.size() == lateLogs.size());
    for (size_t i = 0; i < syncLogs.size(); ++i)
    {
        CHECK(syncLogs[i].first == lateLogs[i].first);
        CHECK(lateLogs[i].second >= syncLogs[i].second);
        CHECK(lateLogs[i].second - syncLogs[i].second <= kDelay);
    }
    CHECK(late.Consecutive() == sync.Consecutive());
    CHECK(late.Pending() == 0);
}

TEST_CASE("Frame/W1-2: BlackFrameTracker: a dropped sample changes nothing", "[render-frame][runtime][I-29][W1-2]")
{
    // Frame 15 could not be queued (no free slot): the observer never calls
    // ExpectSample, just as a failed SamplePixel never touched the count.
    Tracker t;
    std::uint32_t logged = 0;
    for (std::uint32_t f = 1; f <= 31; ++f)
    {
        if (f == 15)
        {
            REQUIRE(t.Advance(f).empty());
            continue;
        }
        if (!SyncSample(t, f, kBlack).empty())
            logged = f;
    }
    // 30 black samples in 31 frames: the run reaches 30 at frame 31.
    CHECK(logged == 31);
    CHECK(t.Consecutive() == 30);
}

TEST_CASE("Frame/W1-2: BlackFrameTracker: a lost readback is skipped once a later one arrives",
          "[render-frame][runtime][I-29][W1-2]")
{
    // Results arrive in queue order, so when frame 12's result comes, frame
    // 11's (never delivered) is lost: it neither extends nor ends the run.
    Tracker t;
    for (std::uint32_t f = 1; f <= 10; ++f)
        SyncSample(t, f, kBlack);
    t.ExpectSample(11);
    t.ExpectSample(12);
    REQUIRE(t.Advance(12).empty());
    REQUIRE(t.Consecutive() == 10);
    t.Resolve(12, kBlack);
    REQUIRE(t.Advance(13).empty());
    CHECK(t.Consecutive() == 11);
    CHECK(t.Pending() == 0);
}

TEST_CASE("Frame/W1-2: BlackFrameTracker: a result that never comes ages out", "[render-frame][runtime][I-29][W1-2]")
{
    // A readback lost with no later result (the ring torn down, say) must not
    // hold the events behind it for ever.
    Tracker t;
    for (std::uint32_t f = 1; f <= 5; ++f)
        SyncSample(t, f, kBlack);
    t.ExpectSample(6);
    t.NoContent(7);
    REQUIRE(t.Advance(6 + Tracker::kMaxDelayFrames).empty());
    CHECK(t.Consecutive() == 5); // still waiting on frame 6
    CHECK(t.Pending() == 2);
    REQUIRE(t.Advance(6 + Tracker::kMaxDelayFrames + 1).empty());
    CHECK(t.Consecutive() == 0); // frame 6 dropped, frame 7's reset applied
    CHECK(t.Pending() == 0);
    // A result for a sample already given up on is ignored.
    t.Resolve(6, kBlack);
    REQUIRE(t.Advance(20).empty());
    CHECK(t.Consecutive() == 0);
}

TEST_CASE("Frame/W1-2: BlackFrameTracker survives the frame counter wrapping", "[render-frame][runtime][I-29][W1-2]")
{
    Tracker t;
    std::uint32_t f = 0xFFFFFFF0u;
    std::vector<int> counts;
    for (int i = 0; i < 40; ++i, ++f)
    {
        t.ExpectSample(f);
        if (i >= 2)
            t.Resolve(f - 2, kBlack);
        for (const auto& l : t.Advance(f))
            counts.push_back(l.consecutive);
    }
    // 38 results delivered: logs at runs 30 to 34 (the cap).
    REQUIRE(counts == std::vector<int>{30, 31, 32, 33, 34});
    CHECK(t.Consecutive() == 38);
    CHECK(t.Pending() == 2);
}
