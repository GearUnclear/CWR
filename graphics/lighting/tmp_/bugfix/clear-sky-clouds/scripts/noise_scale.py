"""Port of Poseidon::remaster::BuildCloudNoise (CloudNoise.cpp) to numpy, to
measure the cloud-shadow blob sizes the shipped field gives on the ground and
how often an eye-level view contains a cloud edge."""
import sys
import numpy as np
from scipy import ndimage

M32 = 0xFFFFFFFF


def hash32(x, y, seed):
    x = x.astype(np.uint64)
    y = y.astype(np.uint64)
    h = ((x * 0x8DA6B343) & M32) ^ ((y * 0xD8163841) & M32) ^ ((seed * 0xCB1AB31F) & M32)
    h ^= h >> 13
    h = (h * 0x5BD1E995) & M32
    h ^= h >> 15
    return h


def lattice(x, y, period, seed):
    wx = ((x % period) + period) % period
    wy = ((y % period) + period) % period
    return (hash32(wx, wy, seed) & 0xFFFF).astype(np.float64) / 65535.0


def value_noise(u, v, period, seed):
    x = u * period
    y = v * period
    x0 = np.floor(x).astype(np.int64)
    y0 = np.floor(y).astype(np.int64)
    tx = x - x0
    ty = y - y0
    tx = tx * tx * tx * (tx * (tx * 6 - 15) + 10)
    ty = ty * ty * ty * (ty * (ty * 6 - 15) + 10)
    a = lattice(x0, y0, period, seed)
    b = lattice(x0 + 1, y0, period, seed)
    c = lattice(x0, y0 + 1, period, seed)
    d = lattice(x0 + 1, y0 + 1, period, seed)
    ab = a + (b - a) * tx
    cd = c + (d - c) * tx
    return ab + (cd - ab) * ty


def build(size, seed, base_period=4, octaves=5):
    idx = (np.arange(size) + 0.5) / size
    u, v = np.meshgrid(idx, idx)
    s = np.zeros_like(u)
    amp = 1.0
    period = base_period
    for o in range(octaves):
        s += amp * value_noise(u, v, period, (seed + o * 7919) & M32)
        amp *= 0.5
        period *= 2
    flat = s.ravel()
    order = np.argsort(flat, kind="stable")
    out = np.empty(flat.size, dtype=np.uint8)
    out[order] = (np.arange(flat.size) * 256) // flat.size
    return out.reshape(size, size)


def blobs(mask, texel_m):
    # wrap-aware labelling: tile 3x3, label, take the centre tile's components
    big = np.tile(mask, (3, 3))
    lab, n = ndimage.label(big)
    n0 = mask.shape[0]
    centre = lab[n0:2 * n0, n0:2 * n0]
    ids = np.unique(centre[centre > 0])
    areas = np.array([(lab == i).sum() for i in ids]) * texel_m * texel_m
    # equivalent diameter
    return 2.0 * np.sqrt(areas / np.pi)


def view_stats(field01, cover, tile_m, rng, n=4000, rmax=1500.0, half_fov=0.6):
    size = field01.shape[0]
    thr = 1.0 - cover
    mixed = 0
    fracs = []
    for _ in range(n):
        cx, cz = rng.uniform(0, tile_m, 2)
        yaw = rng.uniform(0, 2 * np.pi)
        r = np.sqrt(rng.uniform((30 / rmax) ** 2, 1.0, 400)) * rmax
        a = yaw + rng.uniform(-half_fov, half_fov, 400)
        px = (cx + r * np.cos(a)) % tile_m
        pz = (cz + r * np.sin(a)) % tile_m
        ix = (px / tile_m * size).astype(int) % size
        iz = (pz / tile_m * size).astype(int) % size
        cov = field01[iz, ix] >= thr
        f = cov.mean()
        fracs.append(f)
        if 0.1 < f < 0.9:
            mixed += 1
    fracs = np.array(fracs)
    return mixed / n, (fracs < 0.02).mean(), (fracs > 0.98).mean()


def report(label, field, tile_m, cover, rng):
    f01 = field.astype(np.float64) / 255.0
    texel = tile_m / field.shape[0]
    mask = f01 >= 1.0 - cover
    d = blobs(mask, texel)
    g = blobs(~mask, texel)
    mixed, allLit, allShade = view_stats(f01, cover, tile_m, rng)
    print(f"{label:34s} cover {cover:.3f} covered {mask.mean():.3f} | cloud blobs n={len(d):3d} "
          f"median {np.median(d):6.0f} m, area-weighted mean {np.sum(d**3)/np.sum(d**2):6.0f} m | "
          f"gaps n={len(g):3d} area-weighted {np.sum(g**3)/np.sum(g**2):6.0f} m | "
          f"views mixed {mixed:.2f} all-lit {allLit:.2f} all-shade {allShade:.2f}")


if __name__ == "__main__":
    rng = np.random.default_rng(3)
    shipped = build(256, 1)
    for cover in (0.041, 0.10, 0.263, 0.30, 0.50):
        report("shipped 6 km tile", shipped, 6000.0, cover, rng)
    print()
    for cover in (0.263, 0.30):
        report("same field, 3 km tile", shipped, 3000.0, cover, rng)
