"""2026-10-11.4, fourth part: references to files LoBo ships under another name.

Each repoint names a file an installed @LoBo addon already ships, after a
look at both images (CONTENT-REPAIRS.md, "Fourth part"). REQUIRED_MEMBERS
makes plan() fail closed when a reinstall lacks a target. Nothing is added or
generated; same techniques as repair.references (LoBo #14).

Config text (one occurrence each):
- LoBo_FAB: the Snunit boat's picture names \\LoBo_FAB\\Snunit.paa; the
  128x64 picture of that boat ships as SN\\Snunit.paa, next to its hull
  textures.
- LoBoRadios: BAS_Satcom names the folder `icon`; its uiPicture
  (BASirto.paa) and picture (BAS_satcom.paa) ship in `icons`, where the three
  other radios find theirs (their sebirto.paa has BASirto's bytes).
- LoBo_Ob: the medic buildings are Baracken's Brown_Baracke04 (ATTENDANT=1,
  Brown/Green/Grey variants) with MDA and Red Crescent signs. Their editor
  icon names \\LoBo_Ob\\bar\\b4.jpg, which never shipped; Brown_Baracke04's
  icon \\Baracken\\b4.jpg (the medic barrack drawn with a cross) ships in
  Baracken.pbo, part of @LoBo since the Sinai island release.

Models (fixed-width MLOD texture fields, textures.repoint; file length and
every other byte unchanged):
- LoBo_M1 Guard M1A1 and M1A2 (_r models, three-colour tex1 set): LOD 15
  names tex1\\kolo.pac (84 road wheel faces) and LODs 15 and 20
  tex1\\main_side.paa (117 and 20 hull side faces). LODs 5, 7.5 and 20 use
  tex1\\kolo.paa and LODs 5 and 7.5 tex1\\main_side.pac (with main_side2);
  only the extension differs. The UVs match those atlases: LOD 20's 58 wheel
  faces are the author's own repoint of the same wheels to kolo.paa (counted
  with the target), and the LOD 15 side faces cover main_side.pac's layout.
  The sand-coloured tex2\\kolo.pac of the other M1 models was rejected.
- LoBo_F4 Wild Weasel (IAF): LOD 5 names idf\\mer.pac (6 faces, the ECM pod
  pylon) and idf\\dark_grey.pac (3 pod faces). The same LOD uses
  idf\\1\\mer.pac (226) and com\\dark_grey.pac (263). Both F-4 mer.pac and
  the F-16 IAF one have the same bytes, as do the dark_grey.pac of the F-4,
  F-15 and F-16. Counts include those authored faces.
- LoboF5 (three Jordanian F-5 models): LODs 500 and 1100 (pilot view) name
  LoBoF5\\com\\white.paa (10 and 347 faces). LoboF5 ships no white; every
  white.paa in @LoBo (Kfir, IL-28, MiG-21, MiG-25 and the BDE Mirage) is the
  same 32x32 opaque white, and the F-5's red and silver have the bytes of the
  Kfir's and MiG-21's. Target: the Kfir's, which its model draws on 1,282
  faces. The three models end with an empty Oxygen default-path buffer,
  pinned per member like the Hilux.

Left as authored, with reasons, in CONTENT-REPAIRS.md: LoBo_A4
Tex2\\tailwing.paa, LoBo_Ob gw\\has\\HAS.paa and emb\\icon.paa, LoBoJorAPC
CVR\\icon.paa, LoBoRollers' \\LoBoSherm\\iM4, the four LoBoEgAPC pictures,
both USemicon.paa, LoBoF16 backfuse.pac and the pilots'
data\\w_pilot_rucebok_l/_p.pac. Profile-neutral: string literals and texture
names only.
"""
from .rules import apply_rules, p
from .textures import _b, repoint

