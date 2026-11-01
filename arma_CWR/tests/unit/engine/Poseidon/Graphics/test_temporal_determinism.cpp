#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Remaster/TemporalDeterminism.hpp>

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>

// P0-6 temporal determinism (Graphics/Remaster/TemporalDeterminism.hpp, the
// tri verbs in Game/Commands/GameStateExtTestDeterminism.cpp): the reset
// request and its latch, the mist hold, the deterministic-mode rules (the
// readback age, the adaptation step, the reset exposure, the reflections'
// probe frames, the rain streak phase), and source audits that with the flag
// off and no reset requested every reader keeps the path it had before.

using namespace Poseidon::remaster;

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

std::filesystem::path EngineDir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine";
}

std::filesystem::path Gl33Dir()
{
    return EngineDir() / "PoseidonGL33";
}

std::string FunctionBody(const std::string& source, const std::string& signature)
{
    const size_t start = source.find(signature);
    if (start == std::string::npos)
        return {};
    const size_t end = source.find("\n}\n", start);
    return source.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

// Whitespace dropped, so the audits survive a reformat.
std::string Squash(const std::string& text)
{
    std::string out;
    out.reserve(text.size());
    for (const char c : text)
        if (c != ' ' && c != '\n' && c != '\r' && c != '\t')
            out += c;
    return out;
}

// Position of `needle` in `text`, both without whitespace.
size_t FindSq(const std::string& text, const std::string& needle, size_t from = 0)
{
    return Squash(text).find(Squash(needle), from);
}

int CountOf(const std::string& text, const std::string& needle)
{
    int n = 0;
    for (size_t at = text.find(needle); at != std::string::npos; at = text.find(needle, at + needle.size()))
        ++n;
    return n;
}

// The process-wide control state back at its defaults (the tests share it).
struct FreshControl
{
    FreshControl() { TemporalControl() = TemporalControlState{}; }
    ~FreshControl() { TemporalControl() = TemporalControlState{}; }
};

} // namespace

TEST_CASE("Temporal determinism: defaults leave play untouched", "[remaster][TemporalDeterminism]")
{
    FreshControl fresh;
    CHECK_FALSE(TemporalDeterministic());
    CHECK(TemporalControl().resetSerial == 0u);
    float x = 1.0f, z = 2.0f;
    CHECK_FALSE(TemporalMistHeld(x, z));
    CHECK(x == 1.0f); // untouched when not held
    CHECK(z == 2.0f);
    // A backend that never saw a request takes nothing.
    TemporalResetLatch latch;
    CHECK_FALSE(latch.Take(TemporalControl().resetSerial));
    CHECK_FALSE(latch.Take(TemporalControl().resetSerial));
    CHECK_FALSE(TemporalControl().seeded);
}

TEST_CASE("Temporal determinism: a reset request is taken once per request", "[remaster][TemporalDeterminism]")
{
    FreshControl fresh;
    TemporalResetLatch latch;
    RequestTemporalReset(false, 0.0f, 0.0f);
    CHECK(TemporalControl().resetSerial == 1u);
    CHECK(latch.Take(TemporalControl().resetSerial));
    CHECK_FALSE(latch.Take(TemporalControl().resetSerial)); // the next frame: nothing new
    CHECK_FALSE(latch.Take(TemporalControl().resetSerial));

    // Two requests between frames are one reset.
    RequestTemporalReset(false, 0.0f, 0.0f);
    RequestTemporalReset(false, 0.0f, 0.0f);
    CHECK(latch.Take(TemporalControl().resetSerial));
    CHECK_FALSE(latch.Take(TemporalControl().resetSerial));

    // A second backend (a graphics reset) sees the last request once.
    TemporalResetLatch other;
    CHECK(other.Take(TemporalControl().resetSerial));
    CHECK_FALSE(other.Take(TemporalControl().resetSerial));

    // The serial never comes back to 0 ("never requested").
    TemporalControl().resetSerial = std::numeric_limits<std::uint32_t>::max();
    RequestTemporalReset(false, 0.0f, 0.0f);
    CHECK(TemporalControl().resetSerial == 1u);

    NoteTemporalResetApplied(7u, 1234u);
    CHECK(TemporalControl().appliedSerial == 7u);
    CHECK(TemporalControl().appliedFrame == 1234u);
}

