"""gen_flame_flipbook.py: the fire spread's flame flipbook (POSEIDON_XP_FIRESPREAD).

ONE flame tongue, built offline as physical fields and rendered into a
looping flipbook the renderer plays on its flame tongues
(PoseidonGL33/EngineGL33_FireFlames.cpp). Each tongue is drawn as a narrow
axial billboard that turns about its own bent axis to face the eye, and a
burning front is many of them side by side, so a frame holds a single flame:
a dense, bright foot rooted in the fuel bed (the continuous zone, flared so
neighbouring tongues' feet merge into one band), a body that narrows to a
ragged tip, and the pockets it sheds now and then. Nothing reaches the frame's
sides, so a tongue never shows its quad's edge. Our own art: nothing here
comes from a game data file.

The model is kinematic, built to the wildfire consult's flame structure
(2026-11-16, second round), with L the tongue's mean length:
  - the shape: a base 0.15 to 0.25 L wide (length to width 4 to 6 for grass,
    3 to 5 for shrub), tapering to a point; the foot flares into the bed;
  - the tip: pulses at irregular intervals (the period jittered by 40%),
    stretching to about 1.3 L and falling back to about 0.7 L (flame always
    present below about 0.65 L, never above about 1.35 L); about one pulse in
    three it necks and sheds a detached pocket that rises and burns out within
    about 0.3 L;
  - the gas: rises along the tongue at a speed growing as the square root of
    the height (McCaffrey), carrying a turbulent texture (fBm of smooth noise,
    stretched up, morphing as it rises); the centre line licks sideways in a
    wave that climbs with the gas, more toward the tip; the upper part breaks
    into holes and tatters;
  - the temperature: hottest in the foot (about 1450 to 1480 K: the continuous
    zone, 3 to 6 times the mid flame's luminance), about 1260 to 1290 K in the
    body, cooler in the tips and pockets;
  - soot: grass tongues burn clean; a shrub or crown tongue's tips and the
    space above them carry dark soot wisps.
The frames store physical fields, not a look (see "Physical fields" below):
temperature in kelvin and soot optical depth. The renderer makes the light
from them (Planck) in linear scene light, and the camera exposes it.
--preview approximates that over a night and a day background.

Frames loop exactly: the noise's cells are fitted to the loop (its gas rises a
whole number of lattice periods over the 128 frames), the pulses repeat every
loop and the pockets are carried over from a run-in loop, so the last frame
runs into the first with no seam and no cross-fade. Usage:

    python gen_flame_flipbook.py <outdir> [--preview <sheet.png>] [--seed N] [--styles a,b]

writes flame_<style>.png (one 16 x 8 atlas a style, STYLE_ORDER, each frame
FRAME_W x FRAME_H) into outdir, and prints each style's measured rise (frame
heights a frame: the renderer's kFlipRise, EngineGL33_FireFlames.cpp, must
match), its flame tip (the share of a frame's height its flame reaches half
the time: FireSpreadWorld.hpp kFlameTipShare and the shaders' kFlipFlameShare
must match, so the renderer draws a tongue's tip at the length the fire model
gives it), its foot's width (the share of the frame's width the base spans:
FireSpreadWorld.hpp kFlameBaseShare, which sizes each tongue's quad), and its
pulse period (frames).

    python gen_flame_flipbook.py --measure <dir>

prints the tips and bases of the flame_<style>.png already in dir.

Deterministic for a given seed (numpy's PCG64 and float64 throughout).
"""
import argparse
import numpy as np
from scipy.ndimage import map_coordinates

N_FRAMES = 128        # frames in the loop (16 x 8 atlas)
COLS = 16
FRAME_W = 128         # a frame's texels across ...
FRAME_H = 256         # ... and up (the renderer reads the frame's size from the atlas)
SIDE_TAPER = 12       # the frame's outer columns fade to nothing (texels): no tongue touches its quad's side
BOTTOM_FADE = 2.0     # the lowest rows fade in (texels; the renderer fades the foot in from the ground, ragged)


