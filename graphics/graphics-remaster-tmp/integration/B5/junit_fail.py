import re, sys
x = open(sys.argv[1], encoding="utf-8", errors="replace").read()
for m in re.finditer(r'<testcase ([^>]*?)(/>|>(.*?)</testcase>)', x, re.S):
    attrs, body = m.group(1), m.group(3) or ""
    name = re.search(r'name="([^"]*)"', attrs).group(1)
    if "<failure" in body or "<error" in body:
        print("FAIL:", name)
