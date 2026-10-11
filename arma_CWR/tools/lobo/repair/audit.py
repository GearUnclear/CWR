"""Conservative offline diagnostics: structural facts, never gameplay claims."""
from collections import Counter
import re
from .config import assignment_at, class_spans, tokens, value_end

SECTIONS={'cfgammo','cfgweapons','cfgvehicles'}
REFERENCE_KEYS={'ammo':'cfgammo','weapons':'cfgweapons','magazines':'cfgweapons','crew':'cfgvehicles'}
STANDARD_HITS={'hitengine','hithull','hitturret','hitgun','hitltrack','hitrtrack',
               'hitbody','hithead','hithands','hitlegs','hitfuel','hitrglass','hitlglass'}

def assignments(text):
    ts=tokens(text);i=0
    while i<len(ts)-1:
        key=ts[i].text.lower();start=assignment_at(ts,i)
        if start is None:i+=1;continue
        j=value_end(text,ts,start)
        yield key,[t.text.strip('"') for t in ts[start:j] if t.text not in ('{','}',',')]
        i=max(i+1,j)

def config_references(documents):
    known={s:set() for s in SECTIONS};refs=[]
    for file,text in documents.items():
        if not file.endswith('/config.cpp'):continue
        for section,a,b in class_spans(text):
            if section.lower() not in SECTIONS:continue
            body=text[a:b]
            for name,c,d in class_spans(body):
                known[section.lower()].add(name.lower())
                for key,values in assignments(body[c:d]):
                    if key not in REFERENCE_KEYS:continue
                    for value in values:
                        if value.lower().startswith('lobo'):
                            refs.append((file,section,name,key,value))
    return [{'kind':'unresolved_lobo_class','file':f,'class':s+'/'+n,'property':k,'reference':v}
            for f,s,n,k,v in refs if v.lower() not in known[REFERENCE_KEYS[k]]]

def script_candidates(file,text):
    # SQS semicolon comments and case-insensitive labels. Unreachable code and
    # dynamically supplied locals cannot be proven by this heuristic.
    lines=[(i,l) for i,l in enumerate(text.splitlines(),1) if not l.lstrip().startswith(';')]
    labels=[(i,m.group(1).lower()) for i,l in lines if (m:=re.match(r'\s*#\s*(\S+)',l))]
    counts=Counter(n for i,n in labels);out=[]
    for i,n in labels:
        if counts[n]>1:out.append({'kind':'duplicate_label','file':file,'line':i,'label':n})
    for i,l in lines:
        for label in re.findall(r'\bgoto\s+"([^"\n]+)"',l,re.I):
            if label.lower() not in counts:
                out.append({'kind':'missing_label','file':file,'line':i,'label':label,
                            'terminal_spelling':label.lower() in ('exit','end','ende','done')})
    code='\n'.join(l for _,l in lines)
    declared={m.group(1).lower() for m in re.finditer(r'\b(_\w+)\s*=(?!=)',code)}|{'_this','_x','_exception'}
    # Focus on callable locals: missing function initialization is the defect
    # family from issue #69. General variable dataflow needs a real interpreter.
    for i,l in lines:
        for name in re.findall(r'\bcall\s+(_\w+)\b',l,re.I):
            if name.lower() not in declared:out.append({'kind':'uninitialized_local_call_candidate','file':file,'line':i,'variable':name})
    return out

def hitpoint_candidates(documents):
    out=[]
    for file,text in documents.items():
        if not file.endswith('/config.cpp'):continue
        for section,a,b in class_spans(text):
            if section.lower()!='cfgvehicles':continue
            body=text[a:b]
            for name,c,d in class_spans(body):
                for child,_,_ in class_spans(body[c:d]):
                    if child.lower().startswith('hit') and child.lower() not in STANDARD_HITS:
                        out.append({'kind':'custom_hitpoint_candidate','file':file,'class':name,'hitpoint':child})
    return out

def audit(documents):
    parsed={};unparsed=[];candidates=[]
    for file,text in documents.items():
        if not file.endswith('/config.cpp'):continue
        try:class_spans(text)
        except ValueError as exc:
            unparsed.append(file)
            candidates.append({'kind':'unsupported_parse','file':file,'reason':str(exc)})
        else:parsed[file]=text
    errors=config_references(parsed)
    if unparsed:
        # Omitted source can contain definitions for apparent missing references.
        for finding in errors:finding['uncertain_due_to_unparsed_configs']=True
        candidates.extend(errors);errors=[]
    candidates.extend(hitpoint_candidates(parsed))
    for file,text in documents.items():
        if file.endswith('.sqs'):candidates.extend(script_candidates(file,text))
    return {'errors':errors,'candidates':candidates,'unparsed_configs':unparsed,
            'coverage_complete':not unparsed,'note':
            'Unresolved LoBo references are strict errors only with complete config coverage. Unparsed configs make reference findings uncertain. Script/dataflow/custom-hitpoint candidates require reachability and engine review; they are not confirmed gameplay defects.'}
