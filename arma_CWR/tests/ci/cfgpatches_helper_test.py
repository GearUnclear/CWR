"""lobo_cfgpatches.py tests using generated data only; never opens installed LoBo.

Run: python -m unittest discover -s arma_CWR/tests/ci -p "cfgpatches_helper_test.py" -v
(POSEIDON_TOOLS=<PoseidonTools.exe> enables the config.bin case outside a build tree.)
(Named outside the test_lobo_*.py pattern on purpose while the repair tests
are being reworked; rename to test_lobo_cfgpatches.py to put it in the
lobo-repair CI lane.)
"""

import json
from pathlib import Path
import io
import os
import struct
import sys
import tempfile
import unittest
from contextlib import redirect_stdout

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
import lobo_cfgpatches as helper  # noqa: E402

VERS = 0x56657273
TOOLS = Path(os.environ.get("POSEIDON_TOOLS", Path(__file__).resolve().parents[2] / "dist" / "x64-win-rwdi" / "PoseidonTools.exe"))


def make_pbo(entries, prefix=None):
    """Entries are (name, data, packing, original size)."""
    header = []
    if prefix is not None:
        header += [b"\0", struct.pack("<5I", VERS, 0, 0, 0, 0), b"prefix\0", prefix + b"\0", b"\0"]
    for name, data, packing, original in entries:
        header += [name + b"\0", struct.pack("<5I", packing, original, 0, 0, len(data))]
    return b"".join(header) + bytes(21) + b"".join(e[1] for e in entries)


class CfgPatchesTextTests(unittest.TestCase):
    def test_top_level_classes_with_bodies_only(self):
        text = (
            "#define public 2\n"
            "// class CfgPatches { class Commented {}; };\n"
            "/* class CfgPatches { class Blocked {}; }; */\n"
            "class CfgPatches\n{\n"
            "    class Forward;\n"
            "    class Real_One : Base { units[] = {\"class Fake {\"}; requiredAddons[] = {}; };\n"
            "    class Real_Two { class Nested {}; };\n"
            "};\n"
            "class CfgVehicles { class NotAPatch {}; };\n"
            "class CfgPatches { class Real_Three {}; };\n"
        )
        self.assertEqual(helper.cfgpatches_names(text), ["Real_One", "Real_Two", "Real_Three"])

    def test_case_insensitive_section_and_dedup(self):
        text = "class cfgpatches { class A {}; class a {}; };"
        self.assertEqual(helper.cfgpatches_names(text), ["A"])

    def test_nested_cfgpatches_is_not_top_level(self):
        text = "class Outer { class CfgPatches { class Hidden {}; }; };"
        self.assertEqual(helper.cfgpatches_names(text), [])

    def test_directive_continuation_and_doubled_quotes(self):
        text = (
            "#define LONG(a) \\\n  class a {};\n"
            "class CfgPatches { class P { name = \"say \"\"hi\"\" // not a comment\"; }; };"
        )
        self.assertEqual(helper.cfgpatches_names(text), ["P"])


class PboReaderTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.dir = Path(self.tmp.name)

    def tearDown(self):
        self.tmp.cleanup()

    def write(self, name, data):
        path = self.dir / name
        path.write_bytes(data)
        return path

    def run_main(self, *args):
        out = io.StringIO()
        with redirect_stdout(out):
            code = helper.main(list(args))
        return code, json.loads(out.getvalue())

    def test_config_cpp_with_prefix(self):
        cfg = b"class CfgPatches { class Alpha {}; };"
        path = self.write("a.pbo", make_pbo([(b"data.bin", b"x" * 10, 0, 0), (b"config.cpp", cfg, 0, 0)], prefix=b"alpha"))
        code, result = self.run_main(str(path))
        self.assertEqual(code, 0)
        self.assertEqual(result["pbos"][0], {"path": str(path), "config": "config.cpp", "cfgpatches": ["Alpha"], "error": None})

    def test_config_cpp_wins_over_config_bin_and_subfolder_configs_are_ignored(self):
        path = self.write("b.pbo", make_pbo([
            (b"sub\\config.cpp", b"class CfgPatches { class Sub {}; };", 0, 0),
            (b"config.bin", b"\0raP garbage", 0, 0),
            (b"CONFIG.CPP", b"class CfgPatches { class Top {}; };", 0, 0),
        ]))
        code, result = self.run_main(str(path))
        self.assertEqual(code, 0)
        self.assertEqual(result["pbos"][0]["cfgpatches"], ["Top"])

    def test_no_config_is_empty_not_an_error(self):
        path = self.write("c.pbo", make_pbo([(b"tex\\a.paa", b"\1" * 32, 0, 0)]))
        code, result = self.run_main(str(path))
        self.assertEqual(code, 0)
        self.assertEqual(result["pbos"][0]["config"], None)
        self.assertEqual(result["pbos"][0]["cfgpatches"], [])

    def test_damaged_pbo_is_reported_and_others_still_read(self):
        bad = self.write("bad.pbo", b"config.cpp\0\1\2")
        short = self.write("short.pbo", make_pbo([(b"config.cpp", b"class CfgPatches { class S {}; };", 0, 0)])[:-5])
        good = self.write("good.pbo", make_pbo([(b"config.cpp", b"class CfgPatches { class G {}; };", 0, 0)]))
        code, result = self.run_main(str(bad), str(short), str(good))
        self.assertEqual(code, 3)
        by_path = {p["path"]: p for p in result["pbos"]}
        self.assertIn("truncated", by_path[str(bad)]["error"])
        self.assertIn("exceeds", by_path[str(short)]["error"])
        self.assertEqual(by_path[str(good)]["cfgpatches"], ["G"])

    def test_compressed_member_without_tools_is_an_error(self):
        path = self.write("z.pbo", make_pbo([(b"config.cpp", b"\0" * 8, 0x43707273, 40)]))
        code, result = self.run_main(str(path))
        self.assertEqual(code, 3)
        self.assertIn("no --tools", result["pbos"][0]["error"])

    def test_reading_never_writes_next_to_the_pbo(self):
        path = self.write("r.pbo", make_pbo([(b"config.cpp", b"class CfgPatches { class R {}; };", 0, 0)]))
        before = sorted(os.listdir(self.dir))
        self.run_main(str(path))
        self.assertEqual(sorted(os.listdir(self.dir)), before)

    @unittest.skipUnless(TOOLS.is_file(), "PoseidonTools build not found")
    def test_config_bin_is_debinarized_with_poseidontools(self):
        import subprocess
        cpp = self.dir / "in.cpp"
        out = self.dir / "out.bin"
        cpp.write_text("class CfgPatches { class BinOne { units[] = {}; }; class BinTwo {}; };\n", encoding="latin1")
        subprocess.run([str(TOOLS), "config", "bin", str(cpp), "-o", str(out)], check=True, capture_output=True)
        path = self.write("bin.pbo", make_pbo([(b"config.bin", out.read_bytes(), 0, 0)]))
        code, result = self.run_main("--tools", str(TOOLS), str(path))
        self.assertEqual(code, 0)
        self.assertEqual(result["pbos"][0]["config"], "config.bin")
        self.assertEqual(result["pbos"][0]["cfgpatches"], ["BinOne", "BinTwo"])


if __name__ == "__main__":
    unittest.main()
