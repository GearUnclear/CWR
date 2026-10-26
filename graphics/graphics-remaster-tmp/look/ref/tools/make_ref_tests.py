"""Build the reference capture tests (look-ref landed + vanilla upstream).

Landed: split copies of the look-director tests in look-ref's captures dir
(prefix ref_landed_), only the remastered mode.
Vanilla: vanilla-safe copies in the vanilla-upstream copy (prefix ref_vanilla_).
"""
import re
import subprocess
import sys
from pathlib import Path

LREF = Path("D:/Arma_CWA_worktrees/look-ref/arma_CWR/tests/rendering/captures")
VAN = Path("D:/Arma_CWA_worktrees/vanilla-upstream/tests/rendering/captures")
SCR = Path(__file__).resolve().parent / "gen"

CAPBEGIN = re.compile(r'^\["([^"]+)", .*\] call capBegin\s*$')
FORK_ONLY = ["triPinCloudDrift", "triGpuTimes", "triSettleScene", "triSetLighting", "triSceneStats",
             "triPinObjectDetail", "triFeatureState", "triCaptureAtPresent", "triSetFixedStep", "triCamView"]


def split_sections(text):
    lines = text.split("\n")
    begins = [i for i, l in enumerate(lines) if CAPBEGIN.match(l)]
    footer = next(i for i, l in enumerate(lines) if l.startswith('call { "view=done;event=done"'))
    starts = []
    for b in begins:
        s = b
        while s - 1 >= 0 and (lines[s - 1].startswith("//") or lines[s - 1].strip() == ""
                              or lines[s - 1].startswith("call { capRain")):
            s -= 1
        starts.append(s)
    header = lines[:starts[0]]
    sections = []
    for k, s in enumerate(starts):
        e = starts[k + 1] if k + 1 < len(starts) else footer
        name = CAPBEGIN.match(lines[begins[k]]).group(1)
        sections.append((name, lines[s:e]))
    return header, sections, lines[footer:]


def drop_mode_block(lines, drop, keep):
    """Remove every `"<drop>" call capMode` block up to the next `"<keep>" call capMode`."""
    out, skipping = [], False
    for l in lines:
        if l.startswith(f'"{drop}" call capMode'):
            skipping = True
            continue
        if skipping and l.startswith(f'"{keep}" call capMode'):
            skipping = False
        if not skipping:
            out.append(l)
    assert not skipping, "unterminated mode block"
    return out


def vanillaize(lines):
    out = []
    for l in lines:
        if l.strip() == "triCaptureAtPresent true":
            out.append("// (vanilla: no triCaptureAtPresent; gamma 1.2 is applied offline by the collector)")
            continue
        if "triPinCloudDrift" in l:
            continue
        l = l.replace('"remastered" call capMode', '"vanilla" call capMode')
        l = l.replace("__remastered", "__vanilla")
        out.append(l)
    code = "\n".join(x.split("//")[0] for x in out)
    for v in FORK_ONLY:
        assert v not in code, f"fork-only verb left: {v}"
    return out


VANILLA_LOBO = "D:/Arma_CWA_worktrees/vanilla-upstream/tmp/lobo-mods"  # @LoBo without Bin (see NOTES.md)


def write_test(dst_dir, name, toml_text, lines, note):
    if dst_dir == VAN:
        toml_text = toml_text.replace('"--mods-dir", "."', f'"--mods-dir", "{VANILLA_LOBO}"')
    (dst_dir / f"{name}.test.sqf").write_text(f"// {note}\n" + "\n".join(lines).rstrip("\n") + "\n",
                                               encoding="utf-8", newline="\n")
    (dst_dir / f"{name}.test.toml").write_text(toml_text, encoding="utf-8", newline="\n")


def main():
    SCR.mkdir(exist_ok=True)
    made = {"landed": [], "vanilla": []}
    # ---- the 1080p look_final views, generated with one mode
    gens = {}
    for side, modes, tag in (("landed", "landed=remastered", "landed"), ("vanilla", "vanilla=vanilla", "refvanilla")):
        out = LREF if side == "landed" else SCR
        subprocess.check_call([sys.executable, str(LREF / "gen_look_director.py"), "--modes", modes, "--tag", tag,
                               "--out", str(out)], cwd=str(LREF))
        gens[side] = (out, tag)
    chunks = {"abel": [(0, 5, "a"), (5, 10, "b"), (10, 15, "c")], "eden": [(0, 4, "a")], "sinai": [(0, 2, "a")]}
    for side in ("landed", "vanilla"):
        out, tag = gens[side]
        for world, parts in chunks.items():
            text = (out / f"look_{tag}_{world}.test.sqf").read_text(encoding="utf-8")
            toml_text = (out / f"look_{tag}_{world}.test.toml").read_text(encoding="utf-8")
            header, sections, footer = split_sections(text)
            for a, b, suf in parts:
                body = header[:]
                for _, sec in sections[a:b]:
                    body += sec
                body += footer
                if side == "vanilla":
                    body = vanillaize(body)
                name = f"ref_{side}_1080_{world}_{suf}"
                names = ",".join(n for n, _ in sections[a:b])
                write_test(LREF if side == "landed" else VAN, name, toml_text, body,
                           f"look-ref {side} reference, 1080p look_final views: {names}")
                made[side].append(name)
    # ---- the 960x640 run-captures tests, remastered mode only
    src960 = {"abel": "abel_lighting", "eden": "eden_lighting", "sinai": "sinai_lobo_lighting"}
    chunks960 = {"abel": [(0, 4, "a"), (4, 7, "b"), (7, 8, "c")], "eden": [(0, 2, "a")], "sinai": [(0, 2, "a")]}
    for world, src in src960.items():
        text = (LREF / f"{src}.test.sqf").read_text(encoding="utf-8")
        toml_text = (LREF / f"{src}.test.toml").read_text(encoding="utf-8")
        header, sections, footer = split_sections(text)
        for a, b, suf in chunks960[world]:
            body = header[:]
            for _, sec in sections[a:b]:
                body += drop_mode_block(sec, "classic", "remastered")
            body += footer
            names = ",".join(n for n, _ in sections[a:b])
            for side in ("landed", "vanilla"):
                lines = vanillaize(body) if side == "vanilla" else body
                name = f"ref_{side}_960_{world}_{suf}"
                write_test(LREF if side == "landed" else VAN, name, toml_text, lines,
                           f"look-ref {side} reference, 960x640 {src} views (remastered block only): {names}")
                made[side].append(name)
    # ---- the optics test
    text = (LREF / "look_director_optics.test.sqf").read_text(encoding="utf-8")
    toml_text = (LREF / "look_director_optics.test.toml").read_text(encoding="utf-8")
    assert '} forEach ["classic", "remastered"]' in text
    for side, mode in (("landed", "remastered"), ("vanilla", "vanilla")):
        lines = text.replace('} forEach ["classic", "remastered"]', f'}} forEach ["{mode}"]').split("\n")
        if side == "vanilla":
            lines = vanillaize(lines)
        name = f"ref_{side}_optics"
        write_test(LREF if side == "landed" else VAN, name, toml_text, lines,
                   f"look-ref {side} reference, look_director_optics with one mode")
        made[side].append(name)
    for side, names in made.items():
        print(side, ",".join(f"tests/rendering/captures/{n}.test.sqf" for n in names))


if __name__ == "__main__":
    main()
