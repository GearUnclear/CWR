"""Python port of the engine's sun / ambient / fog / material lighting model.

Every function here names the C++ it mirrors. Colours are RGBA numpy arrays
and follow the engine's ColorP arithmetic exactly: `*` and `+` act on all four
channels (alpha included), `Color(r, g, b)` has alpha 1, and SaturateMinMax
clamps all four channels to [0, 1].

Sources (paths relative to arma_CWR/engine/Poseidon):
  Graphics/Rendering/Lighting/Lights.cpp      LightSun::Recalculate, colour constants
  World/Scene/Scene.cpp                        Scene::MainLightChanged (sun diffuse, fog colour)
  Graphics/Core/Engine.cpp                     Engine::InitDraw (_accomodateEye = HWhite * brightness)
  Graphics/Rendering/Lighting/TransLight.cpp   CreateMaterial*, DoSkyLighting, sun object lighting
  Graphics/Rendering/Lighting/Material.cpp     TexMaterial::Combine (specular/emissive * accom)
  World/Terrain/LandscapeRender.cpp            night-eye factor, AnimatorDefault::GetMaterial
  ../PoseidonGL33/EngineGL33_Shaders.cpp       UploadVSMaterialConstants, EnableNightEye
"""

from __future__ import annotations

import math
from dataclasses import dataclass, field

import numpy as np


def Color(r, g, b, a=1.0) -> np.ndarray:  # noqa: N802 - mirrors the engine type name
    return np.array([r, g, b, a], dtype=np.float64)


def saturate(c: np.ndarray) -> np.ndarray:
    """ColorP::SaturateMinMax."""
    return np.clip(c, 0.0, 1.0)


def with_alpha(c: np.ndarray, a: float) -> np.ndarray:
    c = np.array(c, dtype=np.float64)
    c[3] = a
    return c


HBlack = Color(0, 0, 0)
HWhite = Color(1, 1, 1)

# Lights.cpp constants
BackgroundColor = Color(0.55, 0.6, 0.8)
FullSunColor = Color(0.85, 0.75, 0.4)
SunsetColor = Color(0.8, 0.30, 0.23)
MoonColor = Color(0, 0, 0)
SunsetSkyColor = Color(1.0, 0.2, 0.1)
SunsetObjectColor = Color(1.0, 0.8, 0.1)
SunsetHaloObjectColor = Color(0.9, 0.4, 0.2)
SunObjectColor = Color(1, 1, 0.9)
SunHaloObjectColor = Color(0.9, 0.9, 0.7)

MIN_BACK_INTENSITY = 0.05
MIN_SKY_INTENSITY = 0.03
sinSunSunset = math.sin(math.radians(20))  # sunset object limit
sinBegSunset = math.sin(math.radians(25))  # sunset light
sinEndSunset = math.sin(math.radians(10))  # sunrise angle range
sinNightAngle = math.sin(math.radians(5))  # night effect angle


@dataclass
class LightingPreset:
    """Inputs of one lighting setup (what the lab varies)."""

    name: str
    sun_elevation: float  # degrees above the horizon
    sun_azimuth: float  # degrees, 0 = north (+Z), 90 = east (+X)
    sky_through: float = 1.0  # Landscape::SkyThrough (1 clear .. 0 overcast)
    brightness: float = 1.6  # Engine _usrBrightness (GraphicsConfig default 1.6)
    gamma: float = 1.2  # GraphicsConfig default gamma (ApplyGammaPass invGamma = 1/gamma)
    fog_start: float = 100.0  # Scene::GetFogMinRange
    fog_end: float = 900.0  # Scene::GetFogMaxRange (also camera far plane)
    description: str = ""


LIGHTING_PRESETS: dict[str, LightingPreset] = {
    p.name: p
    for p in [
        LightingPreset("noon", 60.0, 235.0, description="high sun, clear"),
        LightingPreset("afternoon", 30.0, 235.0, description="mid sun, clear"),
        LightingPreset("golden", 7.0, 235.0, description="low warm sun, clear"),
        LightingPreset("overcast", 45.0, 235.0, sky_through=0.2, description="mid sun, skyThrough 0.2"),
        LightingPreset("lowsun", 14.0, 195.0, description="low warm sun from the left, clear of the ridge"),
        LightingPreset("broken", 40.0, 215.0, sky_through=0.6, description="broken cloud, skyThrough 0.6"),
        LightingPreset("morning", 32.0, 95.0, description="clear 08:00: sun in the east, behind the right shoulder"),
    ]
}


