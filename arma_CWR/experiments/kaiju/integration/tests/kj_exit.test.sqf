// ============================================================================
//  Dedushka integration: the game must exit cleanly with a kjLight still on
//  him. Before the engine's Kaiju::Shutdown (called from World::~World) the
//  exit died with an access violation after "Shutdown complete" (the static
//  record list destroyed after the world its lights detach from). This test
//  ends ON PURPOSE with the furnace light, the embers and every loop of the
//  set still running. The verdict is the KJX RESULT line AND the game's exit
//  code (run-kj.ps1 counts a crash at exit as a failure).
//      run-kj.ps1 -Name kj_exit
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [4900, 4080, 0]
call compile preprocessFile "kjd_params.sqf"
if (isNil "KJD_class") then { KJD_class = "UDDedushka" }
kjxC = createCenter civilian
kjxG = createGroup civilian
kaiju = objNull
KJD_class createUnit [[4900, 4004, 0], kjxG, "kaiju = this", 0.5, "PRIVATE"]
triSimFrames 5
triAssert [!(isNull kaiju)]
kjxNative = (format ["%1", kjVersion]) != (format ["%1", KJ_noSuchVariable_])
triAssert [kjxNative]
if (true) then { kaiju setDir 90; kaiju setPos [4900, 4004, 0]; kaiju setCaptive true; KJ_brainAuto = false; KJ_ready = false; [kaiju, "idle"] exec "kj\start.sqs" }
triSimUntil { KJ_ready }
triSimFrames 120
kjxLights = kjLightCount kaiju
logInfo format ["KJX at the end: state %1, furnace on %2, lights on him %3, alive %4", KJ_state, KJ_furnaceOn, kjxLights, alive kaiju]
kjxFails = []
if (kjxLights < 1) then { kjxFails = kjxFails + ["a kjLight is on him when the game exits"] }
if (!KJ_furnaceOn) then { kjxFails = kjxFails + ["the furnace loop is still running when the game exits"] }
logInfo format ["KJX RESULT: %1 checks failed: %2", count kjxFails, kjxFails]
// no clean-up on purpose: the light is on him as the game goes down
triEndTest
