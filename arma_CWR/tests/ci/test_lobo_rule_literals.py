"""Literal pins for the 2026-10-11.3 rule tables.

The per-module tests build their fixtures and their expected output from each
module's own table, so they prove the rule engine (exactness, idempotency, fail
closed) but not the replacement values. This file restates every original and
replacement by hand from the implementation spec (sections 4.1 to 4.9, plus the
reviewed deviations noted inline). A changed value, count or baseline in a module
fails here until this file is updated deliberately alongside the spec.
"""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import clusters, content, effects, identifiers, loops, mines, numbers, textures, weapons  # noqa: E402

T2, T3, T4 = "\t\t", "\t\t\t", "\t\t\t\t"
S8, S16 = " " * 8, " " * 16


def as_sets(rules):
    return {member: set(items) for member, items in rules.items()}


LOOPS = {
    "loboeghelo/mk2/float.sqs": {
        ("sub", '#loop\n_floater setpos [getpos _Hfloat select 0, getpos _Hfloat select 1, 1]\ngoto "loop"',
         '#loop\n?(not alive _Hfloat): deleteVehicle _floater; exit\n'
         '_floater setpos [getpos _Hfloat select 0, getpos _Hfloat select 1, 1]\n~0.1\ngoto "loop"', 1)},
    "lobo_fab/dolp/init.sqs": {
        ("sub", '#loop\n?(Behaviour _sub == "STEALTH")',
         '#loop\n?(not alive _sub): goto "exit"\n?(Behaviour _sub == "STEALTH")', 1),
        ("sub", '?(Behaviour _sub == "SAFE"): _sub animate ["arm1", 0]\n\ngoto "loop"',
         '?(Behaviour _sub == "SAFE"): _sub animate ["arm1", 0]\n~1\ngoto "loop"', 1)},
    "lobomig/mig25/sqs/mig25gear.sqs": {
        ("sub", "#CheckPosLoop\n\n;********** check pilot status",
         "#CheckPosLoop\n~0.5\n\n;********** check pilot status", 1),
        ("sub", '\ngoto "CheckPosition"\n', '\n;goto "CheckPosition"\n', 1)},
    "lobosyrapc/scripts/bmp11anim.sqs": {
        ("sub", '?(not alive _unit ): goto "exit"\n\ngoto "GO"\t', '?(not alive _unit ): goto "exit"\n~0.5\ngoto "GO"\t', 1)},
    "lobof5/scripts/autocanopy.sqs": {
        ("sub", 'goto "CanopyDown"\ngoto "PollLoop"', 'goto "CanopyDown"\n~0.5\ngoto "PollLoop"', 1)},
    "lobo_a4/scripts/canopy.sqs": {
        ("sub", "#Mainloop\n_pos = GetPos _plane1;", "#Mainloop\n?(not alive _plane1): exit\n_pos = GetPos _plane1;", 1),
        ("sub", "~0.0001\n", "~0.5\n", 1)},
    "loboart/scripts/m252start.sqs": {
        ("line", '?CoCM252MaxWind == CoCM252MaxWind:Goto "PrecisionSet"', '?CoCM252MaxWind == CoCM252MaxWind:Goto "WindSet"', 1)},
    "loboart/scripts/m109start.sqs": {
        ("line", '?CoCM109MaxWind == CoCM109MaxWind:Goto "PrecisionSet"', '?CoCM109MaxWind == CoCM109MaxWind:Goto "WindSet"', 1)},
    "loboart/scripts/m1064a3start.sqs": {
        ("line", '?CoCm252MaxWind == CoCm252MaxWind:Goto "PrecisionSet"', '?CoCm252MaxWind == CoCm252MaxWind:Goto "WindSet"', 1)},
    "coc_arty/scripts/m252start.sqs": {
        ("line", '?CoCM252MaxWind == CoCM252MaxWind:Goto "PrecisionSet"', '?CoCM252MaxWind == CoCM252MaxWind:Goto "WindSet"', 1)},
    "coc_arty/scripts/m109start.sqs": {
        ("line", '?CoCM109MaxWind == CoCM109MaxWind:Goto "PrecisionSet"', '?CoCM109MaxWind == CoCM109MaxWind:Goto "WindSet"', 1)},
    "lobo_fab/lcm/scripts/setflags.sqs": {("sub", "#repeat\n~0.001\n", "#repeat\n~1\n", 1)},
    "lobo_fab/lcm/scripts/setflags2.sqs": {("sub", "#repeat\n~0.001\n", "#repeat\n~1\n", 1)},
    "lobo_fab/osa/scripts/setflags.sqs": {("sub", "#repeat\n~0.001\n", "#repeat\n~1\n", 1)},
    "lobo_fab/saar5/scripts/setflags.sqs": {("sub", "#repeat\n~0.001\n", "#repeat\n~1\n", 1)},
    "csjairlines/scripts/aidlights.sqs": {
        ("sub", '_plane setobjecttexture [16,"\\csjairlines\\pristroje.pac"]\ngoto "ckL"\n',
         '_plane setobjecttexture [16,"\\csjairlines\\pristroje.pac"]\n?(alive _plane) && (isengineon _plane): goto "ckL"\n', 1)},
    "csjils/ilsact.sqs": {("line", "_wait = .02", "_wait = 1", 1)},
}