def sun_to_direction(elevation_deg: float, azimuth_deg: float) -> np.ndarray:
    """Unit vector the sunlight TRAVELS along (LightSun::_direction / Direction()).

    Engine world: X east, Y up, Z north (left-handed). sinSun = -direction.y.
    """
    el, az = math.radians(elevation_deg), math.radians(azimuth_deg)
    to_sun = np.array([math.sin(az) * math.cos(el), math.sin(el), math.cos(az) * math.cos(el)])
    return -to_sun / np.linalg.norm(to_sun)


@dataclass
class SunLighting:
    """Everything the renderers need from the engine's lighting state."""

    preset: LightingPreset
    direction: np.ndarray  # light travel direction (shader uniform sunDir.xyz)
    to_sun: np.ndarray  # -direction
    sin_sun: float
    color_full: np.ndarray  # LightSun::_colorFull
    diffuse: np.ndarray  # LightSun::Diffuse() after Scene::MainLightChanged
    ambient: np.ndarray  # LightSun::Ambient()
    sky_color: np.ndarray  # LightSun::SkyColor()
    sun_sky_color: np.ndarray  # LightSun::SunSkyColor()
    sun_object_color: np.ndarray
    sun_halo_object_color: np.ndarray
    night_effect: float
    night_eye: float  # Landscape::DrawRect: (1 - diffuse.R) * nightEffect
    accom: np.ndarray  # Engine::GetAccomodateEye()
    sky_texture_color: np.ndarray  # Scene::_skyColor (sky texture pixel (1,1))
    fog_color: np.ndarray  # Engine::FogColor() (also the frame clear colour)
    rgb_eye_coef: np.ndarray  # PS c7 (EngineGL33::EnableNightEye)
    extras: dict = field(default_factory=dict)

    def summary(self) -> dict:
        r = lambda v: [round(float(x), 4) for x in v]  # noqa: E731
        return {
            "sunDir": r(self.direction),
            "sinSun": round(self.sin_sun, 4),
            "colorFull": r(self.color_full),
            "diffuse": r(self.diffuse),
            "ambient": r(self.ambient),
            "skyColor": r(self.sky_color),
            "sunSkyColor": r(self.sun_sky_color),
            "nightEffect": round(self.night_effect, 4),
            "nightEye": round(self.night_eye, 4),
            "accom": r(self.accom),
            "skyTextureColor": r(self.sky_texture_color),
            "fogColor": r(self.fog_color),
        }


def recalculate_sun(direction: np.ndarray) -> dict:
    """LightSun::Recalculate, the part that depends only on the sun direction.

    (The engine derives _direction from date/time/latitude; the lab takes it as
    an input. The moon and stars are ignored.)
    """
    sinSun = float(-direction[1])
    absSinSun = abs(sinSun)
    if sinSun < 0:
        if absSinSun > sinEndSunset:
            sunSkyColor = Color(0, 0, 0)
            colorFull = MoonColor.copy()
        else:
            sunset = 1 - absSinSun / sinEndSunset
            colorFull = MoonColor + (SunsetColor - MoonColor) * sunset
            sunSkyColor = SunsetSkyColor * sunset
        nightEffect = 1.0
        sunColor = SunsetColor.copy()
        sunObjectColor = SunsetObjectColor.copy()
        sunHaloObjectColor = SunsetHaloObjectColor.copy()
    else:
        if absSinSun > sinBegSunset:
            colorFull = FullSunColor.copy()
            sunSkyColor = colorFull.copy()
        else:
            sunset = 1 - absSinSun / sinBegSunset
            colorFull = FullSunColor + (SunsetColor - FullSunColor) * sunset
            sunSkyColor = colorFull + (SunsetSkyColor - colorFull) * sunset
        if absSinSun > sinSunSunset:
            sunObjectColor = SunObjectColor.copy()
            sunHaloObjectColor = SunHaloObjectColor.copy()
        else:
            sunset = 1 - absSinSun / sinSunSunset
            sunHaloObjectColor = SunHaloObjectColor + (SunsetHaloObjectColor - SunHaloObjectColor) * sunset
            sunObjectColor = SunObjectColor + (SunsetObjectColor - SunObjectColor) * sunset
        if absSinSun > sinNightAngle:
            nightEffect = 0.0
        else:
            nightEffect = 1 - absSinSun / sinNightAngle
        sunColor = colorFull.copy()

    ambientI = min(max(sinSun * 1.5, MIN_BACK_INTENSITY), 1.0)
    backgroundI = min(max(sinSun * 2.0, MIN_SKY_INTENSITY), 1.0)
    ambient = BackgroundColor * ambientI
    skyColor = BackgroundColor * backgroundI + colorFull * 0.5

    return {
        "sinSun": sinSun,
        "colorFull": colorFull,
        "sunColor": saturate(sunColor),
        "skyColor": saturate(skyColor),
        "sunSkyColor": saturate(sunSkyColor),
        "ambient": saturate(ambient),
        "sunObjectColor": sunObjectColor,
        "sunHaloObjectColor": sunHaloObjectColor,
        "nightEffect": nightEffect,
        # LightSun constructor: _diffuse = _colorFull until MainLightChanged
        "diffuse": saturate(colorFull),
    }


