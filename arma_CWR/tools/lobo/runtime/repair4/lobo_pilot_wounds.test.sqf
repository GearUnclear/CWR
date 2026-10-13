// ============================================================================
//  Wounded LoBo pilot visuals (repair 2026-10-11.4, R6, installed @LoBo only).
//
//  For each LoBo_Pilot class: the unit is created from the script after the
//  boot (the mission places only SoldierWB and the r4East host pilot), made
//  captive and stationary, photographed healthy from behind, then wounded and
//  photographed from the front, from behind and close behind.
//
//  Wounding: Man::WoundsAnimation swaps a healthy texture for its wounds[]
//  partner only while that part's hit is at least 0.7 (head 0.8), and
//  setDammage sets every part to the same value, so setDammage 0.5 alone shows
//  no wound. rpWound therefore sets total damage 0.5 and raises the body
//  ("telo"), hands ("ruce") and legs ("nohy") hits to 0.8 and the head
//  ("hlava") to 0.85 with setSelectionDamage; total damage stays 0.5 and the
//  unit stays alive. Legs stay below 0.9, so the unit can still stand.
//  Wound textures also need the Blood option on (ENGINE_CONFIG.blood, default
//  on); a fresh tri user profile keeps the default.
//
//  The assertions prove the spawn, the class and that body and legs reached
//  the wound threshold with the unit alive; failures are collected and
//  asserted at the end so every class gets its screenshots. The wound textures
//  themselves need visual review (healthy back vs wounded back per class).
//  With the .4 references and wounds rules applied, every class pairs all
//  eight body textures (back, front, both arms, four leg parts), so each
//  shows wounds front and back in its own suit: EAF brown and IAF grey with
//  the generated red-blood _dr textures, the olive classes with the shipped
//  olive _d art.
//  Mission Repair4.Abel: its addOns[] covers LoBo_Pilot and the owners of
//  every class's weapons[] and magazines[] (LoBoWeapons, LoBoWeapNad,
//  JAM_Magazines); see check_mission_addons.py.
// ============================================================================
triSimUntil { alive player }
setDate [1985, 6, 15, 12, 0]
0 setOvercast 0
0 setFog 0
setViewDistance 1500
showCinemaBorder false
player allowDammage false
player setCaptive true
rpView = {_o = _this select 0; _b = _this select 1; _d = _this select 2; _h = _this select 3; _t = _this select 4; _p = getPosASL _o; [(_p select 0) + _d * (sin _b), (_p select 2) + _h, (_p select 1) + _d * (cos _b), 0 - (sin _b), (_t - _h) / _d, 0 - (cos _b)]}
rpHold = {_u = _this select 0; _u setCaptive true; _u disableAI "MOVE"; _u disableAI "TARGET"; _u disableAI "AUTOTARGET"; _u stop true; _u setDir 0}
rpWound = {_u = _this select 0; _u setDammage 0.5; _u setSelectionDamage ["telo", 0.8]; _u setSelectionDamage ["ruce", 0.8]; _u setSelectionDamage ["nohy", 0.8]; _u setSelectionDamage ["hlava", 0.85]}
rpHits = {_u = _this select 0; [_u getSelectionDamage "hlava", _u getSelectionDamage "telo", _u getSelectionDamage "ruce", _u getSelectionDamage "nohy"]}
rpCheck = {_u = _this select 0; _c = _this select 1; if ((not (alive _u)) || ((_u getSelectionDamage "telo") < 0.7) || ((_u getSelectionDamage "nohy") < 0.7) || ((typeOf _u) != _c)) then {rpBad = rpBad + [_c]}}
rpBad = []
logInfo format ["LOBO_R4_WOUNDS_START lodBias=%1", triGetObjectLodBias]

