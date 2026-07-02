#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <Evaluator/express.hpp>
#include <string>

#include "evaluator_fixture.hpp"

// ============================================================================
// Evaluator testing - CODE values (compile / isNil / code-driven control flow)
// ============================================================================
// The engine has no bytecode stage: a CODE value (GameDataCode, type GameCode)
// still carries the source text, but as a distinct type so scripts can pass
// functions around (`GM_fn = compile "..."; call GM_fn`) with type checking.
//
// Covered here:
// - compile: STRING -> CODE (and CODE -> CODE, idempotent)
// - call (unary and binary `args call code` with _this injection)
// - isNil on global/local names and on code operands
// - private declaring undefined locals + `private _x = 5` shorthand
// - code accepted by then / exitWith / do (while and for) / forEach / count
// - regression: `{...}` literals still evaluate to STRING
// ============================================================================

namespace
{
std::string TextOf(const GameValue& v)
{
    return std::string(v.GetText().Data());
}
} // namespace

TEST_CASE("compile returns a CODE value wrapping the source", "[evaluator][code]")
{
    EvaluatorFixture fixture;

    SECTION("type, type name and braced text")
    {
        GameValue v = GGameState.Evaluate("compile \"1+2\"");

        REQUIRE(v.GetType() == GameCode);
        REQUIRE(std::string(v.GetData()->GetTypeName()) == "code");
        // GetText renders the braced form...
        REQUIRE(TextOf(v) == "{1+2}");
        // ...while the payload string is the raw source
        RString source = v;
        REQUIRE(std::string(source.Data()) == "1+2");
    }

    SECTION("compile of code is idempotent")
    {
        GameValue v = GGameState.Evaluate("compile compile \"1+2\"");

        REQUIRE(v.GetType() == GameCode);
        REQUIRE(TextOf(v) == "{1+2}");

        // still callable after double compile
        GameValue r = GGameState.Evaluate("call compile compile \"1+2\"");
        REQUIRE((float)r == Catch::Approx(3.0f));
    }
}

TEST_CASE("call executes CODE values", "[evaluator][code]")
{
    EvaluatorFixture fixture;

    SECTION("unary call")
    {
        GameValue v = GGameState.Evaluate("call compile \"1+2\"");

        REQUIRE(v.GetType() == GameScalar);
        REQUIRE((float)v == Catch::Approx(3.0f));
        REQUIRE(GGameState.GetLastError() == EvalOK);
    }

    SECTION("binary call injects _this")
    {
        GameValue v = GGameState.Evaluate("[4, 5] call compile \"(_this select 0) + (_this select 1)\"");

        REQUIRE((float)v == Catch::Approx(9.0f));
        REQUIRE(GGameState.GetLastError() == EvalOK);
    }

    SECTION("code stored in a global stays callable")
    {
        GGameState.EvaluateMultiple("gCodeFn = compile \"2 * 21\"");

        GameValue stored = GGameState.VarGet("gCodeFn");
        REQUIRE(stored.GetType() == GameCode);

        GameValue v = GGameState.Evaluate("call gCodeFn");
        REQUIRE((float)v == Catch::Approx(42.0f));

        GGameState.VarDelete("gCodeFn");
    }
}

TEST_CASE("isNil on variable names", "[evaluator][code]")
{
    EvaluatorFixture fixture;

    SECTION("global: true when undefined, false when set, true after delete")
    {
        // never defined
        REQUIRE((bool)GGameState.Evaluate("isNil \"gNilProbeCWR\"") == true);
        REQUIRE(GGameState.GetLastError() == EvalOK);

        GGameState.EvaluateMultiple("gNilProbeCWR = 5");
        REQUIRE((bool)GGameState.Evaluate("isNil \"gNilProbeCWR\"") == false);

        GGameState.VarDelete("gNilProbeCWR");
        REQUIRE((bool)GGameState.Evaluate("isNil \"gNilProbeCWR\"") == true);
    }

    SECTION("local without any context is true and does not raise an error")
    {
        GameValue v = GGameState.Evaluate("isNil \"_neverDefinedLocal\"");

        REQUIRE(v.GetType() == GameBool);
        REQUIRE((bool)v == true);
        REQUIRE(GGameState.GetLastError() == EvalOK);
    }

    SECTION("local inside a context")
    {
        GameVarSpace local;
        GGameState.BeginContext(&local);

        REQUIRE((bool)GGameState.Evaluate("isNil \"_x\"") == true);
        REQUIRE(GGameState.GetLastError() == EvalOK);

        GGameState.EvaluateMultiple("_x = 5");
        REQUIRE((bool)GGameState.Evaluate("isNil \"_x\"") == false);

        GGameState.EndContext();
    }
}

TEST_CASE("isNil evaluates CODE operands", "[evaluator][code]")
{
    EvaluatorFixture fixture;

    SECTION("nil result")
    {
        REQUIRE((bool)GGameState.Evaluate("isNil compile \"nil\"") == true);
        REQUIRE(GGameState.GetLastError() == EvalOK);
    }

    SECTION("non-nil result")
    {
        REQUIRE((bool)GGameState.Evaluate("isNil compile \"42\"") == false);
        REQUIRE(GGameState.GetLastError() == EvalOK);
    }

    SECTION("empty code yields nothing, which counts as nil")
    {
        REQUIRE((bool)GGameState.Evaluate("isNil compile \"\"") == true);
        REQUIRE(GGameState.GetLastError() == EvalOK);
    }
}

