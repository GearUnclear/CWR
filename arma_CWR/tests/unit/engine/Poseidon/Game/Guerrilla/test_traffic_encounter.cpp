// Issue #66: threatened civilian drivers choose surrender or escape (one
// roll per encounter), an unarmed player in the lane is a roadblock, fresh
// gunfire outranks both.  The pure encounter core of the Traffic service.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/AI/Path/ArcadeWaypoint.hpp> // SpeedMode
#include <Poseidon/Core/SaveVersion.hpp>       // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/Traffic.hpp>
#include <Poseidon/Game/Guerrilla/TrafficEncounter.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <filesystem>
#include <math.h>
#include <string>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
using Catch::Approx;

namespace
{

CommandeerObs Obs(float carX, float carZ, float carDirX, float carDirZ, float px, float pz, float pdx, float pdz,
                  bool armed)
{
    CommandeerObs o;
    o.carPos = Vector3(carX, 0, carZ);
    o.carDir = Vector3(carDirX, 0, carDirZ);
    o.playerPos = Vector3(px, 0, pz);
    o.playerDir = Vector3(pdx, 0, pdz);
    o.weaponInHands = armed;
    return o;
}

// one encounter check: the player inside the clear radius unless outside
TrafficEncounterInput In(TrafficState state, bool threat, bool lane, bool outside = false, float roll = 0.0f,
                         float dt = 0.5f, bool danger = false)
{
    TrafficEncounterInput in;
    in.state = state;
    in.armedThreat = threat;
    in.laneObstruction = lane;
    in.playerOutside = outside;
    in.roll = roll;
    in.dt = dt;
    in.freshDanger = danger;
    return in;
}

TrafficEncounter Enc(TrafficEncounterChoice choice, bool dangerOwned = false)
{
    TrafficEncounter e;
    e.choice = choice;
    e.dangerOwned = dangerOwned;
    return e;
}

bool IsDefault(const TrafficEncounter& e)
{
    return e.choice == EncNone && !e.dangerOwned && !e.laneWait && e.surrenderTime == 0 && e.clearHold < 0 &&
           e.laneClear < 0;
}

float Rad(float deg)
{
    return deg * 3.14159265f / 180.0f;
}

} // namespace

TEST_CASE("Traffic - encounter lane obstruction matrix", "[game][guerrilla]")
{
    TrafficTuning t; // radius 25, lane half-width 4; car at origin heading +Z
    SECTION("ahead in the lane, unarmed -> obstruction")
    {
        REQUIRE(Traffic::InLaneObstruction(Obs(0, 0, 0, 1, 1, 15, 0, -1, false), t));
    }
    SECTION("ahead but outside the lane, behind, beside -> no obstruction")
    {
        REQUIRE_FALSE(Traffic::InLaneObstruction(Obs(0, 0, 0, 1, 8, 15, 0, -1, false), t));
        REQUIRE_FALSE(Traffic::InLaneObstruction(Obs(0, 0, 0, 1, 0, -15, 0, 1, false), t));
        REQUIRE_FALSE(Traffic::InLaneObstruction(Obs(0, 0, 0, 1, 10, 0, -1, 0, false), t));
    }
    SECTION("an armed player beside the car aiming at it is no obstruction (the weapon is ignored)")
    {
        REQUIRE_FALSE(Traffic::InLaneObstruction(Obs(0, 0, 0, 1, 10, 0, -1, 0, true), t));
    }
    SECTION("out of radius, even in the lane -> no obstruction")
    {
        REQUIRE_FALSE(Traffic::InLaneObstruction(Obs(0, 0, 0, 1, 0, 40, 0, -1, false), t));
    }
    SECTION("car heading is normalized, not assumed unit")
    {
        REQUIRE(Traffic::InLaneObstruction(Obs(0, 0, 0, 3, 1, 15, 0, -1, false), t));
    }
    SECTION("armed in the lane is an obstruction whichever way he faces")
    {
        REQUIRE(Traffic::InLaneObstruction(Obs(0, 0, 0, 1, 1, 15, 0, 1, true), t));
        REQUIRE(Traffic::InLaneObstruction(Obs(0, 0, 0, 1, 1, 15, 0, -1, true), t));
    }
    SECTION("on the bonnet -> obstruction; a zero-length car heading (not the bonnet) -> none")
    {
        REQUIRE(Traffic::InLaneObstruction(Obs(0, 0, 0, 1, 0, 0, 0, -1, false), t));
        REQUIRE_FALSE(Traffic::InLaneObstruction(Obs(0, 0, 0, 0, 1, 15, 0, -1, false), t));
    }
    SECTION("boundaries: the radius and the half-width are inclusive, the lane cone is 20 deg")
    {
        REQUIRE(Traffic::InLaneObstruction(Obs(0, 0, 0, 1, 0, 25, 0, -1, false), t));
        REQUIRE(Traffic::InLaneObstruction(Obs(0, 0, 0, 1, 4, 20, 0, -1, false), t));
        REQUIRE(Traffic::InLaneObstruction(Obs(0, 0, 0, 1, 10 * sinf(Rad(19)), 10 * cosf(Rad(19)), 0, -1, false), t));
        REQUIRE_FALSE(
            Traffic::InLaneObstruction(Obs(0, 0, 0, 1, 10 * sinf(Rad(21)), 10 * cosf(Rad(21)), 0, -1, false), t));
    }
}

