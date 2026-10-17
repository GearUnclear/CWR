"""ctypes bridge to the engine's pure remastered-lighting C++ (lab_native.cpp).

Builds a shared library from engine/Poseidon/Graphics/Remaster/*.cpp and the
shim on first use (needs clang++ or g++ on PATH), cached under the repo-root
tmp/ (gitignored) and rebuilt when any source changes.
"""

from __future__ import annotations

import ctypes
import hashlib
import shutil
import subprocess
from pathlib import Path

import numpy as np

LAB_DIR = Path(__file__).resolve().parent
ENGINE_ROOT = LAB_DIR.parents[2]  # arma_CWR
REMASTER_DIR = ENGINE_ROOT / "engine" / "Poseidon" / "Graphics" / "Remaster"
SOURCES = [LAB_DIR / "lab_native.cpp", REMASTER_DIR / "LightRig.cpp", REMASTER_DIR / "TerrainHorizon.cpp",
           REMASTER_DIR / "CloudNoise.cpp"]
HEADERS = sorted(REMASTER_DIR.glob("*.hpp"))
CACHE_DIR = ENGINE_ROOT.parent / "tmp" / "lighting_lab" / "_native"

_lib = None


def _digest() -> str:
    h = hashlib.sha256()
    for p in SOURCES + HEADERS:
        h.update(p.read_bytes())
    return h.hexdigest()[:16]


def _compiler() -> str:
    for c in ("clang++", "g++", "c++"):
        if shutil.which(c):
            return c
    raise RuntimeError("no C++ compiler on PATH (clang++/g++) to build the lab's native helper")


def lib() -> ctypes.CDLL:
    global _lib
    if _lib is not None:
        return _lib
    CACHE_DIR.mkdir(parents=True, exist_ok=True)
    out = CACHE_DIR / f"lab_native_{_digest()}.so"
    if not out.exists():
        cmd = [_compiler(), "-std=c++17", "-O2", "-shared", "-fPIC", f"-I{ENGINE_ROOT / 'engine'}",
               *[str(s) for s in SOURCES], "-o", str(out), "-lpthread"]
        subprocess.run(cmd, check=True)
    L = ctypes.CDLL(str(out))
    f32p = ctypes.POINTER(ctypes.c_float)
    u8p = ctypes.POINTER(ctypes.c_uint8)
    L.lab_compute_rig.argtypes = [f32p, f32p, f32p]
    L.lab_default_tuning.argtypes = [f32p]
    u16p = ctypes.POINTER(ctypes.c_uint16)
    L.lab_build_horizon.argtypes = [f32p, ctypes.c_int, ctypes.c_float, u16p, u8p, f32p]
    L.lab_build_horizon.restype = ctypes.c_int
    L.lab_cloud_noise.argtypes = [ctypes.c_int, ctypes.c_uint, u8p]
    L.lab_crown_ellipsoid.argtypes = [f32p, f32p, f32p, f32p]
    L.lab_crown_ellipsoid.restype = None
    _lib = L
    return L


def _fp(a: np.ndarray):
    return a.ctypes.data_as(ctypes.POINTER(ctypes.c_float))


def _u8(a: np.ndarray):
    return a.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8))


TUNING_KEYS = ["exposure", "sun", "sky", "bounce", "warmth", "terrainShadows", "terrainOcclusion", "cloudShadows",
               "specular", "localLights", "moonlight", "ambientOcclusion", "bloom", "shadowSoftness", "saturation",
               "sunShafts"]


def default_tuning() -> dict:
    t = np.zeros(16, dtype=np.float32)
    lib().lab_default_tuning(_fp(t))
    return {k: float(t[i]) for i, k in enumerate(TUNING_KEYS)}


def compute_rig(to_sun, to_moon, moon_phase, sun_diffuse, ambient, fog, sky_through, night, clouds_active,
                tuning: dict) -> dict:
    inp = np.zeros(22, dtype=np.float32)
    inp[0:3] = to_sun
    inp[3:6] = to_moon
    inp[6] = moon_phase
    inp[7:10] = np.asarray(sun_diffuse)[:3]
    inp[10:13] = np.asarray(ambient)[:3]
    inp[13:16] = np.asarray(fog)[:3]
    inp[16] = sky_through
    inp[17] = night
    inp[18] = 1.0 if clouds_active else 0.0
    t = np.zeros(16, dtype=np.float32)
    for i, k in enumerate(TUNING_KEYS):
        t[i] = tuning[k]
    out = np.zeros(24, dtype=np.float32)
    lib().lab_compute_rig(_fp(inp), _fp(t), _fp(out))
    return {"sun": out[0:3].copy(), "zenith": out[3:6].copy(), "horizon": out[6:9].copy(), "ground": out[9:12].copy(),
            "moon": out[12:15].copy(), "elevation": float(out[15]), "cover": float(out[16]),
            "transmit": float(out[17]), "exposure": float(out[18]), "horizon_sun": out[20:23].copy()}


def build_horizon(heights: np.ndarray, cell: float):
    h = np.ascontiguousarray(heights, dtype=np.float32)
    n = h.shape[0]
    hor = np.zeros(4 * n * n * 4, dtype=np.uint16)
    sky = np.zeros(n * n, dtype=np.uint8)
    hgt = np.zeros(n * n, dtype=np.float32)
    size = lib().lab_build_horizon(_fp(h), n, float(cell), hor.ctypes.data_as(ctypes.POINTER(ctypes.c_uint16)),
                                   _u8(sky), _fp(hgt))
    if size != n:
        raise RuntimeError(f"horizon build returned size {size} for a {n} grid")
    return hor.reshape(4, n, n, 4), sky.reshape(n, n), hgt.reshape(n, n)


def crown_ellipsoid(box_min, box_max):
    """remaster::CrownEllipsoid: (centre, inverse half extents)."""
    mn = np.ascontiguousarray(box_min, dtype=np.float32)
    mx = np.ascontiguousarray(box_max, dtype=np.float32)
    c = np.zeros(3, dtype=np.float32)
    ih = np.zeros(3, dtype=np.float32)
    lib().lab_crown_ellipsoid(_fp(mn), _fp(mx), _fp(c), _fp(ih))
    return c, ih


def cloud_noise(size: int = 256, seed: int = 1) -> np.ndarray:
    out = np.zeros(size * size, dtype=np.uint8)
    if not lib().lab_cloud_noise(size, seed, _u8(out)):
        raise RuntimeError("cloud noise build failed")
    return out.reshape(size, size)
