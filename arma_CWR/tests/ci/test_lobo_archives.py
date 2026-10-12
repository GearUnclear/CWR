"""Archive transaction tests using generated data only; never opens installed LoBo."""

import contextlib
import hashlib
import io
import json
from pathlib import Path
import struct
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair.pbo import Archive, CPRS, VERS, normalize  # noqa: E402
import repair_lobo as runner  # noqa: E402


def make_pbo(entries, *, properties=(), trailer=None):
    """Entries are (name, stored bytes, packing, original size, reserved, time)."""
    header = []
    if properties:
        header += [b"\0", struct.pack("<5I", VERS, 0, 0, 0, 0)]
        for key, value in properties:
            header += [key + b"\0", value + b"\0"]
        header.append(b"\0")
    for name, stored, packing, original, reserved, timestamp in entries:
        header += [name + b"\0", struct.pack("<5I", packing, original, reserved, timestamp, len(stored))]
    body = b"".join(header) + bytes(21) + b"".join(entry[1] for entry in entries)
    return body + (b"\0" + hashlib.sha1(body).digest() if trailer is None else trailer)


def sample_pbo(value=b"before"):
    return make_pbo([
        (b"Config.cpp", value, 0, 0, 37, 123456),
        (b"Models\\Compressed.p3d", b"FAKE-COMPRESSED-PAYLOAD", CPRS, 512, 42, 654321),
        (b"README.txt", b"unmodified asset notes\r\n", 0, 0, 3, 33),
    ], properties=((b"prefix", b"SyntheticAddon"), (b"author", b"test")))


def rule(stems=("first", "second"), *, bad=False):
    def repair(documents):
        edits = {}
        for stem in stems:
            key = stem + "/config.cpp"
            if documents[key] not in ("before", "after"):
                raise ValueError("Unexpected synthetic source")
            if documents[key] == "before":
                edits[key] = "after"
        if bad:
            edits[stems[0] + "/unknown.sqs"] = "exit"
        return edits
    return SimpleNamespace(ADDONS=stems, repair=repair, __name__="synthetic-rule")


def adder(stem="first", *, key=None, sticky=False):
    """Synthetic module that adds one member, only when a source is supplied."""
    declared = stem + "/inventory/new.paa"
    key = key or declared
    def add_members(existing, asset_source=None):
        if declared in existing and not sticky:
            return {}, []
        if asset_source is None:
            return {}, [f"{stem}: 1 icon not added: pass --asset-source"]
        return {key: ("Inventory\\new.paa", b"pinned icon bytes")}, []
    return SimpleNamespace(ADDONS=(stem,), ADDED_MEMBERS=(declared,), repair=lambda documents: {},
                           add_members=add_members, __name__="synthetic-adder")