def accomodate_eye(brightness: float, night_vision: bool = False) -> np.ndarray:
    """Engine::InitDraw: _accomodateEye = (HWhite or NV green) * _usrBrightness, A = 1."""
    base = Color(0, 8.0, 0) if night_vision else HWhite.copy()
    return with_alpha(base * brightness, 1.0)


def rgb_eye_coef(night_eye: float) -> np.ndarray:
    """EngineGL33::EnableNightEye -> PS constant c7."""
    if night_eye > 0.01:
        return np.array([0.2, 0.9, 0.4, 1.0 - night_eye])
    return np.array([0.0, 0.0, 0.0, 1.0])


def compute_lighting(preset: LightingPreset, sky_texture_color: np.ndarray) -> SunLighting:
    """LightSun::Recalculate + Scene::MainLightChanged + Engine::InitDraw.

    sky_texture_color: Scene::_skyColor, i.e. the sky texture's pixel (1,1)
    (Scene::CalculateSkyColor). The scene module supplies it.
    """
    direction = sun_to_direction(preset.sun_elevation, preset.sun_azimuth)
    s = recalculate_sun(direction)
    accom = accomodate_eye(preset.brightness)

    # Scene::MainLightChanged
    diffuse = saturate(s["colorFull"] * (preset.sky_through * 0.8 + 0.2))
    lighting = saturate(s["skyColor"] * accom)
    fog = saturate(np.asarray(sky_texture_color, dtype=np.float64) * lighting)
    fog = with_alpha(fog, 1.0)

    # Landscape::DrawRect -> EnableNightEye
    night_eye = (1.0 - diffuse[0]) * s["nightEffect"]

    return SunLighting(
        preset=preset,
        direction=direction,
        to_sun=-direction,
        sin_sun=s["sinSun"],
        color_full=s["colorFull"],
        diffuse=diffuse,
        ambient=s["ambient"],
        sky_color=s["skyColor"],
        sun_sky_color=s["sunSkyColor"],
        sun_object_color=s["sunObjectColor"],
        sun_halo_object_color=s["sunHaloObjectColor"],
        night_effect=s["nightEffect"],
        night_eye=night_eye,
        accom=accom,
        sky_texture_color=np.asarray(sky_texture_color, dtype=np.float64),
        fog_color=fog,
        rgb_eye_coef=rgb_eye_coef(night_eye),
    )


# --------------------------------------------------------------------------
# Materials (TLMaterial) — TransLight.cpp CreateMaterial* + Material.cpp Combine
# --------------------------------------------------------------------------


@dataclass
class TLMaterial:
    emissive: np.ndarray
    ambient: np.ndarray
    diffuse: np.ndarray
    forced_diffuse: np.ndarray
    specular: np.ndarray
    specular_power: int = 0


def create_material_normal(col: np.ndarray) -> TLMaterial:
    """CreateMaterialNormal (vertex lighting type 0, the common case)."""
    return TLMaterial(HBlack.copy(), col.copy(), col.copy(), HBlack.copy(), HBlack.copy(), 0)


def create_material_shining(col: np.ndarray) -> TLMaterial:
    """CreateMaterialShining (MSShining)."""
    return TLMaterial(col.copy(), HBlack.copy(), HBlack.copy(), HBlack.copy(), HBlack.copy(), 0)


def create_material_constant(col: np.ndarray, factor: float) -> TLMaterial:
    """CreateMaterialConstant: MSInShadow (0), MSHalfLighted (0.5), MSFullLighted (1), MSInside (0.2)."""
    dif = with_alpha(col * factor * 0.5, col[3])
    return TLMaterial(HBlack.copy(), col.copy(), dif, col * factor * 0.5, HBlack.copy(), 0)


