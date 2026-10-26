"""Read-only end-to-end dry run of the proposed numbers module against the
installed @LoBo, mirroring repair_lobo.plan(): loads text members, runs the
existing scripts rules and the new rule, checks fixed point, and stages the
archive replacements IN MEMORY ONLY (nothing is written to @LoBo)."""
import hashlib
import sys
from pathlib import Path

sys.path.insert(0, r"D:/Arma_CWA/tmp/lobo-repair-3/issue-8-numbers/pkg")
from repair import numbers, scripts  # noqa: E402
from repair.pbo import Archive  # noqa: E402

TOOLS = Path(r"D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe")
MOD = Path(r"D:/Arma_CWA/@LoBo")
CANDIDATE = Path(r"D:/Arma_CWA/tmp/lobo-repair-3/issue-8-numbers/candidate")
h = lambda b: hashlib.sha256(b).hexdigest()

paths = {p.stem.lower(): p for p in (MOD / "addons").glob("*.pbo")}
wanted = set(scripts.ADDONS) | set(numbers.ADDONS)
docs, original, archives = {}, {}, {}
for stem in sorted(wanted):
    raw = paths[stem].read_bytes()
    arc = Archive(raw)
    archives[stem] = (paths[stem], raw, arc)
    for key in arc.by_key:
        if key == "config.cpp" or key.endswith((".sqs", ".sqf")):
            data = arc.read(key, paths[stem], TOOLS)
            docs[stem + "/" + key] = data.decode("latin1")
            original[stem + "/" + key] = data

print("scripts rule on installed (expect {}):", list(scripts.repair(dict(docs))))
edits = numbers.repair(dict(docs))
print("numbers rule changes:", sorted(edits))
docs2 = docs | edits
print("numbers fixed point (expect {}):", numbers.repair(dict(docs2)))
print("scripts after numbers (expect {}):", list(scripts.repair(dict(docs2))))

for full, text in sorted(edits.items()):
    data = text.encode("latin1")
    stem, key = full.split("/", 1)
    cand = None
    for f in CANDIDATE.rglob("*.sqs"):
        if f.relative_to(CANDIDATE).as_posix().lower() == full:
            cand = f.read_bytes()
    print(f"  {full}: {len(original[full])} -> {len(data)} bytes, {h(original[full])[:16]} -> {h(data)[:16]}, matches prototype={cand == data}")

grouped = {}
for full, text in edits.items():
    stem, key = full.split("/", 1)
    grouped.setdefault(stem, {})[key] = text.encode("latin1")
for stem, repl in sorted(grouped.items()):
    path, raw, arc = archives[stem]
    result = Archive(raw).replace(repl)  # validates untouched members internally
    after = Archive(result)
    ok = all(after.read(k) == v for k, v in repl.items())
    print(f"  staged {path.name}: {len(raw)} -> {len(result)} bytes, members={len(arc.entries)}, replaced={sorted(repl)}, readback={ok}, after_sha256={h(result)}")
