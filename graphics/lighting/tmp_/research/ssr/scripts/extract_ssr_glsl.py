import sys, re
src = open(sys.argv[1], encoding='utf-8').read()
for name, var in (("psPostSsr", "s_psPostSsrGLSL"), ("psPostSsrTag", "s_psPostSsrTagGLSL")):
    m = re.search(r'static const char ' + var + r'\[\] = R"\((.*?)\)";', src, re.S)
    open(f"{sys.argv[2]}/{name}.glsl", "w", encoding='utf-8', newline='\n').write(m.group(1))
    print(name, len(m.group(1)))
