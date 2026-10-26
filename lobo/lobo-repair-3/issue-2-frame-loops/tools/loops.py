import sys, pathlib, re
root = pathlib.Path(sys.argv[1])
for f in sorted(root.rglob("*.sqs")):
    lines = f.read_bytes().decode("latin1").splitlines()
    labels = {}
    for i,l in enumerate(lines):
        s=l.strip()
        if s.startswith("#"): labels[s[1:].strip().lower()] = i
    for i,l in enumerate(lines):
        s=l.strip()
        if s.startswith(";"): continue
        for m in re.finditer(r'goto\s*"([^"]+)"', s, re.I):
            lab=m.group(1).lower()
            if lab in labels and labels[lab] <= i:
                body=[x.strip() for x in lines[labels[lab]:i+1]]
                waits=[x for x in body if x.startswith(("~","@","&"))]
                if not waits:
                    print(f"{f.relative_to(root)}:{i+1}: back-edge to #{lab} (line {labels[lab]+1}) with NO wait in span ({i-labels[lab]+1} lines)")
