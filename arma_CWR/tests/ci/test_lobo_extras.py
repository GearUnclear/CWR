"""@LoBo_Extras build tests on synthetic data only; never opens the recovered downloads."""

import contextlib
import dataclasses
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest import mock

TOOLS = Path(__file__).resolve().parents[2] / "tools" / "lobo"
sys.path.insert(0, str(TOOLS))
_spec = importlib.util.spec_from_file_location("build_lobo_extras", TOOLS / "build-lobo-extras.py")
extras = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(extras)
from repair.pbo import Archive, CPRS  # noqa: E402

RapArray, RapClass, RapFile, RapValue = extras.RapArray, extras.RapClass, extras.RapFile, extras.RapValue
STRING, FLOAT, INT, ARRAY = extras.STRING, extras.FLOAT, extras.INT, extras.ARRAY
BuildError = extras.BuildError


def make_pbo(entries, *, trailer=None, product=True):
    """entries: (name, stored bytes, packing method, original size). trailer None = SHA-1 tail."""
    header = []
    if product:
        header += [b"\0", struct.pack("<5I", 0x56657273, 0, 0, 0, 0), b"product\0OFP: Resistance\0", b"\0"]
    for name, stored, packing, original in entries:
        header += [name + b"\0", struct.pack("<5I", packing, original, 0, 1234, len(stored))]
    body = b"".join(header) + bytes(21) + b"".join(e[1] for e in entries)
    return body + (b"\0" + hashlib.sha1(body).digest() if trailer is None else trailer)


def f32(value):
    return struct.pack("<f", value)


def hk_config(*, scope_kind=STRING, stub_base="Default", stub_entries=(), extra=()):
    mags = []
    for name in extras.HK_STRING_SCOPE_CLASSES:
        weapon = "private" if scope_kind == STRING else 0
        magazine = "public" if scope_kind == STRING else 2
        mags.append(RapClass(name, "HKLMG36", [
            RapValue("scopeWeapon", scope_kind, weapon),
            RapValue("scopeMagazine", scope_kind, magazine),
            RapValue("displayName", STRING, "$STR_MN_" + name),
        ]))
    weapons = RapClass("CfgWeapons", "", [
        RapClass("Default", "", []),
        RapClass("M16GrenadeLauncher", stub_base, list(stub_entries)),
        RapClass("HKLMG36", "Default", [RapValue("scopeWeapon", INT, 2),
                                        RapValue("magazineType", STRING, "2*256"),
                                        RapArray("magazines", [(STRING, "HK36Cmag"), (STRING, "HKG36Kcmag")])]),
        *mags, *extra])
    root = RapClass("config.bin", "", [
        RapClass("CfgPatches", "", [RapClass("TheHKpackBASE", "", [
            RapArray("units", [(STRING, "HK_KSK1")]), RapValue("requiredVersion", FLOAT, f32(1.75))])]),
        weapons])
    return RapFile(4, root, [("private", 0), ("public", 2)])


VIT_CPP = (b'class CfgPatches\r\n{\r\n  class Vit_Iraq_Shilka\r\n  {\r\n    units[] = {"Vit_Iraq_Shilka"};\r\n'
           b'    weapons[] = {};\r\n  };\r\n};\r\nclass CfgVehicles\r\n{\r\n  class ZSU;\r\n'
           b'  class MAF_ZSU234: ZSU\r\n  {\r\n    scope=2;\r\n    crew="Vit_Iraq_Crew";\r\n'
           b'    displayName="Shilka Iraq"; // crew="Vit_Iraq_Crew" in a comment does not count\r\n  };\r\n};\r\n')

FAMAS_CPP = (b"#define public 2\n"
             b"class CfgPatches\n{\n"
             b"\tclass VIPFAMAS_Officer\t{units[] = {VIPFAMASd_DesOfficer};\tweapons[] = {};\t};\n"
             b"\tclass VIPFAMAS_Soldier\t{units[] = {VIPFAMASd_DesSoldier};\tweapons[] = {};\t};\n"
             b"\tclass VIPFAMAS\t\t{units[] = {VIPFAMAS};\tweapons[] = {};\t};\n};\n"
             b"class CfgWeapons\n{\n\tclass M16;\n\tclass VIPFAMAS: M16 { model=\"\\FAMAS\\FAMAS\"; };\n"
             b"\tclass VIPFAMASm203 { model=\"\\FAMAS\\FAMASgranat\"; };\n};\n"
             b"class CfgVehicles\n{\n\tclass SoldierWB;\n\tclass OfficerW;\n"
             b"\tclass VIPFAMASOfficer: OfficerW { access=1; };\n"
             b"\tclass VIPFAMASSoldier: SoldierWB { access=1; };\n"
             b"\tclass VIPFAMASm203Soldier: SoldierWB { access=1; };\n};")

HYK_CPP = (b"// class GoggleGlass in a comment is ignored\r\n#define true 1\r\n"
           b"class CfgTextureToMaterial\r\n{\r\n\tclass GoggleGlass\r\n\t{\r\n"
           b"\t\ttextures[]={\"\\HYK_USsol\\wl\\g_glass.paa\"};\r\n\t\tmaterial=\"#SpecularGlass\";\r\n"
           b"\t};\r\n};\r\nclass CfgVehicles\r\n{\r\n\tclass HYK_USsolWL { displayName=\"GoggleGlass guy\"; };\r\n};\r\n")


