#!/usr/bin/env python3
"""Actual fragment GLSL shadow comparison tests with nonuniform depth arrays.

Requires numpy, moderngl, desktop GL and EGL. Explicit --source selects an O
snapshot; --baseline optionally renders the unchanged legacy shader for parity.
Only the supported comparison preprocessor define is added after #version.
Synthetic vertex outputs isolate fragment PCF/material semantics; this does not
execute engine vertex transforms, sampler/cache lifecycle or temporal gameplay.
Alpha coverage is surviving-fragment preservation only, not alpha-discard/cutout
rasterization or alpha-tested shadow casters.
"""
import argparse
import ctypes
import ctypes.util
import hashlib
import json
import math
from pathlib import Path
import re
import struct

import moderngl
import numpy as np

VERTEX = '''#version 330 core
uniform float distanceValue, fogValue, daylight, sunEnabled;
out vec4 vColor; out vec3 vLightNormal; out vec3 vIndirectLight;
flat out vec4 vSkyAmbient; flat out vec3 vForcedSun;
flat out vec4 vSunTerm; flat out vec3 vSunDirection;
out vec4 vSpecColor; out vec2 vUV0; out vec2 vUV1;
out float vFogTC; out vec3 vWorldRel;
void main() {
 vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);
 gl_Position=vec4(p*2.-1.,0.,1.);
 vColor=vec4(1.,1.,1.,.7); vLightNormal=vec3(0,0,1);
 vIndirectLight=vec3(.05+.1*sunEnabled);
 vSkyAmbient=vec4(0,0,0,daylight); vForcedSun=vec3(.1*sunEnabled);
 vSunTerm=vec4(vec3(.2*sunEnabled),1); vSunDirection=vec3(0,0,1);
 vSpecColor=vec4(0); vUV0=vUV1=vec2(.5);
 vFogTC=fogValue; vWorldRel=vec3(0,0,distanceValue);
}
'''
FRAGMENTS = {'normal':'s_psNormalGLSL', 'detail':'s_psDetailGLSL', 'grass':'s_psGrassGLSL'}


def extract(source):
    return dict(re.findall(r'static const char (s_\w+GLSL)\[\] = R"\((.*?)\)";', source, re.S))


def define_compare(source):
    version, rest = source.split('\n', 1)
    if not version.startswith('#version'):
        raise ValueError('GLSL must start with #version')
    return version + '\n#define CWR_SHADOW_COMPARE_PCF 1\n' + rest


