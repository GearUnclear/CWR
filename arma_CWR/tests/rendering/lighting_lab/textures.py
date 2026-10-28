"""Deterministic procedural textures for the lighting lab (numpy only).

All textures are gamma-space RGBA float arrays in [0, 1] of shape (H, W, 4),
row 0 = v 0. They tile seamlessly (noise is generated in the frequency domain,
which is periodic by construction). `to_rgba8` converts for upload.

The colours are stand-ins chosen to resemble OFP/CWA content (olive grass,
beige plaster, red-brown roofs, olive-drab vehicles); they are not derived from
game data.
"""

from __future__ import annotations

import numpy as np


def periodic_noise(size: int, seed: int, beta: float = 2.0, fmin: float = 1.0, fmax: float | None = None,
                   aspect: tuple[float, float] = (1.0, 1.0)) -> np.ndarray:
    """Tileable fractal noise, zero mean, unit std.

    beta: spectral exponent (power ~ 1/f^beta); fmin/fmax: band limits in
    cycles per texture; aspect stretches the spectrum (e.g. (1, 6) makes
    streaks along v).
    """
    rng = np.random.default_rng(seed)
    white = rng.standard_normal((size, size))
    f = np.fft.fft2(white)
    fy = np.fft.fftfreq(size)[:, None] * size * aspect[1]
    fx = np.fft.fftfreq(size)[None, :] * size * aspect[0]
    r = np.sqrt(fx * fx + fy * fy)
    amp = np.zeros_like(r)
    band = r >= fmin
    if fmax is not None:
        band &= r <= fmax
    amp[band] = 1.0 / np.power(r[band], beta / 2.0)
    n = np.real(np.fft.ifft2(f * amp))
    n -= n.mean()
    s = n.std()
    return n / s if s > 0 else n


def _rgb(base, *mods) -> np.ndarray:
    base = np.asarray(base, dtype=np.float64)
    out = np.broadcast_to(base, mods[0].shape + (3,)).copy() if mods else base
    return out


def _finish(rgb: np.ndarray, alpha: np.ndarray | float = 1.0) -> np.ndarray:
    h, w = rgb.shape[:2]
    out = np.empty((h, w, 4), dtype=np.float64)
    out[..., :3] = np.clip(rgb, 0.0, 1.0)
    out[..., 3] = np.clip(alpha, 0.0, 1.0)
    return out


def _smoothstep(e0, e1, x):
    t = np.clip((x - e0) / (e1 - e0), 0.0, 1.0)
    return t * t * (3 - 2 * t)


def grass(size: int = 256, seed: int = 11) -> np.ndarray:
    """Olive grass, gamma-space mean ~ (0.36, 0.40, 0.20), mottled."""
    low = periodic_noise(size, seed, beta=2.6, fmin=1, fmax=12)
    mid = periodic_noise(size, seed + 1, beta=1.6, fmin=6, fmax=60)
    hi = periodic_noise(size, seed + 2, beta=0.6, fmin=40)
    hue = periodic_noise(size, seed + 3, beta=2.8, fmin=1, fmax=8)
    base = np.array([0.36, 0.40, 0.20])
    yellow = np.array([0.43, 0.42, 0.20]) - base  # dry patches
    lum = 1.0 + 0.10 * low + 0.07 * mid + 0.06 * hi
    rgb = base[None, None, :] * lum[..., None] + yellow[None, None, :] * (0.5 * np.clip(hue, -1, 2))[..., None]
    return _finish(rgb)


def dirt(size: int = 256, seed: int = 21) -> np.ndarray:
    """Packed dirt / track, warm brown with pebbles."""
    low = periodic_noise(size, seed, beta=2.4, fmin=1, fmax=16)
    hi = periodic_noise(size, seed + 1, beta=0.8, fmin=30)
    peb = periodic_noise(size, seed + 2, beta=0.2, fmin=60)
    base = np.array([0.47, 0.40, 0.30])
    lum = 1.0 + 0.10 * low + 0.07 * hi
    rgb = base[None, None, :] * lum[..., None]
    pebbles = _smoothstep(1.6, 2.2, peb)
    rgb = rgb * (1 - pebbles[..., None]) + np.array([0.58, 0.55, 0.50]) * pebbles[..., None]
    grass_tufts = _smoothstep(0.8, 1.6, periodic_noise(size, seed + 3, beta=1.4, fmin=8, fmax=40))
    rgb = rgb * (1 - 0.6 * grass_tufts[..., None]) + np.array([0.36, 0.38, 0.20]) * 0.6 * grass_tufts[..., None]
    return _finish(rgb)