class RapCodecTests(unittest.TestCase):
    def test_round_trip_is_byte_identical(self):
        rap = hk_config()
        rap.root.entries.append(RapClass("Nested", "", [RapArray("deep", [(ARRAY, [(INT, 1), (FLOAT, f32(0.5))]),
                                                                          (STRING, "HK36Cmag")])]))
        data = extras.write_rap(rap)
        self.assertEqual(data[:4], b"\0raP")
        self.assertEqual(extras.write_rap(extras.parse_rap(data)), data)

    def test_string_table_reuses_names(self):
        data = extras.write_rap(hk_config())
        # Used in HKLMG36's magazines[] and as a class name, stored once ("$STR_MN_HK36Cmag" aside).
        self.assertEqual(data.count(b"HK36Cmag\0") - data.count(b"_HK36Cmag\0"), 1)
        self.assertEqual(data.count(b"scopeWeapon\0"), 1)
        self.assertEqual(data.count(b"scopeMagazine\0"), 1)

    def test_legacy_versions_use_plain_int_indices(self):
        for version in (2, 3):
            rap = hk_config()
            rap.version = version
            data = extras.write_rap(rap)
            again = extras.parse_rap(data)
            self.assertEqual(again.version, version)
            self.assertEqual(extras.write_rap(again), data)

    def test_float_bytes_are_kept_exactly(self):
        odd = b"\x01\x00\xc0\x7f"   # a NaN payload a float round trip could canonicalise
        rap = RapFile(4, RapClass("x", "", [RapValue("v", FLOAT, odd)]), [])
        self.assertIn(odd, extras.write_rap(extras.parse_rap(extras.write_rap(rap))))

    def test_rejects_garbage(self):
        with self.assertRaises(ValueError):
            extras.parse_rap(b"\0raQ" + bytes(8))
        data = extras.write_rap(hk_config())
        with self.assertRaises(ValueError):
            extras.parse_rap(data + b"x")
        with self.assertRaises(ValueError):
            extras.parse_rap(data[:-9])


class HkPatchTests(unittest.TestCase):
    def test_patches_scopes_and_stub(self):
        before = extras.write_rap(hk_config())
        after, notes = extras.patch_thehkpack_config(before)
        rap = extras.parse_rap(after)
        weapons = extras.rap_class(rap.root, ("CfgWeapons",))
        for name in extras.HK_STRING_SCOPE_CLASSES:
            cls = extras.rap_class(weapons, (name,))
            self.assertEqual((cls.entries[0].kind, cls.entries[0].value), (INT, 0))
            self.assertEqual((cls.entries[1].kind, cls.entries[1].value), (INT, 2))
            self.assertEqual(cls.entries[2].value, "$STR_MN_" + name)
            self.assertEqual(cls.base, "HKLMG36")
        self.assertEqual(extras.rap_class(weapons, ("M16GrenadeLauncher",)).base, "")
        self.assertEqual(len(notes), 9)
        self.assertEqual(rap.variables, [("private", 0), ("public", 2)])
        # Nothing else moved: undo the 9 edits and the original bytes come back.
        for name in extras.HK_STRING_SCOPE_CLASSES:
            cls = extras.rap_class(weapons, (name,))
            cls.entries[0].kind, cls.entries[0].value = STRING, "private"
            cls.entries[1].kind, cls.entries[1].value = STRING, "public"
        extras.rap_class(weapons, ("M16GrenadeLauncher",)).base = "Default"
        self.assertEqual(extras.write_rap(rap), before)

    def test_refuses_unexpected_states(self):
        cases = {
            "already int": hk_config(scope_kind=INT),
            "stub rebased elsewhere": hk_config(stub_base="Riffle"),
            "stub with body": hk_config(stub_entries=[RapValue("scopeWeapon", INT, 2)]),
            "other string scope": hk_config(extra=[RapClass("HKOther", "", [RapValue("scope", STRING, "public")])]),
        }
        for label, rap in cases.items():
            with self.subTest(label), self.assertRaises(BuildError):
                extras.patch_thehkpack_config(extras.write_rap(rap))
        rap = hk_config()
        weapons = extras.rap_class(rap.root, ("CfgWeapons",))
        weapons.entries = [e for e in weapons.entries if getattr(e, "name", "") != "HKG37mag"]
        with self.assertRaises(BuildError):
            extras.patch_thehkpack_config(extras.write_rap(rap))


