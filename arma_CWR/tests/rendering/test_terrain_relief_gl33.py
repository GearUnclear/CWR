#!/usr/bin/env python3
"""Experimental terrain direct-Lambert relief tests on actual extracted GLSL.

Synthetic planar geometry/detail textures, not engine eligibility, actual terrain
art, cutout rasterization, temporal stability or hardware performance. Requires
numpy, moderngl and EGL. The diagnostic-only suite remains separate and intact.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct

import moderngl
import numpy as np


def fragments(text):
    return dict(re.findall(r'static const char (s_\w+GLSL)\[\] = R"\((.*?)\)";', text, re.S))


def function(text, name):
    start = text.index('vec3 ' + name + '(')
    brace = text.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


VERTEX = '''#version 330 core
uniform vec3 sun; uniform float day; uniform float bearing; uniform vec3 direct;
uniform vec3 ambient; uniform vec3 forced; uniform vec3 localLight;
uniform float worldScale; uniform float distanceOffset; uniform float mirrored;
out vec4 vColor; out vec3 vLightNormal; out vec3 vIndirectLight;
flat out vec4 vSkyAmbient; flat out vec3 vForcedSun; flat out vec4 vSunTerm;
flat out vec3 vSunDirection; out vec4 vSpecColor; out vec2 vUV0; out vec2 vUV1;
out float vFogTC; out vec3 vWorldRel;
void main(){vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);
 gl_Position=vec4(p*2.-1.,0,1);vUV0=p;vUV1=vec2(mirrored<0.?1.-p.x:p.x,p.y);
 vWorldRel=vec3((p.x-.5)*worldScale,0.,distanceOffset+(p.y-.5)*worldScale);
 vColor=vec4(.2,.3,.4,.7);vLightNormal=vec3(0,1,0);
 vIndirectLight=ambient+forced+localLight;vSkyAmbient=vec4(ambient,day);
 vForcedSun=forced;vSunTerm=vec4(direct,bearing);vSunDirection=sun;
 vSpecColor=vec4(0);vFogTC=1.;}
'''
TRIANGLE = '''#version 330 core
void main(){vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);gl_Position=vec4(p*2.-1.,0,1);}
'''


class Fixture:
    def __init__(self, source, baseline, ctx=None):
        self.owns_context = ctx is None
        self.ctx = ctx or moderngl.create_standalone_context(backend='egl', require=330)
        self.programs = {}
        for variant, text in [('current', source), ('baseline', baseline)]:
            for material in ['Normal', 'Detail', 'Grass']:
                self.programs[variant, material] = self.ctx.program(
                    vertex_shader=VERTEX, fragment_shader=fragments(text)['s_ps'+material+'GLSL'])
        helper = function(fragments(source)['s_psDetailGLSL'], 'terrainReliefNormal')
        self.probe = self.ctx.program(vertex_shader=TRIANGLE, fragment_shader='''#version 330 core
uniform vec4 terrainControl; uniform vec4 vSkyAmbient; uniform vec4 vSunTerm;
uniform vec3 vSunDirection; uniform vec3 n; uniform vec3 dx; uniform vec3 dy; uniform vec2 heightGradient;
uniform float distanceToCamera; out vec4 result;
''' + helper + '\nvoid main(){result=vec4(terrainReliefNormal(n,dx,dy,heightGradient,distanceToCamera),1.);}')

    def render(self, program, uniforms, ps=None, height='constant'):
        resources=[]
        try:
            for key,value in uniforms.items():
                if key in program: program[key].value=value
            if ps is not None:
                ubo=self.ctx.buffer(ps.tobytes());resources.append(ubo)
                program['PSConstants'].binding=0;ubo.bind_to_uniform_block(0)
                x=np.arange(32,dtype=np.float32)/31
                alpha={'constant':np.full((32,32),.5,np.float32),
                       'ramp':np.tile(x,(32,1)), 'reverse':np.tile(1-x,(32,1))}[height]
                detail=np.ones((32,32,4),np.float32)*.5;detail[:,:,3]=alpha
                for unit,name,data,size in [(0,'tex0',struct.pack('4f',.3,.4,.5,.8),(1,1)),(1,'tex1',detail.tobytes(),(32,32))]:
                    if name in program:
                        tex=self.ctx.texture(size,4,data,dtype='f4');resources.append(tex)
                        tex.filter=(moderngl.LINEAR,moderngl.LINEAR);tex.repeat_x=False;tex.repeat_y=False
                        tex.use(unit);program[name].value=unit
                if 'shadowMap' in program:
                    tex=self.ctx.texture_array((1,1,1),1,struct.pack('f',.5),dtype='f4');resources.append(tex)
                    tex.use(2);program['shadowMap'].value=2
            target=self.ctx.texture((32,32),4,dtype='f4');resources.append(target)
            fbo=self.ctx.framebuffer([target]);resources.append(fbo);fbo.use();self.ctx.viewport=(0,0,32,32)
            vao=self.ctx.vertex_array(program,[]);resources.append(vao);vao.render(vertices=3)
            return np.frombuffer(fbo.read(viewport=(16,16,1,1),components=4,dtype='f4'),np.float32).copy()
        finally:
            for resource in reversed(resources):resource.release()

    def normal(self, **changes):
        values=dict(terrainControl=(0,.04,.35,0),vSkyAmbient=(0,0,0,1),vSunTerm=(1,1,1,1),
                    vSunDirection=(0,1,0),n=(0,1,0),dx=(.02,0,0),dy=(0,0,.02),heightGradient=(.02,.01),distanceToCamera=0)
        values.update(changes)
        return self.render(self.probe,values)[:3]

    def pixel(self, variant='current',material='Detail',strength=.04,height='ramp',**changes):
        uniforms=dict(sun=(.6,.8,0),day=1,bearing=1,direct=(.3,.3,.3),ambient=(.1,.1,.1),
                      forced=(.05,.05,.05),localLight=(.03,.04,.05),worldScale=.64,distanceOffset=0,mirrored=1)
        uniforms.update(changes)
        ps=np.zeros(112,np.float32)
        ps[12:16]=(1,1,1,1);ps[23]=.75;ps[27]=.35;ps[28:32]=(.3,.6,.1,1)
        ps[108:112]=(0,strength,.35,0)
        return self.render(self.programs[variant,material],uniforms,ps,height)

    def close(self):
        for program in [*self.programs.values(),self.probe]:program.release()
        if self.owns_context:self.ctx.release()


def normal_oracle(model, sun, n=(0,1,0), dx=(.02,0,0), dy=(0,0,.02),
                  heightGradient=(.08,-.03), strength=.04, cap=.35, distance=0., day=1.):
    """Independent tangent-plane linear solve, not extracted/rewritten shader code.

    The slope has zero normal component and its dot products with the two
    screen-space position derivatives equal the scaled height derivatives.
    These tests supply finite, nondegenerate frames; invalid-frame guards are
    exercised separately on the actual GLSL helper.
    """
    n=np.asarray(n,dtype=np.float64);sun=np.asarray(sun,dtype=np.float64)
    slope=np.linalg.solve(np.array([dx,dy,n],dtype=np.float64),
                          np.array([*np.asarray(heightGradient)*strength,0.]))
    magnitude=np.linalg.norm(slope)
    if magnitude>cap:slope*=cap/magnitude
    def smooth(low,high,x):
        t=np.clip((x-low)/(high-low),0.,1.)
        return t*t*(3.-2.*t)
    footprint=max(np.linalg.norm(dx),np.linalg.norm(dy))
    fade=(1.-smooth(.06,.35,footprint))*(1.-smooth(15.,60.,distance))*day
    if model in ['p2','p3']:fade*=smooth(.15,.45,max(np.dot(n,sun),0.))
    if model=='p3':fade*=smooth(.15,.45,max(sun[1],0.))
    perturbed=n-slope*fade
    return perturbed/np.linalg.norm(perturbed)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--source',type=Path,default=Path(__file__).resolve().parents[2]/'engine/PoseidonGL33/EngineGL33_Shaders.cpp');p.add_argument('--baseline',type=Path,required=True)
    p.add_argument('--report',type=Path)
    p.add_argument('--model',choices=['p1','p2','p3'],default='p3',help='P3 by default; select p1/p2 explicitly for historical snapshots, never auto-detected')
    p.add_argument('--p2-source',type=Path,help='Optional frozen P2 source for four historical P3 parity checks')
    p.add_argument('--p1-source',type=Path,help='Optional frozen P1 source for six historical incidence parity checks')
    args=p.parse_args()
    source=args.source.read_text();baseline=args.baseline.read_text();f=Fixture(source,baseline);checks=[];not_run=[]
    def check(label,condition):
        if not condition:raise AssertionError(label)
        checks.append(label)
    def equal(label,a,b,tolerance=0):check(label,np.allclose(a,b,rtol=0,atol=tolerance))
    try:
        for gradient in [(0,0),(.001,0),(.1,-.2),(100,-100),(-100,100)]:
            for distance in [0,15,30,59.999,60,100]:
                n=f.normal(heightGradient=gradient,distanceToCamera=distance)
                check(f'finite unit slope bound {gradient}/{distance}',np.isfinite(n).all() and abs(np.linalg.norm(n)-1)<2e-6 and np.linalg.norm(n[[0,2]])/n[1] <= .350002)
        equal('constant height exact normal',f.normal(heightGradient=(0,0)),[0,1,0])
        oblique=tuple((np.array([.3,.8,.5])/np.linalg.norm([.3,.8,.5])).astype(np.float32))
        equal('constant oblique exact normal',f.normal(n=oblique,heightGradient=(0,0)),oblique)
        equal('fully faded oblique exact normal',f.normal(n=oblique,distanceToCamera=60),oblique)
        for cap,strength,label in [(.35,.04,'nominal'),(.70,.08,'exaggerated diagnostic only')]:
            n=f.normal(terrainControl=(0,strength,cap,0),heightGradient=(100,-100))
            check('cap '+label,abs(np.linalg.norm(n[[0,2]])/n[1]-cap)<2e-6)
        angles=[np.linalg.norm(f.normal(distanceToCamera=x)[[0,2]]) for x in np.linspace(15,60,19)]
        check('monotone distance fade',all(a>=b for a,b in zip(angles,angles[1:])))
        angles=[np.linalg.norm(f.normal(dx=(x,0,0),heightGradient=(x,.01))[[0,2]]) for x in np.linspace(.06,.35,19)]
        check('monotone footprint fade',all(a>=b for a,b in zip(angles,angles[1:])))

        for changes in [dict(terrainControl=(0,0,.35,0)),dict(vSkyAmbient=(0,0,0,0)),dict(vSunTerm=(1,1,1,0)),dict(dx=(0,0,0)),dict(dy=(.02,0,0)),dict(dx=(float('nan'),0,0)),dict(heightGradient=(float('inf'),0))]:
            equal('guard '+str(changes),f.normal(**changes),[0,1,0])
        equal('mirrored derivative frame',f.normal(dx=(-.02,0,0),heightGradient=(-.02,.01)),f.normal(),2e-6)
        for edge in [15,60]:
            equal('distance continuity '+str(edge),f.normal(distanceToCamera=edge-1e-4),f.normal(distanceToCamera=edge+1e-4),2e-6)
        for edge in [.06,.35]:
            equal('footprint continuity '+str(edge),f.normal(dx=(edge-1e-6,0,0)),f.normal(dx=(edge+1e-6,0,0)),2e-6)
        for footprint in [.35,.4,1]:equal('footprint fade '+str(footprint),f.normal(dx=(footprint,0,0)),[0,1,0])
        for material in ['Normal','Detail','Grass']:
            for height in ['constant','ramp','reverse']:
                old=f.pixel('baseline',material,height=height)
                equal('disabled baseline '+material+height,f.pixel(material=material,strength=0,height=height),old)
                if material!='Detail' or height=='constant':equal('excluded/constant baseline '+material+height,f.pixel(material=material,height=height),old)
                for changes in [dict(day=0),dict(bearing=0),dict(direct=(0,0,0)),dict(worldScale=32),dict(distanceOffset=100)]:
                    equal('protected/off/faded '+material+height+str(changes),f.pixel(material=material,height=height,**changes),f.pixel('baseline',material,height=height,**changes),2e-6)
                equal('surviving alpha '+material+height,f.pixel(material=material,height=height)[3],old[3])
        for sign in [-1,1]:
            sun=(sign*.6,.8,0)
            a=f.pixel(height='ramp',sun=sun)-f.pixel(height='ramp',sun=sun,strength=0)
            b=f.pixel(height='reverse',sun=sun)-f.pixel(height='reverse',sun=sun,strength=0)
            check('reversed sun response '+str(sign),np.all((a[:3]-b[:3])*sign<0))
        for light in [dict(ambient=(0,0,0),forced=(0,0,0)),dict(localLight=(.2,.1,.3))]:
            off=f.pixel(strength=0,**light);on=f.pixel(**light)
            check('direct-only response '+str(light),np.any(abs(on[:3]-off[:3])>1e-5))
        # Protected additive light must not change the relief delta, below saturation.
        delta=f.pixel()-f.pixel(strength=0)
        for protected in [dict(localLight=(.1,.09,.08)),dict(ambient=(.15,.14,.13)),dict(forced=(.1,.09,.08))]:
            equal('protected contribution delta '+str(protected),f.pixel(**protected)-f.pixel(strength=0,**protected),delta,2e-6)
        check('relief actually exercised',np.max(np.abs(delta[:3]))>1e-4)
        p2_checks_start=len(checks)
        if args.model in ['p2','p3']:
            prior=Fixture(args.p1_source.read_text(),baseline,ctx=f.ctx) if args.p1_source else None
            try:
                previous=0.
                for incidence in [-1.,-.1,0.,.1,.15,.15001,.2,.3,.4,.44999,.45,.7,1.]:
                    sun=(float(np.sqrt(max(0,1-incidence*incidence))),incidence,0.)
                    actual=f.normal(vSunDirection=sun,heightGradient=(.08,-.03))
                    expected=normal_oracle(args.model,sun)
                    equal('P2 original-normal incidence oracle '+str(incidence),actual,expected,2e-6)
                    slope=np.linalg.norm(actual[[0,2]])/actual[1]
                    check('P2 finite monotone incidence '+str(incidence),np.isfinite(actual).all() and slope+2e-6>=previous)
                    previous=slope
                    if incidence<=.15:
                        equal('P2 exact grazing/dark normal '+str(incidence),actual,[0,1,0])
                        equal('P2 exact grazing/dark full fragment '+str(incidence),f.pixel(sun=sun),f.pixel(strength=0,sun=sun))
                    if incidence>=.45 and prior is None:
                        not_run.extend([dict(result="NOT RUN",reason="--p1-source not supplied",case=kind+str(incidence)) for kind in ["P1 helper parity ","P1 fragment parity "]])
                    if incidence>=.45 and prior is not None:
                        equal('P2 high incidence helper P1 parity '+str(incidence),actual,prior.normal(vSunDirection=sun,heightGradient=(.08,-.03)))
                        equal('P2 high incidence fragment P1 parity '+str(incidence),f.pixel(sun=sun),prior.pixel(sun=sun))
                for edge in [.15,.45]:
                    normals=[f.normal(vSunDirection=(float(np.sqrt(1-y*y)),y,0)) for y in [edge-1e-5,edge+1e-5]]
                    equal('P2 incidence endpoint continuity '+str(edge),*normals,2e-6)
                # Original normal is dark/grazing even if a huge height slope would turn it toward the sun.
                equal('P2 original normal drives gate not perturbed normal',f.normal(vSunDirection=(-.995,.1,0),heightGradient=(100,0)),[0,1,0])
            finally:
                if prior is not None:prior.close()
        p3_checks_start=len(checks)
        if args.model=='p3':
            prior=Fixture(args.p2_source.read_text(),baseline,ctx=f.ctx) if args.p2_source else None
            try:
                # Vertical surface faces the low sun: geometric incidence stays high.
                frame=dict(n=(0,0,1),dx=(.02,0,0),dy=(0,.02,0),heightGradient=(.03,-.01))
                for elevation in [-.5,0.,.1,.15,.15001,.21648,.3,.44999,.45,.7]:
                    sun=(0.,elevation,float(np.sqrt(1-elevation*elevation)))
                    actual=f.normal(vSunDirection=sun,**frame)
                    expected=normal_oracle('p3',sun,**frame)
                    equal('P3 artistic elevation oracle tilted normal '+str(elevation),actual,expected,2e-6)
                    check('P3 finite '+str(elevation),np.isfinite(actual).all())
                    if elevation<=.15:equal('P3 exact tilted low-sun identity '+str(elevation),actual,[0,0,1])
                    if elevation>=.45 and prior is None:
                        not_run.extend([dict(result="NOT RUN",reason="--p2-source not supplied",case=kind+str(elevation)) for kind in ["P2 helper parity ","P2 fragment parity "]])
                    if elevation>=.45 and prior is not None:
                        equal('P3 exact high-elevation helper P2 parity '+str(elevation),actual,prior.normal(vSunDirection=sun,**frame))
                        equal('P3 exact high-elevation fragment P2 parity '+str(elevation),f.pixel(sun=sun),prior.pixel(sun=sun))
                for edge in [.15,.45]:
                    normals=[f.normal(vSunDirection=(0.,y,float(np.sqrt(1-y*y))),**frame) for y in [edge-1e-5,edge+1e-5]]
                    equal('P3 elevation endpoint continuity '+str(edge),*normals,2e-6)
            finally:
                if prior is not None:prior.close()
        report=dict(result='PASS',historical_parity_not_run=not_run,model=args.model,preserved_P1_checks=p2_checks_start,incidence_checks=p3_checks_start-p2_checks_start,P3_elevation_checks=len(checks)-p3_checks_start,checks=len(checks),cases=checks,source=str(args.source),source_sha256=hashlib.sha256(args.source.read_bytes()).hexdigest(),baseline=str(args.baseline),baseline_sha256=hashlib.sha256(args.baseline.read_bytes()).hexdigest(),renderer={k:f.ctx.info.get(k) for k in ['GL_RENDERER','GL_VERSION','GL_VENDOR']},scope=__doc__,slope_caps_tested={'nominal':.35,'exaggerated_diagnostic_only':.70})
        if args.p2_source:report.update(p2_source=str(args.p2_source),p2_source_sha256=hashlib.sha256(args.p2_source.read_bytes()).hexdigest())
        if args.p1_source:report.update(p1_source=str(args.p1_source),p1_source_sha256=hashlib.sha256(args.p1_source.read_bytes()).hexdigest())
        if args.report:args.report.write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps(report,indent=2))
    finally:f.close()


if __name__=='__main__':main()