def detail(size: int = 256, seed: int = 31) -> np.ndarray:
    """Terrain detail texture: grey RGB, alpha ~ 0.5 +- 0.15 (PSDetail uses t1.a * 2, 0.5 = neutral)."""
    n = 0.6 * periodic_noise(size, seed, beta=1.2, fmin=4) + 0.4 * periodic_noise(size, seed + 1, beta=0.4, fmin=24)
    n = n / np.max(np.abs(n))
    a = 0.5 + 0.15 * n
    a = a - a.mean() + 0.5  # mean exactly neutral
    rgb = np.full((size, size, 3), 0.5) + 0.08 * n[..., None]
    return _finish(rgb, a)


def plaster_wall(size: int = 256, seed: int = 41) -> np.ndarray:
    """Beige plaster (~(0.74, 0.69, 0.58)) with one window; one tile = 4 m x 3 m of wall.

    v = 0 at the top of the wall.
    """
    low = periodic_noise(size, seed, beta=2.4, fmin=1, fmax=10)
    hi = periodic_noise(size, seed + 1, beta=1.0, fmin=20)
    base = np.array([0.74, 0.69, 0.58])
    lum = 1.0 + 0.05 * low + 0.035 * hi
    rgb = base[None, None, :] * lum[..., None]
    v = np.linspace(0, 1, size, endpoint=False)[:, None] + 0.5 / size
    u = np.linspace(0, 1, size, endpoint=False)[None, :] + 0.5 / size
    # grime toward the ground
    grime = _smoothstep(0.7, 1.0, v) * (0.12 + 0.05 * low)
    rgb = rgb * (1 - grime[..., None]) + np.array([0.42, 0.38, 0.30]) * grime[..., None]
    # window: frame + glass + cross bars
    wx0, wx1, wy0, wy1 = 0.36, 0.64, 0.24, 0.66
    frame_w = 0.025
    in_frame = (u > wx0) & (u < wx1) & (v > wy0) & (v < wy1)
    in_glass = (u > wx0 + frame_w) & (u < wx1 - frame_w) & (v > wy0 + frame_w) & (v < wy1 - frame_w)
    bar = in_glass & ((np.abs(u - 0.5) < 0.008) | (np.abs(v - 0.43) < 0.008))
    frame_col = np.array([0.36, 0.30, 0.24])
    glass_col = np.array([0.16, 0.19, 0.22]) + 0.06 * (v - wy0)[..., None] * np.ones(3)
    rgb = np.where(in_frame[..., None], frame_col, rgb)
    rgb = np.where(in_glass[..., None], glass_col * (1 + 0.05 * hi[..., None]), rgb)
    rgb = np.where(bar[..., None], frame_col, rgb)
    # sill shadow
    sill = (u > wx0 - 0.02) & (u < wx1 + 0.02) & (v >= wy1) & (v < wy1 + 0.02)
    rgb = np.where(sill[..., None], np.array([0.62, 0.58, 0.50]), rgb)
    return _finish(rgb)


def roof_tiles(size: int = 256, seed: int = 51) -> np.ndarray:
    """Red-brown clay tiles in staggered rows; one tile = 2 m x 2 m of roof."""
    low = periodic_noise(size, seed, beta=2.2, fmin=1, fmax=12)
    hi = periodic_noise(size, seed + 1, beta=0.8, fmin=24)
    base = np.array([0.55, 0.27, 0.18])
    rows, cols = 8, 6
    v = (np.arange(size)[:, None] + 0.5) / size
    u = (np.arange(size)[None, :] + 0.5) / size
    fv = (v * rows) % 1.0
    row_idx = np.floor(v * rows)
    fu = (u * cols + 0.5 * (row_idx % 2)) % 1.0
    tile_rng = np.random.default_rng(seed + 7)
    tint = tile_rng.uniform(-0.08, 0.08, size=(rows, cols * 2))
    ti = (np.floor(u * cols + 0.5 * (row_idx % 2)) % (cols * 2)).astype(int)
    tile_tint = tint[row_idx.astype(int) % rows, ti]
    shade = 1.0 - 0.35 * _smoothstep(0.75, 1.0, fv) + 0.06 * (0.5 - fv)  # overlap shadow at row bottom
    gap = 1.0 - 0.25 * (1 - _smoothstep(0.0, 0.04, np.minimum(fu, 1 - fu)))
    lum = (1 + tile_tint + 0.05 * low + 0.05 * hi) * shade * gap
    rgb = base[None, None, :] * lum[..., None]
    return _finish(rgb)


