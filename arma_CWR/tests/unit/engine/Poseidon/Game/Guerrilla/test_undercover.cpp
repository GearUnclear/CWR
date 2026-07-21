#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Game/Guerrilla/Undercover.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <string.h>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
using Catch::Approx;

namespace
{

// every undercover* key overridden away from its default
const char* kUndercoverConfig = "class CfgGuerrillaZones\n"
                                "{\n"
                                "    undercoverNoticeRadius = 40;\n"
                                "    undercoverBackArcCos = -0.5;\n"
                                "    undercoverInHandsBoost = 3.0;\n"
                                "    undercoverSlungBoost = 1.1;\n"
                                "    undercoverIdentifyAccuracy = 2.0;\n"
                                "    undercoverMinVisibility = 0.1;\n"
                                "    undercoverWarDetectScale = 1.0;\n"
                                "    undercoverForgetSeconds = 120;\n"
                                "    undercoverBoardWitnessSeconds = 25;\n"
                                "    class Zones { class A { name=\"A\"; }; };\n"
                                "};\n";

// keeps the parsed ParamFile alive for the duration of a test; a LOCAL
// system instance so the process-wide singleton stays untouched
struct UndercoverFixture
{
    ParamFile file;
    UndercoverSystem system;

    void Load(const char* config)
    {
        QIStream in(config, strlen(config));
        file.Parse(in);
        system.LoadFromParams(file.FindEntry("CfgGuerrillaZones"));
    }
};

float FloorAt(float value, float floor)
{
    return value < floor ? floor : value;
}

UCObservation MakeObs(UCWeaponShow weapon, float dist2, float cosFacing, float visibility, float sideAccuracy,
                      bool compromised = false, float warDetect = 0.0f)
{
    UCObservation obs;
    obs.weapon = weapon;
    obs.dist2 = dist2;
    obs.cosFacing = cosFacing;
    obs.visibility = visibility;
    obs.sideAccuracy = sideAccuracy;
    obs.alreadyCompromised = compromised;
    obs.warDetect = warDetect;
    return obs;
}

UCVehicleObservation MakeVehObs(UCVehicleClass cls, bool vehicleCompromised, bool personCompromised, bool personRecent,
                                float dist2, float visibility, float sideAccuracy)
{
    UCVehicleObservation obs;
    obs.vehicleClass = cls;
    obs.vehicleRecordCompromised = vehicleCompromised;
    obs.personRecordCompromised = personCompromised;
    obs.personLastSeenRecent = personRecent;
    obs.dist2 = dist2;
    obs.visibility = visibility;
    obs.sideAccuracy = sideAccuracy;
    return obs;
}

} // namespace

TEST_CASE("Undercover - tuning defaults match the design constants", "[game][guerrilla]")
{
    UndercoverSystem sys;
    sys.LoadFromParams(nullptr);

    const UndercoverTuning& t = sys.Tuning();
    REQUIRE(t.undercoverNoticeRadius == Approx(20.0f));
    REQUIRE(t.undercoverBackArcCos == Approx(-0.2f));
    REQUIRE(t.undercoverInHandsBoost == Approx(2.0f));
    REQUIRE(t.undercoverSlungBoost == Approx(1.4f));
    REQUIRE(t.undercoverIdentifyAccuracy == Approx(1.5f)); // the vanilla ID threshold
    REQUIRE(t.undercoverMinVisibility == Approx(0.02f));
    REQUIRE(t.undercoverWarDetectScale == Approx(0.5f));
    REQUIRE(t.undercoverForgetSeconds == Approx(0.0f)); // permanent per group
    REQUIRE(t.undercoverBoardWitnessSeconds == Approx(10.0f));
}

TEST_CASE("Undercover - config keys override the defaults", "[game][guerrilla]")
{
    UndercoverFixture f;
    f.Load(kUndercoverConfig);

    const UndercoverTuning& t = f.system.Tuning();
    REQUIRE(t.undercoverNoticeRadius == Approx(40.0f));
    REQUIRE(t.undercoverBackArcCos == Approx(-0.5f));
    REQUIRE(t.undercoverInHandsBoost == Approx(3.0f));
    REQUIRE(t.undercoverSlungBoost == Approx(1.1f));
    REQUIRE(t.undercoverIdentifyAccuracy == Approx(2.0f));
    REQUIRE(t.undercoverMinVisibility == Approx(0.1f));
    REQUIRE(t.undercoverWarDetectScale == Approx(1.0f));
    REQUIRE(t.undercoverForgetSeconds == Approx(120.0f));
    REQUIRE(t.undercoverBoardWitnessSeconds == Approx(25.0f));

    // a fresh LoadFromParams(null) resets back to the defaults
    f.system.LoadFromParams(nullptr);
    REQUIRE(f.system.Tuning().undercoverNoticeRadius == Approx(20.0f));
    REQUIRE(f.system.Tuning().undercoverIdentifyAccuracy == Approx(1.5f));
}

