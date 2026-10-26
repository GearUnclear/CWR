from pathlib import Path
from docx import Document
from docx.shared import Pt
import shutil, re
root = Path('D:/Arma_CWA')
work = root / 'tmp/lobo-ownership-null-x'
notice = 'LOBO ownership is under the new creator, Null_X.'
paths = [
'@LoBo/README.txt', '@LoBo/addons/BTR-60Readme.txt', '@LoBo/addons/LoBoachzarit_readme.txt',
'@LoBo/docs/IranIraqDemoMission_readme.txt', '@LoBo/docs/LoBoIranVer1_0_README.txt',
'@LoBo/docs/LoBoTerReadme.txt', '@LoBo/docs/SinaiVer1_4_README.txt',
'@LoBo/docs/lebanon/LebanonVer1_3_README.txt', '@LoBo/docs/lebanon/SinaiVer1_5_README.txt',
'@LoBo/docs/archival/jordanian_army_2002_readme.html',
'@LoBo/tools/lobo-modfolder-assembly.txt', 'arma_CWR/tools/lobo/lobo-modfolder-assembly.txt']
for rel in paths:
    p = root / rel
    data = p.read_bytes()
    if notice.encode() in data: continue
    backup = work / 'backups' / rel
    backup.parent.mkdir(parents=True, exist_ok=True)
    if not backup.exists(): shutil.copy2(p, backup)
    nl = b'\r\n' if b'\r\n' in data else b'\n'
    if p.suffix == '.html':
        updated, n = re.subn(rb'(<body\b[^>]*>)', lambda m: m[0] + nl + b'<p><b>' + notice.encode() + b'</b></p>' + nl, data, count=1, flags=re.I)
        assert n == 1
    else:
        bom = b'\xef\xbb\xbf' if data.startswith(b'\xef\xbb\xbf') else b''
        updated = bom + notice.encode() + nl + nl + data[len(bom):]
    was_readonly = not (p.stat().st_mode & 128)
    if was_readonly: p.chmod(p.stat().st_mode | 128)
    p.write_bytes(updated)
    if was_readonly: p.chmod(p.stat().st_mode & ~128)
    print('Updated ' + rel)
for p in (work / 'docx').glob('*.docx'):
    doc = Document(p)
    para = next(x for x in doc.paragraphs[1:] if not x.text.strip())
    para.text = notice
    for run in para.runs:
        run.font.size = Pt(11)
        run.bold = True
    doc.save(work / 'edited' / p.name)
    print('Prepared ' + p.name)