class ArchivePreservationTests(unittest.TestCase):
    def test_replacement_preserves_metadata_opaque_payloads_and_properties(self):
        original = sample_pbo()
        archive = Archive(original)
        changed = archive.replace({"config.cpp": b"longer replacement config\r\n"})
        reread = Archive(changed)
        self.assertEqual(archive.prefix, reread.prefix)
        self.assertEqual(archive.terminator, reread.terminator)
        self.assertEqual([e.name for e in archive.entries], [e.name for e in reread.entries])
        for key in ("models/compressed.p3d", "readme.txt"):
            before, after = archive.by_key[key], reread.by_key[key]
            self.assertEqual(before.fields, after.fields)
            self.assertEqual(bytes(archive.stored(before)), bytes(reread.stored(after)))
        patched = reread.by_key["config.cpp"]
        self.assertEqual((0, 0, 37, 123456, 27), patched.fields)
        self.assertEqual(b"longer replacement config\r\n", reread.read("Config.cpp"))
        self.assertEqual(hashlib.sha1(changed[:-21]).digest(), changed[-20:])

    def test_noop_rebuild_is_byte_identical(self):
        data = sample_pbo()
        self.assertEqual(data, Archive(data).replace({}))

    def test_opaque_trailer_is_preserved_exactly(self):
        data = make_pbo([(b"config.cpp", b"before", 0, 0, 1, 2)], trailer=b"Synthetic old packer signature\n")
        changed = Archive(data).replace({"config.cpp": b"after"})
        self.assertEqual(Archive(data).trailer, Archive(changed).trailer)

    def test_stuffpbo_tail_is_readable_but_modification_is_rejected(self):
        # Observed legacy format: unknown two-byte checksum + packer signature.
        # Do not pretend its checksum is valid after changing a payload.
        tail = b"\x00\xff\x01StuffPBO V1.0b"
        self.assertEqual(17, len(tail))
        data = make_pbo([(b"config.cpp", b"before", 0, 0, 1, 2)], trailer=tail)
        archive = Archive(data)
        self.assertTrue(archive.stuffpbo)
        self.assertEqual(b"before", archive.read("config.cpp"))
        self.assertEqual(data, archive.replace({}))
        with self.assertRaisesRegex(ValueError, "StuffPBO checksum rewriting is unsupported"):
            archive.replace({"config.cpp": b"after"})

    def test_stuffpbo_same_length_uncompressed_replacement_is_in_place(self):
        tail = b"\x12\x34\x01StuffPBO V1.0b"
        data = make_pbo([(b"config.cpp", b"crew=\"Before\";", 0, 0, 1, 2),
                         (b"model.p3d", b"COMPRESSED", CPRS, 64, 3, 4),
                         (b"readme.txt", b"untouched", 0, 0, 5, 6)], trailer=tail)
        archive = Archive(data)
        changed = archive.replace({"config.cpp": b"crew=\"After\"; "})
        self.assertEqual(len(data), len(changed))
        reread = Archive(changed)
        self.assertTrue(reread.stuffpbo)
        self.assertEqual(tail, changed[-17:])
        self.assertEqual(data[:archive.header_end], changed[:reread.header_end])
        self.assertEqual(b"crew=\"After\"; ", reread.read("config.cpp"))
        diff = [i for i, (a, b) in enumerate(zip(data, changed)) if a != b]
        entry = archive.by_key["config.cpp"]
        self.assertTrue(all(entry.offset <= i < entry.offset + entry.fields[4] for i in diff))
        self.assertEqual(changed, Archive(changed).replace({"config.cpp": b"crew=\"After\"; "}))

    def test_stuffpbo_rejects_length_change_compressed_member_and_additions(self):
        tail = b"\x12\x34\x01StuffPBO V1.0b"
        data = make_pbo([(b"config.cpp", b"before", 0, 0, 1, 2),
                         (b"model.p3d", b"COMPRESSED", CPRS, 64, 3, 4)], trailer=tail)
        archive = Archive(data)
        for replacements, additions in (({"config.cpp": b"longer!"}, None),
                                        ({"model.p3d": b"DECODED!!!"}, None),
                                        ({}, {"extra.txt": b"x"}),
                                        ({"config.cpp": b"after_"}, {"extra.txt": b"x"})):
            with self.subTest(replacements=replacements, additions=additions):
                with self.assertRaisesRegex(ValueError, "StuffPBO checksum rewriting is unsupported"):
                    archive.replace(replacements, additions)

    def test_added_members_are_appended_after_existing_records_and_payloads(self):
        original = sample_pbo()
        archive = Archive(original)
        added = {"Inventory\\w_new.paa": b"new icon bytes", "Inventory\\m_new.paa": b"magazine"}
        changed = archive.replace({"config.cpp": b"after"}, added)
        reread = Archive(changed)
        self.assertEqual(list(archive.by_key) + ["inventory/w_new.paa", "inventory/m_new.paa"], list(reread.by_key))
        self.assertEqual([b"Inventory\\w_new.paa", b"Inventory\\m_new.paa"], [e.name for e in reread.entries[-2:]])
        for name, data in added.items():
            entry = reread.by_key[normalize(name)]
            self.assertEqual((0, 0, 0, 0, len(data)), entry.fields)
            self.assertEqual(data, reread.read(name))
        self.assertEqual(archive.prefix, reread.prefix)
        for key in ("models/compressed.p3d", "readme.txt"):
            self.assertEqual(archive.by_key[key].fields, reread.by_key[key].fields)
            self.assertEqual(bytes(archive.stored(archive.by_key[key])), bytes(reread.stored(reread.by_key[key])))
        self.assertEqual(hashlib.sha1(changed[:-21]).digest(), changed[-20:])
        # Layout: the replace() rebuild, then the new records, terminator, payloads.
        base = archive.replace({"config.cpp": b"after"})
        records = b"".join(name.encode() + b"\0" + struct.pack("<5I", 0, 0, 0, 0, len(data)) for name, data in added.items())
        header_end = Archive(base).header_end - 21
        expected = base[:header_end] + records + base[header_end:-21] + b"".join(added.values())
        self.assertEqual(expected, changed[:-21])

    def test_added_members_keep_a_textual_trailer(self):
        data = make_pbo([(b"config.cpp", b"before", 0, 0, 1, 2)], trailer=b" ** Synthetic packer ** ")
        changed = Archive(data).replace({}, {"new.txt": b"x"})
        self.assertEqual(b" ** Synthetic packer ** ", Archive(changed).trailer)
        self.assertEqual(b"x", Archive(changed).read("new.txt"))

    def test_invalid_additions_rejected(self):
        archive = Archive(sample_pbo())
        for additions in ({"README.TXT": b"x"}, {"a/b.txt": b"x", "A\\B.txt": b"y"}, {"../x": b"x"},
                          {"x:y": b"x"}, {"a//b": b"x"}):
            with self.subTest(additions=additions), self.assertRaises(ValueError):
                archive.replace({}, additions)
        with self.assertRaises(TypeError):
            archive.replace({}, {"new.txt": "text"})

    def test_corrupt_sha1_tail_is_rejected(self):
        data = bytearray(sample_pbo())
        data[-1] ^= 0x80
        with self.assertRaisesRegex(ValueError, "checksum mismatch"):
            Archive(bytes(data))

    def test_empty_property_values_are_valid_and_preserved(self):
        data = make_pbo([(b"config.cpp", b"before", 0, 0, 1, 2)],
                        properties=((b"author", b""), (b"prefix", b"Synthetic")))
        archive = Archive(data)
        changed = Archive(archive.replace({"config.cpp": b"after"}))
        self.assertEqual(archive.prefix, changed.prefix)
        self.assertEqual(b"after", changed.read("config.cpp"))

    def test_modified_compressed_member_becomes_uncompressed_without_other_changes(self):
        archive = Archive(sample_pbo())
        changed = Archive(archive.replace({"models/compressed.p3d": b"decoded model"}))
        entry = changed.by_key["models/compressed.p3d"]
        self.assertEqual((0, 0, 42, 654321, 13), entry.fields)
        self.assertEqual(b"decoded model", changed.read("models/compressed.p3d"))
        self.assertEqual(archive.read("config.cpp"), changed.read("config.cpp"))

    def test_compressed_read_requires_decoder_and_matching_size(self):
        archive = Archive(sample_pbo())
        temporary = tempfile.TemporaryDirectory(prefix="lobo-compressed-test-")
        self.addCleanup(temporary.cleanup)
        path = Path(temporary.name) / "fake.pbo"
        path.write_bytes(archive.data)
        with self.assertRaisesRegex(ValueError, "needs PoseidonTools"):
            archive.read("models/compressed.p3d")
        fake = SimpleNamespace(returncode=0, stdout=b"x" * 512, stderr=b"")
        with patch("repair.pbo.subprocess.run", return_value=fake) as run:
            self.assertEqual(b"x" * 512, archive.read("models/compressed.p3d", path, Path("tool.exe")))
            self.assertEqual("pbo", run.call_args.args[0][1])
            self.assertNotIn("shell", run.call_args.kwargs)
        with patch("repair.pbo.subprocess.run", return_value=SimpleNamespace(returncode=0, stdout=b"short", stderr=b"")):
            with self.assertRaisesRegex(ValueError, "size mismatch"):
                archive.read("models/compressed.p3d", path, Path("tool.exe"))
        with patch("repair.pbo.subprocess.run", return_value=SimpleNamespace(returncode=3, stdout=b"", stderr=b"bad stream")):
            with self.assertRaisesRegex(ValueError, "Cannot decode"):
                archive.read("models/compressed.p3d", path, Path("tool.exe"))

    def test_compressed_read_rejects_concurrent_archive_edit(self):
        temporary = tempfile.TemporaryDirectory(prefix="lobo-compressed-test-")
        self.addCleanup(temporary.cleanup)
        path = Path(temporary.name) / "fake.pbo"
        original = sample_pbo()
        path.write_bytes(original)
        def concurrent_decoder(*args, **kwargs):
            path.write_bytes(sample_pbo(b"concurrent config"))
            return SimpleNamespace(returncode=0, stdout=b"x" * 512, stderr=b"")
        with patch("repair.pbo.subprocess.run", side_effect=concurrent_decoder):
            with self.assertRaisesRegex(ValueError, "changed during compressed read"):
                Archive(original).read("models/compressed.p3d", path, Path("tool.exe"))

    def test_short_checksum_trailer_rejected(self):
        data = sample_pbo()
        with self.assertRaises(ValueError):
            Archive(data[:-1])

    def test_unknown_or_nonbyte_replacements_rejected(self):
        archive = Archive(sample_pbo())
        with self.assertRaises(ValueError):
            archive.replace({"missing.cpp": b"data"})
        with self.assertRaises(TypeError):
            archive.replace({"config.cpp": "not bytes"})

    def test_duplicate_replacement_aliases_rejected(self):
        with self.assertRaises(ValueError):
            Archive(sample_pbo()).replace({"Config.cpp": b"one", "config.cpp": b"two"})

    def test_duplicate_archive_entries_rejected(self):
        for names in ((b"config.cpp", b"CONFIG.CPP"), (b"a\\b.txt", b"a/b.txt")):
            data = make_pbo([(name, b"x", 0, 0, 0, 0) for name in names])
            with self.subTest(names=names), self.assertRaisesRegex(ValueError, "Duplicate"):
                Archive(data)

    def test_unsafe_and_ambiguous_paths_rejected(self):
        for name in ("../config.cpp", "/config.cpp", "C:/config.cpp", "a/./b", "a/../b", "a//b", "a/"):
            with self.subTest(name=name), self.assertRaises(ValueError):
                Archive(make_pbo([(name.encode(), b"x", 0, 0, 0, 0)]))
        self.assertEqual("a/b.txt", normalize("A\\B.TXT"))

    def test_truncated_headers_properties_and_payloads_rejected(self):
        malformed = [b"no nul", b"name\0" + bytes(19), b"\0" + struct.pack("<5I", VERS, 0, 0, 0, 0) + b"prefix\0unterminated"]
        data = make_pbo([(b"config.cpp", b"payload", 0, 0, 0, 0)], trailer=b"")
        malformed.append(data[:-1])
        for data in malformed:
            with self.subTest(data=data[:30]), self.assertRaises(ValueError):
                Archive(data)


class RepairTransactionTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="lobo-archive-test-")
        self.addCleanup(self.temporary.cleanup)
        self.mod = Path(self.temporary.name) / "synthetic-mod"
        self.addons = self.mod / "addons"
        self.addons.mkdir(parents=True)
        self.original = {}
        for stem in ("first", "second"):
            raw = sample_pbo()
            (self.addons / (stem + ".pbo")).write_bytes(raw)
            self.original[stem] = raw
        self.rules = patch.object(runner, "modules", return_value=[rule()])
        self.rules.start()
        self.addCleanup(self.rules.stop)

    def plan(self):
        return runner.plan(self.mod, None)

    def installed(self):
        return {stem: (self.addons / (stem + ".pbo")).read_bytes() for stem in self.original}

    def test_plan_does_not_write_and_apply_has_verified_original_backups(self):
        paths, hashes, changed, records, documents, skipped = self.plan()
        self.assertEqual(self.original, self.installed())
        self.assertFalse((self.mod / ".ud-backups").exists())
        self.assertEqual(2, len(changed))
        manifest_path = runner.apply(self.mod, paths, hashes, changed, records)
        manifest = json.loads(manifest_path.read_text())
        self.assertEqual("applied", manifest["state"])
        self.assertEqual(2, len(manifest["archives"]))
        for stem, raw in self.original.items():
            self.assertEqual(raw, (manifest_path.parent / (stem + ".pbo")).read_bytes())
            self.assertEqual(b"after", Archive(self.installed()[stem]).read("config.cpp"))
        next_plan = self.plan()
        self.assertEqual({}, next_plan[2])
        self.assertIsNone(runner.apply(self.mod, *next_plan[:4]))

    def test_unknown_source_fails_before_any_archive_write(self):
        (self.addons / "second.pbo").write_bytes(sample_pbo(b"unrecognized"))
        before = self.installed()
        with self.assertRaisesRegex(ValueError, "Unexpected synthetic source"):
            self.plan()
        self.assertEqual(before, self.installed())
        self.assertFalse((self.mod / ".ud-backups").exists())

    def test_unknown_rule_member_and_nonidempotent_rule_rejected(self):
        with patch.object(runner, "modules", return_value=[rule(bad=True)]):
            with self.assertRaisesRegex(ValueError, "unknown text member"):
                self.plan()
        toggle = SimpleNamespace(ADDONS=("first",), __name__="toggle", repair=lambda docs: {"first/config.cpp": "after" if docs["first/config.cpp"] == "before" else "before"})
        with patch.object(runner, "modules", return_value=[toggle]):
            with self.assertRaisesRegex(ValueError, "Non-idempotent"):
                self.plan()
        self.assertEqual(self.original, self.installed())

    def test_requested_profile_reaches_initial_and_fixed_point_script_pass(self):
        observed = []
        def repair(documents, compatibility="ud"):
            observed.append(compatibility)
            key = "first/config.cpp"
            return {key: compatibility} if documents[key] != compatibility else {}
        module = SimpleNamespace(ADDONS=("first",), __name__="repair.scripts", repair=repair)
        with patch.object(runner, "modules", return_value=[module]):
            plan = runner.plan(self.mod, None, compatibility="classic")
        self.assertEqual(["classic", "classic"], observed)
        self.assertEqual({"first/config.cpp": b"classic"}, plan[2])
        manifest_path = runner.apply(self.mod, *plan[:4], compatibility="classic")
        self.assertEqual("classic", json.loads(manifest_path.read_text())["compatibility"])

    def test_added_member_is_planned_applied_and_reaches_a_fixed_point(self):
        source = Path(self.temporary.name) / "source.pbo"
        source.write_bytes(b"synthetic source")
        with patch.object(runner, "modules", return_value=[rule(), adder()]):
            paths, hashes, changed, records, _, skipped = runner.plan(self.mod, None, asset_source=source)
            self.assertEqual([], skipped)
            self.assertEqual(b"pinned icon bytes", changed["first/inventory/new.paa"])
            record = next(r for r in records if r["member"] == "first/inventory/new.paa")
            self.assertEqual({"added": True, "stored_name": "Inventory\\new.paa", "before_sha256": None,
                              "before_bytes": 0, "after_bytes": 17}, {k: record[k] for k in
                              ("added", "stored_name", "before_sha256", "before_bytes", "after_bytes")})
            manifest_path = runner.apply(self.mod, paths, hashes, changed, records)
            manifest = json.loads(manifest_path.read_text())
            self.assertIn(record, manifest["members"])
            installed = Archive(self.installed()["first"])
            self.assertEqual("inventory/new.paa", list(installed.by_key)[-1])
            self.assertEqual(b"pinned icon bytes", installed.read("inventory/new.paa"))
            self.assertEqual(b"after", installed.read("config.cpp"))
            self.assertNotIn("inventory/new.paa", Archive(self.installed()["second"]).by_key)
            # Restoring the batch's originals removes the added member again.
            self.assertEqual(self.original["first"], (manifest_path.parent / "first.pbo").read_bytes())
            for asset in (source, None):
                again = runner.plan(self.mod, None, asset_source=asset)
                self.assertEqual(({}, []), (again[2], again[5]))

    def test_missing_source_is_reported_not_fatal(self):
        with patch.object(runner, "modules", return_value=[adder()]):
            _, _, changed, records, _, skipped = runner.plan(self.mod, None)
        self.assertEqual({}, changed)
        self.assertEqual(["first: 1 icon not added: pass --asset-source"], skipped)

    def test_weapons_text_pass_sees_the_members_present_after_the_plan(self):
        # The MSG90 picture is repointed only when its icon exists after the plan.
        source = Path(self.temporary.name) / "source.pbo"
        source.write_bytes(b"synthetic source")
        observed = []
        def repair(documents, present=frozenset()):
            observed.append(present)
            return {}
        gated = SimpleNamespace(ADDONS=("first",), __name__="repair.weapons", repair=repair)
        with patch.object(runner, "modules", return_value=[adder(), gated]):
            runner.plan(self.mod, None)
            self.assertEqual([frozenset()] * 2, observed)
            observed.clear()
            paths, hashes, changed, records, _, _ = runner.plan(self.mod, None, asset_source=source)
            self.assertEqual([frozenset({"first/inventory/new.paa"})] * 2, observed)
            runner.apply(self.mod, paths, hashes, changed, records)
            observed.clear()
            runner.plan(self.mod, None)
            self.assertEqual([frozenset({"first/inventory/new.paa"})] * 2, observed)

    def test_undeclared_existing_or_nonidempotent_additions_rejected(self):
        source = Path(self.temporary.name) / "source.pbo"
        source.write_bytes(b"synthetic source")
        for module, message in ((adder(key="first/other.paa"), "undeclared"),
                                (adder(key="first/config.cpp"), "undeclared"),
                                (adder(sticky=True), "Non-idempotent")):
            with self.subTest(message=message), patch.object(runner, "modules", return_value=[module]):
                with self.assertRaisesRegex(ValueError, message):
                    runner.plan(self.mod, None, asset_source=source)
        existing = SimpleNamespace(ADDONS=("first",), ADDED_MEMBERS=("first/readme.txt",), repair=lambda d: {},
                                   add_members=lambda e, s=None: ({"first/readme.txt": ("README.txt", b"x")}, []),
                                   __name__="existing")
        with patch.object(runner, "modules", return_value=[existing]):
            with self.assertRaisesRegex(ValueError, "existing member"):
                runner.plan(self.mod, None, asset_source=source)
        self.assertEqual(self.original, self.installed())

    def test_summary_reports_skips_additions_and_check_exit_codes(self):
        source = Path(self.temporary.name) / "source.pbo"
        source.write_bytes(b"synthetic source")
        def run(*extra):
            out, err = io.StringIO(), io.StringIO()
            with patch.object(runner, "modules", return_value=[adder()]), \
                    contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
                code = runner.main(["--mod", str(self.mod), "--check", *extra])
            return code, json.loads(out.getvalue()), err.getvalue()
        code, summary, err = run()
        self.assertEqual((0, [], 0), (code, summary["archives"], summary["added_members"]))
        self.assertEqual(["first: 1 icon not added: pass --asset-source"], summary["skipped"])
        self.assertIn("WARN: first: 1 icon not added", err)
        self.assertNotIn("asset_source_sha256", summary)
        code, summary, err = run("--asset-source", str(source))
        self.assertEqual((1, 1, 1, []), (code, summary["changed_members"], summary["added_members"], summary["skipped"]))
        self.assertEqual(runner.digest(b"synthetic source"), summary["asset_source_sha256"])
        self.assertEqual("", err)
        self.assertEqual("2026-10-11.4", summary["repair_version"])

    def test_archive_changed_after_preflight_is_rejected(self):
        paths, hashes, changed, records, _, _ = self.plan()
        external = sample_pbo(b"external change")
        (self.addons / "second.pbo").write_bytes(external)
        with self.assertRaisesRegex(ValueError, "changed since preflight"):
            runner.apply(self.mod, paths, hashes, changed, records)
        self.assertEqual(self.original["first"], self.installed()["first"])
        self.assertEqual(external, self.installed()["second"])
        self.assertFalse((self.mod / ".ud-backups").exists())

    def test_failure_during_second_replacement_rolls_back_first(self):
        paths, hashes, changed, records, _, _ = self.plan()
        original_replace = Path.replace
        def fail_second(source, target):
            if source.name == "second.pbo" and source.parent != self.addons:
                raise OSError("simulated replace failure")
            return original_replace(source, target)
        with patch.object(Path, "replace", fail_second):
            with self.assertRaisesRegex(OSError, "simulated replace failure"):
                runner.apply(self.mod, paths, hashes, changed, records)
        self.assertEqual(self.original, self.installed())
        manifests = list((self.mod / ".ud-backups").rglob("manifest.json"))
        self.assertEqual(1, len(manifests))
        self.assertEqual("rolled-back", json.loads(manifests[0].read_text())["state"])

    def test_rollback_never_overwrites_a_later_concurrent_edit(self):
        paths, hashes, changed, records, _, _ = self.plan()
        original_replace = Path.replace
        external = sample_pbo(b"external edit after first replacement")
        def fail_with_concurrent_edit(source, target):
            if source.name == "second.pbo" and source.parent != self.addons:
                (self.addons / "first.pbo").write_bytes(external)
                raise OSError("second replacement blocked")
            return original_replace(source, target)
        with patch.object(Path, "replace", fail_with_concurrent_edit):
            with self.assertRaises((OSError, ValueError)):
                runner.apply(self.mod, paths, hashes, changed, records)
        self.assertEqual(external, self.installed()["first"])
        self.assertEqual(self.original["second"], self.installed()["second"])
        manifests = list((self.mod / ".ud-backups").rglob("manifest.json"))
        self.assertEqual(1, len(manifests))
        self.assertNotEqual("rolled-back", json.loads(manifests[0].read_text())["state"])
        self.assertEqual(self.original["first"], (manifests[0].parent / "first.pbo").read_bytes())


if __name__ == "__main__":
    unittest.main()
