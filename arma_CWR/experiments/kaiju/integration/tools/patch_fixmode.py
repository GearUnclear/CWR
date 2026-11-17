r"""Fourth round: the start fix can be forced onto the blend arithmetic (the stock binary's way) on
the kaiju binary too, so one run can measure both.   Run:  python tools\patch_fixmode.py"""
from patchlib import File

f = File(r"brain\init.sqf")
f.after('''if (isNil "KJB_START_FIX") then { KJB_START_FIX = true };    // [integration] hold the standing foot where it is while the walk blends in (KJB_fnStartFix)
''', '''if (isNil "KJB_START_FIX_MODE") then { KJB_START_FIX_MODE = "auto" }; // [integration] "auto": by the foot on the kaiju binary, by the blend arithmetic on the stock one; "blend": the arithmetic on both
''')
f.rep('''    if (KJ_native) then { if (KJ_hasFootPoints) then { KJB_fixFoot = kjPoint [KJB_k, KJB_plantPt] } };''',
      '''    if (KJ_native and (KJB_START_FIX_MODE == "auto")) then { if (KJ_hasFootPoints) then { KJB_fixFoot = kjPoint [KJB_k, KJB_plantPt] } };''')
f.save()
