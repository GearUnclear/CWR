#!/usr/bin/env python3
"""Execute exact CoreGL scoped background helpers on a real EGL context.

Requires moderngl, a C++17 compiler and desktop GL development headers/library.
The exact selected headers are copied unchanged. A temporary glad/gl.h adapter
only includes system GL declarations: calls reach real GL, with no state mocks.
Optional --blend-header adds N3 blend restoration cases. This does not execute
engine cache invalidation, caller gates, allocation failures or render ordering.
"""
import argparse
import ctypes
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

import moderngl

CORE = Path(__file__).resolve().parents[2] / 'engine/Poseidon/Graphics/Core'
HARNESS = r'''
#include "GLDepthStencilState.hpp"
#include "GLPipelineState.hpp"
#include <type_traits>
#include <algorithm>
using Background = Poseidon::render::depthstencil::ScopedBackground;
using Rgb = Poseidon::render::pipeline::ScopedRgbOnly;
static_assert(!std::is_copy_constructible_v<Background> && !std::is_copy_assignable_v<Background>);
static_assert(!std::is_copy_constructible_v<Rgb> && !std::is_copy_assignable_v<Rgb>);
const GLenum queries[] = {
 GL_DEPTH_FUNC, GL_STENCIL_FUNC, GL_STENCIL_REF, GL_STENCIL_VALUE_MASK,
 GL_STENCIL_WRITEMASK, GL_STENCIL_FAIL, GL_STENCIL_PASS_DEPTH_FAIL,
 GL_STENCIL_PASS_DEPTH_PASS, GL_STENCIL_BACK_FUNC, GL_STENCIL_BACK_REF,
 GL_STENCIL_BACK_VALUE_MASK, GL_STENCIL_BACK_WRITEMASK, GL_STENCIL_BACK_FAIL,
 GL_STENCIL_BACK_PASS_DEPTH_FAIL, GL_STENCIL_BACK_PASS_DEPTH_PASS
};
struct State {
 GLboolean depth, stencil, write, rgba[4];
 GLint untouched[15];
 State() {
  depth=glIsEnabled(GL_DEPTH_TEST); stencil=glIsEnabled(GL_STENCIL_TEST);
  glGetBooleanv(GL_DEPTH_WRITEMASK,&write); glGetBooleanv(GL_COLOR_WRITEMASK,rgba);
  for(int i=0;i<15;++i) glGetIntegerv(queries[i],untouched+i);
 }
 bool same(const State& b) const {
  return depth==b.depth && stencil==b.stencil && write==b.write &&
   std::equal(rgba,rgba+4,b.rgba) && std::equal(untouched,untouched+15,b.untouched);
 }
 bool protectedSame(const State& b) const {
  return std::equal(untouched,untouched+15,b.untouched);
 }
};
void configure(int depth,int stencil,int write,int mask) {
 (depth?glEnable:glDisable)(GL_DEPTH_TEST);
 (stencil?glEnable:glDisable)(GL_STENCIL_TEST);
 glDepthMask(write?GL_TRUE:GL_FALSE);
 glColorMask(mask&1?GL_TRUE:GL_FALSE,mask&2?GL_TRUE:GL_FALSE,
             mask&4?GL_TRUE:GL_FALSE,mask&8?GL_TRUE:GL_FALSE);
 glDepthFunc(GL_GEQUAL);
 glStencilFuncSeparate(GL_FRONT,GL_NOTEQUAL,3,0x35);
 glStencilFuncSeparate(GL_BACK,GL_LESS,5,0x69);
 glStencilMaskSeparate(GL_FRONT,0x53); glStencilMaskSeparate(GL_BACK,0x96);
 glStencilOpSeparate(GL_FRONT,GL_INCR_WRAP,GL_DECR_WRAP,GL_REPLACE);
 glStencilOpSeparate(GL_BACK,GL_INVERT,GL_ZERO,GL_KEEP);
}
int scope(const State& before,int mode) {
 Background background; Rgb rgb;
 State active;
 if(active.depth || active.stencil || active.write || !active.rgba[0] ||
    !active.rgba[1] || !active.rgba[2] || active.rgba[3]) return 1;
 if(!active.protectedSame(before)) return 2;
 if(mode==0) return 0; // Early return must invoke both destructors.
 if(mode==3) { // Nested scopes restore outer active state, then original state.
  { Background nestedBackground; Rgb nestedRgb; }
  if(!State().same(active)) return 3;
 }
 background.Restore(); rgb.Restore();
 if(!State().same(before)) return 4;
 if(mode==2) {
  // A second Restore and the later destructors must not overwrite new state.
  configure(!before.depth,!before.stencil,!before.write,5);
  State changed;
  background.Restore(); rgb.Restore();
  if(!State().same(changed)) return 5;
 } else {
  background.Restore(); rgb.Restore();
  if(!State().same(before)) return 6;
 }
 return 0;
}
extern "C" int run_case(int depth,int stencil,int write,int mask,int mode) {
 configure(depth,stencil,write,mask); State before;
 int result=scope(before,mode); if(result) return result;
 State after;
 if(mode==2) {
  configure(!depth,!stencil,!write,5); State expected;
  if(!after.same(expected)) return 7;
 } else if(!after.same(before)) return 8;
 return glGetError()==GL_NO_ERROR ? 0 : 9;
}
'''