def vehicle_paint(size: int = 256, seed: int = 61) -> np.ndarray:
    """Olive drab (~(0.31, 0.33, 0.21)) with panel lines, rivets and dust."""
    low = periodic_noise(size, seed, beta=2.4, fmin=1, fmax=10)
    hi = periodic_noise(size, seed + 1, beta=1.0, fmin=30)
    base = np.array([0.31, 0.33, 0.21])
    v = (np.arange(size)[:, None] + 0.5) / size
    u = (np.arange(size)[None, :] + 0.5) / size
    lum = 1.0 + 0.05 * low + 0.04 * hi
    rgb = base[None, None, :] * lum[..., None]
    # dust toward the bottom of each tile (tiles map ~2 m)
    dust = _smoothstep(0.55, 1.0, v) * np.clip(0.25 + 0.15 * low, 0, 1)
    rgb = rgb * (1 - dust[..., None]) + np.array([0.46, 0.41, 0.31]) * dust[..., None]
    # panel lines
    pu = np.minimum((u * 2) % 1.0, 1 - (u * 2) % 1.0)
    pv = np.minimum((v * 2) % 1.0, 1 - (v * 2) % 1.0)
    line = (1 - _smoothstep(0.0, 0.012, pu)) + (1 - _smoothstep(0.0, 0.012, pv))
    rgb = rgb * (1 - 0.45 * np.clip(line, 0, 1)[..., None])
    # rivets along panel edges
    ru = (u * 16) % 1.0
    rv = (v * 2) % 1.0
    rivet = ((ru - 0.5) ** 2 + ((rv - 0.05) * 8) ** 2 < 0.02) | ((ru - 0.5) ** 2 + ((rv - 0.95) * 8) ** 2 < 0.02)
    rgb = np.where(rivet[..., None], rgb * 1.25, rgb)
    return _finish(rgb)


def bark(size: int = 256, seed: int = 71) -> np.ndarray:
    """Grey-brown bark with vertical furrows."""
    streak = periodic_noise(size, seed, beta=1.6, fmin=2, aspect=(1.0, 8.0))
    low = periodic_noise(size, seed + 1, beta=2.4, fmin=1, fmax=8)
    base = np.array([0.34, 0.29, 0.23])
    furrow = _smoothstep(0.6, 1.8, streak)
    lum = (1.0 + 0.08 * low) * (1 - 0.45 * furrow) + 0.06 * streak
    rgb = base[None, None, :] * lum[..., None]
    return _finish(rgb)


