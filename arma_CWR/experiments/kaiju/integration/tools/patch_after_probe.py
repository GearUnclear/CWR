r"""Seventh round, from what the probe run measured (kaiju-notes\integration\runs\probe-proxy-1112-003529):
  - the start fix is OFF by default: putting the body forward with setPosASL while the walk blends in
    made the standing foot's drag WORSE (no fix 1.33 m; by the foot 0.82 m with 2.58 m put back and the
    body's own advance cut from 3.59 to 1.58 m; by the blend 1.91 m). A setPos on a walking Man costs
    it most of the step it was taking. The code stays behind KJB_START_FIX for the record.
  - a man thrown with the full impulse (10 m/s) is dead when he lands: a throw that is not meant to
    kill is gentler.
Run:  python tools\patch_after_probe.py"""
from patchlib import File

f = File(r"brain\init.sqf")
f.rep('''if (isNil "KJB_START_FIX") then { KJB_START_FIX = true };    // [integration] hold the standing foot where it is while the walk blends in (KJB_fnStartFix)''',
      '''if (isNil "KJB_START_FIX") then { KJB_START_FIX = false };   // [integration] OFF: an attempt to hold the standing foot while the walk blends in (KJB_fnStartFix). Measured: it makes the drag worse (a setPos on a walking Man costs it the step it was taking). Kept for the record; the cure is the engine's (integration NOTES, engine request 2)''')
f.save()

f = File(r"brain\start.sqs")
f.rep('''? KJB_START_FIX : KJB_strideLoss = 0; KJC_START_LAG = 0''',
      '''? KJB_START_FIX : KJB_strideLoss = 0''')
f.save()

f = File(r"crush\init.sqf")
f.rep('''            _v = 4 + 6 * (_this select 2);
            [_m, [_v * _dx / _d, _v * _dy / _d, 2.5 + 2.5 * (_this select 2)]] call KJ_fnThrow''',
      '''            // measured: 10 m/s out and 5 up kills a man when he lands. A throw that is not
            // meant to kill is a shove off his feet.
            _v = 2 + 2.5 * (_this select 2);
            if (_this select 3) then { _v = 6 + 6 * (_this select 2) };
            [_m, [_v * _dx / _d, _v * _dy / _d, 0.4 * _v]] call KJ_fnThrow''')
f.save()
