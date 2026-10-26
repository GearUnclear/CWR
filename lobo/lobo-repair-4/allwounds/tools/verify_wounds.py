"""Read-only proof for the all-pilot wounds rule (2026-10-11.4).

1. plan() the installed @LoBo (UD, read-only) and take the rebuilt LoBo_Pilot
   config and added members from its output.
2. Parse the original and rebuilt config with PoseidonTools `config tojson`,
   resolve CfgVehicles inheritance, and for every class whose model is a
   LoBo_Pilot p3d prove: each body face texture of the model (a part with a
   _d texture: back, legsbk, front, armle, armri, legsfr, legslft, legsrgt)
   has a wounds[] healthy match (WoundInfo::Load: leading backslash dropped,
   lowercase, exact extension), and each wounded target of a matching pair is
   an archive member (shipped or added).
3. Rebuild LoBo_Pilot.pbo in scratch from the plan and re-plan references +
   wounds on a scratch mod (copies of the other references addons): zero changes.
Nothing under D:/Arma_CWA/@LoBo is written.
"""
import collections
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess
import sys
from unittest.mock import patch

LOBO = Path(r"D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo")
sys.path.insert(0, str(LOBO))
import repair_lobo as runner  # noqa: E402
from repair import references, wounds  # noqa: E402
from repair.config import class_block, tokens  # noqa: E402
from repair.pbo import Archive  # noqa: E402

MOD = Path(r"D:/Arma_CWA/@LoBo")
TOOLS = Path(r"D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe")
ASSET = Path(r"D:/LoBo-Reloaded/sources/recovered-2026-10-11/derived/TheHKpack-1.75.pbo")
OUT = Path(sys.argv[1])
OUT.mkdir(parents=True, exist_ok=True)
BS = chr(92)
PARTS = set(wounds.PARTS)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def lods(data):
    _, _, count = struct.unpack_from("<4sII", data)
    pos, out = 12, []
    for _ in range(count):
        _, header, _, points, normals, faces, _ = struct.unpack_from("<4s6i", data, pos)
        ft = pos + header + points * 16 + normals * 12
        names = [data[ft + j * 104:ft + j * 104 + 32].split(b"\0", 1)[0].decode("latin1") for j in range(faces)]
        pos = ft + faces * 104 + 4
        while True:
            raw, size = struct.unpack_from("<64si", data, pos)
            pos += 68 + size
            if raw.split(b"\0", 1)[0] == b"#EndOfFile#":
                break
        res = struct.unpack_from("<f", data, pos)[0]
        pos += 4
        out.append((res, names))
    return out


def norm(name):
    return name.lstrip(BS + "/").replace("/", BS).lower()


def tojson(text_bytes, name):
    path = OUT / (name + ".cpp")
    path.write_bytes(text_bytes)
    run = subprocess.run([str(TOOLS), "config", "tojson", str(path), "-o", str(OUT / (name + ".json"))],
                         capture_output=True, text=True)
    if run.returncode:
        raise SystemExit(f"tojson {name}: {run.returncode} {run.stderr}")
    return json.loads((OUT / (name + ".json")).read_text(encoding="utf8", errors="replace"))


def find_key(node, key):
    for k, v in node.items():
        if k.lower() == key.lower():
            return v
    return None


def parents(text):
    """{class: base} of the CfgVehicles direct classes, from the config tokens
    (tojson drops the base class)."""
    start, end = class_block(text, ("CfgVehicles",))
    ts = tokens(text[start:end])
    out, depth, i = {}, 0, 0
    while i < len(ts):
        value = ts[i].text
        if value == "{":
            depth += 1
        elif value == "}":
            depth -= 1
        elif depth == 0 and value.lower() == "class":
            name = ts[i + 1].text
            out[name] = ts[i + 3].text if ts[i + 2].text == ":" else None
        i += 1
    return out


def classes(cfg, text):
    """{name: (parent or None, {lowercase key: value})}: values from tojson, bases from the source."""
    veh = find_key(cfg, "CfgVehicles")
    bases = parents(text)
    out = {}
    for name, node in veh.items():
        if isinstance(node, dict):
            out[name] = (bases[name], {k.lower(): v for k, v in node.items()})
    if set(out) != {n for n in bases if isinstance(veh.get(n), dict)}:
        raise SystemExit("class lists differ between tojson and the source")
    return out


def leaves(node, path=()):
    if isinstance(node, dict):
        for k, v in node.items():
            yield from leaves(v, path + (k,))
    elif isinstance(node, list):
        for i, v in enumerate(node):
            yield from leaves(v, path + (i,))
    else:
        yield path, node


# 1. plan (read-only)
paths, hashes, changed, records, docs, skipped = runner.plan(MOD, TOOLS, "ud", ASSET)
pilot_pbo = paths["lobo_pilot"]
raw = pilot_pbo.read_bytes()
archive = Archive(raw)
before_cfg = bytes(archive.stored(archive.by_key["config.cpp"]))
after_cfg = changed["lobo_pilot/config.cpp"]
added = {r["member"]: r["stored_name"] for r in records if r.get("added")}
members = {"lobo_pilot/" + key for key in archive.by_key} | set(added)
report = {"plan_changed": sorted(changed), "added": sorted(added), "skipped": skipped,
          "config_before": [len(before_cfg), sha(before_cfg)], "config_after": [len(after_cfg), sha(after_cfg)]}

# 2. parse both configs with the engine parser
cfg_before, cfg_after = tojson(before_cfg, "config-before"), tojson(after_cfg, "config-after")
before_leaves, after_leaves = dict(leaves(cfg_before)), dict(leaves(cfg_after))
diff = sorted({tuple(map(str, k[:3])) for k in set(before_leaves) | set(after_leaves)
               if before_leaves.get(k, object()) != after_leaves.get(k, object())})
