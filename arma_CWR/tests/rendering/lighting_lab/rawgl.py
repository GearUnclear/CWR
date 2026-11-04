"""The few raw GL calls moderngl does not wrap: depth texture arrays (the
engine's shadow map is a GL_DEPTH_COMPONENT24 2D array), sampler objects with
comparison mode, and binding arbitrary texture targets to a unit."""

from __future__ import annotations

import ctypes

import gl_common as G

GL_TEXTURE_2D = 0x0DE1
GL_TEXTURE_2D_ARRAY = 0x8C1A
GL_TEXTURE0 = 0x84C0
GL_DEPTH_COMPONENT = 0x1902
GL_DEPTH_COMPONENT24 = 0x81A6
GL_FLOAT = 0x1406
GL_UNSIGNED_BYTE = 0x1401
GL_RGBA = 0x1908
GL_RGBA8 = 0x8058
GL_RGBA16 = 0x805B
GL_UNSIGNED_SHORT = 0x1403
GL_RG = 0x8227
GL_RG32F = 0x8230
GL_RED = 0x1903
GL_R8 = 0x8229
GL_NEAREST = 0x2600
GL_LINEAR = 0x2601
GL_REPEAT = 0x2901
GL_CLAMP_TO_EDGE = 0x812F
GL_TEXTURE_MIN_FILTER = 0x2801
GL_TEXTURE_MAG_FILTER = 0x2800
GL_TEXTURE_WRAP_S = 0x2802
GL_TEXTURE_WRAP_T = 0x2803
GL_TEXTURE_COMPARE_MODE = 0x884C
GL_TEXTURE_COMPARE_FUNC = 0x884D
GL_COMPARE_REF_TO_TEXTURE = 0x884E
GL_LEQUAL = 0x0203
GL_FRAMEBUFFER = 0x8D40
GL_DEPTH_ATTACHMENT = 0x8D00
GL_UNPACK_ALIGNMENT = 0x0CF5

_fns: dict = {}

_SIGS = {
    b"glGenTextures": (None, ctypes.c_int, ctypes.POINTER(ctypes.c_uint)),
    b"glBindTexture": (None, ctypes.c_uint, ctypes.c_uint),
    b"glTexImage3D": (None, ctypes.c_uint, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int,
                      ctypes.c_int, ctypes.c_uint, ctypes.c_uint, ctypes.c_void_p),
    b"glTexImage2D": (None, ctypes.c_uint, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int,
                      ctypes.c_uint, ctypes.c_uint, ctypes.c_void_p),
    b"glTexParameteri": (None, ctypes.c_uint, ctypes.c_uint, ctypes.c_int),
    b"glActiveTexture": (None, ctypes.c_uint),
    b"glGenFramebuffers": (None, ctypes.c_int, ctypes.POINTER(ctypes.c_uint)),
    b"glBindFramebuffer": (None, ctypes.c_uint, ctypes.c_uint),
    b"glFramebufferTextureLayer": (None, ctypes.c_uint, ctypes.c_uint, ctypes.c_uint, ctypes.c_int, ctypes.c_int),
    b"glGenSamplers": (None, ctypes.c_int, ctypes.POINTER(ctypes.c_uint)),
    b"glBindSampler": (None, ctypes.c_uint, ctypes.c_uint),
    b"glSamplerParameteri": (None, ctypes.c_uint, ctypes.c_uint, ctypes.c_int),
    b"glPixelStorei": (None, ctypes.c_uint, ctypes.c_int),
}


def fn(name: bytes):
    if name not in _fns:
        p = G._get_proc(name)
        if not p:
            raise RuntimeError(f"GL entry point {name!r} unavailable")
        restype, *args = _SIGS[name]
        _fns[name] = ctypes.CFUNCTYPE(restype, *args)(p)
    return _fns[name]


def gen_texture() -> int:
    t = ctypes.c_uint(0)
    fn(b"glGenTextures")(1, ctypes.byref(t))
    return t.value


def bind_texture(unit: int, target: int, tex: int) -> None:
    fn(b"glActiveTexture")(GL_TEXTURE0 + unit)
    fn(b"glBindTexture")(target, tex)
    fn(b"glActiveTexture")(GL_TEXTURE0)


