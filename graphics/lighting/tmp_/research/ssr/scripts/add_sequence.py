"""Insert a short motion sequence (sim clock stepped between shots) before a given shot block.

    python add_sequence.py <test.sqf> <view> <before_mode> <prefix> "<knob>,..." <n> <step_s>
"""
import sys
path, view, before, prefix, knobs, n, step = sys.argv[1:8]
n = int(n); step = float(step)
src = open(path, encoding="utf-8", newline="").read()
nl = "\r\n" if "\r\n" in src else "\n"
if f'"{view}__{prefix}0"' in src:
    print("already present"); sys.exit(0)
shot = f'triScreenshot "{view}__{before}"'
i = src.index(shot)
j = src.rindex(f'["{before}",', 0, i)
ks = ", ".join(f'"{k}"' for k in knobs.split(","))
lines = [f'["{prefix}", [{ks}]] call capModeX', 'triSimFrames 90', '[] call capReadTimers',
         'call { capSeqT0 = capSimT }']
for k in range(n):
    lines += [f'call {{ capSimT = capSeqT0 + {k * step:.3f} }}', '[] call capPin', 'triSimFrames 3',
              f'triScreenshot "{view}__{prefix}{k}"', f'"{view}__{prefix}{k}" call capRecord']
lines += ['call { capSimT = capSeqT0 }']
block = nl.join(lines) + nl
src = src[:j] + block + src[j:]
open(path, "w", encoding="utf-8", newline="").write(src)
print("inserted sequence", view, prefix, n)
