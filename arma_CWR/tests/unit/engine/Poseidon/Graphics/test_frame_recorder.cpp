#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Shared/FrameRecorder.hpp>
#include <Poseidon/World/TriCamPath.hpp>

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// P0-10 real-time recorder and camera path (Graphics/Shared/FrameRecorder.hpp,
// World/TriCamPath.hpp; verbs in Game/Commands/GameStateExtTestRecord.cpp,
// the GL half in PoseidonGL33/EngineGL33_Recorder.cpp): the arm and stop
// rules, the frame packing, the camera path evaluation, and source audits
// that the recorder reads the pixels triScreenshot reads at the same point
// and that play never arms either.

namespace fr = Poseidon::framerec;
namespace tc = Poseidon::tricam;

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

std::string FunctionBody(const std::string& source, const std::string& signature)
{
    const size_t start = source.find(signature);
    if (start == std::string::npos)
        return {};
    const size_t end = source.find("\n}\n", start);
    return source.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

std::string Squash(const std::string& text)
{
    std::string out;
    out.reserve(text.size());
    for (const char c : text)
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
            out += c;
    return out;
}

void ResetRecorder()
{
    fr::Recorder() = fr::RecorderControl();
}

tc::CamKey Key(float t, float x, float y, float z, float h, float p, bool ease = false)
{
    tc::CamKey k;
    k.t = t;
    k.pos[0] = x;
    k.pos[1] = y;
    k.pos[2] = z;
    k.headingDeg = h;
    k.pitchDeg = p;
    k.ease = ease;
    return k;
}

} // namespace

TEST_CASE("P0-10: arming the recorder validates and starts a recording", "[FrameRecorder]")
{
    ResetRecorder();
    std::string why;
    CHECK_FALSE(fr::ArmRecorder("", 10, 0, &why));
    CHECK(why == "empty path");
    CHECK_FALSE(fr::ArmRecorder("a.raw", 0, 0, &why));
    CHECK_FALSE(fr::ArmRecorder("a.raw", -3, 0, &why));
    CHECK_FALSE(fr::ArmRecorder("a.raw", 10, 3, &why));
    CHECK_FALSE(fr::ArmRecorder("a.raw", 10, -1, &why));
    CHECK(fr::Recorder().state == fr::RecordState::Idle);
    CHECK(fr::Recorder().serial == 0);

    REQUIRE(fr::ArmRecorder("a.raw", 120, 1, &why));
    const fr::RecorderControl& r = fr::Recorder();
    CHECK(r.state == fr::RecordState::Recording);
    CHECK(r.mode == fr::RecordMode::ShotFramesOnly);
    CHECK(r.want == 120);
    CHECK(r.got == 0);
    CHECK(r.serial == 1);
    CHECK_FALSE(fr::ArmRecorder("b.raw", 10, 0, &why));
    CHECK(why == "busy");

    fr::RequestRecorderStop();
    CHECK(r.stopRequested);
    fr::MarkRecorderWritten(7);
    CHECK(r.state == fr::RecordState::Written);
    CHECK(r.got == 7);
    CHECK_FALSE(r.stopRequested);
    // a stop with nothing recording does nothing
    fr::RequestRecorderStop();
    CHECK_FALSE(r.stopRequested);

    REQUIRE(fr::ArmRecorder("c.raw", 5, 2, &why));
    CHECK(r.serial == 2);
    fr::MarkRecorderFailed("write failed");
    CHECK(r.state == fr::RecordState::Failed);
    CHECK(r.failReason == "write failed");
    CHECK(std::string(fr::RecordStateName(r.state)) == "failed");
    // a new recording clears the failure
    REQUIRE(fr::ArmRecorder("d.raw", 5, 0, &why));
    CHECK(r.failReason.empty());
    ResetRecorder();
}

