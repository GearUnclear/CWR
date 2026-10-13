"""LoBoWeapons references to files that do not exist.

Text: the R10/22 doubled bank prefix, the Glock 17 icon, the MSG90 icon bank,
the M4A1 LD M203 model (the parent model inheritance would give) and the KSVK
building-hit sounds (the vanilla ricochets the class would inherit).

Members: seven HK inventory icons LoBo referenced but never shipped, copied
byte for byte from a user-supplied TheHKpack.pbo 1.75 (OFP: HK, "The HK Pack",
by the LoBo Team and Vipersheart, 2002) whose seven members match their pinned
SHA-256 values (the original 1.75 archive or the @LoBo_Extras copy, which
differs only in its patched config.bin). The icon bytes are never part of this
repository; without a source the icons are reported as skipped and nothing is
added.

The MSG90 picture names the authored \\TheHKpack\\ bank, which resolves only
when @LoBo_Extras (TheHKpack.pbo, no prefix) is mounted. It is repointed to the
LoBoWeapons copy only when that icon is present or added in the same plan;
otherwise the authored path is kept (and a repoint made without the icon is
put back), so the repair never hides an icon Extras would show.
"""
import hashlib
from pathlib import Path

from .pbo import Archive
from .rules import apply_rules, p

BS = chr(92)
CONFIG = "loboweapons/config.cpp"
HK_PACK_SHA256 = "79cdb0bb937e9e05d43781da0337af36f6d3cca93e4d09658e9847a4d70b3ff5"
HK_PACK_NAME = "TheHKpack.pbo 1.75"
# added member key -> (stored name in LoBoWeapons, source member in TheHKpack, sha256)
ICONS = {
    "loboweapons/inventory/w_msg90.paa": ("Inventory" + BS + "w_msg90.paa", "inventory/w_msg90.paa",
                                          "df3c75bbf03feac8ae6852664561aa351fb6e0941975e5f62ef0a816776bc507"),
    "loboweapons/inventory/w_psg1.paa": ("Inventory" + BS + "w_psg1.paa", "inventory/w_psg1.paa",
                                         "7565c8428f8b80590446844fca9d9f410542aab2a94582cd93eb9fea7557c4f6"),
    "loboweapons/inventory/w_ump.paa": ("Inventory" + BS + "w_ump.paa", "inventory/w_ump.paa",
                                        "672af23564079246c0cc078d8a3dfcf9a002b6e4d15801159d87e890121cc1fd"),
    "loboweapons/inventory/w_mp5n.paa": ("Inventory" + BS + "w_mp5n.paa", "inventory/w_mp5n.paa",
                                         "3dd51b7a66e18425370e32c3c8b18dd0d469f961234b2b2db35c4433f04f4546"),
    "loboweapons/inventory/w_mp5k.paa": ("Inventory" + BS + "w_mp5k.paa", "inventory/w_mp5k.paa",
                                         "3a7d928fb1be5dbdc6637c581689f3d8af32ed6811fe4359f820a46e43b01fa7"),
    "loboweapons/inventory/w_mp5kpure.paa": ("Inventory" + BS + "w_mp5kpure.paa", "inventory/w_mp5kpure.paa",
                                             "a9d382e29592a68eeae361c20f4ca2e5069c7b62dfc5856bc40a9e5ff6e923ca"),
    "loboweapons/inventory/m_mp5n.paa": ("Inventory" + BS + "m_mp5n.paa", "inventory/m_mp5n.paa",
                                         "f8a0590906680bbbda19f8d90f0a9db7751ffdb0c6350d6a7a6342af1b0e0e06"),
}
ADDED_MEMBERS = tuple(ICONS)
MSG90 = "loboweapons/inventory/w_msg90.paa"
MSG90_AUTHORED = '\t\tpicture="' + p("TheHKpack", "inventory", "w_msg90.paa") + '";'
MSG90_ADDED = '\t\tpicture="' + p("LoBoWeapons", "inventory", "w_msg90.paa") + '";'
# The table as applied when every icon is present (the installed state).
RULES = {CONFIG: (
    ("line", '\t\tpicture="' + p("LoBoWeapons", "LoBoWeapons", "Inventory", "w_1022sd.paa") + '";',
     '\t\tpicture="' + p("LoBoWeapons", "Inventory", "w_1022sd.paa") + '";', 1),
    ("line", '\t\tpicture="' + p("LoBoWeapons", "LoBoWeapons", "Inventory", "M_1022sd.paa") + '";',
     '\t\tpicture="' + p("LoBoWeapons", "Inventory", "M_1022sd.paa") + '";', 1),
    ("line", '\t\tpicture="' + p("LoBoWeapons", "GearPics", "glock18gear.paa") + '";',
     '\t\tpicture="' + p("LoBoWeapons", "GearPics", "glockgear.paa") + '";', 1),
    # The added icon occupies the LoBoWeapons path; see rules_for().
    ("line", MSG90_AUTHORED, MSG90_ADDED, 1),
    ("line", '\t\tmodel="' + p("LoBoWeapons", "LoBo_M4A1_LD_m203.p3d") + '";',
     '\t\tmodel="' + p("LoBoWeapons", "LoBo_M4A1_m203.p3d") + '";', 1, 1),
    ("line", '\t\tsoundHitBuilding1[]={"' + p("LoBoWeapons", "Sounds", "KsvkHitBuilding1.wav") + '",0.031623,1};',
     '\t\tsoundHitBuilding1[]={"weapons' + BS + 'bulletricochet4",0.031623,1};', 1),
    ("line", '\t\tsoundHitBuilding2[]={"' + p("LoBoWeapons", "Sounds", "KsvkHitBuilding22.wav") + '",0.031623,1};',
     '\t\tsoundHitBuilding2[]={"weapons' + BS + 'bulletricochet5",0.031623,1};', 1),
)}
ADDONS = ("loboweapons",)


