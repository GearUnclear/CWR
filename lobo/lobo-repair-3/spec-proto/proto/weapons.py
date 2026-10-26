"""SPEC PROTOTYPE: LoBoWeapons references to missing files.

Text: R10/22 doubled bank prefix, Glock 17 icon, MSG90 icon bank, M4A1 LD M203
model, KSVK building-hit sounds. Members: seven HK inventory icons LoBo
referenced but never shipped, copied byte-for-byte from a user-supplied,
hash-pinned TheHKpack.pbo 1.75 (Vipersheart and the LoBo team). The icon bytes
are never part of this repository.
"""
import hashlib
from pathlib import Path

from repair.pbo import Archive

from .common import apply_rules, p

CONFIG = "loboweapons/config.cpp"
HK_PACK_SHA256 = "79cdb0bb937e9e05d43781da0337af36f6d3cca93e4d09658e9847a4d70b3ff5"
HK_PACK_NAME = "TheHKpack.pbo 1.75"
# added member key -> (stored name in LoBoWeapons, source member in TheHKpack, sha256)
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
ADDED_MEMBERS = tuple(ICONS)
RULES = {CONFIG: (
    ("line", '\t\tpicture="' + p("LoBoWeapons", "LoBoWeapons", "Inventory", "w_1022sd.paa") + '";',
     '\t\tpicture="' + p("LoBoWeapons", "Inventory", "w_1022sd.paa") + '";', 1),
    ("line", '\t\tpicture="' + p("LoBoWeapons", "LoBoWeapons", "Inventory", "M_1022sd.paa") + '";',
     '\t\tpicture="' + p("LoBoWeapons", "Inventory", "M_1022sd.paa") + '";', 1),
    ("line", '\t\tpicture="' + p("LoBoWeapons", "GearPics", "glock18gear.paa") + '";',
     '\t\tpicture="' + p("LoBoWeapons", "GearPics", "glockgear.paa") + '";', 1),
    ("line", '\t\tpicture="' + p("TheHKpack", "inventory", "w_msg90.paa") + '";',
     '\t\tpicture="' + p("LoBoWeapons", "inventory", "w_msg90.paa") + '";', 1),
    ("line", '\t\tmodel="' + p("LoBoWeapons", "LoBo_M4A1_LD_m203.p3d") + '";',
     '\t\tmodel="' + p("LoBoWeapons", "LoBo_M4A1_m203.p3d") + '";', 1, 1),
    ("line", '\t\tsoundHitBuilding1[]={"' + p("LoBoWeapons", "Sounds", "KsvkHitBuilding1.wav") + '",0.031623,1};',
     '\t\tsoundHitBuilding1[]={"weapons' + chr(92) + 'bulletricochet4",0.031623,1};', 1),
    ("line", '\t\tsoundHitBuilding2[]={"' + p("LoBoWeapons", "Sounds", "KsvkHitBuilding22.wav") + '",0.031623,1};',
     '\t\tsoundHitBuilding2[]={"weapons' + chr(92) + 'bulletricochet5",0.031623,1};', 1),
)}
ADDONS = ("loboweapons",)


def repair(documents):
    return apply_rules(documents, RULES)


def add_members(existing, asset_source=None):
    """existing: {key: bytes} for the ADDED_MEMBERS already in the archive.

    Returns ({key: (stored_name, bytes)}, [skip report lines]). Present members
    must already carry the pinned bytes; nothing is added without a verified
    source, and a wrong source is an error even when nothing is missing.
    """
    for key, data in existing.items():
        if hashlib.sha256(data).hexdigest() != ICONS[key][2]:
            raise ValueError(f"{key}: present but not the pinned {HK_PACK_NAME} icon")
    missing = [key for key in ICONS if key not in existing]
    source = None
    if asset_source is not None:
        raw = Path(asset_source).read_bytes()
        if hashlib.sha256(raw).hexdigest() != HK_PACK_SHA256:
            raise ValueError(f"--asset-source is not {HK_PACK_NAME} (sha256 {HK_PACK_SHA256})")
        source = Archive(raw)
    if not missing:
        return {}, []
    if source is None:
        return {}, [f"loboweapons: {len(missing)} HK inventory icon(s) not added: pass --asset-source "
                    f"<{HK_PACK_NAME}, sha256 {HK_PACK_SHA256}> ({', '.join(k.rsplit('/', 1)[1] for k in missing)})"]
    added = {}
    for key in missing:
        stored, member, digest = ICONS[key]
        entry = source.by_key[member]
        if entry.fields[0] != 0:
            raise ValueError(f"{HK_PACK_NAME}: {member} is compressed")
        data = bytes(source.stored(entry))
        if hashlib.sha256(data).hexdigest() != digest:
            raise ValueError(f"{HK_PACK_NAME}: {member} does not match its pinned sha256")
        added[key] = (stored, data)
    return added, []
