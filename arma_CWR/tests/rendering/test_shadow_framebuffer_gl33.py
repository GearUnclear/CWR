#!/usr/bin/env python3
"""Compile exact shadow allocator/RAII definitions and execute them on EGL GL.

Requires moderngl, a C++17 compiler, OpenGL development headers and linker library.
--source selects EngineGL33_ShadowDepth.cpp (production by default). Extracted
production definitions are not rewritten. Logging and GL33Bind cache methods are
stubbed; this is not the complete engine, render entrypoint, or runtime state test.
"""
import argparse
import ctypes
import ctypes.util
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile

import moderngl

SOURCE = Path(__file__).resolve().parents[2] / 'engine/PoseidonGL33/EngineGL33_ShadowDepth.cpp'


def definition(source, pattern, *, semicolon=False):
    """Select an exact named definition, balancing braces outside comments/strings."""
    match = re.search(pattern, source)
    if not match:
        raise ValueError(f'Required production definition absent: {pattern}')
    opening = source.find('{', match.start())
    level, index = 0, opening
    while index < len(source):
        if source.startswith('//', index):
            newline = source.find('\n', index+2)
            index = len(source) if newline < 0 else newline+1
            continue
        if source.startswith('/*', index):
            closing = source.find('*/', index+2)
            if closing < 0:
                raise ValueError('Unterminated C++ comment')
            index = closing+2
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
            level += 1
        elif char == '}':
            level -= 1
            if level == 0:
                end = index+1
                if semicolon:
                    while end < len(source) and source[end].isspace():
                        end += 1
                    if end == len(source) or source[end] != ';':
                        raise ValueError('Missing struct terminator')
                    end += 1
                return source[match.start():end]
        index += 1
    raise ValueError('Unterminated C++ definition')


PREAMBLE = r'''
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#define LOG_ERROR(...)
// These cache/log stubs isolate allocation; they do not test engine cache semantics.
namespace GL33Bind { void OnTexDeleted(GLuint) {} void Invalidate() {} }
GLuint s_fbo=0, s_tex=0, s_arrFbo=0, s_arrTex=0;
int s_res=0, s_arrRes=0, s_arrLayers=0;
'''
WRAPPERS = r'''
extern "C" int initialize(int array, int size, int layers) {
    return array ? EnsureArrayTarget(size, layers) : EnsureTarget(size);
}
extern "C" int cached_size(int array) { return array ? s_arrRes : s_res; }
extern "C" int cached_layers() { return s_arrLayers; }
extern "C" void cleanup() {
    glDeleteFramebuffers(1, &s_fbo); glDeleteFramebuffers(1, &s_arrFbo);
    glDeleteTextures(1, &s_tex); glDeleteTextures(1, &s_arrTex);
    s_fbo=s_arrFbo=s_tex=s_arrTex=0; s_res=s_arrRes=s_arrLayers=0;
}
'''


def extracted_unit(source):
    parts = (
        definition(source, r'struct\s+ShadowFramebufferBindings\b', semicolon=True),
        definition(source, r'bool\s+EnsureTarget\s*\(\s*int\s+res\s*\)'),
        definition(source, r'bool\s+EnsureArrayTarget\s*\(\s*int\s+res\s*,\s*int\s+layers\s*\)'),
    )
    return PREAMBLE+'\n\n'.join(parts)+WRAPPERS,parts