report["parsed_difference_scopes"] = diff
report["parsed_leaf_differences"] = sum(1 for k in set(before_leaves) | set(after_leaves)
                                        if before_leaves.get(k, object()) != after_leaves.get(k, object()))
veh_after = find_key(cfg_after, "CfgVehicles")
sample = next(iter(v for v in veh_after.values() if isinstance(v, dict)))
report["tojson_class_keys_sample"] = sorted(sample)[:12]


def effective(cls_map, name, key):
    seen = set()
    while name and name not in seen and name in cls_map:
        seen.add(name)
        parent, props = cls_map[name]
        if key in props:
            return props[key], name
        name = parent
    return None, None


models = {}


def model_faces(model):
    key = norm(model)
    if not key.endswith(".p3d"):
        key += ".p3d"
    member = key.split(BS, 1)[1].replace(BS, "/")
    if member not in models:
        data = archive.read(member, pilot_pbo, TOOLS)
        models[member] = lods(data)
    return member, models[member]


results = {}
for state, cfg, src in (("before", cfg_before, before_cfg), ("after", cfg_after, after_cfg)):
    cls_map = classes(cfg, src.decode("latin1"))
    rows = {}
    for name in cls_map:
        model, _ = effective(cls_map, name, "model")
        if not isinstance(model, str) or not norm(model).startswith("lobo_pilot" + BS):
            continue
        wounds_arr, owner = effective(cls_map, name, "wounds")
        arr = list(wounds_arr or [])
        pairs = list(zip(arr[0::2], arr[1::2]))
        member, lod_list = model_faces(model)
        faces = collections.Counter()
        for res, names in lod_list:
            for n in names:
                faces[norm(n)] += 1
        # LOD 9.0 draws <side>\lod\{back,front,side,top}.paa: no _d art exists in any lod folder.
        body = sorted(n for n in faces if n.startswith("lobo_pilot" + BS) and BS + "lod" + BS not in n
                      and n.rsplit(BS, 1)[1].rsplit(".", 1)[0] in PARTS)
        healthy = {norm(h): w for h, w in pairs}
        unmatched = [n for n in body if n not in healthy]
        live = [(h, w) for h, w in pairs if norm(h) in faces]
        dead = [h for h, _ in pairs if norm(h).startswith("lobo_pilot" + BS) and norm(h) not in faces]
        missing_targets = [w for h, w in live if norm(w).startswith("lobo_pilot" + BS)
                           and "lobo_pilot/" + norm(w).split(BS, 1)[1].replace(BS, "/") not in members]
        lod_unpaired = sorted(n for n in faces if n.startswith("lobo_pilot" + BS) and n not in healthy
                              and n not in body)
        rows[name] = {"model": member, "wounds_from": owner, "pairs": len(pairs),
                      "body_faces": len(body), "body_unmatched": unmatched,
                      "live_body_pairs": sum(1 for h, _ in live if norm(h) in body),
                      "dead_lobo_pilot_pairs": dead, "missing_wounded_targets": missing_targets,
                      "wounded_body_targets": sorted({norm(w) for h, w in live if norm(h) in body}),
                      "other_unpaired_lobo_pilot_faces": lod_unpaired}
    results[state] = rows
report["classes"] = results

# 3. rebuild LoBo_Pilot.pbo in scratch and re-plan references + wounds on a scratch mod
scratch_mod = OUT / "mod"
(scratch_mod / "addons").mkdir(parents=True, exist_ok=True)
edits = {k.split("/", 1)[1]: v for k, v in changed.items() if k not in added}
additions = {added[k]: v for k, v in changed.items() if k in added}
rebuilt = Archive(raw).replace(edits, additions)
(scratch_mod / "addons" / pilot_pbo.name).write_bytes(rebuilt)
for stem in ("lobomig", "loboter", "lobo_airammo"):
    target = scratch_mod / "addons" / paths[stem].name
    if not target.exists() or sha(target.read_bytes()) != hashes[stem]:
        shutil.copyfile(paths[stem], target)
with patch.object(runner, "modules", return_value=[references, wounds]), \
        patch.object(wounds, "generate", side_effect=AssertionError("regenerated")):
    again = runner.plan(scratch_mod, TOOLS, "ud", ASSET)
report["rebuilt_pilot_pbo"] = [len(rebuilt), sha(rebuilt)]
report["replan_changed"] = sorted(again[2])
(OUT / "verify.json").write_text(json.dumps(report, indent=1) + "\n", encoding="utf8")

print("plan changed", len(changed), "added", len(added), "skipped", skipped)
print("config", report["config_before"], "->", report["config_after"])
print("parsed leaf differences", report["parsed_leaf_differences"], "scopes", diff)
print("rebuilt", report["rebuilt_pilot_pbo"], "replan changed", report["replan_changed"])
for name in results["after"]:
    b, a = results["before"].get(name), results["after"][name]
    print(f"{name:28} {a['model']:26} from {a['wounds_from']:22} body {a['body_faces']:2} "
          f"before unmatched {len(b['body_unmatched']):2} after unmatched {len(a['body_unmatched'])} "
          f"dead {len(a['dead_lobo_pilot_pairs'])} missing {len(a['missing_wounded_targets'])} "
          f"targets {','.join(sorted({t.split(BS)[-2] + '/' + t.split(BS)[-1] for t in a['wounded_body_targets']}))[:200]}")
ok = all(not r["body_unmatched"] and not r["missing_wounded_targets"] for r in results["after"].values())
ok = ok and not report["replan_changed"]
print("ALL PILOT CLASSES COVERED:", ok)
