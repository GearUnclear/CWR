from pathlib import Path
import json, re
p=Path(r"D:\Arma_CWA\tmp\narrative-rewrite")
for i in range(1,37):
    f=p/f"b{i:02}.json"
    if not f.exists(): continue
    d=json.loads(f.read_text())
    t=d["text"]
    issues=[]
    if not 20<=len(t.split())<=36: issues.append("word count")
    if t.count("{Name}")!=1 or t.count("{Place}")!=1: issues.append("placeholders")
    if re.search(r"\b(he|she|his|her|him|hers)\b",t,re.I): issues.append("gendered pronoun")
    if not t.isascii(): issues.append("non-ASCII")
    if re.search(r"[0-9\n\r\t]",t): issues.append("digit/control")
    if not re.search(r'[.!?]"?$',t): issues.append("punctuation")
    print(f'{d["id"]} ({len(t.split())})'+(" ISSUES="+str(issues) if issues else "")+": "+t)