class TextPatchTests(unittest.TestCase):
    def test_vit_crew_and_units(self):
        out, notes = extras.patch_vit_config(VIT_CPP)
        self.assertIn(b'crew="SoldierECrew";', out)
        self.assertIn(b'units[] = {"MAF_ZSU234"};', out)
        self.assertIn(b'// crew="Vit_Iraq_Crew" in a comment', out)
        self.assertEqual(out.count(b"\r\n"), VIT_CPP.count(b"\r\n"))
        self.assertEqual(len(notes), 2)

    def test_vit_refuses_other_crew(self):
        with self.assertRaises(BuildError):
            extras.patch_vit_config(VIT_CPP.replace(b'crew="Vit_Iraq_Crew";', b'crew="SoldierECrew";'))
        with self.assertRaises(BuildError):
            extras.patch_vit_config(VIT_CPP.replace(b'{"Vit_Iraq_Shilka"}', b'{"MAF_ZSU234"}'))

    def test_hyk_rename_only_touches_the_class_name(self):
        out, _ = extras.patch_hyk_config(HYK_CPP)
        self.assertEqual(out, HYK_CPP.replace(b"\tclass GoggleGlass\r\n", b"\tclass HYK_GoggleGlass\r\n"))
        self.assertIn(b"// class GoggleGlass in a comment", out)

    def test_hyk_refuses_when_name_is_used_or_taken(self):
        used = HYK_CPP + b"class CfgX { class Foo: GoggleGlass {}; };\r\n"
        taken = HYK_CPP.replace(b"};\r\nclass CfgVehicles", b"\tclass HYK_GoggleGlass {};\r\n};\r\nclass CfgVehicles", 1)
        for label, data in (("referenced", used), ("taken", taken), ("missing", b"class CfgTextureToMaterial {};")):
            with self.subTest(label), self.assertRaises(BuildError):
                extras.patch_hyk_config(data)

    def test_famas_units_and_models(self):
        out, notes = extras.patch_famas_config(FAMAS_CPP)
        text = out.decode("latin1")
        self.assertIn('{units[] = {"VIPFAMASOfficer"};', text)
        self.assertIn('{units[] = {"VIPFAMASSoldier"};', text)
        self.assertIn('{units[] = {"VIPFAMASm203Soldier"};', text)
        self.assertTrue(text.startswith(FAMAS_CPP.decode("latin1").split("class CfgPatches")[0]))
        cfg = extras.TextConfig(out)
        self.assertEqual(cfg.toks[cfg.cls(("CfgModels", "famas")).base_tok].text, "Weapon")
        self.assertEqual(cfg.toks[cfg.cls(("CfgModels", "famasgranat")).base_tok].text, "Weapon")
        self.assertEqual(cfg.toks[cfg.cls(("CfgModels", "Weapon")).base_tok].text, "Default")
        self.assertNotIn("\r\n", text)
        self.assertEqual(len(notes), 4)

    def test_famas_refuses_existing_models_or_missing_units(self):
        with self.assertRaises(BuildError):
            extras.patch_famas_config(FAMAS_CPP + b"\nclass CfgModels { class famas {}; };\n")
        with self.assertRaises(BuildError):
            extras.patch_famas_config(FAMAS_CPP.replace(b"class VIPFAMASm203Soldier", b"class Other"))
        with self.assertRaises(BuildError):
            extras.patch_famas_config(FAMAS_CPP.replace(b"{VIPFAMAS};", b"{VIPFAMAS,Foo};"))

    def test_tokenizer_ignores_comments_strings_and_preprocessor(self):
        text = (b'#define CLASS class Fake {}\n/* class Hidden { x=1; }; */\n'
                b'class Real { s="class Quoted {"; a[]={{1,2},{3}}; class Inner {}; };\n')
        paths = sorted(s.path for s in extras.TextConfig(text).spans)
        self.assertEqual(paths, [("Real",), ("Real", "Inner")])
        with self.assertRaises(BuildError):
            extras.TextConfig(b'class A { x="open; };')


class RepackTests(unittest.TestCase):
    def test_checksummed_pbo_keeps_other_members(self):
        raw = make_pbo([(b"config.cpp", b"old", 0, 0), (b"model.p3d", b"MODEL", CPRS, 99), (b"x.paa", b"TEX", 0, 0)])
        out = extras.repack_pbo(raw, {"config.cpp": b"new config"})
        archive = Archive(out)
        self.assertTrue(archive.checksummed)
        self.assertEqual(bytes(archive.stored(archive.by_key["config.cpp"])), b"new config")
        self.assertEqual(archive.by_key["model.p3d"].fields, Archive(raw).by_key["model.p3d"].fields)
        self.assertEqual(bytes(archive.stored(archive.by_key["x.paa"])), b"TEX")

    def test_stuffpbo_trailer_is_dropped(self):
        tail = b"\x12\x34\x01StuffPBO V1.0b"
        raw = make_pbo([(b"config.cpp", b"old", 0, 0), (b"vit.p3d", b"MODEL", 0, 0)], trailer=tail, product=False)
        self.assertTrue(Archive(raw).stuffpbo)
        out = extras.repack_pbo(raw, {"config.cpp": b"patched"})
        self.assertFalse(out.endswith(tail))
        archive = Archive(out)
        self.assertEqual(archive.trailer, b"")
        self.assertEqual(bytes(archive.stored(archive.by_key["vit.p3d"])), b"MODEL")
        with self.assertRaises(BuildError):
            extras.repack_pbo(raw, {"new.txt": b"x"})

    def test_compressed_config_is_refused(self):
        payload = next(p for p in extras.PAYLOADS if p.patch == "vit")
        raw = make_pbo([(b"config.cpp", b"\x00compressed", CPRS, 500), (b"vit_iraq_shilka.p3d", b"M", 0, 0)])
        with self.assertRaisesRegex(BuildError, "compressed"):
            extras.apply_patch(payload, raw)


def index(label, classes, *, stem=None, patches=()):
    """classes: {path string: (base, {key: value}, access)}"""
    out = {}
    for path, (base, values, access) in classes.items():
        key = tuple(path.lower().split(">>"))
        out[key] = extras.ClassInfo(path, base, {k: extras._norm_item(STRING, v) for k, v in values.items()}, access)
    return extras.ConfigIndex(label, stem if stem is not None else label.lower(), set(patches), out)