def run(library):
    gl_name = ctypes.util.find_library('GL')
    if not gl_name:
        raise RuntimeError('Desktop OpenGL runtime library not found')
    gl = ctypes.CDLL(gl_name)
    uint,integer = ctypes.c_uint,ctypes.c_int
    for name,args,result in (
        ('glGenFramebuffers',[integer,ctypes.POINTER(uint)],None),
        ('glDeleteFramebuffers',[integer,ctypes.POINTER(uint)],None),
        ('glBindFramebuffer',[uint,uint],None),
        ('glGetIntegerv',[uint,ctypes.POINTER(integer)],None),
        ('glGetError',[],uint),
    ):
        function = getattr(gl,name)
        function.argtypes,function.restype = args,result
    library.initialize.argtypes,library.initialize.restype = [integer,integer,integer],integer
    library.cached_size.argtypes,library.cached_size.restype = [integer],integer
    library.cached_layers.argtypes,library.cached_layers.restype = [],integer
    library.cleanup.argtypes,library.cleanup.restype = [],None
    read_fbo,draw_fbo = uint(),uint()
    rows = []
    gl.glGenFramebuffers(1,ctypes.byref(read_fbo))
    gl.glGenFramebuffers(1,ctypes.byref(draw_fbo))
    try:
        for array in (False,True):
            cases = [('first',32,4,1),('reuse',32,4,1),('resize',64,4,1),
                     ('zero_size_failure',0,4,0),('recovery',64,4,1)]
            if array:
                cases += [('layer_count_change',64,2,1)]
            for name,size,layers,success in cases:
                gl.glBindFramebuffer(0x8CA8,read_fbo)  # READ_FRAMEBUFFER
                gl.glBindFramebuffer(0x8CA9,draw_fbo)  # DRAW_FRAMEBUFFER
                result = library.initialize(array,size,layers)
                actual_read,actual_draw = integer(),integer()
                gl.glGetIntegerv(0x8CAA,ctypes.byref(actual_read))
                gl.glGetIntegerv(0x8CA6,ctypes.byref(actual_draw))
                if result != success:
                    raise AssertionError(f'{name}: allocator returned {result}, expected {success}')
                if actual_read.value != read_fbo.value or actual_draw.value != draw_fbo.value:
                    raise AssertionError(f'{name}: separate read/draw framebuffer bindings not restored')
                if library.cached_size(array) != (size if success else 0):
                    raise AssertionError(f'{name}: stale cached dimensions')
                if array and library.cached_layers() != (layers if success else 0):
                    raise AssertionError(f'{name}: stale cached layer count')
                error = gl.glGetError()
                if error:
                    raise AssertionError(f'{name}: unexpected GL error 0x{error:x}')
                rows.append({'case':name,'target':'array' if array else 'single',
                             'requested_size':size,'requested_layers':layers if array else None,
                             'allocation_succeeded':bool(result),'distinct_read_draw_restored':True,
                             'cached_metadata_valid':True,'result':'PASS'})
    finally:
        library.cleanup()
        gl.glBindFramebuffer(0x8D40,0)
        gl.glDeleteFramebuffers(1,ctypes.byref(read_fbo))
        gl.glDeleteFramebuffers(1,ctypes.byref(draw_fbo))
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source',type=Path,default=SOURCE)
    parser.add_argument('--report',type=Path)
    parser.add_argument('--compiler',default=os.environ.get('CXX','c++'),help='C++ compiler command; defaults to CXX or c++')
    parser.add_argument('--include-dir',type=Path,action='append',default=[],help='Additional OpenGL header directory; repeatable')
    parser.add_argument('--gl-library',default='-lGL',help='GL linker option or explicit library path; defaults to -lGL')
    args = parser.parse_args()
    source_bytes = args.source.read_bytes()
    unit,parts = extracted_unit(source_bytes.decode('utf-8'))
    with tempfile.TemporaryDirectory(prefix='gl33-shadow-init-') as directory:
        directory = Path(directory)
        cpp,shared = directory/'shadow_init.cpp',directory/'shadow_init.so'
        cpp.write_text(unit)
        command = shlex.split(args.compiler)+['-shared','-fPIC','-std=c++17']
        for include in args.include_dir:
            command += ['-I',str(include)]
        command += [str(cpp),args.gl_library,'-o',str(shared)]
        compile_result = subprocess.run(command,capture_output=True,text=True)
        if compile_result.returncode:
            raise RuntimeError('Could not compile extracted allocator. Install C++/GL development files or specify --include-dir/--gl-library.\n'+compile_result.stderr)
        ctx = moderngl.create_standalone_context(backend='egl',require=330)
        try:
            library = ctypes.CDLL(str(shared))
            rows = run(library)
            report = {'source':str(args.source.resolve()),'source_sha256':hashlib.sha256(source_bytes).hexdigest(),
                      'extracted_definitions_sha256':[hashlib.sha256(part.encode()).hexdigest() for part in parts],
                      'renderer':ctx.info['GL_RENDERER'],'version':ctx.info['GL_VERSION'],
                      'scope':'Exact production allocator and RAII definitions, real GL, logging/cache stubs. Not complete engine entrypoint, runtime state/cache integration, motion or hardware performance.',
                      'tests':len(rows),'results':rows}
            output = json.dumps(report,indent=2)
            print(output)
            if args.report:
                args.report.write_text(output+'\n')
        finally:
            ctx.release()


if __name__ == '__main__':
    main()
