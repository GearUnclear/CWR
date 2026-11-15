"""Export the chupacabra blend to the SP3X MLOD the udchupa addon packs.

Headless, Blender 4.3:

  blender --background <art>/model/chupacabra.blend --python export_udchupa_p3d.py -- <out.p3d> [<summary.json>]

Loads the io_import_p3d add-on from this checkout (not an installed copy) and
writes collection "chupa" with the default options. The blend's material
property p3d_texture says "chupa\\chupa_co.paa"; the addon pbo is named
udchupa, and a pbo's file name is its path prefix, so the face texture is
retargeted to "udchupa\\chupa_co.paa" in memory before the export. The blend
file itself is not saved.
"""
import json
import os
import sys

import bpy

HERE = os.path.dirname(os.path.abspath(__file__))
# <repo>/arma_CWR/experiments/cryptid/addon -> <repo>/arma_CWR
ENGINE_ROOT = os.path.normpath(os.path.join(HERE, "..", "..", ".."))
ADDON_DIR = os.path.join(ENGINE_ROOT, "apps", "tools", "BlenderAddon")

OLD_TEXTURE = "chupa\\chupa_co.paa"
NEW_TEXTURE = "udchupa\\chupa_co.paa"

argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
if not argv:
    raise SystemExit("usage: blender -b chupacabra.blend --python export_udchupa_p3d.py -- <out.p3d> [summary.json]")
out_p3d = os.path.abspath(argv[0])
out_json = os.path.abspath(argv[1]) if len(argv) > 1 else None

sys.path.insert(0, ADDON_DIR)
import io_import_p3d as addon  # noqa: E402

addon.register()
from io_import_p3d.export_p3d import export_p3d  # noqa: E402

retargeted = []
for mat in bpy.data.materials:
    tex = mat.get("p3d_texture")
    if tex is not None and str(tex).replace("/", "\\").lower() == OLD_TEXTURE:
        mat["p3d_texture"] = NEW_TEXTURE
        retargeted.append(mat.name)
print("[udchupa] texture retargeted on materials:", retargeted, flush=True)

os.makedirs(os.path.dirname(out_p3d), exist_ok=True)
summary = export_p3d(out_p3d, "chupa")
summary["udchupa_retargeted_materials"] = retargeted
if out_json:
    with open(out_json, "w") as fh:
        json.dump(summary, fh, indent=1, default=str)
print("[udchupa] wrote", out_p3d, flush=True)
print("[udchupa] warnings:", summary.get("warnings"), flush=True)
