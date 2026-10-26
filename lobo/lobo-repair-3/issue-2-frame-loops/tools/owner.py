import sys, re, pathlib
base = pathlib.Path(r"D:\Arma_CWA\tmp\lobo-repair-3\issue-2-frame-loops\extract")
targets = {"LoBoEghelo":[1305,1354],"LoBo_FAB":[1907],"LoBoMig":[1082,1130],"LoBoSyrAPC":[165,257],"LoboF5":[223,255,286],"LoBo_A4":[234,261,288]}
for addon, lines in targets.items():
    text = (base/addon/"config.cpp").read_bytes().decode("latin1").splitlines()
    stack=[]
    owners={}
    for i,l in enumerate(text,1):
        code=re.sub(r'"[^"]*"','',l.split("//")[0])
        m=re.match(r'\s*class\s+(\w+)',code)
        pending = m.group(1) if m and '{' in code and '}' not in code.split('{',1)[1] else None
        if m and '{' not in code: pending_name=m.group(1)
        for ch in code:
            if ch=='{':
                name = m.group(1) if m else (locals().get('pending_name') or '?')
                stack.append(name); m=None; pending_name=None
            elif ch=='}':
                if stack: stack.pop()
        if i in lines: owners[i]=list(stack)
    for ln in lines: print(addon, ln, " > ".join(owners.get(ln,[])))
