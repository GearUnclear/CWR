"""Probe: the dusk firefight in several lighting modes (look director round 2).

python gen_probe_ff.py OUT_DIR "label=setting;label=setting"
Writes look_probe_ff.test.sqf/.toml into OUT_DIR (the captures folder).
"""
import sys
from pathlib import Path

out = Path(sys.argv[1])
modes = [m.split("=", 1) for m in sys.argv[2].split(";") if m.strip()]
src = (out / "abel_lighting.test.sqf").read_text(encoding="utf-8").splitlines()
head = src[:58]
# the firefight block: from its capBegin to the classic shots' end
start = next(i for i, l in enumerate(src) if l.startswith('["firefight"'))
eng = next(i for i, l in enumerate(src) if l.startswith('[] call capFFStage'))
body = src[start:eng]
lines = head + body
for label, setting in modes:
    lines += [
        '[] call capFFStage',
        f'"{setting}" call capMode',
        '[] call capEngage',
        'triSimFrames 200',
        '[] call capReadTimers',
        'call { [] call capPinEnv; [] call capPinView; [] call capEngage; [] call capBurn }',
        '[62, -2] call capBoom',
        'triSimFrames 5',
        f'triScreenshot "firefight_1__{label}"',
        f'"firefight_1__{label}" call capRecord',
        '[] call capEngage',
        '[58, -32] call capBoom',
        'triSimFrames 6',
        f'triScreenshot "firefight_2__{label}"',
        f'"firefight_2__{label}" call capRecord',
        '[] call capEngage',
        'triSimFrames 20',
        f'triScreenshot "firefight_3__{label}"',
        f'"firefight_3__{label}" call capRecord',
        '[] call capFFClear',
        'triSimFrames 30',
    ]
lines += ['call { "view=done;event=done" call capLog }', 'triEndTest']
(out / "look_probe_ff.test.sqf").write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
toml = (out / "abel_lighting.test.toml").read_text(encoding="utf-8")
(out / "look_probe_ff.test.toml").write_text(toml, encoding="utf-8", newline="\n")
print("ok", len(modes))