// --- LoBo_Egypt_Pilot: EAF, lobo_epilot.p3d; 8 EAF\tex pairs on the brown EAF\tex\*_dr.paa (back/legsbk healthy names repointed to .paa) ---
call {rpU = objNull; "LoBo_Egypt_Pilot" createUnit [[8000, 9800, 0], group r4East, "rpU = this", 0.6, "PRIVATE"]}
triAssertEq [typeOf rpU, "LoBo_Egypt_Pilot"]
[rpU] call rpHold
triSimFrames 60
logInfo format ["LOBO_R4_WOUNDS healthy class=%1 typeOf=%2 damage=%3 alive=%4 dir=%5 hitpoints=%6", "LoBo_Egypt_Pilot", typeOf rpU, damage rpU, alive rpU, getDir rpU, GetHitpointsNames rpU]
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "egypt_pilot_healthy_back"
[rpU] call rpWound
triSimFrames 30
logInfo format ["LOBO_R4_WOUNDS wounded class=%1 typeOf=%2 damage=%3 alive=%4 hits[hlava,telo,ruce,nohy]=%5", "LoBo_Egypt_Pilot", typeOf rpU, damage rpU, alive rpU, [rpU] call rpHits]
triSetView ([rpU, getDir rpU, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "egypt_pilot_wounded_front"
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "egypt_pilot_wounded_back"
triSetView ([rpU, (getDir rpU) + 180, 1.7, 1.25, 1.0] call rpView)
triSimFrames 5
triScreenshot "egypt_pilot_wounded_back_close"
triClearView
call {[rpU, "LoBo_Egypt_Pilot"] call rpCheck; deleteVehicle rpU}
triSimFrames 10

// --- LoBo_IAF_Pilot: IAF 101st Sqn, lobo_iafpilot.p3d; olive, 8 IAF\tex pairs on the shipped _d art, unchanged ---
call {rpU = objNull; "LoBo_IAF_Pilot" createUnit [[8000, 9800, 0], group r4East, "rpU = this", 0.6, "PRIVATE"]}
triAssertEq [typeOf rpU, "LoBo_IAF_Pilot"]
[rpU] call rpHold
triSimFrames 60
logInfo format ["LOBO_R4_WOUNDS healthy class=%1 typeOf=%2 damage=%3 alive=%4 dir=%5 hitpoints=%6", "LoBo_IAF_Pilot", typeOf rpU, damage rpU, alive rpU, getDir rpU, GetHitpointsNames rpU]
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf101_pilot_healthy_back"
[rpU] call rpWound
triSimFrames 30
logInfo format ["LOBO_R4_WOUNDS wounded class=%1 typeOf=%2 damage=%3 alive=%4 hits[hlava,telo,ruce,nohy]=%5", "LoBo_IAF_Pilot", typeOf rpU, damage rpU, alive rpU, [rpU] call rpHits]
triSetView ([rpU, getDir rpU, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf101_pilot_wounded_front"
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf101_pilot_wounded_back"
triSetView ([rpU, (getDir rpU) + 180, 1.7, 1.25, 1.0] call rpView)
triSimFrames 5
triScreenshot "iaf101_pilot_wounded_back_close"
triClearView
call {[rpU, "LoBo_IAF_Pilot"] call rpCheck; deleteVehicle rpU}
triSimFrames 10

// --- LoBo_IAF_Pilot2: IAF 253rd Sqn, lobo_iafpilot2.p3d; olive, own wounds[]: 7 IAF\tex pairs plus tex5\armle on the shipped tex5\armle_d.paa ---
call {rpU = objNull; "LoBo_IAF_Pilot2" createUnit [[8000, 9800, 0], group r4East, "rpU = this", 0.6, "PRIVATE"]}
triAssertEq [typeOf rpU, "LoBo_IAF_Pilot2"]
[rpU] call rpHold
triSimFrames 60
logInfo format ["LOBO_R4_WOUNDS healthy class=%1 typeOf=%2 damage=%3 alive=%4 dir=%5 hitpoints=%6", "LoBo_IAF_Pilot2", typeOf rpU, damage rpU, alive rpU, getDir rpU, GetHitpointsNames rpU]
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf253_pilot_healthy_back"
[rpU] call rpWound
triSimFrames 30
logInfo format ["LOBO_R4_WOUNDS wounded class=%1 typeOf=%2 damage=%3 alive=%4 hits[hlava,telo,ruce,nohy]=%5", "LoBo_IAF_Pilot2", typeOf rpU, damage rpU, alive rpU, [rpU] call rpHits]
triSetView ([rpU, getDir rpU, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf253_pilot_wounded_front"
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf253_pilot_wounded_back"
triSetView ([rpU, (getDir rpU) + 180, 1.7, 1.25, 1.0] call rpView)
triSimFrames 5
triScreenshot "iaf253_pilot_wounded_back_close"
triClearView
call {[rpU, "LoBo_IAF_Pilot2"] call rpCheck; deleteVehicle rpU}
triSimFrames 10

// --- LoBo_IAF_Pilot_grey: IAF 69th Sqn, lobo_iafpilot_grey.p3d; 8 IAF\tex2 pairs on the grey IAF\tex2\*_dr.paa (legsbk healthy name repointed to .paa) ---
call {rpU = objNull; "LoBo_IAF_Pilot_grey" createUnit [[8000, 9800, 0], group r4East, "rpU = this", 0.6, "PRIVATE"]}
triAssertEq [typeOf rpU, "LoBo_IAF_Pilot_grey"]
[rpU] call rpHold
triSimFrames 60
logInfo format ["LOBO_R4_WOUNDS healthy class=%1 typeOf=%2 damage=%3 alive=%4 dir=%5 hitpoints=%6", "LoBo_IAF_Pilot_grey", typeOf rpU, damage rpU, alive rpU, getDir rpU, GetHitpointsNames rpU]
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf69_pilot_healthy_back"
[rpU] call rpWound
triSimFrames 30
logInfo format ["LOBO_R4_WOUNDS wounded class=%1 typeOf=%2 damage=%3 alive=%4 hits[hlava,telo,ruce,nohy]=%5", "LoBo_IAF_Pilot_grey", typeOf rpU, damage rpU, alive rpU, [rpU] call rpHits]
triSetView ([rpU, getDir rpU, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf69_pilot_wounded_front"
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf69_pilot_wounded_back"
triSetView ([rpU, (getDir rpU) + 180, 1.7, 1.25, 1.0] call rpView)
triSimFrames 5
triScreenshot "iaf69_pilot_wounded_back_close"
triClearView
call {[rpU, "LoBo_IAF_Pilot_grey"] call rpCheck; deleteVehicle rpU}
triSimFrames 10

// --- LoBo_IAF_Pilot_grey2: IAF 106th Sqn, lobo_iafpilot_grey2.p3d; full own wounds[]: 7 IAF\tex2 pairs on tex2\*_dr.paa, tex3\armle on tex3\armle_dr.paa ---
call {rpU = objNull; "LoBo_IAF_Pilot_grey2" createUnit [[8000, 9800, 0], group r4East, "rpU = this", 0.6, "PRIVATE"]}
triAssertEq [typeOf rpU, "LoBo_IAF_Pilot_grey2"]
[rpU] call rpHold
triSimFrames 60
logInfo format ["LOBO_R4_WOUNDS healthy class=%1 typeOf=%2 damage=%3 alive=%4 dir=%5 hitpoints=%6", "LoBo_IAF_Pilot_grey2", typeOf rpU, damage rpU, alive rpU, getDir rpU, GetHitpointsNames rpU]
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf106_pilot_healthy_back"
[rpU] call rpWound
triSimFrames 30
logInfo format ["LOBO_R4_WOUNDS wounded class=%1 typeOf=%2 damage=%3 alive=%4 hits[hlava,telo,ruce,nohy]=%5", "LoBo_IAF_Pilot_grey2", typeOf rpU, damage rpU, alive rpU, [rpU] call rpHits]
triSetView ([rpU, getDir rpU, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf106_pilot_wounded_front"
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf106_pilot_wounded_back"
triSetView ([rpU, (getDir rpU) + 180, 1.7, 1.25, 1.0] call rpView)
triSimFrames 5
triScreenshot "iaf106_pilot_wounded_back_close"
triClearView
call {[rpU, "LoBo_IAF_Pilot_grey2"] call rpCheck; deleteVehicle rpU}
triSimFrames 10

// --- LoBo_IAF_Pilot_grey3: IAF 133rd Sqn, lobo_iafpilot_grey3.p3d; full own wounds[]: 7 IAF\tex2 pairs on tex2\*_dr.paa, tex4\armle on tex4\armle_dr.paa ---
call {rpU = objNull; "LoBo_IAF_Pilot_grey3" createUnit [[8000, 9800, 0], group r4East, "rpU = this", 0.6, "PRIVATE"]}
triAssertEq [typeOf rpU, "LoBo_IAF_Pilot_grey3"]
[rpU] call rpHold
triSimFrames 60
logInfo format ["LOBO_R4_WOUNDS healthy class=%1 typeOf=%2 damage=%3 alive=%4 dir=%5 hitpoints=%6", "LoBo_IAF_Pilot_grey3", typeOf rpU, damage rpU, alive rpU, getDir rpU, GetHitpointsNames rpU]
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf133_pilot_healthy_back"
[rpU] call rpWound
triSimFrames 30
logInfo format ["LOBO_R4_WOUNDS wounded class=%1 typeOf=%2 damage=%3 alive=%4 hits[hlava,telo,ruce,nohy]=%5", "LoBo_IAF_Pilot_grey3", typeOf rpU, damage rpU, alive rpU, [rpU] call rpHits]
triSetView ([rpU, getDir rpU, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf133_pilot_wounded_front"
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "iaf133_pilot_wounded_back"
triSetView ([rpU, (getDir rpU) + 180, 1.7, 1.25, 1.0] call rpView)
triSimFrames 5
triScreenshot "iaf133_pilot_wounded_back_close"
triClearView
call {[rpU, "LoBo_IAF_Pilot_grey3"] call rpCheck; deleteVehicle rpU}
triSimFrames 10

// --- LoBo_Jordan_Pilot: RJAF, lobo_jpilot.p3d; olive, 8 RJAF\tex pairs on the shipped _d art (back/legsbk healthy names repointed to .paa) ---
call {rpU = objNull; "LoBo_Jordan_Pilot" createUnit [[8000, 9800, 0], group r4East, "rpU = this", 0.6, "PRIVATE"]}
triAssertEq [typeOf rpU, "LoBo_Jordan_Pilot"]
[rpU] call rpHold
triSimFrames 60
logInfo format ["LOBO_R4_WOUNDS healthy class=%1 typeOf=%2 damage=%3 alive=%4 dir=%5 hitpoints=%6", "LoBo_Jordan_Pilot", typeOf rpU, damage rpU, alive rpU, getDir rpU, GetHitpointsNames rpU]
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "rjaf_pilot_healthy_back"
[rpU] call rpWound
triSimFrames 30
logInfo format ["LOBO_R4_WOUNDS wounded class=%1 typeOf=%2 damage=%3 alive=%4 hits[hlava,telo,ruce,nohy]=%5", "LoBo_Jordan_Pilot", typeOf rpU, damage rpU, alive rpU, [rpU] call rpHits]
triSetView ([rpU, getDir rpU, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "rjaf_pilot_wounded_front"
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "rjaf_pilot_wounded_back"
triSetView ([rpU, (getDir rpU) + 180, 1.7, 1.25, 1.0] call rpView)
triSimFrames 5
triScreenshot "rjaf_pilot_wounded_back_close"
triClearView
call {[rpU, "LoBo_Jordan_Pilot"] call rpCheck; deleteVehicle rpU}
triSimFrames 10

// --- LoBo_Syria_Pilot: SyAF, lobo_spilot.p3d; olive, own wounds[]: 8 SyAF\tex pairs on the shipped SyAF\tex\*_d.paa (the inherited EAF pairs matched no face) ---
call {rpU = objNull; "LoBo_Syria_Pilot" createUnit [[8000, 9800, 0], group r4East, "rpU = this", 0.6, "PRIVATE"]}
triAssertEq [typeOf rpU, "LoBo_Syria_Pilot"]
[rpU] call rpHold
triSimFrames 60
logInfo format ["LOBO_R4_WOUNDS healthy class=%1 typeOf=%2 damage=%3 alive=%4 dir=%5 hitpoints=%6", "LoBo_Syria_Pilot", typeOf rpU, damage rpU, alive rpU, getDir rpU, GetHitpointsNames rpU]
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "syaf_pilot_healthy_back"
[rpU] call rpWound
triSimFrames 30
logInfo format ["LOBO_R4_WOUNDS wounded class=%1 typeOf=%2 damage=%3 alive=%4 hits[hlava,telo,ruce,nohy]=%5", "LoBo_Syria_Pilot", typeOf rpU, damage rpU, alive rpU, [rpU] call rpHits]
triSetView ([rpU, getDir rpU, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "syaf_pilot_wounded_front"
triSetView ([rpU, (getDir rpU) + 180, 3.2, 1.2, 0.95] call rpView)
triSimFrames 5
triScreenshot "syaf_pilot_wounded_back"
triSetView ([rpU, (getDir rpU) + 180, 1.7, 1.25, 1.0] call rpView)
triSimFrames 5
triScreenshot "syaf_pilot_wounded_back_close"
triClearView
call {[rpU, "LoBo_Syria_Pilot"] call rpCheck; deleteVehicle rpU}
triSimFrames 10

logInfo format ["LOBO_R4_WOUNDS_DONE failed=%1", rpBad]
triAssertEq [count rpBad, 0]
triSimFrames 30
triEndTest
