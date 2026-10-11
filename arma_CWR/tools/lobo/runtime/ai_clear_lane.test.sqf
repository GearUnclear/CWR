// Terrain-selected native AI observations. Run with run-local.ps1 -Retries 0.
// A zero-shot window remains open. Terrain samples do not detect object occlusion.
triSimUntil {alive player}
player allowDammage false
player setCaptive true
setDate [1985,6,15,12,0]
setViewDistance 2000
0 setOvercast 0
0 setFog 0
0 setRain 0
createCenter west
createCenter east
lcWG=createGroup west
lcEG=createGroup east
lcHaveLane=false
lcLaneStart=[0,0,0]
lcLaneHeading=0
lcLaneHeights=[]
lcLaneMin300=-999
lcLaneMin500=-999
lcShooter=objNull
lcTarget=objNull
lcWindowEnd=time
lcShots=0
lcCamera="camera" camCreate getPos player
triAssert [not isNull lcCamera]
lcCamera cameraEffect ["internal","back"]
// Prefer airfield candidates, then try the original area. Sample native terrain
// every25m using camCreate's SurfaceYAboveWater conversion, then destroy the probe.
// Also require the300m ray, since a clear500m ray need not clear a lower300m endpoint.
lcCandidates=[[8000,9500],[8200,9800],[7800,10000],[7415,5860],[7480,5700]]
{
    lcCandidate=_x;
    {
        if (not lcHaveLane) then {
            lcHeading=_x;
            lcHeights=[];
            lcDistance=0;
            while {lcDistance<=500} do {
                lcProbe="camera" camCreate [(lcCandidate select 0)+(sin lcHeading)*lcDistance,(lcCandidate select 1)+(cos lcHeading)*lcDistance,0];
                lcHeights=lcHeights+[(getPosASL lcProbe) select 2];
                camDestroy lcProbe;
                lcDistance=lcDistance+25;
            };
            lcH0=lcHeights select 0;
            lcH300=lcHeights select 12;
            lcH500=lcHeights select 20;
            lcMin300=999;
            lcMin500=999;
            lcIndex=1;
            while {lcIndex<20} do {
                lcSampleDistance=lcIndex*25;
                lcRay500=lcH0+1.7+(lcH500-lcH0)*(lcSampleDistance/500);
                lcClear500=lcRay500-(lcHeights select lcIndex);
                if (lcClear500<lcMin500) then {lcMin500=lcClear500};
                if (lcIndex<12) then {
                    lcRay300=lcH0+1.7+(lcH300-lcH0)*(lcSampleDistance/300);
                    lcClear300=lcRay300-(lcHeights select lcIndex);
                    if (lcClear300<lcMin300) then {lcMin300=lcClear300};
                };
                lcIndex=lcIndex+1;
            };
            lcEligible=(lcH0>2) and (lcH300>2) and (lcH500>2) and (lcMin300>=0.2) and (lcMin500>=0.2);
            logInfo format ["LOBO_AI_LANE_CANDIDATE start=%1 heading=%2 groundStart=%3 ground300=%4 ground500=%5 minClearance300=%6 minClearance500=%7 eligible=%8",lcCandidate,lcHeading,lcH0,lcH300,lcH500,lcMin300,lcMin500,lcEligible];
            if (lcEligible) then {
                lcHaveLane=true;
                lcLaneStart=[lcCandidate select 0,lcCandidate select 1,0];
                lcLaneHeading=lcHeading;
                lcLaneHeights=lcHeights;
                lcLaneMin300=lcMin300;
                lcLaneMin500=lcMin500;
            };
        };
    } forEach [0,45,90,135,180,225,270,315];
} forEach lcCandidates
lcLaneEnd300=[(lcLaneStart select 0)+(sin lcLaneHeading)*300,(lcLaneStart select 1)+(cos lcLaneHeading)*300,0]
lcLaneEnd500=[(lcLaneStart select 0)+(sin lcLaneHeading)*500,(lcLaneStart select 1)+(cos lcLaneHeading)*500,0]
logInfo format ["LOBO_AI_LANE_SELECTED found=%1 start=%2 end300=%3 end500=%4 heading=%5 minClearance300=%6 minClearance500=%7 samplesEvery25m=%8 approximateEyeHeight=1.7 objectOcclusionUnverified=true",lcHaveLane,lcLaneStart,lcLaneEnd300,lcLaneEnd500,lcLaneHeading,lcLaneMin300,lcLaneMin500,lcLaneHeights]
if (not lcHaveLane) then {logInfo "LOBO_AI_CLEAR_LANE_BLOCKED no_eligible_terrain_lane=true all_firing_windows_skipped=true"}
// Keep the local player near the selected lane so simulation distance is representative.
if (lcHaveLane) then {player setPos [(lcLaneStart select 0)-80,(lcLaneStart select 1)-80,0]}
// The helpers below only create real infantry and issue native targeting orders.
// No simulation disabling, forced projectile creation or synthetic hit is used.
lcSetupCase={
    if (lcHaveLane) then {
        lcTargetPos=[(lcLaneStart select 0)+(sin lcLaneHeading)*lcRange,(lcLaneStart select 1)+(cos lcLaneHeading)*lcRange,0];
        "SoldierWB" createUnit [lcLaneStart,lcWG,"lcShooter=this",1,"PRIVATE"];
        "SoldierEB" createUnit [lcTargetPos,lcEG,"lcTarget=this",1,"PRIVATE"];
        removeAllWeapons lcShooter;
        removeAllWeapons lcTarget;
        lcShooter setPos lcLaneStart;
        lcTarget setPos lcTargetPos;
        lcTarget allowDammage false;
        lcTarget disableAI "MOVE";
        lcTarget setUnitPos "UP";
        lcTarget setBehaviour "CARELESS";
        lcShooter disableAI "MOVE";
        lcShooter setDir lcLaneHeading;
        lcShooter setUnitPos "UP";
        lcShooter setBehaviour "COMBAT";
        lcShooter setCombatMode "BLUE";
    };
}
lcArmCase={
    if (lcHaveLane) then {
        lcShooter addMagazine lcMagazine;
        lcShooter addWeapon lcWeapon;
        lcShots=0;
        lcShooter addEventHandler ["Fired",{lcShots=lcShots+1; logInfo format ["LOBO_AI_CLEAR_SHOT case=%1 weapon=%2 ammo=%3 shot=%4 time=%5",lcCase,_this select 1,_this select 4,lcShots,time]}];
    };
}
lcStartCase={
    if (lcHaveLane) then {
        lcShooter setCombatMode "RED";
        lcShooter reveal lcTarget;
        lcShooter doTarget lcTarget;
        lcShooter doFire lcTarget;
        lcWindowStart=time;
        lcWindowEnd=lcWindowStart+30;
        lcS=getPosASL lcShooter;
        lcT=getPosASL lcTarget;
        triSetView [lcS select 0,(lcS select 2)+1.7,lcS select 1,(lcT select 0)-(lcS select 0),(lcT select 2)-(lcS select 2),(lcT select 1)-(lcS select 1)];
        logInfo format ["LOBO_AI_CLEAR_START case=%1 distance=%2 shooterASL=%3 targetASL=%4 shooterATL=%5 targetATL=%6 knowledge=%7 viewDistance=2000 windowSeconds=30 objectOcclusionUnverified=true",lcCase,lcShooter distance lcTarget,lcS,lcT,getPos lcShooter,getPos lcTarget,lcShooter knowsAbout lcTarget];
    };
}
lcFinishCase={
    if (lcHaveLane) then {
        logInfo format ["LOBO_AI_CLEAR_RESULT case=%1 fired=%2 remaining=%3 elapsed=%4 distance=%5 shooterASL=%6 targetASL=%7 knowledge=%8 zeroShotsRemainOpen=%9",lcCase,lcShots,lcShooter ammo lcWeapon,time-lcWindowStart,lcShooter distance lcTarget,getPosASL lcShooter,getPosASL lcTarget,lcShooter knowsAbout lcTarget,lcShots==0];
        deleteVehicle lcShooter;
        deleteVehicle lcTarget;
        lcShooter=objNull;
        lcTarget=objNull;
    };
}