class ClashTests(unittest.TestCase):
    def test_name_patches_and_value_clashes(self):
        mine = index("hk", {"CfgVehicles>>Foo": ("", {"model": "a"}, -1)}, patches=("hkpatch",))
        core = index("core", {"CfgVehicles>>Foo": ("", {"model": "b"}, -1)}, patches=("HKPATCH".lower(),))
        same_name = index("x", {}, stem="hk")
        errors, _ = extras.check_clashes([mine], [core, same_name], [])
        self.assertTrue(any(e.startswith("pbo name clash") for e in errors))
        self.assertTrue(any(e.startswith("CfgPatches clash") for e in errors))
        self.assertTrue(any(e.startswith("class clash") for e in errors))

    def test_existing_and_additive_values_are_warnings(self):
        mine = index("hyk", {"CfgGroups>>West": ("", {"name": "West"}, -1),
                             "CfgRecoils": ("", {"mine": "1"}, -1)})
        core_a = index("a", {"CfgGroups>>West": ("", {"name": "$STR_WEST"}, -1), "CfgRecoils": ("", {"theirs": "2"}, -1)})
        core_b = index("b", {"CfgGroups>>West": ("", {"name": "West"}, -1)})
        errors, warnings = extras.check_clashes([mine], [core_a, core_b], [])
        self.assertEqual(errors, [])
        self.assertTrue(any(w.startswith("existing-value: CfgGroups>>West") for w in warnings))
        self.assertTrue(any(w.startswith("additive: CfgRecoils") for w in warnings))

    def test_stock_base_mismatch_depends_on_lock(self):
        mine = index("hk", {"CfgWeapons>>M16GrenadeLauncher": ("default", {}, -1),
                            "CfgWeapons>>AALauncher": ("default", {}, -1)})
        stock = index("stock", {"CfgWeapons>>M16GrenadeLauncher": ("", {"scopeWeapon": "2"}, 1),
                                "CfgWeapons>>AALauncher": ("carlgustavlauncher", {}, 3)}, stem="")
        errors, warnings = extras.check_clashes([mine], [], [stock])
        self.assertEqual(len(errors), 1)
        self.assertIn("M16GrenadeLauncher", errors[0])
        self.assertTrue(any(w.startswith("stock-base-locked: CfgWeapons>>AALauncher") for w in warnings))

    def test_core_rebase_is_an_error(self):
        mine = index("hyk", {"CfgVehicles>>LoBoSoldier": ("soldiereb", {}, -1)})
        core = index("@LoBo/LoBoIs.pbo", {"CfgVehicles>>LoBoSoldier": ("soldierwb", {"side": "1"}, -1)}, stem="lobois")
        errors, _ = extras.check_clashes([mine], [core], [])
        self.assertEqual(len(errors), 1)
        self.assertTrue(errors[0].startswith("class rebase: CfgVehicles>>LoBoSoldier"), errors)

    def test_stock_addons_rebase_is_an_error(self):
        # HKG3 is created by stock AddOns\O.pbo, so no access lock protects it.
        mine = index("hk", {"CfgWeapons>>HKG3": ("m16", {}, -1)})
        o_pbo = index("stock AddOns/O.pbo", {"CfgWeapons>>HKG3": ("hkg3base", {"model": "g3"}, -1)}, stem="o")
        stock = index("stock BIN/CONFIG.BIN", {"CfgWeapons>>M16": ("riffle", {}, 3)}, stem="")
        errors, _ = extras.check_clashes([mine], [o_pbo], [stock])
        self.assertTrue(any(e.startswith("class rebase: CfgWeapons>>HKG3") and "O.pbo" in e for e in errors), errors)

    def test_rebase_refused_by_a_stock_lock_is_a_warning(self):
        mine = index("bwmod", {"CfgWeapons>>AALauncher": ("default", {}, -1)})
        core = index("core", {"CfgWeapons>>AALauncher": ("carlgustavlauncher", {}, -1)})
        stock = index("stock", {"CfgWeapons>>AALauncher": ("carlgustavlauncher", {}, 3)}, stem="")
        errors, warnings = extras.check_clashes([mine], [core], [stock])
        self.assertEqual(errors, [])
        self.assertTrue(any(w.startswith("stock-base-locked: CfgWeapons>>AALauncher") for w in warnings))

    def test_accepted_rebase_is_a_warning(self):
        mine = index("@LoBo_Extras/bas_lbs185.pbo", {"cfgModels>>Helicopter": ("", {}, -1)}, stem="bas_lbs185")
        core = index("@LoBo/LoBoEghelo.pbo", {"cfgModels>>Helicopter": ("air", {}, -1)}, stem="loboeghelo")
        stock = index("stock", {"cfgModels>>Helicopter": ("", {}, -1)}, stem="")
        errors, warnings = extras.check_clashes([mine], [core], [stock])
        self.assertEqual(errors, [])
        self.assertTrue(any(w.startswith("accepted-base: cfgModels>>Helicopter") and "stock shape" in w
                            for w in warnings), warnings)
        # The same overlap from any other pbo is an error.
        other = index("@LoBo_Extras/x.pbo", {"cfgModels>>Helicopter": ("", {}, -1)}, stem="x")
        errors, _ = extras.check_clashes([other], [core], [stock])
        self.assertTrue(errors and errors[0].startswith("class rebase"), errors)

    def test_keys_added_to_an_unlocked_stock_class_warn(self):
        def info(values, access):
            return extras.ClassInfo("CfgVehicles>>SoldierWB", "soldier", values, access)
        scalar, array = ("n", 1.0), ("a", (("s", "x"),))
        mine = extras.ConfigIndex("ext", "ext", set(), {("cfgvehicles", "soldierwb"):
                                                        info({"newscalar": scalar, "newarray": array}, -1)})
        for access, expected in ((-1, "['newarray', 'newscalar']"), (1, "['newarray']"), (2, None)):
            with self.subTest(access=access):
                stock = extras.ConfigIndex("stock", "", set(), {("cfgvehicles", "soldierwb"): info({}, access)})
                errors, warnings = extras.check_clashes([mine], [], [stock])
                self.assertEqual(errors, [])
                added = [w for w in warnings if w.startswith("stock-keys-added")]
                if expected is None:
                    self.assertEqual(added, [])
                else:
                    self.assertEqual(len(added), 1)
                    self.assertIn(expected, added[0])

    def test_stock_access_follows_bases_and_parents(self):
        root = RapClass("config.bin", "", [
            RapClass("CfgWeapons", "", [
                RapValue("access", INT, 1),
                RapClass("Default", "", [RapValue("access", INT, 3)]),
                RapClass("CarlGustavLauncher", "Default", []),
                RapClass("M16GrenadeLauncher", "", []),
            ]),
            RapClass("CfgFree", "", [RapClass("Thing", "", [])]),
        ])
        idx = extras.index_rap("stock", "", RapFile(4, root, []), with_access=True)
        self.assertEqual(idx.classes[("cfgweapons", "carlgustavlauncher")].access, 3)
        self.assertEqual(idx.classes[("cfgweapons", "m16grenadelauncher")].access, 1)
        self.assertEqual(idx.classes[("cfgfree", "thing")].access, -1)


