#include "GameDemoApplication.hpp"
#include <Poseidon/UI/Missions/MissionsModule.hpp>
#include <Poseidon/UI/Editor/EditorModule.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaModule.hpp>

void GameDemoApplication::RegisterGameModules()
{
    // Demo ships Single Missions only; unregistered modules stay disabled in the main menu.
    Poseidon::MissionsModule::Register();
    // Guerrilla Mode develops against the Demo data (guerrilla-mode/); the
    // entry stays harmless without a Guerrilla.<World> template on disk.
    Poseidon::GuerrillaModule::Register();
    // Poseidon::CampaignsModule::Register();
    // Poseidon::MultiplayerModule::Register();
    // Poseidon::EditorModule::Register();
    // ModsModule::Register();
}
