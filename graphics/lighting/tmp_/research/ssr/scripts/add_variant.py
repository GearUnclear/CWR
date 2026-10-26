"""Insert an extra shot variant into a research capture test, before a given label's shot block.

    python add_variant.py <test.sqf> <view> <before_mode> <new_mode> "<knob>,<knob>,..." [reset_knob]
Example: add_variant.py ssr_abel3.test.sqf coastline_sunset ssrtags ssrw06 "ssr=1,ssrDebug=0,ssrWater=0.6" ssrWater=1
"""
import sys, re
path, view, before, mode, knobs = sys.argv[1:6]
reset = sys.argv[6] if len(sys.argv) > 6 else None
src = open(path, encoding="utf-8", newline="").read()
nl = "\r\n" if "\r\n" in src else "\n"
if f'"{view}__{mode}"' in src:
    print("already present"); sys.exit(0)
# the capModeX line that starts the 'before' block of this view
shot = f'triScreenshot "{view}__{before}"'
i = src.index(shot)
j = src.rindex(f'["{before}",', 0, i)
ks = ", ".join(f'"{k}"' for k in knobs.split(","))
frames = 6 if "ssrDebug=1" in knobs or "ssrDebug=3" in knobs else 90
block = nl.join([
    f'["{mode}", [{ks}]] call capModeX',
    f'triSimFrames {frames}',
    '[] call capReadTimers',
    '[] call capPin',
    'triSimFrames 3',
    f'triScreenshot "{view}__{mode}"',
    f'"{view}__{mode}" call capRecord',
]) + nl
if reset:
    block += f'call {{ capKnob = triSetLighting "{reset}" }}' + nl
src = src[:j] + block + src[j:]
open(path, "w", encoding="utf-8", newline="").write(src)
print("inserted", view, mode)
