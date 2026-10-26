import sys, pathlib, hashlib, json
sys.path.insert(0, r"D:\Arma_CWA\.claude\worktrees\lobo-repair-3\arma_CWR\tools\lobo")
from repair.pbo import Archive
mod = pathlib.Path(r"D:\Arma_CWA\@LoBo\addons")
out = pathlib.Path(r"D:\Arma_CWA\tmp\lobo-repair-3\issue-2-frame-loops\extract")
tool = r"D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe"
targets = {"LoBoEghelo.pbo":None,"LoBo_FAB.pbo":None,"LoBoMig.pbo":None,"LoBoSyrAPC.pbo":None,"LoboF5.pbo":None,"LoBo_A4.pbo":None}
summary={}
for name in targets:
    p = mod/name; raw=p.read_bytes(); a=Archive(raw)
    summary[name]={"sha256":hashlib.sha256(raw).hexdigest(),"checksummed":a.checksummed,"stuffpbo":a.stuffpbo,"trailer_len":len(a.trailer),"members":{}}
    for key,e in a.by_key.items():
        if key=="config.cpp" or key.endswith((".sqs",".sqf",".cpp",".hpp",".ext")):
            data=a.read(key,p,tool)
            dst=out/p.stem/key; dst.parent.mkdir(parents=True,exist_ok=True); dst.write_bytes(data)
            summary[name]["members"][key]={"packing":e.fields[0],"len":len(data),"sha256":hashlib.sha256(data).hexdigest()}
(out/"summary.json").write_text(json.dumps(summary,indent=1))
for n,s in summary.items(): print(n,s["sha256"][:16],s["checksummed"],s["stuffpbo"],s["trailer_len"],len(s["members"]))