TEST_CASE("private declares undefined locals", "[evaluator][code]")
{
    EvaluatorFixture fixture;

    GameVarSpace local;
    GGameState.BeginContext(&local);

    SECTION("private \"_x\" makes the local exist but undefined")
    {
        GGameState.EvaluateMultiple("private \"_pv\"");
        REQUIRE(GGameState.GetLastError() == EvalOK);

        REQUIRE((bool)GGameState.Evaluate("isNil \"_pv\"") == true);

        // assignable afterwards
        GGameState.EvaluateMultiple("_pv = 3");
        REQUIRE((bool)GGameState.Evaluate("isNil \"_pv\"") == false);
        REQUIRE((float)GGameState.Evaluate("_pv") == Catch::Approx(3.0f));
    }

    SECTION("private _x = 5 assignment shorthand")
    {
        GGameState.EvaluateMultiple("private _qv = 5");
        REQUIRE(GGameState.GetLastError() == EvalOK);

        REQUIRE((bool)GGameState.Evaluate("isNil \"_qv\"") == false);
        REQUIRE((float)GGameState.Evaluate("_qv") == Catch::Approx(5.0f));
    }

    GGameState.EndContext();
}

TEST_CASE("code drives if-then", "[evaluator][code]")
{
    EvaluatorFixture fixture;

    SECTION("taken branch returns the code's value")
    {
        GameValue v = GGameState.Evaluate("if (true) then compile \"7\"");

        REQUIRE(v.GetType() == GameScalar);
        REQUIRE((float)v == Catch::Approx(7.0f));
    }

    SECTION("untaken branch returns nil (same as the string form)")
    {
        GameValue v = GGameState.Evaluate("if (false) then compile \"7\"");
        GameValue s = GGameState.Evaluate("if (false) then \"7\"");

        REQUIRE(v.GetNil());
        REQUIRE(v.GetType() == s.GetType());
    }
}

TEST_CASE("code drives while-do", "[evaluator][code]")
{
    EvaluatorFixture fixture;

    GGameState.EvaluateMultiple("gCodeCnt = 0");

    GGameState.Evaluate("while compile \"gCodeCnt < 3\" do compile \"gCodeCnt = gCodeCnt + 1\"");

    REQUIRE(GGameState.GetLastError() == EvalOK);
    REQUIRE((float)GGameState.VarGet("gCodeCnt") == Catch::Approx(3.0f));

    GGameState.VarDelete("gCodeCnt");
}

TEST_CASE("code drives forEach and count", "[evaluator][code]")
{
    EvaluatorFixture fixture;

    SECTION("forEach accumulates via _x")
    {
        GGameState.EvaluateMultiple("gCodeSum = 0; gCodeBody = compile \"gCodeSum = gCodeSum + _x\"");

        GGameState.Evaluate("gCodeBody forEach [1, 2, 3]");

        REQUIRE(GGameState.GetLastError() == EvalOK);
        REQUIRE((float)GGameState.VarGet("gCodeSum") == Catch::Approx(6.0f));

        GGameState.VarDelete("gCodeSum");
        GGameState.VarDelete("gCodeBody");
    }

    SECTION("count with a code condition (condition on the left)")
    {
        GameValue n = GGameState.Evaluate("(compile \"_x > 1\") count [1, 2, 3]");

        REQUIRE(n.GetType() == GameScalar);
        REQUIRE((float)n == Catch::Approx(2.0f));
    }

    SECTION("count with a string condition still works (regression)")
    {
        GameValue n = GGameState.Evaluate("\"_x > 1\" count [1, 2, 3]");

        REQUIRE((float)n == Catch::Approx(2.0f));
    }
}

TEST_CASE("exitWith code propagates the exit value out of a for-loop", "[evaluator][code]")
{
    EvaluatorFixture fixture;

    SECTION("exitWith with a CODE operand")
    {
        GGameState.EvaluateMultiple("gExitCode = compile \"_i * 2\"");

        GameValue v = GGameState.Evaluate("for \"_i\" from 1 to 10 do compile \"if (_i == 3) exitWith gExitCode\"");

        REQUIRE(GGameState.GetLastError() == EvalOK);
        REQUIRE((float)v == Catch::Approx(6.0f));

        GGameState.VarDelete("gExitCode");
    }

    SECTION("exitWith with a brace-string operand inside compiled body")
    {
        GameValue v = GGameState.Evaluate("for \"_i\" from 1 to 5 do compile \"if (_i == 2) exitWith {_i + 10}\"");

        REQUIRE(GGameState.GetLastError() == EvalOK);
        REQUIRE((float)v == Catch::Approx(12.0f));
    }
}

TEST_CASE("brace literals still evaluate to STRING (backward compat)", "[evaluator][code]")
{
    EvaluatorFixture fixture;

    SECTION("literal type is STRING, not CODE")
    {
        GameValue v = GGameState.Evaluate("{1+2}");

        REQUIRE(v.GetType() == GameString);
        RString s = v;
        REQUIRE(std::string(s.Data()) == "1+2");
    }

    SECTION("brace literals remain callable as strings")
    {
        GameValue v = GGameState.Evaluate("call {1+2}");

        REQUIRE((float)v == Catch::Approx(3.0f));
    }
}