TEST_CASE("Traffic - encounter armed threat matrix", "[game][guerrilla]")
{
    TrafficTuning t;
    SECTION("armed beside the car aiming at it -> threat; aiming elsewhere -> none")
    {
        REQUIRE(Traffic::ArmedThreat(Obs(0, 0, 0, 1, 10, 0, -1, 0, true), t));
        REQUIRE_FALSE(Traffic::ArmedThreat(Obs(0, 0, 0, 1, 10, 0, 0, 1, true), t));
    }
    SECTION("unarmed is never a threat, looking or in the lane")
    {
        REQUIRE_FALSE(Traffic::ArmedThreat(Obs(0, 0, 0, 1, 10, 0, -1, 0, false), t));
        REQUIRE_FALSE(Traffic::ArmedThreat(Obs(0, 0, 0, 1, 1, 15, 0, -1, false), t));
    }
    SECTION("armed in the lane: facing away is no threat, facing the car is")
    {
        REQUIRE_FALSE(Traffic::ArmedThreat(Obs(0, 0, 0, 1, 1, 15, 0, 1, true), t));
        REQUIRE(Traffic::ArmedThreat(Obs(0, 0, 0, 1, 0, 15, 0, -1, true), t));
    }
    SECTION("out of radius, aiming -> none")
    {
        REQUIRE_FALSE(Traffic::ArmedThreat(Obs(0, 0, 0, 1, 0, 40, 0, -1, true), t));
    }
    SECTION("aim cone edge: 14 deg in, 16 deg out")
    {
        REQUIRE(Traffic::ArmedThreat(Obs(0, 0, 0, 1, 10, 0, -cosf(Rad(14)), sinf(Rad(14)), true), t));
        REQUIRE_FALSE(Traffic::ArmedThreat(Obs(0, 0, 0, 1, 10, 0, -cosf(Rad(16)), sinf(Rad(16)), true), t));
    }
    SECTION("the bonnet: armed is a threat, unarmed is not; a zero-length facing is none")
    {
        REQUIRE(Traffic::ArmedThreat(Obs(0, 0, 0, 1, 0, 0, 0, 0, true), t));
        REQUIRE_FALSE(Traffic::ArmedThreat(Obs(0, 0, 0, 1, 0, 0, 0, 0, false), t));
        REQUIRE_FALSE(Traffic::ArmedThreat(Obs(0, 0, 0, 1, 10, 0, 0, 0, true), t));
    }
}

TEST_CASE("Traffic - encounter surrender choice", "[game][guerrilla]")
{
    REQUIRE(TrafficTuning().commandeerSurrenderChance == Approx(0.5f));
    SECTION("chance 0 always escapes, chance 1 always surrenders")
    {
        REQUIRE_FALSE(Traffic::ChooseSurrender(0.0f, 0.0f));
        REQUIRE_FALSE(Traffic::ChooseSurrender(0.5f, 0.0f));
        REQUIRE_FALSE(Traffic::ChooseSurrender(0.999f, 0.0f));
        REQUIRE(Traffic::ChooseSurrender(0.0f, 1.0f));
        REQUIRE(Traffic::ChooseSurrender(0.999f, 1.0f));
    }
    SECTION("chance 0.5: below surrenders, equality and above escape")
    {
        REQUIRE(Traffic::ChooseSurrender(0.4999f, 0.5f));
        REQUIRE_FALSE(Traffic::ChooseSurrender(0.5f, 0.5f));
        REQUIRE_FALSE(Traffic::ChooseSurrender(0.5001f, 0.5f));
    }
    SECTION("out-of-range chances saturate")
    {
        REQUIRE_FALSE(Traffic::ChooseSurrender(0.0f, -0.2f));
        REQUIRE(Traffic::ChooseSurrender(0.999f, 1.3f));
    }
}

TEST_CASE("Traffic - encounter: one roll per encounter", "[game][guerrilla]")
{
    TrafficTuning t; // stop delay 2.5
    SECTION("surrender: the roll is spent, the delay bails exactly at 2.5 s")
    {
        TrafficEncounterStep s = Traffic::StepEncounter(TrafficEncounter(), In(TSDriving, true, true, false, 0.1f), t);
        REQUIRE(s.action == EActBeginSurrender);
        REQUIRE(s.rolled);
        REQUIRE(s.next.choice == EncSurrender);
        TrafficEncounter enc = s.next;
        const float rolls[4] = {0.9f, 0.0f, 0.7f, 0.3f};
        for (int k = 0; k < 4; k++)
        {
            s = Traffic::StepEncounter(enc, In(TSStopping, true, true, false, rolls[k]), t);
            REQUIRE(s.action == EActNone);
            REQUIRE_FALSE(s.rolled);
            REQUIRE(s.next.choice == EncSurrender);
            enc = s.next;
        }
        REQUIRE(enc.surrenderTime == Approx(2.0f));
        s = Traffic::StepEncounter(enc, In(TSStopping, true, true, false, 0.9f), t);
        REQUIRE(s.action == EActSurrenderBail);
        REQUIRE_FALSE(s.rolled);
        REQUIRE(s.next.surrenderTime == Approx(2.5f));
    }
    SECTION("escape: no further roll and no lane wait while the choice is latched")
    {
        TrafficEncounterStep s = Traffic::StepEncounter(TrafficEncounter(), In(TSDriving, true, false, false, 0.9f), t);
        REQUIRE(s.action == EActBeginEscape);
        REQUIRE(s.rolled);
        REQUIRE(s.next.choice == EncEscape);
        TrafficEncounter enc = s.next;
        for (int k = 0; k < 20; k++)
        {
            bool threat = (k % 2) == 0;
            bool lane = (k % 3) == 0;
            s = Traffic::StepEncounter(enc, In(TSDriving, threat, lane, false, (float)k / 20.0f), t);
            REQUIRE(s.action == EActNone);
            REQUIRE_FALSE(s.rolled);
            REQUIRE(s.next.choice == EncEscape);
            REQUIRE_FALSE(s.next.laneWait);
            enc = s.next;
        }
    }
}

