import re, sys, pathlib, collections
root = pathlib.Path(sys.argv[1])
BS = "\\"
for p in sorted(root.rglob('*number*.sqs')):
    raw = p.read_bytes()
    t = raw.decode('latin1')
    n = t.replace('\r\n', '\n')
    paths = re.findall(r'"(\\[^"]+?\.paa)"', n)
    prefixes = collections.Counter(re.sub(r'\d+\.paa$', '#.paa', x) for x in paths)
    shape = re.sub(r'"\\[^"]+?\.paa"', '"P"', n)
    print('==', p.relative_to(root))
    print('   prefixes', dict(prefixes))
    print('   shape sha', __import__('hashlib').sha256(shape.encode()).hexdigest()[:12])
