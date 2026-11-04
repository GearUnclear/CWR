"""Lighting lab CLI: render the synthetic scene through a renderer module.

  python run_lab.py --out DIR [--width 960 --height 540] [--source PATH_OR_REV]
                    [--renderer classic|<module>|path/to/module.py]
                    [--camera ground,overview,closeup] [--lighting noon,afternoon,golden,overcast]

Writes one PNG per camera x lighting preset (<camera>_<lighting>.png), a
labelled contact_sheet.png, and manifest.json (timings, image stats, the
lighting values and per-mesh VS constants the renderer reported).

A renderer is any module exposing
    render(ctx, scene, preset, width, height) -> PIL.Image
and optionally
    configure(source=..., **options)
    NAME = "..."
"""

from __future__ import annotations

import argparse
import importlib
import importlib.util
import json
import sys
import time
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

LAB_DIR = Path(__file__).resolve().parent
if str(LAB_DIR) not in sys.path:
    sys.path.insert(0, str(LAB_DIR))

import gl_common as G  # noqa: E402
import glsl_extract  # noqa: E402
from presets import CAMERA_ORDER, LIGHTING_ORDER, make_presets  # noqa: E402
from scene import build_scene  # noqa: E402


def load_renderer(spec: str):
    p = Path(spec)
    if p.suffix == ".py" and p.exists():
        modspec = importlib.util.spec_from_file_location(p.stem, p)
        mod = importlib.util.module_from_spec(modspec)
        modspec.loader.exec_module(mod)
        return mod
    return importlib.import_module(spec)


def image_stats(img: Image.Image) -> dict:
    a = np.asarray(img.convert("RGB"), dtype=np.float64) / 255.0
    lum = a @ np.array([0.299, 0.587, 0.114])
    return {
        "mean_rgb": [round(float(x), 4) for x in a.reshape(-1, 3).mean(0)],
        "mean_luma": round(float(lum.mean()), 4),
        "std_luma": round(float(lum.std()), 4),
        "black_frac": round(float(np.mean(a.max(axis=2) < 1.5 / 255)), 5),
        "white_frac": round(float(np.mean(a.min(axis=2) > 253.5 / 255)), 5),
        "clipped_channel_frac": round(float(np.mean(a > 253.5 / 255)), 5),
    }


