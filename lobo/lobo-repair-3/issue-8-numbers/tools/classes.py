"""Read-only: list CfgVehicles classes whose (own) EventHandlers init mention a
target script, with own scope/model/hiddenSelections, plus inheritance chain."""
import re
import sys
from pathlib import Path

cfg = Path(sys.argv[1])
needle = sys.argv[2].lower()
text = cfg.read_bytes().decode("latin1").replace("\r\n", "\n")
text_nc = re.sub(r"//[^\n]*", "", text)

# crude tokenizer over class blocks
pos = 0
stack = []  # (name, parent, start)
classes = {}
for m in re.finditer(r'"(?:[^"]|"")*"|\bclass\s+(\w+)\s*(?::\s*(\w+))?\s*(\{|;)|\{|\}', text_nc):
    tok = m.group(0)
    if tok.startswith('"'):
        continue
    if tok.startswith("class"):
        if m.group(3) == "{":
            stack.append([m.group(1), m.group(2), m.end()])
        else:
            classes.setdefault(m.group(1), {"parent": m.group(2), "body": "", "path": "/".join(s[0] for s in stack)})
        continue
    if tok == "{":
        stack.append([None, None, m.end()])
    elif tok == "}":
        name, parent, start = stack.pop()
        if name:
            path = "/".join(s[0] or "?" for s in stack)
            classes[name] = {"parent": parent, "body": text_nc[start:m.start()], "path": path}

def own(body, key):
    # strip nested class bodies roughly
    flat = re.sub(r"class\s+\w+[^{;]*\{(?:[^{}]|\{[^{}]*\})*\}", "", body)
    m = re.search(rf"\b{key}\s*=\s*([^;]+);", flat, re.I)
    return m.group(1).strip() if m else None

def chain(name):
    out = []
    while name in classes:
        out.append(name)
        name = classes[name]["parent"]
    out.append(name)
    return out

for name, c in classes.items():
    if not c["path"].lower().endswith("cfgvehicles"):
        continue
    eh = re.search(r"class\s+EventHandlers[^{]*\{(.*?)\}", c["body"], re.S | re.I)
    if eh and needle in eh.group(1).lower():
        sc = None
        for n in chain(name):
            if n in classes and own(classes[n]["body"], "scope") is not None:
                sc = (n, own(classes[n]["body"], "scope"))
                break
        mdl = None
        for n in chain(name):
            if n in classes and own(classes[n]["body"], "model") is not None:
                mdl = (n, own(classes[n]["body"], "model"))
                break
        hs = None
        for n in chain(name):
            if n in classes and re.search(r"hiddenSelections\[\]\s*=\s*\{[^}]*\}", classes[n]["body"], re.I):
                hs = (n, re.search(r"hiddenSelections\[\]\s*=\s*(\{[^}]*\})", classes[n]["body"], re.I).group(1))
                break
        print(f"{name}: scope={sc} model={mdl} hiddenSelections={hs}")