def foliage(size: int = 256, seed: int = 81) -> np.ndarray:
    """Leaf cluster card with an alpha cutout (alpha 1 on leaves, 0 in gaps).

    RGB under transparent texels is filled with the mean leaf colour so
    bilinear/mip filtering does not drag dark fringes into the cutout edge.
    """
    rng = np.random.default_rng(seed)
    yy, xx = np.mgrid[0:size, 0:size].astype(np.float64)
    yy = (yy + 0.5) / size
    xx = (xx + 0.5) / size
    alpha = np.zeros((size, size))
    rgb = np.zeros((size, size, 3))
    # crown envelope: blobby ellipse, denser in the middle
    cx, cy = 0.5, 0.48
    env_noise = periodic_noise(size, seed + 5, beta=2.2, fmin=1, fmax=6)
    rr = np.sqrt(((xx - cx) / 0.46) ** 2 + ((yy - cy) / 0.44) ** 2) + 0.08 * env_noise
    n_leaves = 1500
    for _ in range(n_leaves):
        # sample leaf centres inside the envelope
        for _try in range(8):
            px, py = rng.uniform(0.04, 0.96, 2)
            ix, iy = int(px * size), int(py * size)
            if rr[iy, ix] < rng.uniform(0.55, 1.0):
                break
        else:
            continue
        ang = rng.uniform(0, np.pi)
        L = rng.uniform(0.022, 0.045)
        W = L * rng.uniform(0.35, 0.55)
        x0 = max(int((px - L) * size), 0)
        x1 = min(int((px + L) * size) + 2, size)
        y0 = max(int((py - L) * size), 0)
        y1 = min(int((py + L) * size) + 2, size)
        sx = xx[y0:y1, x0:x1] - px
        sy = yy[y0:y1, x0:x1] - py
        ca, sa = np.cos(ang), np.sin(ang)
        lu = (sx * ca + sy * sa) / L
        lv = (-sx * sa + sy * ca) / W
        inside = lu * lu + lv * lv < 1.0
        shade = rng.uniform(0.75, 1.15)
        col = np.array([rng.uniform(0.20, 0.30), rng.uniform(0.30, 0.40), rng.uniform(0.09, 0.15)]) * shade
        # slight gradient across the leaf (vein side lighter)
        leaf_rgb = col[None, None, :] * (1.0 + 0.12 * lv[..., None])
        sub_a = alpha[y0:y1, x0:x1]
        sub_rgb = rgb[y0:y1, x0:x1]
        sub_rgb[inside] = leaf_rgb[inside]
        sub_a[inside] = 1.0
    # a few twigs
    for _ in range(7):
        ang = rng.uniform(0, 2 * np.pi)
        r0 = rng.uniform(0.0, 0.1)
        r1 = rng.uniform(0.25, 0.42)
        t = np.linspace(r0, r1, 200)
        px = cx + np.cos(ang) * t
        py = cy + np.sin(ang) * t + 0.08 * t * t
        ix = np.clip((px * size).astype(int), 0, size - 1)
        iy = np.clip((py * size).astype(int), 0, size - 1)
        for d in (0, 1):
            alpha[iy, np.clip(ix + d, 0, size - 1)] = 1.0
            rgb[iy, np.clip(ix + d, 0, size - 1)] = (0.22, 0.18, 0.13)
    mean_leaf = rgb[alpha > 0.5].mean(axis=0) if np.any(alpha > 0.5) else np.array([0.25, 0.35, 0.12])
    rgb[alpha < 0.5] = mean_leaf
    return _finish(rgb, alpha)


def sky(size: int = 256, overcast: float = 0.0, seed: int = 91) -> np.ndarray:
    """Sky dome texture: row 0 (v = 0) = horizon, last row = zenith.

    Clear: horizon (0.75, 0.80, 0.85) -> zenith (0.45, 0.60, 0.85).
    Overcast: flat grey with soft cloud structure; `overcast` in [0,1] blends.
    Scene::CalculateSkyColor reads pixel (1,1), which with this layout is the
    horizon tone (an assumption about the original sky textures' layout).
    """
    v = (np.arange(size)[:, None] + 0.5) / size
    t = np.power(v, 0.55)
    horizon = np.array([0.75, 0.80, 0.85])
    zenith = np.array([0.45, 0.60, 0.85])
    clear = horizon[None, None, :] * (1 - t[..., None]) + zenith[None, None, :] * t[..., None]
    clear = np.broadcast_to(clear, (size, size, 3)).copy()
    clouds = periodic_noise(size, seed, beta=2.4, fmin=1, fmax=20, aspect=(1.0, 2.0))
    o_h = np.array([0.70, 0.72, 0.74])
    o_z = np.array([0.56, 0.58, 0.62])
    over = o_h[None, None, :] * (1 - t[..., None]) + o_z[None, None, :] * t[..., None]
    over = over * (1.0 + 0.06 * clouds[..., None] * (0.3 + 0.7 * t[..., None]))
    rgb = clear * (1 - overcast) + over * overcast
    return _finish(rgb)