// One native30-second window, no shot-count assertion and no automatic rerun.
lcCase="LoBoM240_300m"
lcWeapon="LoBoM240"
lcMagazine="LoBoM240Mag"
lcRange=300
call lcSetupCase
triAssert [(not lcHaveLane) or (not isNull lcShooter)]
triAssert [(not lcHaveLane) or (not isNull lcTarget)]
triSimFrames 30
call lcArmCase
triAssert [(not lcHaveLane) or ((lcShooter ammo lcWeapon)>0)]
call lcStartCase
triScreenshot "clear_lane_1_LoBoM240_300m"
triSimUntil {(not lcHaveLane) or (time>=lcWindowEnd)}
call lcFinishCase
triSimFrames 5

// One native30-second window, no shot-count assertion and no automatic rerun.
lcCase="LoBoM240_500m"
lcWeapon="LoBoM240"
lcMagazine="LoBoM240Mag"
lcRange=500
call lcSetupCase
triAssert [(not lcHaveLane) or (not isNull lcShooter)]
triAssert [(not lcHaveLane) or (not isNull lcTarget)]
triSimFrames 30
call lcArmCase
triAssert [(not lcHaveLane) or ((lcShooter ammo lcWeapon)>0)]
call lcStartCase
triScreenshot "clear_lane_2_LoBoM240_500m"
triSimUntil {(not lcHaveLane) or (time>=lcWindowEnd)}
call lcFinishCase
triSimFrames 5

