#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <stddef.h>

// I-33 / B-037 — TL emission timing.
//
// B-037 introduced a regression where the frame layer's end-of-frame `Execute`
// re-issued every captured TL draw.  At that point in the frame
// the legacy sky depth, shadow stencil, and darken quad had
// already landed, so the deferred draws fell behind state they
// no longer matched (horizon trees lost to sky depth, opaque
// objects painted over shadow modulation).  The fix moves the frame layer's
// `EmitDraw` inline at the `DrawSectionTL` callsite, where the
// per-draw state has just been established.
//
// This audit enforces:
//   1. `EngineGL33::EmitDraw` is called inline at `DrawSectionTL`
//      in `EngineGL33_VertexBuffer.cpp` — the descriptor-landed
//      site where emission belongs.
//   2. The end-of-frame validation layer (Graphics/Rendering/Frame)
//      contains no dispatch machinery and no emission call — the
//      replay path doesn't exist, so the B-037 ordering bug is
//      structurally unrepresentable.

namespace
{
std::string ReadTextFile(const std::filesystem::path& p)
{
    std::ifstream f(p);
    if (!f.is_open())
        return {};
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::filesystem::path RepoRoot()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path();
}
} // namespace

TEST_CASE("I-33: EmitDraw is called inline at DrawSectionTL", "[Rendering][Frame][EmissionTiming][I-33]")
{
    const std::string body = ReadTextFile(RepoRoot() / "engine" / "PoseidonGL33" / "EngineGL33_VertexBuffer.cpp");
    REQUIRE_FALSE(body.empty());

    // The `DrawSectionTL` function (the legacy state-landed site)
    // must contain exactly one `EmitDraw(d)` call.  More than one
    // would mean a duplicated path; zero means the emission
    // seam fell off the legacy path.
    const size_t fnStart = body.find("DrawSectionTL");
    REQUIRE(fnStart != std::string::npos);

    int inlineCalls = 0;
    size_t pos = 0;
    while ((pos = body.find("EmitDraw(d)", pos)) != std::string::npos)
    {
        ++inlineCalls;
        pos += 1;
    }
    REQUIRE(inlineCalls == 1);
}

TEST_CASE("I-33: the frame validator contains no replay dispatch", "[Rendering][Frame][EmissionTiming][I-33]")
{
    const std::filesystem::path frameDir = RepoRoot() / "engine" / "Poseidon" / "Graphics" / "Rendering" / "Frame";

    // The end-of-frame replay machinery (Execute / IFrameBackend /
    // backends) is deleted, not just unused — reintroducing a file
    // that walks the Frame against a backend re-opens the B-037
    // ordering bug.
    REQUIRE_FALSE(std::filesystem::exists(frameDir / "Execute.hpp"));
    REQUIRE_FALSE(std::filesystem::exists(frameDir / "Execute.cpp"));
    REQUIRE_FALSE(std::filesystem::exists(frameDir / "IFrameBackend.hpp"));
    REQUIRE_FALSE(std::filesystem::exists(frameDir / "EngineGL33Backend.cpp"));

    // And the validator itself never emits: no draw or pipeline call
    // sites anywhere in the observation layer.
    int scanned = 0;
    for (const auto& entry : std::filesystem::directory_iterator(frameDir))
    {
        const auto ext = entry.path().extension();
        if (ext != ".cpp" && ext != ".hpp")
            continue;
        ++scanned;
        const std::string body = ReadTextFile(entry.path());
        INFO(entry.path().filename().string());
        REQUIRE(body.find("EmitDraw(") == std::string::npos);
        REQUIRE(body.find("ApplyPipelineDescriptor(") == std::string::npos);
    }
    REQUIRE(scanned > 0);
}

