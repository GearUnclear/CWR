#!/usr/bin/env python3
"""Execute the exact cloud Bindings RAII struct against real desktop GL on EGL.

Requires moderngl, a C++17 compiler, and desktop GL headers/library. Only the
unchanged production Bindings definition is compiled, with standard GL headers
and a test harness. No GL functions or state queries are mocked. Engine methods,
cache invalidation, allocator failure handling, and rendering are NOT executed.
Separate source guards check helper placement and the opt-in-before-allocation
contract; these are structural checks, not engine integration coverage.
"""
import argparse
import ctypes
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile

import moderngl

SOURCE = Path(__file__).resolve().parents[2] / 'engine/PoseidonGL33/EngineGL33_CloudRegistration.cpp'
MODES = ('lexical_destructor', 'early_return', 'exception_unwind', 'nested_scope',
         'incomplete_framebuffer_return', 'shader_compile_failure_return',
         'program_link_failure_return', 'partial_resource_cleanup_return')
FIELDS = {1: 'read_fbo', 2: 'draw_fbo', 4: 'program', 8: 'vao', 16: 'array_buffer',
          32: 'active_texture', 64: 'viewport', 128: 'polygon_mode',
          256: 'textures_units_0_3_7_5', 512: 'samplers_units_0_3_7_5',
          1024: 'six_capabilities', 2048: 'clip_distances',
          4096: 'unowned_depth_stencil_blend_masks', 8192: 'vao_element_buffer'}
ERRORS = {1: 'constructor_did_not_disable_caps_or_clips',
          2: 'constructor_changed_other_state', 3: 'nested_scope_restore_failed',
          4: 'failure_scenario_did_not_fail', 5: 'final_restore_failed',
          6: 'unexpected_gl_error', 7: 'initial_setup_gl_error'}


def definition(source, pattern, *, semicolon=False):
    """Return original bytes-as-text, balancing braces outside comments/strings."""
    matches = list(re.finditer(pattern, source))
    if len(matches) != 1:
        raise ValueError(f'Expected exactly one production definition: {pattern}')
    start = matches[0].start()
    index = source.index('{', matches[0].end() - 1)
    depth = 0
    while index < len(source):
        if source.startswith('//', index):
            end = source.find('\n', index + 2)
            index = len(source) if end < 0 else end + 1
            continue
        if source.startswith('/*', index):
            end = source.find('*/', index + 2)
            if end < 0:
                raise ValueError('Unterminated C++ comment')
            index = end + 2
            continue
        raw = re.match(r'R"([^ ()\\\t\r\n]{0,16})\(', source[index:])
        if raw:
            end = source.find(')' + raw[1] + '"', index + raw.end())
            if end < 0:
                raise ValueError('Unterminated C++ raw string')
            index = end + len(raw[1]) + 2
            continue
        char = source[index]
        if char in ('"', "'"):
            quote = char
            index += 1
            while index < len(source):
                if source[index] == '\\':
                    index += 2
                elif source[index] == quote:
                    index += 1
                    break
                else:
                    index += 1
            continue
        if char == '{':
            depth += 1
        elif char == '}':
            depth -= 1
            if depth == 0:
                end = index + 1
                if semicolon:
                    while end < len(source) and source[end].isspace():
                        end += 1
                    if end == len(source) or source[end] != ';':
                        raise ValueError('Missing struct terminator')
                    end += 1
                return source[start:end]
        index += 1
    raise ValueError('Unterminated C++ definition')


