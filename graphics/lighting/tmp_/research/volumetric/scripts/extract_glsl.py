"""Writes the prototype's post shaders out of EngineGL33_PostFX.cpp as override-ready .glsl files."""
import os
import re
import sys

src, outdir = sys.argv[1], sys.argv[2]
text = open(src, encoding='utf-8').read()
names = {
    's_psPostVolumetricGLSL': 'psPostVolumetric',
    's_psPostVolBlurGLSL': 'psPostVolBlur',
    's_psPostVolTemporalGLSL': 'psPostVolTemporal',
    's_psPostCompositeGLSL': 'psPostComposite',
}
os.makedirs(outdir, exist_ok=True)
for var, name in names.items():
    m = re.search(r'static const char %s\[\] = R"\((.*?)\)";' % var, text, re.S)
    if not m:
        print('missing', var)
        continue
    with open(os.path.join(outdir, name + '.glsl'), 'w', encoding='utf-8', newline='\n') as f:
        f.write(m.group(1))
    print('wrote', name)
