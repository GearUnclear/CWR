import sys, pathlib, re, json, hashlib
sys.path.insert(0, r"D:\Arma_CWA\.claude\worktrees\lobo-repair-3\arma_CWR\tools\lobo")
from repair.scripts import _replace, script_findings, _explicit_exit
base = pathlib.Path(r"D:\Arma_CWA\tmp\lobo-repair-3\issue-2-frame-loops\extract")
out = pathlib.Path(r"D:\Arma_CWA\tmp\lobo-repair-3\issue-2-frame-loops\proposed")
FILES = {
 "loboeghelo/mk2/float.sqs": ("LoBoEghelo/mk2/float.sqs", (
   ('#loop\n_floater setpos [getpos _Hfloat select 0, getpos _Hfloat select 1, 1]\ngoto "loop"',
    '#loop\n?(not alive _Hfloat): deleteVehicle _floater; exit\n_floater setpos [getpos _Hfloat select 0, getpos _Hfloat select 1, 1]\n~0.1\ngoto "loop"'),)),
 "lobo_fab/dolp/init.sqs": ("LoBo_FAB/dolp/init.sqs", (
   ('#loop\n?(Behaviour _sub == "STEALTH")', '#loop\n?(not alive _sub): goto "exit"\n?(Behaviour _sub == "STEALTH")'),
   ('?(Behaviour _sub == "SAFE"): _sub animate ["arm1", 0]\n\ngoto "loop"', '?(Behaviour _sub == "SAFE"): _sub animate ["arm1", 0]\n~1\ngoto "loop"'),)),
 "lobomig/mig25/sqs/mig25gear.sqs": ("LoBoMig/mig25/sqs/mig25gear.sqs", (
   ('#CheckPosLoop\n\n;********** check pilot status', '#CheckPosLoop\n~0.5\n\n;********** check pilot status'),
   ('\ngoto "CheckPosition"\n', '\n;goto "CheckPosition"\n'),)),
 "lobosyrapc/scripts/bmp11anim.sqs": ("LoBoSyrAPC/scripts/bmp11anim.sqs", (
   ('?(not alive _unit ): goto "exit"\n\ngoto "GO"\t', '?(not alive _unit ): goto "exit"\n~0.5\ngoto "GO"\t'),)),
 "lobof5/scripts/autocanopy.sqs": ("LoboF5/scripts/autocanopy.sqs", (
   ('goto "CanopyDown"\ngoto "PollLoop"', 'goto "CanopyDown"\n~0.5\ngoto "PollLoop"'),)),
 "lobo_a4/scripts/canopy.sqs": ("LoBo_A4/scripts/canopy.sqs", (
   ('#Mainloop\n_pos = GetPos _plane1;', '#Mainloop\n?(not alive _plane1): exit\n_pos = GetPos _plane1;'),
   ('~0.0001\n', '~0.5\n'),)),
}
def loops(text):
    lines=text.split("\n"); labels={}
    for i,l in enumerate(lines):
        s=l.strip()
        if s.startswith("#"): labels[s[1:].strip().lower()]=i
    bad=[]
    for i,l in enumerate(lines):
        s=l.strip()
        if s.startswith(";"): continue
        for m in re.finditer(r'goto\s*"([^"]+)"',s,re.I):
            lab=m.group(1).lower()
            if lab in labels and labels[lab]<=i:
                if not any(x.strip().startswith(("~","@","&")) for x in lines[labels[lab]:i+1]): bad.append((i+1,lab))
    return bad
report={}
for key,(rel,rules) in FILES.items():
    raw=(base/rel).read_bytes().decode("latin1")
    nl="\r\n" if "\r\n" in raw else "\n"
    src=raw.replace("\r\n","\n")
    upd=src
    for b,a in rules: upd=_replace(upd,b,a)
    if key.startswith('lobosyrapc'): upd=_explicit_exit(upd)
    again=upd
    for b,a in rules: again=_replace(again,b,a)
    if key.startswith('lobosyrapc'): again=_explicit_exit(again)
    final=upd.replace("\n",nl)
    dst=out/rel; dst.parent.mkdir(parents=True,exist_ok=True); dst.write_bytes(final.encode("latin1"))
    report[key]={"before_findings":script_findings(src),"after_findings":script_findings(upd),
      "before_waitless_backedges":loops(src),"after_waitless_backedges":loops(upd),"idempotent":again==upd,
      "before_bytes":len(raw),"after_bytes":len(final),"after_sha256":hashlib.sha256(final.encode("latin1")).hexdigest()}
print(json.dumps(report,indent=1))
(out/"report.json").write_text(json.dumps(report,indent=1))
