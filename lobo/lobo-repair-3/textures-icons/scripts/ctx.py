"""Print enclosing class name, scope and the line for given config line numbers.

Usage: ctx.py config.cpp line [line...]
"""
import re, sys
lines = open(sys.argv[1], encoding='latin-1').read().split('\n')
for n in map(int, sys.argv[2:]):
    depth = 0
    cls = None
    for i in range(n - 1, -1, -1):
        l = re.sub(r'//.*', '', lines[i])
        depth += l.count('}') - l.count('{')
        m = re.match(r'\s*class\s+(\w+)\s*(?::\s*(\w+))?', l)
        if m and depth < 0:
            cls = (i + 1, m.group(1), m.group(2))
            break
    print(f'{n}: {lines[n-1].strip()}    [class {cls[1]}:{cls[2]} at {cls[0]}]' if cls else f'{n}: {lines[n-1].strip()}')