def source_guards(source):
    """Report bounded lexical contracts separately from executed GL cases."""
    enabled = definition(source, r'bool\s+EngineGL33::CloudRegistrationEnabled\s*\([^)]*\)\s*const\s*\{')
    prepare = definition(source, r'void\s+EngineGL33::PrepareCloudRegistration\s*\([^)]*\)\s*\{')
    # Remove comments so a commented-out guard cannot satisfy these checks.
    uncommented = re.sub(r'//[^\n]*|/\*.*?\*/', '', prepare, flags=re.S)
    compact = re.sub(r'\s+', '', uncommented)
    gate = compact.find('if(!CloudRegistrationEnabled()||_cloudFailed)return;')
    invalid_input = re.search(r'if\(!input\.complete\|\|.*?\)\{.*?return;\}', compact)
    allocation = re.search(r'\b(?:compileCloud|glCreate\w+|glGen\w+)\s*\(', uncommented)
    before_allocation = uncommented[:allocation.start()] if allocation else ''
    compact_before = re.sub(r'\s+', '', before_allocation)
    helpers = ('Bindings bindings;', 'Poseidon::render::depthstencil::ScopedBackground depth;',
               'Poseidon::render::pipeline::ScopedRgbOnly color;',
               'Poseidon::render::blend::ScopedAlphaOverlay blend;')
    rows = [
        ('explicit_environment_opt_in', bool(re.search(r'std::getenv\("CWR_CLOUD_REGISTRATION"\)', enabled))
         and bool(re.search(r'return\s+p\s*&&\s*std::strcmp\(p,\s*"1"\)\s*==\s*0\s*;', enabled))),
        ('disabled_or_failed_returns_before_allocation', gate >= 0 and gate < len(compact_before)),
        ('invalid_input_returns_before_allocation', bool(invalid_input)
         and invalid_input.end() <= len(compact_before)),
    ]
    rows.extend((helper.split(' ')[0] + '_before_allocation',
                 re.sub(r'\s+', '', helper) in compact_before) for helper in helpers)
    return [{'case': name, 'result': 'PASS' if passed else 'FAIL',
             'coverage': 'source_only_not_executed_engine'} for name, passed in rows]