_PLAIN = {("sub", "#Zut\n~0.001\n_J = nearestobject [_X,_Y,_Z]\n",
           "#Zut\n~0.1\n? isNull _L1 : exit\n_J = nearestobject [_X,_Y,_Z]\n", 1)}
_LADUNG = {("sub", "#Zut\n~0.001\n_W = _D + 45\n", "#Zut\n~0.1\n? isNull _L1 : exit\n_W = _D + 45\n", 1)}
_CLAYMORE = {("sub", "#Zut\n~0.01\n_M = _L1\n", "#Zut\n~0.1\n? isNull _L1 : exit\n_M = _L1\n", 1)}
_LEUCHT = {("sub", "#Zut\n~0.0001\n", "#Zut\n~0.03\n? isNull _L1 : deleteVehicle _Mess; exit\n", 1),
           ("sub", '#Ende\n_L1 animate ["aus", 1]\n', '#Ende\ndeleteVehicle _Mess\n_L1 animate ["aus", 1]\n', 1),
           ("sub", "? not alive _L1:exit\n", "? not alive _L1:deleteVehicle _Mess; exit\n", 1)}
MINES = {
    "lobo_mines/scripts/ap_mine.sqs": _PLAIN, "lobo_mines/scripts/at1_mine.sqs": _PLAIN,
    "lobo_mines/scripts/at2_mine.sqs": _PLAIN, "lobo_mines/scripts/at3_mine.sqs": _PLAIN,
    "lobo_mines/scripts/at4_mine.sqs": _PLAIN, "lobo_mines/scripts/at5_mine.sqs": _PLAIN,
    "lobo_mines/scripts/at6_mine.sqs": _PLAIN, "lobo_mines/scripts/egyptnavmine.sqs": _PLAIN,
    "lobo_mines/scripts/m08navmine.sqs": _PLAIN, "lobo_mines/scripts/mk118_mine.sqs": _PLAIN,
    "jof_objects1/ap_mine.sqs": _PLAIN,
    "lobo_mines/scripts/ladung.sqs": _LADUNG, "jof_objects1/ladung.sqs": _LADUNG,
    "lobo_mines/scripts/claymore.sqs": _CLAYMORE, "jof_objects1/claymore.sqs": _CLAYMORE,
    "lobo_mines/scripts/leucht_mine.sqs": _LEUCHT, "jof_objects1/leucht_mine.sqs": _LEUCHT,
}

_AIRAMMO = 'exec "\\LoBo_airammo\\Scripts\\cbu\\rockeye.sqs"'
_DEPLOYED = ("sub", '"LoBoMk20Deployed" createvehicle', '"LoBo_mk20Deployed" createvehicle', 1)
CLUSTERS = {
    "lobo_a4/scripts/firemissiles_a2.sqs": {("sub", 'exec "\\LoBoF16\\scr\\rockeye.sqs"', _AIRAMMO, 1)},
    "lobo_a4/scripts/firemissiles_a3.sqs": {("sub", 'exec "\\LoBoF16\\scr\\rockeye.sqs"', _AIRAMMO, 1)},
    "lobo_f4/scripts/firemissiles_ww.sqs": {("sub", 'exec "\\LoBoF16\\scr\\rockeye.sqs"', _AIRAMMO, 1)},
    "loboil28/script/fire_mk20.sqs": {
        ("sub", 'exec "\\LoBoF15\\scr\\rockeye.sqs"', _AIRAMMO, 1),
        ("sub", "_missobj = nearestobject[_plane, _ammoname]",
         '_missobj = nearestobject[_plane, _ammoname]\n? isNull _missobj: goto "Exit"\n? !(local _missobj): goto "Exit"', 1)},
    "lobof5/scripts/rockeye.sqs": {_DEPLOYED},
    "loboil28/script/rockeye.sqs": {_DEPLOYED},
    "lobo_airammo/scripts/cbu/rbk500.sqs": {("sub", '"LoBoRBKBomblet" createvehicle', '"LoBoRBK500Bomblet" createvehicle', 1)},
}

