def edit(p, old, new, cnt=1):
    s=open(p,encoding='utf-8',newline='').read()
    n=s.count(old)
    assert n==cnt, (p, n, old[:80])
    s=s.replace(old,new)
    open(p,'w',encoding='utf-8',newline='').write(s)