def _font(size: int):
    for name in ("DejaVuSans.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "LiberationSans-Regular.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


def label_image(img: Image.Image, text: str, size: int = 18) -> Image.Image:
    img = img.copy()
    d = ImageDraw.Draw(img)
    font = _font(size)
    bbox = d.textbbox((0, 0), text, font=font)
    w, h = bbox[2] - bbox[0], bbox[3] - bbox[1]
    d.rectangle([0, 0, w + 12, h + 10], fill=(0, 0, 0))
    d.text((6, 4 - bbox[1]), text, fill=(255, 255, 255), font=font)
    return img


def contact_sheet(images: list[tuple[str, Image.Image]], cols: int, tile_w: int = 480, title: str = "") -> Image.Image:
    if not images:
        raise ValueError("no images")
    w0, h0 = images[0][1].size
    tile_h = round(tile_w * h0 / w0)
    rows = (len(images) + cols - 1) // cols
    head = 34 if title else 0
    sheet = Image.new("RGB", (cols * tile_w, rows * tile_h + head), (24, 24, 24))
    if title:
        d = ImageDraw.Draw(sheet)
        d.text((8, 6), title, fill=(230, 230, 230), font=_font(18))
    for i, (name, im) in enumerate(images):
        t = label_image(im.resize((tile_w, tile_h), Image.LANCZOS), name, 15)
        sheet.paste(t, ((i % cols) * tile_w, head + (i // cols) * tile_h))
    return sheet


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", required=True, help="output directory")
    ap.add_argument("--width", type=int, default=960)
    ap.add_argument("--height", type=int, default=540)
    ap.add_argument("--source", default=None, help="shader C++ source: path, git rev or rev:path (default: worktree)")
    ap.add_argument("--renderer", default="classic", help="renderer module name or .py path (default classic)")
    ap.add_argument("--camera", default=",".join(CAMERA_ORDER), help="comma list of camera presets")
    ap.add_argument("--lighting", default=",".join(LIGHTING_ORDER), help="comma list of lighting presets")
    ap.add_argument("--shader-override-dir", default=None,
                    help="directory of <name>.glsl overrides (engine CWR_SHADER_OVERRIDE_DIR convention)")
    ap.add_argument("--no-nan-check", action="store_true", help="skip the RGBA32F non-finite pixel check")
    ap.add_argument("--seed", type=int, default=1, help="scene seed")
    ap.add_argument("--no-sheet", action="store_true", help="skip contact_sheet.png")
    ap.add_argument("--tuning", default="",
                    help="remaster renderer: comma list of key=value LightingTuning overrides (e.g. sun=1.1,bloom=0)")
    ap.add_argument("--debug-view", type=int, default=0,
                    help="remaster renderer: 1 sun visibility, 2 sky visibility, 3 normals, 4 ambient occlusion")
    ap.add_argument("--no-post", action="store_true", help="remaster renderer: skip the post pass")
    ap.add_argument("--no-shadows", action="store_true", help="remaster renderer: skip the shadow maps")
    args = ap.parse_args(argv)
    tuning = {}
    for kv in (x for x in args.tuning.split(",") if x):
        k, v = kv.split("=", 1)
        tuning[k.strip()] = float(v)

    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    t0 = time.perf_counter()
    renderer = load_renderer(args.renderer)
    if hasattr(renderer, "configure"):
        renderer.configure(source=args.source, shader_override_dir=args.shader_override_dir,
                           nan_check=not args.no_nan_check, tuning=tuning or None, debug_view=args.debug_view,
                           post=not args.no_post, shadows=not args.no_shadows)
    scene = build_scene(args.seed)
    t_scene = time.perf_counter() - t0
    ctx = G.create_context()
    print(f"GL: {ctx.info['GL_RENDERER']} | {ctx.info['GL_VERSION']}")
    print(f"renderer: {getattr(renderer, 'NAME', args.renderer)} | shaders: {glsl_extract.describe_source(args.source)}")
    print(f"scene: {scene.stats()} built in {t_scene:.2f}s")

    presets = make_presets(scene, [c for c in args.camera.split(",") if c], [l for l in args.lighting.split(",") if l])
    manifest = {
        "renderer": getattr(renderer, "NAME", args.renderer),
        "shader_source": glsl_extract.describe_source(args.source),
        "shader_override_dir": args.shader_override_dir,
        "size": [args.width, args.height],
        "seed": args.seed,
        "gl_renderer": ctx.info["GL_RENDERER"],
        "images": {},
    }
    rendered: list[tuple[str, Image.Image]] = []
    bad: list[str] = []
    total = 0.0
    first = True
    for p in presets:
        t1 = time.perf_counter()
        img = renderer.render(ctx, scene, p, args.width, args.height)
        dt = time.perf_counter() - t1
        total += dt
        path = out / f"{p.name}.png"
        img.save(path)
        st = image_stats(img)
        note = " (incl. GPU resource setup)" if first else ""
        first = False
        info = dict(img.info.get("lab", {}))
        nf = info.get("nonfinite_values")
        nf_txt = "" if nf is None else f"  nonfinite {nf}"
        print(f"  {p.name:22s} {dt * 1000:8.1f} ms{note}  luma {st['mean_luma']:.3f}  black {st['black_frac']:.4f}  "
              f"clipped {st['clipped_channel_frac']:.4f}{nf_txt}")
        if nf:
            bad.append(p.name)
        manifest["images"][p.name] = {"file": path.name, "ms": round(dt * 1000, 1), "stats": st,
                                      "camera": p.camera.name, "lighting": p.lighting.name, **info}
        rendered.append((p.name, img))
    print(f"total render {total:.2f}s for {len(presets)} images")

    if not args.no_sheet and rendered:
        cams = list(dict.fromkeys(p.camera.name for p in presets))
        lits = list(dict.fromkeys(p.lighting.name for p in presets))
        cols = len(lits) if len(cams) > 1 else min(len(rendered), 4)
        title = f"{manifest['renderer']} | {manifest['shader_source']} | {args.width}x{args.height}"
        contact_sheet(rendered, cols, title=title).save(out / "contact_sheet.png")
        print(f"wrote {out / 'contact_sheet.png'}")
    (out / "manifest.json").write_text(json.dumps(manifest, indent=1))
    if bad:
        print(f"ERROR: non-finite pixel values in {bad}")
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