TEST_CASE("P0-10: which frames a mode records and their size", "[FrameRecorder]")
{
    CHECK(fr::RecordsFrame(fr::RecordMode::EveryFrame, false));
    CHECK(fr::RecordsFrame(fr::RecordMode::EveryFrame, true));
    CHECK_FALSE(fr::RecordsFrame(fr::RecordMode::ShotFramesOnly, false));
    CHECK(fr::RecordsFrame(fr::RecordMode::ShotFramesOnly, true));
    CHECK(fr::RecordsFrame(fr::RecordMode::EveryFrameHalf, false));
    CHECK(fr::OutWidth(fr::RecordMode::EveryFrame, 1920) == 1920);
    CHECK(fr::OutHeight(fr::RecordMode::ShotFramesOnly, 1080) == 1080);
    CHECK(fr::OutWidth(fr::RecordMode::EveryFrameHalf, 1921) == 960);
    CHECK(fr::OutHeight(fr::RecordMode::EveryFrameHalf, 1080) == 540);
    CHECK(fr::RecordBytes(fr::RecordMode::EveryFrame, 1920, 1080, 900) == 1920ull * 1080ull * 3ull * 900ull);
    CHECK(fr::RecordBytes(fr::RecordMode::EveryFrameHalf, 1920, 1080, 2) == 960ull * 540ull * 3ull * 2ull);
    CHECK(fr::RecordBytes(fr::RecordMode::EveryFrame, 0, 1080, 2) == 0);
    // a 15 s clip at 1080p fits; an hour does not
    CHECK(fr::RecordBytes(fr::RecordMode::EveryFrame, 1920, 1080, 900) <= fr::kMaxRecordBytes);
    CHECK(fr::RecordBytes(fr::RecordMode::EveryFrame, 1920, 1080, 216000) > fr::kMaxRecordBytes);
    CHECK(fr::SidecarHeader(1920, 1080, 300, fr::RecordMode::EveryFrame) ==
          "w=1920 h=1080 frames=300 mode=0 half=0 rows=bottom-up format=rgb24");
    CHECK(fr::SidecarHeader(960, 540, 3, fr::RecordMode::EveryFrameHalf) ==
          "w=960 h=540 frames=3 mode=2 half=1 rows=bottom-up format=rgb24");
}

TEST_CASE("P0-10: a full-size frame keeps every RGB byte and the row order", "[FrameRecorder]")
{
    const int w = 3, h = 2;
    std::vector<std::uint8_t> rgba(w * h * 4);
    for (size_t i = 0; i < rgba.size(); ++i)
        rgba[i] = static_cast<std::uint8_t>(i * 7 + 1);
    std::vector<std::uint8_t> rgb(w * h * 3, 0xEE);
    fr::PackFrame(rgba.data(), w, h, false, rgb.data());
    for (int p = 0; p < w * h; ++p)
        for (int c = 0; c < 3; ++c)
            CHECK(rgb[p * 3 + c] == rgba[p * 4 + c]);
}

TEST_CASE("P0-10: a half-size frame is the rounded 2x2 box mean", "[FrameRecorder]")
{
    const int w = 4, h = 2; // -> 2 x 1
    std::vector<std::uint8_t> rgba(w * h * 4, 0);
    auto px = [&](int x, int y, std::uint8_t r, std::uint8_t g, std::uint8_t b)
    {
        std::uint8_t* d = rgba.data() + (y * w + x) * 4;
        d[0] = r;
        d[1] = g;
        d[2] = b;
        d[3] = 255;
    };
    px(0, 0, 0, 255, 1);
    px(1, 0, 1, 255, 1);
    px(0, 1, 1, 255, 1);
    px(1, 1, 0, 255, 2);
    px(2, 0, 10, 20, 30);
    px(3, 0, 10, 20, 30);
    px(2, 1, 10, 20, 30);
    px(3, 1, 11, 20, 30);
    std::vector<std::uint8_t> rgb(2 * 1 * 3, 0xEE);
    fr::PackFrame(rgba.data(), w, h, true, rgb.data());
    CHECK(rgb[0] == 1);   // (0 + 1 + 1 + 0 + 2) / 4
    CHECK(rgb[1] == 255); // (1020 + 2) / 4
    CHECK(rgb[2] == 1);   // (1 + 1 + 1 + 2 + 2) / 4
    CHECK(rgb[3] == 10);  // (41 + 2) / 4
    CHECK(rgb[4] == 20);
    CHECK(rgb[5] == 30);
}

TEST_CASE("P0-10: camera path keys must be ordered", "[FrameRecorder]")
{
    std::string why;
    CHECK_FALSE(tc::ValidatePath({}, &why));
    CHECK(tc::ValidatePath({Key(0, 1, 2, 3, 0, 0)}, &why));
    CHECK_FALSE(tc::ValidatePath({Key(1, 0, 0, 0, 0, 0), Key(1, 0, 0, 0, 0, 0)}, &why));
    CHECK(why == "key times must increase");
    CHECK_FALSE(tc::ValidatePath({Key(2, 0, 0, 0, 0, 0), Key(1, 0, 0, 0, 0, 0)}, &why));
    CHECK(tc::PathDuration({Key(0.5f, 0, 0, 0, 0, 0), Key(8.5f, 0, 0, 0, 0, 0)}) == Catch::Approx(8.0f));
}

