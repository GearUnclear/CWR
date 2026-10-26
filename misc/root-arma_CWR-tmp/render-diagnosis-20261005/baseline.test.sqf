triSimFrames 30
triCaptureAtPresent true
triScreenshot "world_after_30_frames"
rdT = time
triSimUntil { time > rdT + 3 }
triScreenshot "world_after_3_seconds"
triEndTest
