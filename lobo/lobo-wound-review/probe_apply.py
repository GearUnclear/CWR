"""Scratch-copy apply/re-plan/restore of the wounds module only (never touches @LoBo)."""
import hashlib
import shutil
import sys
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, r"D:\Arma_CWA\.claude\worktrees\lobo-repair-4\arma_CWR\tools\lobo")
import repair_lobo as runner  # noqa: E402
from repair import wounds  # noqa: E402
from repair.pbo import Archive  # noqa: E402

SRC = Path(r"D:\Arma_CWA\@LoBo\addons\LoBo_Pilot.pbo")
MOD = Path(r"D:\Arma_CWA\tmp\lobo-wound-review\mod")
TOOL = Path(r"D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe")
if MOD.exists():
    shutil.rmtree(MOD)
(MOD / "addons").mkdir(parents=True)
target = MOD / "addons" / "LoBo_Pilot.pbo"
shutil.copyfile(SRC, target)
orig = target.read_bytes()
sha = lambda b: hashlib.sha256(b).hexdigest()  # noqa: E731

with patch.object(runner, "modules", return_value=[wounds]):
    paths, hashes, changed, records, docs, skipped = runner.plan(MOD, TOOL)
    print("changed", len(changed), "added", sum(1 for r in records if r.get("added")), "skipped", skipped)
    manifest = runner.apply(MOD, paths, hashes, changed, records)
    print("manifest", manifest)
    new = target.read_bytes()
    a, b = Archive(orig), Archive(new)
    keys_a, keys_b = list(a.by_key), list(b.by_key)
    print("prefix kept", keys_b[:len(keys_a)] == keys_a, "appended", keys_b[len(keys_a):] == list(wounds.ADDED_MEMBERS))
    diff = [k for k in keys_a if bytes(a.stored(a.by_key[k])) != bytes(b.stored(b.by_key[k])) or a.by_key[k].fields != b.by_key[k].fields]
    print("members differing", diff, "trailer", b.trailer, "size", len(orig), "->", len(new))
    for k in wounds.ADDED_MEMBERS:
        e = b.by_key[k.split("/", 1)[1]]
        assert sha(bytes(b.stored(e))) == wounds.OUTPUTS[k][1], k
        assert e.name.decode("latin1") == wounds.OUTPUTS[k][0], (e.name, k)
    print("added bytes/names pinned OK")
    with patch.object(wounds, "generate", side_effect=AssertionError("regenerated")):
        again = runner.plan(MOD, TOOL)
    print("second plan changes", len(again[2]))
    backup = manifest.parent / "LoBo_Pilot.pbo"
    print("backup == original", backup.read_bytes() == orig)
    shutil.copyfile(backup, target)
    restored = Archive(target.read_bytes())
    print("after restore: _dr members", [k for k in restored.by_key if k.endswith("_dr.paa")],
          "sha equal", sha(target.read_bytes()) == sha(orig))