TEST_CASE("P0-10: camera path evaluation holds, interpolates and eases", "[FrameRecorder]")
{
    const std::vector<tc::CamKey> keys = {Key(0, 100, 50, 200, 28, -6), Key(2, 110, 50, 220, 48, -5)};
    tc::CamPose before = tc::EvaluatePath(keys, -1.0f);
    CHECK(before.pos[0] == Catch::Approx(100));
    CHECK(before.pos[2] == Catch::Approx(200));
    tc::CamPose mid = tc::EvaluatePath(keys, 1.0f);
    CHECK(mid.pos[0] == Catch::Approx(105));
    CHECK(mid.pos[2] == Catch::Approx(210));
    tc::CamPose after = tc::EvaluatePath(keys, 5.0f);
    CHECK(after.pos[0] == Catch::Approx(110));
    CHECK(after.pos[2] == Catch::Approx(220));

    // the eased segment: smoothstep(0.25) = 0.15625
    const std::vector<tc::CamKey> eased = {Key(0, 0, 0, 0, 0, 0), Key(4, 64, 0, 0, 0, 0, true)};
    CHECK(tc::EvaluatePath(eased, 1.0f).pos[0] == Catch::Approx(10.0f));
    CHECK(tc::EvaluatePath(eased, 2.0f).pos[0] == Catch::Approx(32.0f));

    // three keys: the second segment
    const std::vector<tc::CamKey> three = {Key(0, 0, 0, 0, 0, 0), Key(1, 10, 0, 0, 0, 0), Key(3, 10, 20, 0, 0, 0)};
    CHECK(tc::EvaluatePath(three, 2.0f).pos[1] == Catch::Approx(10.0f));
    CHECK(tc::EvaluatePath(three, 2.0f).pos[0] == Catch::Approx(10.0f));
}

TEST_CASE("P0-10: camera path direction follows triSetView's setDir convention", "[FrameRecorder]")
{
    auto dir = [](float h, float p) { return tc::EvaluatePath({Key(0, 0, 0, 0, h, p)}, 0.0f); };
    tc::CamPose north = dir(0, 0);
    CHECK(north.dir[0] == Catch::Approx(0).margin(1e-6));
    CHECK(north.dir[2] == Catch::Approx(1));
    tc::CamPose east = dir(90, 0);
    CHECK(east.dir[0] == Catch::Approx(1));
    CHECK(east.dir[2] == Catch::Approx(0).margin(1e-6));
    tc::CamPose up = dir(0, 30);
    CHECK(up.dir[1] == Catch::Approx(0.5));
    CHECK(up.dir[2] == Catch::Approx(std::cos(3.14159265358979 / 6)));
}

TEST_CASE("P0-10: the camera path clock follows the simulation step", "[FrameRecorder]")
{
    tc::CamPose pose;
    tc::StopCamPath();
    CHECK_FALSE(tc::StepCamPath(1.0f / 60.0f, pose));
    std::string why;
    REQUIRE(tc::SetCamPath({Key(0, 0, 0, 0, 0, 0), Key(1, 60, 0, 0, 0, 0)}, &why));
    CHECK(tc::CamPathActive());
    CHECK(tc::CamPathTime() == 0.0f);
    for (int i = 0; i < 30; ++i)
        REQUIRE(tc::StepCamPath(1.0f / 60.0f, pose));
    CHECK(tc::CamPathTime() == Catch::Approx(0.5f));
    CHECK(pose.pos[0] == Catch::Approx(30.0f));
    // a zero step (setAccTime 0) holds the pose
    REQUIRE(tc::StepCamPath(0.0f, pose));
    CHECK(pose.pos[0] == Catch::Approx(30.0f));
    tc::StopCamPath();
    CHECK_FALSE(tc::CamPathActive());
    CHECK_FALSE(tc::SetCamPath({}, &why));
}

