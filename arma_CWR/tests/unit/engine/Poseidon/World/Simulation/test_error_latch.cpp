// test_error_latch.cpp - the WarningMessageLevel error latch that rejects an
// --autotest boot (World::InitVehicles returns GetMaxError() < EMError), the
// reason the boot failure reports, and the WARN line logged when it latches.

#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Foundation/Logging/Logging.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>
#include <spdlog/sinks/callback_sink.h>
#include <spdlog/logger.h>
#include <algorithm>
#include <cstdarg>
#include <memory>
#include <string>
#include <vector>

namespace Poseidon
{
void WarningMessageLevel(Foundation::ErrorMessageLevel level, const char* format, va_list argptr);
} // namespace Poseidon

namespace
{
void Latch(Poseidon::Foundation::ErrorMessageLevel level, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    Poseidon::WarningMessageLevel(level, format, args);
    va_end(args);
}
} // namespace

TEST_CASE("error latch: a plain warning rejects the boot and keeps its text", "[world][errorlatch]")
{
    using namespace Poseidon::Foundation;
    using LS = LoggingSystem;
    LS logSys;
    logSys.Initialize("trace");

    std::vector<std::pair<spdlog::level::level_enum, std::string>> lines;
    auto sink = std::make_shared<spdlog::sinks::callback_sink_mt>(
        [&lines](const spdlog::details::log_msg& msg)
        { lines.emplace_back(msg.level, std::string(msg.payload.data(), msg.payload.size())); });
    auto& core = LogDetail::g_loggers[static_cast<int>(LogCategory::Core)];
    REQUIRE(core);
    core->sinks().push_back(sink);

    LS::SetStrictMode(true);
    LS::ResetErrorCount();
    Poseidon::ResetErrors();
    CHECK(Poseidon::GetMaxError() < EMError);
    CHECK(std::string(Poseidon::DescribeErrorLatch(Poseidon::GetMaxError(), Poseidon::GetMaxErrorMessage())) ==
          "no error latched");

    // WarningMessageLevel stores level + 1, so EMWarning already rejects a boot.
    Latch(EMWarning, "Cannot create '%s'", "LoBo_throw2");
    CHECK(Poseidon::GetMaxError() >= EMError);
    CHECK(std::string(Poseidon::GetMaxErrorMessage()) == "Cannot create 'LoBo_throw2'");
    CHECK(std::string(Poseidon::DescribeErrorLatch(Poseidon::GetMaxError(), Poseidon::GetMaxErrorMessage())) ==
          "latched warning: Cannot create 'LoBo_throw2'");

    // A second warning at the same level neither replaces the text nor logs again.
    Latch(EMWarning, "second warning");
    CHECK(std::string(Poseidon::GetMaxErrorMessage()) == "Cannot create 'LoBo_throw2'");

    int warnLines = 0;
    for (const auto& [level, text] : lines)
    {
        CHECK(level < spdlog::level::err);
        if (text.find("Cannot create 'LoBo_throw2'") != std::string::npos)
        {
            CHECK(level == spdlog::level::warn);
            ++warnLines;
        }
        CHECK(text.find("second warning") == std::string::npos);
    }
    CHECK(warnLines == 1);
    // Logged at WARN, so --strict stays untripped.
    CHECK_FALSE(LS::StrictTripped());

    Poseidon::ResetErrors();
    CHECK(Poseidon::GetMaxError() < EMError);
    CHECK(Poseidon::GetMaxErrorMessage().GetLength() == 0);

    auto& sinks = core->sinks();
    sinks.erase(std::remove(sinks.begin(), sinks.end(), sink), sinks.end());
    LS::SetStrictMode(false);
    LS::ResetErrorCount();
}

TEST_CASE("error latch: describe a latch without text, and the boot-failure slot", "[world][errorlatch]")
{
    using namespace Poseidon::Foundation;
    CHECK(std::string(Poseidon::DescribeErrorLatch(EMError, RString())) == "error latch set without a message");

    Poseidon::SetAutoTestBootFailure("latched warning: x");
    CHECK(std::string(Poseidon::GetAutoTestBootFailure()) == "latched warning: x");
    Poseidon::SetAutoTestBootFailure(RString());
    CHECK(Poseidon::GetAutoTestBootFailure().GetLength() == 0);
}
