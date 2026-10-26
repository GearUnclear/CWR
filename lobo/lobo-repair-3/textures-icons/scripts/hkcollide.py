"""Non-base-game class collisions between a candidate addon config and LoBo configs.
Usage: hkcollide.py <scriptsdir> base-config.cpp candidate.cpp cfgdir
"""
import glob, os, sys
sys.path.insert(0, sys.argv[1])
src = open(os.path.join(sys.argv[1], 'classes.py')).read().split('\n\nnew = defs')[0]
exec(src)
base = defs(sys.argv[2])
new = defs(sys.argv[3])
others = {os.path.basename(os.path.dirname(p)): defs(p) for p in glob.glob(os.path.join(sys.argv[4], '*', 'config.cpp'))}
print('base classes', len(base), 'candidate classes', len(new))
nonbase = [k for k in new if k not in base]
print('candidate non-base classes', len(nonbase))
for key in sorted(nonbase):
    hits = [name for name, o in others.items() if key in o]
    if hits:
        print('COLLISION', key, sorted(new[key]), {h: sorted(others[h][key]) for h in hits})
