"""gputab.py RUN_DIR [...]: per shot, the GPU frame and its sections from the UDCAP shot lines."""
import re, sys, glob, os
rows = []
for run in sys.argv[1:]:
    for log in glob.glob(os.path.join(run, 'tri', '*', 'game_stdout.log')):
        for line in open(log, encoding='utf-8', errors='replace'):
            if 'event=shot' not in line:
                continue
            lab = re.search(r'label=([^;]+)', line).group(1)
            g = re.search(r'gpu=frame ([0-9.]+) ms \| world ([0-9.]+)(?:.*?\| post ([0-9.]+): ([^|]*))?', line)
            fps = re.search(r';fps=([0-9.]+)', line)
            if not g:
                continue
            secs = dict(re.findall(r'([a-z]+) ([0-9.]+)', g.group(4) or ''))
            rows.append((lab, float(g.group(1)), float(g.group(2)), float(g.group(3) or 0), secs))
for lab, fr, wo, po, secs in rows:
    s = ' '.join(f'{k}={v}' for k, v in secs.items())
    print(f'{lab:40s} frame {fr:6.2f} world {wo:6.2f} post {po:5.2f} | {s}')