# The flame styles: the fire picks one per tongue (FireSpreadWorld.cpp FlameStyle),
# by its fuel and intensity, with a random mix on top. All single tongues.
#   L0     the mean tip, a share of the frame's height
#   width  the base's width over the length (the consult: 0.15 to 0.25 grass, 0.2 to 0.3 shrub)
#   flare  the foot's flare: (extra width share at the ground, its height over the length)
#   taper  the half width falls as (1 - y / tip)^taper
#   sway   the centre line's lick at the tip (a share of the length)
#   edge   the edge's noise (a share of the half width, growing up the tongue)
#   holes  how far the upper part breaks into holes and tatters (0..1)
#   pulse  (period in frames, jitter share, stretch): the tip runs between (1 - s) and (1 + s) of L; the
#          gas rises about 0.65 L a pulse (the consult: f = 2.5 U^0.2 L^-0.6 against 0.7 sqrt(g L))
#   shed   the share of pulses that shed a pocket
#   fork   (chance a pulse forks, the side tongue's offset over the base half width, its length share)
#   temp   the simulation temperature at the foot, in the body and at the tip (kelvin = 1150 + 330 T)
#   tnoise how far the body's temperature varies (a share of it, a noise stretched up the tongue: streaks)
#   soot   the soot's optical depth over the tips and above (0: burns clean)
#   rise   the gas's rise at half the mean length (texels a frame); it grows as sqrt(height)
STYLES = {
    # a grass head tongue: a bright foot, a slim pointed body, clean tips
    'front': dict(L0=0.60, width=0.22, flare=(0.3, 0.08), taper=0.55, sway=0.08, edge=0.45, holes=0.45,
                  pulse=(19, 0.4, 0.28), shed=0.33, fork=(0.25, 0.9, 0.55), temp=(0.85, 0.36, 0.28),
                  soot=0.05, rise=7.6, tnoise=0.32),
    # a slimmer, livelier tongue that stretches, necks and sheds more often
    'tongues': dict(L0=0.62, width=0.18, flare=(0.4, 0.07), taper=0.5, sway=0.11, edge=0.5, holes=0.55,
                    pulse=(17, 0.45, 0.32), shed=0.45, fork=(0.15, 0.8, 0.5), temp=(0.85, 0.36, 0.28),
                    soot=0.04, rise=8.0, tnoise=0.34),
    # a low flank or backing flame: stubby, a wide foot, small tips
    'creep': dict(L0=0.56, width=0.24, flare=(0.25, 0.10), taper=0.5, sway=0.07, edge=0.5, holes=0.45,
                  pulse=(16, 0.45, 0.25), shed=0.25, fork=(0.3, 0.9, 0.5), temp=(0.82, 0.35, 0.28),
                  soot=0.04, rise=7.0, tnoise=0.32),
    # a shrub or crown tongue: fat, often forked, sooty, dark tips into smoke
    'bushy': dict(L0=0.58, width=0.28, flare=(0.3, 0.10), taper=0.5, sway=0.09, edge=0.55, holes=0.5,
                  pulse=(21, 0.4, 0.30), shed=0.40, fork=(0.6, 1.0, 0.65), temp=(0.82, 0.38, 0.29),
                  soot=1.6, rise=7.8, tnoise=0.36),
}
STYLE_ORDER = ['front', 'tongues', 'creep', 'bushy']