TEST_CASE("Undercover - overridden tuning drives the rules", "[game][guerrilla]")
{
    UndercoverFixture f;
    f.Load(kUndercoverConfig);
    const UndercoverTuning& t = f.system.Tuning();
    float out = -1.0f;

    // notice radius widened to 40 m: a slung rifle at 30 m front-on is
    // inside conversational range now, floored at the raised ID accuracy
    REQUIRE(EvaluateUndercoverRule(MakeObs(UCWSlung, 900.0f, 0.8f, 0.5f, 0.4f), t, out) == UCExposed);
    REQUIRE(out == Approx(2.0f));

    // identify accuracy raised to 2.0: in-hands 0.5 x 3.0 = 1.5 no longer IDs
    REQUIRE(EvaluateUndercoverRule(MakeObs(UCWInHands, 3600.0f, 0.8f, 0.5f, 0.5f), t, out) == UCSuspect);
    REQUIRE(out == Approx(1.5f));

    // min visibility raised to 0.1: a 0.05 glimpse is gated out entirely
    REQUIRE(EvaluateUndercoverRule(MakeObs(UCWSlung, 100.0f, 0.8f, 0.05f, 1.2f), t, out) == UCCivil);
    REQUIRE(out == Approx(1.2f));
}

TEST_CASE("Undercover - person rule verdict matrix", "[game][guerrilla]")
{
    UndercoverTuning tuning; // defaults

    const UCWeaponShow weapons[] = {UCWNone, UCWSlung, UCWInHands};
    const float facings[] = {0.8f, -0.8f};     // front / back
    const float dists[] = {100.0f, 3600.0f};   // 10 m inside / 60 m outside the notice radius
    const float visibilities[] = {0.0f, 0.5f}; // below the gate / normal
    const float accuracies[] = {0.4f, 1.2f};   // low / high sensor side accuracy
    const bool compromiseStates[] = {false, true};

    for (UCWeaponShow weapon : weapons)
    {
        for (float cosFacing : facings)
        {
            for (float dist2 : dists)
            {
                for (float visibility : visibilities)
                {
                    for (float accuracy : accuracies)
                    {
                        for (bool compromised : compromiseStates)
                        {
                            // spec oracle (plan pseudocode with the default constants)
                            float expectedOut = accuracy;
                            UCVerdict expected;
                            if (compromised)
                            {
                                expectedOut = FloorAt(accuracy, 1.5f);
                                expected = UCExposed;
                            }
                            else if (visibility < 0.02f)
                            {
                                expected = UCCivil;
                            }
                            else if (weapon == UCWInHands)
                            {
                                expectedOut = accuracy * 2.0f;
                                expected = expectedOut >= 1.5f ? UCExposed : UCSuspect;
                            }
                            else if (weapon == UCWSlung)
                            {
                                if (dist2 < 400.0f)
                                {
                                    expectedOut = FloorAt(accuracy, 1.5f);
                                    expected = UCExposed;
                                }
                                else if (cosFacing < -0.2f)
                                {
                                    expectedOut = accuracy * 1.4f;
                                    expected = expectedOut >= 1.5f ? UCExposed : UCSuspect;
                                }
                                else
                                {
                                    expected = UCCivil;
                                }
                            }
                            else
                            {
                                expected = UCCivil;
                            }

                            float out = -1.0f;
                            UCVerdict verdict = EvaluateUndercoverRule(
                                MakeObs(weapon, dist2, cosFacing, visibility, accuracy, compromised), tuning, out);
                            CAPTURE((int)weapon, cosFacing, dist2, visibility, accuracy, compromised);
                            CHECK(verdict == expected);
                            CHECK(out == Approx(expectedOut));
                        }
                    }
                }
            }
        }
    }
}