_VAIR, _VAIR_Q, _TYPE2, _TYPE1 = T2 + "type=VAir;", T2 + 'type="VAir";', T2 + "type=2;", T2 + "type=1;"
_TYPO = "CanseeCompass+CanSeeRadar+CanSeeEye+CanSeeEar;"
_SPEED_MACRO = ("line", T4 + "speed=SPEED_STATIC; \\", T4 + "speed=1e10; \\")
IDENTIFIERS = {
    "coc_arty/config.cpp": {
        ("line", T2 + "driverCanSee = " + _TYPO, T2 + "driverCanSee = 27;", 1),
        ("line", T2 + "drivercansee = " + _TYPO, T2 + "drivercansee = 27;", 1),
        ("line", T2 + "type=Varmor;", _TYPE1, 1),
        _SPEED_MACRO + (2,)},
    "loboart/config.cpp": {
        ("line", T2 + "driverCanSee = " + _TYPO, T2 + "driverCanSee = 27;", 1),
        ("line", T2 + "drivercansee = " + _TYPO, T2 + "drivercansee = 27;", 2),
        ("line", T2 + "type=Varmor;", _TYPE1, 2),
        ("line", T3 + 'speed="SPEED_STATIC";', T3 + "speed=1e10;", 3),
        ("line", T3 + 'duty="RestDuty";', T3 + "duty=-1;", 2),
        ("line", T2 + "weaponType=WeaponHardMounted;", T2 + "weaponType=65536;", 2)},
    "lobohizballcar/config.cpp": {
        ("line", T2 + "gunnerCanSee = CanSeeOptics+CanSeeEar+CanSeeCompass+CanSeeRadar;", T2 + "gunnerCanSee = 29;", 1),
        ("line", T2 + "driverCanSee = CanSeeOptics+CanSeeEar+CanSeeCompass+CanSeeRadar;", T2 + "driverCanSee = 29;", 1)},
    "lobo_110/config.cpp": {("line", T2 + "driverCanSee=CanSeeEye+CanSeeEar;", T2 + "driverCanSee=10;", 1)},
    "lobo_btr60/config.cpp": {
        ("line", S16 + "driverCanSee=CanSeeEye+CanSeeOptics+CanSeeCompass;", S16 + "driverCanSee=22;", 1),
        ("line", S16 + "gunnerCanSee=CanSeeEye+CanSeeOptics+CanSeeCompass;", S16 + "gunnerCanSee=22;", 1)},
    "loboidfhelo/config.cpp": {
        ("line", T2 + "gunnerCanSee = CanSeeAll;", T2 + "gunnerCanSee = 31;", 1),
        ("line", T2 + "gunnerCanSee = CanSeeEye+CanSeeCompass+CanSeeOptics;", T2 + "gunnerCanSee = 22;", 1),
        ("line", T2 + "driverCanSee = CanSeeEye+CanSeeCompass;", T2 + "driverCanSee = 18;", 1),
        ("line", (_VAIR, _VAIR_Q), _TYPE2, 8, 2),
        _SPEED_MACRO + (5,),
        ("line", T3 + "duty = RestDuty;", T3 + "duty = -1;", 1)},
    "lobojorhelo/config.cpp": {
        ("line", T2 + "gunnerCanSee = CanSeeAll;", T2 + "gunnerCanSee = 31;", 1),
        ("line", _VAIR, _TYPE2, 1, 2)},
    "lobomerkava2/config.cpp": {("line", T2 + 'gunnerCanSee="CanSeeAll";', T2 + "gunnerCanSee=31;", 1)},
    "lobomerkava4/config.cpp": {("line", T2 + 'gunnerCanSee="CanSeeAll";', T2 + "gunnerCanSee=31;", 1)},
    "bde_mir3/config.cpp": {("line", T2 + "type \t\t\t\t= VAir;", T2 + "type \t\t\t\t= 2;", 1)},
    "csjairlines/config.cpp": {("line", _VAIR, _TYPE2, 1)},
    "csjcitation/config.cpp": {("line", _VAIR, _TYPE2, 1)},
    "loboc130/config.cpp": {("line", _VAIR, _TYPE2, 1)},
    "loboeghelo/config.cpp": {("line", (_VAIR, _VAIR_Q), _TYPE2, 6)},
    "lobof15/config.cpp": {("line", _VAIR_Q, _TYPE2, 9)},
    "lobof16/config.cpp": {("line", _VAIR, _TYPE2, 13)},
    "loboil28/config.cpp": {("line", _VAIR, _TYPE2, 5)},
    "lobomig/config.cpp": {("line", (_VAIR, _VAIR_Q), _TYPE2, 10)},
    "lobo_a4/config.cpp": {("line", _VAIR, _TYPE2, 1)},
    "lobo_f4/config.cpp": {("line", _VAIR, _TYPE2, 5)},
    "lobo_kfir/config.cpp": {("line", T2 + "type \t= VAir;", T2 + "type \t= 2;", 1)},
    "lobof5/config.cpp": {("line", _VAIR, _TYPE2, 3)},
    "lobocen/config.cpp": {("line", T2 + "type=VArmor;", _TYPE1, 1)},
    "lobochallen/config.cpp": {("line", T2 + "type=VArmor;", _TYPE1, 1)},
    "lobo_fab/config.cpp": {("line", T2 + "type=VArmor;", _TYPE1, 1, 4),
                            ("line", T2 + "hasCommander=o", T2 + "hasCommander=0", 1, 2)},
    "map_oiladdon_script/config.cpp": {("line", T2 + 'type="VArmor";', _TYPE1, 2)},
    "loboegapc/config.cpp": {("line", T3 + 'speed="SPEED_STATIC";', T3 + "speed=1e10;", 33),
                             ("line", T3 + 'duty="RestDuty";', T3 + "duty=-1;", 32)},
    "coc_torpedoes/config.cpp": {("line", T2 + "canLock=LockYes;", T2 + "canLock=2;", 1)},
    # Review deviation from spec 4.6: 0 (UnitInfoSoldier), what the nil token
    # always read as, not 1 (UnitInfoTank); the first draft's 1 is accepted too.
    "lobo_zu23/config.cpp": {("line", (T2 + "UnitInfoType=Helicopter,", T2 + "UnitInfoType=1;"), T2 + "UnitInfoType=0;", 1)},
}

