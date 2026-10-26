"""Compile + link the remastered fragment pieces with the remastered VS on the local GPU (moderngl)."""
import sys, moderngl
from pathlib import Path
dump = Path(__file__).resolve().parent.parent / 'glsl_dump_remaster'
ovr = Path(sys.argv[1])
common = (ovr / 'psRemasterCommon.glsl') if (ovr / 'psRemasterCommon.glsl').exists() else dump / 'psRemasterCommon.glsl'
vs = (dump / 'vsTransformRemaster.glsl').read_text()
ctx = moderngl.create_standalone_context(require=330)
ok = True
for main in ['psNormalRemasterMain', 'psDetailRemasterMain', 'psGrassRemasterMain', 'psWaterRemasterMain']:
    m = (ovr / f'{main}.glsl') if (ovr / f'{main}.glsl').exists() else dump / f'{main}.glsl'
    try:
        ctx.program(vertex_shader=vs, fragment_shader=common.read_text() + m.read_text())
        print('OK  ', main)
    except Exception as e:
        ok = False
        print('FAIL', main, str(e)[:2000])
for f in ovr.glob('psPost*.glsl'):
    vsp = 'D:/Arma_CWA_worktrees/lighting/tmp/research/shade/scripts/vsPost.glsl'
    try:
        ctx.program(vertex_shader=Path(vsp).read_text(), fragment_shader=f.read_text())
        print('OK  ', f.name)
    except Exception as e:
        ok = False
        print('FAIL', f.name, str(e)[:2000])
sys.exit(0 if ok else 1)