TEST_CASE("Undercover - person rule branch semantics", "[game][guerrilla]")
{
    UndercoverTuning tuning;
    float out = -1.0f;

    SECTION("a compromised record wins before the visibility gate")
    {
        // unseen, unarmed, front-on, far: nothing observable, yet the group
        // already knows the face
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWNone, 250000.0f, 0.8f, 0.0f, 0.4f, true), tuning, out) == UCExposed);
        // ... and the record is pinned at the ID floor so it cannot fade back
        REQUIRE(out == Approx(1.5f));

        // the floor never lowers an already-high accuracy
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWNone, 250000.0f, 0.8f, 0.0f, 2.0f, true), tuning, out) == UCExposed);
        REQUIRE(out == Approx(2.0f));
    }

    SECTION("weapon in hands ignores facing and distance")
    {
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWInHands, 250000.0f, 0.8f, 0.5f, 1.2f), tuning, out) == UCExposed);
        REQUIRE(out == Approx(2.4f)); // 1.2 x 2.0

        // low accuracy at range: suspicious silhouette, no positive ID yet
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWInHands, 250000.0f, 0.8f, 0.5f, 0.4f), tuning, out) == UCSuspect);
        REQUIRE(out == Approx(0.8f));
    }

    SECTION("a slung rifle is unmissable at conversational range")
    {
        // front-on does not help inside the notice radius; accuracy floored
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWSlung, 100.0f, 0.8f, 0.5f, 0.3f), tuning, out) == UCExposed);
        REQUIRE(out == Approx(1.5f));
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWSlung, 100.0f, 0.8f, 0.5f, 2.0f), tuning, out) == UCExposed);
        REQUIRE(out == Approx(2.0f)); // floor never lowers
    }

    SECTION("a slung rifle hides behind the torso front-on at range")
    {
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWSlung, 3600.0f, 0.8f, 0.5f, 1.2f), tuning, out) == UCCivil);
        REQUIRE(out == Approx(1.2f)); // passthrough, no boost
    }

    SECTION("a slung rifle seen from behind boosts identification")
    {
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWSlung, 3600.0f, -0.8f, 0.5f, 1.2f), tuning, out) == UCExposed);
        REQUIRE(out == Approx(1.68f)); // 1.2 x 1.4
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWSlung, 3600.0f, -0.8f, 0.5f, 0.4f), tuning, out) == UCSuspect);
        REQUIRE(out == Approx(0.56f));
    }

    SECTION("unarmed and clean reads civilian with accuracy passthrough")
    {
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWNone, 100.0f, -0.8f, 1.0f, 1.2f), tuning, out) == UCCivil);
        REQUIRE(out == Approx(1.2f));
    }

    SECTION("below the visibility gate nothing is evaluated")
    {
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWInHands, 100.0f, -0.8f, 0.01f, 4.0f), tuning, out) == UCCivil);
        REQUIRE(out == Approx(4.0f)); // passthrough
    }
}

TEST_CASE("Undercover - war level scales identification", "[game][guerrilla]")
{
    UndercoverTuning tuning;
    float out = -1.0f;

    SECTION("in hands: war detect flips Suspect to Exposed")
    {
        // 0.7 x 2.0 = 1.4 < 1.5 at peace
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWInHands, 3600.0f, 0.8f, 0.5f, 0.7f, false, 0.0f), tuning, out) ==
                UCSuspect);
        REQUIRE(out == Approx(1.4f));
        // 0.7 x 2.0 x (1 + 0.5 x 1.0) = 2.1 >= 1.5 at full war
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWInHands, 3600.0f, 0.8f, 0.5f, 0.7f, false, 1.0f), tuning, out) ==
                UCExposed);
        REQUIRE(out == Approx(2.1f));
    }

    SECTION("slung from behind: war detect flips Suspect to Exposed")
    {
        // 1.05 x 1.4 = 1.47 < 1.5 at peace
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWSlung, 3600.0f, -0.8f, 0.5f, 1.05f, false, 0.0f), tuning, out) ==
                UCSuspect);
        REQUIRE(out == Approx(1.47f));
        // 1.05 x 1.4 x 1.5 = 2.205 at full war
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWSlung, 3600.0f, -0.8f, 0.5f, 1.05f, false, 1.0f), tuning, out) ==
                UCExposed);
        REQUIRE(out == Approx(2.205f));
    }
}

