"""Small source-preserving helpers for legacy OFP config repairs.

Only class/property structure is parsed; macro evaluation is left to PoseidonTools.
Strings, doubled quotes, comments and array braces cannot create fake classes.
"""
from dataclasses import dataclass
import re

_TOKEN = re.compile(r'//[^\r\n]*|/\*[\s\S]*?\*/|"(?:""|[^"\r\n])*"|[A-Za-z_][\w]*|[^\s]', re.MULTILINE)

@dataclass(frozen=True)
class Token:
    text: str
    start: int
    end: int

def tokens(text):
    out=[]
    for m in _TOKEN.finditer(text):
        value=m.group()
        if value=='"' or (value=='/' and text[m.start():].startswith('/*')):
            raise ValueError('Unterminated config string or block comment')
        if not value.startswith(('//','/*')):out.append(Token(value,m.start(),m.end()))
    return out

def assignment_at(ts, i):
    """Return the value start for an identifier[optional []]= assignment."""
    if i>=len(ts) or not re.fullmatch(r'[A-Za-z_]\w*',ts[i].text):return None
    j=i+1
    if j+1<len(ts) and ts[j].text=='[' and ts[j+1].text==']':j+=2
    return j+1 if j<len(ts) and ts[j].text=='=' else None

def value_end(text, ts, start):
    """Legacy newline termination, without truncating continued expressions."""
    j=start;depth=0
    while j<len(ts):
        value=ts[j].text
        if depth==0:
            if value in (';', '}'):break
            if j>start and '\n' in text[ts[j-1].end:ts[j].start]:
                if value.lower()=='class' or assignment_at(ts,j) is not None:break
        if value=='{':depth+=1
        elif value=='}':depth-=1
        j+=1
    if depth:raise ValueError('Unbalanced config array braces')
    return j

def _matching(ts, at, opening='{', closing='}'):
    depth=0
    for i in range(at,len(ts)):
        if ts[i].text==opening: depth+=1
        elif ts[i].text==closing:
            depth-=1
            if depth==0:return i
    raise ValueError('Unbalanced config braces')

def class_spans(text):
    """Return direct named class spans (name, inner start, closing brace offset)."""
    ts=tokens(text); out=[]; i=0
    while i<len(ts):
        if ts[i].text.lower()=='class' and i+1<len(ts):
            name=ts[i+1].text; j=i+2
            while j<len(ts) and ts[j].text not in ('{',';'):j+=1
            if j<len(ts) and ts[j].text=='{':
                end=_matching(ts,j); out.append((name,ts[j].end,ts[end].start)); i=end+1; continue
            i=j+1;continue
        if ts[i].text=='{':i=_matching(ts,i)+1
        elif ts[i].text=='}':raise ValueError('Unmatched config closing brace')
        else:i+=1
    return out

def class_block(text, path):
    """Return (inner_start, closing_brace_offset) for one case-insensitive path."""
    start,end=0,len(text)
    for part in path:
        matches=[(a,b) for n,a,b in class_spans(text[start:end]) if n.lower()==part.lower()]
        if len(matches)!=1:raise ValueError(f'Expected exactly one class {"/".join(path)}; found {len(matches)} at {part}')
        a,b=matches[0];end=start+b;start+=a
    return start,end

def property_span(text,path,key):
    """Locate direct scalar value; accepts legacy newline-terminated assignments."""
    start,end=class_block(text,path);ts=tokens(text[start:end]); found=[];i=0
    while i<len(ts):
        if ts[i].text=='{':i=_matching(ts,i)+1;continue
        if ts[i].text.lower()==key.lower() and i+1<len(ts) and ts[i+1].text=='=':
            a=i+2;j=value_end(text[start:end],ts,a)
            if j==a:raise ValueError(f'Empty value {path}/{key}')
            found.append((start+ts[a].start,start+ts[j-1].end));i=j;continue
        i+=1
    if len(found)>1:raise ValueError(f'Duplicate property {path}/{key}')
    return found[0] if found else None

def scalar(text,path,key):
    span=property_span(text,path,key)
    return text[slice(*span)] if span else None

def set_scalar(text,path,key,value):
    value=str(value);span=property_span(text,path,key)
    if span:return text[:span[0]]+value+text[span[1]:]
    _,end=class_block(text,path);nl='\r\n' if '\r\n' in text else '\n'
    return text[:end]+f'{nl}\t\t{key}={value};{nl}\t'+text[end:]

def replace_exact(text,old,new,expected=1):
    """Idempotent substitution, rejecting unexpected occurrences or mixed states."""
    if old==new:return text
    count=text.count(old)
    if count==0:
        if text.count(new)!=expected:raise ValueError(f'Expected {expected} repaired occurrence(s): {new[:80]}')
        return text
    if count!=expected or new in text:raise ValueError(f'Unexpected source occurrences: {old[:80]}')
    return text.replace(old,new)