TEST_CASE("Traffic - encounter: the pending surrender timer", "[game][guerrilla]")
{
    TrafficTuning t;
    TrafficEncounter enc = Enc(EncSurrender);
    TrafficEncounterStep s;
    SECTION("accrues inside, freezes outside, bails only inside")
    {
        for (int k = 0; k < 2; k++)
        {
            s = Traffic::StepEncounter(enc, In(TSStopping, false, false, false), t);
            enc = s.next;
        }
        REQUIRE(enc.surrenderTime == Approx(1.0f));
        for (int k = 0; k < 3; k++)
        {
            s = Traffic::StepEncounter(enc, In(TSStopping, false, false, true), t);
            REQUIRE(s.action == EActNone);
            enc = s.next;
        }
        REQUIRE(enc.surrenderTime == Approx(1.0f));
        REQUIRE(enc.clearHold == Approx(1.0f));
        s = Traffic::StepEncounter(enc, In(TSStopping, false, false, false), t);
        REQUIRE(s.action == EActNone);
        REQUIRE(s.next.clearHold < 0); // back inside: the hold restarts
        enc = s.next;
        s = Traffic::StepEncounter(enc, In(TSStopping, false, false, false), t);
        REQUIRE(s.action == EActNone);
        enc = s.next;
        s = Traffic::StepEncounter(enc, In(TSStopping, false, false, false), t);
        REQUIRE(s.action == EActSurrenderBail);
    }
    SECTION("the player walking away calls the stop off")
    {
        int clearedAt = -1;
        for (int k = 0; k < 20 && clearedAt < 0; k++)
        {
            s = Traffic::StepEncounter(enc, In(TSStopping, false, false, true), t);
            if (s.action == EActClear)
            {
                clearedAt = k;
            }
            else
            {
                REQUIRE(s.action == EActNone);
            }
            enc = s.next;
        }
        REQUIRE(clearedAt == 10); // 0 + 10 x 0.5 s = the 5 s hold
        REQUIRE(IsDefault(enc));
    }
}

TEST_CASE("Traffic - encounter: the clear hold", "[game][guerrilla]")
{
    TrafficTuning t;
    SECTION("the first outside observation credits nothing, whatever dt")
    {
        TrafficEncounterStep s = Traffic::StepEncounter(Enc(EncEscape), In(TSDriving, false, false, true), t);
        REQUIRE(s.next.clearHold == Approx(0.0f));
        s = Traffic::StepEncounter(Enc(EncEscape), In(TSDriving, false, false, true, 0, 5.0f), t);
        REQUIRE(s.action == EActNone);
        REQUIRE(s.next.clearHold == Approx(0.0f));
    }
    SECTION("accrues to exactly 5 s, then clears to the default encounter")
    {
        TrafficEncounter enc = Enc(EncEscape);
        TrafficEncounterStep s;
        for (int k = 0; k < 10; k++)
        {
            s = Traffic::StepEncounter(enc, In(TSDriving, false, false, true), t);
            REQUIRE(s.action == EActNone);
            enc = s.next;
        }
        REQUIRE(enc.clearHold == Approx(4.5f));
        s = Traffic::StepEncounter(enc, In(TSDriving, false, false, true), t);
        REQUIRE(s.action == EActClear);
        REQUIRE(IsDefault(s.next));
    }
    SECTION("coming back inside resets the hold; the full hold is needed again")
    {
        TrafficEncounter enc = Enc(EncEscape);
        enc.clearHold = 3.0f;
        TrafficEncounterStep s = Traffic::StepEncounter(enc, In(TSDriving, false, false, false), t);
        REQUIRE(s.next.clearHold < 0);
        enc = s.next;
        for (int k = 0; k < 10; k++)
        {
            s = Traffic::StepEncounter(enc, In(TSDriving, false, false, true), t);
            REQUIRE(s.action == EActNone);
            enc = s.next;
        }
        REQUIRE(Traffic::StepEncounter(enc, In(TSDriving, false, false, true), t).action == EActClear);
    }
    SECTION("a danger-owned encounter clears the same way, and so does a missing player")
    {
        TrafficEncounter enc = Enc(EncNone, true);
        enc.clearHold = 4.5f;
        TrafficEncounterStep s = Traffic::StepEncounter(enc, In(TSDriving, false, false, true), t);
        REQUIRE(s.action == EActClear);
        REQUIRE(IsDefault(s.next));
        // no player: outside, no threat, no lane
        enc = Enc(EncSurrender, true);
        int clearedAt = -1;
        for (int k = 0; k < 20 && clearedAt < 0; k++)
        {
            s = Traffic::StepEncounter(enc, In(TSDriving, false, false, true), t);
            clearedAt = s.action == EActClear ? k : -1;
            enc = s.next;
        }
        REQUIRE(clearedAt == 10);
    }
    SECTION("an inactive encounter keeps the hold off")
    {
        TrafficEncounterStep s = Traffic::StepEncounter(TrafficEncounter(), In(TSDriving, false, false, true), t);
        REQUIRE(s.action == EActNone);
        REQUIRE(s.next.clearHold < 0);
    }
}

