"""Rebuild the owner's pixel-art poster on its native grid (2x2 source pixels per art pixel)
and cut the title lettering out as a clean mask. Read-only on the source PNG."""
from PIL import Image
import numpy as np, os

SRC = r'D:/Arma_CWA/ChatGPT Image Oct 5, 2026, 09_49_52 PM.png'
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'assets')
os.makedirs(OUT, exist_ok=True)

RED_BORDER = (175, 78, 46)
RED_TITLE = (171, 68, 51)
CREAM = (234, 214, 179)
ORANGE = (221, 139, 57)

def native():
    im = np.asarray(Image.open(SRC).convert('RGB')).astype(np.int32)
    # grid phase measured from edge statistics: blocks start at odd columns, even rows
    H, W = im.shape[:2]
    rows = H // 2            # 384
    cols = (W - 2) // 2      # 255, covering source cols 1..510
    out = np.zeros((rows, cols, 3), np.uint8)
    for r in range(rows):
        blk = im[2*r:2*r+2, 1:1+2*cols].reshape(2, cols, 2, 3).transpose(1, 0, 2, 3).reshape(cols, 4, 3)
        med = np.median(blk, axis=1)
        d = np.abs(blk - med[:, None, :]).sum(2)
        pick = d.argmin(1)
        out[r] = blk[np.arange(cols), pick]
    return out

if __name__ == '__main__':
    n = native()
    Image.fromarray(n).save(os.path.join(OUT, 'poster_native.png'))
    # title: source rows 48..293, cols 39..480 -> native rows 24..146, cols 19..239
    t = n[24:147, 19:240].astype(np.int32)
    dr = np.abs(t - np.array(RED_TITLE)).sum(2)
    dc = np.abs(t - np.array(CREAM)).sum(2)
    mask = (dr < dc) & (dr < 120)
    # split words at the gap (source cols 253..258 -> native col 126..128 of full, 107..109 in crop)
    colsum = mask.sum(0)
    print('title native size', t.shape, 'empty cols', [i for i in range(len(colsum)) if colsum[i] == 0])
    a = np.zeros(t.shape[:2] + (4,), np.uint8)
    a[mask] = RED_TITLE + (255,)
    Image.fromarray(a, 'RGBA').save(os.path.join(OUT, 'title_native.png'))
    print('poster native', n.shape)