class Noise3:
    """fBm of smooth noise on a periodic lattice (t, l, x), sampled with a cubic
    B-spline (no prefilter: smooth, never overshooting); about unit variance."""

    def __init__(self, rng, shape, octaves=3, gain=0.55):
        self.shape = shape
        self.lat = [rng.standard_normal(tuple(s * 2 ** o for s in shape)) for o in range(octaves)]
        self.gain = gain
        self.scale = 1.0
        probe = self(np.linspace(0, shape[2], 4096), np.linspace(0, 3.7 * shape[1], 4096) % shape[1],
                     np.linspace(0, 1.3 * shape[0], 4096) % shape[0])
        self.scale = 1.0 / max(float(np.std(probe)), 1e-6)

    def __call__(self, x, l, t):
        x, l, t = np.broadcast_arrays(np.asarray(x, dtype=np.float64), np.asarray(l, dtype=np.float64),
                                      np.asarray(t, dtype=np.float64))
        out, amp = 0.0, 1.0
        for o, lat in enumerate(self.lat):
            f = 2 ** o
            out = out + amp * map_coordinates(lat, [t * f, l * f, x * f], order=3, mode='grid-wrap', prefilter=False)
            amp *= self.gain
        return out * self.scale

    def at(self, x, cx, l, cl, v0, t, ct):
        """Sampled at x / cx across, the gas's label l over a cell near cl and the
        time t over a cell near ct, both cells fitted to the loop: the label
        falls v0 N_FRAMES over it, a whole number of lattice periods, and so
        does the time, so frame N_FRAMES is frame 0."""
        nt, nl, nx = self.shape
        cl = v0 * N_FRAMES / (nl * max(1, round(v0 * N_FRAMES / (nl * cl))))
        ct = N_FRAMES / (nt * max(1, round(N_FRAMES / (nt * ct))))
        return self((x / cx) % nx, (l / cl) % nl, (t / ct) % nt)


def sstep(a, b, x):
    t = np.clip((x - a) / (b - a), 0.0, 1.0)
    return t * t * (3.0 - 2.0 * t)