TEST_CASE("Traffic - encounter: the roadblock wait", "[game][guerrilla]")
{
    TrafficTuning t;
    SECTION("brake once, hold while blocked, resume once after 1 s of clear lane")
    {
        TrafficEncounterStep s = Traffic::StepEncounter(TrafficEncounter(), In(TSDriving, false, true), t);
        REQUIRE(s.action == EActBeginLaneWait);
        REQUIRE(s.next.laneWait);
        REQUIRE_FALSE(s.rolled);
        TrafficEncounter enc = s.next;
        s = Traffic::StepEncounter(enc, In(TSDriving, false, true), t);
        REQUIRE(s.action == EActNone);
        enc = s.next;
        s = Traffic::StepEncounter(enc, In(TSDriving, false, false), t);
        REQUIRE(s.action == EActNone);
        REQUIRE(s.next.laneClear == Approx(0.0f));
        enc = s.next;
        s = Traffic::StepEncounter(enc, In(TSDriving, false, false), t);
        REQUIRE(s.action == EActNone);
        REQUIRE(s.next.laneClear == Approx(0.5f));
        enc = s.next;
        s = Traffic::StepEncounter(enc, In(TSDriving, false, false), t);
        REQUIRE(s.action == EActResumeLaneWait);
        REQUIRE_FALSE(s.next.laneWait);
        enc = s.next;
        s = Traffic::StepEncounter(enc, In(TSDriving, false, false), t);
        REQUIRE(s.action == EActNone);
        // the wait is not an encounter: it never clears via the hold
        REQUIRE_FALSE(enc.Active());
    }
    SECTION("a flickering lane never resumes before 1 s of continuous clear")
    {
        TrafficEncounter enc;
        enc.laneWait = true;
        for (int k = 0; k < 12; k++)
        {
            bool lane = (k % 3) == 2; // clear, clear, blocked, ...
            TrafficEncounterStep s = Traffic::StepEncounter(enc, In(TSDriving, false, lane), t);
            REQUIRE(s.action == EActNone);
            REQUIRE(s.next.laneWait);
            enc = s.next;
        }
    }
    SECTION("only a driving car waits")
    {
        const TrafficState others[5] = {TSArrived, TSLingering, TSStalled, TSPanicked, TSParking};
        for (int k = 0; k < 5; k++)
        {
            TrafficEncounterStep s = Traffic::StepEncounter(TrafficEncounter(), In(others[k], false, true), t);
            REQUIRE(s.action == EActNone);
            REQUIRE_FALSE(s.next.laneWait);
        }
    }
    SECTION("a raised weapon during the wait rolls, and the wait is dropped")
    {
        TrafficEncounter enc;
        enc.laneWait = true;
        TrafficEncounterStep s = Traffic::StepEncounter(enc, In(TSDriving, true, true, false, 0.9f), t);
        REQUIRE(s.action == EActBeginEscape);
        REQUIRE(s.rolled);
        REQUIRE_FALSE(s.next.laneWait);
    }
    SECTION("a missing player clears the lane: the wait resumes after the hold")
    {
        TrafficEncounter enc;
        enc.laneWait = true;
        TrafficEncounterStep s = Traffic::StepEncounter(enc, In(TSDriving, false, false, true), t);
        REQUIRE(s.action == EActNone);
        s = Traffic::StepEncounter(s.next, In(TSDriving, false, false, true), t);
        REQUIRE(s.action == EActNone);
        s = Traffic::StepEncounter(s.next, In(TSDriving, false, false, true), t);
        REQUIRE(s.action == EActResumeLaneWait);
    }
    SECTION("someone else moved the car: the wait is dropped without an order")
    {
        TrafficEncounter enc;
        enc.laneWait = true;
        TrafficEncounterStep s = Traffic::StepEncounter(enc, In(TSPanicked, false, true), t);
        REQUIRE(s.action == EActNone);
        REQUIRE_FALSE(s.next.laneWait);
    }
}

TEST_CASE("Traffic - encounter: danger precedence", "[game][guerrilla]")
{
    TrafficTuning t;
    t.commandeerSurrenderChance = 1.0f;
    SECTION("fresh danger instead of the roll; the takeover latches, no roll follows")
    {
        TrafficEncounterStep s =
            Traffic::StepEncounter(TrafficEncounter(), In(TSDriving, true, true, false, 0.1f, 0.5f, true), t);
        REQUIRE(s.action == EActDangerTakeover);
        REQUIRE_FALSE(s.rolled);
        REQUIRE(s.next.choice == EncNone);
        REQUIRE(s.next.dangerOwned);
        TrafficEncounter enc = s.next;
        for (int k = 0; k < 6; k++)
        {
            s = Traffic::StepEncounter(enc, In(TSDriving, true, true, false, 0.0f), t);
            REQUIRE(s.action == EActNone);
            REQUIRE_FALSE(s.rolled);
            enc = s.next;
        }
    }
    SECTION("fresh danger before the bail")
    {
        TrafficEncounter enc = Enc(EncSurrender);
        enc.surrenderTime = 2.4f;
        TrafficEncounterStep s = Traffic::StepEncounter(enc, In(TSStopping, true, true, false, 0, 0.5f, true), t);
        REQUIRE(s.action == EActDangerTakeover);
        REQUIRE(s.next.dangerOwned);
    }
    SECTION("fresh danger over a running escape; the choice is history")
    {
        TrafficEncounterStep s =
            Traffic::StepEncounter(Enc(EncEscape), In(TSDriving, false, false, false, 0, 0.5f, true), t);
        REQUIRE(s.action == EActDangerTakeover);
        TrafficEncounter enc = s.next;
        for (int k = 0; k < 4; k++)
        {
            s = Traffic::StepEncounter(enc, In(TSDriving, true, false), t);
            REQUIRE(s.action == EActNone);
            REQUIRE(s.next.choice == EncEscape);
            enc = s.next;
        }
        REQUIRE_FALSE(enc.EscapeOwnsMovement());
    }
    SECTION("no map-wide suppression: danger alone, or with a lane wait, takes nothing over")
    {
        TrafficEncounterStep s =
            Traffic::StepEncounter(TrafficEncounter(), In(TSDriving, false, false, false, 0, 0.5f, true), t);
        REQUIRE(s.action == EActNone);
        REQUIRE_FALSE(s.next.dangerOwned);
        TrafficEncounter wait;
        wait.laneWait = true;
        s = Traffic::StepEncounter(wait, In(TSDriving, false, true, false, 0, 0.5f, true), t);
        REQUIRE(s.action == EActNone);
        REQUIRE_FALSE(s.next.dangerOwned);
    }
    SECTION("an owned encounter ignores fresh danger; once cleared, the next threat rolls again")
    {
        TrafficEncounter enc = Enc(EncNone, true);
        TrafficEncounterStep s = Traffic::StepEncounter(enc, In(TSDriving, true, false, false, 0, 0.5f, true), t);
        REQUIRE(s.action == EActNone);
        enc.clearHold = 5.0f;
        s = Traffic::StepEncounter(enc, In(TSDriving, false, false, true), t);
        REQUIRE(s.action == EActClear);
        s = Traffic::StepEncounter(s.next, In(TSDriving, true, false, false, 0.3f), t);
        REQUIRE(s.action == EActBeginSurrender);
        REQUIRE(s.rolled);
    }
}