TEST_CASE("Temporal determinism: the mist hold and its release", "[remaster][TemporalDeterminism]")
{
    FreshControl fresh;
    RequestTemporalReset(true, 1830.0f, -970.5f);
    float x = 0.0f, z = 0.0f;
    REQUIRE(TemporalMistHeld(x, z));
    CHECK(x == 1830.0f);
    CHECK(z == -970.5f);
    // A reset with a new offset moves the hold; one without releases it.
    RequestTemporalReset(true, 1.0f, 2.0f);
    REQUIRE(TemporalMistHeld(x, z));
    CHECK(x == 1.0f);
    CHECK(z == 2.0f);
    RequestTemporalReset(false, 5.0f, 6.0f);
    CHECK_FALSE(TemporalMistHeld(x, z));
    CHECK(TemporalControl().mist[0] == 0.0f);
    CHECK(TemporalControl().mist[1] == 0.0f);
}

TEST_CASE("Temporal determinism: the flag and the seed note", "[remaster][TemporalDeterminism]")
{
    FreshControl fresh;
    SetTemporalDeterministic(true);
    CHECK(TemporalDeterministic());
    SetTemporalDeterministic(false);
    CHECK_FALSE(TemporalDeterministic());
    NoteRandomSeed(1);
    CHECK(TemporalControl().seeded);
    CHECK(TemporalControl().seed == 1);
    // The flag does not request a reset, and a reset does not set the flag.
    SetTemporalDeterministic(true);
    CHECK(TemporalControl().resetSerial == 0u);
    SetTemporalDeterministic(false);
    RequestTemporalReset(false, 0.0f, 0.0f);
    CHECK_FALSE(TemporalDeterministic());
}

TEST_CASE("Temporal determinism: readbacks are read at a fixed age", "[remaster][TemporalDeterminism]")
{
    STATIC_REQUIRE(kDeterministicReadbackLag == 3);
    // Queued at frame 10: due from frame 13 on, never before.
    CHECK_FALSE(DeterministicReadbackDue(10u, 10u));
    CHECK_FALSE(DeterministicReadbackDue(10u, 11u));
    CHECK_FALSE(DeterministicReadbackDue(10u, 12u));
    CHECK(DeterministicReadbackDue(10u, 13u));
    CHECK(DeterministicReadbackDue(10u, 20u));
    // A slot "from the future" (a counter reset) is never due.
    CHECK_FALSE(DeterministicReadbackDue(14u, 13u));
    // Across the frame counter's wrap.
    CHECK(DeterministicReadbackDue(0xfffffffeu, 1u));
    CHECK_FALSE(DeterministicReadbackDue(0xffffffffu, 1u));
    // Another lag.
    CHECK(DeterministicReadbackDue(10u, 11u, 1));
    CHECK_FALSE(DeterministicReadbackDue(10u, 11u, 2));

    // The camera's ring (4 slots, a meter every frame): at frame k the meter
    // of k - 3 is read and k - 2, k - 1 stay in flight, so the ring never
    // fills and no meter is skipped; the reflections' ring (3 slots) reads
    // k - 3 into the slot it then writes.
    for (std::uint32_t k = 100; k < 110; ++k)
    {
        int due = 0;
        for (std::uint32_t q = k - 3; q < k; ++q)
            due += DeterministicReadbackDue(q, k) ? 1 : 0;
        CHECK(due == 1);
    }
}

TEST_CASE("Temporal determinism: the adaptation step and the reset exposure", "[remaster][TemporalDeterminism]")
{
    CHECK(DeterministicAdaptStep(0.0f) == Catch::Approx(1.0f / 60.0f));
    CHECK(DeterministicAdaptStep(-1.0f) == Catch::Approx(1.0f / 60.0f));
    CHECK(DeterministicAdaptStep(1.0f / 30.0f) == Catch::Approx(1.0f / 30.0f));
    CHECK(DeterministicAdaptStep(0.3f) == Catch::Approx(0.3f));

    // Whole stops; two runs whose exposures differ in the last bits get one.
    CHECK(ResetExposureLog2(2.4f) == 2.0f);
    CHECK(ResetExposureLog2(2.6f) == 3.0f);
    CHECK(ResetExposureLog2(-1.6f) == -2.0f);
    CHECK(ResetExposureLog2(-0.4f) == 0.0f);
    CHECK(ResetExposureLog2(3.0f) == 3.0f);
    CHECK(ResetExposureLog2(5.21f) == ResetExposureLog2(5.2101f));
    CHECK(ResetExposureLog2(std::numeric_limits<float>::quiet_NaN()) == 0.0f);
    CHECK(ResetExposureLog2(std::numeric_limits<float>::infinity()) == 0.0f);
}