def horizon(width: int = 256, height: int = 64, seed: int = 151) -> np.ndarray:
    """Stand-in for the horizont texture: v 0..0.5 is the upper strip (alpha
    ramps 0 at the top -> 1 at the mid line, with a distant tree-line edge),
    v 0.5..1 the lower strip (opaque). It is drawn ~900 m away, so the fragment
    colour is almost fully fog; only the alpha shape matters."""
    v = (np.arange(height)[:, None] + 0.5) / height
    rng = np.random.default_rng(seed)
    u = np.arange(width) / width
    edge = np.zeros(width)
    for k in range(1, 14):
        edge += rng.uniform(0.2, 1.0) / k * np.sin(2 * np.pi * (k * 3 * u + rng.uniform(0, 1)))
    edge = 0.30 + 0.07 * edge / np.max(np.abs(edge))  # tree-line height in v
    ramp = _smoothstep(0.0, 1.0, (v - 0.05) / (edge[None, :] - 0.05))
    alpha = np.where(v < edge[None, :], ramp * 0.55, 1.0)
    alpha = np.where(v >= 0.5, 1.0, alpha)
    rgb = np.broadcast_to(np.array([0.42, 0.47, 0.42]), (height, width, 3)).copy()
    return _finish(rgb, alpha)


def sun_disc(size: int = 64) -> np.ndarray:
    """Sun object texture: white disc, soft edge, alpha outside 0."""
    yy, xx = np.mgrid[0:size, 0:size].astype(np.float64)
    r = np.sqrt(((xx + 0.5) / size - 0.5) ** 2 + ((yy + 0.5) / size - 0.5) ** 2) * 2
    a = 1 - _smoothstep(0.80, 0.98, r)
    return _finish(np.ones((size, size, 3)), a)


def sun_halo(size: int = 128) -> np.ndarray:
    """Sun halo texture: radial falloff in alpha."""
    yy, xx = np.mgrid[0:size, 0:size].astype(np.float64)
    r = np.sqrt(((xx + 0.5) / size - 0.5) ** 2 + ((yy + 0.5) / size - 0.5) ** 2) * 2
    a = np.clip(1 - r, 0, 1) ** 2.2 * 0.85
    return _finish(np.ones((size, size, 3)), a)


def rock(size: int = 256, seed: int = 101) -> np.ndarray:
    low = periodic_noise(size, seed, beta=2.0, fmin=1, fmax=16)
    hi = periodic_noise(size, seed + 1, beta=0.8, fmin=20)
    lich = _smoothstep(1.0, 1.8, periodic_noise(size, seed + 2, beta=2.0, fmin=2, fmax=24))
    base = np.array([0.47, 0.46, 0.43])
    rgb = base[None, None, :] * (1 + 0.12 * low + 0.08 * hi)[..., None]
    rgb = rgb * (1 - 0.5 * lich[..., None]) + np.array([0.45, 0.47, 0.30]) * 0.5 * lich[..., None]
    return _finish(rgb)


def crate(size: int = 256, seed: int = 111) -> np.ndarray:
    """Wooden crate side: planks + frame; one tile = one crate face."""
    grain = periodic_noise(size, seed, beta=1.4, fmin=2, aspect=(8.0, 1.0))
    low = periodic_noise(size, seed + 1, beta=2.2, fmin=1, fmax=8)
    base = np.array([0.52, 0.42, 0.27])
    v = (np.arange(size)[:, None] + 0.5) / size
    u = (np.arange(size)[None, :] + 0.5) / size
    plank = np.minimum((v * 5) % 1.0, 1 - (v * 5) % 1.0)
    seam = 1 - _smoothstep(0.0, 0.03, plank)
    edge = (u < 0.08) | (u > 0.92) | (v < 0.08) | (v > 0.92)
    lum = (1 + 0.06 * grain + 0.06 * low) * (1 - 0.4 * seam)
    rgb = base[None, None, :] * lum[..., None]
    rgb = np.where(edge[..., None], rgb * 0.82, rgb)
    return _finish(rgb)