CONTENT = {
    "rksl-hangers/config.cpp": {("line", T2 + "armour=1000;", T2 + "armor=1000;", 3),
                                ("line", T2 + "armour=2000;", T2 + "armor=2000;", 6)},
    "tmyk_bridges/config.cpp": {
        ("line", S8 + 'model = "\\TMYK_Bridges\\TMYK_ViaductCenter.p3d"', S8 + 'model = "\\TMYK_Bridges\\TMYK_Viaduct.p3d";', 1),
        ("line", S8 + 'icon = "\\TMYK_Bridges\\icons\\TMYK_Viaduct.pac";',
         S8 + 'icon = "\\TMYK_Bridges\\icons\\TMYK_ViaductCenter.pac";', 1)},
    "lobois/config.cpp": {("line", T2 + 'model="\\LoBoIs\\LoBo_para.p3d";', T2 + 'model="para";', 1)},
    "bwm_boot3/config.cpp": {("line", T2 + 'crew="BWMOD_Soldier";', T2 + 'crew="SoldierWB";    ', 1)},
}

WEAPONS = {"loboweapons/config.cpp": {
    ("line", T2 + 'picture="\\LoBoWeapons\\LoBoWeapons\\Inventory\\w_1022sd.paa";',
     T2 + 'picture="\\LoBoWeapons\\Inventory\\w_1022sd.paa";', 1),
    ("line", T2 + 'picture="\\LoBoWeapons\\LoBoWeapons\\Inventory\\M_1022sd.paa";',
     T2 + 'picture="\\LoBoWeapons\\Inventory\\M_1022sd.paa";', 1),
    ("line", T2 + 'picture="\\LoBoWeapons\\GearPics\\glock18gear.paa";', T2 + 'picture="\\LoBoWeapons\\GearPics\\glockgear.paa";', 1),
    ("line", T2 + 'picture="\\TheHKpack\\inventory\\w_msg90.paa";', T2 + 'picture="\\LoBoWeapons\\inventory\\w_msg90.paa";', 1),
    ("line", T2 + 'model="\\LoBoWeapons\\LoBo_M4A1_LD_m203.p3d";', T2 + 'model="\\LoBoWeapons\\LoBo_M4A1_m203.p3d";', 1, 1),
    ("line", T2 + 'soundHitBuilding1[]={"\\LoBoWeapons\\Sounds\\KsvkHitBuilding1.wav",0.031623,1};',
     T2 + 'soundHitBuilding1[]={"weapons\\bulletricochet4",0.031623,1};', 1),
    ("line", T2 + 'soundHitBuilding2[]={"\\LoBoWeapons\\Sounds\\KsvkHitBuilding22.wav",0.031623,1};',
     T2 + 'soundHitBuilding2[]={"weapons\\bulletricochet5",0.031623,1};', 1)}}

