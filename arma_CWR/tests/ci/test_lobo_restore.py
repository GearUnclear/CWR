"""Synthetic tests for repair.restore, the byte-for-byte restoration of members
LoBo references but its final release lacks (2026-10-11.4).

No LoBo bytes are used: the source archives and their pins are generated here
and patched over the module's tables. RealSourceTests checks the real library
pbos read-only when LOBO_RESTORE_SOURCES names them (os.pathsep separated).
"""
import hashlib
import json
import os
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import restore  # noqa: E402
from repair.pbo import Archive, CPRS  # noqa: E402
import repair_lobo as runner  # noqa: E402

BS = chr(92)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def make_pbo(entries, original=None):
    """Entries are (name bytes, stored bytes, packing); no trailer, as the LoBo releases."""
    header = [name + b"\0" + struct.pack("<5I", packing, (original or {}).get(name, 0), 0, 0, len(data))
              for name, data, packing in entries]
    return b"".join(header) + bytes(21) + b"".join(data for _, data, _ in entries)


def model(name):
    return b"MLOD" + ("synthetic " + name).encode() * 5


class RestoreTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix="lobo-restore-test-")
        self.addCleanup(temporary.cleanup)
        self.dir = Path(temporary.name)
        self.data = {key: model(key) for key in restore.MEMBERS}
        self.entries = {"pack3-airammo": [(b"config.cpp", b"class CfgPatches {};", 0)],
                        "pack1-egypt": [(b"config.cpp", b"class CfgPatches {};", 0)]}
        for key, (sid, stored, _, _) in restore.MEMBERS.items():
            self.entries[sid].append((stored.encode(), self.data[key], 0))
        self.paths = {sid: self.write(sid + ".pbo", make_pbo(entries)) for sid, entries in self.entries.items()}
        sources = {sid: (desc, {sha(self.paths[sid].read_bytes()): "synthetic"}, lib)
                   for sid, (desc, _, lib) in restore.SOURCES.items()}
        members = {key: (sid, stored, sha(self.data[key]), len(self.data[key]))
                   for key, (sid, stored, _, _) in restore.MEMBERS.items()}
        for active in (patch.dict(restore.SOURCES, sources), patch.dict(restore.MEMBERS, members)):
            active.start()
            self.addCleanup(active.stop)

    def write(self, name, data):
        path = self.dir / name
        path.write_bytes(data)
        return path

    def test_both_sources_restore_every_member_byte_for_byte_in_table_order(self):
        added, skipped = restore.add_members({}, tuple(self.paths.values()))
        self.assertEqual([], skipped)
        self.assertEqual(list(restore.MEMBERS), list(added))
        for key, (stored, data) in added.items():
            self.assertEqual(self.data[key], data)
            self.assertEqual(restore.MEMBERS[key][1], stored)
            self.assertEqual(key.split("/", 1)[1], stored.replace(BS, "/").lower())
        present = {key: data for key, (_, data) in added.items()}
        self.assertEqual(({}, []), restore.add_members(present, tuple(self.paths.values())))
        self.assertEqual(({}, []), restore.add_members(present, None))

    def test_each_missing_source_is_one_skip_line_and_the_other_still_restores(self):
        added, skipped = restore.add_members({}, None)
        self.assertEqual({}, added)
        self.assertEqual(2, len(skipped))
        self.assertTrue(skipped[0].startswith("lobo_airammo: 9 member(s) not restored (LoBo_Mk81.p3d, "))
        self.assertIn("LoBo_airammo-LoBoAddonPack3-2007-01-06.pbo", skipped[0])
        self.assertTrue(skipped[1].startswith("loboegypt: 1 member(s) not restored (helm_s_in.pac)"))
        added, skipped_one = restore.add_members({}, (self.paths["pack3-airammo"],))
        self.assertEqual([k for k, v in restore.MEMBERS.items() if v[0] == "pack3-airammo"], list(added))
        self.assertEqual(skipped[1:], skipped_one)
        present = {key: data for key, (_, data) in added.items()}
        self.assertEqual(({}, skipped[1:]), restore.add_members(present, (self.paths["pack3-airammo"],)))
        # A single path (direct call) works as the one-element tuple.
        self.assertEqual(restore.add_members({}, self.paths["pack1-egypt"]),
                         restore.add_members({}, (self.paths["pack1-egypt"],)))

    def test_wrong_sources_fail_closed_even_when_nothing_is_missing(self):
        airammo = self.entries["pack3-airammo"]
        first = airammo[1][0]
        cases = {
            "unpinned copy": (make_pbo(airammo + [(b"extra.txt", b"x", 0)]), "is not LoBo_airammo.pbo of LoBoAddonPack3"),
            "member tampered": (make_pbo([(n, b"tampered" if n == first else d, c) for n, d, c in airammo]), "not pinned"),
            "not a pbo": (b"synthetic", "not a readable PBO"),
        }
        everything = dict(self.data)
        for name, (data, message) in cases.items():
            wrong = self.write(name.replace(" ", "-") + ".pbo", data)
            for present in ({}, everything):
                with self.subTest(case=name, present=len(present)), self.assertRaisesRegex(ValueError, message):
                    restore.add_members(present, (wrong,))
        with self.assertRaisesRegex(ValueError, "Several --asset-source"):
            copy = self.write("copy.pbo", self.paths["pack3-airammo"].read_bytes())
            restore.add_members({}, (self.paths["pack3-airammo"], copy))

    def test_pinned_archive_with_a_bad_member_pin_or_layout_is_rejected(self):
        # The archive pin passes; the per-member checks still apply.
        key = next(iter(restore.MEMBERS))
        sid, stored, pin, size = restore.MEMBERS[key]
        for mode in ("absent", "compressed", "mismatch", "size"):
            entries = [e for e in self.entries[sid] if not (mode == "absent" and e[0] == stored.encode())]
            original = {stored.encode(): size * 2} if mode == "compressed" else None
            data = make_pbo(entries, original)
            path = self.write(mode + ".pbo", data)
            members = dict(restore.MEMBERS)
            if mode == "mismatch":
                members[key] = (sid, stored, sha(b"other"), size)
            if mode == "size":
                members[key] = (sid, stored, pin, size + 1)
            sources = dict(restore.SOURCES)
            sources[sid] = (sources[sid][0], {sha(data): "synthetic"}, sources[sid][2])
            with patch.dict(restore.MEMBERS, members), patch.dict(restore.SOURCES, sources):
                with self.subTest(mode=mode), self.assertRaisesRegex(ValueError, "asset-source"):
                    restore.add_members({}, (path,))

    def test_present_member_must_carry_its_pin(self):
        key = next(iter(restore.MEMBERS))
        with self.assertRaisesRegex(ValueError, "present but not the pinned"):
            restore.add_members({key: b"different"}, None)
        with self.assertRaisesRegex(ValueError, "Unknown added"):
            restore.add_members({"lobo_airammo/other.p3d": b"x"}, None)

    def test_claims_its_pinned_archives_and_lookalikes_only(self):
        for path in self.paths.values():
            self.assertTrue(restore.claims_source(Archive(path.read_bytes())))
        lookalike = make_pbo([(b"LoBo_Mk81.p3d", b"another bomb", 0)])
        self.assertTrue(restore.claims_source(Archive(lookalike)))
        self.assertFalse(restore.claims_source(Archive(make_pbo([(b"inventory\\w_msg90.paa", b"x", 0)]))))
        self.assertFalse(restore.claims_source(Archive(make_pbo([(b"LoBo_Mk82.p3d", b"x", 0)]))))

    def test_plan_restores_into_a_synthetic_mod_and_reaches_a_fixed_point(self):
        mod = self.dir / "mod"
        (mod / "addons").mkdir(parents=True)
        originals = {}
        for stem, name in (("lobo_airammo", "LoBo_airammo.pbo"), ("loboegypt", "LoBoEgypt.pbo")):
            raw = make_pbo([(b"config.cpp", b"class CfgPatches {};", 0), (b"tex\\bomb.pac", b"bomb", 0)])
            (mod / "addons" / name).write_bytes(raw)
            originals[stem] = raw
        with patch.object(runner, "modules", return_value=[restore]):
            paths, hashes, changed, records, _, skipped = runner.plan(mod, None, asset_source=list(self.paths.values()))
            self.assertEqual([], skipped)
            self.assertEqual(set(restore.MEMBERS), set(changed))
            self.assertTrue(all(r["added"] for r in records))
            manifest = runner.apply(mod, paths, hashes, changed, records)
            self.assertEqual("applied", json.loads(manifest.read_text())["state"])
            airammo = Archive((mod / "addons" / "LoBo_airammo.pbo").read_bytes())
            self.assertEqual([n.encode() for k, (s, n, _, _) in restore.MEMBERS.items() if s == "pack3-airammo"],
                             [e.name for e in airammo.entries[2:]])
            egypt = Archive((mod / "addons" / "LoBoEgypt.pbo").read_bytes())
            self.assertEqual(self.data["loboegypt/inf/helm_s_in.pac"], egypt.read("inf/helm_s_in.pac"))
            for sources in (list(self.paths.values()), None):
                again = runner.plan(mod, None, asset_source=sources)
                self.assertEqual(({}, []), (again[2], again[5]))
        for stem, name in (("lobo_airammo", "LoBo_airammo.pbo"), ("loboegypt", "LoBoEgypt.pbo")):
            self.assertEqual(originals[stem], (manifest.parent / name).read_bytes())


