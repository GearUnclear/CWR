triAssertEq [triDisplay,0]
triBarrier hosted_setup
// Direct harness join is restricted to the local host's exact ephemeral port.
triMpJoin 127.0.0.1 ${ports.game}
triAssertEq [triDisplay,70]
triAssertEq [triMpAssignSelfSlot "WEST:1","OK"]
triBarrier hosted_slots_taken
triAssertNgsClient 12
triAssertNgsClient 14
triInvokeButton 1
if (triDisplay == 204) then {triInvokeButton 1}
triWait 1500
triAssertMissionPlayable
triBarrier hosted_playable
// Diagnostic only. No damage, glow-count or natural-cleanup acceptance assertions.
triAssertEq [triPlayerAuthority,"OK"]
triAssertEq [player==lwPlayer1,true]
triAssertEq [local player,true]
ldRole="client1"
ldBase=getPos lwWreck
ldStart=time
ldObserve={
    ldNearby=[];
    ldGlowCount=0;
    {
        ldType=typeOf _x;
        if (ldType=="M1Smoker") then {ldGlowCount=ldGlowCount+1};
        ldNearby=ldNearby+[[ldType,getPos _x,getPosASL _x]];
    } forEach (nearestObjects [getPos lwWreck,[],30]);
    logInfo format ["LOBO_WRECK_DIAGNOSTIC role=%1 sample=%2 time=%3 elapsed=%4 alive=%5 damage=%6 local=%7 wreckPos=%8 glowCount=%9 diagnosticOnly=true",ldRole,ldSample,time,time-ldStart,alive lwWreck,getDammage lwWreck,local lwWreck,getPos lwWreck,ldGlowCount];
    logInfo format ["LOBO_WRECK_LOCALITY role=%1 sample=%2 player1Local=%3 player2Local=%4 aiLocal=%5 wreckASL=%6",ldRole,ldSample,local lwPlayer1,local lwPlayer2,local lwAI,getPosASL lwWreck];
    logInfo format ["LOBO_WRECK_NEARBY role=%1 sample=%2 entries_type_position_ASL=%3",ldRole,ldSample,ldNearby];
}
ldSample="before"
call ldObserve
triBarrier diagnose_start
// The host owns the only destruction command.
triWait 5000
ldSample="after5s"
call ldObserve
triWait 10000
ldSample="after15s"
call ldObserve
triBarrier diagnose_done
logInfo "LOBO_WRECK_DIAGNOSTIC_COMPLETE role=client1 no_fire_ownership_or_cleanup_acceptance=true"
triEndTest
