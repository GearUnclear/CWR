r"""Third round of integration changes: he has to be killable (owner, 2026-11-12). The crush
feature's heal loop sits behind ONE switch (KJ_unkillable), everything stops cleanly when he is
dead (KJ_dead, kj\start.sqs watches), and the brain and the beam read KJ_woundStage through
KJ_fnWound (kj\init.sqf). The combat feature (kj\combat\start.sqs) takes over the hit signal
when it is in the mission.   Run:  python tools\patch_hooks.py"""
from patchlib import File

# ---- crush: the heal loop and the hit signal run only while he is unkillable -----------------------
f = File(r"crush\loop.sqs")
f.rep('''[_k] exec "kj\\crush\\hurt.sqs"
''', '''; [integration] ONE switch: KJ_unkillable. While it is true the crush feature's hurt.sqs heals
; him on every read and undoes a killing blow (the experiment's first rule). When it is false
; (the combat feature is in the mission, or a mission says so) hurt.sqs does not run at all: the
; combat feature owns his health and raises KJ_hurt itself.
? KJ_unkillable : [_k] exec "kj\\crush\\hurt.sqs"
''')
f.rep('''; dead: hurt.sqs brings him back if it can; this loop only waits
#wait
~1
goto "tick"''', '''; dead: hurt.sqs brings him back if it can; this loop only waits. [integration] When he can die, it ends.
#wait
? (not KJ_unkillable) or KJ_dead : goto "end"
~1
goto "tick"''')
f.save()

f = File(r"crush\hurt.sqs")
f.rep('''#tick
? not KJ_crushOn : goto "end"
? isNull _k : goto "end"
? alive _k : goto "live"''', '''#tick
? not KJ_crushOn : goto "end"
? isNull _k : goto "end"
? not KJ_unkillable : goto "end"
? alive _k : goto "live"''')
f.save()

f = File(r"crush\steps.sqs")
f.rep('''? not (alive _k) : goto "tick"
''', '''? KJ_dead : goto "end"
? not (alive _k) : goto "tick"
''')
f.save()

# ---- brain: dead is dead; the wounds make him slower to start and quicker to flinch ---------------
f = File(r"brain\init.sqf")
f.rep('''    if (!(alive KJB_k)) then {
        if (KJ_state != "dead") then { "dead" call KJB_fnEnter }
    } else {''', '''    if ((!(alive KJB_k)) or KJ_dead) then {
        // [integration] dead (the engine's, or the combat feature's KJ_dead): no walking corpse.
        // While he is unkillable a killing blow is undone by the crush feature within a fifth of a
        // second, so the brain waits half a second before it believes it.
        if (alive KJB_k) then { KJB_deadSince = -1 };
        if ((!(alive KJB_k)) and (KJB_deadSince < 0)) then { KJB_deadSince = _now };
        if (KJ_dead or (!KJ_unkillable) or (_now > (KJB_deadSince + 0.5))) then {
            if (KJ_state != "dead") then { "dead" call KJB_fnEnter }
        }
    } else {
        KJB_deadSince = -1;''')
f.after('''KJB_dormantSecs = 16;
''', '''KJB_deadSince = -1;
''')
f.rep('''        KJB_idleUntil = time + KJB_IDLE_MIN + random KJB_IDLE_RAND;''',
      '''        // [integration] wounded, the pause between two walks grows (half as long again per wound stage)
        KJB_idleUntil = time + (KJB_IDLE_MIN + random KJB_IDLE_RAND) * (1 + 0.5 * (call KJ_fnWound));''')
f.rep('''    if ((KJ_brainAuto or KJB_HIT_REACT) and (_sev >= KJB_HIT_HARD) and ((time - KJB_lastHit) >= KJB_HIT_GAP) and (KJ_state in ["idle", "wander", "drawn", "dormant"])) then {''',
      '''    // [integration] wounded, he flinches sooner again (the gap shrinks with the wound stage)
    if ((KJ_brainAuto or KJB_HIT_REACT) and (_sev >= KJB_HIT_HARD) and ((time - KJB_lastHit) >= (KJB_HIT_GAP / (1 + (call KJ_fnWound)))) and (KJ_state in ["idle", "wander", "drawn", "dormant"])) then {''')
f.save()

# ---- beam: wounded, the charge takes longer; dead, it stops ------------------------------------------
f = File(r"beam\fire.sqs")
f.rep('''; A caller may change any of the above for one run: KJ_beamOverride = { KJ_beamT_still = 1 }.
? not (isNil "KJ_beamOverride") : call KJ_beamOverride''', '''; A caller may change any of the above for one run: KJ_beamOverride = { KJ_beamT_still = 1 }.
? not (isNil "KJ_beamOverride") : call KJ_beamOverride
; [integration] wounded, the furnace is slower to come up (a quarter longer per wound stage)
? not (isNil "KJ_fnWound") : KJ_beamT_charge = KJ_beamT_charge * (1 + 0.25 * (call KJ_fnWound))''')
f.save()

# ---- roar: no drone, no lamps, no footfalls from a dead body ------------------------------------------
f = File(r"roar\presence.sqs")
f.rep('''#tick
? not KJ_presenceOn : goto "end"
? not alive _k : goto "end"''', '''#tick
? not KJ_presenceOn : goto "end"
? not alive _k : goto "end"
? KJ_dead : goto "end"''')
f.save()
