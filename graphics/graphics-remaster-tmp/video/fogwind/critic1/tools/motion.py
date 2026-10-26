"""critic1 motion instrument: what moves in the fog that the camera does not explain.

For each 30-frame window over the WHOLE clip and each fog region (boxes defined at the
frame-328 pose and carried with the scene by the camera's own track, measured on the
vanilla clip, so the soldiers stay out and the fog itself cannot bias the camera):
  R_t   = I_{t+1} - I_t shifted by the camera (the change the camera does not explain)
  MID   = rms of R band-passed to 2..16 px (blotches, bands), FINE = rms of R below 1.5 px (grain)
  SHEET = rms over t of the box mean of R (a whole-region pump), B60 = rms of 60 px block means
  Travel: the normalised cross-correlation of consecutive MID residuals (the later one
  aligned to the scene), averaged over the window: C0 = at lag 0; PEAK = the highest value
  at |lag| >= 2 px and its lag (px/frame vs the scene, x right, y down); TRAV = PEAK - max(C0, 0)
  clipped at 0; plus the sub-pixel peak position near the origin (|lag| < 3) = slow drift.
usage: python motion.py out.json label [label ...]
"""
import sys, json
import numpy as np
from scipy import ndimage
from concurrent.futures import ThreadPoolExecutor
from common import load, SCRATCH, Y0, phase_shift, fshift, bandpass, hann2

REG328 = {  # full-frame x0, x1, y0, y1 at frame 328
    "farhill": (500, 950, 495, 560),
    "farvalley": (1150, 1290, 520, 585),
    "valley": (1140, 1250, 595, 700),
    "bay": (1450, 1880, 640, 740),
    "haze": (1200, 1900, 470, 555),
    "lowmist": (1420, 1640, 770, 830),
}
PAD = 24
LAG = 20


def camera_track():
    """Per-region consecutive camera shift (dy, dx), from the vanilla clip, smoothed in time."""
    try:
        return np.load(f"{SCRATCH}/camtrack.npy", allow_pickle=True).item()
    except Exception:
        pass
    v = load("van")
    T = v.shape[0]
    # global track first (near hill + shore), to place the boxes
    g = np.load(f"{SCRATCH}/van_cam.npy")  # (T, 4, 2) from cam.py
    gd = g[:, [0, 2], :].mean(1)  # nearhill + shore
    gd = ndimage.uniform_filter1d(gd, 9, axis=0)
    cum = np.cumsum(gd, axis=0)  # cum[t] = position shift of content at t vs frame 0
    out = {"cum": cum, "reg": {}}

    def boxat(name, t):
        x0, x1, y0, y1 = REG328[name]
        dx = cum[t, 1] - cum[328, 1]
        dy = cum[t, 0] - cum[328, 0]
        return x0 + dx, x1 + dx, y0 + dy, y1 + dy

    for name in REG328:
        d = np.zeros((T, 2), np.float32)

        def one(t):
            x0, x1, y0, y1 = boxat(name, t)
            X0, X1 = int(max(0, x0 - 40)), int(min(1920, x1 + 40))
            YY0, YY1 = int(max(Y0, y0 - 30)), int(min(Y0 + v.shape[1], y1 + 30))
            if X1 - X0 < 120:
                return t, gd[t + 1]
            a = v[t, YY0 - Y0:YY1 - Y0, X0:X1]
            b = v[t + 1, YY0 - Y0:YY1 - Y0, X0:X1]
            return t, phase_shift(a, b, bp=(1.0, 6.0))

        with ThreadPoolExecutor(14) as ex:
            for t, s in ex.map(one, range(T - 1)):
                d[t + 1] = s
        # robust smoothing: median then mean over 9 frames
        d = ndimage.median_filter(d, size=(9, 1), mode="nearest")
        d = ndimage.uniform_filter1d(d, 5, axis=0)
        out["reg"][name] = d
    np.save(f"{SCRATCH}/camtrack.npy", out, allow_pickle=True)
    return out


def ncc_surface(X, Y, lag=LAG):
    """sum X(x) Y(x+l) / norm for |l| <= lag (both zero-mean windowed)."""
    h, w = X.shape
    H, W = h + lag, w + lag
    FX = np.fft.rfft2(X, (H, W))
    FY = np.fft.rfft2(Y, (H, W))
    c = np.fft.irfft2(np.conj(FX) * FY, (H, W))
    c = np.roll(np.roll(c, lag, 0), lag, 1)[:2 * lag + 1, :2 * lag + 1]
    return c / (np.sqrt((X * X).sum() * (Y * Y).sum()) + 1e-12)


