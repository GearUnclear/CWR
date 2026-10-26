import sys
path=sys.argv[1]
ctx=int(sys.argv[2]) if len(sys.argv)>2 else 6
data=open(path,'rb').read().decode('utf-8')
lines=data.split('\n')
i=0;n=0
while i<len(lines):
    if lines[i].startswith('<<<<<<<'):
        n+=1
        print(f'######## conflict {n} at line {i+1} (marker ends CR: {lines[i].endswith(chr(13))})')
        for k in range(max(0,i-ctx),i): print('  ctx|'+lines[k].rstrip('\r'))
        j=i
        while not lines[j].startswith('>>>>>>>'): 
            print('     |'+lines[j].rstrip('\r')); j+=1
        print('     |'+lines[j].rstrip('\r'))
        for k in range(j+1,min(len(lines),j+1+ctx)): print('  ctx|'+lines[k].rstrip('\r'))
        i=j
    i+=1
