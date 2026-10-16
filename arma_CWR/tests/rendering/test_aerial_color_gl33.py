#!/usr/bin/env python3
"""S directional aerial color: actual GLSL, independent color oracle, no engine.

Checks unchanged fog transmittance via baseline fog-color substitution, protected
foreground/alpha, nearfield fade and exact analytic-sky horizon agreement. Artistic
color coefficients are not physical scattering or a visual-quality acceptance.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import numpy as np
import moderngl

LUMA=np.array([.2126,.7152,.0722])

def color_oracle(base,ray,sun,weight):
    base=np.asarray(base,np.float64);ray=np.asarray(ray,np.float64);sun=np.asarray(sun,np.float64)
    if weight<=0 or not np.isfinite(np.concatenate([base,ray,sun])).all() or min(base)<0 or np.dot(ray,ray)<=1e-12 or np.dot(sun,sun)<=1e-12 or np.dot(base,LUMA)<=1e-8:return base
    colors=[base*np.array(tint) for tint in [(.8,.96,1.14),(1.12,1.02,.86)]]
    colors=[c*(np.dot(base,LUMA)/np.dot(c,LUMA)) for c in colors]
    cosine=max(np.dot(ray,sun)/(np.linalg.norm(ray)*np.linalg.norm(sun)),0.)
    directional=colors[0]*(1-cosine**4)+colors[1]*cosine**4
    weight=np.clip(weight,0,1)
    return base*(1-weight)+directional*weight


def distance_weight(world):
    t=np.clip((np.linalg.norm(world)-120)/230,0,1)
    return t*t*(3-2*t)


def extract(source):return dict(re.findall(r'static const char (s_\w+GLSL)\[\] = R"\((.*?)\)";',source,re.S))


def helper(source):
    start=source.index('vec3 directionalAerialColor(');i=source.index('{',start)+1;depth=1
    while depth:
        depth+=(source[i]=='{')-(source[i]=='}');i+=1
    return source[start:i]


VERTEX='''#version 330 core
uniform vec3 world;uniform float day;uniform float fog;
out vec4 vColor;out vec3 vLightNormal;out vec3 vIndirectLight;
flat out vec4 vSkyAmbient;flat out vec3 vForcedSun;flat out vec4 vSunTerm;flat out vec3 vSunDirection;
out vec4 vSpecColor;out vec2 vUV0;out vec2 vUV1;out float vFogTC;out vec3 vWorldRel;
void main(){vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);gl_Position=vec4(p*2.-1.,0,1);
vColor=vec4(.2,.3,.4,.7);vLightNormal=vec3(0,1,0);vIndirectLight=vec3(.4,.5,.6);
vSkyAmbient=vec4(.1,.2,.3,day);vForcedSun=vec3(.1);vSunTerm=vec4(.3,.2,.1,1);
vSunDirection=vec3(0,1,0);vSpecColor=vec4(.02,.03,.04,0);vUV0=vUV1=vec2(.5);vFogTC=fog;vWorldRel=world;}
'''
TRIANGLE='''#version 330 core
void main(){vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);gl_Position=vec4(p*2.-1.,0,1);}
'''


class Fixture:
    def __init__(self,current,baseline,sky):
        self.ctx=moderngl.create_standalone_context(backend='egl',require=330);self.programs={}
        for variant,text in [('current',current),('baseline',baseline)]:
            for material in ['Normal','Detail','Grass','Water']:
                self.programs[variant,material]=self.ctx.program(vertex_shader=VERTEX,fragment_shader=extract(text)['s_ps'+material+'GLSL'])
        literals=dict(re.findall(r'const char\* (sky\w+)\s*=\s*R"\((.*?)\)";',sky,re.S))
        self.sky=self.ctx.program(vertex_shader=literals['skyVS'],fragment_shader=literals['skyFS'])
        self.probe=self.ctx.program(vertex_shader=TRIANGLE,fragment_shader='#version 330 core\nuniform vec3 base,ray,sun;uniform float weight;out vec4 outputColor;\n'+helper(literals['skyFS'])+'\nvoid main(){outputColor=vec4(directionalAerialColor(base,ray,sun,weight),1);}')
        copies=[helper(extract(current)['s_ps'+m+'GLSL']) for m in ['Normal','Detail','Grass','Water']]+[helper(literals['skyFS'])]
        if len(set(copies))!=1:raise AssertionError('Mesh and sky directional helper literals differ')

    def render(self,p,uniforms,ps=None):
        ctx=self.ctx;resources=[]
        def keep(r):resources.append(r);return r
        try:
            for key,value in uniforms.items():
                if key in p:p[key].value=value
            if ps is not None:
                ubo=keep(ctx.buffer(ps.tobytes()));p['PSConstants'].binding=0;ubo.bind_to_uniform_block(0)
                for unit,name,color in [(0,'tex0',(.4,.5,.6,.8)),(1,'tex1',(.25,.5,.75,.5)),(3,'cloudTransmittance',(1.,))]:
                    if name in p:
                        tex=keep(ctx.texture((1,1),len(color),np.array(color,np.float32).tobytes(),dtype='f4'));tex.use(unit);p[name].value=unit
                if 'shadowMap' in p:
                    tex=keep(ctx.texture_array((1,1,1),1,np.array([1.],np.float32).tobytes(),dtype='f4'));tex.use(2);p['shadowMap'].value=2
            tex=keep(ctx.texture((8,8),4,dtype='f4'));fb=keep(ctx.framebuffer([tex]));fb.use();ctx.viewport=(0,0,8,8);fb.clear()
            vao=keep(ctx.vertex_array(p,[]));vao.render(vertices=3)
            return np.frombuffer(fb.read(viewport=(4,4,1,1),components=4,dtype='f4'),np.float32).copy()
        finally:
            for r in reversed(resources):r.release()

    def pixel(self,variant='current',material='Normal',world=(1000,0,0),sun=(.8,.6,0),weight=1,fog=.5,day=1,base=(.2,.3,.4),night=1):
        ps=np.zeros(124,np.float32);ps[:4]=(*base,1);ps[12:16]=(.8,.7,.9,.6);ps[16:20]=(.3,.2,-.5,0);ps[28:32]=(.2126,.7152,.0722,night);ps[120:124]=(*sun,weight)
        return self.render(self.programs[variant,material],dict(world=world,day=day,fog=fog),ps)

    def horizon(self,ray,sun,base,weight):
        return self.render(self.sky,dict(forwardRay=ray,rightRay=(0,0,0),upRay=(0,0,0),sunRay=sun,horizonColor=base,skyColor=(.7,.8,.9),sunColor=(1,.9,.7),clearSky=1,overlayWeight=.73,aerialWeight=weight))

    def close(self):
        for p in [*self.programs.values(),self.sky,self.probe]:p.release()
        self.ctx.release()


def main():
    p=argparse.ArgumentParser(description=__doc__);root=Path(__file__).resolve().parents[2]/'engine/PoseidonGL33'
    p.add_argument('--source',type=Path,default=root/'EngineGL33_Shaders.cpp');p.add_argument('--sky-source',type=Path,default=root/'EngineGL33_ExperimentalSky.cpp');p.add_argument('--baseline',type=Path,required=True);p.add_argument('--report',type=Path);a=p.parse_args()
    f=Fixture(a.source.read_text(),a.baseline.read_text(),a.sky_source.read_text());rows=[]
    def check(label,actual,expected,tolerance=2e-6):
        if not np.allclose(actual,expected,atol=tolerance,rtol=0,equal_nan=True):raise AssertionError((label,actual,expected))
        rows.append(label)
    try:
        for base in [(.2,.3,.4),(.5,.4,.2),(0,0,0),(1,1,1)]:
            for ray in [(1,0,0),(-1,0,0),(0,0,1),(0,0,-1),(0,1,0)]:
                for weight in [0,.25,1]:
                    sun=(.8,.6,0);actual=f.render(f.probe,dict(base=base,ray=ray,sun=sun,weight=weight))[:3]
                    check('independent directional oracle',actual,color_oracle(base,ray,sun,weight))
                    check('scattering luminance preservation',np.dot(actual,LUMA),np.dot(base,LUMA))
        for ray,sun in [((0,0,0),(.8,.6,0)),((1,0,0),(0,0,0)),((float('nan'),0,0),(.8,.6,0)),((1,0,0),(float('inf'),0,0))]:
            check('invalid direction identity',f.render(f.probe,dict(base=(.2,.3,.4),ray=ray,sun=sun,weight=1))[:3],[.2,.3,.4])
        for material in ['Normal','Detail','Grass','Water']:
            for radius in [0,120,200,350,1000]:
                for direction in [-1,1]:
                    world=(direction*radius,0,0)
                    for fog in [0,.3,1]:
                        for weight in [0,.5,1]:
                            scattering=color_oracle((.2,.3,.4),world,(.8,.6,0),weight*distance_weight(world))
                            got=f.pixel(material=material,world=world,fog=fog,weight=weight)
                            expected=f.pixel('baseline',material=material,world=world,fog=fog,base=scattering)
                            check('unchanged transmittance and foreground '+material,got,expected)
                            old=f.pixel('baseline',material=material,world=world,fog=fog)
                            check('surviving alpha '+material,got[3],old[3],0)
                            if fog==1 or radius<=120 or weight==0:check('nearfield/no fog/disabled identity '+material,got,old,0)
            for day in [0,.3,1]:
                for night in [0,1]:
                    # Runtime exclusions upload zero, rather than shader-guessing night from color.
                    check('zero-control night contract '+material,f.pixel(material=material,weight=0,day=day,night=night),f.pixel('baseline',material=material,day=day,night=night),0)
        for edge in [120,350]:check('distance fade continuity '+str(edge),f.pixel(world=(edge-.001,0,0)),f.pixel(world=(edge+.001,0,0)))
        for ray in [(1,0,0),(-1,0,0),(0,0,1),(0,0,-1)]:
            for weight in [0,.5,1]:
                sky=f.horizon(ray,(.8,.6,0),(.2,.3,.4),weight)
                terrain=f.pixel(world=tuple(x*1000 for x in ray),weight=weight,fog=0)
                check('exact matched analytic horizon',sky[:3],terrain[:3])
                check('sky overlay alpha preserved',sky[3],.73)
        report=dict(result='PASS',checks=len(rows),cases=rows,source_sha256={str(path):hashlib.sha256(path.read_bytes()).hexdigest() for path in [a.source,a.sky_source,a.baseline]},renderer={k:f.ctx.info.get(k) for k in ['GL_RENDERER','GL_VERSION','GL_VENDOR']},scope=__doc__,limits='Synthetic full fragment and helper tests; CPU eligibility/activation/upload gates separate. No engine registration, actual atmospheric art, continuous motion or hardware result.')
        if a.report:a.report.write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps(report,indent=2))
    finally:f.close()
if __name__=='__main__':main()
