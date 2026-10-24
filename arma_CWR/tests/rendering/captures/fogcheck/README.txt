Fog flicker and motion checks (from the 2026-10-23 dawn-fog work, ITERATIONS.md F1 and F2)
===========================================================================================

Run with plain "python" (numpy, scipy and Pillow live in the user site; "python -I" hides them).
ffmpeg is expected at C:/ffmpeg/bin/ffmpeg.exe.

bandmetric.py   flicker: the temporal second difference of 15 px block means over a band of the frame
                (default: the valley band of the trailer's dawn shot); blind to patterns that travel.
lowfreq.py      a heat map of where that flicker sits, over a mid frame.
fogmotion.py    flicker, travel (the "wind": a pattern that moves when the camera does not explain it),
                boil and grain per region, camera-compensated; --static for a held camera. Its REGIONS
                are fractions of the dawn shot; add your own for other shots.
xtslice.py      x-t and y-t slices of a band: the camera draws lines of one slope; horizontal stripes are
                pumping, other slants are travel.
dstrip.py, curtain.py, windchar.py, tempspec.py, pulse.py, spectrum3d.py, kxnu.py
                difference strips, screen-locked streaks, wind direction and speed, temporal spectra.
gen_probe_fog1.py, gen_rt_probe.py, run_probe_fog.ps1, run_rt.ps1, measure_rt.sh
                the probes used (static and moving cuts of the dawn shot, and real-time 60 fps recordings
                with a diagnostic recorder build); they expect the 2026-10-23 scratch layout.

Lesson (owner, 2026-10-23): a fog fix passed bandmetric and still showed "wind". Judge temporal fixes in
motion: no flicker, no travel, no grain, the same look.
