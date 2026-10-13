"""Byte-for-byte restoration of files LoBo references but its final release lacks (2026-10-11.4).

Each member below is named by the installed LoBo data, missing from the archive
that should hold it, and shipped under the same name, in the same addon, by an
earlier LoBo release we hold. It is copied unchanged (bytes and stored name)
from that release's pbo; nothing is generated, converted or repointed.

- LoBo_airammo (Jordanian/Syrian pack, 2007-05-26): CfgAmmo LoBo_MK81 and the
  LoBo_MK8x_LDGB classes and CfgNonAIVehicles ProxyLoBo_Mk81 and
  ProxyLoBo_Mk8x_LDGB name LoBo_Mk81 and LoBo_Mk8x_LDGB(_d), which this
  release dropped (the MiG-21bis MR1 and MR2 and the A-4 carry the Mk-81, the
  F-15 and F-16I the LDGB bombs). LoBoAddonPack3's LoBo_airammo.pbo
  (2007-01-06) has all nine: MLOD 257 with two LODs (visual and Geometry), no
  proxies, the only texture LoBo_airammo\\tex\\bomb.pac (same bytes in both
  releases).
- LoBoEgypt (LoBoAddonPack3, 2006-12-16): LOD 10 of loboegyhi, loboegygrd,
  loboegypara and loboegyamphi.p3d (1, 19, 1 and 1 faces) names
  LoBoEgypt\\inf\\helm_s_in.pac, which that release dropped. LoBoAddonPack1
  (2004-06-11) and LoBoDemo2 (2004-08-25) ship it with the same bytes; in
  those releases it was the helmet texture of LoBoEgyHI's first LOD (126
  faces). DXT1 256x256, 7 levels, opaque.

The sources are user-supplied pbos (--asset-source), one per release, matched
to this rule by content and accepted only when the whole archive equals one of
its pinned SHA-256 values and every member it supplies equals its own pin
(stored, not compressed). The library keeps byte-identical extracts of the
release archives' pbos under sources\\recovered-2026-10-11\\derived (7-Zip; see
the NOTE there). A source that is not given leaves its members out with a skip
line; a present member must already carry its pinned bytes. The bytes are
never part of this repository. Profile-neutral: plain archive members.
"""
import hashlib
from pathlib import Path

from .pbo import Archive