VIT_PBO = make_pbo([(b"config.cpp", VIT_CPP, 0, 0), (b"Vit_Iraq_Shilka.p3d", b"ODOL", 0, 0)],
                   trailer=b"\x0e\x2a\x01StuffPBO V1.0b", product=False)
DOC = b"VIT COPYRIGHT\r\n"
MISSION = make_pbo([(b"mission.sqm", b"version=11;", 0, 0)])


def synthetic_payloads(bwmod=b"BWMOD"):
    def pin(dest, data, **kw):
        return extras.Payload(kw.pop("pack", "vit"), "synthetic.ace", "m/" + dest, hashlib.sha256(data).hexdigest(),
                              len(data), dest, **kw)
    blobs = {"addons/Vit_Iraq_Shilka.pbo": VIT_PBO, "docs/Vit_Iraq_Shilka/Vit Addons.txt": DOC,
             "Missions/Test.Abel.pbo": MISSION, "addons/BWMOD_Scripts.pbo": bwmod}
    payloads = (pin("addons/Vit_Iraq_Shilka.pbo", VIT_PBO, patch="vit"), pin("docs/Vit_Iraq_Shilka/Vit Addons.txt", DOC),
                pin("Missions/Test.Abel.pbo", MISSION), pin("addons/BWMOD_Scripts.pbo", bwmod, pack="bwmod"))
    return payloads, blobs


class BuildPipelineTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        payloads, self.blobs = synthetic_payloads()
        patched, *_ = extras.apply_patch(payloads[0], VIT_PBO)
        self.patches = [
            mock.patch.object(extras, "PAYLOADS", payloads),
            mock.patch.object(extras, "SOURCES", {"synthetic.ace": ("0" * 64, 1)}),
            mock.patch.dict(extras.OUTPUT_SHA256, {"addons/Vit_Iraq_Shilka.pbo": hashlib.sha256(patched).hexdigest()},
                            clear=True),
        ]
        for p in self.patches:
            p.start()
        self.patched = patched

    def tearDown(self):
        for p in reversed(self.patches):
            p.stop()
        self.tmp.cleanup()

    def extractor(self, payloads):
        return {p.dest: self.blobs[p.dest] for p in payloads}

    def build(self, **kw):
        return extras.build(None, self.root / "out" / "@LoBo_Extras", extractor=self.extractor,
                            work=self.root / "work", log=lambda m: None, **kw)

    def test_layout_readme_and_manifest(self):
        manifest = self.build()
        out = self.root / "out" / "@LoBo_Extras"
        self.assertEqual((out / "addons" / "Vit_Iraq_Shilka.pbo").read_bytes(), self.patched)
        self.assertEqual((out / "Missions" / "Test.Abel.pbo").read_bytes(), MISSION)
        self.assertEqual((out / "docs" / "Vit_Iraq_Shilka" / "Vit Addons.txt").read_bytes(), DOC)
        self.assertFalse(any(p.name.lower() == "bin" for p in out.rglob("*")))
        readme = (out / "README.txt").read_bytes().decode("ascii")
        self.assertIn('PoseidonGame.exe --mod "@LoBo_Extras;@LoBo;@LoBo_Deps"', readme)
        self.assertIn('PoseidonGame.exe --mod "@LoBo_Extras;@LoBo;@LoBo_Deps;@LoBo_UD"', readme)
        self.assertIn("List @LoBo_Extras FIRST", readme)
        self.assertIn("Never put a Bin folder", readme)
        self.assertIn('crew: "Vit_Iraq_Crew" -> "SoldierECrew"', readme)
        self.assertNotIn(chr(0x2014), readme)
        self.assertNotIn(chr(0x2013), readme)
        self.assertNotIn("\n", readme.replace("\r\n", ""))
        saved = json.loads((out / "docs" / "BUILD-MANIFEST.json").read_text("utf8"))
        self.assertEqual(saved["files"], manifest["files"])
        self.assertFalse(saved["clash_check"]["checked"])
        self.assertFalse((self.root / "out" / "@LoBo_Extras.partial").exists())

    def test_build_is_reproducible(self):
        self.build()
        out = self.root / "out" / "@LoBo_Extras"
        first = {p.relative_to(out): p.read_bytes() for p in out.rglob("*") if p.is_file()}
        self.build(replace=True)
        second = {p.relative_to(out): p.read_bytes() for p in out.rglob("*") if p.is_file()}
        self.assertEqual(first, second)
        # Fixed times on every entry, the folder itself included, so archives of it match.
        for p in [out, *out.rglob("*")]:
            self.assertEqual(int(p.stat().st_mtime), extras.OUTPUT_MTIME, p)

    def test_no_bwmod_leaves_bwmod_out(self):
        self.build(include_bwmod=False)
        self.assertFalse((self.root / "out" / "@LoBo_Extras" / "addons" / "BWMOD_Scripts.pbo").exists())

    def test_payload_pin_mismatch_is_refused(self):
        self.blobs["docs/Vit_Iraq_Shilka/Vit Addons.txt"] = b"tampered"
        with self.assertRaises(BuildError):
            self.build()
        self.assertFalse((self.root / "out" / "@LoBo_Extras").exists())

    def test_output_pin_mismatch_is_refused(self):
        extras.OUTPUT_SHA256["addons/Vit_Iraq_Shilka.pbo"] = "f" * 64
        with self.assertRaisesRegex(BuildError, "does not match its pin"):
            self.build()

    def test_existing_folder_is_not_overwritten(self):
        out = self.root / "out" / "@LoBo_Extras"
        out.mkdir(parents=True)
        (out / "keep.txt").write_bytes(b"user data")
        with self.assertRaises(BuildError):
            self.build(replace=True)
        self.assertEqual((out / "keep.txt").read_bytes(), b"user data")

    def test_never_ship_pbo_is_refused(self):
        payloads, _ = synthetic_payloads()
        bad = dataclasses.replace(payloads[2], dest="addons/bas_soar.pbo")
        self.blobs["addons/bas_soar.pbo"] = MISSION
        with mock.patch.object(extras, "PAYLOADS", payloads[:2] + (bad,)):
            with self.assertRaisesRegex(BuildError, "never-ship"):
                self.build()
        self.assertFalse((self.root / "out" / "@LoBo_Extras").exists())

    def test_layout_guard_catches_a_never_ship_pbo(self):
        stage = self.root / "stage"
        (stage / "addons").mkdir(parents=True)
        (stage / "README.txt").write_bytes(b"x")
        (stage / "addons" / "LoBoWeapons.pbo").write_bytes(b"x")
        with self.assertRaisesRegex(BuildError, "never-ship"):
            extras.assert_layout(stage)
        (stage / "addons" / "LoBoWeapons.pbo").unlink()
        (stage / "Bin").mkdir()
        with self.assertRaises(BuildError):
            extras.assert_layout(stage)

    def test_wrong_output_name_is_refused(self):
        with self.assertRaises(BuildError):
            extras.build(None, self.root / "Extras", extractor=self.extractor, log=lambda m: None)

    # --- builds with PoseidonTools: the engine's reading is faked -------------

    def checked_build(self, *, rap="clean", output="", core=None, **kw):
        tools = self.root / "PoseidonTools.exe"
        tools.write_bytes(b"stub")
        core_dir = self.root / "core"
        (core_dir / "addons").mkdir(parents=True, exist_ok=True)
        core = core if core is not None else [index("@LoBo/LoBoIs.pbo", {"CfgVehicles>>LoBoSoldier": ("soldierwb", {}, -1)},
                                                    stem="lobois")]

        def read_root_config(path, tools, scratch):
            path = Path(path)
            if path.name == "Vit_Iraq_Shilka.pbo":
                return extras.RootConfig("cpp", vit_rap() if rap == "clean" else rap,
                                         Archive(path.read_bytes()).read("config.cpp"), output, "a" * 64)
            return extras.RootConfig("", sha256="b" * 64)

        with mock.patch.object(extras, "read_root_config", read_root_config), \
                mock.patch.object(extras, "index_folder", lambda folder, tools, scratch, label=None: core), \
                mock.patch.object(extras, "validate_with_tools", lambda *a: "faked"):
            return self.build(tools=tools, against=[core_dir], **kw)

    def test_checked_build_records_the_scan_and_the_core_it_checked(self):
        manifest = self.checked_build()
        self.assertEqual(manifest["config_scan"], {"checked": True, "configs": 1,
                                                   "without_root_config": ["BWMOD_Scripts.pbo"]})
        folder = manifest["clash_check"]["against_folders"][0]
        self.assertEqual((folder["name"], folder["pbos"]), ("core", 1))
        self.assertRegex(folder["sha256"], "^[0-9a-f]{64}$")
        readme = (self.root / "out" / "@LoBo_Extras" / "README.txt").read_text("ascii")
        self.assertIn("checked every pack for pbo-name, CfgPatches and class clashes", readme)
        self.assertIn("Because @LoBo_Extras is listed first, @LoBo and @LoBo_Deps win any pbo-name", readme)

    def test_config_defects_stop_the_build(self):
        warning = ("[warning] config: undefined scope keyword 'public' in <scratch>/Vit_Iraq_Shilka/config.cpp/"
                   "CfgVehicles/MAF_ZSU234.scope - resolved as 2\n")
        for label, kw in (("scope keyword", {"output": warning}), ("unparsed", {"rap": None})):
            with self.subTest(label), self.assertRaisesRegex(BuildError, "config defects"):
                self.checked_build(**kw)
            self.assertFalse((self.root / "out" / "@LoBo_Extras").exists())

    def test_partly_read_extras_config_stops_the_clash_check(self):
        partial = vit_rap()
        partial.root.entries.pop()   # the engine "lost" CfgVehicles>>MAF_ZSU234
        with self.assertRaisesRegex(BuildError, "did not read every extras config"):
            self.checked_build(rap=partial)
        self.assertFalse((self.root / "out" / "@LoBo_Extras").exists())

    def test_core_rebase_stops_the_build(self):
        core = [index("@LoBo/LoBoSA6.pbo", {"CfgVehicles>>MAF_ZSU234": ("tank", {}, -1)}, stem="lobosa6")]
        with self.assertRaisesRegex(BuildError, "class rebase: CfgVehicles>>MAF_ZSU234"):
            self.checked_build(core=core)
        self.assertFalse((self.root / "out" / "@LoBo_Extras").exists())

    def test_check_mode(self):
        self.build()
        out = self.root / "out" / "@LoBo_Extras"
        tools = self.root / "PoseidonTools.exe"
        tools.write_bytes(b"stub")

        def run_check(core):
            def read_root_config(path, tools, scratch):
                if Path(path).name == "Vit_Iraq_Shilka.pbo":
                    return extras.RootConfig("cpp", vit_rap(), Archive(Path(path).read_bytes()).read("config.cpp"))
                return extras.RootConfig("")
            report = self.root / "check.json"
            with mock.patch.object(extras, "read_root_config", read_root_config), \
                    mock.patch.object(extras, "index_folder", lambda folder, tools, scratch, label=None: core), \
                    contextlib.redirect_stdout(io.StringIO()) as printed:
                code = extras.main(["--check", str(out), "--against", str(self.root / "core"), "--tools", str(tools),
                                    "--work", str(self.root / "check-work"), "--report", str(report)])
            data = json.loads(report.read_text("utf8"))
            self.assertEqual(json.loads(printed.getvalue()), data)
            return code, data

        code, data = run_check([index("@LoBo/LoBoIs.pbo", {"CfgVehicles>>LoBoSoldier": ("soldierwb", {}, -1)})])
        self.assertEqual((code, data["errors"], data["pbos"]), (0, [], 2))
        code, data = run_check([index("@LoBo/LoBoSA6.pbo", {"CfgVehicles>>MAF_ZSU234": ("tank", {}, -1)})])
        self.assertEqual(code, 1)
        self.assertTrue(data["errors"][0].startswith("class rebase"), data["errors"])
        with contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(extras.main(["--check", str(out), "--tools", str(tools)]), 2)   # no --against