class Tongue:
    def __init__(self, rng, style):
        self.st = st = STYLES[style]
        self.rng = rng
        self.L0 = st['L0'] * FRAME_H
        self.lref = 0.5 * self.L0
        self.y0 = 0.15 * self.L0
        self.v0 = st['rise']
        # the pulses: a whole number over the loop at irregular intervals (the
        # period jittered), each with its own stretch, fork and shed, repeating
        # every loop
        period, jitter, _ = st['pulse']
        n = max(2, int(round(N_FRAMES / period)))
        gaps = period * (1.0 + jitter * rng.uniform(-1.0, 1.0, size=n))
        gaps *= N_FRAMES / gaps.sum()
        starts = rng.uniform(0.0, gaps[0]) + np.concatenate([[0.0], np.cumsum(gaps)[:-1]])
        amp = rng.uniform(0.85, 1.15, size=n)  # how far each pulse stretches
        shed = rng.uniform(0, 1, size=n) < st['shed']
        fork = rng.uniform(0, 1, size=n) < st['fork'][0]
        fside = np.where(rng.uniform(0, 1, size=n) < 0.5, -1.0, 1.0)
        reps = 5  # loops -2 .. 2
        self.starts = np.concatenate([starts + N_FRAMES * k for k in range(-2, 3)])
        self.amp, self.shed = np.tile(amp, reps), np.tile(shed, reps)
        self.fork, self.fside = np.tile(fork, reps), np.tile(fside, reps)
        self.npulse = n
        # the noise: x across, l the gas's label up, t time (Noise3.at fits them to the loop)
        self.n_edge = Noise3(rng, (16, 64, 12), octaves=3)
        self.n_int = Noise3(rng, (16, 64, 12), octaves=3)
        self.n_hole = Noise3(rng, (16, 64, 12), octaves=2)
        self.n_sway = Noise3(rng, (8, 32, 6), octaves=2)
        self.n_soot = Noise3(rng, (16, 64, 12), octaves=3)
        self.n_hot = Noise3(rng, (16, 64, 12), octaves=2)
        yy, xx = np.mgrid[0:FRAME_H, 0:FRAME_W].astype(np.float64)
        self.y = yy + 0.5
        self.x = xx + 0.5 - 0.5 * FRAME_W
        self.pockets = []  # [birth frame, x, y, half height, half width, temperature, pulse]

    def label(self, y, t):
        """The gas's label: constant on a parcel rising at v0 sqrt((y + y0) / (lref + y0)) * sqrt(..)."""
        return 2.0 * np.sqrt((y + self.y0) * (self.lref + self.y0)) - self.v0 * t

    def speed(self, y):
        return self.v0 * np.sqrt((y + self.y0) / (self.lref + self.y0))

    def tip(self, t):
        """The main tongue's tip now (texels) and the pulse it is in."""
        st = self.st
        i = int(np.searchsorted(self.starts, t, side='right') - 1)
        t0, t1 = self.starts[i], self.starts[i + 1]
        p = (t - t0) / (t1 - t0)
        s = st['pulse'][2]
        a = 1.0 - s * self.amp[i]
        b = 1.0 + s * self.amp[i]
        an = 1.0 - s * self.amp[i + 1]
        grow = 0.72
        if p < grow:
            f = a + (b - a) * sstep(0.0, grow, p) ** 0.85
        else:
            q = (p - grow) / (1.0 - grow)
            # a shed drops the tip at once (the top goes on as a pocket); else it collapses back
            k = sstep(0.0, 0.25, q) if self.shed[i] else sstep(0.0, 1.0, q)
            f = b + (an - b) * k
        return f * self.L0, i, p

    def envelope(self, x, y, xc, tipy, halfw0, t, taper, side=0.0):
        """The tongue's density (0..1): a tapering body about its centre line xc,
        its edge broken by noise climbing with the gas."""
        st = self.st
        fl, fh = st['flare']
        h = np.clip(1.0 - y / np.maximum(tipy, 1.0), 0.0, 1.0) ** taper
        hw = halfw0 * h * (1.0 + fl * np.exp(-(y / (fh * self.L0)) ** 2))
        l = self.label(y, t)
        rel = y / self.L0
        ne = self.n_edge.at(self.x + 31.0 * side, 10.0, l, 14.0, self.v0, t, 9.0)
        r = np.abs(x - xc) / np.maximum(hw, 0.6) + st['edge'] * 0.35 * ne * (0.9 + rel)
        # (nothing past the tip: the body closes over its last few percent)
        d = (1.0 - sstep(0.55, 1.05, r)) * (1.0 - sstep(0.9, 1.02, y / np.maximum(tipy, 1.0)))
        # the upper part breaks into holes and tatters
        nh = self.n_hole.at(self.x + 32.0 * side, 6.0, l, 9.0, self.v0, t, 6.0)
        d *= 1.0 - st['holes'] * sstep(0.45, 0.95, y / np.maximum(tipy, 1.0)) * sstep(0.2, 0.9, nh)
        return d

    def centre(self, y, t):
        st = self.st
        l = self.label(y, t)
        ns = self.n_sway.at(np.full_like(y, 2.0), 1.0, l, 30.0, self.v0, t, 14.0)
        return st['sway'] * self.L0 * (y / self.L0) ** 1.3 * np.clip(ns, -2.0, 2.0) * 0.6

    def frame(self, t):
        """(temperature, soot optical depth), row 0 at the BOTTOM."""
        st = self.st
        x, y = self.x, self.y
        tipy, i, p = self.tip(t)
        xc = self.centre(y, t)
        halfw0 = 0.5 * st['width'] * self.L0
        d = self.envelope(x, y, xc, tipy, halfw0, t, st['taper'])
        # a fork: a side tongue rising out of the main one's upper half
        if self.fork[i]:
            fo, fl = st['fork'][1], st['fork'][2]
            grow = sstep(0.0, 0.3, p) * (1.0 - sstep(0.85, 1.0, p))
            off = self.fside[i] * fo * halfw0 * sstep(0.1 * self.L0, 0.55 * self.L0, y)
            d2 = self.envelope(x, y, xc + off, np.maximum(fl * tipy * grow, 1.0), 0.75 * halfw0, t, st['taper'],
                               side=1.0)
            d = np.maximum(d, d2 * grow)
        self.update_pockets(t)
        dp = np.zeros_like(d)
        tp = np.zeros_like(d)
        life = self.pocket_life()
        for pk in self.pockets:
            age = t - pk[0]
            if age < 0 or age > life:
                continue
            yc = pk[2] + self.speed(pk[2]) * age * 1.1
            k = 1.0 - age / life
            hh, hw = pk[3] * (1.0 + 0.3 * age / life), pk[4] * (1.0 - 0.3 * age / life)
            l = self.label(y, t)
            ne = self.n_edge.at(self.x + 61.0, 8.0, l, 11.0, self.v0, t, 7.0)
            r = np.sqrt(((x - pk[1]) / hw) ** 2 + ((y - yc) / hh) ** 2) + 0.45 * ne
            nh = self.n_hole.at(self.x + 45.0, 5.0, l, 7.0, self.v0, t, 5.0)
            dk = (1.0 - sstep(0.35, 1.0, r)) * sstep(0.0, 0.25, k) * (1.0 - 0.6 * sstep(0.0, 0.9, nh) * (1.0 - k))
            dp = np.maximum(dp, dk)
            tp = np.maximum(tp, dk * pk[5] * (0.75 + 0.25 * k))
        hw0 = 0.5 * st['width'] * self.L0 * np.clip(1.0 - y / np.maximum(tipy, 1.0), 0.05, 1.0) ** st['taper']
        core = 1.0 - sstep(0.0, 1.0, np.abs(x - xc) / np.maximum(0.8 * hw0, 1.0))
        return self.fields(t, d, dp, tp, xc, tipy, core)

    def pocket_life(self):
        return 0.3 * self.L0 / max(float(self.speed(self.L0)), 0.5)

    def update_pockets(self, t):
        """A shed: at the end of a shedding pulse's stretch the top above the
        neck goes on as a pocket (kept for its life)."""
        st = self.st
        tipy, i, p = self.tip(t)
        life = self.pocket_life()
        self.pockets = [pk for pk in self.pockets if t - pk[0] <= life]
        if self.shed[i] and p >= 0.72 and not any(pk[6] == i for pk in self.pockets):
            s = st['pulse'][2]
            top = (1.0 + s * self.amp[i]) * self.L0
            neck = 0.62 * top
            yc = 0.5 * (neck + top)
            xcp = float(self.centre(np.array([yc]), t)[0])
            hwp = 0.5 * st['width'] * self.L0 * (1.0 - neck / top) ** st['taper'] * 1.4
            self.pockets.append([t, xcp, yc, 0.5 * (top - neck) * 0.6, max(hwp, 3.0),
                                 0.5 * (st['temp'][1] + st['temp'][2]), i])

    def fields(self, t, d, dp, tp, xc, tipy, core):
        st = self.st
        x, y = self.x, self.y
        # the temperature: hottest in the foot (the continuous zone, its top
        # wandering across the tongue and in time: no level line where the
        # hot foot meets the body), cooling up the body to the tips
        tb, tm, tt = st['temp']
        rel = y / self.L0
        l = self.label(y, t)
        hot = 0.22 + 0.09 * np.clip(self.n_hot.at(self.x, 9.0, 0.0, 14.0, self.v0, t, 12.0), -2.0, 2.0)
        rel_n = rel + 0.06 * np.clip(self.n_hot.at(self.x + 40.0, 5.0, l, 9.0, self.v0, t, 7.0), -2.0, 2.0)
        # (hot on the core: the bright foot is a small pointed flame inside the tongue, not a band across it)
        prof = tm + (tb - tm) * (1.0 - sstep(hot - 0.12, hot + 0.14, rel_n)) * (0.3 + 0.7 * core ** 0.7)
        tipn = 0.1 * self.L0 * np.clip(self.n_hot.at(self.x + 80.0, 7.0, l, 12.0, self.v0, t, 9.0), -2.0, 2.0)
        prof = prof + (tt - tm) * sstep(0.45, 1.0, (y + tipn) / np.maximum(tipy, 1.0))
        ni = self.n_int.at(self.x, 5.0, l, 13.0, self.v0, t, 8.0)
        temp = d ** 0.7 * prof * (1.0 + st['tnoise'] * np.clip(ni, -2.5, 2.5))
        temp = np.maximum(temp, tp)
        # soot: over the upper part and the tips, wisps above the tip
        soot = np.zeros_like(temp)
        if st['soot'] > 0:
            ns = self.n_soot.at(self.x, 9.0, l, 16.0, self.v0, t, 10.0)
            wide = self.envelope(x, y, xc, tipy * 1.45, 0.5 * st['width'] * self.L0 * 1.5, t, 0.45, side=2.0)
            up = sstep(0.55 * tipy, 1.0 * tipy, y)
            soot = st['soot'] * np.clip(wide * up * (0.6 + 0.4 * ns), 0.0, None)
            soot = np.maximum(soot, st['soot'] * 0.8 * dp * sstep(0.0, 0.6, 1.0 - tp / max(tt, 1e-3)))
            soot *= 1.0 - sstep(0.80, 0.97, y / FRAME_H)
        # the frame's edges: the lowest rows fade in, the sides taper to nothing
        edge = sstep(0.0, 1.0, np.minimum(x + 0.5 * FRAME_W, 0.5 * FRAME_W - x) / SIDE_TAPER)
        bottom = sstep(0.0, BOTTOM_FADE, y - 0.5)
        topf = 1.0 - sstep(0.86, 0.98, y / FRAME_H)
        temp = temp * edge * bottom * topf
        soot = soot * edge * bottom
        return np.stack([temp, soot], axis=-1)