TEST_CASE("Traffic - encounter: a danger reaction already under way is not overwritten", "[game][guerrilla]")
{
    TrafficTuning t;
    SECTION("the episode signal: the latch running on a car that is not cowering")
    {
        REQUIRE(Traffic::InDangerEpisode(10.0f, TSDriving));
        REQUIRE(Traffic::InDangerEpisode(10.0f, TSArrived));
        REQUIRE_FALSE(Traffic::InDangerEpisode(0.0f, TSDriving));
        // a cowering car is braked at the roadside: held at gunpoint it makes
        // the ordinary choice
        REQUIRE_FALSE(Traffic::InDangerEpisode(10.0f, TSPanicked));
    }
    SECTION("a rushing or U-turning car is not rolled and does not stop for a roadblock")
    {
        TrafficEncounterInput in = In(TSDriving, true, true, false, 0.1f);
        in.dangerEpisode = true;
        TrafficEncounterStep s = Traffic::StepEncounter(TrafficEncounter(), in, t);
        REQUIRE(s.action == EActNone);
        REQUIRE_FALSE(s.rolled);
        REQUIRE(s.next.choice == EncNone);
        REQUIRE_FALSE(s.next.laneWait);
        in = In(TSDriving, false, true);
        in.dangerEpisode = true;
        s = Traffic::StepEncounter(TrafficEncounter(), in, t);
        REQUIRE(s.action == EActNone);
        REQUIRE_FALSE(s.next.laneWait);
        // fresh danger with no encounter and no possible roll: no takeover
        in = In(TSDriving, true, false, false, 0, 0.5f, true);
        in.dangerEpisode = true;
        s = Traffic::StepEncounter(TrafficEncounter(), in, t);
        REQUIRE(s.action == EActNone);
        REQUIRE_FALSE(s.next.dangerOwned);
    }
    SECTION("once the episode is over the next threat rolls as usual")
    {
        TrafficEncounterStep s = Traffic::StepEncounter(TrafficEncounter(), In(TSDriving, true, false, false, 0.1f), t);
        REQUIRE(s.rolled);
        REQUIRE(s.action == EActBeginSurrender);
    }
    SECTION("a cowering car held at gunpoint rolls")
    {
        TrafficEncounterInput in = In(TSPanicked, true, false, false, 0.9f);
        in.dangerEpisode = Traffic::InDangerEpisode(10.0f, TSPanicked);
        TrafficEncounterStep s = Traffic::StepEncounter(TrafficEncounter(), in, t);
        REQUIRE(s.rolled);
        REQUIRE(s.action == EActBeginEscape);
    }
}

TEST_CASE("Traffic - encounter: the random draw and its consumption share one test", "[game][guerrilla]")
{
    TrafficTuning t;
    SECTION("RollPossible")
    {
        REQUIRE(Traffic::RollPossible(true, TSDriving, TrafficEncounter(), false));
        REQUIRE_FALSE(Traffic::RollPossible(false, TSDriving, TrafficEncounter(), false));
        REQUIRE_FALSE(Traffic::RollPossible(true, TSParking, TrafficEncounter(), false));
        REQUIRE_FALSE(Traffic::RollPossible(true, TSStopping, TrafficEncounter(), false));
        REQUIRE_FALSE(Traffic::RollPossible(true, TSDriving, Enc(EncEscape), false));
        REQUIRE_FALSE(Traffic::RollPossible(true, TSDriving, Enc(EncNone, true), false));
        REQUIRE_FALSE(Traffic::RollPossible(true, TSDriving, TrafficEncounter(), true));
    }
    SECTION("over every input: rolled == RollPossible && no fresh danger (what the world draws for)")
    {
        const TrafficEncounterChoice choices[3] = {EncNone, EncSurrender, EncEscape};
        for (int s = 0; s < NTrafficStates; s++)
        {
            for (int bits = 0; bits < 128; bits++)
            {
                for (int c = 0; c < 3; c++)
                {
                    TrafficEncounter enc = Enc(choices[c], (bits & 1) != 0);
                    enc.laneWait = (bits & 2) != 0;
                    TrafficEncounterInput in;
                    in.state = (TrafficState)s;
                    in.armedThreat = (bits & 4) != 0;
                    in.laneObstruction = (bits & 8) != 0;
                    in.playerOutside = (bits & 16) != 0;
                    in.freshDanger = (bits & 32) != 0;
                    in.dangerEpisode = (bits & 64) != 0;
                    in.dt = 0.5f;
                    in.roll = 0.3f;
                    bool drawn =
                        Traffic::RollPossible(in.armedThreat, in.state, enc, in.dangerEpisode) && !in.freshDanger;
                    TrafficEncounterStep step = Traffic::StepEncounter(enc, in, t);
                    CHECK(step.rolled == drawn);
                }
            }
        }
    }
}