def _params(target: int, linear: bool, repeat: bool) -> None:
    f = GL_LINEAR if linear else GL_NEAREST
    w = GL_REPEAT if repeat else GL_CLAMP_TO_EDGE
    p = fn(b"glTexParameteri")
    p(target, GL_TEXTURE_MIN_FILTER, f)
    p(target, GL_TEXTURE_MAG_FILTER, f)
    p(target, GL_TEXTURE_WRAP_S, w)
    p(target, GL_TEXTURE_WRAP_T, w)


def depth_array(res: int, layers: int = 1) -> int:
    tex = gen_texture()
    fn(b"glActiveTexture")(GL_TEXTURE0 + 6)
    fn(b"glBindTexture")(GL_TEXTURE_2D_ARRAY, tex)
    fn(b"glTexImage3D")(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, res, res, layers, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                        None)
    _params(GL_TEXTURE_2D_ARRAY, False, False)
    fn(b"glBindTexture")(GL_TEXTURE_2D_ARRAY, 0)
    fn(b"glActiveTexture")(GL_TEXTURE0)
    return tex


def depth_layer_fbo(tex: int, layer: int = 0) -> int:
    f = ctypes.c_uint(0)
    fn(b"glGenFramebuffers")(1, ctypes.byref(f))
    fn(b"glBindFramebuffer")(GL_FRAMEBUFFER, f.value)
    fn(b"glFramebufferTextureLayer")(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex, 0, layer)
    fn(b"glBindFramebuffer")(GL_FRAMEBUFFER, 0)
    return f.value


def rgba16_array(data: bytes, width: int, height: int, layers: int) -> int:
    tex = gen_texture()
    fn(b"glActiveTexture")(GL_TEXTURE0 + 6)
    fn(b"glBindTexture")(GL_TEXTURE_2D_ARRAY, tex)
    fn(b"glPixelStorei")(GL_UNPACK_ALIGNMENT, 1)
    fn(b"glTexImage3D")(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA16, width, height, layers, 0, GL_RGBA, GL_UNSIGNED_SHORT, data)
    _params(GL_TEXTURE_2D_ARRAY, True, False)
    fn(b"glBindTexture")(GL_TEXTURE_2D_ARRAY, 0)
    fn(b"glActiveTexture")(GL_TEXTURE0)
    return tex


def compare_sampler() -> int:
    s = ctypes.c_uint(0)
    fn(b"glGenSamplers")(1, ctypes.byref(s))
    p = fn(b"glSamplerParameteri")
    p(s.value, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
    p(s.value, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
    p(s.value, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
    p(s.value, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)
    p(s.value, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE)
    p(s.value, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL)
    return s.value


def bind_sampler(unit: int, sampler: int) -> None:
    fn(b"glBindSampler")(unit, sampler)


_SIGS.update({
    b"glUseProgram": (None, ctypes.c_uint),
    b"glGetUniformLocation": (ctypes.c_int, ctypes.c_uint, ctypes.c_char_p),
    b"glUniform1i": (None, ctypes.c_int, ctypes.c_int),
    b"glGetIntegerv": (None, ctypes.c_uint, ctypes.POINTER(ctypes.c_int)),
})
GL_CURRENT_PROGRAM = 0x8B8D


def set_sampler_unit(program_glo: int, name: str, unit: int) -> bool:
    """glUniform1i for sampler types moderngl does not map (sampler2DArrayShadow)."""
    loc = fn(b"glGetUniformLocation")(program_glo, name.encode())
    if loc < 0:
        return False
    cur = ctypes.c_int(0)
    fn(b"glGetIntegerv")(GL_CURRENT_PROGRAM, ctypes.byref(cur))
    fn(b"glUseProgram")(program_glo)
    fn(b"glUniform1i")(loc, unit)
    fn(b"glUseProgram")(cur.value)
    return True


_SIGS.update({
    b"glReadPixels": (None, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_uint, ctypes.c_uint,
                      ctypes.c_void_p),
})


def read_depth(fbo_glo: int, width: int, height: int):
    import numpy as np
    buf = np.zeros(width * height, dtype=np.float32)
    fn(b"glBindFramebuffer")(GL_FRAMEBUFFER, fbo_glo)
    fn(b"glReadPixels")(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, buf.ctypes.data)
    fn(b"glBindFramebuffer")(GL_FRAMEBUFFER, 0)
    return buf.reshape(height, width)