# ---- Physical fields, stored for the renderer ----
# The renderer, not this script, makes the light: the frame stores what the
# flame physically is, and the shader turns it into radiance (Planck at the
# temperature, times the emissivity) in linear scene light, which the camera
# then exposes like everything else. So no colour ramp and no glow are baked
# here: the colour is the black body's, the glow the camera's glare.
#
#   R  temperature: (K - K0) / K_SPAN, the soot's temperature in kelvin
#   G  soot optical depth / TAU_MAX (smoke above and round the flame)
#   B  0
#   A  the luminous zone's emissivity (0..1; the measurements and the preview)
K0, K_SPAN = 600.0, 1000.0        # R's range: 600 to 1600 K
TAU_MAX = 4.0                     # G's range: optical depth 0 to 4
# The simulation's temperature (0 cold air, about 1 in the hottest gas) to the
# soot's radiation temperature in kelvin, what the eye sees: the luminous zone
# runs from about 1190 K at its edge (EMIT_LO) through about 1260 to 1300 K in
# its body to about 1450 to 1480 K in the foot.
SIM_K_COLD, SIM_K_PER_T = 1150.0, 330.0
EMISSIVITY = 0.6                  # the preview's emissivity (the renderer sets each tongue's own)
EMIT_LO, EMIT_HI = 0.12, 0.45     # the luminous zone fades in over this simulation temperature


