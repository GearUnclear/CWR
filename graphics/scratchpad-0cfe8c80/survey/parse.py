import re,sys,collections
def load(w):
    rows=[]
    for l in open(f'{w}.objs.txt',encoding='latin-1').read().split('\n')[2:]:
        m=re.match(r'^(\d+)\s+(.*?\.p3d)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s*$',l)
        if m: rows.append((m.group(2).lower(),float(m.group(3)),float(m.group(4)),float(m.group(5))))
    return rows
if __name__=='__main__':
    w=sys.argv[1]; rows=load(w)
    c=collections.Counter(r[0] for r in rows)
    pat=sys.argv[2] if len(sys.argv)>2 else None
    for k,v in c.most_common():
        if pat is None or re.search(pat,k): print(v,k)
