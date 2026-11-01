// P0-10 motion clip verbs (Graphics/Shared/FrameRecorder.hpp holds the
// recorder state, PoseidonGL33/EngineGL33_Recorder.cpp reads the frames,
// World/TriCamPath.hpp holds the camera path World::Simulate steps):
//
//   triRecord [path, nFrames, mode]   arm the real-time recorder: the next
//                                     nFrames presented frames (mode 0), the
//                                     next nFrames triScreenshot frames
//                                     (mode 1) or every presented frame at
//                                     half size (mode 2), read at the
//                                     triScreenshot point, written as raw
//                                     RGB24 (rows bottom-up) + <path>.txt;
//                                     [path, 0, mode] while recording stops
//                                     and writes what is recorded
//   triRecordState                    "FAIL:recording;..." while recording
//                                     (a top-level statement waits on it),
//                                     else "OK:<state>;..."
//   triCamPath [[t, east, upASL, north, heading, pitch(, ease)], ...]
//                                     start a camera path (its clock follows
//                                     the simulation step); [] stops it
//
// The clip recipe: pin the view, settle, triSetFixedStep (1/60) with
// setAccTime 1 (one render per sim tick), the P0-6 reset when the dist has
// it, triCamPath, triRecord, then triRecordState at top level until the file
// is written. Play never calls these.

#include <Evaluator/express.hpp>
using namespace Poseidon;
#include <Poseidon/Foundation/Common/FltOpts.hpp>
#include <Poseidon/Foundation/Logging/Logging.hpp>
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Graphics/Shared/FrameRecorder.hpp>
#include <Poseidon/World/TriCamPath.hpp>

#include <cstdio>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

std::string GetTriOutputDir(); // GameStateExtTestRender.cpp: TRI_OUTPUT_DIR or the default

namespace fr = Poseidon::framerec;
namespace tc = Poseidon::tricam;