TEST_CASE("Undercover - person rule boundaries", "[game][guerrilla]")
{
    UndercoverTuning tuning;
    float out = -1.0f;

    SECTION("exactly the notice radius is NOT inside (strict less-than)")
    {
        // dist2 == 20^2, front-on slung: falls through to the front-arc civil
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWSlung, 400.0f, 0.8f, 0.5f, 0.4f), tuning, out) == UCCivil);
        REQUIRE(out == Approx(0.4f));
        // a hair inside is inside
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWSlung, 399.9f, 0.8f, 0.5f, 0.4f), tuning, out) == UCExposed);
    }

    SECTION("exactly the back-arc cosine is NOT behind (strict less-than)")
    {
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWSlung, 3600.0f, -0.2f, 0.5f, 1.2f), tuning, out) == UCCivil);
        REQUIRE(out == Approx(1.2f));
        // a hair beyond is behind
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWSlung, 3600.0f, -0.21f, 0.5f, 1.2f), tuning, out) == UCExposed);
    }

    SECTION("boosted accuracy exactly at the ID threshold exposes (inclusive)")
    {
        // 0.75 x 2.0 = 1.5 exactly
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWInHands, 3600.0f, 0.8f, 0.5f, 0.75f), tuning, out) == UCExposed);
        REQUIRE(out == Approx(1.5f));
    }

    SECTION("visibility exactly at the gate is evaluated (inclusive)")
    {
        REQUIRE(EvaluateUndercoverRule(MakeObs(UCWSlung, 100.0f, 0.8f, 0.02f, 0.4f), tuning, out) == UCExposed);
    }
}

TEST_CASE("Undercover - vehicle rule verdict matrix", "[game][guerrilla]")
{
    UndercoverTuning tuning; // defaults

    struct RecordState
    {
        bool vehicleCompromised;
        bool personCompromised;
        bool personRecent;
    };
    const UCVehicleClass classes[] = {UCVCivilian, UCVOccupierMilitary, UCVOther};
    const RecordState records[] = {
        {false, false, false}, // clean
        {false, true, true},   // person compromised, seen recently (getaway)
        {false, true, false},  // person compromised, stale
        {true, false, false},  // vehicle itself remembered
    };
    const float dists[] = {100.0f, 3600.0f};   // 10 m inside / 60 m outside the notice radius
    const float visibilities[] = {0.0f, 0.5f}; // unseen / seen
    const float accuracies[] = {1.2f, 1.4f};   // below / above the stolen-vehicle idiom's 1.35

    for (UCVehicleClass cls : classes)
    {
        for (const RecordState& rec : records)
        {
            for (float dist2 : dists)
            {
                for (float visibility : visibilities)
                {
                    for (float accuracy : accuracies)
                    {
                        // spec oracle ("Vehicle policy" priority list, default constants)
                        bool seen = visibility >= 0.02f;
                        bool close = dist2 < 400.0f;
                        float expectedOut = accuracy;
                        UCVerdict expected;
                        if (rec.vehicleCompromised)
                        {
                            expectedOut = FloorAt(accuracy, 1.5f);
                            expected = UCExposed;
                        }
                        else if (rec.personCompromised && (rec.personRecent || (seen && close)))
                        {
                            expectedOut = FloorAt(accuracy, 1.5f);
                            expected = UCExposed;
                        }
                        else if (cls == UCVCivilian)
                        {
                            expected = UCCivil;
                        }
                        else if (seen && close)
                        {
                            expectedOut = FloorAt(accuracy, 1.5f);
                            expected = UCExposed;
                        }
                        else if (accuracy >= 1.35f)
                        {
                            expected = UCSuspect;
                        }
                        else
                        {
                            expected = UCCivil;
                        }

                        float out = -1.0f;
                        UCVerdict verdict = EvaluateUndercoverVehicleRule(
                            MakeVehObs(cls, rec.vehicleCompromised, rec.personCompromised, rec.personRecent, dist2,
                                       visibility, accuracy),
                            tuning, out);
                        CAPTURE((int)cls, rec.vehicleCompromised, rec.personCompromised, rec.personRecent, dist2,
                                visibility, accuracy);
                        CHECK(verdict == expected);
                        CHECK(out == Approx(expectedOut));
                    }
                }
            }
        }
    }
}