// One native30-second window, no shot-count assertion and no automatic rerun.
lcCase="LoBoM249_300m"
lcWeapon="LoBoM249"
lcMagazine="LoBoM249Mag"
lcRange=300
call lcSetupCase
triAssert [(not lcHaveLane) or (not isNull lcShooter)]
triAssert [(not lcHaveLane) or (not isNull lcTarget)]
triSimFrames 30
call lcArmCase
triAssert [(not lcHaveLane) or ((lcShooter ammo lcWeapon)>0)]
call lcStartCase
triScreenshot "clear_lane_3_LoBoM249_300m"
triSimUntil {(not lcHaveLane) or (time>=lcWindowEnd)}
call lcFinishCase
triSimFrames 5

// One native30-second window, no shot-count assertion and no automatic rerun.
lcCase="LoBoNegev_300m"
lcWeapon="LoBoNegev"
lcMagazine="LoBoNegevMag"
lcRange=300
call lcSetupCase
triAssert [(not lcHaveLane) or (not isNull lcShooter)]
triAssert [(not lcHaveLane) or (not isNull lcTarget)]
triSimFrames 30
call lcArmCase
triAssert [(not lcHaveLane) or ((lcShooter ammo lcWeapon)>0)]
call lcStartCase
triScreenshot "clear_lane_4_LoBoNegev_300m"
triSimUntil {(not lcHaveLane) or (time>=lcWindowEnd)}
call lcFinishCase
triSimFrames 5

// One native30-second window, no shot-count assertion and no automatic rerun.
lcCase="LoBoNegev_Commando_SD_300m"
lcWeapon="LoBoNegev_Commando_SD"
lcMagazine="LoBoNegevSDMag"
lcRange=300
call lcSetupCase
triAssert [(not lcHaveLane) or (not isNull lcShooter)]
triAssert [(not lcHaveLane) or (not isNull lcTarget)]
triSimFrames 30
call lcArmCase
triAssert [(not lcHaveLane) or ((lcShooter ammo lcWeapon)>0)]
call lcStartCase
triScreenshot "clear_lane_5_LoBoNegev_Commando_SD_300m"
triSimUntil {(not lcHaveLane) or (time>=lcWindowEnd)}
call lcFinishCase
triSimFrames 5

camDestroy lcCamera
logInfo format ["LOBO_AI_CLEAR_COMPLETE laneFound=%1 windowsRequested=5 terrainOnly=true noBlanketRangeAcceptance=true",lcHaveLane]
triEndTest