class PinTests(unittest.TestCase):
    def test_tables(self):
        self.assertEqual(("lobo_airammo", "loboegypt"), restore.ADDONS)
        self.assertEqual(tuple(restore.MEMBERS), restore.ADDED_MEMBERS)
        self.assertEqual(10, len(restore.MEMBERS))
        for key, (sid, stored, pin, size) in restore.MEMBERS.items():
            with self.subTest(key=key):
                self.assertIn(sid, restore.SOURCES)
                self.assertEqual(key.split("/", 1)[1], stored.replace(BS, "/").lower())
                self.assertRegex(pin, "^[0-9a-f]{64}$")
                self.assertGreater(size, 0)


@unittest.skipUnless(os.environ.get("LOBO_RESTORE_SOURCES"),
                     "set LOBO_RESTORE_SOURCES to the derived source pbos to run (read-only)")
class RealSourceTests(unittest.TestCase):
    def test_real_sources_give_the_pinned_members(self):
        paths = tuple(Path(p) for p in os.environ["LOBO_RESTORE_SOURCES"].split(os.pathsep) if p)
        added, skipped = restore.add_members({}, paths)
        want = {sid for p in paths for sid, (_, pins, _) in restore.SOURCES.items() if sha(p.read_bytes()) in pins}
        self.assertEqual({k for k, v in restore.MEMBERS.items() if v[0] in want}, set(added))
        for key, (stored, data) in added.items():
            self.assertEqual(restore.MEMBERS[key][2], sha(data))
            self.assertEqual(restore.MEMBERS[key][3], len(data))
        self.assertEqual(len(restore.SOURCES) - len(want), len(skipped))


if __name__ == "__main__":
    unittest.main()