class Fixture:
    def __init__(self, shaders):
        self.ctx = moderngl.create_standalone_context(backend='egl', require=330)
        self.shaders, self.programs = shaders, {}
        self.depth_format = 'D32F'
        self.gl = ctypes.CDLL(ctypes.util.find_library('GL'))
        signatures = {
            'glGenTextures':([ctypes.c_int, ctypes.POINTER(ctypes.c_uint)],None),
            'glDeleteTextures':([ctypes.c_int, ctypes.POINTER(ctypes.c_uint)],None),
            'glActiveTexture':([ctypes.c_uint],None),
            'glBindTexture':([ctypes.c_uint,ctypes.c_uint],None),
            'glTexParameteri':([ctypes.c_uint,ctypes.c_uint,ctypes.c_int],None),
            'glGetTexParameteriv':([ctypes.c_uint,ctypes.c_uint,ctypes.POINTER(ctypes.c_int)],None),
            'glTexImage3D':([ctypes.c_uint,ctypes.c_int,ctypes.c_int,ctypes.c_int,ctypes.c_int,
                             ctypes.c_int,ctypes.c_int,ctypes.c_uint,ctypes.c_uint,ctypes.c_void_p],None),
            'glBindSampler':([ctypes.c_uint,ctypes.c_uint],None),
            'glGetError':([],ctypes.c_uint),
            'glUseProgram':([ctypes.c_uint],None),
            'glUniform1i':([ctypes.c_int,ctypes.c_int],None),
            'glGetIntegeri_v':([ctypes.c_uint,ctypes.c_uint,ctypes.POINTER(ctypes.c_int)],None),
            'glGetSamplerParameteriv':([ctypes.c_uint,ctypes.c_uint,ctypes.POINTER(ctypes.c_int)],None),
        }
        for name,(args,result) in signatures.items():
            getattr(self.gl,name).argtypes=args
            getattr(self.gl,name).restype=result
        self.raw = self.ctx.sampler(filter=(moderngl.NEAREST,moderngl.NEAREST),
                                    repeat_x=False,repeat_y=False,repeat_z=False)
        self.compare = self.ctx.sampler(filter=(moderngl.LINEAR,moderngl.LINEAR),
                                        repeat_x=False,repeat_y=False,repeat_z=False,compare_func='<=')

    def render(self, fragment, mode, depth, case, variant='current'):
        key = (fragment,mode,variant)
        if key not in self.programs:
            fs=self.shaders[variant][FRAGMENTS[fragment]]
            self.programs[key]=self.ctx.program(vertex_shader=VERTEX,
                                                fragment_shader=define_compare(fs) if mode else fs)
        program=self.programs[key]
        for name,value in {'distanceValue':case.get('distance',.5),'fogValue':case.get('fog',1),
                           'daylight':case.get('day',0),'sunEnabled':case.get('sun',1)}.items():
            program[name].value=value
        ps=np.zeros(108,dtype='f4')
        def slot(i,v):ps[i*4:i*4+len(v)]=v
        slot(0,(.1,.2,.3,1));slot(2,(case.get('enabled',1),0,.25,case.get('texel_size',1/depth.shape[1])));slot(3,(1,1,1,1))
        slot(5,(0,0,0,.75));slot(6,(0,0,0,.35));slot(7,(.3,.6,.1,1))
        for layer in range(4):
            uv=case.get('uv',(.5,.5))
            if layer in case.get('outside_layers',[]):uv=(-.1,.5)
            matrix=[0.]*16;matrix[12:16]=[uv[0]*2-1,uv[1]*2-1,case.get('reference',.5),1]
            slot(8+layer*4,matrix)
        slot(24,(1,2,3,4));slot(25,(case.get('layers',4),case.get('fade',.01),case.get('bias',0),0))
        slot(26,(0,0,1,0))
        resources=[];texture=ctypes.c_uint()
        try:
            ubo=self.ctx.buffer(ps.tobytes());resources.append(ubo)
            program['PSConstants'].binding=0;ubo.bind_to_uniform_block(0)
            for unit,name,value in [(0,'tex0',(1,1,1,1)),(1,'tex1',(.5,.5,.5,.5))]:
                if name in program:
                    tex=self.ctx.texture((1,1),4,struct.pack('4f',*value),dtype='f4')
                    resources.append(tex);tex.use(unit);program[name].value=unit
            gl=self.gl;gl.glGenTextures(1,ctypes.byref(texture));gl.glActiveTexture(0x84C0+2)
            gl.glBindTexture(0x8C1A,texture.value)
            # Depth-array storage stays raw even when a comparison sampler is bound.
            for param,value in [(0x884C,0),(0x2801,0x2600),(0x2800,0x2600),
                                (0x2802,0x812F),(0x2803,0x812F)]:
                gl.glTexParameteri(0x8C1A,param,value)
            data=np.ascontiguousarray(depth,dtype='f4')
            gl.glTexImage3D(0x8C1A,0,0x81A6 if self.depth_format=='D24' else 0x8CAC,data.shape[2],data.shape[1],data.shape[0],0,
                            0x1902,0x1406,data.ctypes.data)
            rawmode=ctypes.c_int();gl.glGetTexParameteriv(0x8C1A,0x884C,ctypes.byref(rawmode))
            if rawmode.value!=0:raise AssertionError('Depth texture compare mode must stay NONE')
            sampler=self.compare if mode else self.raw
            sampler.use(2)
            bound_sampler=ctypes.c_int()
            gl.glGetIntegeri_v(0x8919,2,ctypes.byref(bound_sampler))
            for parameter,expected_parameter in [(0x884C,0x884E if mode else 0),
                                                  (0x2801,0x2601 if mode else 0x2600),
                                                  (0x2800,0x2601 if mode else 0x2600)]:
                actual_parameter=ctypes.c_int()
                gl.glGetSamplerParameteriv(bound_sampler.value,parameter,ctypes.byref(actual_parameter))
                if actual_parameter.value!=expected_parameter:
                    raise AssertionError('Sampler mode/filter mismatch')
            # Set sampler2DArrayShadow through GL directly: older ModernGL versions
            # do not dispatch this sampler uniform type consistently.
            gl.glUseProgram(program.glo)
            gl.glUniform1i(program['shadowMap'].location,2)
            target=self.ctx.texture((8,8),4,dtype='f4');resources.append(target)
            fbo=self.ctx.framebuffer([target]);resources.append(fbo);fbo.use();self.ctx.viewport=(0,0,8,8)
            vao=self.ctx.vertex_array(program,[]);resources.append(vao);fbo.clear();vao.render(vertices=3)
            result=np.frombuffer(fbo.read(viewport=(4,4,1,1),components=4,dtype='f4'),np.float32).copy()
            error=gl.glGetError()
            if error:raise AssertionError(f'GL error 0x{error:x}')
            return result
        finally:
            self.gl.glBindSampler(2,0)
            self.gl.glDeleteTextures(1,ctypes.byref(texture))
            for resource in reversed(resources):resource.release()

    def close(self):
        for program in self.programs.values():program.release()
        self.raw.release();self.compare.release();self.ctx.release()