BS = chr(92)
LIBRARY = "D:/LoBo-Reloaded/sources/recovered-2026-10-11/derived/"
# source id -> (description, {archive sha256: provenance}, library file name)
SOURCES = {
    "pack3-airammo": (
        "LoBo_airammo.pbo of LoBoAddonPack3 (2007-01-06)",
        {"de7f4728cc41bd98e93a40e8d01d4816632a39805f191dc05ea6c023b56489fd":
         "LoBoAddonPack3.rar (sha256 b36101e2cc4e201ac129ca1557d88ff1ff17cebecace5d5815aba5e1513b4f18), "
         "member LoBoAddonPack3\\addons\\LoBo_airammo.pbo, 7,597,104 bytes"},
        "LoBo_airammo-LoBoAddonPack3-2007-01-06.pbo"),
    "pack1-egypt": (
        "LoBoEgypt.pbo of LoBoAddonPack1 (2004-06-11) or LoBoDemo2 (2004-08-25)",
        {"992d304a27fefae4105da52cfc58fd9ae6ae0c33d9a963d0ee20161ce77340e9":
         "LoBoAddonPack1.rar (sha256 76a879698585fa878695ab8eea5edf18d3b2ba23011686e1f80b07ec74202c30), "
         "member LoBoEgypt.pbo, 14,432,312 bytes",
         "f7fe8ca93ca6502f4dbfee693765945a1fc02f6d581f6a3ef7718b7b7cd12052":
         "LoBoDemo2.rar (sha256 042e8fb3341692bed25fa84fa9f0f56b0c4ecab2a231a2a4cdf1730072e1393b), "
         "member LoBoEgypt.pbo, 23,194,556 bytes"},
        "LoBoEgypt-LoBoAddonPack1-2004-06-11.pbo"),
}
# added member key -> (source id, stored name in the target and the source, sha256, bytes)
MEMBERS = {
    "lobo_airammo/lobo_mk81.p3d": ("pack3-airammo", "LoBo_Mk81.p3d",
                                   "02e83da4b977cc78bfe351357423dab55064dd65ac450d07a97f0092737a5336", 23291),
    "lobo_airammo/lobo_mk81_ldgb.p3d": ("pack3-airammo", "LoBo_Mk81_LDGB.p3d",
                                        "fd59a0bfdfaa93e6d690efe835f9d39a360ceb0eaaa7f7817769f648c34980d9", 25562),
    "lobo_airammo/lobo_mk81_ldgb_d.p3d": ("pack3-airammo", "LoBo_Mk81_LDGB_d.p3d",
                                          "54a6e76b488551b83bbe8bc0c2ce0c82bad97b80d498010d62792998c12813cd", 30592),
    "lobo_airammo/lobo_mk82_ldgb.p3d": ("pack3-airammo", "LoBo_Mk82_LDGB.p3d",
                                        "51254074addff3a70adac198d51bfb9a7f19bf026d7655cc901e46b7e98e42bc", 25968),
    "lobo_airammo/lobo_mk82_ldgb_d.p3d": ("pack3-airammo", "LoBo_Mk82_LDGB_d.p3d",
                                          "3ce1aad6ec76a4ee35831015e64f060a1767878a7183f9ddc2bfb33ec39e5adc", 30592),
    "lobo_airammo/lobo_mk83_ldgb.p3d": ("pack3-airammo", "LoBo_Mk83_LDGB.p3d",
                                        "65dc83bef7944e27bd2f9311eb74c3f9806e05d1dee9f88850e7dca95d686806", 26050),
    "lobo_airammo/lobo_mk83_ldgb_d.p3d": ("pack3-airammo", "LoBo_Mk83_LDGB_d.p3d",
                                          "18b13a48e160f4b605f282c671042b6fa500c6488536ec80392e1b13a7a55567", 30674),
    "lobo_airammo/lobo_mk84_ldgb.p3d": ("pack3-airammo", "LoBo_Mk84_LDGB.p3d",
                                        "5a4436dc254c8b8d4943e14bb2da6413d057e93198e15ef3e4bf392d28b5adc5", 26050),
    "lobo_airammo/lobo_mk84_ldgb_d.p3d": ("pack3-airammo", "LoBo_Mk84_LDGB_d.p3d",
                                          "09db3129eb6bc21d2495488af9c9a6c18f4c8755f090d66032bf020d38da38f5", 30592),
    "loboegypt/inf/helm_s_in.pac": ("pack1-egypt", "inf" + BS + "helm_s_in.pac",
                                    "e21bd718fa96dd5b6e51c29af5f1a8ec1332d55461f4505ae10d9a00be874669", 43839),
}
ADDED_MEMBERS = tuple(MEMBERS)
ADDONS = tuple(sorted({key.split("/", 1)[0] for key in MEMBERS}))
ASSET_SOURCE_NAME = "; ".join(f"{desc} ({sum(1 for m in MEMBERS.values() if m[0] == sid)} member(s))"
                              for sid, (desc, _, _) in SOURCES.items())


def _source_key(key: str) -> str:
    """The member's key inside its source pbo (the target key without the addon)."""
    return key.split("/", 1)[1]


def claims_source(archive: Archive) -> bool:
    """An --asset-source is this rule's when it is a pinned source archive or
    holds any restored member name; add_members then requires the archive pin
    and every member pin (fail closed)."""
    if hashlib.sha256(archive.data).hexdigest() in {h for _, pins, _ in SOURCES.values() for h in pins}:
        return True
    return any(_source_key(key) in archive.by_key for key in MEMBERS)