def kelvin(T):
    return SIM_K_COLD + SIM_K_PER_T * np.clip(T, 0.0, 1.6)


def bake(raw):
    """raw: rows x columns x 2 (temperature, soot tau), row 0 at the bottom; out row 0 at the top."""
    T = raw[..., 0][::-1]
    tau = raw[..., 1][::-1]
    zone = sstep(EMIT_LO, EMIT_HI, T)
    k = kelvin(T)
    # cold air is written as 0, not as SIM_K_COLD: the shader rebuilds the
    # emissivity from R (EngineGL33_FireFlameShaders.cpp kFlipSimKCold,
    # kFlipSimKPerT, which must equal SIM_K_COLD, SIM_K_PER_T here)
    r = np.where(zone > 0.0, np.clip((k - K0) / K_SPAN, 0.0, 1.0), 0.0)
    g = np.clip(tau / TAU_MAX, 0.0, 1.0)
    return np.stack([r, g, np.zeros_like(r), EMISSIVITY * zone], axis=-1)


def planck_rgb(k):
    """Linear sRGB radiance of a black body at k kelvin, relative to 1300 K's
    luminance (a crude three-wavelength sample; the preview only)."""
    lam = np.array([610e-9, 550e-9, 465e-9])
    c2 = 1.4388e-2

    def b(kk):
        return 1.0 / (lam ** 5 * (np.exp(c2 / (lam * kk[..., None])) - 1.0))
    ref = b(np.array([1300.0]))[0]
    lum = lambda x: 0.2126 * x[..., 0] + 0.7152 * x[..., 1] + 0.0722 * x[..., 2]
    return b(np.maximum(k, 300.0)) / lum(ref)