TEST_CASE("P0-10 audit: the recorder reads the presented frame where triScreenshot does", "[FrameRecorder]")
{
    const std::string vb = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_VertexBuffer.cpp");
    REQUIRE_FALSE(vb.empty());
    const std::string body = Squash(FunctionBody(vb, "void EngineGL33::BackToFront()"));
    REQUIRE_FALSE(body.empty());
    const size_t flag = body.find("constboolscreenshotFrame=_pendingScreenshotPath.GetLength()>0;");
    const size_t shot = body.find("CaptureScreenshotIfPending();");
    const size_t rec = body.find("GL33RecordPresentedFrame(_sdlWindow,_w,_h,screenshotFrame);");
    const size_t swap = body.find("SDL_GL_SwapWindow(");
    const size_t gamma = body.find("ApplyGammaPass();");
    const size_t overlay = body.find("DebugOverlay::Render();");
    REQUIRE(flag != std::string::npos);
    REQUIRE(shot != std::string::npos);
    REQUIRE(rec != std::string::npos);
    REQUIRE(swap != std::string::npos);
    CHECK(gamma < flag);
    CHECK(overlay < flag);
    CHECK(flag < shot);
    CHECK(shot < rec);
    CHECK(rec < swap);

    // the same read as CaptureScreenshotIfPending: default framebuffer, RGBA8
    const std::string capture = Squash(FunctionBody(vb, "void EngineGL33::CaptureScreenshotIfPending()"));
    CHECK(capture.find("glBindFramebuffer(GL_READ_FRAMEBUFFER,0);") != std::string::npos);
    CHECK(capture.find("glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,") != std::string::npos);
    const std::string recSrc = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_Recorder.cpp");
    REQUIRE_FALSE(recSrc.empty());
    const std::string frame = Squash(FunctionBody(recSrc, "bool screenshotFrame)\n{"));
    REQUIRE_FALSE(frame.empty());
    CHECK(frame.find("glBindFramebuffer(GL_READ_FRAMEBUFFER,0);") != std::string::npos);
    CHECK(frame.find("glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);") != std::string::npos);
    CHECK(frame.find("SDL_GetWindowSizeInPixels(window,&w,&h);") != std::string::npos);
    // play: the idle test comes first
    const size_t idle = frame.find("if(rc.state!=fr::RecordState::Recording)return;");
    REQUIRE(idle != std::string::npos);
    CHECK(idle < frame.find("glBindFramebuffer"));
    // the pack buffer is unbound again before the frame goes on
    CHECK(frame.rfind("glBindBuffer(GL_PIXEL_PACK_BUFFER,0);") > frame.find("glReadPixels("));
}

TEST_CASE("P0-10 audit: the camera path drives the view override and only the verbs set state", "[FrameRecorder]")
{
    const std::string world = Squash(ReadTextFile(EngineDir() / "Poseidon" / "World" / "World.cpp"));
    REQUIRE_FALSE(world.empty());
    const size_t step = world.find("Poseidon::tricam::StepCamPath(deltaT,pathPose)");
    const size_t apply = world.find("if(s_triViewActive){transform=s_triViewTransform;}");
    REQUIRE(step != std::string::npos);
    REQUIRE(apply != std::string::npos);
    CHECK(step < apply);

    const std::string verbs =
        ReadTextFile(EngineDir() / "Poseidon" / "Game" / "Commands" / "GameStateExtTestRecord.cpp");
    REQUIRE_FALSE(verbs.empty());
    CHECK(verbs.find("\"triRecord\", TriRecord, GameArray") != std::string::npos);
    CHECK(verbs.find("\"triRecordState\", TriRecordState") != std::string::npos);
    CHECK(verbs.find("\"triCamPath\", TriCamPath, GameArray") != std::string::npos);
    const std::string audio =
        ReadTextFile(EngineDir() / "Poseidon" / "Game" / "Commands" / "GameStateExtTestAudio.cpp");
    CHECK(audio.find("\n    EnsureGameStateExtTestRecordLinked();\n") != std::string::npos);

    // no engine file other than the verbs arms the recorder or sets a path
    const std::filesystem::path verbsFile = std::filesystem::weakly_canonical(
        EngineDir() / "Poseidon" / "Game" / "Commands" / "GameStateExtTestRecord.cpp");
    int offenders = 0;
    for (const auto& e : std::filesystem::recursive_directory_iterator(EngineDir()))
    {
        if (!e.is_regular_file())
            continue;
        const std::string ext = e.path().extension().string();
        if (ext != ".cpp" && ext != ".hpp" && ext != ".inc")
            continue;
        if (std::filesystem::weakly_canonical(e.path()) == verbsFile)
            continue;
        const std::string name = e.path().filename().string();
        if (name.rfind("FrameRecorder.", 0) == 0 || name.rfind("TriCamPath.", 0) == 0)
            continue;
        const std::string src = ReadTextFile(e.path());
        if (src.find("ArmRecorder(") != std::string::npos || src.find("SetCamPath(") != std::string::npos ||
            src.find("RequestRecorderStop(") != std::string::npos)
        {
            ++offenders;
            UNSCOPED_INFO("sets recorder or camera path state: " << e.path().string());
        }
    }
    CHECK(offenders == 0);
}