def rules_for(present) -> dict:
    """RULES when the MSG90 icon is present (or added); otherwise keep the
    authored \\TheHKpack\\ path and undo a repoint that has no icon behind it."""
    if MSG90 in present:
        return RULES
    return {CONFIG: tuple(("line", MSG90_ADDED, MSG90_AUTHORED, 1) if rule[1] == MSG90_AUTHORED else rule
                          for rule in RULES[CONFIG])}


def repair(documents: dict[str, str], present=frozenset()) -> dict[str, str]:
    """present: the ADDED_MEMBERS keys that exist after this plan (repair_lobo.plan passes them)."""
    return apply_rules(documents, rules_for(present))


ASSET_SOURCE_NAME = f"{HK_PACK_NAME} (or @LoBo_Extras/addons/TheHKpack.pbo) for the HK icons"


def claims_source(archive: Archive) -> bool:
    """An --asset-source is this module's when it holds any of the seven icon
    member names; _source then requires all seven pinned bytes (fail closed)."""
    return any(member in archive.by_key for _, member, _ in ICONS.values())


def _one(asset_source):
    """A path, None, or the tuple repair_lobo.plan passes (at most one HK pack)."""
    if asset_source is None or isinstance(asset_source, (str, Path)):
        return asset_source
    paths = tuple(asset_source)
    if len(paths) > 1:
        raise ValueError(f"Several --asset-source archives claim to be {HK_PACK_NAME}: "
                         + ", ".join(str(p) for p in paths))
    return paths[0] if paths else None


def _source(asset_source) -> Archive:
    """Accept a source only when all seven icon members carry their pinned bytes."""
    raw = Path(asset_source).read_bytes()
    try:
        archive = Archive(raw)
    except ValueError as exc:
        raise ValueError(f"--asset-source is not a readable PBO ({exc}); expected {HK_PACK_NAME}") from exc
    for _, member, pin in ICONS.values():
        entry = archive.by_key.get(member)
        if entry is None:
            raise ValueError(f"--asset-source is not {HK_PACK_NAME}: missing {member}")
        if entry.fields[0] != 0:
            raise ValueError(f"--asset-source is not {HK_PACK_NAME}: {member} is compressed")
        if hashlib.sha256(archive.stored(entry)).hexdigest() != pin:
            raise ValueError(f"--asset-source is not {HK_PACK_NAME}: {member} does not match its pinned sha256")
    return archive


def add_members(existing: dict[str, bytes], asset_source=None) -> tuple[dict, list[str]]:
    """existing: {key: bytes} for the ADDED_MEMBERS already in the archive.

    Returns ({key: (stored name, bytes)}, [skip report lines]). Present members
    must already carry the pinned bytes; nothing is added without a verified
    source, and a wrong source (any of the seven members absent, compressed or
    not its pinned bytes) is an error even when nothing is missing. Additions
    follow the ICONS order.
    """
    asset_source = _one(asset_source)
    unknown = sorted(set(existing) - set(ICONS))
    if unknown:
        raise ValueError("Unknown added members: " + ", ".join(unknown))
    for key, data in existing.items():
        if hashlib.sha256(data).hexdigest() != ICONS[key][2]:
            raise ValueError(f"{key}: present but not the pinned {HK_PACK_NAME} icon")
    missing = [key for key in ICONS if key not in existing]
    source = _source(asset_source) if asset_source is not None else None
    if not missing:
        return {}, []
    if source is None:
        names = ", ".join(key.rsplit("/", 1)[1] for key in missing)
        kept = " (the MSG90 picture keeps its TheHKpack path, shown only with @LoBo_Extras)" if MSG90 in missing else ""
        return {}, [f"loboweapons: {len(missing)} HK inventory icon(s) not added ({names}){kept}: pass "
                    f"--asset-source <{HK_PACK_NAME}, e.g. sha256 {HK_PACK_SHA256}, or @LoBo_Extras/addons/TheHKpack.pbo>"]
    added = {}
    for key in missing:
        stored, member, _ = ICONS[key]
        added[key] = (stored, bytes(source.stored(source.by_key[member])))
    return added, []
