"""Stacks the per-clip contact sheets (encode_promo.py) whose names start with a prefix into one image.
    python review.py <prefix> [<prefix> ...]   -> <UD_PROMO_DIR>/work/sheets/review_<first prefix>.jpg
"""
import glob
import os
import sys

from PIL import Image, ImageDraw

from promopaths import WORK

D = os.path.join(WORK, 'sheets')
fs = sorted(f for p in sys.argv[1:] for f in glob.glob(os.path.join(D, p + '*.jpg')) if not os.path.basename(f).startswith(('all_', 'review_')))
ims = [Image.open(f) for f in fs]
W = max(i.width for i in ims)
s = Image.new('RGB', (W, sum(i.height + 18 for i in ims)))
y = 0
for f, i in zip(fs, ims):
    ImageDraw.Draw(s).text((4, y + 3), os.path.basename(f), fill=(255, 255, 0))
    y += 18
    s.paste(i, (0, y))
    y += i.height
out = os.path.join(D, 'review_%s.jpg' % sys.argv[1])
s.save(out, quality=80)
print(out, len(fs))
