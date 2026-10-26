import sys, subprocess, numpy as np
FF = "C:/ffmpeg/bin/ffmpeg.exe"
for c in sys.argv[1:]:
    p = subprocess.run([FF, '-hide_banner', '-loglevel', 'error', '-i', c, '-vf', 'scale=480:270:flags=area,format=gray', '-f', 'rawvideo', '-'], capture_output=True, check=True)
    v = np.frombuffer(p.stdout, np.uint8).reshape(-1, 270, 480).astype(np.float32)[68:120]
    for name, sl in (('sky', np.s_[10:90, 20:460]), ('land', np.s_[110:200, 20:460]), ('ground', np.s_[220:265, 60:240])):
        m = v[(slice(None),) + sl].mean(axis=(1, 2))
        d2 = m[1:-1] - 0.5 * (m[:-2] + m[2:])
        print(c.split('/')[-1], name, 'd2 rms %.3f' % np.sqrt((d2 ** 2).mean()), ' '.join('%+.2f' % x for x in d2[:20]))