namespace
{
// The body of the function whose definition starts with `signature`, up to
// the first line that is a lone closing brace.
std::string FunctionBody(const std::string& file, const std::string& signature)
{
    const size_t start = file.find(signature);
    if (start == std::string::npos)
        return {};
    const size_t end = file.find("\n}\n", start);
    return file.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

int CountOf(const std::string& haystack, const std::string& needle)
{
    int n = 0;
    for (size_t pos = haystack.find(needle); pos != std::string::npos; pos = haystack.find(needle, pos + 1))
        ++n;
    return n;
}
} // namespace

TEST_CASE("W1-2: the frame observer never reads the frame synchronously", "[Rendering][Frame][I-29][W1-2]")
{
    // A synchronous centre-pixel read in ObserveRenderedFrame resolved the
    // whole multisampled frame twice and waited for the GPU (about 100 ms per
    // observed frame on long views). The observer queues an asynchronous
    // probe and collects finished ones instead.
    const std::filesystem::path frameDir = RepoRoot() / "engine" / "Poseidon" / "Graphics" / "Rendering" / "Frame";
    const std::string obs = ReadTextFile(frameDir / "WorldFrameObserver.cpp");
    REQUIRE_FALSE(obs.empty());
    CHECK(obs.find("SamplePixel(") == std::string::npos);
    CHECK(obs.find("SampleBackBufferNonBlack(") == std::string::npos);
    CHECK(obs.find("ResolveSSAAToDefault") == std::string::npos);
    const std::string observe = FunctionBody(obs, "void ObserveRenderedFrame(Engine& engine, Scene& scene)");
    REQUIRE_FALSE(observe.empty());
    CHECK(CountOf(observe, "engine.QueueWorldProbe(cx, cy)") == 1);
    // Results are collected at the top, before the sampling cadence returns.
    const size_t collect = observe.find("CollectWorldProbes(engine);");
    const size_t cadence = observe.find("--s_sampleCountdown");
    REQUIRE(collect != std::string::npos);
    REQUIRE(cadence != std::string::npos);
    CHECK(collect < cadence);
    CHECK(obs.find("engine.PollWorldProbes(") != std::string::npos);
}

TEST_CASE("W1-2: the GL33 world probe never waits and never resolves the whole frame", "[Rendering][Frame][I-29][W1-2]")
{
    const std::string vb = ReadTextFile(RepoRoot() / "engine" / "PoseidonGL33" / "EngineGL33_VertexBuffer.cpp");
    REQUIRE_FALSE(vb.empty());
    const std::string queue = FunctionBody(vb, "bool EngineGL33::QueueWorldProbe(int x, int y)");
    const std::string poll = FunctionBody(vb, "int EngineGL33::PollWorldProbes(WorldProbe* out, int max)");
    REQUIRE_FALSE(queue.empty());
    REQUIRE_FALSE(poll.empty());
    for (const std::string* body : {&queue, &poll})
    {
        CHECK(body->find("ResolveSSAAToDefault") == std::string::npos);
        CHECK(body->find("BindFrameRenderTarget") == std::string::npos); // no viewport change
        CHECK(body->find("glViewport") == std::string::npos);
        CHECK(body->find("glFinish") == std::string::npos);
        CHECK(body->find("glGetIntegerv") == std::string::npos);
    }
    // One pixel resolved into the 1x1 target, read into the slot's pack buffer
    // behind a fence; a busy slot drops the sample.
    CHECK(CountOf(queue, "glBlitFramebuffer(readX, readY, readX + 1, readY + 1, 0, 0, 1, 1,") == 1);
    CHECK(CountOf(queue, "glReadPixels(readX, readY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);") == 1);
    CHECK(CountOf(queue, "glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0)") == 1);
    CHECK(queue.find("if (p.fence[i])\n        return false;") != std::string::npos);
    // The poll only ever asks with a zero timeout.
    CHECK(CountOf(poll, "glClientWaitSync(") == 1);
    CHECK(CountOf(poll, "glClientWaitSync(sync, 0, 0)") == 1);
    // tri's pixel verbs keep the synchronous read of the final window image.
    const std::string sample = FunctionBody(vb, "bool EngineGL33::SamplePixel(int x, int y, uint8_t* outRGB)");
    REQUIRE_FALSE(sample.empty());
    CHECK(sample.find("ResolveSSAAToDefault();") != std::string::npos);
}

TEST_CASE("S3-28: draw items are recorded only on frames the observer reads", "[Rendering][Frame][W1-2]")
{
    // Both DrawItem recording sites ask the observer's cadence first; nothing
    // else reads the recording.
    const std::filesystem::path gl33 = RepoRoot() / "engine" / "PoseidonGL33";
    const std::string vb = ReadTextFile(gl33 / "EngineGL33_VertexBuffer.cpp");
    const std::string q = ReadTextFile(gl33 / "EngineGL33_Queue.cpp");
    REQUIRE_FALSE(vb.empty());
    REQUIRE_FALSE(q.empty());
    for (const std::string* file : {&vb, &q})
    {
        const size_t gate = file->find("if (Poseidon::render::frame::ObserverRecordsThisFrame())");
        const size_t push = file->find("_drawItems.push_back(item);");
        REQUIRE(gate != std::string::npos);
        REQUIRE(push != std::string::npos);
        CHECK(gate < push);
        CHECK(CountOf(*file, "_drawItems.push_back(") == 1);
    }
}