def simulate(style, seed, rise=None):
    """The style's N_FRAMES looping frames (raw, row 0 at the bottom).
    rise (a list): gets the luminous gas's mean upward speed, in frame heights a frame."""
    rng = np.random.Generator(np.random.PCG64(seed))
    tg = Tongue(rng, style)
    # a run-in loop: the pockets shed before frame 0 are carried in
    for f in range(-N_FRAMES, 0):
        tg.update_pockets(float(f))
    frames, speeds = [], []
    for f in range(N_FRAMES):
        fr = tg.frame(float(f))
        frames.append(fr)
        lum = fr[..., 0] > EMIT_HI
        if lum.any():
            speeds.append(float(tg.speed(tg.y[lum]).mean()))
    if rise is not None:
        rise.append(float(np.mean(speeds)) / FRAME_H if speeds else 0.0)
    return frames


TIP_ZONE = 0.25  # luminous where the zone (emissivity / EMISSIVITY) is past this


def lum_mask(alpha):
    """frames x rows x columns, row 0 at the BOTTOM: luminous."""
    a = np.asarray(alpha)[:, ::-1, :]
    return a > EMISSIVITY * TIP_ZONE


def tip_share(alpha):
    """The flame tip as a share of the frame's height: the highest row where
    flame is present (anywhere across) in at least half the frames (the mean
    flame length: flame there half the time)."""
    lum = lum_mask(alpha)
    present = lum.any(axis=2).mean(axis=0)
    rows = np.nonzero(present >= 0.5)[0]
    return float(rows.max() + 1) / lum.shape[1] if rows.size else 0.0


def span_share(alpha, at):
    """The median over the frames of the luminous span across at row share `at`."""
    lum = lum_mask(alpha)
    row = max(1, int(round(at * lum.shape[1])))
    spans = []
    for f in lum[:, row, :]:
        idx = np.nonzero(f)[0]
        spans.append((idx.max() - idx.min() + 1) if idx.size else 0)
    return float(np.median(spans)) / lum.shape[2]