def _paths(asset_source) -> tuple:
    if asset_source is None:
        return ()
    if isinstance(asset_source, (str, Path)):
        return (Path(asset_source),)
    return tuple(Path(p) for p in asset_source)


def _sources(asset_source) -> dict:
    """{source id: Archive}: each given path must be one pinned source archive,
    at most one per source, and carry every member of that source with its pin."""
    found = {}
    for path in _paths(asset_source):
        raw = path.read_bytes()
        try:
            archive = Archive(raw)
        except ValueError as exc:
            raise ValueError(f"--asset-source {path} is not a readable PBO ({exc})") from exc
        digest = hashlib.sha256(raw).hexdigest()
        ids = [sid for sid, (_, pins, _) in SOURCES.items() if digest in pins]
        if not ids:
            # Name which source it looks like, so a modified copy is reported as such.
            like = sorted({MEMBERS[key][0] for key in MEMBERS if _source_key(key) in archive.by_key})
            wanted = " or ".join(SOURCES[sid][0] for sid in like) or "a pinned LoBo source archive"
            pins = ", ".join(h for sid in like for h in SOURCES[sid][1]) or "see repair/restore.py"
            raise ValueError(f"--asset-source {path} is not {wanted}: sha256 {digest} is not pinned ({pins})")
        sid = ids[0]
        if sid in found:
            raise ValueError(f"Several --asset-source archives are {SOURCES[sid][0]}")
        for key, (owner, _, pin, size) in MEMBERS.items():
            if owner != sid:
                continue
            entry = archive.by_key.get(_source_key(key))
            if entry is None:
                raise ValueError(f"--asset-source {path} lacks {_source_key(key)}")
            if entry.fields[0] != 0 or entry.fields[1] not in (0, entry.fields[4]) or entry.fields[4] != size:
                raise ValueError(f"--asset-source {path}: {_source_key(key)} is compressed or not {size} bytes")
            if hashlib.sha256(archive.stored(entry)).hexdigest() != pin:
                raise ValueError(f"--asset-source {path}: {_source_key(key)} does not match its pinned sha256")
        found[sid] = archive
    return found


def repair(documents: dict[str, str]) -> dict[str, str]:
    # plan() calls repair() on every module; this one has no text rules.
    return {}


def add_members(existing: dict[str, bytes], asset_source=None) -> tuple[dict, list[str]]:
    """existing: {key: bytes} for the ADDED_MEMBERS already in their archives.

    Returns ({key: (stored name, bytes)}, [skip lines]) in MEMBERS order.
    Present members must carry their pinned bytes; a given source must be a
    pinned archive with every pinned member, even when nothing is missing.
    A member whose source is not given is reported, one line per source.
    """
    unknown = sorted(set(existing) - set(MEMBERS))
    if unknown:
        raise ValueError("Unknown added members: " + ", ".join(unknown))
    for key, data in existing.items():
        if hashlib.sha256(data).hexdigest() != MEMBERS[key][2]:
            raise ValueError(f"{key}: present but not the pinned {SOURCES[MEMBERS[key][0]][0]} member")
    sources = _sources(asset_source)
    added, skipped = {}, {}
    for key, (sid, stored, _, _) in MEMBERS.items():
        if key in existing:
            continue
        if sid not in sources:
            skipped.setdefault(sid, []).append(key)
            continue
        archive = sources[sid]
        added[key] = (stored, bytes(archive.stored(archive.by_key[_source_key(key)])))
    lines = []
    for sid, keys in skipped.items():
        addon = keys[0].split("/", 1)[0]
        names = ", ".join(MEMBERS[key][1].rsplit(BS, 1)[-1] for key in keys)
        lines.append(f"{addon}: {len(keys)} member(s) not restored ({names}): pass --asset-source "
                     f"<{SOURCES[sid][0]}, sha256 {next(iter(SOURCES[sid][1]))}, e.g. {LIBRARY}{SOURCES[sid][2]}>")
    return added, lines
