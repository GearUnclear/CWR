"""Round 9 timing for the haze track: many short GPU readings per mode, so the
minimum catches moments when the other games sharing the GPU are quiet.

Labels <view>__t<round*100+k>_<mode>; 3 rounds x 5 modes x 16 readings, 4 frames
apart, after 40 settle frames. Reuses the header of haze_timing_r6.test.sqf.

python gen_timing_r9.py <test_dir>
"""
import sys

T = sys.argv[1]
s = open(T + "/haze_timing_r6.test.sqf", encoding="utf-8").read()
head = s.split("\n// ---- sunrise_ridge (timing)", 1)[0]
views = [("sunrise_ridge", '["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin'),
         ("valley_morning", '["valley_morning", 6100, 5000, 1.75, 50, -6, [1985, 7, 5, 6, 30], 0, 0, 4000] call capBegin')]
body = ""
for v, b in views:
    body += f"\n// ---- {v} (timing r9)\n{b}\ntriSimFrames 150\n"
    for r in range(3):
        for m in ("classic", 0, 31, 32, 33):
            st = '"classic" call capMode' if m == "classic" else f"{m} call capVar"
            lab = "classic" if m == "classic" else f"v{m}"
            body += f"{st}\ntriSimFrames 40\n"
            for k in range(16):
                body += f'triSimFrames 4\n[] call capReadTimers\n"{v}__t{r * 100 + k}_{lab}" call capRecord\n'
body += '\ncall { capSat = triSetLighting "saturation=1.12" }\ncall { "view=end;event=done" call capLog }\ntriEndTest\n'
open(T + "/haze_timing_r9.test.sqf", "w", newline="\n").write(head + body)
open(T + "/haze_timing_r9.test.toml", "w", newline="\n").write(open(T + "/haze_timing_r6.test.toml").read())
print("wrote haze_timing_r9")