ICONS = {
    "loboweapons/inventory/w_msg90.paa": ("Inventory\\w_msg90.paa", "inventory/w_msg90.paa",
                                          "df3c75bbf03feac8ae6852664561aa351fb6e0941975e5f62ef0a816776bc507"),
    "loboweapons/inventory/w_psg1.paa": ("Inventory\\w_psg1.paa", "inventory/w_psg1.paa",
                                         "7565c8428f8b80590446844fca9d9f410542aab2a94582cd93eb9fea7557c4f6"),
    "loboweapons/inventory/w_ump.paa": ("Inventory\\w_ump.paa", "inventory/w_ump.paa",
                                        "672af23564079246c0cc078d8a3dfcf9a002b6e4d15801159d87e890121cc1fd"),
    "loboweapons/inventory/w_mp5n.paa": ("Inventory\\w_mp5n.paa", "inventory/w_mp5n.paa",
                                         "3dd51b7a66e18425370e32c3c8b18dd0d469f961234b2b2db35c4433f04f4546"),
    "loboweapons/inventory/w_mp5k.paa": ("Inventory\\w_mp5k.paa", "inventory/w_mp5k.paa",
                                         "3a7d928fb1be5dbdc6637c581689f3d8af32ed6811fe4359f820a46e43b01fa7"),
    "loboweapons/inventory/w_mp5kpure.paa": ("Inventory\\w_mp5kpure.paa", "inventory/w_mp5kpure.paa",
                                             "a9d382e29592a68eeae361c20f4ca2e5069c7b62dfc5856bc40a9e5ff6e923ca"),
    "loboweapons/inventory/m_mp5n.paa": ("Inventory\\m_mp5n.paa", "inventory/m_mp5n.paa",
                                         "f8a0590906680bbbda19f8d90f0a9db7751ffdb0c6350d6a7a6342af1b0e0e06"),
}

TEXTURES = {
    "loboidfhelo/lobo_md500d_r.p3d": ((1e13, 5e15, 7e15), {
        b"bas_lbs\\m\\uh1ffarpod.pac": (b"LoboIDFhelo\\MD\\uh1ffarpod.pac", 10, False),
        b"bas_lbs\\m\\uh1podend.pac": (b"LoboIDFhelo\\MD\\uh1podend.pac", 8, False)}),
    "lobojordan/lobo_crew_jor.p3d": ((5.0,), {
        b"LoBoJordan\\wl\\rba_frt.paa": (b"LoBoJordan\\wl\\ba_frt.paa", 26, False),
        b"LoBoJordan\\wl\\rba_bk.paa": (b"LoBoJordan\\wl\\ba_bk.paa", 26, False)}),
    "lobosyria/lobo_crew_syria.p3d": ((5.0,), {
        b"LoBoSyria\\syr_crew\\e_tank_hrud_p": (b"LoBoSyria\\crew\\hrud_p.pac", 23, True),
        b"LoBoSyria\\syr_crew\\e_tank_nohy_p": (b"LoBoSyria\\crew\\nohy_p.pac", 15, True),
        b"LoBoSyria\\syr_crew\\e_tank_nohy_z": (b"LoBoSyria\\crew\\nohy_z.pac", 6, True),
        b"LoBoSyria\\syr_crew\\e_tank_zada_z": (b"LoBoSyria\\crew\\zada_z.pac", 18, True),
        b"LoBoSyria\\syr_crew\\00000mc_tanki": (b"data\\00000mc_tankistae2.paa", 38, True),
        b"LoBoSyria\\syr_crew\\00002mc_tanki": (b"data\\00002mc_tankistae2.paa", 24, True)}),
}

NUMBERS = {
    "lobof15/scr/number.sqs": (2, True),
    "lobof16/scr/number.sqs": (2, True),
    "lobof16/scr/number2.sqs": (2, True),
    "loboil28/script/number.sqs": (0, True),
    "lobo_f4/scripts/number.sqs": (1, False),
}