FAB = "lobo_fab/config.cpp"
RADIOS = "loboradios/config.cpp"
OB = "lobo_ob/config.cpp"
RULES = {
    FAB: (("sub", 'picture="' + p("LoBo_FAB", "Snunit.paa") + '";',
           'picture="' + p("LoBo_FAB", "SN", "Snunit.paa") + '";', 1),),
    RADIOS: (("sub", '"' + p("LoBoRadios", "icon", "BAS_satcom.paa") + '"',
              '"' + p("LoBoRadios", "icons", "BAS_satcom.paa") + '"', 1),
             ("sub", '"' + p("LoBoRadios", "icon", "BASirto.paa") + '"',
              '"' + p("LoBoRadios", "icons", "BASirto.paa") + '"', 1)),
    OB: (("sub", 'icon="' + p("LoBo_Ob", "bar", "b4.jpg") + '";',
          'icon="' + p("Baracken", "b4.jpg") + '";', 1),),
}

M1A1_R, M1A2_R = "lobo_m1/lobo_m1a1e_r.p3d", "lobo_m1/lobo_m1a2e_r.p3d"
F4_WW = "lobo_f4/lobo_f4_ww_iaf.p3d"
F5_AA, F5_AT, F5_MIR = "lobof5/lobo_f5_aa_jor.p3d", "lobof5/lobo_f5_at_jor.p3d", "lobof5/lobo_f5_mir_jor.p3d"
MODEL_MEMBERS = (M1A1_R, M1A2_R, F4_WW, F5_AA, F5_AT, F5_MIR)
M1_GUARD = {
    # (LOD 15, LOD 20): LOD 20 already names kolo.paa on its 58 wheel faces.
    _b("LoBo_M1", "tex1", "kolo.pac"): (_b("LoBo_M1", "tex1", "kolo.paa"), (84, 58), False),
    _b("LoBo_M1", "tex1", "main_side.paa"): (_b("LoBo_M1", "tex1", "main_side.pac"), (117, 20), False),
}
F5_WHITE = {_b("LoBoF5", "com", "white.paa"): (_b("LoBo_Kfir", "kfir1", "white.paa"), (10, 347), False)}
PLANS = {  # member: (resolutions, mapping, pinned trailer or None)
    M1A1_R: ((15.0, 20.0), M1_GUARD, None),
    M1A2_R: ((15.0, 20.0), M1_GUARD, None),
    # 6 + 226 and 3 + 263: the repointed faces plus the authored ones in LOD 5.
    F4_WW: ((5.0,), {
        _b("LoBo_F4", "idf", "mer.pac"): (_b("LoBo_F4", "idf", "1", "mer.pac"), 232, False),
        _b("LoBo_F4", "idf", "dark_grey.pac"): (_b("LoBo_F4", "com", "dark_grey.pac"), 266, False),
    }, None),
    F5_AA: ((500.0, 1100.0), F5_WHITE,
            bytes.fromhex("0000000000000000000000004029a50007000000e0eba00000b5a000a95fe300")),
    F5_AT: ((500.0, 1100.0), F5_WHITE,
            bytes.fromhex("000000000000000000000000e03ea10007000000d0eca00000b5a000a95fe300")),
    F5_MIR: ((500.0, 1100.0), F5_WHITE,
             bytes.fromhex("00000000000000000000000010c1a20007000000e0eba00000b5a000a95fe300")),
}
REQUIRED_MEMBERS = ("lobo_fab/sn/snunit.paa", "loboradios/icons/bas_satcom.paa", "loboradios/icons/basirto.paa",
                    "baracken/b4.jpg", "lobo_m1/tex1/kolo.paa", "lobo_m1/tex1/main_side.pac",
                    "lobo_f4/idf/1/mer.pac", "lobo_f4/com/dark_grey.pac", "lobo_kfir/kfir1/white.paa")
ADDONS = ("baracken", "lobo_f4", "lobo_fab", "lobo_kfir", "lobo_m1", "lobo_ob", "lobof5", "loboradios")


def repair(documents: dict[str, str]) -> dict[str, str]:
    return apply_rules(documents, RULES)


def repair_models(documents: dict[str, bytes]) -> dict[str, bytes]:
    changed = {}
    for key, (resolutions, mapping, trailer) in PLANS.items():
        if key not in documents:
            raise ValueError("Missing model " + key)
        try:
            fixed = repoint(documents[key], resolutions, mapping, trailer)
        except ValueError as exc:
            raise ValueError(f"{key}: {exc}") from exc
        if fixed != documents[key]:
            changed[key] = fixed
    return changed
