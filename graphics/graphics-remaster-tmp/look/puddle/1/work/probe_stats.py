"""Probe evidence: before (round-3 base, shots_probe0) | after (final build,
shots_probe_final). Sheet of the reflection-only views and numbers:
- refl view (debug=10): mean luminance of the ground band, and the share of
  ground pixels with any reflection (luma > 0.03), which outside the puddles
  should drop to about zero.
- streak energy: the forward pass's lamp/fire glints, ovh minus no-lamp (ssr
  off in both), mean over the ground band."""
from PIL import Image
import numpy as np

B = 'shots_probe0/tri/look_probe_pd1_abel/'
A = 'shots_probe_final/tri/look_probe_pd1_abel/'


def lum(p, box):
    im = np.asarray(Image.open(p).convert('RGB').crop(box), dtype=np.float32) / 255.0
    return im @ np.array([0.2126, 0.7152, 0.0722], dtype=np.float32)


band = (0, 640, 1920, 1080)  # the ground in front of the eye
for view, i0 in (('town_night_rain', 0), ('seam_day', 4)):
    for tag, d in (('before', B), ('after', A)):
        r = lum(d + '%03d_%s__refl.png' % (i0 + 1, view), band)
        ns = lum(d + '%03d_%s__nossr.png' % (i0 + 2, view), band)
        nl = lum(d + '%03d_%s__nossrlamp.png' % (i0 + 3, view), band)
        ov = lum(d + '%03d_%s__ovh.png' % (i0, view), band)
        print('%-16s %-6s refl mean %.4f  refl>0.03 %.1f%%  lamp glint (nossr - nossrlamp) mean %.4f max %.3f  ovh mean %.4f  ssr add (ovh - nossr) mean %.4f'
              % (view, tag, r.mean(), 100.0 * (r > 0.03).mean(), (ns - nl).mean(), (ns - nl).max(), ov.mean(), (ov - ns).mean()))

s = Image.new('RGB', (1920, 1080))
for k, (d, n) in enumerate(((B, '001_town_night_rain__refl'), (A, '001_town_night_rain__refl'),
                            (B, '005_seam_day__refl'), (A, '005_seam_day__refl'))):
    s.paste(Image.open(d + n + '.png').convert('RGB').resize((960, 540)), ((k % 2) * 960, (k // 2) * 540))
s.save('work/probe_refl_before_after.png')
