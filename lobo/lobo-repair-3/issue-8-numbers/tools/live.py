"""Read-only: for each config.cpp init line that execs a number script, print the
exact exec fragment, the owning class (nearest preceding 'class X'), and scope."""
import re
import sys
from pathlib import Path

root = Path(sys.argv[1])
for cfg in sorted(root.glob("*/config.cpp")):
    text = cfg.read_bytes().decode("latin1").replace("\r\n", "\n")
    lines = text.split("\n")
    for i, line in enumerate(lines):
        for m in re.finditer(r'[^;"]*exec\s*""[^"]*(number2?\.sqs)""', line, re.I):
            # owning class: walk back counting braces
            depth = 0
            owner = None
            for j in range(i, -1, -1):
                l = lines[j]
                if j != i:
                    depth += l.count("}") - l.count("{")
                cm = re.match(r"\s*class\s+(\w+)\s*(?::\s*(\w+))?", l)
                if cm and depth < 0:
                    # first enclosing class above EventHandlers
                    if cm.group(1).lower() == "eventhandlers":
                        depth = 0
                        continue
                    owner = (cm.group(1), cm.group(2), j + 1)
                    break
            print(f"{cfg.parent.name}/config.cpp:{i+1}: [{m.group(0).strip()}] owner={owner}")
