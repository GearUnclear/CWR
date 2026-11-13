"""The model finishing round's two sheets: before and after, in Blender and in game, a caption on every frame.

  python make_sheets.py a|d [--notes <kaiju-notes folder>]

Reads: finishing/models/motion (Blender frames, before and after), features/finals/shots and
integration/shots (the game before), finishing/models/runs (the game after). Writes
finishing/models/sheet_<letter>.png and sheet_<letter>_CAPTIONS.txt. A frame that is not there is drawn
as a grey panel that says so: the sheet never pretends.
"""
import argparse
import glob
import os

from PIL import Image, ImageDraw, ImageFont

BG = (20, 21, 24)


def font(size):
    for name in ("segoeui.ttf", "arial.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


def wrap(draw, text, fnt, width):
    words, lines, cur = text.split(), [], ""
    for w in words:
        t = (cur + " " + w).strip()
        if draw.textlength(t, font=fnt) <= width:
            cur = t
        else:
            if cur:
                lines.append(cur)
            cur = w
    if cur:
        lines.append(cur)
    return lines


def find(pattern):
    hits = sorted(glob.glob(pattern, recursive=True))
    return hits[-1] if hits else None


def panel(path, w, h, crop=None):
    if path is None or not os.path.isfile(path):
        im = Image.new("RGB", (w, h), (60, 60, 64))
        ImageDraw.Draw(im).text((10, h // 2 - 8), "frame not made", fill=(230, 230, 230), font=font(15))
        return im, False
    im = Image.open(path).convert("RGB")
    if crop:
        W, H = im.size
        im = im.crop((int(crop[0] * W), int(crop[1] * H), int(crop[2] * W), int(crop[3] * H)))
    # fit, keeping the aspect, on a dark panel
    r = min(w / im.size[0], h / im.size[1])
    im = im.resize((max(1, int(im.size[0] * r)), max(1, int(im.size[1] * r))), Image.LANCZOS)
    out = Image.new("RGB", (w, h), (8, 8, 10))
    out.paste(im, ((w - im.size[0]) // 2, (h - im.size[1]) // 2))
    return out, True


def sheet(rows, out_png, title, sub, cell_w=384, cap_lines=3):
    f_cap, f_row, f_title = font(13), font(17), font(24)
    probe = ImageDraw.Draw(Image.new("RGB", (8, 8)))
    cap_h = 17 * cap_lines + 6
    ncol = max(len(r[2]) for r in rows)
    width = ncol * cell_w + 16
    y = 70
    layout = []
    for row_title, cell_h, cells in rows:
        layout.append((y, row_title, cell_h, cells))
        y += 30 + cell_h + cap_h + 10
    img = Image.new("RGB", (width, y + 10), BG)
    d = ImageDraw.Draw(img)
    d.text((10, 8), title, fill=(240, 240, 240), font=f_title)
    for li, line in enumerate(wrap(probe, sub, f_cap, width - 20)[:2]):
        d.text((10, 38 + 15 * li), line, fill=(190, 190, 190), font=f_cap)
    captions, missing = [], 0
    for y0, row_title, cell_h, cells in layout:
        d.text((10, y0 + 4), row_title, fill=(235, 200, 120), font=f_row)
        for k, cell in enumerate(cells):
            path, cap = cell[0], cell[1]
            crop = cell[2] if len(cell) > 2 else None
            im, ok = panel(path, cell_w - 6, cell_h, crop)
            missing += 0 if ok else 1
            x = 8 + k * cell_w
            img.paste(im, (x, y0 + 30))
            for li, line in enumerate(wrap(probe, cap, f_cap, cell_w - 12)[:cap_lines]):
                d.text((x + 2, y0 + 32 + cell_h + 17 * li), line, fill=(215, 215, 215), font=f_cap)
            captions.append("%s | %s | %s%s" % (row_title, cap, path or "(none)", "" if ok else "  [FRAME NOT MADE]"))
    img.save(out_png)
    with open(os.path.splitext(out_png)[0] + "_CAPTIONS.txt", "w", encoding="utf-8") as fh:
        fh.write(title + "\n" + sub + "\n\n" + "\n".join(captions) + "\n")
    print("wrote %s %s, %d frames, %d missing" % (out_png, img.size, len(captions), missing))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("tag", choices=["a", "d"])
    ap.add_argument("--notes", default=r"D:\Arma_CWA_worktrees\kaiju-notes")
    a = ap.parse_args()
    t = a.tag
    N = a.notes
    M = os.path.join(N, "finishing", "models")
    mo = os.path.join(M, "motion")
    runs = os.path.join(M, "runs")
    fin = os.path.join(N, "features", "finals", "shots", "%s_anim1" % t)
    ish = os.path.join(N, "integration", "shots")

    def bl(which, move, cam, k):
        return os.path.join(mo, "%s_%s" % (t, which), "%s_%s_%02d.png" % (move, cam, k))

    def game(tag, name):
        return find(os.path.join(runs, "%s*" % tag, "**", "*%s.png" % name))

    low = (0.10, 0.36, 0.86, 1.0)          # the lower body of a 512 frame
    rows = []
    if t == "a":
        title = "Dedushka A, the Starved Bear (UDDedushka): the finishing round, before and after"
        sub = ("LOD0 15716 triangles as before (weights, hit and collision shapes, memory points changed; no vertex moved). "
               "Top rows: Blender 4.3, the built P3D posed with the game's own clips. Lower rows: in game, kaiju binary, the same cameras before and after.")
        rows.append(("1. Walk in Blender, front right quarter: BEFORE (round 3 build, the integration's clips)", 300,
                     [(bl("before", "walk", "quarter", k), "Before, walk phase %d of 8. %s" % (k, c), low) for k, c in
                      ((1, "The right hand's claws are inside the right knee (7 vertices, half a metre deep)."),
                       (2, "The claws drag through the shin as the leg comes forward."),
                       (3, "Still touching: the hand hangs where the leg swings."),
                       (5, "Left side: the hand is clear, the claws 0.25 m off the ground."))]))
        rows.append(("2. The same four phases AFTER: arms carried 3 and 5 degrees out and 5 further back as the leg of that side comes forward", 300,
                     [(bl("after", "walk", "quarter", k), "After, walk phase %d of 8. %s" % (k, c), low) for k, c in
                      ((1, "No vertex of either hand is inside a leg or the pelvis in any of the 12 moves."),
                       (2, "The hand hangs behind and outside the knee."),
                       (3, "Heel peels 19 degrees before toe-off; the ball of the foot bends over 1.7 m."),
                       (5, "Lowest claw point over all moves 0.27 m (was 0.36 m under the ground in the dormant pose)."))]))
        rows.append(("3. Waist and feet in Blender: before (left pair) and after (right pair)", 300,
                     [(bl("before_close", "walk", "waist", 2), "Before, the waist from in front: the spine tube was one rigid piece of zebra between 20.5 and 26.5 m.", (0.2, 0.25, 0.8, 1.0)),
                      (bl("before_close", "walk", "feet", 3), "Before, low at the right foot at heel-off: the foot bent at one ring."),
                      (bl("after_close", "walk", "waist", 2), "After: pelvis, waist and chest blend over 5 m each way, so the column bends like a spine.", (0.2, 0.25, 0.8, 1.0)),
                      (bl("after_close", "walk", "feet", 3), "After: the toes hang 9 degrees in the swing and the heel-off is deeper; the thigh's root stays in the hip.")]))
    else:
        title = "Dedushka D, the dog and the lizard (UDDedushkaHound): the finishing round, before and after"
        sub = ("LOD0 15306 triangles as before (weights, hit and collision shapes, memory points changed; the furnace slot and the three other openings drawn wider, no vertex or UV added). "
               "Top rows: Blender 4.3, the built P3D posed with the game's own clips. Lower rows: in game, kaiju binary, the same cameras before and after.")
        hipc = (0.2, 0.28, 0.85, 1.0)
        rows.append(("1. Walk in Blender from behind his right: BEFORE (round 3 build, the integration's clips)", 300,
                     [(bl("before", "walk", "backq", k), "Before, walk phase %d of 8. %s" % (k, c), hipc) for k, c in
                      ((4, "The hound's thigh is a tube weighted to the thigh to its very end: its root lifts out of the rump."),
                       (5, "The pale flat lip of the root stands proud of the hip."),
                       (6, "The tail hangs where the hock swings: they cross."),
                       (7, "The tail sways 5 to 9 degrees: it reads as still."))]))
        rows.append(("2. The same four phases AFTER: thigh root blended into the pelvis over 4.6 m; tail carried 16 degrees back, sway 2.4 times, a whip after each footfall", 300,
                     [(bl("after", "walk", "backq", k), "After, walk phase %d of 8. %s" % (k, c), hipc) for k, c in
                      ((4, "The root of the thigh stays in the hip (it rides 90 percent with the pelvis at its end)."),
                       (5, "No lip: the tube shears over three rings instead of swinging whole."),
                       (6, "The tail is carried behind the hock."),
                       (7, "The tail swings 12 to 22 degrees and whips after each footfall."))]))
        rows.append(("3. The furnace in Blender: before (left pair) and after (right pair)", 300,
                     [(os.path.join(mo, "d_w0", "idle_front_00.png"), "Before, from in front: the slot is 7 m across and 12 m tall."),
                      (os.path.join(mo, "d_w0", "idle_back_00.png"), "Before, from behind: the split in the back is 3.6 m wide."),
                      (os.path.join(mo, "d_w1", "idle_front_00.png"), "After: the slot drawn 28 percent wider and 8 percent taller (up to 1.2 m of movement, same vertices, same UVs)."),
                      (os.path.join(mo, "d_w1", "idle_back_00.png"), "After: the back split and both flank holes 50 percent wider. From behind he is no longer dark.")]))
    # far LODs
    def lod(which, l):
        return os.path.join(mo, "%s_%s_lod%d" % (t, which, l), "walk_quarter_02.png")
    tri_after = {"a": (2594, 1002, 438), "d": (2600, 1016, 446)}[t]
    rows.append(("4. Far LODs, same walk pose: automatic decimations BEFORE (top of the pair) ...", 250,
                 [(lod("before", 1), "Before, LOD1: half of LOD0 by automatic collapse, the five big sheets (UV seams open, ribs fray)."),
                  (lod("before", 2), "Before, LOD2: a fifth of LOD0."),
                  (lod("before", 3), "Before, LOD3 (about 470 faces, the shadow LOD): " + ("the body is a cloud of shards." if t == "a" else "limbs thinned to slivers, the head a lump.")),
                  (lod("after", 1), "After, far LOD 1: %d triangles, one closed skin per body part, own 1024 hide sheet and 256 glow sheet." % tri_after[0])]))
    rows.append(("... and AFTER: skins rebuilt for the distance, furnace a bright face behind dark bars, claws as blades", 250,
                 [(lod("after", 2), "After, far LOD 2: %d triangles, 512 and 128 sheets." % tri_after[1]),
                  (lod("after", 3), "After, far LOD 3: %d triangles (under the 512 the shadow needs), 256 and 64 sheets." % tri_after[2]),
                  (os.path.join(mo, "%s_after_lod3" % t, "walk_backq_04.png"), "After, far LOD 3 from behind his right, another walk phase: it moves with the skeleton (two bones a vertex)."),
                  (os.path.join(mo, "%s_after_lod2" % t, "walk_backq_00.png"), "After, far LOD 2 from behind his right.")]))
    # in game
    wk = "walk-%s" % t
    if t == "a":
        rows.append(("5. In game, the finals' own camera: BEFORE (finals run a_anim1) and AFTER (my mount, the same test)", 216,
                     [(os.path.join(fin, "w028_front_quarter.png"), "Before, in game: the right hand's claws through the right thigh, once a stride."),
                      (game(wk, "w028_front_quarter"), "After, in game, the same camera and second of the walk."),
                      (os.path.join(fin, "w038_legs_side_40m.png"), "Before, in game: toe-off, the whole sole one slab."),
                      (game(wk, "w038_legs_side_40m"), "After, in game, the same camera.")]))
    else:
        rows.append(("5. In game, the finals' own camera: BEFORE (finals run d_anim1) and AFTER (my mount, the same test)", 216,
                     [(os.path.join(fin, "w043_legs_side_40m.png"), "Before, in game: the hound's thigh root lifts out of the hip as the leg swings back."),
                      (game(wk, "w043_legs_side_40m"), "After, in game, the same camera and second of the walk."),
                      (os.path.join(fin, "w073_passing_a_man.png"), "Before, in game: passing a man."),
                      (game(wk, "w073_passing_a_man"), "After, in game, the same camera.")]))
    pre = "dusk" if t == "a" else "hound_dusk"
    dm = "demo-%s-fin" % t
    rows.append(("6. The demo at dusk in game: BEFORE (integration, automatic far LODs) and AFTER (my mount)", 216,
                 [(os.path.join(ish, "%s_01_glow_far_out_on_the_bay.png" % pre), "Before: dormant, from 1.85 km out on the bay (the engine's far draw).", (0.3, 0.2, 0.7, 0.75)),
                  (game(dm, "01_glow_far_out_on_the_bay"), "After, the same camera: the far LOD made for it.", (0.3, 0.2, 0.7, 0.75)),
                  (os.path.join(ish, "%s_04_roar_from_among_the_garrison.png" % pre), "Before: the roar from among the garrison."),
                  (game(dm, "04_roar_from_among_the_garrison"), "After, the same beat and camera.")]))
    rows.append(("7. The demo, later beats, AFTER (my mount)", 216,
                 [(game(dm, "03c_among_the_houses"), "After: among the houses at the west end of the row."),
                  (game(dm, "07_the_beam_from_the_north"), "After: the beam from the open ground north of the row."),
                  (game(dm, "08c_spent_from_the_street"), "After: spent, from the street, 27 m from his feet."),
                  (game(dm, "09_turned_south_toward_the_sea"), "After: turned south toward the sea.")]))
    ht = "hits-%s" % t
    rows.append(("8. Hit regions and passing under him, in game (weapons fired)", 216,
                 [(game(ht, "H01_tank_rounds_in_the_furnace"), "An M1A1 300 m in front fires three rounds: each is reported in the furnace (2.5 points each)."),
                  (game(ht, "H02_a_shell_on_the_skull"), "A shell let fall on the skull: reported as head (1.5 points)."),
                  (game(ht, "H04_a_shell_on_the_leg"), "A shell let fall on the leg's own hit box: a direct hit at full weight, reported as legs (0.4 of its points)."),
                  (game(ht, "P02_tank_past_his_foot"), "Two men walked and a T-80 rolled through where his legs stand: he moved 0 m.")]))
    sheet(rows, os.path.join(M, "sheet_%s.png" % t), title, sub)


if __name__ == "__main__":
    main()