def vit_rap():
    """What the engine parser makes of the patched VIT_CPP."""
    return RapFile(4, RapClass("config.bin", "", [
        RapClass("CfgPatches", "", [RapClass("Vit_Iraq_Shilka", "", [RapArray("units", [(STRING, "MAF_ZSU234")])])]),
        RapClass("CfgVehicles", "", [RapClass("MAF_ZSU234", "ZSU", [RapValue("crew", STRING, "SoldierECrew")])]),
    ]), [])


class ConfigScanTests(unittest.TestCase):
    def test_text_config_defects_come_from_the_engine_parser(self):
        output = ("[w] config: undefined scope keyword 'public' in <scratch>/x/config.cpp/CfgVehicles/A.scope - "
                  "resolved as 2 (the value the missing #define would have given it)\r\n"
                  "[w] config: malformed float literal '0.0.1' in <scratch>/x/config.cpp/CfgAmmo/B.c[0] - parsed as 0\r\n"
                  "Note: binarising from text does not reconstruct the enum/variable table\r\n")
        found = extras.config_defects(extras.RootConfig("cpp", vit_rap(), b"", output))
        self.assertEqual(len(found), 2)
        self.assertTrue(found[0].startswith("undefined scope keyword 'public'"))
        self.assertTrue(found[1].startswith("malformed float literal '0.0.1'"))
        self.assertEqual(extras.config_defects(extras.RootConfig("cpp", None, b"")), ["the engine parser produced no config"])
        self.assertEqual(extras.config_defects(extras.RootConfig("")), [])

    def test_binarized_string_scopes_and_floats(self):
        rap = RapFile(4, RapClass("config.bin", "", [RapClass("CfgWeapons", "", [
            RapClass("Mag", "", [RapValue("scopeWeapon", STRING, "private"), RapValue("scopeMagazine", STRING, "2"),
                                 RapValue("scope", INT, 2), RapValue("displayName", STRING, "1.0.2 build")]),
            RapClass("Ammo", "", [RapArray("tracerColor", [(FLOAT, f32(1)), (STRING, "0.0.1")])]),
        ])]), [])
        found = extras.config_defects(extras.RootConfig("bin", rap))
        self.assertEqual(found, ['string scope CfgWeapons>>Mag>>scopeWeapon="private" (reads as 0)',
                                 'malformed float CfgWeapons>>Ammo>>tracerColor[]: "0.0.1"'])

    def test_incomplete_extras_index_is_a_problem(self):
        tolerant = extras.RootConfig("cpp", None, b"class CfgPatches { class P { units[]={}; }; };")
        _, problems = extras.index_extras([(Path("x.pbo"), tolerant), (Path("res.pbo"), extras.RootConfig(""))])
        self.assertEqual(problems, ["@LoBo_Extras/x.pbo: indexed with the tolerant text reader"])

    def test_unsafe_pbo_prefix_cannot_escape_the_scratch_folder(self):
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            pbo = tmp / "evil.pbo"
            header = [b"\0", struct.pack("<5I", 0x56657273, 0, 0, 0, 0), b"prefix\0../../escaped\0", b"\0"]
            text = b"class CfgPatches {};"
            pbo.write_bytes(b"".join(header) + b"config.cpp\0" + struct.pack("<5I", 0, 0, 0, 0, len(text))
                            + bytes(21) + text)
            (tmp / "scan").mkdir()
            # sys.executable stands in for PoseidonTools: "config bin" then fails, harmlessly.
            cfg = extras.read_root_config(pbo, sys.executable, tmp / "scan")
            self.assertEqual((cfg.kind, cfg.rap, cfg.text), ("cpp", None, text))
            self.assertFalse((tmp / "escaped").exists())
            self.assertEqual(list((tmp / "scan").iterdir()), [])


