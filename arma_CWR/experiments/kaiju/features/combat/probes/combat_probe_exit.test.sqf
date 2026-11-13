// ============================================================================
//  Dedushka COMBAT, does a light still bound to him at exit crash the game
//  (a diagnostic, not part of the gate). Nothing fires. combat_wounds once
//  ended with its verdict PASS and the game then died on the way out with an
//  access violation (exit code -1073741819); a kjLight of the wound effects
//  was probably still on him. This run ends with one on him, on purpose.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
call compile preprocessFile "kjt.sqf"
call compile preprocessFile "kjt_class.sqf"
[kjtClass, 0] call kjtSpawn
triSimFrames 5
triAssert [!(isNull kjtDed)]
call kjtHold
triSimFrames 30
kjxOk = kjLight [kjtDed, "kjx", [0, 5, 30], [1, 0.5, 0.15], 3]
triSimFrames 30
logInfo format ["KJX light set %1, lights on him %2: ending the test with it on", kjxOk, kjLightCount kjtDed]
triEndTest
