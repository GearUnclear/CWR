import pathlib,re,collections,json
root=pathlib.Path('tmp/lobo-audit-20261011')
issues=[]
for p in root.rglob('*.sqs'):
 text=p.read_text(errors='replace')
 labels={m.group(1).lower() for m in re.finditer(r'^\s*#([^\s;]+)',text,re.M)}
 for i,l in enumerate(text.splitlines(),1):
  if l.lstrip().startswith(';'): continue
  for target in re.findall(r'\bgoto\s+"([^"\n]+)"',l,re.I):
   if target.lower() not in labels: issues.append(f'{p}:{i}: missing label {target}: {l.strip()}')
print('\n'.join(issues))
(root/'missing-labels.txt').write_text('\n'.join(issues))
print('Scripts scanned:',len(list(root.rglob('*.sqs'))),'missing jumps:',len(issues))
