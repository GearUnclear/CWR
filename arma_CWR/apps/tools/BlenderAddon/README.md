# Poseidon P3D Importer for Blender

Blender addon for importing Arma: Cold War Assault - Remastered P3D models.
Supports both MLOD (source) and ODOL v7 (binarized) formats with textures, selections, and proxies.

## Quick Install

### Option A: Install from ZIP

1. Build the DLL (if not already):
   ```
   cmake --build build\win-x64-clang-rwdi --target PoseidonFormats_shared
   ```

2. Package the addon:
   ```
   powershell -File apps\tools\BlenderAddon\package.ps1
   ```

3. In Blender: **Edit > Preferences > Add-ons > Install from Disk** > select `dist\io_import_p3d.zip`

4. Enable the addon: search for **"Poseidon P3D"** and tick the checkbox

### Option B: Manual / Development Install

1. Copy (or symlink) the `apps\tools\BlenderAddon\io_import_p3d` folder into your Blender addons directory:
   ```
   %APPDATA%\Blender Foundation\Blender\5.0\scripts\addons\io_import_p3d
   ```

2. Copy `dist\x64-win-rwdi\PoseidonFormats.dll` into that same `io_import_p3d` folder

3. Enable in Blender: **Edit > Preferences > Add-ons** > search **"Poseidon P3D"**

### Symlink for Development

To avoid copying files after each change, create a symlink (run as Administrator):
```
mklink /D "%APPDATA%\Blender Foundation\Blender\5.0\scripts\addons\io_import_p3d" "F:\ofpisnotdead.com\sources\CWA\OFPR\apps\tools\BlenderAddon\io_import_p3d"
```
Then just copy the DLL once into the addon folder:
```
copy dist\x64-win-rwdi\PoseidonFormats.dll apps\tools\BlenderAddon\io_import_p3d\
```

## Usage

1. **File > Import > Poseidon P3D (.p3d)**
2. Browse to a `.p3d` file (from game data or unpacked PBOs)
3. Set import options in the sidebar panel:

| Option | Default | Description |
|--------|---------|-------------|
| LODs | First Only | All / First Only / Visual Only (skip shadow/geo LODs) |
| Max LOD Resolution | 100000 | Skip LODs above this distance threshold |
| Materials | On | Create materials with PAA textures |
| Normals | On | Import custom split normals |
| Named Selections | On | Import as vertex groups |
| Proxies | On | Import as empty objects |
| Properties | On | Import as custom object properties |
| Texture Directory | (empty) | Extra folder to search for PAA/PAC textures |
| Model Space | Off | Keep the model's own coordinates (ODOL: plus its bounding center) instead of re-centring each LOD; use this for a reference you model new units over |

## What Gets Imported

- **Mesh**: Vertices, faces, UVs (V-flipped), normals, coordinate-converted (Y-up to Z-up)
- **Materials**: One per unique texture, with Principled BSDF node and PAA texture if found
- **Vertex Groups**: From named selections (e.g., "velka vrtule", "pravy volant") with weights
- **Empties**: From proxy attachment points with correct transforms
- **Custom Properties**: `ofp_lod_index`, `ofp_lod_resolution`, `ofp_format`, model properties
- **Collections**: Each LOD in its own sub-collection (when importing multiple LODs)

## Export (MLOD)

**File > Export > Poseidon MLOD P3D (.p3d)** writes the OFP-era MLOD format
(`MLOD` header, `SP3X` LODs, version 1.01) that the engine loads at runtime. The
headless entry point is `io_import_p3d.export_p3d.export_p3d(filepath, collection)`;
the byte layout is in `mlod_writer.py`, which needs no Blender
(tests: `tests/ci/test_p3d_mlod_writer.py`).

- One collection per LOD. Resolution from the collection property
  `p3d_resolution`, the importer's `cwr_lod_resolution`, or the collection name
  (`0.15`, `1.0`, `Geometry`, `Memory`, `LandContact`, `HitPoints`,
  `FireGeometry`, `ViewGeometry`, `ViewPilot`, ...). There is no shadow-volume LOD
  in this engine.
- Vertex groups become named selections; material `p3d_texture` (or the
  importer's `cwr_texture`, or the image file name plus the Texture Prefix) is the
  face texture, 31 characters at most.
- `p3d_prop_<name>` custom properties on a LOD collection or object become
  `#Property#` tags; `p3d_mass` (collection, object, or a float point attribute)
  becomes `#Mass#` on the Geometry LOD.
- Proxies: objects with `p3d_proxy` (or the importer's `cwr_proxy_path`), or
  named `proxy:...`.
- An imported model exports back to its source coordinates: the importer's
  re-centring (`cwr_center_offset`) and an ODOL's bounding center
  (`cwr_bounding_center`) are added back.

## RTM animation

**File > Import / Export > Poseidon RTM Animation (.rtm)** and the sidebar panel
**Poseidon > RTM Animation** (Build Stock Man Rig, Bind Selected Meshes to Rig,
Import RTM, Export RTM). Headless entry points live in `rtm_blender.py`:
`build_rig(rest_json)`, `bind_mesh(rig, meshes)`, `import_rtm(rig, path)`,
`export_rtm(path, rig, action=None, step=None)`.

- `rtm_io.py` reads and writes RTM exactly as `Asset/Formats/RTM/RTMReader.hpp`
  and `AnimationRT::Load` do (no bpy); `read_rtm(..., strict=True)` also
  rejects what the engine would misplay (no phases, duplicate bones, unsorted
  times, mirrored matrices).
- `rtm_engine.py` reimplements the engine's playback (`Find`,
  `PrepareMatrices`, `RemoveLoopFrame`, weight quantization, skinning, the
  `reversed` load) for checks; `rtm_rig.py` holds the stock man hierarchy and
  derives the joint pivots from stock RTMs (`derive_rest_joints_from_dir`,
  written to a local JSON because it is Bohemia-derived data).
- Matrices are absolute and act on the model's file space; Blender (x, y, z)
  is game (x, z, y), so every matrix is conjugated by that swap, never copied.
- Tests: `python -m unittest discover -s arma_CWR/tests/ci -p "test_rtm_*.py"`
  (synthetic data; set `UD_RTM_STOCK_DIR` to also round-trip local stock RTMs).

## Texture Resolution

The addon searches for PAA/PAC textures in this order:
1. Same directory as the P3D file
2. Game root directory (auto-detected by looking for `dta/` or `Addons/` folders)
3. Custom texture directory (if specified in import options)

For best results, point the Texture Directory to your game installation root.

## Troubleshooting

**"PoseidonFormats.dll not found"**: The DLL must be in the addon folder or set `POSEIDON_FORMATS_DLL` env var.

**No textures loaded**: Set the Texture Directory to your game root. Textures use relative paths like `data\uh60_infra_side.pac`.

**Blender crashes on import**: Check Blender's System Console (Window > Toggle System Console) for error messages.

## Architecture

```
PoseidonFormats.dll    C++ engine (P3D/PAA/PBO readers)
       |
   ctypes (C API)
       |
   p3d_lib.py          Python wrapper (P3DModel, PAAImage classes)
       |
   import_p3d.py       Blender import operator
   export_p3d.py       Blender MLOD export operator
   mlod_writer.py      MLOD (SP3X) writer and strict reader, no bpy
   rtm_blender.py      RTM rig, import/export operators and panel
   rtm_io.py           RTM reader/writer and strict checks, no bpy
   rtm_engine.py       engine RTM playback math for checks, no bpy
   rtm_rig.py          stock man hierarchy and pivot derivation, no bpy
       |
   __init__.py          Addon registration
```
