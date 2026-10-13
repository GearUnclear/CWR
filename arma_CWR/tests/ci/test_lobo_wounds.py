"""LoBo_Pilot wounds for every pilot class, in the suit's own colour (2026-10-11.4).

Synthetic fixtures only, like the other LoBo repair tests. One opt-in test
reads a real LoBo_Pilot.pbo when LOBO_PILOT_PBO names it (read-only) and
checks the 18 generated members against their pins and a second run.
"""
import hashlib
import os
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import paa, references, wounds  # noqa: E402
from repair.pbo import Archive  # noqa: E402
import repair_lobo as runner  # noqa: E402

BS = chr(92)
PILOT = "lobo_pilot/config.cpp"


def image(w, h, fn):
    pixels = [fn(x, y) for y in range(h) for x in range(w)]
    return (w, h, [p[0] for p in pixels], [p[1] for p in pixels], [p[2] for p in pixels])


def olive(x, y):
    return 60 + (x * 7 + y * 3) % 5, 68 + (x + 2 * y) % 4, 54 + (x * y) % 3


def in_blob(x, y, w, h):
    return (x - w // 2) ** 2 + (y - h // 2) ** 2 <= (min(w, h) // 4) ** 2


def wounded_olive(w, h):
    return image(w, h, lambda x, y: (150, 22, 16) if in_blob(x, y, w, h) else olive(x, y))


def sha(data):
    return hashlib.sha256(data).hexdigest()


def lzss_literals(data: bytes) -> bytes:
    """A literal-only OFP LZSS stream with its checksum."""
    out = bytearray()
    for i in range(0, len(data), 8):
        chunk = data[i:i + 8]
        out.append((1 << len(chunk)) - 1)
        out += chunk
    return bytes(out) + struct.pack("<i", sum(c - 256 if c > 127 else c for c in data))


def ai88(w, h, stream):
    level = struct.pack("<HH", w, h) + len(stream).to_bytes(3, "little") + stream
    return struct.pack("<H", paa.AI88) + b"\0\0" + level + paa.TERMINATOR


class PaaCodecTests(unittest.TestCase):
    def test_written_layout_matches_the_shipped_d_files(self):
        src = image(16, 32, lambda x, y: (x * 15, y * 8, 255 - x * 15))
        data = paa.write(src, 3)
        info = paa.read(data)
        self.assertEqual(paa.DXT1, info["format"])
        self.assertEqual([b"CGVA", b"SFFO"], [name for name, _ in info["tags"]])
        self.assertEqual([(16, 32), (8, 16), (4, 8)], [(w, h) for w, h, _ in info["levels"]])
        offsets = struct.unpack("<16I", dict(info["tags"])[b"SFFO"])
        pos = 96
        for i, (w, h, blocks) in enumerate(info["levels"]):
            self.assertEqual(pos, offsets[i])
            self.assertEqual(struct.pack("<HH", w, h) + len(blocks).to_bytes(3, "little"), data[pos:pos + 7])
            pos += 7 + len(blocks)
        self.assertEqual((0,) * 13, offsets[3:])
        self.assertEqual(paa.TERMINATOR, data[pos:])
        top = paa.decode(data)
        self.assertEqual(paa.average(top), struct.unpack("<I", dict(info["tags"])[b"CGVA"])[0])
        # Red and blue fall along x while green rises along y: a 4x4 block can
        # follow only one direction, so green keeps most of its error.
        errors = [sum((a - b) ** 2 for a, b in zip(ca, cb)) / (16 * 32) for ca, cb in zip(top[2:], src[2:])]
        self.assertLess(errors[0], 8.0)
        self.assertLess(errors[1], 90.0)
        self.assertLess(errors[2], 8.0)
        with self.assertRaisesRegex(ValueError, "Cannot build"):
            paa.write(src, 4)

    def test_every_block_is_four_colour_and_flat_blocks_are_exact_565(self):
        src = image(8, 8, lambda x, y: (200, 100, 50) if x < 4 else (x * 30, 255 - y * 30, x * y * 4))
        blocks = paa.read(paa.write(src, 1))["levels"][0][2]
        for i in range(0, len(blocks), 8):
            c0, c1, idx = struct.unpack_from("<HHI", blocks, i)
            self.assertTrue(c0 > c1 or (c0 == c1 and idx == 0))
        flat = image(4, 4, lambda x, y: (8, 4, 8))
        self.assertEqual(([8] * 16, [4] * 16, [8] * 16), paa.decode(paa.write(flat, 1))[2:])

    def test_encoder_output_is_pinned(self):
        # Integer and IEEE +,-,*,/ only: the same bytes on every platform and Python.
        src = image(32, 16, lambda x, y: ((x * 37 + y * 11) % 256, (x * x + 3 * y) % 256, (255 - x * 5 + y * y) % 256))
        self.assertEqual(paa.write(src, 3), paa.write(src, 3))
        self.assertEqual("5a2aa62f32eb9c37053db3a1f65be731b701f84a9f0ea298b35a72ab8a2af387", sha(paa.write(src, 3)))

    def test_decoder_follows_the_engine_including_three_colour_blocks(self):
        def single(c0, c1, idx):
            body = struct.pack("<HHI", c0, c1, idx)
            return struct.pack("<H", paa.DXT1) + b"\0\0" + struct.pack("<HH", 4, 4) + b"\x08\0\0" + body + paa.TERMINATOR
        white, black = 0xFFFF, 0x0000
        pixels = paa.decode(single(white, black, 0b11100100))
        self.assertEqual([255, 0, 170, 85], pixels[2][:4])
        three = paa.decode(single(black, white, 0b11100100))
        self.assertEqual([0, 255, 127, 0], three[2][:4])
        # bit replication: 5-bit 1 -> 8, 6-bit 1 -> 4
        rep = paa.decode(single(0x0821, 0x0821, 0))
        self.assertEqual((8, 4, 8), (rep[2][0], rep[3][0], rep[4][0]))

    def test_ai88_lzss_decodes_eight_bit_grey_and_checks_its_checksum(self):
        raw = bytes(v for i in range(16) for v in (10 * i + 5, 255))
        grey = paa.decode(ai88(4, 4, lzss_literals(raw)))
        self.assertEqual([10 * i + 5 for i in range(16)], grey[2])
        self.assertEqual(grey[2], grey[3])
        # Two literals then backward copies of distance 2, lengths 18 and 12.
        stream = bytes([0x03, 100, 255, 2, 0x0F, 2, 0x09]) + struct.pack("<i", 16 * 100 - 16)
        self.assertEqual([100] * 16, paa.decode(ai88(4, 4, stream))[2])
        bad = bytearray(lzss_literals(raw))
        bad[-1] ^= 1
        with self.assertRaisesRegex(ValueError, "checksum"):
            paa.decode(ai88(4, 4, bytes(bad)))
        translucent = bytes(v for i in range(16) for v in (10, 128))
        with self.assertRaisesRegex(ValueError, "opaque"):
            paa.decode(ai88(4, 4, lzss_literals(translucent)))
        with self.assertRaisesRegex(ValueError, "Truncated"):
            paa.decode(ai88(4, 4, lzss_literals(raw)[:-9]))

    def test_reader_rejects_unsupported_layouts(self):
        good = paa.write(image(4, 4, olive), 1)
        cases = {"not": b"\0\0" + good[2:],
                 "palette": good[:94] + b"\1\0" + good[96:],
                 "Unsupported": good[:96] + struct.pack("<H", 0x8004) + good[98:],
                 "size": good[:100] + b"\x09" + good[101:],
                 "Truncated": good[:108],
                 "truncated pAA level": good[:-6]}
        for message, data in cases.items():
            with self.subTest(message=message), self.assertRaisesRegex(ValueError, f"(?i){message}"):
                paa.read(data)

    def test_downscale_is_pillow_box_rounding(self):
        src = (4, 2, [0, 1, 2, 2, 1, 1, 255, 254], [0] * 8, [9] * 8)
        self.assertEqual((2, 1, [1, 129], [0, 0], [9, 9]), paa.downscale(src))


class PipelineTests(unittest.TestCase):
    def test_blur_equals_pillow_gaussian_blur_0_6(self):
        values = [0] * 35
        values[0], values[17], values[34] = 200, 255, 90
        # Pillow 12.3 ImageFilter.GaussianBlur(0.6) of the same 7x5 L image.
        expected = [141, 25, 2, 2, 0, 0, 0, 25, 5, 5, 25, 5, 0, 0, 2, 3, 25, 125, 25, 3, 1,
                    0, 0, 5, 25, 5, 3, 12, 0, 0, 0, 2, 1, 11, 64]
        self.assertEqual(expected, wounds._blur(values, 7, 5))

    def test_blood_lands_on_the_healthy_suit_and_nothing_else_changes(self):
        w = h = 32
        base = image(w, h, olive)
        alpha, layer = wounds.blood_layer(wounded_olive(w, h), base, base)
        grey = image(w, h, lambda x, y: (120 + x % 3, 121, 119 + y % 2))
        out = wounds.composite(grey, alpha, layer)
        for i in range(w * h):
            x, y = i % w, i // w
            if alpha[i] == 0.0:
                self.assertEqual((grey[2][i], grey[3][i], grey[4][i]), (out[2][i], out[3][i], out[4][i]))
            if (x - 16) ** 2 + (y - 16) ** 2 <= 9:
                self.assertEqual(1.0, alpha[i])
                self.assertGreater(out[2][i], 3 * max(out[3][i], out[4][i]))
        self.assertTrue(all(a == 0.0 for i, a in enumerate(alpha) if abs(i % w - 16) > 12 or abs(i // w - 16) > 12))

    def test_layer_never_exceeds_alpha_so_highlights_do_not_turn_pink(self):
        # A _d brighter than its base (a fold highlight, no blood) once leaked
        # through the unmix as a pink fringe; alpha is 0 there, so the layer is too.
        w = h = 16
        base = image(w, h, olive)
        bright = image(w, h, lambda x, y: (100, 104, 90) if y < 4 else olive(x, y))
        alpha, layer = wounds.blood_layer(bright, base, base)
        self.assertTrue(all(a == 0.0 for a in alpha))
        self.assertTrue(all(c == 0.0 for px in layer for c in px))

    def test_rjaf_only_patches_are_filled_and_never_read_as_blood(self):
        w = h = 32
        flag = image(w, h, lambda x, y: (200, 30, 30) if 2 <= x < 10 and 2 <= y < 8 else olive(x, y))
        plain = image(w, h, olive)
        # The _d lacks the patch: without the fill the red flag would unmix as blood.
        alpha, _ = wounds.blood_layer(plain, flag, plain)
        self.assertTrue(all(a == 0.0 for a in alpha))
        # A fill that has its own patch there falls back to the mean olive.
        alpha, _ = wounds.blood_layer(plain, flag, flag)
        self.assertLess(max(alpha), 0.2)

    def test_fit_halves_to_the_d_size_or_fails(self):
        big = image(16, 32, olive)
        self.assertEqual((4, 8), wounds._fit(big, 4, 8)[:2])
        with self.assertRaisesRegex(ValueError, "Cannot fit"):
            wounds._fit(big, 6, 8)


def synthetic_sources(scale=2):
    """{SOURCE_MEMBERS key: PAA bytes}: 16x16 olive _d set with blood, healthy at 16 or 32 px."""
    out = {}
    for part in wounds.PARTS:
        d = 16
        size = d * (1 if part in ("back", "legsbk") else scale)
        out[wounds._key(wounds.OLIVE_WOUNDED, part + "_d.paa")] = paa.write(wounded_olive(d, d), 3)
        out[wounds._key(wounds.OLIVE_BASE[0], wounds.OLIVE_BASE[1][part])] = paa.write(image(size, size, olive), 3)
        out[wounds._key(wounds.OLIVE_FILL[0], wounds.OLIVE_FILL[1][part])] = paa.write(image(size, size, olive), 3)
        for n, (folder, names) in enumerate(wounds.SUITS.items()):
            if part in names:
                suit = image(size, size, lambda x, y, n=n: (90 + 20 * n, 50 + 25 * n, 30 + 30 * n))
                out[wounds._key(folder, names[part])] = paa.write(suit, 3)
    return out


def pinned(sources):
    made = wounds.generate(sources)
    return ({key: sha(data) for key, data in sources.items()},
            {key: (stored, sha(made[key])) for key, stored in wounds.outputs().items()}, made)


class AddMembersTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.sources = synthetic_sources()
        cls.source_pins, cls.output_pins, cls.made = pinned(cls.sources)

    def setUp(self):
        for name, value in (("SOURCE_SHA256", self.source_pins), ("OUTPUTS", self.output_pins)):
            patcher = patch.object(wounds, name, value)
            patcher.start()
            self.addCleanup(patcher.stop)

    def test_all_members_are_added_with_stored_names_and_the_shipped_level_count(self):
        added, skips = wounds.add_members({}, None, self.sources)
        self.assertEqual([], skips)
        self.assertEqual(list(wounds.ADDED_MEMBERS), list(added))
        self.assertEqual(18, len(added))
        for key, (stored, data) in added.items():
            self.assertEqual(self.output_pins[key][0], stored)
            self.assertTrue(stored.endswith("_dr.paa") and BS in stored)
            info = paa.read(data)
            self.assertEqual([(16, 16), (8, 8), (4, 4)], [(w, h) for w, h, _ in info["levels"]])
        self.assertEqual(set(wounds.SOURCE_MEMBERS), set(self.sources))

    def test_generation_is_deterministic(self):
        again = wounds.generate(dict(self.sources))
        self.assertEqual(self.made, again)

    def test_present_members_are_verified_not_regenerated(self):
        everything = {key: data for key, data in self.made.items()}
        with patch.object(wounds, "generate", side_effect=AssertionError("regenerated")):
            self.assertEqual(({}, []), wounds.add_members(everything, None, self.sources))
        some = dict(list(everything.items())[:5])
        added, _ = wounds.add_members(some, None, self.sources)
        self.assertEqual(list(wounds.ADDED_MEMBERS)[5:], list(added))

    def test_fail_closed(self):
        key = next(iter(self.sources))
        cases = (
            ({}, {k: v for k, v in self.sources.items() if k != key}, "missing"),
            ({}, {**self.sources, key: self.sources[key] + b"\0"}, "different LoBo install"),
            ({wounds.ADDED_MEMBERS[0]: b"other"}, self.sources, "not the pinned generated"),
            ({"lobo_pilot/eaf/tex/other_dr.paa": b"x"}, self.sources, "Unknown added"),
        )
        for existing, sources, message in cases:
            with self.subTest(message=message), self.assertRaisesRegex(ValueError, message):
                wounds.add_members(existing, None, sources)
        wrong = dict(self.output_pins)
        wrong[wounds.ADDED_MEMBERS[3]] = (wrong[wounds.ADDED_MEMBERS[3]][0], "0" * 64)
        with patch.object(wounds, "OUTPUTS", wrong), self.assertRaisesRegex(ValueError, "do not match"):
            wounds.add_members({}, None, self.sources)


def pair(folder, healthy, wounded):
    root = BS + "LoBo_Pilot" + BS + folder + BS
    return f'"{root}{healthy}", "{root}{wounded}"'


IAF, EAF, RJAF, SYAF = (folder + BS + "tex" for folder in ("IAF", "EAF", "RJAF", "SyAF"))
IAF2, TEX3, TEX4, TEX5 = ("IAF" + BS + tex for tex in ("tex2", "tex3", "tex4", "tex5"))
# The pair order of every authored full array in LoBo_Pilot's config.
ORDER = ("armle", "armri", "back", "front", "legsbk", "legsfr", "legslft", "legsrgt")


def pairs(folder, suffix, pac=(), arm=None):
    return ", ".join(pair(arm if p == "armle" and arm else folder, p + (".pac" if p in pac else ".paa"), p + suffix)
                     for p in ORDER)


def full(body):
    return ["\t\twounds[] =", "\t\t{", "\t\t\t" + wounds.FACES + ", " + body, "\t\t};"]


ANIMATIONS = ["\t\tclass Animations", "\t\t{", "\t\t\tclass goggles", "\t\t\t{", '\t\t\t\tselection="visor";',
              "\t\t\t};", "\t\t};"]
HANDLERS = ["\t\tclass EventHandlers", "\t\t{", '\t\t\tfired = "{_this}";', "\t\t};"]


def pilot_config(state=False, *, extra=None):
    """A CfgVehicles laid out like LoBo_Pilot's: every pilot class, authored or repaired."""
    extra = extra or {}
    grey_authored = ["\t\twounds[] =", "\t\t{" + pair(TEX3, "armle.paa", "armle_d.paa") + "};"]
    tex2 = "_dr.paa" if state else "_d.paa"
    classes = (  # head, Animations, wounds[] lines, EventHandlers
        ("LoBo_IAF_Pilot: SoldierWPilot", True, full(pairs(IAF, "_d.paa", ("back", "legsbk"))), True),
        ("LoBo_IAF_Pilot_cap: LoBo_IAF_Pilot", False, [], False),
        ("LoBo_IAF_Pilot2: LoBo_IAF_Pilot", True,
         full(pairs(IAF, "_d.paa", ("back", "legsbk"), TEX5)) if state else [], True),
        ("LoBo_IAF_Pilot2_cap: LoBo_IAF_Pilot2", False, [], False),
        ("LoBo_IAF_Pilot_grey: LoBo_IAF_Pilot", False, full(pairs(IAF2, tex2, ("back",))), False),
        ("LoBo_IAF_Pilot_Cap_grey: LoBo_IAF_Pilot_grey", False, [], False),
        ("LoBo_IAF_Pilot_grey2: LoBo_IAF_Pilot_grey", False,
         full(pairs(IAF2, "_dr.paa", ("back",), TEX3)) if state else grey_authored, False),
        ("LoBo_IAF_Pilot_Cap_grey2: LoBo_IAF_Pilot_grey2", False, [], False),
        ("LoBo_IAF_Pilot_grey3: LoBo_IAF_Pilot_grey", False,
         full(pairs(IAF2, "_dr.paa", ("back",), TEX4)) if state else grey_authored, False),
        ("LoBo_IAF_Pilot_Cap_grey3: LoBo_IAF_Pilot_grey3", False, [], False),
        ("LoBo_Egypt_Pilot: SoldierEPilot", True, full(pairs(EAF, "_dr.paa" if state else "_d.paa")), True),
        ("LoBo_Egypt_Pilot_cap: LoBo_Egypt_Pilot", False, [], False),
        ("LoBo_Syria_Pilot: LoBo_Egypt_Pilot", True,
         full(pairs(SYAF, "_d.paa", ("back", "legsbk"))) if state else [], False),
        ("LoBo_Jordan_Pilot: LoBo_Egypt_Pilot", True, full(pairs(RJAF, "_d.paa")), False),
        ("LoBo_Jordan_Pilot_cap: LoBo_Jordan_Pilot", False, [], False),
    )
    lines = ["class CfgVehicles", "{", "\tclass SoldierWPilot: SoldierWB{};"]
    for head, animations, wound, handlers in classes:
        name = head.split(":")[0]
        wound = list(wound)
        if name in extra and len(wound) == 4:
            wound[2] += extra[name]
        elif name in extra:
            wound[1] = wound[1][:-2] + extra[name] + "};"
        lines += ["\tclass " + head, "\t{", f'\t\tmodel="{BS}LoBo_Pilot{BS}{name}";']
        lines += (ANIMATIONS if animations else []) + wound + (HANDLERS if handlers else []) + ["\t};", ""]
    return "\r\n".join(lines + ["};", ""])


def class_body(text, name):
    lines = text.replace("\r\n", "\n").split("\n")
    start, end = wounds._class_span(lines, name)
    return "\n".join(lines[start:end])


def healthy_names(text, name):
    """Healthy LoBo_Pilot texture names of a class's own wounds[] (face entries skipped)."""
    body = class_body(text, name)
    line = next(line for line in body.split("\n") if line.startswith('\t\t\t"xicht_a.paa"'))
    names = line.strip().split(", ")
    rest = [n for n in names if n.startswith('"' + BS + "LoBo_Pilot")]
    return {n.strip('"')[1:] for n in rest[0::2]}, [n.strip('"')[1:] for n in rest[1::2]]


# Body face textures of each pilot model with a _d part (read from the p3d
# files of LoBo_Pilot.pbo, every LOD; lod\ folders have no _d). Keyed by the
# class whose own wounds[] covers the model after the repair.
MODEL_FACES = {
    "LoBo_IAF_Pilot": {f"LoBo_Pilot{BS}{IAF}{BS}{n}" for n in (
        "armle.paa", "armri.paa", "back.pac", "front.paa", "legsbk.pac", "legsfr.paa", "legslft.paa", "legsrgt.paa")},
    "LoBo_IAF_Pilot2": {f"LoBo_Pilot{BS}{TEX5}{BS}armle.paa"} | {f"LoBo_Pilot{BS}{IAF}{BS}{n}" for n in (
        "armri.paa", "back.pac", "front.paa", "legsbk.pac", "legsfr.paa", "legslft.paa", "legsrgt.paa")},
    "LoBo_IAF_Pilot_grey": {f"LoBo_Pilot{BS}{IAF2}{BS}{n}" for n in (
        "armle.paa", "armri.paa", "back.pac", "front.paa", "legsbk.paa", "legsfr.paa", "legslft.paa", "legsrgt.paa")},
    "LoBo_IAF_Pilot_grey2": {f"LoBo_Pilot{BS}{TEX3}{BS}armle.paa"} | {f"LoBo_Pilot{BS}{IAF2}{BS}{n}" for n in (
        "armri.paa", "back.pac", "front.paa", "legsbk.paa", "legsfr.paa", "legslft.paa", "legsrgt.paa")},
    "LoBo_IAF_Pilot_grey3": {f"LoBo_Pilot{BS}{TEX4}{BS}armle.paa"} | {f"LoBo_Pilot{BS}{IAF2}{BS}{n}" for n in (
        "armri.paa", "back.pac", "front.paa", "legsbk.paa", "legsfr.paa", "legslft.paa", "legsrgt.paa")},
    "LoBo_Egypt_Pilot": {f"LoBo_Pilot{BS}{EAF}{BS}{p}.paa" for p in ORDER},
    "LoBo_Syria_Pilot": {f"LoBo_Pilot{BS}{SYAF}{BS}{n}" for n in (
        "armle.paa", "armri.paa", "back.pac", "front.paa", "legsbk.pac", "legsfr.paa", "legslft.paa", "legsrgt.paa")},
    "LoBo_Jordan_Pilot": {f"LoBo_Pilot{BS}{RJAF}{BS}{p}.paa" for p in ORDER},
}


class ConfigRuleTests(unittest.TestCase):
    def test_every_pilot_class_gets_its_pairs(self):
        before, after = pilot_config(), pilot_config(True)
        self.assertEqual({PILOT: after}, wounds.repair({PILOT: before}))
        self.assertEqual({}, wounds.repair({PILOT: after}))
        self.assertEqual(after.count("\n"), after.count("\r\n"))
        for name in ("LoBo_IAF_Pilot", "LoBo_IAF_Pilot_cap", "LoBo_IAF_Pilot2_cap", "LoBo_IAF_Pilot_Cap_grey",
                     "LoBo_Egypt_Pilot_cap", "LoBo_Jordan_Pilot", "LoBo_Jordan_Pilot_cap"):
            self.assertEqual(class_body(before, name), class_body(after, name))
        # Eight _dr pairs each: Egypt, grey and the two copies of grey's body in grey2 and grey3.
        self.assertEqual(32, after.count("_dr.paa"))
        for name in MODEL_FACES:
            with self.subTest(name=name):
                self.assertEqual(1, class_body(after, name).count("wounds[]"))
                self.assertEqual(1, class_body(after, name).count(wounds.FACE_HEAD))
        # The inherited arrays sit after class Animations, as in their parents.
        for name in ("LoBo_IAF_Pilot2", "LoBo_Syria_Pilot"):
            body = class_body(after, name)
            self.assertIn("\t\t\t};\n\t\t};\n\t\twounds[] =\n\t\t{\n", body)

    def test_each_class_pairs_exactly_its_model_faces(self):
        after = pilot_config(True)
        for name, faces in MODEL_FACES.items():
            with self.subTest(name=name):
                healthy, wounded = healthy_names(after, name)
                self.assertEqual(faces, healthy)
                self.assertEqual(8, len(wounded))
        for cls, (_, rows) in wounds.BLOCKS.items():
            self.assertEqual(MODEL_FACES[cls], {f"LoBo_Pilot{BS}{folder}{BS}{healthy}" for folder, healthy, _ in rows})

    def test_every_wounded_target_is_generated_or_required(self):
        targets = {wounds._key(folder, wounded) for _, rows in wounds.BLOCKS.values() for folder, _, wounded in rows}
        targets |= {wounds._key(folder, part + "_dr.paa") for folder in wounds.CLASSES.values() for part in wounds.PARTS}
        self.assertEqual(targets, set(wounds.OUTPUTS) | set(wounds.REQUIRED_MEMBERS))
        self.assertFalse(set(wounds.OUTPUTS) & set(wounds.REQUIRED_MEMBERS))
        self.assertEqual(16, len(wounds.REQUIRED_MEMBERS))
        self.assertTrue(all(key.endswith("_d.paa") for key in wounds.REQUIRED_MEMBERS))

    def test_partial_state_completes(self):
        partial = pilot_config().replace(pair(EAF, "front.paa", "front_d.paa"), pair(EAF, "front.paa", "front_dr.paa"))
        done = pilot_config(True)
        lines = partial.replace("\r\n", "\n").split("\n")
        start, end = wounds._class_span(lines, "LoBo_IAF_Pilot_grey2")
        lines[start:end] = class_body(done, "LoBo_IAF_Pilot_grey2").split("\n")
        partial = "\r\n".join(lines)
        self.assertEqual(done, wounds.repair({PILOT: partial})[PILOT])

    def test_fail_closed(self):
        egypt = pair(EAF, "back.paa", "back_d.paa")
        own = "\t\twounds[] =\r\n\t\t{" + pair(SYAF, "back.pac", "back_d.paa") + "};\r\n"
        broken = {
            "duplicate pair": pilot_config(extra={"LoBo_Egypt_Pilot": ", " + egypt}),
            "missing pair": pilot_config().replace(egypt, pair(EAF, "back.paa", "back.paa")),
            "missing class": pilot_config().replace("\tclass LoBo_IAF_Pilot_grey3:", "\tclass LoBo_IAF_Pilot_grey9:"),
            "unrepaired .pac name": pilot_config().replace(egypt, pair(EAF, "back.pac", "back_d.paa")),
            "own wounds[] already": pilot_config().replace(
                '\t\tmodel="' + BS + "LoBo_Pilot" + BS + 'LoBo_Syria_Pilot";\r\n',
                '\t\tmodel="' + BS + "LoBo_Pilot" + BS + 'LoBo_Syria_Pilot";\r\n' + own),
            "other authored grey2": pilot_config(extra={"LoBo_IAF_Pilot_grey2": ", " + pair(TEX3, "a.paa", "b.paa")}),
            "other face entries": pilot_config().replace('"xicht_e.paa", "xicht_e_zranen"',
                                                         '"xicht_e.paa", "xicht_a_zranen"', 1),
            "no Animations": pilot_config().replace(
                '\t\tmodel="' + BS + "LoBo_Pilot" + BS + 'LoBo_IAF_Pilot2";\r\n' + "\r\n".join(ANIMATIONS),
                '\t\tmodel="' + BS + "LoBo_Pilot" + BS + 'LoBo_IAF_Pilot2";'),
            "half-written array": pilot_config(True).replace(pair(SYAF, "front.paa", "front_d.paa"),
                                                             pair(SYAF, "front.paa", "front_dr.paa")),
        }
        for message, text in broken.items():
            with self.subTest(message=message), self.assertRaises(ValueError):
                wounds.repair({PILOT: text})
        with self.assertRaisesRegex(ValueError, "Missing members"):
            wounds.repair({})

    def test_references_and_wounds_reach_a_joint_fixed_point(self):
        # .4 references repoints five healthy .pac names; wounds then repoints
        # the wounded half and copies the repaired IAF tex2 legsbk pair into
        # grey2 and grey3. references counts per class and accepts the result.
        authored = pilot_config()
        for folder in (EAF, RJAF):
            for part in ("back", "legsbk"):
                authored = authored.replace(pair(folder, part + ".paa", part + "_d.paa"),
                                            pair(folder, part + ".pac", part + "_d.paa"))
        authored = authored.replace(pair(IAF2, "legsbk.paa", "legsbk_d.paa"), pair(IAF2, "legsbk.pac", "legsbk_d.paa"))
        mig = '\t\tsoundLandCrash[]={"' + BS + "LoBoMig" + BS + "sounds" + BS + 'jet_crash.wss",0.006000,1};'
        docs = {PILOT: authored, "lobomig/config.cpp": mig}
        docs.update(references.repair(docs))
        self.assertEqual(pilot_config(), docs[PILOT])
        docs.update(wounds.repair(docs))
        self.assertEqual(pilot_config(True), docs[PILOT])
        self.assertEqual(3, docs[PILOT].count(pair(IAF2, "legsbk.paa", "legsbk_dr.paa")))
        self.assertEqual({}, references.repair(docs))
        self.assertEqual({}, wounds.repair(docs))


def make_pbo(members):
    header = b"".join(name.encode("latin1") + b"\0" + struct.pack("<5I", 0, 0, 0, 0, len(data))
                      for name, data in members.items())
    body = header + bytes(21) + b"".join(members.values())
    return body + b"\0" + hashlib.sha1(body).digest()


def stored_name(key):
    folder, name = key.split("/", 1)[1].rsplit("/", 1)
    return BS.join(part.upper() if part in ("eaf", "iaf", "rjaf") else part for part in folder.split("/")) + BS + name


class PlanTests(unittest.TestCase):
    """repair_lobo.plan reads SOURCE_MEMBERS and passes them to add_members."""

    @classmethod
    def setUpClass(cls):
        cls.sources = synthetic_sources()
        cls.source_pins, cls.output_pins, _ = pinned(cls.sources)

    def setUp(self):
        for name, value in (("SOURCE_SHA256", self.source_pins), ("OUTPUTS", self.output_pins)):
            patcher = patch.object(wounds, name, value)
            patcher.start()
            self.addCleanup(patcher.stop)
        modules = patch.object(runner, "modules", return_value=[wounds])
        modules.start()
        self.addCleanup(modules.stop)
        self.temporary = tempfile.TemporaryDirectory(prefix="lobo-wounds-test-")
        self.addCleanup(self.temporary.cleanup)
        self.mod = Path(self.temporary.name)
        (self.mod / "addons").mkdir()

    def write(self, sources, config=None, required=None):
        members = {"config.cpp": (config or pilot_config()).encode("latin1")}
        members.update({stored_name(key): data for key, data in sources.items()})
        members.update({stored_name(key): b"shipped olive _d" for key in
                        (wounds.REQUIRED_MEMBERS if required is None else required)})
        (self.mod / "addons" / "LoBo_Pilot.pbo").write_bytes(make_pbo(members))

    def test_a_missing_shipped_wounded_target_fails_closed(self):
        for key in (wounds.REQUIRED_MEMBERS[0], "lobo_pilot/syaf/tex/legsbk_d.paa"):
            with self.subTest(key=key):
                self.write(self.sources, required=[k for k in wounds.REQUIRED_MEMBERS if k != key])
                with self.assertRaisesRegex(ValueError, "Required members missing"):
                    runner.plan(self.mod, None)

    def test_plan_adds_members_and_repoints_then_reaches_a_fixed_point(self):
        self.write(self.sources)
        paths, hashes, changed, records, _, skipped = runner.plan(self.mod, None)
        self.assertEqual([], skipped)
        self.assertEqual({PILOT} | set(wounds.ADDED_MEMBERS), set(changed))
        self.assertEqual(pilot_config(True).encode("latin1"), changed[PILOT])
        added = [r for r in records if r.get("added")]
        self.assertEqual(list(wounds.ADDED_MEMBERS), [r["member"] for r in added])
        self.assertEqual([self.output_pins[r["member"]][1] for r in added], [r["after_sha256"] for r in added])
        runner.apply(self.mod, paths, hashes, changed, records)
        archive = Archive((self.mod / "addons" / "LoBo_Pilot.pbo").read_bytes())
        for key in wounds.ADDED_MEMBERS:
            self.assertEqual(self.output_pins[key][1], sha(archive.read(key.split("/", 1)[1])))
        with patch.object(wounds, "generate", side_effect=AssertionError("regenerated")):
            self.assertEqual({}, runner.plan(self.mod, None)[2])

    def test_missing_or_changed_source_fails_before_any_write(self):
        key = wounds.SOURCE_MEMBERS[7]
        self.write({k: v for k, v in self.sources.items() if k != key})
        before = (self.mod / "addons" / "LoBo_Pilot.pbo").read_bytes()
        with self.assertRaisesRegex(ValueError, "Source members missing"):
            runner.plan(self.mod, None)
        self.write({**self.sources, key: self.sources[key][:-6] + b"\1" + self.sources[key][-5:]})
        with self.assertRaisesRegex(ValueError, "different LoBo install"):
            runner.plan(self.mod, None)
        self.assertNotEqual(before, b"")

    def test_source_member_declarations_are_validated(self):
        self.write(self.sources)
        for keys, message in ((("other/tex/a.paa",), "outside the repaired addons"),
                              (("lobo_pilot/Tex/a.paa",), "normalized"),
                              (("lobo_pilot/config.cpp",), "untouched")):
            with self.subTest(message=message), patch.object(wounds, "SOURCE_MEMBERS", keys), \
                    self.assertRaisesRegex(ValueError, message):
                runner.plan(self.mod, None)


@unittest.skipUnless(os.environ.get("LOBO_PILOT_PBO"), "set LOBO_PILOT_PBO to a LoBo_Pilot.pbo to run (read-only)")
class RealArchiveTests(unittest.TestCase):
    def test_real_sources_give_the_pinned_members_twice(self):
        archive = Archive(Path(os.environ["LOBO_PILOT_PBO"]).read_bytes())
        sources = {key: bytes(archive.stored(archive.by_key[key.split("/", 1)[1]])) for key in wounds.SOURCE_MEMBERS}
        existing = {key: bytes(archive.stored(archive.by_key[key.split("/", 1)[1]]))
                    for key in wounds.ADDED_MEMBERS if key.split("/", 1)[1] in archive.by_key}
        if existing:
            self.assertEqual(({}, []), wounds.add_members(existing, None, sources))
        added, _ = wounds.add_members({}, None, sources)
        self.assertEqual({key: pin for key, (_, pin) in wounds.OUTPUTS.items()},
                         {key: sha(data) for key, (_, data) in added.items()})
        self.assertEqual({key: data for key, (_, data) in added.items()}, wounds.generate(sources))


if __name__ == "__main__":
    unittest.main()