GLOW_BLOCK = (
    "class CfgVehicles\n{\n\tclass All {};\n\tclass Static: All {};\n\tclass Building: Static {};\n"
    "\tclass NonStrategic: Building {};\n\tclass Fire: NonStrategic {};\n"
    "\t// UD repair, LoBo issue 9: invisible fireplace that the wreck fire.sqs\n"
    "\t// scripts create for inflame and say. Vanilla Fire supplies simulation,\n"
    "\t// sound, Light and Smoke; only the campfire mesh is removed.\n"
    "\tclass LoBo_WreckGlow: Fire\n\t{\n\t\tscope=1;\n\t\tmodel=\"empty\";\n\t\tdisplayName=\"Wreck fire\";\n\t};\n"
    "\tclass M1Smoker: LoBo_WreckGlow {};\n\tclass M60Smoker: LoBo_WreckGlow {};\n"
    "\tclass Smoker: LoBo_WreckGlow {};\n\tclass KioGlowFire: LoBo_WreckGlow {};\n};\n"
)
# Review addition to spec 4.5: the ground-script burn policy for the five
# helicopter copies the defined KioGlowFire makes reachable.
HELI_BURN = {
    ("sub", "_lifeTick = _lifeTicks\n_ouch = 0\n", "_lifeTick = _lifeTicks\n_ouch = 0\n_fire = 0\n", 1),
    ("sub", '#client\n\n;burn player\n? player distance _glow <10: player setdammage ((getdammage player) +0.04),goto "scream"\n',
     '#client\n? !(local _object): goto "fire"\n\n;burn player\n? player != (vehicle player): goto "fire"\n'
     '? local player && alive player && player distance _object < 10 && _ouch != 1: '
     'player setdammage ((getdammage player) +0.04); goto "scream"\n', 1),
    ("sub", "_lifeTick = _lifeTick - 1\n_soundloop = _soundloop +1\n",
     "_lifeTick = _lifeTick - 1\n_soundloop = _soundloop +1\n_fire = _fire + 1\n? _fire >= 40: _fire = 0; _ouch = 0\n", 1),
}
HELI_MEMBERS = {"loboeghelo/engine/fire.sqs", "loboeghelo/engine2/fire.sqs", "loboeghelo/engine3/fire.sqs",
                "loboidfhelo/ch53/engine/fire.sqs", "loboidfhelo/uh1/engine/fire.sqs"}


class RuleLiteralTests(unittest.TestCase):
    def assert_table(self, expected, rules):
        self.assertEqual(set(expected), set(rules))
        actual = as_sets(rules)
        for member in expected:
            with self.subTest(member=member):
                self.assertEqual(expected[member], actual[member])

    def test_loops(self):
        self.assert_table(LOOPS, loops.RULES)

    def test_mines(self):
        self.assert_table(MINES, mines.RULES)
        self.assertEqual(("~0.1", "~0.03"), (mines.POLL, mines.FLARE_POLL))

    def test_clusters(self):
        self.assert_table(CLUSTERS, clusters.RULES)
        self.assertEqual(("LoBo_mk20Deployed", "LoBoMk20Bomblet", "LoBoRBK500Bomblet"), clusters.REQUIRED_CLASSES)

    def test_identifiers(self):
        self.assert_table(IDENTIFIERS, identifiers.RULES)

    def test_content(self):
        self.assert_table(CONTENT, content.RULES)
        self.assertEqual(("bwm_boot3/config.cpp",), content.SAME_LENGTH)

    def test_weapons_and_icon_pins(self):
        self.assert_table(WEAPONS, weapons.RULES)
        self.assertEqual(ICONS, weapons.ICONS)
        self.assertEqual("79cdb0bb937e9e05d43781da0337af36f6d3cca93e4d09658e9847a4d70b3ff5", weapons.HK_PACK_SHA256)

    def test_textures(self):
        self.assertEqual(TEXTURES, textures.PLANS)

    def test_numbers(self):
        self.assertEqual(NUMBERS, numbers.NUMBER_SCRIPTS)
        self.assertEqual("; Init provides only the vehicle; an unused second argument is unnecessary.",
                         numbers.UNUSED_ARGUMENT)

    def test_effects(self):
        self.assertEqual(GLOW_BLOCK, effects.BLOCK)
        self.assertEqual(HELI_MEMBERS, set(effects.RULES))
        for member, rules in effects.RULES.items():
            with self.subTest(member=member):
                self.assertEqual(HELI_BURN, set(rules))


if __name__ == "__main__":
    unittest.main()