def uniform_cloth(size: int = 256, seed: int = 121) -> np.ndarray:
    """Olive uniform with a subtle blotch pattern."""
    blot = periodic_noise(size, seed, beta=2.6, fmin=2, fmax=14)
    hi = periodic_noise(size, seed + 1, beta=0.6, fmin=40)
    base = np.array([0.33, 0.34, 0.24])
    dark = _smoothstep(0.4, 0.7, blot)
    rgb = base[None, None, :] * (1 + 0.04 * hi)[..., None]
    rgb = rgb * (1 - dark[..., None]) + np.array([0.24, 0.23, 0.17]) * dark[..., None]
    return _finish(rgb)


def skin(size: int = 64, seed: int = 131) -> np.ndarray:
    n = periodic_noise(size, seed, beta=2.0, fmin=1, fmax=8)
    rgb = np.array([0.72, 0.56, 0.45])[None, None, :] * (1 + 0.04 * n)[..., None]
    return _finish(rgb)


def rubber(size: int = 128, seed: int = 141) -> np.ndarray:
    n = periodic_noise(size, seed, beta=1.0, fmin=4)
    v = (np.arange(size)[:, None] + 0.5) / size
    tread = (np.floor(v * 12) % 2) * 0.03
    rgb = np.full((size, size, 3), 0.13) * (1 + 0.08 * n + tread)[..., None]
    return _finish(rgb)


def camo_net(size: int = 256, seed: int = 161) -> np.ndarray:
    """Camouflage net card: a diamond mesh of cord with leaf-like garnish
    patches in olive, brown and sand (alpha 1), holes between (alpha 0)."""
    yy, xx = np.mgrid[0:size, 0:size].astype(np.float64)
    u = (xx + 0.5) / size
    v = (yy + 0.5) / size
    cells = 10.0
    d1 = np.abs(((u + v) * cells) % 1.0 - 0.5)
    d2 = np.abs(((u - v) * cells) % 1.0 - 0.5)
    cord = (np.minimum(d1, d2) > 0.44).astype(np.float64)
    patches = periodic_noise(size, seed, beta=1.6, fmin=3, fmax=40)
    tone = periodic_noise(size, seed + 1, beta=2.0, fmin=1, fmax=8)
    garnish = (patches > -0.15).astype(np.float64)
    alpha = np.maximum(cord, garnish)
    olive = np.array([0.30, 0.33, 0.18])
    brown = np.array([0.33, 0.27, 0.17])
    sand = np.array([0.45, 0.42, 0.30])
    w1 = _smoothstep(-0.6, 0.4, tone)[..., None]
    w2 = _smoothstep(0.8, 1.6, tone)[..., None]
    rgb = olive * (1 - w1) + brown * w1
    rgb = rgb * (1 - w2) + sand * w2
    rgb = rgb * (0.9 + 0.1 * np.clip(patches, -1, 1))[..., None]
    rgb[cord > 0.5] = np.array([0.24, 0.23, 0.17])
    return _finish(rgb, alpha)


def build_all(seed: int = 0) -> dict[str, dict]:
    """name -> {'rgba': float array, 'clamp_u': bool, 'clamp_v': bool}."""
    s = seed
    tex = {
        "grass": grass(256, 11 + s),
        "dirt": dirt(256, 21 + s),
        "detail": detail(256, 31 + s),
        "plaster": plaster_wall(256, 41 + s),
        "roof": roof_tiles(256, 51 + s),
        "paint": vehicle_paint(256, 61 + s),
        "bark": bark(256, 71 + s),
        "foliage": foliage(256, 81 + s),
        "rock": rock(256, 101 + s),
        "crate": crate(256, 111 + s),
        "uniform": uniform_cloth(256, 121 + s),
        "skin": skin(64, 131 + s),
        "rubber": rubber(128, 141 + s),
        "sun_disc": sun_disc(64),
        "sun_halo": sun_halo(128),
        "horizon": horizon(256, 64, 151 + s),
        "camonet": camo_net(256, 161 + s),
    }
    out = {k: {"rgba": v, "clamp_u": False, "clamp_v": False} for k, v in tex.items()}
    out["horizon"].update(clamp_v=True)  # horizont shape: ClampV
    out["sun_disc"].update(clamp_u=True, clamp_v=True)
    out["sun_halo"].update(clamp_u=True, clamp_v=True)
    return out


def to_rgba8(rgba: np.ndarray) -> np.ndarray:
    return np.clip(np.round(rgba * 255.0), 0, 255).astype(np.uint8)