TEST_CASE("Undercover - vehicle rule priority order", "[game][guerrilla]")
{
    UndercoverTuning tuning;
    float out = -1.0f;

    SECTION("a remembered vehicle wins even as an unseen civilian car far away")
    {
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVCivilian, true, false, false, 250000.0f, 0.0f, 0.4f),
                                              tuning, out) == UCExposed);
        REQUIRE(out == Approx(1.5f)); // floored at the ID accuracy
    }

    SECTION("a witnessed getaway engages the car regardless of range")
    {
        // person record compromised + seen recently: the group watched him board
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVCivilian, false, true, true, 250000.0f, 0.0f, 0.4f), tuning,
                                              out) == UCExposed);
        REQUIRE(out == Approx(1.5f));
    }

    SECTION("a stale person record needs a close look through the windshield")
    {
        // stale + far: NOT exposed - the civilian car stays anonymous
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVCivilian, false, true, false, 3600.0f, 0.5f, 1.2f), tuning,
                                              out) == UCCivil);
        REQUIRE(out == Approx(1.2f)); // passthrough
        // stale + close but unseen: still anonymous
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVCivilian, false, true, false, 100.0f, 0.0f, 1.2f), tuning,
                                              out) == UCCivil);
        // stale + close + seen: recognized at the wheel
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVCivilian, false, true, false, 100.0f, 0.5f, 1.2f), tuning,
                                              out) == UCExposed);
        REQUIRE(out == Approx(1.5f));
    }

    SECTION("the recent flag alone means nothing without a compromised person record")
    {
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVCivilian, false, false, true, 100.0f, 0.5f, 1.4f), tuning,
                                              out) == UCCivil);
    }

    SECTION("a clean civilian car is anonymous even at checkpoint range")
    {
        // civilian policy outranks the close-look rule
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVCivilian, false, false, false, 100.0f, 0.5f, 1.4f), tuning,
                                              out) == UCCivil);
        REQUIRE(out == Approx(1.4f)); // passthrough, no suspicion accumulates
    }

    SECTION("a stolen military vehicle fails at checkpoint range")
    {
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVOccupierMilitary, false, false, false, 100.0f, 0.5f, 0.4f),
                                              tuning, out) == UCExposed);
        REQUIRE(out == Approx(1.5f)); // floored at the ID accuracy
    }

    SECTION("at range a clean stolen military vehicle reads the typical side")
    {
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVOccupierMilitary, false, false, false, 3600.0f, 0.5f, 1.2f),
                                              tuning, out) == UCCivil);
        REQUIRE(out == Approx(1.2f)); // low accuracy: the disguise holds
    }

    SECTION("a high-accuracy look flips the stolen vehicle to Suspect")
    {
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVOccupierMilitary, false, false, false, 3600.0f, 0.5f, 1.4f),
                                              tuning, out) == UCSuspect);
        REQUIRE(out == Approx(1.4f)); // the vanilla stolen-vehicle idiom, passthrough
    }

    SECTION("other military types behave like the occupier's own")
    {
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVOther, false, false, false, 100.0f, 0.5f, 0.4f), tuning,
                                              out) == UCExposed);
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVOther, false, false, false, 3600.0f, 0.5f, 1.4f), tuning,
                                              out) == UCSuspect);
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVOther, false, false, false, 3600.0f, 0.5f, 1.2f), tuning,
                                              out) == UCCivil);
    }
}

TEST_CASE("Undercover - vehicle rule boundaries", "[game][guerrilla]")
{
    UndercoverTuning tuning;
    float out = -1.0f;

    SECTION("side accuracy exactly at the stolen-vehicle threshold is Suspect (inclusive)")
    {
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVOccupierMilitary, false, false, false, 3600.0f, 0.5f,
                                                         1.35f),
                                              tuning, out) == UCSuspect);
    }

    SECTION("exactly the notice radius is NOT close (strict less-than)")
    {
        // seen at exactly 20 m: no close-look exposure, low accuracy holds the disguise
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVOccupierMilitary, false, false, false, 400.0f, 0.5f, 0.4f),
                                              tuning, out) == UCCivil);
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVOccupierMilitary, false, false, false, 399.9f, 0.5f, 0.4f),
                                              tuning, out) == UCExposed);
    }

    SECTION("visibility exactly at the gate counts as seen (inclusive)")
    {
        REQUIRE(EvaluateUndercoverVehicleRule(MakeVehObs(UCVOccupierMilitary, false, false, false, 100.0f, 0.02f,
                                                         0.4f),
                                              tuning, out) == UCExposed);
    }
}
