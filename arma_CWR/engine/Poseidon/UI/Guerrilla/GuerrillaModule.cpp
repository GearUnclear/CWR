#include <Poseidon/UI/Guerrilla/GuerrillaModule.hpp>
#include <Poseidon/UI/GameModule.hpp>
#include <functional>

// The entry Display (GuerrillaNewGame) and CreateDisplayGuerrilla live in
// UI/Guerrilla/GuerrillaNewGame.cpp — this module just registers the menu
// item, mirroring the sibling modules. The application decides at startup
// which modules exist (see GameApplication.cpp), so a build/data setup
// without Guerrilla Mode simply never calls Register().
namespace Poseidon
{
void GuerrillaModule::Register()
{
    GameModuleRegistry::Register({GameModuleId::Guerrilla, "Guerrilla", IDC_MAIN_GUERRILLA, CreateDisplayGuerrilla});
}
} // namespace Poseidon