TEST_CASE("Temporal determinism: the reflections' probe frames", "[remaster][TemporalDeterminism]")
{
    // Flag off: the engine frame counter alone, as before (frame % period).
    for (std::uint32_t frame = 0; frame < 64; ++frame)
        for (std::uint32_t base : {0u, 3u, 17u})
            CHECK(SsrShareProbeDue(frame, base, 8, false) == (frame % 8u == 0u));
    // Flag on: counted from the last reset.
    CHECK(SsrShareProbeDue(5u, 5u, 8, true));
    CHECK_FALSE(SsrShareProbeDue(8u, 5u, 8, true));
    CHECK(SsrShareProbeDue(13u, 5u, 8, true));
    CHECK(SsrShareProbeDue(21u, 5u, 8, true));
    // A period of 1 (or less) probes every frame.
    CHECK(SsrShareProbeDue(7u, 0u, 1, false));
    CHECK(SsrShareProbeDue(7u, 0u, 0, true));
}

TEST_CASE("Temporal determinism: the rain streak phase from the sim clock", "[remaster][TemporalDeterminism]")
{
    CHECK(RainStreakPhase(0) == 0.0f);
    CHECK(RainStreakPhase(1000) == 0.0f);
    CHECK(RainStreakPhase(2600123) == Catch::Approx(0.123f));
    CHECK(RainStreakPhase(999) == Catch::Approx(0.999f));
    CHECK(RainStreakPhase(-250) == Catch::Approx(0.75f));
    for (int ms : {0, 17, 999, 1000, 123456, -1, -999})
    {
        const float p = RainStreakPhase(ms);
        CHECK(p >= 0.0f);
        CHECK(p < 1.0f);
    }
}