PREAMBLE = r'''
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <algorithm>
#include <stdexcept>
'''
HARNESS = r'''
namespace probe {
constexpr GLenum caps[] = {GL_CULL_FACE, GL_SCISSOR_TEST, GL_SAMPLE_ALPHA_TO_COVERAGE,
 GL_COLOR_LOGIC_OP, GL_RASTERIZER_DISCARD, GL_FRAMEBUFFER_SRGB};
constexpr int units[] = {0, 3, 7, 5}; // 7 is texture-upload scratch; 5 is unowned.
GLuint fbos[3]{}, programs[2]{}, vaos[2]{}, buffers[4]{}, textures[8]{}, samplers[8]{};
int clips=0, maxUnits=0, lastDiff=0, lastStage=0;
GLenum lastError=GL_NO_ERROR;
struct State {
 GLint read,draw,program,vao,array,active,element,viewport[4],polygon[2],texture[4],sampler[4];
 GLboolean enabled[6],clip[8]{},unowned[8];
 State() {
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&read); glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&draw);
  glGetIntegerv(GL_CURRENT_PROGRAM,&program); glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&vao);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&array); glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING,&element);
  glGetIntegerv(GL_VIEWPORT,viewport); glGetIntegerv(GL_POLYGON_MODE,polygon);
  for(int i=0;i<4;++i) {
   glActiveTexture(GL_TEXTURE0+units[i]); glGetIntegerv(GL_TEXTURE_BINDING_2D,texture+i);
   glGetIntegeri_v(GL_SAMPLER_BINDING,units[i],sampler+i);
  }
  glActiveTexture(active);
  for(int i=0;i<6;++i) enabled[i]=glIsEnabled(caps[i]);
  for(int i=0;i<clips;++i) clip[i]=glIsEnabled(GL_CLIP_DISTANCE0+i);
  unowned[0]=glIsEnabled(GL_DEPTH_TEST); unowned[1]=glIsEnabled(GL_STENCIL_TEST);
  unowned[2]=glIsEnabled(GL_BLEND); glGetBooleanv(GL_DEPTH_WRITEMASK,unowned+3);
  glGetBooleanv(GL_COLOR_WRITEMASK,unowned+4);
 }
 int difference(const State& b) const {
  int d=(read!=b.read?1:0)|(draw!=b.draw?2:0)|(program!=b.program?4:0)|
   (vao!=b.vao?8:0)|(array!=b.array?16:0)|(active!=b.active?32:0)|
   (!std::equal(viewport,viewport+4,b.viewport)?64:0)|
   (!std::equal(polygon,polygon+2,b.polygon)?128:0)|
   (!std::equal(texture,texture+4,b.texture)?256:0)|
   (!std::equal(sampler,sampler+4,b.sampler)?512:0)|
   (!std::equal(enabled,enabled+6,b.enabled)?1024:0)|
   (!std::equal(clip,clip+clips,b.clip)?2048:0)|
   (!std::equal(unowned,unowned+8,b.unowned)?4096:0)|(element!=b.element?8192:0);
  return d;
 }
};
GLuint shader(GLenum type,const char* text) {
 GLuint id=glCreateShader(type); glShaderSource(id,1,&text,nullptr); glCompileShader(id);
 GLint ok=0; glGetShaderiv(id,GL_COMPILE_STATUS,&ok);
 if(!ok) {glDeleteShader(id); return 0;} return id;
}
GLuint program() {
 const char* vs="#version 330 core\nvoid main(){gl_Position=vec4(0,0,0,1);}";
 const char* fs="#version 330 core\nout vec4 color;void main(){color=vec4(1);}";
 GLuint v=shader(GL_VERTEX_SHADER,vs),f=shader(GL_FRAGMENT_SHADER,fs);
 if(!v||!f) {if(v)glDeleteShader(v);if(f)glDeleteShader(f);return 0;}
 GLuint p=glCreateProgram(); glAttachShader(p,v); glAttachShader(p,f); glLinkProgram(p);
 glDeleteShader(v); glDeleteShader(f); GLint ok=0; glGetProgramiv(p,GL_LINK_STATUS,&ok);
 if(!ok) {glDeleteProgram(p);return 0;} return p;
}
void setCaps(unsigned mask,unsigned clipMask) {
 for(int i=0;i<6;++i) (mask&(1u<<i)?glEnable:glDisable)(caps[i]);
 for(int i=0;i<clips;++i) (clipMask&(1u<<i)?glEnable:glDisable)(GL_CLIP_DISTANCE0+i);
}
void configure(unsigned mask,unsigned clipMask,int profile) {
 glBindFramebuffer(GL_READ_FRAMEBUFFER,profile==1?0:fbos[profile==3?1:0]);
 glBindFramebuffer(GL_DRAW_FRAMEBUFFER,profile==2?0:fbos[profile==3?0:1]);
 glUseProgram(profile==1?0:programs[profile%2]);
 glBindVertexArray(profile==1?0:vaos[profile%2]);
 glBindBuffer(GL_ARRAY_BUFFER,profile==1?0:buffers[profile%2]);
 glViewport(11+profile,17-profile,111+profile,83+profile);
 GLenum modes[]={GL_LINE,GL_POINT,GL_FILL,GL_LINE}; glPolygonMode(GL_FRONT_AND_BACK,modes[profile]);
 for(int i=0;i<4;++i) {
  glActiveTexture(GL_TEXTURE0+units[i]);
  const bool zeroTexture=(profile==1&&i%2==0)||(profile==2&&i%2==1);
  const bool zeroSampler=(profile==2&&i%2==0)||(profile==1&&i%2==1);
  glBindTexture(GL_TEXTURE_2D,zeroTexture?0:textures[i]);
  glBindSampler(units[i],zeroSampler?0:samplers[i]);
 }
 const int active[]={0,3,5,7}; glActiveTexture(GL_TEXTURE0+active[profile]);
 setCaps(mask,clipMask);
 (profile&1?glEnable:glDisable)(GL_DEPTH_TEST); (profile&2?glEnable:glDisable)(GL_STENCIL_TEST);
 (profile&1?glDisable:glEnable)(GL_BLEND); glDepthMask(profile&2?GL_TRUE:GL_FALSE);
 glColorMask(profile&1?GL_TRUE:GL_FALSE,GL_TRUE,GL_FALSE,profile&2?GL_TRUE:GL_FALSE);
}
void mutate(unsigned mask,unsigned clipMask,int profile) {
 glBindFramebuffer(GL_READ_FRAMEBUFFER,fbos[2]); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,fbos[2]);
 glUseProgram(programs[(profile+1)%2]); glBindVertexArray(vaos[(profile+1)%2]);
 glBindBuffer(GL_ARRAY_BUFFER,buffers[3]); glViewport(1,2,37,29);
 glPolygonMode(GL_FRONT_AND_BACK,profile==2?GL_LINE:GL_FILL);
 for(int i=0;i<3;++i) {
  glActiveTexture(GL_TEXTURE0+units[i]); glBindTexture(GL_TEXTURE_2D,textures[i+4]);
  glBindSampler(units[i],samplers[i+4]);
 }
 glActiveTexture(GL_TEXTURE0+maxUnits-1); setCaps(mask^63u,clipMask^255u);
}
int body(const State& before,unsigned mask,unsigned clipMask,int profile,int mode) {
 Bindings bindings;
 State entered;
 for(auto enabled:entered.enabled) if(enabled) return 1;
 for(int i=0;i<clips;++i) if(entered.clip[i]) return 1;
 lastDiff=entered.difference(before)&~(32|1024|2048);
 if(lastDiff) return 2;
 mutate(mask,clipMask,profile);
 if(mode==1) return 0;
 if(mode==2) throw std::runtime_error("intentional probe unwind");
 if(mode==3) {
  State outer;
  {
   Bindings nested; mutate(mask^21u,clipMask^85u,(profile+1)%4);
   glBindFramebuffer(GL_READ_FRAMEBUFFER,0); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,fbos[1]);
   glViewport(8,9,23,45);
   for(int i=0;i<3;++i) {
    glActiveTexture(GL_TEXTURE0+units[i]); glBindTexture(GL_TEXTURE_2D,textures[i]);
    glBindSampler(units[i],samplers[i]);
   }
   glActiveTexture(GL_TEXTURE0);
  }
  if((lastDiff=State().difference(outer))) return 3;
 }
 if(mode==4) {
  lastStage=glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(lastStage==GL_FRAMEBUFFER_COMPLETE) return 4;
  return 0; // Actual incomplete FBO, then exact production destructor.
 }
 if(mode==5) {
  GLuint bad=shader(GL_FRAGMENT_SHADER,"#version 330 core\nthis is not valid GLSL");
  if(bad) {glDeleteShader(bad);return 4;} lastStage=1; return 0;
 }
 if(mode==6) {
  GLuint bad=glCreateProgram(); glLinkProgram(bad);
  GLint ok=1; glGetProgramiv(bad,GL_LINK_STATUS,&ok); glDeleteProgram(bad);
  if(ok) return 4; lastStage=1; return 0;
 }
 if(mode==7) {
  // Genuine transient objects are bound and deleted before early return. This
  // tests cleanup-shaped state, not a forced OOM or the engine failure branch.
  GLuint f,t,s,v,b;
  glGenFramebuffers(1,&f); glBindFramebuffer(GL_FRAMEBUFFER,f);
  glGenTextures(1,&t); glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D,t);
  glGenSamplers(1,&s); glBindSampler(7,s);
  glGenVertexArrays(1,&v); glBindVertexArray(v);
  glGenBuffers(1,&b); glBindBuffer(GL_ARRAY_BUFFER,b);
  glDeleteFramebuffers(1,&f); glDeleteTextures(1,&t); glDeleteSamplers(1,&s);
  glDeleteVertexArrays(1,&v); glDeleteBuffers(1,&b); lastStage=1; return 0;
 }
 return 0;
}
} // namespace probe
extern "C" int initialize() {
 using namespace probe;
 glGetIntegerv(GL_MAX_CLIP_DISTANCES,&clips); clips=std::min(clips,8);
 glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,&maxUnits);
 if(clips<1||maxUnits<8) return 1;
 glGenFramebuffers(3,fbos); glGenVertexArrays(2,vaos); glGenBuffers(4,buffers);
 glGenTextures(8,textures); glGenSamplers(8,samplers);
 programs[0]=program(); programs[1]=program(); if(!programs[0]||!programs[1]) return 2;
 for(int i=0;i<2;++i) {glBindVertexArray(vaos[i]); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,buffers[i+2]);}
 return glGetError()==GL_NO_ERROR?0:3;
}
extern "C" int clip_count() {return probe::clips;}
extern "C" int last_difference() {return probe::lastDiff;}
extern "C" int last_stage() {return probe::lastStage;}
extern "C" unsigned last_gl_error() {return probe::lastError;}
extern "C" int run_case(int mask,int clipMask,int profile,int mode) {
 using namespace probe; lastDiff=0; lastStage=0; lastError=GL_NO_ERROR;
 configure(mask,clipMask,profile); State before;
 if((lastError=glGetError())!=GL_NO_ERROR) return 7;
 int code=0;
 try {code=body(before,mask,clipMask,profile,mode);}
 catch(const std::runtime_error&) {if(mode!=2) throw; lastStage=1;}
 const int finalDiff=State().difference(before);
 if(finalDiff) {lastDiff=finalDiff;if(!code)code=5;}
 lastError=glGetError(); if(lastError!=GL_NO_ERROR&&!code)code=6;
 return code;
}
extern "C" void cleanup() {
 using namespace probe;
 glUseProgram(0); glBindFramebuffer(GL_FRAMEBUFFER,0); glBindVertexArray(0);
 glBindBuffer(GL_ARRAY_BUFFER,0);
 for(int unit:units) {glActiveTexture(GL_TEXTURE0+unit);glBindTexture(GL_TEXTURE_2D,0);glBindSampler(unit,0);}
 setCaps(0,0); glActiveTexture(GL_TEXTURE0);
 for(GLuint p:programs) if(p)glDeleteProgram(p);
 glDeleteFramebuffers(3,fbos); glDeleteVertexArrays(2,vaos); glDeleteBuffers(4,buffers);
 glDeleteTextures(8,textures); glDeleteSamplers(8,samplers);
}
'''


