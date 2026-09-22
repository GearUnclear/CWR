// ScriptVars: the shared null-safe readers over the script-owned globals the
// native Guerrilla services observe, and the seeded rolls both the Legend
// registry and the history generator draw from.
//
// The readers are the thing three subsystems each grew a private copy of, so
// the contracts they promise are pinned here rather than inside whichever
// service happens to call them:
//   * an absent global is not a null pointer.  VarGet hands back the shared nil
//     value, so every reader gates on GetType() and returns its fallback.
//   * VarGet lowercases the name itself, so "GM_COMP_NAMES" and "gm_comp_names"
//     are the same global.  The lowercase spelling is convention, not a rule.
//   * a wrongly typed ELEMENT of an array still occupies its slot.  That is what
//     keeps index correspondence across GM_COMP_NAMES / _XP / _RANK / _ALIVE
//     when one of them was written badly by a script.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/LegendSeed.hpp>
#include <Poseidon/Game/Guerrilla/ScriptVars.hpp>

#include <cstdio>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

// every name this file writes, so a case can hand the bank back the way it
// found it (GGameState is process-wide)
struct ScopedVars
{
    std::vector<std::string> names;

    void Set(const char* name, const GameValue& value)
    {
        names.push_back(name);
        GGameState.VarSet(name, value);
    }
    ~ScopedVars()
    {
        for (const std::string& name : names)
        {
            GGameState.VarDelete(name.c_str());
        }
    }
};

} // namespace

TEST_CASE("ScriptVars - absent and wrongly typed globals read as their fallback", "[game][guerrilla][scriptvars]")
{
    ScopedVars vars;
    vars.Set("ud_sv_scalar", GameValue(7.5f));
    vars.Set("ud_sv_string", GameValue(RString("hello")));
    vars.Set("ud_sv_bool", GameValue(true));

    // present and of the right type
    bool found = false;
    CHECK(ScriptVars::Scalar("ud_sv_scalar", -1.0f, &found) == 7.5f);
    CHECK(found);
    CHECK(Str(ScriptVars::String("ud_sv_string")) == "hello");
    CHECK(ScriptVars::Bool("ud_sv_bool"));

    // absent: the fallback, and `found` says so.  An absent global is the shared
    // nil value, never a null pointer, which is why nothing here checks for one.
    found = true;
    CHECK(ScriptVars::Scalar("ud_sv_missing", -1.0f, &found) == -1.0f);
    CHECK(!found);
    CHECK(ScriptVars::Scalar("ud_sv_missing") == 0.0f);
    CHECK(Str(ScriptVars::String("ud_sv_missing")).empty());
    CHECK(!ScriptVars::Bool("ud_sv_missing"));
    CHECK(ScriptVars::ArraySize("ud_sv_missing") == 0);

    // present but of another type: the same fallback, no crash, no coercion
    found = true;
    CHECK(ScriptVars::Scalar("ud_sv_string", -1.0f, &found) == -1.0f);
    CHECK(!found);
    CHECK(Str(ScriptVars::String("ud_sv_scalar")).empty());
    CHECK(!ScriptVars::Bool("ud_sv_scalar"));
    CHECK(ScriptVars::ArraySize("ud_sv_scalar") == 0);

    // a null name is legal and reads as absent
    CHECK(ScriptVars::Scalar(nullptr, 3.0f) == 3.0f);
    CHECK(Str(ScriptVars::String(nullptr)).empty());
    CHECK(!ScriptVars::Bool(nullptr));
    CHECK(ScriptVars::ArraySize(nullptr) == 0);
}

TEST_CASE("ScriptVars - a global's name is case insensitive", "[game][guerrilla][scriptvars]")
{
    ScopedVars vars;
    // the scripts spell it GM_COMP_NAMES; VarGet lowercases both ends
    vars.Set("GM_UD_SV_MIXED", GameValue(RString("Petra")));
    CHECK(Str(ScriptVars::String("gm_ud_sv_mixed")) == "Petra");
    CHECK(Str(ScriptVars::String("GM_UD_SV_MIXED")) == "Petra");
    CHECK(Str(ScriptVars::String("Gm_Ud_Sv_Mixed")) == "Petra");
}

