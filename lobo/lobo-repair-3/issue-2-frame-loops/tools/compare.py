import sys, pathlib, hashlib
sys.path.insert(0, r"D:\Arma_CWA\.claude\worktrees\lobo-repair-3\arma_CWR\tools\lobo")
from repair.pbo import Archive
tool = r"D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe"
inst = pathlib.Path(r"D:\Arma_CWA\@LoBo\addons")
b1 = pathlib.Path(r"D:\Arma_CWA\@LoBo\.ud-backups\issue-69\20261011T105837Z-33d55270")
members = {"LoBoEghelo.pbo":["mk2/float.sqs","config.cpp"],"LoBo_FAB.pbo":["dolp/init.sqs","config.cpp"],"LoBoMig.pbo":["mig25/sqs/mig25gear.sqs","config.cpp"],
 "LoBoSyrAPC.pbo":["scripts/bmp11anim.sqs","config.cpp"],"LoboF5.pbo":["scripts/autocanopy.sqs","config.cpp"],"LoBo_A4.pbo":["scripts/canopy.sqs","config.cpp"]}
for pbo, keys in members.items():
    p=inst/pbo; a=Archive(p.read_bytes())
    for k in keys:
        e=a.by_key[k]; d=a.read(k,p,tool)
        line=f"{pbo}:{k} packing={e.fields[0]:#x} stored={e.fields[4]} len={len(d)} crlf={d.count(b'\r\n')} lf={d.count(b'\n')} trailing_newline={d.endswith(b'\n')} sha256={hashlib.sha256(d).hexdigest()[:16]}"
        bp=b1/pbo
        if bp.exists():
            ba=Archive(bp.read_bytes()); bd=ba.read(k,bp,tool); line+=f" same_as_pre69={bd==d}"
        else: line+=" (archive never repaired)"
        print(line)