def run_cases(library):
    for name in ('initialize', 'clip_count', 'last_difference', 'last_stage', 'last_gl_error'):
        getattr(library, name).argtypes = []
        getattr(library, name).restype = ctypes.c_uint if name == 'last_gl_error' else ctypes.c_int
    library.cleanup.argtypes, library.cleanup.restype = [], None
    library.run_case.argtypes, library.run_case.restype = [ctypes.c_int] * 4, ctypes.c_int
    initialized = library.initialize()
    if initialized:
        library.cleanup()
        raise RuntimeError(f'Real-GL harness initialization failed: {initialized}')
    clips = library.clip_count()
    rows = []
    try:
        def case(group, mask, clip_mask, profile, mode):
            code = library.run_case(mask, clip_mask, profile, mode)
            row = dict(group=group, caps_mask=mask, clip_mask=clip_mask, profile=profile,
                       mode=MODES[mode], result='FAIL' if code else 'PASS')
            if code:
                bits = library.last_difference()
                row.update(error_code=code, error=ERRORS.get(code, 'unknown'),
                           differing_fields=[name for flag, name in FIELDS.items() if bits & flag],
                           gl_error=hex(library.last_gl_error()))
            if mode >= 4:
                row['failure_stage_evidence'] = library.last_stage()
            rows.append(row)
        for mask in range(64):
            for profile in range(4):
                for mode in range(4):
                    case('all_six_capability_combinations', mask, (mask * 37) % (1 << clips), profile, mode)
        for clip_mask in range(1 << clips):
            for mode in range(4):
                case('all_available_clip_combinations', (clip_mask * 13) % 64, clip_mask, clip_mask % 4, mode)
        for mask in (0, 21, 42, 63):
            for profile in range(4):
                for mode in range(4, 8):
                    case('real_gl_failure_shaped_exit', mask, (1 << clips) - 1 if profile & 1 else 0, profile, mode)
    finally:
        library.cleanup()
    return clips, rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=SOURCE)
    parser.add_argument('--compiler', default=os.environ.get('CXX', 'c++'))
    parser.add_argument('--include-dir', type=Path, action='append', default=[])
    parser.add_argument('--gl-library', default='-lGL', help='GL linker flags or explicit library path')
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    source_bytes = args.source.read_bytes()
    source = source_bytes.decode('utf-8')
    bindings = definition(source, r'struct\s+Bindings\s*\{', semicolon=True)
    guards = source_guards(source)
    with tempfile.TemporaryDirectory(prefix='cwr-cloud-bindings-') as directory:
        root = Path(directory)
        cpp, shared = root / 'probe.cpp', root / 'probe.so'
        cpp.write_text(PREAMBLE + '\n' + bindings + '\n' + HARNESS)
        command = shlex.split(args.compiler) + ['-std=c++17', '-shared', '-fPIC', '-O1']
        for include in args.include_dir:
            command += ['-I', str(include)]
        command += [str(cpp), '-o', str(shared)] + shlex.split(args.gl_library)
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode:
            raise RuntimeError('Cannot compile exact Bindings. Specify GL development headers/library with '
                               '--include-dir and --gl-library if needed.\n' + result.stderr)
        ctx = moderngl.create_standalone_context(backend='egl', require=330)
        try:
            library = ctypes.CDLL(str(shared))
            clips, rows = run_cases(library)
            failures = [row for row in rows + guards if row['result'] == 'FAIL']
            report = dict(result='FAIL' if failures else 'PASS',
                          source=str(args.source.resolve()), source_sha256=hashlib.sha256(source_bytes).hexdigest(),
                          exact_bindings_sha256=hashlib.sha256(bindings.encode()).hexdigest(),
                          backend='EGL', renderer={key: ctx.info.get(key) for key in ('GL_VENDOR', 'GL_RENDERER', 'GL_VERSION')},
                          compiler=args.compiler, gl_library=args.gl_library, clip_distances_tested=clips,
                          owned_texture_units=[0, 3, 7], untouched_texture_unit=5,
                          executed_cases=len(rows), source_guard_cases=len(guards), failures=failures,
                          coverage='64 capability combinations x4 binding profiles x4 lifecycle modes; '
                                   'every available clip-distance mask x4 lifecycle modes; '
                                   '64 real-GL failure-shaped returns. Distinct READ/DRAW FBOs including '
                                   'one default binding; zero/nonzero program/VAO/buffer/texture/sampler; '
                                   'all three polygon modes; four incoming active units; VAO element-buffer '
                                   'and depth/stencil/blend/write-mask sentinels.',
                          limitations='Failure operations are harness branches, not executed engine branches. '
                                       'No OOM injection, engine cache checks, rendering, pixel validation, '
                                       'resource allocation-policy execution, or timing/performance claims. '
                                       'Core background/RGB/blend helpers are checked for source placement only.',
                          source_guards=guards, results=rows)
            output = json.dumps(report, indent=2)
            if args.report:
                args.report.parent.mkdir(parents=True, exist_ok=True)
                args.report.write_text(output + '\n')
            print(output)
            return 1 if failures else 0
        finally:
            ctx.release()


if __name__ == '__main__':
    raise SystemExit(main())
