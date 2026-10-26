"""BEST_*.png sheets for the gbuffer track (from captures already on disk).

    python make_best.py
"""
from pathlib import Path

from PIL import Image, ImageDraw

import collect_gb as cg

B = Path(__file__).resolve().parent


def rows_of(run):
    rows, _ = cg.read_rows(B / run / "tri")
    return rows


def panel_sheet(dst, title, panels, scale=1.0, crop=None, cols=None, footer=None):
    """panels: list of (run, view, label, caption)."""
    ims = []
    for run, view, label, cap in panels:
        im = Image.open(B / run / f"{view}__{label}.png").convert("RGB")
        if crop:
            im = im.crop(crop)
        if scale != 1.0:
            im = im.resize((int(im.width * scale), int(im.height * scale)), Image.LANCZOS)
        r = rows_of(run).get((view, label))
        ims.append((im, cap, r, run))
    cols = cols or len(ims)
    rows_n = (len(ims) + cols - 1) // cols
    w, h = ims[0][0].size
    gap, top = 8, 44
    foot = 34 if footer else 0
    out = Image.new("RGB", (cols * w + (cols - 1) * gap, top + rows_n * h + (rows_n - 1) * gap + foot), (24, 24, 24))
    d = ImageDraw.Draw(out)
    d.text((10, 10), title, font=cg.font(22), fill=(235, 235, 235))
    for i, (im, cap, r, run) in enumerate(ims):
        rr, cc = divmod(i, cols)
        x0, y0 = cc * (w + gap), top + rr * (h + gap)
        out.paste(im, (x0, y0))
        cg.label_box(d, (x0 + 10, y0 + 8), cap, cg.font(20))
        if r and r.get("frame") is not None:
            txt = f"GPU frame {r['frame']:.2f} ms | world {r['world'] or 0:.2f} | post {r['post'] or 0:.2f} (ao/gi {r['ao'] or 0:.2f})  [{run}]"
        else:
            txt = f"[{run}]"
        cg.label_box(d, (x0 + 10, y0 + h - 26), txt, cg.font(14))
    if footer:
        d.text((10, out.height - foot + 8), footer, font=cg.font(16), fill=(210, 210, 210))
    out.save(B / dst)
    print(dst, out.size)


def main():
    note = ("GPU ms are timestamp-query averages on a GPU shared with up to three other research games; "
            "world-pass numbers swing up to 2x between runs. 960x640 window, 1.5 render scale, 4x MSAA, RTX 4060 Ti.")
    panel_sheet("BEST_town_day.png",
                "town_day 14:00: Classic | Remastered (current) | Remastered + late VBO + G-buffer one-bounce GI (r 15 m, x3, whole-frame source, bounce only)",
                [("run06_v2", "town_day", "classic", "Classic"),
                 ("run06_v2", "town_day", "remastered", "Remastered (current)"),
                 ("run07_v3", "town_day", "v3r15_x3s_noao", "+ late VBO + G-buffer GI (strongest variant)")],
                footer=note)
    panel_sheet("BEST_town_day_crop.png",
                "town_day crop x2: soldier and Jeep shaded sides. The bounce is the faint warm-green lift on the Jeep's rear and the soldier's legs",
                [("run06_v2", "town_day", "classic", "Classic"),
                 ("run06_v2", "town_day", "remastered", "Remastered (current)"),
                 ("run07_v3", "town_day", "rm_vbo", "+ late VBO"),
                 ("run07_v3", "town_day", "v3r15_x3s_noao", "+ G-buffer GI r15 x3")],
                crop=(230, 320, 700, 520), scale=2.0, cols=2, footer=note)
    panel_sheet("BEST_lowsun_fp.png",
                "lowsun_fp 08:00: Classic | Remastered (current) | Remastered + late VBO + G-buffer GI (r 15 m, x3, whole-frame source, bounce only)",
                [("run06_v2", "lowsun_fp", "classic", "Classic"),
                 ("run06_v2", "lowsun_fp", "remastered", "Remastered (current)"),
                 ("run07_v3", "lowsun_fp", "v3r15_x3s_noao", "+ late VBO + G-buffer GI (strongest variant)")],
                footer=note)
    panel_sheet("BEST_lowsun_shadow.png",
                "Script-spawned vehicles skip the Remastered shader in the stock build. Sun-visibility debug view (debug 1): white = sunlit, black = shadowed, textured = never reached the lit shader",
                [("run09_stock", "lowsun_shadow", "stock_sundbg", "STOCK build (dist/lighting-rwdi): spawned M113 + Jeeps pre-lit"),
                 ("run08_shadow", "lowsun_shadow", "sundbg_vbo", "gbuffer build, late VBO: lit per pixel"),
                 ("run11_pre", "lowsun_shadow", "pre_sundbg", "same classes preloaded by the mission (normal VBO)"),
                 ("run09_stock", "lowsun_shadow", "stock_remastered", "STOCK final frame"),
                 ("run08_shadow", "lowsun_shadow", "rm_vbo", "late VBO final frame"),
                 ("run11_pre", "lowsun_shadow", "pre_remastered", "preloaded final frame")],
                scale=0.75, cols=3, footer=note)
    panel_sheet("BEST_forest_edge_golden.png",
                "G-buffer targets on real data (run05): crown normals on trees and the LoBo camo net, per-pixel vehicle normals, non-sky radiance",
                [("run05_main", "forest_edge_golden", "gi", "final frame (+ GI v1)"),
                 ("run05_main", "forest_edge_golden", "dbg_normals", "view-space normals (crowns = ellipsoid normals)"),
                 ("run05_main", "forest_edge_golden", "dbg_direct", "non-sky radiance (sun, moon, local)"),
                 ("run05_main", "town_day", "dbg_normals", "town_day normals (buildings, Jeep, birch)"),
                 ("run05_main", "lowsun_fp", "dbg_normals", "lowsun_fp normals (late VBO vehicles)"),
                 ("run05_main", "sinai_lowsun_fp", "dbg_normals", "Sinai + LoBo normals (camo net on crown path)")],
                scale=0.75, cols=3)


if __name__ == "__main__":
    main()