@dataclass
class MaterialRecipe:
    """Scene-side description of a material, resolved per frame against accom.

    kind:
      'normal'   CreateMaterialNormal(accom * color)
      'shining'  CreateMaterialShining(accom * color)
      'constant' CreateMaterialConstant(accom * color, factor)   (MS* vertex flags)
      'custom'   normal material plus forced_diffuse = accom * color * forced_diffuse
    specular / specular_power / emissive model a TexMaterial (CfgMaterials) that
    TexMaterial::Combine adds on top: specular += _specular * accom.
    """

    kind: str = "normal"
    color: tuple = (1.0, 1.0, 1.0, 1.0)
    factor: float = 0.0
    forced_diffuse: float = 0.0
    specular: tuple = (0.0, 0.0, 0.0, 1.0)
    specular_power: int = 0
    emissive: tuple = (0.0, 0.0, 0.0, 0.0)


def resolve_material(recipe: MaterialRecipe, accom: np.ndarray) -> TLMaterial:
    col = accom * np.asarray(recipe.color, dtype=np.float64)
    if recipe.kind == "normal":
        mat = create_material_normal(col)
    elif recipe.kind == "shining":
        mat = create_material_shining(col)
    elif recipe.kind == "constant":
        mat = create_material_constant(col, recipe.factor)
    elif recipe.kind == "custom":
        mat = create_material_normal(col)
        mat.forced_diffuse = with_alpha(col * recipe.forced_diffuse, 1.0)
    else:
        raise ValueError(f"unknown material kind {recipe.kind!r}")
    # TexMaterial::Combine (additive parts, scaled by accom)
    mat.specular = mat.specular + np.asarray(recipe.specular, dtype=np.float64) * accom
    mat.specular_power = int(mat.specular_power + recipe.specular_power)
    mat.emissive = mat.emissive + np.asarray(recipe.emissive, dtype=np.float64) * accom
    return mat


def vs_material_constants(sun: SunLighting, mat: TLMaterial) -> dict[str, np.ndarray]:
    """EngineGL33::UploadVSMaterialConstants values (before packing)."""
    dif = sun.diffuse * mat.diffuse
    amb = sun.ambient * mat.ambient + sun.diffuse * mat.forced_diffuse
    spec_col = sun.diffuse * mat.specular
    return {
        "ambient": amb,
        "diffuse": dif,
        "emissive": mat.emissive.copy(),
        "specular": np.array([spec_col[0], spec_col[1], spec_col[2], float(mat.specular_power)]),
        "specEn": np.array([1.0 if mat.specular_power > 0 else 0.0, 0.0, 0.0, 0.0]),
    }


# --------------------------------------------------------------------------
# CPU-lit (TL path) colours: sky dome and sun object
# --------------------------------------------------------------------------


def sky_vertex_colors(sun: SunLighting, normals: np.ndarray) -> np.ndarray:
    """TLVertexTable::DoSkyLighting, vectorised. Returns (N,4) float in [0,1].

    colorI = SunSkyColor * cosSun^6 + SkyColor; v.color = PackedColorRGB(colorI * accom, 255)
    where cosSun = max(0, -sunDir . normal) (sky normals point outward).
    """
    cos_sun = np.clip(normals @ (-sun.direction), 0.0, None)
    coef = cos_sun**6
    col = coef[:, None] * sun.sun_sky_color[None, :] + sun.sky_color[None, :]
    col = col * sun.accom[None, :]
    col = np.clip(col, 0.0, 1.0)
    col[:, 3] = 1.0
    return col


def sun_object_colors(sun: SunLighting) -> tuple[np.ndarray, np.ndarray]:
    """TLVertexTable lighting of the sun object (ClipLightSun / ClipLightSunHalo).

    colorI = Sun{,Halo}ObjectColor * accom(A=1); A *= SkyThrough; PackedColor clamps.
    """
    acc = with_alpha(sun.accom, 1.0)
    disc = sun.sun_object_color * acc
    halo = sun.sun_halo_object_color * acc
    disc[3] *= sun.preset.sky_through
    halo[3] *= sun.preset.sky_through
    return np.clip(disc, 0, 1), np.clip(halo, 0, 1)


def quantize8(c: np.ndarray) -> np.ndarray:
    """PackedColor round trip (toInt(x*255)/255) for colours the engine packs to 8 bit."""
    return np.round(np.clip(c, 0, 1) * 255.0) / 255.0
