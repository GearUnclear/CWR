#!/usr/bin/env python3
"""Offline Lobo reference/label/hitpoint diagnostics; never starts the game."""
import argparse
import json
from pathlib import Path
from repair.pbo import Archive
from repair.audit import audit

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--mod',type=Path,required=True)
    p.add_argument('--tools',type=Path,default=Path(__file__).resolve().parents[2]/'dist/x64-win-rwdi/PoseidonTools.exe')
    p.add_argument('--report',type=Path,required=True);p.add_argument('--strict',action='store_true')
    args=p.parse_args();documents={};scanned=0
    for archive_path in sorted((args.mod/'addons').glob('*.pbo')):
        a=Archive(archive_path.read_bytes());stem=archive_path.stem.lower();scanned+=1
        for key in a.by_key:
            if key=='config.cpp' or (stem.startswith('lobo') and key.endswith('.sqs')):
                documents[stem+'/'+key]=a.read(key,archive_path,args.tools.resolve()).decode('latin1')
    result=audit(documents);result['archives_scanned']=scanned;result['text_members']=len(documents)
    args.report.parent.mkdir(parents=True,exist_ok=True);args.report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps({'archives_scanned':scanned,'unresolved_lobo_references':len(result['errors']),
                      'coverage_complete':result['coverage_complete'],'unparsed_configs':result['unparsed_configs'],
                      'review_candidates':len(result['candidates']),'report':str(args.report)},indent=2))
    return 1 if args.strict and (result['errors'] or not result['coverage_complete']) else 0

if __name__=='__main__':raise SystemExit(main())