TEST_CASE("Traffic - encounter: threat eligibility", "[game][guerrilla]")
{
    TrafficTuning t;
    const TrafficState parked[3] = {TSParking, TSDwelling, TSDeparting};
    for (int k = 0; k < 3; k++)
    {
        TrafficEncounterStep s = Traffic::StepEncounter(TrafficEncounter(), In(parked[k], true, false), t);
        REQUIRE(s.action == EActNone);
        REQUIRE_FALSE(s.rolled);
        REQUIRE_FALSE(Traffic::ThreatEligible(parked[k]));
    }
    const TrafficState live[5] = {TSDriving, TSArrived, TSStalled, TSLingering, TSPanicked};
    for (int k = 0; k < 5; k++)
    {
        TrafficEncounterStep s = Traffic::StepEncounter(TrafficEncounter(), In(live[k], true, false), t);
        REQUIRE(s.rolled);
        REQUIRE(Traffic::ThreatEligible(live[k]));
    }
    REQUIRE_FALSE(Traffic::ThreatEligible(TSStopping));
    REQUIRE_FALSE(Traffic::ThreatEligible(TSExiting));
}

TEST_CASE("Traffic - encounter escape leg choice", "[game][guerrilla]")
{
    // mid-trip: the current leg, whatever resolves (the player in the lane
    // does not turn the car; the blocked recovery does)
    for (int o = 0; o < 2; o++)
    {
        for (int n = 0; n < 2; n++)
        {
            REQUIRE(Traffic::ChooseEscapeLeg(false, o != 0, n != 0) == EscCurrent);
        }
    }
    // at the destination: a new leg, else home, else nowhere
    REQUIRE(Traffic::ChooseEscapeLeg(true, false, true) == EscNewLeg);
    REQUIRE(Traffic::ChooseEscapeLeg(true, true, true) == EscNewLeg);
    REQUIRE(Traffic::ChooseEscapeLeg(true, true, false) == EscUTurn);
    REQUIRE(Traffic::ChooseEscapeLeg(true, false, false) == EscNone);
}

TEST_CASE("Traffic - encounter route speed and park eligibility", "[game][guerrilla]")
{
    REQUIRE(Traffic::RouteSpeed(TKCiv, false) == SpeedLimited);
    REQUIRE(Traffic::RouteSpeed(TKCiv, true) == SpeedFull);
    REQUIRE(Traffic::RouteSpeed(TKPatrol, false) == SpeedNormal);
    REQUIRE(Traffic::RouteSpeed(TKPatrol, true) == SpeedNormal);
    REQUIRE(Traffic::RouteSpeed(TKConvoy, false) == SpeedLimited);
    REQUIRE(Traffic::RouteSpeed(TKConvoy, true) == SpeedLimited);
    REQUIRE(Traffic::ArrivalParkEligible(TKCiv, false));
    REQUIRE_FALSE(Traffic::ArrivalParkEligible(TKCiv, true));
    REQUIRE_FALSE(Traffic::ArrivalParkEligible(TKPatrol, false));
    REQUIRE(Enc(EncEscape).EscapeOwnsMovement());
    REQUIRE_FALSE(Enc(EncEscape, true).EscapeOwnsMovement());
    REQUIRE_FALSE(Enc(EncSurrender).EscapeOwnsMovement());
}

TEST_CASE("Traffic - encounter danger state mapping and band", "[game][guerrilla]")
{
    TrafficTuning t; // dangerRadius 200
    SECTION("pending surrender, the dead exiting state and a cower decide as the car they were")
    {
        for (int s = 0; s < NTrafficStates; s++)
        {
            TrafficState st = (TrafficState)s;
            bool mapped = st == TSStopping || st == TSExiting || st == TSPanicked;
            REQUIRE(Traffic::DangerDecideState(st, true) == (mapped ? TSArrived : st));
            REQUIRE(Traffic::DangerDecideState(st, false) == (mapped ? TSDriving : st));
        }
    }
    SECTION("the band edge agrees with DecideDangerReaction")
    {
        REQUIRE(Traffic::DangerInBand(200.0f, 1.0f, t));
        REQUIRE_FALSE(Traffic::DangerInBand(200.5f, 1.0f, t));
        REQUIRE(Traffic::DecideDangerReaction(200.0f, 1.0f, TKCiv, TSDriving, 0, 0.9f, t) != TDRNone);
        REQUIRE(Traffic::DecideDangerReaction(200.5f, 1.0f, TKCiv, TSDriving, 0, 0.9f, t) == TDRNone);
        REQUIRE(Traffic::DangerInBand(300.0f, 4.0f, t));
        REQUIRE_FALSE(Traffic::DangerInBand(300.5f, 4.0f, t));
        REQUIRE_FALSE(Traffic::DangerInBand(10.0f, 0.0f, t));
        t.dangerRadius = 0;
        REQUIRE_FALSE(Traffic::DangerInBand(1.0f, 1.0f, t));
    }
    SECTION("encounter rows read the ring only; a lane wait alone does not")
    {
        REQUIRE(Traffic::EncounterDangerRingOnly(Enc(EncEscape), TSDriving));
        REQUIRE(Traffic::EncounterDangerRingOnly(Enc(EncNone, true), TSDriving));
        REQUIRE(Traffic::EncounterDangerRingOnly(TrafficEncounter(), TSStopping));
        REQUIRE(Traffic::EncounterDangerRingOnly(TrafficEncounter(), TSExiting));
        TrafficEncounter wait;
        wait.laneWait = true;
        REQUIRE_FALSE(Traffic::EncounterDangerRingOnly(wait, TSDriving));
        REQUIRE_FALSE(Traffic::EncounterDangerRingOnly(TrafficEncounter(), TSPanicked));
    }
}

