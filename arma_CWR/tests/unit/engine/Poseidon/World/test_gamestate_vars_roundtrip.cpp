// ParamArchive save/load must round-trip every
// entry of GameState._vars — that container holds every named SQF
// variable in a mission (commander, jeep, every text="..." object,
// every player-script-set value).  When World::LoadBin runs without
// _vars being restored, the post-load SQF environment is empty: the
// `jeep` binding becomes not_an_object, `commander sideRadio "MSG2"`
// no-ops, and the entire trigger graph stalls.
//
// What broke this in OFPR:  expressExt.cpp owns the GParamArchiveFunctions
// and GGameStateStringtableInfoFunctions globals that wire GameState's
// _defaultArchiveFunctions / _defaultInfoFunctions slots to real
// implementations.  Once expressExt.cpp moved into Poseidon.lib and
// nothing else in the program referenced any of its symbols, the linker
// silently dropped the TU; the defaults stayed at the no-op base, and
// every GameState::Serialize static helper became a no-op write/read.
//
// The fix is the explicit-init pattern in engine/Poseidon/Foundation/Platform/
// PoseidonInit.{hpp,cpp}: each app's main() calls Poseidon::InitDefaults(),
// which forwards to InitScriptingDefaults() (and its peers).  Both
// keeps the TU live and re-registers the real impls after any
// cross-library dynamic-init clobber.  This test is the regression
// that proves the pattern still works.
//
// Test mechanics: set a scalar var, save via ParamArchive, Reset()
// (matching what World::Clear→CleanUpDeinit→GGameState.Reset() does),
// load with FirstPass + SecondPass, assert the var came back.  Then
// the same for a string var.  If either fails, save/load of SQF state
// is silently dropped — the user-visible symptom.

#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Core/SaveVersion.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/Game/Scripting/Scripts.hpp>
#include <Evaluator/express.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <filesystem>
#include <string.h>
#include <catch2/matchers/catch_matchers.hpp>
#include <string>
#include <Poseidon/Foundation/Strings/RString.hpp>

using namespace Poseidon;

namespace
{
RString NumberedName(const char* prefix, int index)
{
    return RString((std::string(prefix) + std::to_string(index)).c_str());
}

LSError SerializeVars(ParamArchive& ar)
{
    void* old = ar.GetParams();
    ar.SetParams(&GGameState);
    LSError result = ar.Serialize("Variables", GGameState.GetVariables(), 1);
    ar.SetParams(old);
    return result;
}
} // namespace

TEST_CASE("GameState second load pass follows saved names after rehash", "[gameState][vars][save][load]")
{
    const auto directory = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(directory);
    const auto path = directory / "gamestate-vars-rehash.bin";
    GGameState.Init();
    GGameState.Reset();
    // Mix scalar/string/array values: pairing different archive items on the
    // reference pass either errors on data.value or silently corrupts links.
    for (int i = 0; i < 600; ++i)
    {
        const RString name = NumberedName("human_suite_", i);
        GameValue value(static_cast<float>(i));
        if (i % 3 == 1)
            value = GameValue(NumberedName("text_", i));
        if (i % 3 == 2)
        {
            value = GGameState.CreateGameValue(GameArray);
            GameArrayType& array = value;
            array.Add(GameValue(static_cast<float>(i)));
            array.Add(GameValue(NumberedName("nested_", i)));
        }
        GGameState.VarSet(name, value, true);
    }
    {
        ParamArchiveSave archive(WorldSerializeVersion);
        REQUIRE(GGameState.Serialize(archive) == LSOK);
        REQUIRE(archive.SaveBin(path.string().c_str()));
    }
    GGameState.Reset();
    {
        ParamArchiveLoad archive;
        REQUIRE(archive.LoadBin(path.string().c_str()));
        archive.FirstPass();
        REQUIRE(GGameState.Serialize(archive) == LSOK);
        GGameState.GetVariables().Rebuild(GGameState.GetVariables().NTables() * 2 + 1);
        GGameState.VarSet("added_between_passes", GameValue(99.0f), true);
        archive.SecondPass();
        REQUIRE(GGameState.Serialize(archive) == LSOK);
    }
    for (int i = 0; i < 600; ++i)
    {
        const GameValue& value = GGameState.VarGet(NumberedName("human_suite_", i));
        if (i % 3 == 0)
        {
            REQUIRE(value.GetType() == GameScalar);
            CHECK(static_cast<float>(value) == static_cast<float>(i));
        }
        else if (i % 3 == 1)
        {
            REQUIRE(value.GetType() == GameString);
            CHECK(static_cast<RString>(value) == NumberedName("text_", i));
        }
        else
        {
            REQUIRE(value.GetType() == GameArray);
            const GameArrayType& array = value;
            REQUIRE(array.Size() == 2);
            CHECK(static_cast<float>(array[0]) == static_cast<float>(i));
            CHECK(static_cast<RString>(array[1]) == NumberedName("nested_", i));
        }
    }
    CHECK(static_cast<float>(GGameState.VarGet("added_between_passes")) == 99.0f);
    GGameState.Reset();
    std::filesystem::remove(path);
}

TEST_CASE("GameState scalar var survives ParamArchive save/load", "[gameState][vars][save][load]")
{
    // test_main.cpp calls Poseidon::InitDefaults() up-front, so the
    // ArchiveFunctions slot is already wired by the time this runs.
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "gamestate-vars-scalar.bin";

    {
        GGameState.Init();
        GGameState.Reset();
        GGameState.VarSet("regress_scalar", GameValue(42.5f), true);

        REQUIRE(GGameState.VarGet("regress_scalar").GetType() == GameScalar);

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(SerializeVars(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }

    GGameState.Reset();
    REQUIRE(GGameState.VarGet("regress_scalar").GetType() != GameScalar);

    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(SerializeVars(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(SerializeVars(ar) == LSOK);
    }

    GameValue restored = GGameState.VarGet("regress_scalar");
    CHECK(restored.GetType() == GameScalar);
    CHECK_THAT(static_cast<float>(restored), Catch::Matchers::WithinAbs(42.5f, 0.001f));

    std::filesystem::remove(archivePath);
}

TEST_CASE("GameState string var survives ParamArchive save/load", "[gameState][vars][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "gamestate-vars-string.bin";

    {
        GGameState.Init();
        GGameState.Reset();
        GGameState.VarSet("regress_string", GameValue(RString("hello world")), true);
        REQUIRE(GGameState.VarGet("regress_string").GetType() == GameString);

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(SerializeVars(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }

    GGameState.Reset();
    REQUIRE(GGameState.VarGet("regress_string").GetType() != GameString);

    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(SerializeVars(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(SerializeVars(ar) == LSOK);
    }

    GameValue restored = GGameState.VarGet("regress_string");
    CHECK(restored.GetType() == GameString);
    CHECK(strcmp((const char*)static_cast<RString>(restored), "hello world") == 0);

    std::filesystem::remove(archivePath);
}