TEST_CASE("Temporal determinism audit: play keeps its readbacks, clocks and drift",
          "[remaster][TemporalDeterminism][StateCacheAudit]")
{
    // The camera meter: in play every passed meter is collected without a
    // wait (zero timeout) and adaptation runs on the wall clock; the wait and
    // the fixed step sit inside the deterministic branches only.
    const std::string camera = ReadTextFile(Gl33Dir() / "EngineGL33_Camera.cpp");
    REQUIRE_FALSE(camera.empty());
    const std::string adapt = FunctionBody(camera, "void EngineGL33::CameraAdapt()");
    REQUIRE_FALSE(adapt.empty());
    CHECK(FindSq(adapt, "const bool deterministic = rm::TemporalDeterministic();") != std::string::npos);
    const size_t detBranch = FindSq(adapt, "if (deterministic) {");
    const size_t wait = FindSq(adapt, "WaitFenceDeterministic(sync)");
    const size_t poll = FindSq(adapt, "glClientWaitSync(sync, 0, 0)");
    REQUIRE(detBranch != std::string::npos);
    REQUIRE(wait != std::string::npos);
    REQUIRE(poll != std::string::npos);
    CHECK(detBranch < wait);
    CHECK(wait < poll); // the poll is the else branch
    CHECK(FindSq(adapt, "const double now = NowSeconds();") != std::string::npos);
    CHECK(FindSq(adapt, "if (deterministic) dt = rm::DeterministicAdaptStep(Poseidon::FixedSimStep());") !=
          std::string::npos);
    CHECK(CountOf(adapt, "DropTemporalHistories();") == 1);
    CHECK(adapt.find("if (_temporalDet.redropOnSnap)") != std::string::npos);

    // The reflections' shares: the same split.
    const std::string ssr = ReadTextFile(Gl33Dir() / "EngineGL33_PostSsr.cpp");
    REQUIRE_FALSE(ssr.empty());
    const std::string shares = FunctionBody(ssr, "void EngineGL33::ReadSsrShares()");
    REQUIRE_FALSE(shares.empty());
    const size_t ssrWait = FindSq(shares, "WaitFenceDeterministic(s.fence[i])");
    const size_t ssrPoll = FindSq(shares, "glClientWaitSync(s.fence[i], 0, 0)");
    REQUIRE(ssrWait != std::string::npos);
    REQUIRE(ssrPoll != std::string::npos);
    CHECK(FindSq(shares, "if (deterministic)") < ssrWait);
    CHECK(ssrWait < ssrPoll);

    // Nothing else waits on a fence: the one blocking wait is the helper, and
    // only the two deterministic branches call it; every other client wait in
    // the backend polls with a zero timeout.
    int helperCalls = 0, flushWaits = 0, zeroPolls = 0, clientWaits = 0;
    for (const auto& entry : std::filesystem::directory_iterator(Gl33Dir()))
    {
        const auto ext = entry.path().extension();
        if (ext != ".cpp" && ext != ".hpp")
            continue;
        const std::string text = ReadTextFile(entry.path());
        helperCalls += CountOf(text, "WaitFenceDeterministic(");
        flushWaits += CountOf(text, "GL_SYNC_FLUSH_COMMANDS_BIT");
        for (size_t at = text.find("glClientWaitSync("); at != std::string::npos;
             at = text.find("glClientWaitSync(", at + 1))
        {
            ++clientWaits;
            const size_t close = text.find(')', at);
            const std::string call = close == std::string::npos ? std::string() : Squash(text.substr(at, close - at));
            if (call.size() >= 4 && call.compare(call.size() - 4, 4, ",0,0") == 0)
                ++zeroPolls;
        }
    }
    CHECK(helperCalls == 3);             // the definition, the camera, the reflections
    CHECK(flushWaits == 1);              // inside the helper
    CHECK(clientWaits == zeroPolls + 1); // every wait but the helper's polls
    CHECK(zeroPolls >= 2);
    const std::string postPass = ReadTextFile(Gl33Dir() / "EngineGL33_PostPass.hpp");
    CHECK(
        FunctionBody(postPass, "inline bool WaitFenceDeterministic(GLsync sync)").find("GL_SYNC_FLUSH_COMMANDS_BIT") !=
        std::string::npos);

    // The reflections' probe frames: the engine counter in play.
    const std::string post = ReadTextFile(Gl33Dir() / "EngineGL33_PostFX.cpp");
    REQUIRE_FALSE(post.empty());
    const std::string end = FunctionBody(post, "void EngineGL33::EndWorldScene()");
    REQUIRE_FALSE(end.empty());
    CHECK(FindSq(end, "rm::SsrShareProbeDue(GetFrameCounter(), _temporalDet.frameBase, rm::kSsrShareProbeFrames, "
                      "rm::TemporalDeterministic())") != std::string::npos);
    CHECK(FindSq(end, "GetFrameCounter() % static_cast<std::uint32_t>(rm::kSsrShareProbeFrames)") == std::string::npos);

    // The mist drift: integrated with the wind unless a capture holds it.
    const std::string fog = ReadTextFile(Gl33Dir() / "EngineGL33_PostFog.cpp");
    REQUIRE_FALSE(fog.empty());
    const size_t held = FindSq(fog, "if (rm::TemporalMistHeld(held[0], held[1]))");
    const size_t integrate = FindSq(fog, "else if (_fog.driftTime >= 0.0f)");
    REQUIRE(held != std::string::npos);
    REQUIRE(integrate != std::string::npos);
    CHECK(held < integrate);
    CHECK(FindSq(fog, "drift[0] -= wx * dt;", integrate) != std::string::npos);
}

TEST_CASE("Temporal determinism audit: a reset is applied once, only on request",
          "[remaster][TemporalDeterminism][StateCacheAudit]")
{
    const std::string temporal = ReadTextFile(Gl33Dir() / "EngineGL33_PostTemporal.cpp");
    REQUIRE_FALSE(temporal.empty());
    const std::string reset = FunctionBody(temporal, "void EngineGL33::ApplyTemporalReset()");
    REQUIRE_FALSE(reset.empty());
    // The latch first: no request, no change.
    const size_t take = FindSq(reset, "if (!_temporalDet.latch.Take(tc.resetSerial)) return;");
    REQUIRE(take != std::string::npos);
    for (const char* write : {"_taa.sequence = 0;", "_fog.sequence = 0;", "DropTemporalHistories();",
                              "_camera.snap = true;", "_camera.snapFrame = frame;"})
    {
        const size_t at = FindSq(reset, write);
        CHECK(at != std::string::npos);
        CHECK(at > take);
    }
    // The exposure is rounded only in deterministic mode.
    CHECK(FindSq(reset, "if (tc.deterministic) _camera.exposureLog2 = rm::ResetExposureLog2(_camera.exposureLog2);") !=
          std::string::npos);
    const std::string drop = FunctionBody(temporal, "void EngineGL33::DropTemporalHistories()");
    CHECK(drop.find("_taa.history.exists = false;") != std::string::npos);
    CHECK(drop.find("_fog.historyValid = false;") != std::string::npos);
    CHECK(drop.find("_surfAoHist.exists = false;") != std::string::npos);

    // Applied at the first of the frame's passes that reads temporal state:
    // first statement of each of the three.
    struct Site
    {
        const char* file;
        const char* signature;
    };
    for (const Site& site : {Site{"EngineGL33_PostTemporal.cpp", "void EngineGL33::ApplyTaaJitter()"},
                             Site{"EngineGL33_Camera.cpp", "void EngineGL33::UpdateCameraFrame("},
                             Site{"EngineGL33_PostFX.cpp", "void EngineGL33::EndWorldScene()"}})
    {
        const std::string body = FunctionBody(ReadTextFile(Gl33Dir() / site.file), site.signature);
        INFO(site.signature);
        REQUIRE_FALSE(body.empty());
        const std::string sq = Squash(body);
        const size_t open = sq.find('{');
        REQUIRE(open != std::string::npos);
        CHECK(sq.compare(open + 1, 21, "ApplyTemporalReset();") == 0);
    }
    int sites = 0;
    for (const auto& entry : std::filesystem::directory_iterator(Gl33Dir()))
        if (entry.path().extension() == ".cpp")
            sites += CountOf(ReadTextFile(entry.path()), "    ApplyTemporalReset();");
    CHECK(sites == 3);
}