TEST_CASE("ScriptVars - a wrongly typed array element keeps its slot", "[game][guerrilla][scriptvars]")
{
    ScopedVars vars;

    GameArrayType names;
    names.Add(GameValue(RString("Petra")));
    names.Add(GameValue(4.0f)); // a script wrote a number into a name slot
    names.Add(GameValue(RString("Yazan")));
    vars.Set("ud_sv_names", GameValue(names));

    GameArrayType xp;
    xp.Add(GameValue(100.0f));
    xp.Add(GameValue(RString("nope")));
    xp.Add(GameValue(250.0f));
    vars.Set("ud_sv_xp", GameValue(xp));

    GameArrayType alive;
    alive.Add(GameValue(true));
    alive.Add(GameValue(0.0f));
    alive.Add(GameValue(false));
    vars.Set("ud_sv_alive", GameValue(alive));

    AutoArray<RString> readNames;
    REQUIRE(ScriptVars::StringArray("ud_sv_names", readNames));
    REQUIRE(readNames.Size() == 3);
    CHECK(Str(readNames[0]) == "Petra");
    CHECK(Str(readNames[1]).empty()); // the bad element, still at index 1
    CHECK(Str(readNames[2]) == "Yazan");

    AutoArray<float> readXp;
    REQUIRE(ScriptVars::ScalarArray("ud_sv_xp", readXp));
    REQUIRE(readXp.Size() == 3);
    CHECK(readXp[0] == 100.0f);
    CHECK(readXp[1] == 0.0f);
    CHECK(readXp[2] == 250.0f);

    AutoArray<bool> readAlive;
    REQUIRE(ScriptVars::BoolArray("ud_sv_alive", readAlive));
    REQUIRE(readAlive.Size() == 3);
    CHECK(readAlive[0]);
    CHECK(!readAlive[1]);
    CHECK(!readAlive[2]);

    // index correspondence across the three arrays is what all of this is for
    CHECK(readNames.Size() == readXp.Size());
    CHECK(readNames.Size() == readAlive.Size());
    CHECK(ScriptVars::ArraySize("ud_sv_names") == 3);
}

TEST_CASE("ScriptVars - an absent or mistyped array clears the caller's buffer", "[game][guerrilla][scriptvars]")
{
    ScopedVars vars;
    vars.Set("ud_sv_notanarray", GameValue(RString("scalar")));

    AutoArray<RString> strings;
    strings.Add(RString("stale"));
    CHECK(!ScriptVars::StringArray("ud_sv_missing_array", strings));
    CHECK(strings.Size() == 0);

    strings.Add(RString("stale"));
    CHECK(!ScriptVars::StringArray("ud_sv_notanarray", strings));
    CHECK(strings.Size() == 0);

    AutoArray<float> scalars;
    scalars.Add(1.0f);
    CHECK(!ScriptVars::ScalarArray("ud_sv_missing_array", scalars));
    CHECK(scalars.Size() == 0);

    AutoArray<bool> bools;
    bools.Add(true);
    CHECK(!ScriptVars::BoolArray("ud_sv_missing_array", bools));
    CHECK(bools.Size() == 0);

    // an empty array is present, not absent
    GameArrayType empty;
    vars.Set("ud_sv_empty", GameValue(empty));
    CHECK(ScriptVars::StringArray("ud_sv_empty", strings));
    CHECK(strings.Size() == 0);
}

TEST_CASE("LegendSeed - the rolls are pure and every channel is its own", "[game][guerrilla][legends][seed]")
{
    const unsigned long long key = HashKey("comp_0_petra", 12345);
    const unsigned long long same = HashKey("comp_0_petra", 12345);
    const unsigned long long other = HashKey("comp_1_yazan", 12345);
    const unsigned long long reseeded = HashKey("comp_0_petra", 999);

    CHECK(key == same); // pure in both arguments
    CHECK(key != other);
    CHECK(key != reseeded);
    CHECK(HashKey(nullptr, 12345) == HashKey("", 12345)); // a null key is the empty key

    // a roll is a pure function of (key, channel, mod)
    for (int i = 0; i < 64; i++)
    {
        CHECK(Roll(key, CH_LAST, 20) == Roll(key, CH_LAST, 20));
    }
    CHECK(Roll(key, CH_LAST, 20) < 20u);
    CHECK(Roll(key, CH_LAST, 0) == 0u);
    CHECK(Roll(key, CH_LAST, -3) == 0u);
    CHECK(Roll(key, CH_LAST, 1) == 0u);

    // ONE draw kind per channel, never reused: that is what keeps a row's rolls
    // from moving when another draw is added.  Award slot and award word are
    // four separate kinds, not two.
    const int channels[] = {CH_OPEN_A,    CH_OPEN_B,   CH_OPEN_C,   CH_OPEN_D,  CH_EVENT0,     CH_EVENT1,
                            CH_EVENT2,    CH_PLACE0,   CH_PLACE1,   CH_PLACE2,  CH_LAST,       CH_FACE,
                            CH_BIOEVENT,  CH_BIOVAR,   CH_AWARD1,   CH_AWARD2,  CH_AWARD1WORD, CH_AWARD2WORD,
                            CH_BOSSFIRST, CH_BOSSLAST, CH_BOSSSLOT, CH_BOSSWORD};
    const int n = (int)(sizeof(channels) / sizeof(channels[0]));
    for (int i = 0; i < n; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            CHECK(channels[i] != channels[j]);
        }
    }

    // and two channels off one key really do disagree most of the time
    int differ = 0;
    for (int i = 0; i < 200; i++)
    {
        char name[32];
        snprintf(name, sizeof(name), "comp_%d_x", i);
        const unsigned long long k = HashKey(name, 4242);
        if (Roll(k, CH_AWARD1, 3) != Roll(k, CH_AWARD1WORD, 3))
        {
            differ++;
        }
    }
    CHECK(differ > 100);
}