namespace
{

/// triRecord [path, nFrames, mode]: arms the recorder (see the header). A
/// relative path is taken under the tri output dir (TRI_OUTPUT_DIR), where
/// the screenshots go; its folder is created. [path, 0, mode] while a
/// recording runs asks it to write what it has. Returns "OK:armed <serial>
/// <path>", "OK:stopping" or "FAIL:<why>".
GameValue TriRecord(const GameState* /*state*/, GameValuePar arg)
{
    if (arg.GetType() != GameArray)
        return GameValue("FAIL:expected [path, nFrames, mode]");
    const GameArrayType& a = arg;
    if (a.Size() != 3 || a[0].GetType() != GameString || a[1].GetType() != GameScalar || a[2].GetType() != GameScalar)
        return GameValue("FAIL:expected [path, nFrames, mode]");
    const int frames = toInt(static_cast<float>(static_cast<GameScalarType>(a[1])));
    const int mode = toInt(static_cast<float>(static_cast<GameScalarType>(a[2])));
    if (frames == 0 && fr::Recorder().state == fr::RecordState::Recording)
    {
        fr::RequestRecorderStop();
        LOG_INFO(Core, "[tri] triRecord stop requested at {} of {} frames", fr::Recorder().got, fr::Recorder().want);
        return GameValue("OK:stopping");
    }
    const GameStringType ps = static_cast<GameStringType>(a[0]);
    std::filesystem::path path((const char*)ps);
    if (!path.empty() && path.is_relative())
        path = std::filesystem::path(GetTriOutputDir()) / path;
    std::error_code ec;
    if (path.has_parent_path())
        std::filesystem::create_directories(path.parent_path(), ec);
    std::string why;
    if (!fr::ArmRecorder(path.generic_string(), frames, mode, &why))
    {
        LOG_ERROR(Core, "[tri] triRecord refused: {}", why);
        return GameValue(("FAIL:" + why).c_str());
    }
    LOG_INFO(Core, "[tri] triRecord armed {} frames={} mode={}", fr::Recorder().path, frames, mode);
    char buf[640];
    snprintf(buf, sizeof(buf), "OK:armed %u %s", fr::Recorder().serial, fr::Recorder().path.c_str());
    return GameValue(buf);
}

/// triRecordState: "FAIL:recording;got=<n>;want=<n>;pathT=<s>" while a
/// recording runs (tri retries a top-level FAIL, so the statement waits for
/// the file), otherwise "OK:<idle|written|failed>;got=..;want=..;pathT=..;
/// serial=..;path=.." with ";why=<reason>" after a failure.
GameValue TriRecordState(const GameState* /*state*/)
{
    const fr::RecorderControl& r = fr::Recorder();
    char buf[768];
    if (r.state == fr::RecordState::Recording)
    {
        snprintf(buf, sizeof(buf), "FAIL:recording;got=%d;want=%d;pathT=%.4f", r.got, r.want,
                 static_cast<double>(tc::CamPathTime()));
        return GameValue(buf);
    }
    snprintf(buf, sizeof(buf), "OK:%s;got=%d;want=%d;pathT=%.4f;serial=%u;path=%s%s%s", fr::RecordStateName(r.state),
             r.got, r.want, static_cast<double>(tc::CamPathTime()), r.serial, r.path.c_str(),
             r.state == fr::RecordState::Failed ? ";why=" : "",
             r.state == fr::RecordState::Failed ? r.failReason.c_str() : "");
    return GameValue(buf);
}

/// triCamPath [[t, east, upASL, north, heading, pitch(, ease)], ...]: starts a
/// camera path (World/TriCamPath.hpp), its clock at 0; [] stops it (the view
/// stays where the path left it). Returns "OK:keys=<n>;dur=<s>", "OK:stopped"
/// or "FAIL:<why>".
GameValue TriCamPath(const GameState* /*state*/, GameValuePar arg)
{
    if (arg.GetType() != GameArray)
        return GameValue("FAIL:expected [[t, east, upASL, north, heading, pitch(, ease)], ...] or []");
    const GameArrayType& a = arg;
    if (a.Size() == 0)
    {
        tc::StopCamPath();
        LOG_INFO(Core, "[tri] triCamPath stopped at t={}", tc::CamPathTime());
        return GameValue("OK:stopped");
    }
    std::vector<tc::CamKey> keys;
    keys.reserve(a.Size());
    for (int i = 0; i < a.Size(); ++i)
    {
        if (a[i].GetType() != GameArray)
            return GameValue("FAIL:every key is [t, east, upASL, north, heading, pitch(, ease)]");
        const GameArrayType& k = a[i];
        if (k.Size() != 6 && k.Size() != 7)
            return GameValue("FAIL:every key is [t, east, upASL, north, heading, pitch(, ease)]");
        float v[7] = {};
        for (int j = 0; j < k.Size(); ++j)
        {
            if (k[j].GetType() != GameScalar)
                return GameValue("FAIL:key values must be numbers");
            v[j] = static_cast<float>(static_cast<GameScalarType>(k[j]));
        }
        tc::CamKey key;
        key.t = v[0];
        key.pos[0] = v[1];
        key.pos[1] = v[2];
        key.pos[2] = v[3];
        key.headingDeg = v[4];
        key.pitchDeg = v[5];
        key.ease = v[6] != 0.0f;
        keys.push_back(key);
    }
    std::string why;
    if (!tc::SetCamPath(keys, &why))
        return GameValue(("FAIL:" + why).c_str());
    char buf[96];
    snprintf(buf, sizeof(buf), "OK:keys=%d;dur=%.4f", static_cast<int>(keys.size()),
             static_cast<double>(tc::PathDuration(keys)));
    LOG_INFO(Core, "[tri] triCamPath {}", buf + 3);
    return GameValue(buf);
}

} // namespace

// Called from GameStateExtTestAudio.cpp's INIT_MODULE to force this TU into
// the link (no other code references these verbs).
void EnsureGameStateExtTestRecordLinked() {}

INIT_MODULE(GameStateExtTestRecord, 3)
{
    GGameState.NewFunction(GameFunction(GameString, "triRecord", TriRecord, GameArray));
    GGameState.NewNularOp(GameNular(GameString, "triRecordState", TriRecordState));
    GGameState.NewFunction(GameFunction(GameString, "triCamPath", TriCamPath, GameArray));
};