TEST_CASE("Temporal determinism audit: only the tri verbs set the controls",
          "[remaster][TemporalDeterminism][StateCacheAudit]")
{
    // The verbs exist and are linked into the game.
    const std::filesystem::path commands = EngineDir() / "Poseidon" / "Game" / "Commands";
    const std::string verbs = ReadTextFile(commands / "GameStateExtTestDeterminism.cpp");
    REQUIRE_FALSE(verbs.empty());
    for (const char* name :
         {"\"triDeterministic\"", "\"triResetTemporal\"", "\"triSeedRandom\"", "\"triTemporalState\""})
        CHECK(verbs.find(name) != std::string::npos);
    CHECK(verbs.find("GRandGen.SetSeed(seed);") != std::string::npos);
    CHECK(verbs.find("std::srand(") != std::string::npos);
    // M2's scratch form (a number) stays accepted: gen_golden.py calls it.
    CHECK(FindSq(verbs, "\"triResetTemporal\", TriResetTemporal, GameArray | GameScalar") != std::string::npos);
    const std::string audio = ReadTextFile(commands / "GameStateExtTestAudio.cpp");
    CHECK(CountOf(audio, "EnsureGameStateExtTestDeterminismLinked();") == 2); // declaration and call

    // No engine code outside the verbs sets them, so play can never turn
    // deterministic mode on, request a reset or hold the mist.
    int setters = 0;
    for (const auto& root : {EngineDir() / "Poseidon", EngineDir() / "PoseidonGL33"})
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
        {
            const auto ext = entry.path().extension();
            if (!entry.is_regular_file() || (ext != ".cpp" && ext != ".hpp"))
                continue;
            const std::string name = entry.path().filename().string();
            if (name == "GameStateExtTestDeterminism.cpp" || name == "TemporalDeterminism.cpp" ||
                name == "TemporalDeterminism.hpp")
                continue;
            const std::string text = ReadTextFile(entry.path());
            const int n = CountOf(text, "RequestTemporalReset(") + CountOf(text, "SetTemporalDeterministic(") +
                          CountOf(text, "NoteRandomSeed(");
            if (n != 0)
            {
                INFO(entry.path().string());
                CHECK(n == 0);
            }
            setters += n;
        }
    }
    CHECK(setters == 0);

    // The rain streak hook (lane L4, SceneDraw.cpp), when present, reads the
    // flag before it changes the phase.
    const std::string scene = ReadTextFile(EngineDir() / "Poseidon" / "World" / "Scene" / "SceneDraw.cpp");
    REQUIRE_FALSE(scene.empty());
    if (scene.find("RainStreakPhase(") != std::string::npos)
    {
        const std::string rain = FunctionBody(scene, "void Scene::DrawRain()");
        const size_t guard = FindSq(rain, "if (remaster::TemporalDeterministic())");
        const size_t phase = FindSq(rain, "yOffset = -remaster::RainStreakPhase(Glob.time.toInt());");
        REQUIRE(guard != std::string::npos);
        REQUIRE(phase != std::string::npos);
        CHECK(guard < phase);
        CHECK(FindSq(rain, "float yOffset = -fastFmod(rainOffset, 1);") < guard);
    }
}
