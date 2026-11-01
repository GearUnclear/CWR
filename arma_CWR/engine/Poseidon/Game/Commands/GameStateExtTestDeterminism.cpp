// P0-6 temporal determinism verbs for captures (Graphics/Remaster/
// TemporalDeterminism.hpp holds the state, the GL backend applies it):
//
//   triDeterministic true|false   readbacks at a fixed age, adaptation by the
//                                 fixed step, rain streaks by the sim clock
//   triResetTemporal [x, z] | []  reset the temporal state at the next frame;
//                                 [x, z] also holds the mist drift there, []
//                                 releases a hold
//   triResetTemporal n            M2's scratch form (gen_golden.py calls it):
//                                 [0, 0], then triSeedRandom n
//   triSeedRandom n               seed GRandGen and the C rand() stream
//   triTemporalState              "det=..;reset=..;applied=..@..;mist=..;seed=.."
//
// The capture recipe (plan 3.3): pin the view and the sim clock, settle,
// triSetFixedStep (1/60), triDeterministic true, triResetTemporal [x, z],
// triSeedRandom 1, then the same number of frames before every shot.
// Play never calls these: with the flag off and no reset requested every
// reader takes the path it took before.

#include <Evaluator/express.hpp>
using namespace Poseidon;
#include <Poseidon/Foundation/Common/FltOpts.hpp>
#include <Poseidon/Foundation/Logging/Logging.hpp>
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Graphics/Remaster/TemporalDeterminism.hpp>
#include <Random/randomGen.hpp>

#include <cstdio>
#include <cstdlib>

namespace rm = Poseidon::remaster;

namespace
{

void SeedRandomStreams(int seed)
{
    GRandGen.SetSeed(seed);
    std::srand(static_cast<unsigned>(seed));
    rm::NoteRandomSeed(seed);
}

/// triDeterministic <on>: deterministic temporal mode (see the header). Returns
/// "OK:1" or "OK:0", the state now.
GameValue TriDeterministic(const GameState* /*state*/, GameValuePar arg)
{
    const bool on = static_cast<bool>(arg);
    rm::SetTemporalDeterministic(on);
    LOG_INFO(Core, "[tri] triDeterministic {}", on ? 1 : 0);
    return GameValue(on ? "OK:1" : "OK:0");
}

/// triResetTemporal [x, z] or []: at the next frame the TAA jitter index and
/// the fog's frame sequence start at 0, the TAA, fog and surface-occlusion
/// histories are dropped, and the photoreal camera snaps to the first meter
/// of that frame (the histories are dropped again when the snap lands).
/// [x, z] holds the volumetric fog's mist drift at that offset (metres) until
/// a later [] releases it. A number n (M2's scratch form, which
/// gen_golden.py uses) is [0, 0] followed by triSeedRandom n. Returns
/// "OK:reset <serial> mist held <x> <z>", "OK:reset <serial> mist free" or
/// "FAIL:<why>".
GameValue TriResetTemporal(const GameState* /*state*/, GameValuePar arg)
{
    if (arg.GetType() == GameScalar)
    {
        const int seed = toInt(static_cast<float>(arg));
        rm::RequestTemporalReset(true, 0.0f, 0.0f);
        SeedRandomStreams(seed);
        char buf[96];
        snprintf(buf, sizeof(buf), "OK:reset %u mist held 0.00 0.00 seed %d", rm::TemporalControl().resetSerial, seed);
        LOG_INFO(Core, "[tri] triResetTemporal {}", buf + 3);
        return GameValue(buf);
    }
    if (arg.GetType() != GameArray)
        return GameValue("FAIL:expected [x, z] or []");
    const GameArrayType& a = arg;
    if (a.Size() != 0 && a.Size() != 2)
        return GameValue("FAIL:expected [x, z] or []");
    const bool hold = a.Size() == 2;
    float x = 0.0f, z = 0.0f;
    if (hold)
    {
        if (a[0].GetType() != GameScalar || a[1].GetType() != GameScalar)
            return GameValue("FAIL:expected [x, z] or []");
        x = static_cast<float>(static_cast<GameScalarType>(a[0]));
        z = static_cast<float>(static_cast<GameScalarType>(a[1]));
    }
    rm::RequestTemporalReset(hold, x, z);
    char buf[96];
    if (hold)
        snprintf(buf, sizeof(buf), "OK:reset %u mist held %.2f %.2f", rm::TemporalControl().resetSerial, x, z);
    else
        snprintf(buf, sizeof(buf), "OK:reset %u mist free", rm::TemporalControl().resetSerial);
    LOG_INFO(Core, "[tri] triResetTemporal {}", buf + 3);
    return GameValue(buf);
}

/// triSeedRandom <n>: seeds the engine's random stream (GRandGen: weather,
/// AI, effects) and the C library's rand() (the scripted particles,
/// Smokes.cpp CParticle). Returns "OK:<n>".
GameValue TriSeedRandom(const GameState* /*state*/, GameValuePar arg)
{
    const int seed = toInt(static_cast<float>(arg));
    SeedRandomStreams(seed);
    char buf[32];
    snprintf(buf, sizeof(buf), "OK:%d", seed);
    LOG_INFO(Core, "[tri] triSeedRandom {}", seed);
    return GameValue(buf);
}

/// triTemporalState: "det=<0|1>;reset=<serial>;applied=<serial>@<frame>;
/// mist=held <x> <z>|free;seed=<n>|none" (applied: the last reset the
/// backend has applied and the engine frame it applied at).
GameValue TriTemporalState(const GameState* /*state*/)
{
    const rm::TemporalControlState& s = rm::TemporalControl();
    char mist[48];
    if (s.mistHeld)
        snprintf(mist, sizeof(mist), "held %.2f %.2f", s.mist[0], s.mist[1]);
    else
        snprintf(mist, sizeof(mist), "free");
    char seed[24];
    if (s.seeded)
        snprintf(seed, sizeof(seed), "%d", s.seed);
    else
        snprintf(seed, sizeof(seed), "none");
    char buf[192];
    snprintf(buf, sizeof(buf), "det=%d;reset=%u;applied=%u@%u;mist=%s;seed=%s", s.deterministic ? 1 : 0, s.resetSerial,
             s.appliedSerial, s.appliedFrame, mist, seed);
    return GameValue(buf);
}

} // namespace

// Called from GameStateExtTestAudio.cpp's INIT_MODULE to force this TU into
// the link (no other code references these verbs).
void EnsureGameStateExtTestDeterminismLinked() {}

INIT_MODULE(GameStateExtTestDeterminism, 3)
{
    GGameState.NewFunction(GameFunction(GameString, "triDeterministic", TriDeterministic, GameBool));
    GGameState.NewFunction(GameFunction(GameString, "triResetTemporal", TriResetTemporal, GameArray | GameScalar));
    GGameState.NewFunction(GameFunction(GameString, "triSeedRandom", TriSeedRandom, GameScalar));
    GGameState.NewNularOp(GameNular(GameString, "triTemporalState", TriTemporalState));
};
