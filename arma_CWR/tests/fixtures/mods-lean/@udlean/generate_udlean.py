"""Rebuild an original, untextured thin wall for infantry collision tests.

Uses the existing synthetic island fixture writer; no game assets are copied.
Run before the leaning_cover integration lane.
"""
from pathlib import Path
import runpy
import struct

ROOT = Path(__file__).resolve().parent
writer = runpy.run_path(str(ROOT.parents[1] / "mods-island/@udisland/generate_udisland.py"))
box_lod, fixed = writer["box_lod"], writer["fixed"]


def geometry(resolution, thickness=0.1):
    lod = box_lod(thickness, 3.0, 4.0, [], resolution)
    # One convex component, all eight points and six faces.
    component = fixed("component01", 64) + struct.pack("<I", 14) + bytes([1] * 14)
    component += fixed("#Mass#", 64) + struct.pack("<I8f", 32, *([1000.0] * 8))
    return lod.replace(fixed("#EndOfFile#", 64), component + fixed("#EndOfFile#", 64))


config = r'''
class CfgPatches { class UDLeanFixture { units[]={"UDLeanWall","UDLeanThinWall"}; weapons[]={}; requiredVersion=1.99; }; };
class CfgVehicles {
    class All {}; class Static: All {}; class NonStrategic: Static {}; class House: NonStrategic {};
    class UDLeanWall: House {
        scope=2; model="\udlean\wall"; displayName="Lean test wall"; armor=100;
    };
    class UDLeanThinWall: UDLeanWall {
        model="\udlean\thinwall"; displayName="Lean barrel test barrier";
    };
};
'''
model = writer["mlod_file"]([
    box_lod(0.1, 3.0, 4.0, [("class", "house")], 1.0),
    geometry(1.0e13), geometry(7.0e15),
])
# A passable 1cm bullet barrier isolates the eye-to-muzzle shot-origin guard.
# Omitting body Geometry avoids inserting a solid wall through the standing
# character. The solid 10cm wall above still tests body/lean collision.
thin_model = writer["mlod_file"]([
    box_lod(0.01, 3.0, 4.0, [("class", "house")], 1.0),
    geometry(7.0e15, 0.01),
])
out = ROOT / "addons/udlean.pbo"
out.parent.mkdir(parents=True, exist_ok=True)
out.write_bytes(writer["build_pbo"]([
    ("config.cpp", config.encode("ascii")), ("wall.p3d", model), ("thinwall.p3d", thin_model),
]))
print(out)