TEST_CASE("Traffic - encounter clear radius", "[game][guerrilla]")
{
    TrafficTuning t;
    REQUIRE(Traffic::EncounterClearRadius(t) == Approx(60.0f));
    t.commandeerRadius = 50;
    REQUIRE(Traffic::EncounterClearRadius(t) == Approx(65.0f));
    t.commandeerRadius = 0;
    REQUIRE(Traffic::EncounterClearRadius(t) == Approx(60.0f));
}

TEST_CASE("Traffic - encounter choice enum stays append-only", "[game][guerrilla]")
{
    // saved as int: never renumber
    REQUIRE((int)EncNone == 0);
    REQUIRE((int)EncSurrender == 1);
    REQUIRE((int)EncEscape == 2);
    REQUIRE((int)NEncounterChoices == 3);
    REQUIRE(std::string(EncounterChoiceName(EncNone)) == "none");
    REQUIRE(std::string(EncounterChoiceName(EncSurrender)) == "surrender");
    REQUIRE(std::string(EncounterChoiceName(EncEscape)) == "escape");
    REQUIRE(std::string(EncounterChoiceName(9)) == "none");
}

TEST_CASE("Traffic - encounter archive round trip", "[game][guerrilla][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "traffic-encounter-roundtrip.bin";

    SECTION("the saved fields survive, the hold clocks do not")
    {
        {
            TrafficEncounter enc;
            enc.choice = EncEscape;
            enc.dangerOwned = true;
            enc.laneWait = false;
            enc.surrenderTime = 1.25f;
            enc.clearHold = 3.0f;
            enc.laneClear = 0.5f;
            ParamArchiveSave ar(WorldSerializeVersion);
            REQUIRE(SerializeEncounter(ar, enc) == LSOK);
            REQUIRE(ar.SaveBin(archivePath.string().c_str()));
        }
        TrafficEncounter loaded;
        loaded.clearHold = 7.0f; // proves the load resets it
        loaded.laneClear = 7.0f;
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(SerializeEncounter(ar, loaded) == LSOK);
        CHECK(loaded.choice == EncEscape);
        CHECK(loaded.dangerOwned);
        CHECK_FALSE(loaded.laneWait);
        CHECK(loaded.surrenderTime == Approx(1.25f));
        CHECK(loaded.clearHold < 0);
        CHECK(loaded.laneClear < 0);
    }
    SECTION("an archive without the keys (a pre-#66 save) loads no encounter")
    {
        {
            ParamArchiveSave ar(WorldSerializeVersion);
            REQUIRE(ar.SaveBin(archivePath.string().c_str()));
        }
        TrafficEncounter loaded = Enc(EncSurrender, true);
        loaded.laneWait = true;
        loaded.surrenderTime = 2.0f;
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(SerializeEncounter(ar, loaded) == LSOK);
        CHECK(IsDefault(loaded));
    }
    SECTION("an out-of-range choice loads as none")
    {
        const int bad[2] = {7, -1};
        for (int k = 0; k < 2; k++)
        {
            {
                ParamArchiveSave ar(WorldSerializeVersion);
                int c = bad[k];
                REQUIRE(ar.Serialize("encounter", c, 1) == LSOK);
                REQUIRE(ar.SaveBin(archivePath.string().c_str()));
            }
            TrafficEncounter loaded;
            ParamArchiveLoad ar;
            REQUIRE(ar.LoadBin(archivePath.string().c_str()));
            ar.FirstPass();
            REQUIRE(SerializeEncounter(ar, loaded) == LSOK);
            CHECK(loaded.choice == EncNone);
        }
    }
    std::filesystem::remove(archivePath);
}

