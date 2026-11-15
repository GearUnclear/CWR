# Texturing and exporting P3D models: check them in game

Verified 2026-11-07 in the `cryptid` worktree (detached at `5696d7d2`, changes
uncommitted) with the chupacabra model (`UDChupacabra`, addon `udchupa`) and the
stock civilian. Paths are relative to `arma_CWR/` unless they start with a drive.

## What went wrong

The chupacabra looked fine in Blender and in its Blender preview renders, yet
in game:

- the glowing eyes never showed (no orange pixel in 14 close-up frames);
- from behind, the back of the head showed a "face": the red mouth and the
  eye sockets;
- the body looked pale and oddly lit.

The texture was not at fault. The PAA decoded to the painted PNG, and the eye's
UV island was in the right place.

## Why

The whole model was exported inside out. The engine draws only front faces, so
it culled every face pointing at the camera and drew the inside of the far wall.
A symmetric body hides that from the front. From behind you look at the inside
of the face, and the eye discs, which point outward, are culled.

The cause was a winding bug on both sides of the Blender add-on:

- Blender to game swaps two axes (`_to_game`,
  `apps/tools/BlenderAddon/io_import_p3d/export_p3d.py:56`, and
  `_convert_coord`, `import_p3d.py:13`). An axis swap is a mirror, so each
  face's corner order has to be reversed as well. Neither side did it.
- The importer therefore brought the stock civilian into Blender inside out
  (observed: 248 of 261 head faces pointing inward), and the exporter turned it
  back. The round trip of the stock model stayed exact, so the round-trip test
  could not see the bug. Anything modelled directly in Blender came out inside
  out.
- Blender's viewport and renders draw both sides of a face by default, so the
  model looked correct there.

Fixed (uncommitted) by reversing corners once on each side:
`import_p3d.py:241` on the way in, and `export_p3d.py:323` on the way out.
Proxy triangles keep their order, because the engine reads their placement
from the point order. Observed after the fix: an import and re-export of the
stock civilian gives the same points and the same corner order as before the
fix in all 11 LODs. The chupacabra now winds like the stock model, and in game
its eyes glow and its back is solid.

## What to do

1. **Turn on Backface Culling in Blender** (Viewport Shading > Options) while
   modelling and painting. An inside-out mesh then looks hollow at once.
2. **Compare winding with a stock model, not only with a round trip.** For a
   P3D written by the exporter, read it with `mlod_writer.read_mlod` and, for
   the faces of one selection such as `hlava`, test
   `cross(p1 - p0, p2 - p0) . (centroid - selection_center)` with the corners
   in reversed file order. The stock civilian gives `+` for about 95% of its
   faces (247 of 262 on the head). An inside-out export gives mostly `-`.
3. **Look at it in game, close up, before calling the texture done.** Use
   `experiments/cryptid/cryptid_texture_shot.test.{sqf,toml}`: 14 views at
   1600x1200 with no HUD in soft daylight: a full orbit, close-ups of the head
   and claws, and two prone frames. Run it with `--render gl33` on a quiet
   machine, through the hidden-desktop wrapper. Copy its camera setup for
   other models; the rules come from `tools/screenshots/README.md`.
4. **Check the parts that only show from one side:** eyes, the inside of the
   mouth, and the back of the head. Look for features showing through from the
   other side, such as a mouth seen from behind.
5. **Paint glow into the colour itself.** The engine has no emissive maps, so
   a Blender emission shader shows only in Blender renders. A bright painted
   eye island is what reads in game.

## Not checked

- Whether other models made with the add-on before this fix are affected.
  Only models built directly in Blender are; imported and re-exported stock
  models are not.
- A Blender-side regression test. `tests/ci/test_p3d_mlod_writer.py` covers the
  writer only and needs no Blender, so it cannot see this bug.