def pulse_frames(alpha):
    """The pulse's mean period (frames): from the luminous top's time series,
    the first peak of its autocorrelation."""
    lum = lum_mask(alpha)
    tops = np.array([(np.nonzero(f.any(axis=1))[0].max() if f.any() else 0) for f in lum], dtype=np.float64)
    x = tops - tops.mean()
    ac = np.correlate(np.concatenate([x, x]), x, mode='valid')[:len(x) // 2]
    ac /= ac[0] if ac[0] else 1.0
    for i in range(3, len(ac) - 1):
        if ac[i] > ac[i - 1] and ac[i] >= ac[i + 1] and ac[i] > 0.0:
            return float(i)
    return 0.0


def kelvin_stats(frames, tip):
    """The luminous zone's kelvin (p50, p90, p99) and the foot's median (its lowest 25%)."""
    ks, foot = [], []
    for f in frames:
        m = f[..., 3] > 0.5 * EMISSIVITY
        k = K0 + K_SPAN * f[..., 0]
        ks.append(k[m])
        lo = int(round((1.0 - 0.25 * tip) * f.shape[0]))
        foot.append(k[lo:][m[lo:]])
    ks = np.concatenate(ks)
    foot = np.concatenate(foot)
    p = np.percentile(ks, [50, 90, 99]) if ks.size else np.zeros(3)
    return p, (float(np.median(foot)) if foot.size else 0.0)


def preview_rgb(img, bg, exposure):
    """Roughly what the camera shows: the black-body light (linear) at an
    exposure, over a flat background bg seen through the soot, then a soft
    shoulder and the display gamma (the preview only)."""
    k = K0 + K_SPAN * img[..., 0]
    tau = TAU_MAX * img[..., 1]
    eps = img[..., 3:4]
    light = eps * planck_rgb(k) * exposure
    trans = np.exp(-tau)[..., None]
    smoke = 0.35 * bg.mean() * np.array([1.0, 0.92, 0.82])
    out = (bg * trans + smoke * (1.0 - trans)) * (1.0 - 0.6 * eps) + light
    out = out / (1.0 + out / 4.0)
    return np.round(np.clip(out, 0, 1) ** (1 / 2.2) * 255).astype(np.uint8)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('outdir', help='writes flame_<style>.png for every style there (with --measure: reads them)')
    ap.add_argument('--measure', action='store_true', help='print the tips of the files in outdir, build nothing')
    ap.add_argument('--preview', default='', help='a sheet: sixteen frames of each style, one row a style')
    ap.add_argument('--seed', type=int, default=1985)
    ap.add_argument('--styles', default=','.join(STYLE_ORDER))
    a = ap.parse_args()
    from PIL import Image
    import os
    rows_n = (N_FRAMES + COLS - 1) // COLS
    if a.measure:
        for style in a.styles.split(','):
            img = np.asarray(Image.open(os.path.join(a.outdir, f'flame_{style}.png'))).astype(np.float64) / 255.0
            fw, fh = img.shape[1] // COLS, img.shape[0] // rows_n
            frames = [img[(i // COLS) * fh:(i // COLS + 1) * fh, (i % COLS) * fw:(i % COLS + 1) * fw, 3]
                      for i in range(N_FRAMES)]
            tip = tip_share(frames)
            print(f'{style}: tip {tip:.3f} of a frame, base {span_share(frames, 0.12 * tip):.3f} of its width')
        return
    rows_prev = []
    for k, style in enumerate(a.styles.split(',')):
        rise = []
        out = [bake(f) for f in simulate(style, a.seed + 101 * k, rise)]
        alpha = [f[..., 3] for f in out]
        tip = tip_share(alpha)
        ks, foot = kelvin_stats(out, tip)
        print(f'{style}: rise {rise[0]:.5f} frame heights a frame, tip {tip:.3f} of a frame, '
              f'base {span_share(alpha, 0.12 * tip):.3f} and mid {span_share(alpha, 0.5 * tip):.3f} of its width, '
              f'pulse {pulse_frames(alpha):.0f} frames, kelvin p50/p90/p99 {ks[0]:.0f}/{ks[1]:.0f}/{ks[2]:.0f}, '
              f'foot {foot:.0f}')
        atlas = np.zeros((rows_n * FRAME_H, COLS * FRAME_W, 4))
        for i, f in enumerate(out):
            r, c = divmod(i, COLS)
            atlas[r * FRAME_H:(r + 1) * FRAME_H, c * FRAME_W:(c + 1) * FRAME_W] = f
        path = os.path.join(a.outdir, f'flame_{style}.png')
        Image.fromarray(np.round(np.clip(atlas, 0, 1) * 255).astype(np.uint8), 'RGBA').save(path, format='PNG',
                                                                                             optimize=True)
        print(path)
        rows_prev.append(np.concatenate([out[i] for i in range(0, 32, 2)], axis=1))
    if a.preview:
        sheet = np.concatenate(rows_prev, axis=0)
        night = preview_rgb(sheet, np.array([0.015, 0.012, 0.01]), 0.6)
        day = preview_rgb(sheet, np.array([0.30, 0.27, 0.18]), 0.04)
        sky = preview_rgb(sheet, np.array([0.35, 0.45, 0.65]), 0.04)
        Image.fromarray(np.concatenate([night, day, sky], axis=0), 'RGB').save(a.preview, format='PNG')
        print(a.preview)


if __name__ == '__main__':
    main()