def pcf(depth,layer,uv,reference,compare,texel_size=None):
    size=depth.shape[1]
    def tap(x,y):return float(reference <= float(depth[layer,min(size-1,max(0,y)),min(size-1,max(0,x))]))
    total=0.
    for dy in [-1,0,1]:
        for dx in [-1,0,1]:
            step=1 if texel_size is None else texel_size*size
            x,y=uv[0]*size+dx*step,uv[1]*size+dy*step
            if not compare:total+=tap(math.floor(x),math.floor(y));continue
            x-=.5;y-=.5;ix,iy=math.floor(x),math.floor(y);fx,fy=x-ix,y-iy
            total+=sum(tap(ix+i,iy+j)*(fx if i else 1-fx)*(fy if j else 1-fy)
                       for i in [0,1] for j in [0,1])
    return total/9


def expected(depth,case,compare):
    distance=case.get('distance',.5);count=case.get('layers',4);uv=case.get('uv',(.5,.5))
    first=next((i for i in range(count) if distance<=i+1),count)
    visibility=1.
    if case.get('enabled',1) and first<count:
        blend=min(1,max(0,(distance-(first+1-.15))/.15)) if first+1<count else 0
        total=weight=0.
        for layer in range(first,count):
            delta=layer-first
            w=1-blend if delta==0 else (1 if weight<=0 else (blend if delta==1 else 0))
            if w<=0 or layer in case.get('outside_layers',[]):continue
            reference=case.get('reference',.5)
            if not (0<uv[0]<1 and 0<uv[1]<1 and 0<reference<1):continue
            total+=w*pcf(depth,layer,uv,reference-case.get('bias',0)*(layer+1)**2,compare,case.get('texel_size'));weight+=w
        if weight:
            fade=min(1,max(0,(count-distance)/max(case.get('fade',.01),.001)))
            visibility=1-.75*(1-total/weight)*fade*case.get('fog',1)
    day=case.get('day',0);sun=case.get('sun',1)
    direct=visibility*(1-day)+visibility**2*day
    light=.05+.1*sun*((1-day)+direct*day)+.2*sun*direct
    fog=case.get('fog',1)
    return np.array([fog*light+(1-fog)*x for x in [.1,.2,.3]]+[.7])


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source',type=Path,required=True)
    parser.add_argument('--baseline',type=Path)
    parser.add_argument('--comparison-source',type=Path,
                        help='Optional optimized comparison shader tested against --source and the nine-tap oracle')
    parser.add_argument('--report',type=Path)
    args=parser.parse_args();source=args.source.read_text();shaders={'current':extract(source)}
    baseline_bytes=args.baseline.read_bytes() if args.baseline else None
    if baseline_bytes is not None:shaders['baseline']=extract(baseline_bytes.decode())
    rows=[]
    guards={
        'cache_hash_variant':r'HashShaderSources\s*\(\s*bool\s+comparisonPcf\s*\)',
        'cache_distinct_tags':r'add\(comparisonPcf\s*\?\s*"([^"]+)"\s*:\s*"([^"]+)"\)',
        'cache_uses_effective_variant':r'HashShaderSources\(_shadowComparePcf\)',
        'compare_sampler_LEQUAL':r'glSamplerParameteri\(_shadowCompareSampler,\s*GL_TEXTURE_COMPARE_FUNC,\s*GL_LEQUAL\)',
        'compare_sampler_linear':r'glSamplerParameteri\(_shadowCompareSampler,\s*GL_TEXTURE_MIN_FILTER,\s*GL_LINEAR\)',
        'sampler_bound_with_variant':r'glBindSampler\(2,\s*_shadowComparePcf\s*\?\s*_shadowCompareSampler\s*:\s*0\)',
    }
    for label,pattern in guards.items():
        match=re.search(pattern,source)
        if not match or (label=='cache_distinct_tags' and match[1]==match[2]):
            raise AssertionError('Source wiring guard failed: '+label)
        rows.append(dict(check=label,result='PASS',scope='source guard only, not runtime cache test'))
    comparison_bytes=args.comparison_source.read_bytes() if args.comparison_source else None
    if comparison_bytes is not None:shaders['optimized']=extract(comparison_bytes.decode())
    f=Fixture(shaders)
    try:
        size=8;split=np.zeros((4,size,size),dtype='f4');split[:,:,:4]=.25;split[:,:,4:]=.75
        split[1]=.75;split[2]=.25;split[3]=np.indices((size,size)).sum(axis=0)%2*.5+.25
        cases=[('lit',{},np.ones_like(split)*.75),('occluded',{},np.ones_like(split)*.25),
               ('LEQUAL_equal',{},np.ones_like(split)*.5),('bias_before',{'reference':.5001},np.ones_like(split)*.5),
               ('bias_after',{'reference':.5001,'bias':.001},np.ones_like(split)*.5)]
        for uv in [(.01,.5),(.99,.5),(.5,.01),(.5,.99),(-.01,.5),(1.01,.5)]:cases.append((f'boundary{uv}',{'uv':uv},split))
        for x in np.linspace(.4,.6,17):cases.append((f'subtexel{x:.4f}',{'uv':(float(x),.5)},split))
        for x,y in [(.43,.46),(.49,.51),(.54,.58),(.02,.97)]:
            cases.append((f'checkerboard{x},{y}',{'distance':3.5,'uv':(x,y)},split))
        for ref in [0.,1.,-.01,1.01]:
            cases.append((f'depth_boundary{ref}',{'reference':ref},split))
        cases.append(('layer_bias_squared',{'distance':2.5,'reference':.255,'bias':.001},split))
        for layer in range(4):cases.append((f'layer{layer}',{'distance':layer+.5},split))
        for distance in [.85,.9,.95,1.,1.05,3.995,4.1]:cases.append((f'cascade{distance}',{'distance':distance},split))
        cases += [('coverage_fallthrough',{'outside_layers':[0]},split),('coverage_none',{'outside_layers':[0,1,2,3]},split)]
        for key,value in [('enabled',0),('sun',0),('day',1),('fog',0),('fog',.4)]:cases.append((f'{key}{value}',{key:value},split))
        for fragment in FRAGMENTS:
            for name,case,depth in cases:
                for compare in [False,True]:
                    actual=f.render(fragment,compare,depth,case);want=expected(depth,case,compare)
                    if not np.allclose(actual,want,atol=3e-5,rtol=0):
                        raise AssertionError(f'{fragment}/{name}/{compare}: {actual} != {want}')
                    rows.append(dict(fragment=fragment,case=name,compare=compare,result='PASS',rgba=actual.tolist()))
                    if not compare and args.baseline:
                        baseline=f.render(fragment,False,depth,case,'baseline')
                        if not np.array_equal(actual,baseline):raise AssertionError(f'Legacy baseline changed: {fragment}/{name}')
                        rows.append(dict(fragment=fragment,case=name,check='legacy_baseline_bitwise',result='PASS'))
        for fragment in FRAGMENTS:
            curves={}
            for compare in [False,True]:
                curves[compare]=[x['rgba'][0] for x in rows if x.get('fragment')==fragment
                                 and x.get('case','').startswith('subtexel') and x.get('compare')==compare]
            increments=np.diff(curves[True])
            if not (np.all(increments>0) and len(set(round(x,5) for x in curves[True]))>
                    len(set(round(x,5) for x in curves[False]))):
                raise AssertionError('Comparison subtexel sweep must be monotonic and less stepped')
            rows.append(dict(fragment=fragment,check='subtexel_monotonic_smoothness',result='PASS'))
        equivalence_rows=[]
        if comparison_bytes is not None:
            for depth_format in ['D32F','D24']:
                f.depth_format=depth_format
                for name,case,depth in cases+[('zero_texel_size',{'texel_size':0,'uv':(.49,.51)},split),
                                         ('negative_texel_size',{'texel_size':-.125,'uv':(.49,.51)},split)]:
                    for fragment in FRAGMENTS:
                        old=f.render(fragment,True,depth,case)
                        new=f.render(fragment,True,depth,case,'optimized')
                        error=float(np.max(abs(old-new)))
                        if not np.allclose(old,new,atol=3e-5,rtol=0):
                            raise AssertionError(f'O1/O2 established case {depth_format}/{fragment}/{name}: {old} != {new}')
                        if depth_format=='D32F' and not np.allclose(new,expected(depth,case,True),atol=3e-5,rtol=0):
                            raise AssertionError('Optimized established case differs from nine-tap oracle')
                        equivalence_rows.append(dict(depth_format=depth_format,fragment=fragment,
                                                     case=name,max_error=error,result='PASS'))
            rng=np.random.default_rng(20261016)
            maps=[split]+[rng.uniform(.05,.95,(4,8,8)).astype('f4') for _ in range(4)]
            # Stay away from depth comparison thresholds for portable oracle checks;
            # compare both actual shaders on identical storage, including D24 quantization.
            positions=[(.001,.001),(.999,.999),(.001,.999),(.999,.001),
                       (.5,.5),(.4375,.5625),(.4999,.5001),(.125,.875)]
            positions += [tuple(x) for x in rng.uniform(.001,.999,(12,2))]
            for depth_format in ['D32F','D24']:
                f.depth_format=depth_format
                for map_index,depth in enumerate(maps):
                    for position_index,uv in enumerate(positions):
                        case=dict(uv=uv,distance=[.5,.9,.95,1.5,2.5,3.5][position_index%6],
                                  reference=[.2,.5,.8][position_index%3],
                                  bias=.0001 if position_index%2 else 0)
                        if position_index%7==0:case['outside_layers']=[0]
                        for fragment in FRAGMENTS:
                            old=f.render(fragment,True,depth,case)
                            new=f.render(fragment,True,depth,case,'optimized')
                            error=float(np.max(abs(old-new)))
                            if not np.allclose(old,new,atol=3e-5,rtol=0):
                                raise AssertionError(f'O1/O2 {depth_format}/{fragment}/{map_index}/{uv}: {old} != {new}')
                            if depth_format=='D32F' and not np.allclose(new,expected(depth,case,True),atol=3e-5,rtol=0):
                                raise AssertionError('Optimized comparison differs from independent nine-tap oracle')
                            raw=f.render(fragment,False,depth,case)
                            optimized_raw=f.render(fragment,False,depth,case,'optimized')
                            if not np.array_equal(raw,optimized_raw):
                                raise AssertionError('Optimization changed legacy raw result')
                            equivalence_rows.append(dict(depth_format=depth_format,fragment=fragment,
                                                         map=map_index,uv=uv,max_error=error,
                                                         legacy_bitwise=True,result='PASS'))
        report=dict(result='PASS',checks=len(rows),source=str(args.source),source_sha256=hashlib.sha256(source.encode()).hexdigest(),
                    renderer={k:f.ctx.info.get(k) for k in ['GL_RENDERER','GL_VERSION','GL_VENDOR']},
                    baseline=str(args.baseline) if args.baseline else None,
                    baseline_sha256=hashlib.sha256(baseline_bytes).hexdigest() if baseline_bytes is not None else None,
                    results=rows,scope=__doc__,
                    comparison_source=str(args.comparison_source) if args.comparison_source else None,
                    comparison_sha256=hashlib.sha256(comparison_bytes).hexdigest() if comparison_bytes else None,
                    equivalence_cases=len(equivalence_rows),equivalence=equivalence_rows,
                    equivalence_limits='3e-5 absolute RGBA tolerance; driver interpolation precision may differ. '
                                       'D32F also checked against independent nine-bilinear oracle; D24 compares '
                                       'actual shaders on identical quantized storage. Not a universal hardware guarantee.',
                    sampler_contract='Depth32F array compareNONE; dedicated NEAREST raw and LINEAR LEQUAL comparison samplers, CLAMP_TO_EDGE.')
        text=json.dumps(report,indent=2)
        if args.report:args.report.write_text(text+'\n')
        print(text)
    finally:f.close()


if __name__=='__main__':main()
