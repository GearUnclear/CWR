#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <Poseidon/Game/Guerrilla/AssailantSystem.hpp>
#include <Poseidon/AI/AI.hpp>
using namespace Poseidon;
using namespace Poseidon::Guerrilla;

TEST_CASE("Assailant spare side covers every ordered war pairing", "[guerrilla][assailants][ai]")
{
    int sides[] = {TEast, TWest, TGuerrila};
    for (int occ : sides) for (int res : sides)
    {
        int spare = AssailantSystem::SpareSide(occ, res);
        if (occ == res) REQUIRE(spare == -1);
        else { REQUIRE(spare != occ); REQUIRE(spare != res); REQUIRE(spare >= 0); }
    }
    REQUIRE(AssailantSystem::SpareSide(TCivilian, TWest) == -1);
    REQUIRE(AssailantSystem::SpareSide(TEast, TSideUnknown) == -1);
}
TEST_CASE("Individual hostility preserves defensive and indiscriminate modes", "[guerrilla][assailants][ai]")
{
    for (bool personal : {false, true}) for (bool combatant : {false, true}) for (bool rogue : {false, true})
    {
        REQUIRE(AssailantSystem::Hostile(ASRogue, personal, combatant, rogue));
        REQUIRE(AssailantSystem::Hostile(ASResister, personal, combatant, rogue) == (personal || combatant || rogue));
        REQUIRE_FALSE(AssailantSystem::Hostile(ASNone, personal, combatant, rogue));
    }
}
TEST_CASE("Assailant admission has independent live and rogue limits", "[guerrilla][assailants]")
{
    REQUIRE(AssailantSystem::HasCapacity(7, 2, ASResister));
    REQUIRE_FALSE(AssailantSystem::HasCapacity(7, 2, ASRogue));
    REQUIRE(AssailantSystem::HasCapacity(7, 1, ASRogue));
    REQUIRE_FALSE(AssailantSystem::HasCapacity(8, 0, ASResister));
    REQUIRE_FALSE(AssailantSystem::HasCapacity(8, 0, ASRogue));
    REQUIRE_FALSE(AssailantSystem::HasCapacity(0, 0, ASNone));
}
TEST_CASE("One exponential countdown pauses and never accumulates debt", "[guerrilla][assailants]")
{
    const float median = AssailantSystem::Interval(0.5f);
    REQUIRE(median == Catch::Approx(1663.5532f));
    REQUIRE(AssailantSystem::Interval(0) > 0);
    REQUIRE(AssailantSystem::Interval(1) > median);
    REQUIRE(AssailantSystem::Advance(300, 10000, false) == 300);
    REQUIRE(AssailantSystem::Advance(300, 100, true) == 200);
    REQUIRE(AssailantSystem::Advance(300, 10000, true) == 0);
    REQUIRE(AssailantSystem::Advance(300, -50, true) == 300);
    float saved = AssailantSystem::Advance(median, 100, true);
    REQUIRE(AssailantSystem::Advance(saved, 10, true) == Catch::Approx(median - 110));
}
