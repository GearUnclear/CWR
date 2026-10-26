import sys, pathlib
sys.path.insert(0, r"D:\Arma_CWA\.claude\worktrees\lobo-repair-3\arma_CWR\tools\lobo")
from repair.pbo import Archive
mod = pathlib.Path(r"D:\Arma_CWA\@LoBo\addons")
out = pathlib.Path(r"D:\Arma_CWA\tmp\lobo-repair-3\issue-2-frame-loops\extract-related")
tool = r"D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe"
for name in sys.argv[1:]:
    p = mod/name; a=Archive(p.read_bytes())
    for key,e in a.by_key.items():
        if key.endswith((".sqs",".sqf",".cpp")):
            dst=out/p.stem/key; dst.parent.mkdir(parents=True,exist_ok=True); dst.write_bytes(a.read(key,p,tool))
    print(name, "ok")
