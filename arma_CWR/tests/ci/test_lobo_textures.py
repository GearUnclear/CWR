"""Synthetic MLOD tests for the fixed-width face texture repoints (textures-icons d1, e1, e2)."""
from pathlib import Path
import struct
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import models, textures  # noqa: E402

BS = chr(92)
OLD = ("pack" + BS + "m" + BS + "pod.pac").encode()
NEW = ("Mine" + BS + "MD" + BS + "pod.pac").encode()
RAW32 = ("Faction" + BS + "missing_dir" + BS + "e_tank_hrud_p_lon").encode()[:32]
RAW_NEW = ("Faction" + BS + "crew" + BS + "hrud_p.pac").encode()


def field(name):
    return name[:32] + bytes(32 - len(name[:32]))


def lod(resolution, names, points=3):
    faces = b"".join(field(name) + bytes([7]) * 72 for name in names)
    return (struct.pack("<4s6i", b"SP3X", 28, 153, points, 0, len(names), 0) + bytes(range(16)) * points
            + faces + b"TAGG" + struct.pack("<64sI", b"#EndOfFile#", 0) + struct.pack("<f", resolution))


def model(*lods, trailer=b""):
    return struct.pack("<4sII", b"MLOD", 257, len(lods)) + b"".join(lods) + trailer


def names(data):
    # Resolutions are float32 in the file; compare per-LOD name lists in order.
    return [[f.split(b"\0", 1)[0] for f in fields] for _, fields in textures.face_textures(data)]


class RepointTests(unittest.TestCase):
    def setUp(self):
        self.mapping = {OLD: (NEW, 2, False)}
        self.original = model(lod(1.0, [b"visual.paa", b"other.paa"]),
                              lod(1e13, [OLD, b"keep.paa", OLD.upper()]),
                              lod(5e15, [OLD, OLD, b"x"]))

    def test_repoint_exact_bytes_and_fixed_point(self):
        fixed = textures.repoint(self.original, (1e13, 5e15), self.mapping)
        self.assertEqual(len(self.original), len(fixed))
        self.assertEqual([[b"visual.paa", b"other.paa"], [NEW, b"keep.paa", NEW], [NEW, NEW, b"x"]], names(fixed))
        for _, fields in textures.face_textures(fixed):
            for raw in fields:
                if raw.startswith(NEW):
                    self.assertEqual(field(NEW), raw)
        self.assertEqual(fixed, textures.repoint(fixed, (1e13, 5e15), self.mapping))

    def test_only_texture_fields_change(self):
        fixed = textures.repoint(self.original, (1e13, 5e15), self.mapping)
        for (_, before), (_, after) in zip(textures.face_textures(self.original), textures.face_textures(fixed)):
            self.assertEqual(len(before), len(after))
        stripped = bytearray(fixed)
        original = bytearray(self.original)
        for data in (stripped, original):
            for lod_ in models._lods(bytes(data)):
                start, faces = textures._face_table(bytes(data), lod_)
                for i in range(faces):
                    data[start + i * 104:start + i * 104 + 32] = bytes(32)
        self.assertEqual(original, stripped)

    def test_raw32_match_and_nul_led_residue_untouched(self):
        residue = b"\0" + RAW32[1:]
        source = model(lod(5.0, [RAW32, residue, RAW32]), lod(6.0, [residue]))
        fixed = textures.repoint(source, (5.0,), {RAW32: (RAW_NEW, 2, True)})
        fields = textures.face_textures(fixed)
        self.assertEqual([field(RAW_NEW), residue, field(RAW_NEW)], fields[0][1])
        self.assertEqual([residue], fields[1][1])
        # A NUL-terminated name equal to the first 31 bytes is not an overflowed field.
        shorter = model(lod(5.0, [RAW32[:31], RAW32, RAW32]))
        with self.assertRaises(ValueError):
            textures.repoint(shorter, (5.0,), {RAW32: (RAW_NEW, 3, True)})

    def test_match_in_unlisted_lod_raises(self):
        with self.assertRaisesRegex(ValueError, "outside the listed LODs"):
            textures.repoint(self.original, (5e15,), self.mapping)

    def test_wrong_count_raises(self):
        for expected in (1, 3):
            with self.subTest(expected=expected), self.assertRaisesRegex(ValueError, "expected"):
                textures.repoint(self.original, (1e13, 5e15), {OLD: (NEW, expected, False)})
        partial = model(lod(1e13, [OLD, NEW]), lod(5e15, [NEW, NEW]))
        fixed = textures.repoint(partial, (1e13, 5e15), self.mapping)
        self.assertEqual([[NEW, NEW], [NEW, NEW]], names(fixed))

    def test_bad_names_raise(self):
        for new in (b"x" * 32, b"", b"a\0b"):
            with self.subTest(new=new), self.assertRaisesRegex(ValueError, "Texture name"):
                textures.repoint(self.original, (1e13, 5e15), {OLD: (new, 2, False)})
        with self.assertRaisesRegex(ValueError, "Overflowed"):
            textures.repoint(self.original, (1e13, 5e15), {OLD: (NEW, 2, True)})

    def test_oxygen_path_trailers(self):
        codemast = b"H:" + BS.encode() + b"Codemast\0" + bytes(range(1, 21))
        self.assertEqual(32, len(codemast))
        with_trailer = self.original + codemast
        fixed = textures.repoint(with_trailer, (1e13, 5e15), self.mapping)
        self.assertEqual(codemast, fixed[-32:])
        for bad in (bytes(32), b"\x01" * 32, b"data" + BS.encode() + b"\0" + bytes(25), b"\x7f" + bytes(31)):
            with self.subTest(trailer=bad[:8]), self.assertRaises(ValueError):
                textures.repoint(self.original + bad, (1e13, 5e15), self.mapping)

    def test_repair_models_uses_plans_and_requires_members(self):
        plans = {"pack/model.p3d": ((1e13, 5e15), self.mapping)}
        with patch.object(textures, "PLANS", plans):
            changed = textures.repair_models({"pack/model.p3d": self.original})
            self.assertEqual(["pack/model.p3d"], list(changed))
            self.assertEqual({}, textures.repair_models(changed))
            with self.assertRaisesRegex(ValueError, "Missing model"):
                textures.repair_models({})
        self.assertEqual({}, textures.repair({"any/config.cpp": "x"}))

    def test_real_plans_are_well_formed(self):
        for key, (resolutions, mapping) in textures.PLANS.items():
            self.assertIn(key, textures.MODEL_MEMBERS)
            for old, (new, count, raw32) in mapping.items():
                self.assertLessEqual(len(new), 31)
                self.assertGreater(count, 0)
                if raw32:
                    self.assertEqual(32, len(old))
        self.assertEqual({"loboidfhelo", "lobojordan", "lobosyria"}, set(textures.ADDONS))


if __name__ == "__main__":
    unittest.main()
