"""Render presets = one camera preset x one lighting preset (renderer-independent)."""

from __future__ import annotations

from dataclasses import dataclass

from lighting import LIGHTING_PRESETS, LightingPreset
from scene import CameraPreset, Scene

CAMERA_ORDER = ["testview", "ground", "overview", "closeup"]
LIGHTING_ORDER = ["noon", "afternoon", "golden", "overcast"]


@dataclass
class RenderPreset:
    camera: CameraPreset
    lighting: LightingPreset

    @property
    def name(self) -> str:
        return f"{self.camera.name}_{self.lighting.name}"


def make_presets(scene: Scene, cameras: list[str] | None = None, lightings: list[str] | None = None) -> list[RenderPreset]:
    cams = cameras or CAMERA_ORDER
    lits = lightings or LIGHTING_ORDER
    out = []
    for c in cams:
        if c not in scene.cameras:
            raise KeyError(f"unknown camera preset {c!r}; have {sorted(scene.cameras)}")
        for lname in lits:
            if lname not in LIGHTING_PRESETS:
                raise KeyError(f"unknown lighting preset {lname!r}; have {sorted(LIGHTING_PRESETS)}")
            out.append(RenderPreset(scene.cameras[c], LIGHTING_PRESETS[lname]))
    return out