BLEND_HARNESS = r'''
#include "GLBlendState.hpp"
using Overlay = Poseidon::render::blend::ScopedAlphaOverlay;
static_assert(!std::is_copy_constructible_v<Overlay> && !std::is_copy_assignable_v<Overlay>);
const GLenum blendFields[] = {GL_BLEND_SRC_RGB,GL_BLEND_DST_RGB,GL_BLEND_SRC_ALPHA,
 GL_BLEND_DST_ALPHA,GL_BLEND_EQUATION_RGB,GL_BLEND_EQUATION_ALPHA};
struct BlendState {
 GLboolean enabled; GLint fields[6];
 BlendState() : enabled(glIsEnabled(GL_BLEND)) {
  for(int i=0;i<6;++i) glGetIntegerv(blendFields[i],fields+i);
 }
 bool same(const BlendState& other) const {
  return enabled==other.enabled && std::equal(fields,fields+6,other.fields);
 }
};
const GLenum profiles[4][4] = {
 {GL_ONE,GL_ZERO,GL_ONE,GL_ZERO},
 {GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_DST_ALPHA,GL_ONE_MINUS_DST_ALPHA},
 {GL_DST_COLOR,GL_ONE_MINUS_SRC_COLOR,GL_ONE_MINUS_DST_ALPHA,GL_SRC_ALPHA},
 {GL_CONSTANT_COLOR,GL_ONE_MINUS_CONSTANT_COLOR,GL_CONSTANT_ALPHA,GL_ONE_MINUS_CONSTANT_ALPHA}
};
const GLenum equations[] = {GL_FUNC_ADD,GL_FUNC_SUBTRACT,GL_FUNC_REVERSE_SUBTRACT,GL_MIN,GL_MAX};
void configureBlend(int enabled,int profile,int rgb,int alpha) {
 (enabled?glEnable:glDisable)(GL_BLEND);
 glBlendFuncSeparate(profiles[profile][0],profiles[profile][1],profiles[profile][2],profiles[profile][3]);
 glBlendEquationSeparate(equations[rgb],equations[alpha]);
}
int blendScope(const BlendState& before,int mode) {
 Overlay overlay; BlendState active;
 const GLint expected[]={GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ZERO,GL_FUNC_ADD,GL_FUNC_ADD};
 if(!active.enabled || !std::equal(active.fields,active.fields+6,expected)) return 11;
 if(mode==0) return 0;
 if(mode==3) { { Overlay nested; } if(!BlendState().same(active)) return 12; }
 overlay.Restore();
 if(!BlendState().same(before) || !overlay.MatchesSavedState()) return 13;
 if(mode==2) {
  configureBlend(!before.enabled,3,3,4); BlendState changed;
  overlay.Restore();
  if(!BlendState().same(changed)) return 14;
 } else {
  overlay.Restore();
  if(!BlendState().same(before) || !overlay.MatchesSavedState()) return 15;
 }
 return 0;
}
extern "C" int run_blend_case(int enabled,int profile,int rgb,int alpha,int mode) {
 configureBlend(enabled,profile,rgb,alpha); BlendState before; State unrelatedBefore;
 int result=blendScope(before,mode); if(result) return result;
 if(!State().same(unrelatedBefore)) return 16;
 BlendState after;
 if(mode==2) {
  configureBlend(!enabled,3,3,4); if(!after.same(BlendState())) return 17;
 } else if(!after.same(before)) return 18;
 return glGetError()==GL_NO_ERROR?0:19;
}
'''

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--depth-header', type=Path, default=CORE / 'GLDepthStencilState.hpp')
    parser.add_argument('--pipeline-header', type=Path, default=CORE / 'GLPipelineState.hpp')
    parser.add_argument('--blend-header', type=Path, help='Optional N3 GLBlendState.hpp; enables exhaustive blend-state cases')
    parser.add_argument('--compiler', default=os.environ.get('CXX', 'c++'))
    parser.add_argument('--include-dir', action='append', default=[])
    parser.add_argument('--gl-library', default='-lGL')
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    headers = {name: path.read_bytes() for name, path in
               [('GLDepthStencilState.hpp', args.depth_header), ('GLPipelineState.hpp', args.pipeline_header)]}
    if args.blend_header:
        headers['GLBlendState.hpp'] = args.blend_header.read_bytes()
    with tempfile.TemporaryDirectory(prefix='cwr-background-state-') as directory:
        root = Path(directory)
        for name, content in headers.items():
            (root / name).write_bytes(content)
        (root / 'glad').mkdir()
        (root / 'glad/gl.h').write_text('#define GL_GLEXT_PROTOTYPES\n#include <GL/gl.h>\n#include <GL/glext.h>\n')
        (root / 'probe.cpp').write_text(HARNESS + (BLEND_HARNESS if args.blend_header else ''))
        library_path = root / 'probe.so'
        command = shlex.split(args.compiler) + ['-std=c++17', '-shared', '-fPIC', '-I', str(root)]
        for include in args.include_dir:
            command += ['-I', include]
        command += [str(root / 'probe.cpp'), '-o', str(library_path)] + shlex.split(args.gl_library)
        subprocess.run(command, check=True, capture_output=True, text=True)
        ctx = moderngl.create_standalone_context(backend='egl', require=330)
        try:
            library = ctypes.CDLL(str(library_path))
            library.run_case.argtypes = [ctypes.c_int] * 5
            library.run_case.restype = ctypes.c_int
            cases, failures = [], []
            modes = ['early_return_destructors', 'explicit_restore_twice',
                     'restore_is_inert_after_state_change', 'nested_scope']
            for depth in (0, 1):
                for stencil in (0, 1):
                    for write in (0, 1):
                        for mask in range(16):
                            for mode, name in enumerate(modes):
                                result = library.run_case(depth, stencil, write, mask, mode)
                                row = dict(depth=depth, stencil=stencil, depth_write=write,
                                           rgba_mask=mask, mode=name, error_code=result)
                                cases.append(row)
                                if result:
                                    failures.append(row)
            blend_cases=[]
            if args.blend_header:
                library.run_blend_case.argtypes=[ctypes.c_int]*5
                library.run_blend_case.restype=ctypes.c_int
                for enabled in (0,1):
                    for profile in range(4):
                        for rgb in range(5):
                            for alpha in range(5):
                                for mode,name in enumerate(modes):
                                    code=library.run_blend_case(enabled,profile,rgb,alpha,mode)
                                    row=dict(enabled=enabled,factor_profile=profile,rgb_equation=rgb,
                                             alpha_equation=alpha,mode=name,error_code=code)
                                    blend_cases.append(row)
                                    if code:failures.append(row)
            report = dict(result='FAIL' if failures else 'PASS', cases=len(cases), blend_cases=len(blend_cases), failures=failures,
                          source_sha256={name: hashlib.sha256(data).hexdigest() for name, data in headers.items()},
                          source_paths=[str(args.depth_header), str(args.pipeline_header)]+([str(args.blend_header)] if args.blend_header else []),
                          renderer={k: ctx.info.get(k) for k in ('GL_VENDOR', 'GL_RENDERER', 'GL_VERSION')},
                          compiler=args.compiler, backend='EGL', modes=modes,
                          coverage='128 initial enable/write-mask combinations x4 lifecycle modes; '
                                   'depth function and distinct front/back stencil function/ref/value/write masks/ops preserved; '
                                   'compile-time noncopyability checks.', scope=__doc__, results=cases, blend_results=blend_cases)
            text = json.dumps(report, indent=2)
            if args.report:
                args.report.write_text(text + '\n')
            print(text)
            return 1 if failures else 0
        finally:
            ctx.release()


if __name__ == '__main__':
    raise SystemExit(main())