TEST_CASE("Traffic - loaded encounter state", "[game][guerrilla][save][load]")
{
    SECTION("an unseated driver resets the encounter; stop states drive on")
    {
        TrafficState st = TSStopping;
        TrafficEncounter enc = Enc(EncSurrender);
        enc.surrenderTime = 1.5f;
        REQUIRE(Traffic::LoadedEncounterState(st, enc, false) == TLRNone);
        CHECK(st == TSDriving);
        CHECK(IsDefault(enc));

        st = TSDriving;
        enc = Enc(EncEscape);
        REQUIRE(Traffic::LoadedEncounterState(st, enc, false) == TLRNone);
        CHECK(st == TSDriving);
        CHECK(IsDefault(enc));

        st = TSPanicked;
        enc = TrafficEncounter();
        REQUIRE(Traffic::LoadedEncounterState(st, enc, false) == TLRNone);
        CHECK(st == TSDriving);

        st = TSDriving;
        enc = TrafficEncounter();
        enc.laneWait = true;
        REQUIRE(Traffic::LoadedEncounterState(st, enc, false) == TLRNone);
        CHECK(IsDefault(enc));
    }
    SECTION("a seated pending surrender keeps its stop and its elapsed delay")
    {
        TrafficState st = TSStopping;
        TrafficEncounter enc = Enc(EncSurrender);
        enc.surrenderTime = 1.5f;
        enc.clearHold = 2.0f;
        REQUIRE(Traffic::LoadedEncounterState(st, enc, true) == TLRSurrenderStop);
        CHECK(st == TSStopping);
        CHECK(enc.choice == EncSurrender);
        CHECK(enc.surrenderTime == Approx(1.5f));
        CHECK(enc.clearHold < 0);
    }
    SECTION("a pre-#66 stop or TSExiting drives on with no encounter; a danger-owned stop keeps its memory")
    {
        TrafficState st = TSStopping;
        TrafficEncounter enc;
        REQUIRE(Traffic::LoadedEncounterState(st, enc, true) == TLRResumeDrive);
        CHECK(st == TSDriving);
        CHECK(IsDefault(enc));

        st = TSStopping;
        enc = Enc(EncSurrender, true);
        enc.surrenderTime = 1.0f;
        REQUIRE(Traffic::LoadedEncounterState(st, enc, true) == TLRResumeDrive);
        CHECK(st == TSDriving);
        CHECK(enc.choice == EncSurrender);
        CHECK(enc.dangerOwned);
        CHECK(enc.surrenderTime == Approx(0.0f));

        st = TSExiting;
        enc = TrafficEncounter();
        REQUIRE(Traffic::LoadedEncounterState(st, enc, true) == TLRResumeDrive);
        CHECK(st == TSDriving);
    }
    SECTION("a cower drives on and keeps its encounter memory")
    {
        TrafficState st = TSPanicked;
        TrafficEncounter enc = Enc(EncEscape, true);
        enc.laneWait = true;
        REQUIRE(Traffic::LoadedEncounterState(st, enc, true) == TLRResumeDrive);
        CHECK(st == TSDriving);
        CHECK(enc.choice == EncEscape);
        CHECK(enc.dangerOwned);
        CHECK_FALSE(enc.laneWait);
    }
    SECTION("a roadblock wait keeps braking, an escape keeps its leg")
    {
        TrafficState st = TSDriving;
        TrafficEncounter enc;
        enc.laneWait = true;
        REQUIRE(Traffic::LoadedEncounterState(st, enc, true) == TLRLaneStop);
        CHECK(st == TSDriving);
        CHECK(enc.laneWait);

        enc = Enc(EncEscape);
        enc.surrenderTime = 1.0f;
        REQUIRE(Traffic::LoadedEncounterState(st, enc, true) == TLREscapeRoute);
        CHECK(enc.choice == EncEscape);
        CHECK(enc.surrenderTime == Approx(0.0f));
    }
    SECTION("a surrender that is no longer pending is inert and resets")
    {
        TrafficState st = TSDriving;
        TrafficEncounter enc = Enc(EncSurrender);
        REQUIRE(Traffic::LoadedEncounterState(st, enc, true) == TLRNone);
        CHECK(IsDefault(enc));
        enc = Enc(EncSurrender);
        REQUIRE(Traffic::LoadedEncounterState(st, enc, false) == TLRNone);
        CHECK(IsDefault(enc));
    }
    SECTION("a surrender gunfire took over (the car drove off) keeps its memory: no second roll after a load")
    {
        TrafficState st = TSDriving;
        TrafficEncounter enc = Enc(EncSurrender, true);
        REQUIRE(Traffic::LoadedEncounterState(st, enc, true) == TLRNone);
        CHECK(st == TSDriving);
        CHECK(enc.choice == EncSurrender);
        CHECK(enc.dangerOwned);
        // and the reloaded encounter cannot roll again
        TrafficTuning t;
        TrafficEncounterStep s = Traffic::StepEncounter(enc, In(TSDriving, true, false, false, 0.9f), t);
        CHECK_FALSE(s.rolled);
        CHECK(s.action == EActNone);
    }
    SECTION("the park family and the other states keep the choice, never the wait, never an order")
    {
        const TrafficState keep[6] = {TSParking, TSDwelling, TSDeparting, TSLingering, TSArrived, TSStalled};
        for (int k = 0; k < 6; k++)
        {
            for (int seated = 0; seated < 2; seated++)
            {
                TrafficState st = keep[k];
                TrafficEncounter enc = Enc(EncEscape, true);
                enc.laneWait = true;
                enc.surrenderTime = 1.0f;
                REQUIRE(Traffic::LoadedEncounterState(st, enc, seated != 0) == TLRNone);
                bool parkFamily = k < 4;
                CHECK(st == keep[k]);
                if (parkFamily || seated)
                {
                    CHECK(enc.choice == EncEscape);
                    CHECK(enc.dangerOwned);
                }
                CHECK_FALSE(enc.laneWait);
                CHECK(enc.surrenderTime == Approx(0.0f));
            }
        }
    }
    SECTION("never a different choice, never TSStopping from anything but a seated surrender")
    {
        const TrafficEncounterChoice choices[3] = {EncNone, EncSurrender, EncEscape};
        for (int s = 0; s < NTrafficStates; s++)
        {
            for (int c = 0; c < 3; c++)
            {
                for (int d = 0; d < 2; d++)
                {
                    for (int seated = 0; seated < 2; seated++)
                    {
                        TrafficState st = (TrafficState)s;
                        TrafficEncounter enc = Enc(choices[c], d != 0);
                        Traffic::LoadedEncounterState(st, enc, seated != 0);
                        CHECK((enc.choice == EncNone || enc.choice == choices[c]));
                        // a danger-owned encounter is never silently reset
                        // while its driver is seated (or the car is parked):
                        // a reset would hand the player a second roll
                        bool parkFamily = s == TSParking || s == TSDwelling || s == TSDeparting || s == TSLingering;
                        if (d != 0 && (seated != 0 || parkFamily))
                        {
                            CHECK(enc.dangerOwned);
                            CHECK(enc.choice == choices[c]);
                        }
                        if (st == TSStopping)
                        {
                            CHECK(s == TSStopping);
                            CHECK(choices[c] == EncSurrender);
                            CHECK(d == 0);
                            CHECK(seated == 1);
                        }
                        CHECK(enc.clearHold < 0);
                        CHECK(enc.laneClear < 0);
                    }
                }
            }
        }
    }
}
