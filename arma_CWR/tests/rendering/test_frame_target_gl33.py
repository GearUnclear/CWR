#!/usr/bin/env python3
"""Execute the exact GL33 frame-target transition methods against counted GL stubs.

This is a C++17 state/logic regression, not a complete engine or real-GL test.
No game assets or OpenGL development packages are needed. The actual method
bodies and member initializers are extracted without rewriting them. GL/SDL and
logging stubs expose allocation counts, capability changes, and deterministic
framebuffer failures. Optional --real-gl also executes genuine EGL allocations
and zero-size framebuffer failures (Linux, moderngl and libGL required). --source and --header support preserved before snapshots.
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

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'engine/PoseidonGL33/EngineGL33_VertexBuffer.cpp'
HEADER = ROOT / 'engine/PoseidonGL33/EngineGL33.hpp'


def definition(source, name):
    # These top-level definitions have indented internal braces. Selecting the
    # next column-zero close preserves the production function verbatim.
    match = re.search(r'^void EngineGL33::' + name + r'\([^\n]*\)\n\{.*?^\}',
                      source, re.MULTILINE | re.DOTALL)
    if not match:
        raise ValueError(f'Required production definition absent: {name}')
    return match.group(0)


PREAMBLE = r'''
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <utility>
using GLenum = unsigned int;
using GLuint = unsigned int;
using GLint = int;
using GLsizei = int;
constexpr GLenum GL_MAX_SAMPLES = 1, GL_RENDERBUFFER = 2, GL_RGBA8 = 3;
constexpr GLenum GL_DEPTH24_STENCIL8 = 4, GL_FRAMEBUFFER = 5;
constexpr GLenum GL_COLOR_ATTACHMENT0 = 6, GL_DEPTH_STENCIL_ATTACHMENT = 7;
constexpr GLenum GL_FRAMEBUFFER_COMPLETE = 8, GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT = 9;
struct SDL_Window { int w = 64, h = 48; };
void SDL_GetWindowSizeInPixels(SDL_Window* window, int* w, int* h) {
    *w = window->w; *h = window->h;
}
struct StubState {
    int maxSamples = 4, allocations = 0, deletes = 0, checks = 0, warnings = 0;
    bool failScaled = false, failResolve = false;
    GLuint nextId = 1;
    std::set<GLuint> live;
} stub;
#define LOG_WARN(...) (++stub.warnings)
#define LOG_INFO(...) ((void)0)
void glGetIntegerv(GLenum name, GLint* value) {
    if (name != GL_MAX_SAMPLES) std::abort();
    *value = stub.maxSamples;
}
void generate(GLsizei count, GLuint* ids) {
    for (int i = 0; i < count; ++i) { ids[i] = stub.nextId++; stub.live.insert(ids[i]); }
}
void glGenRenderbuffers(GLsizei n, GLuint* ids) { generate(n, ids); }
void glGenFramebuffers(GLsizei n, GLuint* ids) { generate(n, ids); }
void destroy(GLsizei count, const GLuint* ids) {
    for (int i = 0; i < count; ++i) if (ids[i]) { ++stub.deletes; stub.live.erase(ids[i]); }
}
void glDeleteRenderbuffers(GLsizei n, const GLuint* ids) { destroy(n, ids); }
void glDeleteFramebuffers(GLsizei n, const GLuint* ids) { destroy(n, ids); }
void glBindRenderbuffer(GLenum, GLuint) {}
void glBindFramebuffer(GLenum, GLuint) {}
void glFramebufferRenderbuffer(GLenum, GLenum, GLenum, GLuint) {}
void glRenderbufferStorageMultisample(GLenum, GLsizei samples, GLenum format, GLsizei, GLsizei) {
    if (samples < 0 || samples > stub.maxSamples) std::abort();
    if (format == GL_RGBA8) ++stub.allocations;
}
void glRenderbufferStorage(GLenum, GLenum, GLsizei, GLsizei) {}
GLenum glCheckFramebufferStatus(GLenum) {
    bool scaled = (stub.checks++ % 2) == 0;
    return (scaled ? stub.failScaled : stub.failResolve)
        ? GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT : GL_FRAMEBUFFER_COMPLETE;
}
class EngineGL33 {
public:
    int _w = 64, _h = 48;
    SDL_Window* _sdlWindow = nullptr;
    bool _msaaActive = false;
'''

TESTS = r'''
int failures = 0, checks = 0;
void check(const std::string& name, bool passed, int allocations = -1) {
    if (checks++) std::cout << ",\n";
    std::cout << "  {\"case\":\"" << name << "\",\"result\":\"" << (passed ? "PASS" : "FAIL") << "\"";
    if (allocations >= 0) std::cout << ",\"allocation_attempts\":" << allocations;
    std::cout << "}";
    if (!passed) ++failures;
}
void frames(EngineGL33& e, int count = 120) {
    for (int i = 0; i < count; ++i) e.ApplyPendingRenderScale();
}
void stable(const std::string& name, EngineGL33& e, int count = 120) {
    int allocations = stub.allocations, deletes = stub.deletes, warnings = stub.warnings;
    frames(e, count);
    check(name, stub.allocations == allocations && stub.deletes == deletes && stub.warnings == warnings,
          stub.allocations - allocations);
}
void valid(const std::string& name, const EngineGL33& e, int width, int height, int samples, float scale) {
    check(name, e.SSAAActive() && e._ssaaW == width && e._ssaaH == height &&
          e._msaaSamples == samples && e._renderScale == scale && e._msaaActive == (samples > 1) &&
          stub.live.size() == 5);
}
int main() {
    std::cout << "[\n";
    {
        stub = {};
        EngineGL33 e;
        check("defaults_remain_off", e._pendingMsaaSamples == 0 && e._pendingRenderScale == 1.0f);
        e.ApplyPendingRenderScale();
        valid("off_target", e, 64, 48, 0, 1.0f);
        stable("off_120_frames", e);
        e.SetMsaaSamples(4); e.ApplyPendingRenderScale();
        valid("supported_4x", e, 64, 48, 4, 1.0f);
        stable("supported_4x_120_frames", e);
        e.DestroySSAATarget();
        check("explicit_destroy_releases_all", stub.live.empty() && !e.SSAAActive());
    }
    {
        stub = {};
        EngineGL33 e;
        e.SetMsaaSamples(8);
        frames(e);
        check("request8_cap4_allocates_once", stub.allocations == 1, stub.allocations);
        valid("request8_cap4_effective4", e, 64, 48, 4, 1.0f);
        check("clamp_preserves_requested8", e._pendingMsaaSamples == 8);
        stable("clamped_120_more_frames", e);
        int before = stub.allocations;
        e.SetMsaaSamples(4); e.ApplyPendingRenderScale();
        check("equivalent_request_reuses_target", stub.allocations == before);
        e.SetMsaaSamples(8); e.ApplyPendingRenderScale();
        check("equivalent_clamped_request_reuses_target", stub.allocations == before);
        e.SetRenderScale(1.5f); e.ApplyPendingRenderScale();
        check("scale_change_allocates_once", stub.allocations == before + 1);
        valid("scale_change_dimensions", e, 96, 72, 4, 1.5f);
        stable("scale_change_120_frames", e);
        SDL_Window window;
        window.w = 100; window.h = 70; e._sdlWindow = &window;
        before = stub.allocations; e.ApplyPendingRenderScale();
        check("window_pixel_change_allocates_once", stub.allocations == before + 1);
        valid("window_pixel_dimensions", e, 150, 105, 4, 1.5f);
        stable("window_change_120_frames", e);
        before = stub.allocations; stub.maxSamples = 2; e.ApplyPendingRenderScale();
        check("capability_decrease_allocates_once", stub.allocations == before + 1);
        valid("capability_decrease_effective2", e, 150, 105, 2, 1.5f);
        stable("capability_decrease_120_frames", e);
        before = stub.allocations; stub.maxSamples = 8; e.ApplyPendingRenderScale();
        check("capability_increase_allocates_once", stub.allocations == before + 1);
        valid("capability_increase_restores_requested8", e, 150, 105, 8, 1.5f);
        stable("capability_increase_120_frames", e);
        e.DestroySSAATarget();
        before = stub.allocations; e.ApplyPendingRenderScale();
        check("explicit_reset_recreates_same_target_once", stub.allocations == before + 1);
        stable("explicit_reset_120_frames", e);
        e.DestroySSAATarget();
    }
    for (int cap : {0, 1, 2, 4, 8}) {
        stub = {}; stub.maxSamples = cap;
        EngineGL33 e; e.SetMsaaSamples(8); e.ApplyPendingRenderScale();
        std::string label = "cap_" + std::to_string(cap);
        valid(label + "_effective", e, 64, 48, cap, 1.0f);
        stable(label + "_120_frames", e);
        e.DestroySSAATarget();
    }
    for (int failureKind = 0; failureKind < 2; ++failureKind) {
        for (int recoveryKind = 0; recoveryKind < 5; ++recoveryKind) {
            stub = {};
            stub.failScaled = failureKind == 0; stub.failResolve = failureKind == 1;
            EngineGL33 e; e.SetMsaaSamples(8); e.SetRenderScale(1.5f);
            std::string label = std::string(failureKind ? "resolve_failure_" : "scaled_failure_") +
                                std::to_string(recoveryKind);
            e.ApplyPendingRenderScale();
            check(label + "_clean_fallback", !e.SSAAActive() && stub.live.empty() &&
                  !e._msaaActive && e._msaaSamples == 0 && e._renderScale == 1.0f);
            check(label + "_preferences_preserved", e._pendingMsaaSamples == 8 && e._pendingRenderScale == 1.5f);
            stable(label + "_120_failed_frames_no_retry", e);
            check(label + "_single_warning", stub.warnings == 1);
            stub.failScaled = stub.failResolve = false;
            stable(label + "_no_automatic_retry_after_transient_failure", e);
            int before = stub.allocations;
            if (recoveryKind == 0) e._w = 80;
            if (recoveryKind == 1) e.SetRenderScale(2.0f);
            if (recoveryKind == 2) e.SetMsaaSamples(2);
            if (recoveryKind == 3) e.DestroySSAATarget();
            if (recoveryKind == 4) stub.maxSamples = 2;
            e.ApplyPendingRenderScale();
            check(label + "_changed_key_or_reset_retries_once", stub.allocations == before + 1 && e.SSAAActive());
            valid(label + "_recovered_preferences", e, recoveryKind == 0 ? 120 : (recoveryKind == 1 ? 128 : 96),
                  recoveryKind == 1 ? 96 : 72, (recoveryKind == 2 || recoveryKind == 4) ? 2 : 4,
                  recoveryKind == 1 ? 2.0f : 1.5f);
            stable(label + "_120_recovered_frames", e);
            e.DestroySSAATarget();
            check(label + "_no_leaks", stub.live.empty());
        }
    }
    {
        stub = {};
        EngineGL33 e; e.SetMsaaSamples(8); e.SetRenderScale(2.0f); e.ApplyPendingRenderScale();
        stub.failResolve = true; e._w = 80; e.ApplyPendingRenderScale();
        check("active_target_failure_clears_effective_aa", !e.SSAAActive() && !e._msaaActive &&
              e._msaaSamples == 0 && e._renderScale == 1.0f && stub.live.empty());
        stable("active_target_failure_no_retry", e);
        e.DestroySSAATarget();
        stub.failResolve = false; stub.maxSamples = 2; e.ApplyPendingRenderScale();
        valid("explicit_target_reset_requeries_capability", e, 160, 96, 2, 2.0f);
        e.DestroySSAATarget();
    }
    {
        stub = {};
        EngineGL33 first, second;
        first.SetMsaaSamples(8); second.SetMsaaSamples(8);
        first.ApplyPendingRenderScale(); second.ApplyPendingRenderScale();
        check("instance_caches_are_independent", stub.allocations == 2 && stub.live.size() == 10);
        stable("first_instance_stable", first);
        first.DestroySSAATarget();
        stable("second_instance_survives_other_reset", second);
        second.DestroySSAATarget();
        check("both_instances_release_resources", stub.live.empty());
    }
    {
        EngineGL33 e;
        for (auto pair : {std::pair<int,int>{-3,0}, {0,0}, {1,0}, {2,2}, {3,2}, {4,4}, {7,4}, {8,8}, {16,8}}) {
            e.SetMsaaSamples(pair.first);
            check("requested_step_" + std::to_string(pair.first), e._pendingMsaaSamples == pair.second);
        }
        e.SetRenderScale(0.5f); check("scale_minimum_unchanged", e._pendingRenderScale == 1.0f);
        e.SetRenderScale(3.0f); check("scale_maximum_unchanged", e._pendingRenderScale == 2.0f);
    }
    std::cout << "\n]\n";
    return failures ? 1 : 0;
}
'''


def extracted_unit(source, header):
    methods = [definition(source, name) for name in
               ('SetRenderScale', 'SetMsaaSamples', 'DestroySSAATarget', 'ApplyPendingRenderScale')]
    start = header.index('    float _renderScale =')
    end = header.index('    // Pixel size of the current render target:', start)
    members = header[start:end]
    declarations = '\n'.join(method.split('\n{', 1)[0].replace('EngineGL33::', '') + ';' for method in methods)
    return PREAMBLE + members + declarations + '\n};\n' + '\n\n'.join(methods) + TESTS, methods, members



REAL_GL_PREAMBLE = r'''
using GLenum = unsigned int; using GLuint = unsigned int; using GLint = int; using GLsizei = int;
constexpr GLenum GL_MAX_SAMPLES=0x8D57, GL_RENDERBUFFER=0x8D41, GL_RGBA8=0x8058;
constexpr GLenum GL_DEPTH24_STENCIL8=0x88F0, GL_FRAMEBUFFER=0x8D40;
constexpr GLenum GL_COLOR_ATTACHMENT0=0x8CE0, GL_DEPTH_STENCIL_ATTACHMENT=0x821A;
constexpr GLenum GL_FRAMEBUFFER_COMPLETE=0x8CD5;
extern "C" {
void glGetIntegerv(GLenum, GLint*); void glGenFramebuffers(GLsizei,GLuint*);
void glGenRenderbuffers(GLsizei,GLuint*); void glDeleteFramebuffers(GLsizei,const GLuint*);
void glDeleteRenderbuffers(GLsizei,const GLuint*); void glBindFramebuffer(GLenum,GLuint);
void glBindRenderbuffer(GLenum,GLuint); void glFramebufferRenderbuffer(GLenum,GLenum,GLenum,GLuint);
void glRenderbufferStorageMultisample(GLenum,GLsizei,GLenum,GLsizei,GLsizei);
void glRenderbufferStorage(GLenum,GLenum,GLsizei,GLsizei); GLenum glCheckFramebufferStatus(GLenum);
}
int generated=0, warnings=0;
void countedGenFramebuffers(GLsizei n,GLuint* ids) { generated+=n; glGenFramebuffers(n,ids); }
#define glGenFramebuffers countedGenFramebuffers
#define LOG_WARN(...) (++warnings)
#define LOG_INFO(...) ((void)0)
struct SDL_Window {};
void SDL_GetWindowSizeInPixels(SDL_Window*,int*,int*) {}
class EngineGL33 {
public:
int _w=64, _h=48; SDL_Window* _sdlWindow=nullptr; bool _msaaActive=false;
'''
REAL_GL_WRAPPERS = r'''
EngineGL33 engine;
extern "C" void request(int samples, float scale) { engine.SetMsaaSamples(samples); engine.SetRenderScale(scale); }
extern "C" void resize(int w,int h) { engine._w=w; engine._h=h; }
extern "C" void apply(int n) { for(int i=0;i<n;++i) engine.ApplyPendingRenderScale(); }
extern "C" void cleanup() { engine.DestroySSAATarget(); }
extern "C" int state(int index) {
 switch(index) {
 case 0: return generated/2; case 1: return engine._msaaSamples; case 2: return engine._pendingMsaaSamples;
 case 3: return engine._ssaaW; case 4: return engine._ssaaH; case 5: return engine._msaaActive;
 case 6: return engine.SSAAActive(); case 7: return int(engine._renderScale*1000);
 case 8: return int(engine._pendingRenderScale*1000); case 9: return warnings;
 } return -1;
}
'''


def run_real_gl(source, header, compiler):
    """Run the same exact methods with genuine EGL allocations and failures."""
    import moderngl
    _, methods, members = extracted_unit(source, header)
    declarations = '\n'.join(method.split('\n{', 1)[0].replace('EngineGL33::', '') + ';' for method in methods)
    unit = REAL_GL_PREAMBLE + members + declarations + '\n};\n' + '\n\n'.join(methods) + REAL_GL_WRAPPERS
    gl_name = ctypes.util.find_library('GL')
    if not gl_name:
        raise RuntimeError('Desktop OpenGL runtime library not found')
    ctx = moderngl.create_standalone_context(backend='egl', require=330)
    gl = ctypes.CDLL(gl_name)
    gl.glGetError.restype = ctypes.c_uint
    gl.glGetIntegerv.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_int)]
    gl.glGetIntegerv.restype = None
    maximum = ctypes.c_int()
    gl.glGetIntegerv(0x8D57, ctypes.byref(maximum))  # GL_MAX_SAMPLES
    effective = min(8, maximum.value)
    rows = []
    try:
        with tempfile.TemporaryDirectory(prefix='gl33-frame-target-real-') as directory:
            cpp, shared = Path(directory) / 'frame_target.cpp', Path(directory) / 'frame_target.so'
            cpp.write_text(unit)
            subprocess.run(shlex.split(compiler) + ['-std=c++17', '-shared', '-fPIC', str(cpp),
                                                   '-Wl,-l:' + gl_name, '-o', str(shared)], check=True)
            library = ctypes.CDLL(str(shared))
            library.request.argtypes = [ctypes.c_int, ctypes.c_float]
            library.resize.argtypes = [ctypes.c_int, ctypes.c_int]
            library.apply.argtypes = library.state.argtypes = [ctypes.c_int]
            library.state.restype = ctypes.c_int
            for name in ('request', 'resize', 'apply', 'cleanup'):
                getattr(library, name).restype = None
            def snapshot(name, attempts, width, height, active=True):
                state = [library.state(i) for i in range(10)]
                error = gl.glGetError()
                expected = [attempts, effective if active else 0, 8, width, height,
                            int(active and effective > 1), int(active), 1500 if active else 1000,
                            1500, int(attempts >= 3)]
                rows.append({'case': name, 'backend': 'real_EGL_GL',
                             'result': 'PASS' if state == expected and error == 0 else 'FAIL',
                             'allocation_attempts': state[0], 'effective_samples': state[1],
                             'requested_samples': state[2], 'target_width': state[3], 'target_height': state[4],
                             'target_active': bool(state[6]), 'effective_scale_milli': state[7],
                             'requested_scale_milli': state[8], 'warnings': state[9], 'gl_error': error})
            try:
                library.request(8, 1.5)
                library.apply(120)
                snapshot('real_requested8_120_frames', 1, 96, 72)
                library.resize(80, 60)
                library.apply(120)
                snapshot('real_resized_120_frames', 2, 120, 90)
                # Zero-size GL renderbuffer storage is legal but cannot make a
                # complete framebuffer: a genuine allocation-failure path.
                library.resize(0, 0)
                library.apply(120)
                snapshot('real_zero_size_failure_120_frames', 3, 0, 0, False)
                library.resize(64, 48)
                library.apply(120)
                snapshot('real_restored_120_frames', 4, 96, 72)
                library.cleanup()
                library.apply(120)
                snapshot('real_explicit_reset_120_frames', 5, 96, 72)
            finally:
                library.cleanup()
        return rows, {'GL_VERSION': ctx.info.get('GL_VERSION'), 'GL_RENDERER': ctx.info.get('GL_RENDERER'),
                      'GL_MAX_SAMPLES': maximum.value,
                      'scope': 'Exact production methods using real EGL GL, class/SDL shims and counted framebuffer generation; not full engine or performance acceptance.'}
    finally:
        ctx.release()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=SOURCE)
    parser.add_argument('--header', type=Path, default=HEADER)
    parser.add_argument('--compiler', default=os.environ.get('CXX', 'clang++'))
    parser.add_argument('--report', type=Path)
    parser.add_argument('--real-gl', action='store_true', help='Also execute the allocator with EGL GL (requires moderngl and libGL on Linux)')
    args = parser.parse_args()
    source, header = args.source.read_bytes(), args.header.read_bytes()
    unit, methods, members = extracted_unit(source.decode(), header.decode())
    with tempfile.TemporaryDirectory(prefix='gl33-frame-target-') as directory:
        cpp, binary = Path(directory) / 'frame_target.cpp', Path(directory) / 'frame_target'
        cpp.write_text(unit)
        command = shlex.split(args.compiler) + ['-std=c++17', '-Wall', '-Wextra', '-Werror', str(cpp), '-o', str(binary)]
        subprocess.run(command, check=True)
        result = subprocess.run([str(binary)], check=False, capture_output=True, text=True)
        if result.returncode not in (0, 1):
            raise RuntimeError(f'Fixture crashed ({result.returncode}): {result.stderr}')
        rows = json.loads(result.stdout)
    real_gl = None
    if args.real_gl:
        real_rows, real_gl = run_real_gl(source.decode(), header.decode(), args.compiler)
        rows.extend(real_rows)
    report = {
        'scope': 'Exact production C++ methods and member initializers; counted GL/SDL stubs, not real GL or full engine.',
        'source': str(args.source), 'source_sha256': hashlib.sha256(source).hexdigest(),
        'header': str(args.header), 'header_sha256': hashlib.sha256(header).hexdigest(),
        'extracted_methods_sha256': hashlib.sha256('\n\n'.join(methods).encode()).hexdigest(),
        'extracted_members_sha256': hashlib.sha256(members.encode()).hexdigest(),
        'compiler': args.compiler, 'real_gl': real_gl, 'passed': sum(r['result'] == 'PASS' for r in rows),
        'failed': sum(r['result'] == 'FAIL' for r in rows), 'checks': rows,
    }
    if args.report:
        args.report.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({key: value for key, value in report.items() if key != 'checks'}, indent=2))
    for row in rows:
        if row['result'] == 'FAIL':
            print('FAIL:', row['case'], row.get('allocation_attempts', ''))
    raise SystemExit(1 if report['failed'] else 0)


if __name__ == '__main__':
    main()