class ExtractionPinTests(unittest.TestCase):
    """extract_payloads: the archive pin is checked before any reader opens it."""

    class Reader:
        def __init__(self, member, listed=None, on_read=None):
            self.member, self.listed, self.on_read, self.calls = member, listed, on_read, []

        def listing(self, archive):
            self.calls.append("listing")
            return {"m.pbo": len(self.member) if self.listed is None else self.listed}

        def read(self, archive, member):
            self.calls.append("read")
            if self.on_read:
                self.on_read(archive)
            return self.member

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.member = b"pinned payload"
        self.archive_bytes = b"untrusted archive bytes"

    def tearDown(self):
        self.tmp.cleanup()

    def extract(self, reader, *, name="pack.zip", digest=None, size=None, other=None):
        (self.root / name).write_bytes(self.archive_bytes)
        pin = (digest or hashlib.sha256(self.archive_bytes).hexdigest(), size or len(self.archive_bytes))
        payload = extras.Payload("t", name, "m.pbo", hashlib.sha256(self.member).hexdigest(), len(self.member),
                                 "addons/m.pbo")
        with mock.patch.dict(extras.SOURCES, {name: pin}, clear=True):
            seven, ace = (reader, other) if not name.endswith(".ace") else (other, reader)
            return extras.extract_payloads(self.root, [payload], seven, ace)

    def test_pinned_archive_is_read(self):
        reader = self.Reader(self.member)
        self.assertEqual(self.extract(reader), {"addons/m.pbo": self.member})
        self.assertEqual(reader.calls, ["listing", "read"])

    def test_ace_goes_to_the_ace_reader(self):
        ace, seven = self.Reader(self.member), self.Reader(self.member)
        self.extract(ace, name="pack.ace", other=seven)
        self.assertEqual((ace.calls, seven.calls), (["listing", "read"], []))

    def test_wrong_archive_is_refused_before_it_is_opened(self):
        for label, kw in (("size", {"size": len(self.archive_bytes) + 1}), ("sha256", {"digest": "f" * 64})):
            reader = self.Reader(self.member)
            with self.subTest(label), self.assertRaisesRegex(BuildError, "does not match its pin"):
                self.extract(reader, **kw)
            self.assertEqual(reader.calls, [])

    def test_missing_archive_is_refused(self):
        payload = extras.Payload("t", "gone.zip", "m.pbo", "0" * 64, 1, "addons/m.pbo")
        with mock.patch.dict(extras.SOURCES, {"gone.zip": ("0" * 64, 1)}, clear=True):
            with self.assertRaisesRegex(BuildError, "missing source archive"):
                extras.extract_payloads(self.root, [payload], self.Reader(b""), self.Reader(b""))

    def test_listing_size_mismatch_is_refused_before_reading(self):
        reader = self.Reader(self.member, listed=len(self.member) + 1)
        with self.assertRaisesRegex(BuildError, "missing or wrong size"):
            self.extract(reader)
        self.assertEqual(reader.calls, ["listing"])

    def test_payload_that_misses_its_pin_is_refused(self):
        reader = self.Reader(b"other payload!")   # same length, different bytes
        with self.assertRaisesRegex(BuildError, "expected"):
            self.extract(reader)

    def test_archive_changed_while_read_is_refused(self):
        def tamper(archive):
            Path(archive).write_bytes(self.archive_bytes + b"!")
        with self.assertRaisesRegex(BuildError, "changed while it was being read"):
            self.extract(self.Reader(self.member, on_read=tamper))


class TableTests(unittest.TestCase):
    def test_real_tables_pass_self_check(self):
        extras.self_check()
        sources = {p.archive for p in extras.PAYLOADS}
        self.assertFalse(sources & extras.NEVER_SHIP_ARCHIVES)
        self.assertNotIn(("baslittlebirds1_1for185.zip", "bas_soar.pbo"), {(p.archive, p.member) for p in extras.PAYLOADS})
        stems = {Path(p.dest).stem.lower() for p in extras.PAYLOADS if p.dest.endswith(".pbo")}
        self.assertFalse(stems & extras.NEVER_SHIP_PBOS)
        self.assertEqual({p.pack for p in extras.selected_payloads(False)}, {"hk", "hyk", "vit", "bas", "famas"})

    def test_never_ship_archive_is_refused(self):
        bad = extras.Payload("x", "OFPHKpack.zip", "TheHKpack.pbo", "0" * 64, 1, "addons/TheHKpack2.pbo")
        with mock.patch.object(extras, "PAYLOADS", extras.PAYLOADS + (bad,)), \
                mock.patch.dict(extras.SOURCES, {"OFPHKpack.zip": ("0" * 64, 1)}):
            with self.assertRaisesRegex(BuildError, "never-ship"):
                extras.self_check()

    def test_seven_zip_member_patterns_are_refused(self):
        with tempfile.NamedTemporaryFile(suffix=".exe", delete=False) as fake:
            path = Path(fake.name)
        try:
            with self.assertRaises(BuildError):
                extras.SevenZip(path).read(path, "*.pbo")
        finally:
            path.unlink()


if __name__ == "__main__":
    unittest.main()