def analyse(label, track, win=30):
    a = load(label)
    T = a.shape[0]
    cum = track["cum"]
    res = {}
    for name in REG328:
        # the haze box is smooth air in vanilla (no texture to track): use the far valley's track
        d = track["reg"]["farvalley" if name == "haze" else name]
        rows = []
        for w0 in range(0, T - 2, win):
            w1 = min(w0 + win, T - 2)
            if w1 - w0 < 10:
                break
            tc = (w0 + w1) // 2
            x0, x1, y0, y1 = REG328[name]
            dx = cum[tc, 1] - cum[328, 1]
            dy = cum[tc, 0] - cum[328, 0]
            x0, x1, y0, y1 = int(round(x0 + dx)), int(round(x1 + dx)), int(round(y0 + dy)), int(round(y1 + dy))
            x0, x1 = max(PAD, x0), min(1920 - PAD, x1)
            y0, y1 = max(Y0 + PAD, y0), min(Y0 + a.shape[1] - PAD, y1)
            if x1 - x0 < 100 or y1 - y0 < 40:
                continue
            win2 = hann2(y1 - y0, x1 - x0)

            def resid(t):
                A = a[t, y0 - Y0 - PAD:y1 - Y0 + PAD, x0 - PAD:x1 + PAD].astype(np.float32)
                B = a[t + 1, y0 - Y0 - PAD:y1 - Y0 + PAD, x0 - PAD:x1 + PAD].astype(np.float32)
                R = B - fshift(A, d[t + 1])
                return R

            Rs = [resid(t) for t in range(w0, w1 + 1)]
            mids = [bandpass(R, 1.0, 8.0)[PAD:-PAD, PAD:-PAD] for R in Rs]
            fines = [(R - ndimage.gaussian_filter(R, 1.0))[PAD:-PAD, PAD:-PAD] for R in Rs]
            core = [R[PAD:-PAD, PAD:-PAD] for R in Rs]
            sheet = np.array([c.mean() for c in core])
            b60 = []
            for c in core:
                hh, ww = (c.shape[0] // 30) * 30, (c.shape[1] // 60) * 60
                if hh >= 30 and ww >= 60:
                    bm = c[:hh, :ww].reshape(hh // 30, 30, ww // 60, 60).mean((1, 3))
                    b60.append(bm - bm.mean() * 0)  # absolute block means (incl. sheet)
            b60 = np.array(b60)
            C = np.zeros((2 * LAG + 1, 2 * LAG + 1))
            C2 = np.zeros_like(C)
            n = 0
            for i in range(len(mids) - 1):
                # align R_i to the scene of R_{i+1}: shift by the camera of step i+1
                X = fshift(mids[i], d[w0 + i + 2]) * win2
                Y = mids[i + 1] * win2
                X -= X.mean()
                Y -= Y.mean()
                C += ncc_surface(X, Y)
                if i + 2 < len(mids):
                    X2 = fshift(mids[i], d[w0 + i + 2] + d[w0 + i + 3]) * win2
                    Y2 = mids[i + 2] * win2
                    C2 += ncc_surface(X2 - X2.mean(), Y2 - Y2.mean())
                n += 1
            C /= n
            C2 /= max(n - 1, 1)
            ly, lx = np.mgrid[-LAG:LAG + 1, -LAG:LAG + 1]
            far = np.hypot(ly, lx) >= 2
            i = np.argmax(np.where(far, C, -9))
            pk, pky, pkx = C.flat[i], ly.flat[i], lx.flat[i]
            i2 = np.argmax(np.where(far, C2, -9))
            pk2, pk2y, pk2x = C2.flat[i2], ly.flat[i2], lx.flat[i2]
            c0 = C[LAG, LAG]
            # sub-pixel peak near the origin (centroid of positive C in |l|<=2)
            near = (np.hypot(ly, lx) <= 2) & (C > 0)
            wsum = C[near].sum()
            sx = float((C[near] * lx[near]).sum() / wsum) if wsum > 0 else 0.0
            sy = float((C[near] * ly[near]).sum() / wsum) if wsum > 0 else 0.0
            rows.append({
                "w": [w0, w1], "box": [x0, x1, y0, y1],
                "MID": float(np.sqrt(np.mean([m.var() for m in mids]))),
                "FINE": float(np.sqrt(np.mean([f.var() for f in fines]))),
                "SHEET": float(sheet.std()),
                "B60": float(np.sqrt(np.mean((b60 - b60.mean(0)) ** 2))) if len(b60) else None,
                "C0": float(c0), "PEAK": float(pk), "PEAKLAG": [int(pkx), int(pky)],
                "TRAV": float(max(0.0, pk - max(c0, 0.0))),
                "PEAK2": float(pk2), "PEAK2LAG": [int(pk2x), int(pk2y)],
                "SUBPX": [sx, sy],
                "C2_0": float(C2[LAG, LAG]),
                "Csurf": np.round(C[LAG - 6:LAG + 7, LAG - 14:LAG + 15], 4).tolist(),
                "C2surf": np.round(C2[LAG - 6:LAG + 7, LAG - 14:LAG + 15], 4).tolist(),
            })
        res[name] = rows
    return res


if __name__ == "__main__":
    out = sys.argv[1]
    track = camera_track()
    allres = {}
    for lab in sys.argv[2:]:
        allres[lab] = analyse(lab, track)
        print("done", lab, flush=True)
    json.dump(allres, open(out, "w"), indent=1)
