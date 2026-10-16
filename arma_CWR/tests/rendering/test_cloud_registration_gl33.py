#!/usr/bin/env python3
"""Q diagnostic cloud layout/projection and actual-GL transmission tests.

No engine or assets loaded. Deterministic synthetic RNG checks layout refactoring
against extracted old source, not actual random-generator distribution. Synthetic
quad alpha tests do not establish actual cloud-to-terrain visual registration.
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
import numpy as np


def between(text,start,end):
    return text[text.index(start):text.index(end,text.index(start))]


def native_checks(root,legacy,compiler):
    old=legacy.read_text();source=(root/'PoseidonGL33/EngineGL33_CloudRegistration.cpp').read_text()
    speeds=between(old,'#define SP (2.0f)','        int xx, zz;')
    phase=between(old,'        float cPos = CloudsPosition();','        float height = CloudsBrightness();')
    offsets=between(old,'        int xMoveInt = toIntFloor(xMove);','        float posY = skyY[skyLevel] * height;')
    seed=between(old,'                int seedXZ = _randGen.GetSeed','                if (isHere >= N_CLOUDS)')
    placement=between(old,'                float xOffset = xMove','                // Point3 pos=Vector3')
    projection=between(source,'      float x=p.X()-input.towardSun.X()','      if(!std::isfinite(x)')
    cpp=r'''
#include <cmath>
#include <cstdint>
#include <Poseidon/World/Terrain/CloudInstanceLayout.hpp>
struct RNG { int GetSeed(int x,int z,int layer){return int((uint32_t(x)*73856093u ^ uint32_t(z)*19349663u ^ uint32_t(layer)*83492791u)&0x3fffffff);}
 float RandomValue(int s){uint32_t n=uint32_t(s);n^=n<<13;n^=n>>17;n^=n<<5;return (n&0xffffff)/16777216.f;} };
constexpr int SKY_LEVELS=3;
constexpr float SKY_GRID=6000,H_PI=3.14159265358979323846f;
int toIntFloor(float x){return int(std::floor(x));}
struct Info{float posX,posZ,azimut;};
extern "C" void layout(float position,int skyLevel,int xx,int zz,float*out){
 RNG _randGen;
 auto current=Poseidon::clouds::AtCell(_randGen,Poseidon::clouds::MakePhase(position,skyLevel),xx,zz,skyLevel);
 out[0]=current.type;out[1]=current.x;out[2]=current.z;out[3]=current.azimuth;
 auto CloudsPosition=[&](){return position;};Info info;
''' +speeds+phase+offsets+seed+placement+r'''
 out[4]=isHere;out[5]=info.posX;out[6]=info.posZ;out[7]=info.azimut;
}
struct V{float x,y,z;float X()const{return x;}float Y()const{return y;}float Z()const{return z;}};
struct Input{V towardSun,camera;};
extern "C" void project(float*xin,float*out){V p{xin[0],xin[1],xin[2]};Input input{{xin[3],xin[4],xin[5]},{0,xin[6],0}};
''' +projection+r'''
 out[0]=x;out[1]=z;
}
'''
    with tempfile.TemporaryDirectory(prefix='cwr-cloud-layout-') as directory:
        p=Path(directory);(p/'probe.cpp').write_text(cpp)
        subprocess.run(shlex.split(compiler)+['-std=c++17','-shared','-fPIC','-I',str(root),str(p/'probe.cpp'),'-o',str(p/'probe.so')],check=True,text=True)
        lib=ctypes.CDLL(str(p/'probe.so'));ptr=ctypes.POINTER(ctypes.c_float)
        lib.layout.argtypes=[ctypes.c_float,ctypes.c_int,ctypes.c_int,ctypes.c_int,ptr];lib.project.argtypes=[ptr,ptr]
        checks=0
        for phase_value in [-250.001,-250,-249.999,-125.001,-125,-124.999,-.001,0,.001,24.999,25,25.001,249.999,250,250.001,9999]:
            for layer in range(3):
                for x in [-21,-1,0,1,17]:
                    for z in [-17,-1,0,1,21]:
                        out=np.zeros(8,np.float32);lib.layout(phase_value,layer,x,z,out.ctypes.data_as(ptr))
                        if out[0]!=out[4] or (out[0]<4 and not np.array_equal(out[:4],out[4:])):raise AssertionError(('old/new enumeration',phase_value,layer,x,z,out))
                        checks+=1
        # Follow the same seeded cloud through exact phase-cell wraps.
        for layer,speed in enumerate([.04,.024,.02]):
            for boundary in [-2,-1,0,1,2]:
                phase0=boundary/speed-.0001;phase1=boundary/speed+.0001
                cells=[];positions=[]
                for value in [phase0,phase1]:
                    xs=np.float32(value)*np.float32(speed);zs=np.float32(value)*np.float32([.008,-.004,-.008][layer])
                    cx=int(np.floor(xs));cz=int(np.floor(zs));out=np.zeros(8,np.float32)
                    lib.layout(value,layer,cx+3,cz-2,out.ctypes.data_as(ptr));cells.append(out[0]);positions.append(out[1:4])
                if cells[0]!=cells[1] or np.max(abs(positions[1]-positions[0]))>.1:raise AssertionError('phase continuity')
                checks+=1
        for height in [2000,4000,6000]:
            for camera_height in [-50,0,400]:
                for sun in [(0,1,0),(.6,.8,0),(-.4,.5,.3)]:
                    point=np.array([100.,height,-200.]);sun=np.array(sun);ray_t=(height-camera_height)/sun[1]
                    expected=(point-ray_t*sun)[[0,2]]
                    values=np.array([*point,*sun,camera_height],np.float32);out=np.zeros(2,np.float32)
                    lib.project(values.ctypes.data_as(ptr),out.ctypes.data_as(ptr))
                    if not np.allclose(out,expected,rtol=0,atol=.002):raise AssertionError(('projection sign',out,expected))
                    # Receiver is on the same sun ray at a different height.
                    receiver=point-sun*321.
                    lookup=(receiver[[0,2]]-sun[[0,2]]/sun[1]*(receiver[1]-camera_height))
                    if not np.allclose(lookup,out,rtol=0,atol=.002):raise AssertionError('receiver height shear')
                    checks+=2
    return checks


VERTEX='''#version 330 core
uniform vec3 world;uniform float bearing;
out vec4 vColor;out vec3 vLightNormal;out vec3 vIndirectLight;
flat out vec4 vSkyAmbient;flat out vec3 vForcedSun;flat out vec4 vSunTerm;flat out vec3 vSunDirection;
out vec4 vSpecColor;out vec2 vUV0;out vec2 vUV1;out float vFogTC;out vec3 vWorldRel;
void main(){vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);gl_Position=vec4(p*2.-1.,0,1);
vColor=vec4(.2,.3,.4,.7);vLightNormal=vec3(0,1,0);vIndirectLight=vec3(.2);
vSkyAmbient=vec4(.1,.1,.1,1);vForcedSun=vec3(.05);vSunTerm=vec4(.15,.15,.15,bearing);
vSunDirection=vec3(0,1,0);vSpecColor=vec4(0);vUV0=vUV1=vec2(.5);vFogTC=1.;vWorldRel=world;}
'''


def gl_checks(root,baseline):
    ctx=moderngl.create_standalone_context(backend='egl',require=330);rows=[]
    text=(root/'PoseidonGL33/EngineGL33_CloudRegistration.cpp').read_text()
    literal=lambda name:re.search(r'const char\* '+name+r'=R"\((.*?)\)";',text,re.S).group(1)
    program=ctx.program(vertex_shader=literal('cloudVS'),fragment_shader=literal('cloudFS'))
    buf=ctx.buffer(np.array([-1,-1,0,0,3,-1,2,0,-1,3,0,2],np.float32).tobytes())
    vao=ctx.vertex_array(program,[(buf,'2f 2f','position','uv')]);target=ctx.texture((8,8),1,dtype='f2');fbo=ctx.framebuffer([target]);fbo.use();ctx.viewport=(0,0,8,8)
    ctx.enable(moderngl.BLEND);ctx.blend_func=(moderngl.ZERO,moderngl.ONE_MINUS_SRC_ALPHA);ctx.blend_equation=moderngl.FUNC_ADD
    try:
        for intensity in [0,.3,1]:
            for alpha in [0,.25,.5,1]:
                for weather in [0,.4,1]:
                    # Synthetic AI88 channel contract: intensity R, alpha G, swizzle RRRG.
                    tex=ctx.texture((1,1),2,np.array([intensity,alpha],np.float32).tobytes(),dtype='f4');tex.swizzle='RRRG';tex.use(0);program['authoredCloud'].value=0;program['weatherAlpha'].value=weather
                    for overlap in [1,2,4]:
                        fbo.clear(1,1,1,1)
                        for _ in range(overlap):vao.render(vertices=3)
                        value=float(np.frombuffer(fbo.read(viewport=(4,4,1,1),components=1,dtype='f4'),np.float32)[0]);expected=(1-alpha*weather)**overlap
                        if abs(value-expected)>.002:raise AssertionError(('alpha product',intensity,alpha,weather,overlap,value,expected))
                        rows.append('AI88 product/intensity independence')
                    tex.release()
        # Asymmetric authored UV alpha detects accidental reflection/axis swaps.
        alpha=np.array([0.,.25,.5,1.],np.float32).reshape(2,2)
        pixels=np.stack([np.full((2,2),.7,np.float32),alpha],axis=-1)
        tex=ctx.texture((2,2),2,pixels.tobytes(),dtype='f4');tex.swizzle='RRRG'
        tex.filter=(moderngl.NEAREST,moderngl.NEAREST);tex.repeat_x=False;tex.repeat_y=False
        tex.use(0);program['weatherAlpha'].value=1.;fbo.use();fbo.clear(1,1,1,1);vao.render(vertices=3)
        for x,y,expected in [(1,1,1.),(6,1,.75),(1,6,.5),(6,6,0.)]:
            got=float(np.frombuffer(fbo.read(viewport=(x,y,1,1),components=1,dtype='f4'),np.float32)[0])
            if abs(got-expected)>.002:raise AssertionError(('authored UV alpha orientation',x,y,got,expected))
            rows.append('asymmetric UV alpha orientation')
        # Match the authored ClampU|ClampV policy with a dedicated linear sampler.
        # Outside-UV probes distinguish CLAMP_TO_EDGE from REPEAT decisively.
        sampler=ctx.sampler(texture=tex,filter=(moderngl.LINEAR,moderngl.LINEAR),repeat_x=False,repeat_y=False)
        sampler.use(0)
        edge_cases=[(-.25,.25,1.),(-.25,.75,.5),(1.25,.25,.75),(1.25,.75,0.),
                    (.25,-.25,1.),(.75,-.25,.75),(.25,1.25,.5),(.75,1.25,0.)]
        for u,v,expected in edge_cases:
            vertices=np.array([-1,-1,u,v,3,-1,u,v,-1,3,u,v],np.float32)
            edge_buf=ctx.buffer(vertices.tobytes());edge_vao=ctx.vertex_array(program,[(edge_buf,'2f 2f','position','uv')])
            fbo.clear(1,1,1,1);edge_vao.render(vertices=3)
            got=float(np.frombuffer(fbo.read(viewport=(4,4,1,1),components=1,dtype='f4'),np.float32)[0])
            edge_vao.release();edge_buf.release()
            if abs(got-expected)>.002:raise AssertionError(('authored clamp edge',u,v,got,expected))
            rows.append('authored clamp outside UV edge')
        sampler.clear(0);sampler.release()
        tex.release()
        ctx.disable(moderngl.BLEND)
        def extract(s):return dict(re.findall(r'static const char (s_\w+GLSL)\[\] = R"\((.*?)\)";',s,re.S))
        sources={'current':extract((root/'PoseidonGL33/EngineGL33_Shaders.cpp').read_text()),'baseline':extract(baseline.read_text())}
        programs={(variant,material):ctx.program(vertex_shader=VERTEX,fragment_shader=source['s_ps'+material+'GLSL']) for variant,source in sources.items() for material in ['Normal','Detail','Grass']}
        def pixel(variant,material,enabled,world=(0,0,0),bearing=1,transmission=.25):
            p=programs[variant,material];resources=[]
            try:
                for k,v in [('world',world),('bearing',bearing)]:
                    if k in p:p[k].value=v
                ps=np.zeros(120,np.float32);ps[12:16]=1;ps[28:32]=(.3,.6,.1,1);ps[112:116]=(.5,.5,1/4096,enabled);ps[116:120]=(.75,-.25,0,0)
                ubo=ctx.buffer(ps.tobytes());resources.append(ubo);p['PSConstants'].binding=0;ubo.bind_to_uniform_block(0)
                for unit,name,value in [(0,'tex0',[.3,.4,.5,.8]),(1,'tex1',[.5,.5,.5,.5]),(3,'cloudTransmittance',[transmission])]:
                    if name in p:
                        tex=ctx.texture((1,1),len(value),np.array(value,np.float32).tobytes(),dtype='f4');resources.append(tex);tex.use(unit);p[name].value=unit
                if 'shadowMap' in p:
                    tex=ctx.texture_array((1,1,1),1,np.array([.5],np.float32).tobytes(),dtype='f4');resources.append(tex);tex.use(2);p['shadowMap'].value=2
                target=ctx.texture((8,8),4,dtype='f4');resources.append(target);fb=ctx.framebuffer([target]);resources.append(fb);fb.use();v=ctx.vertex_array(p,[]);resources.append(v);v.render(vertices=3)
                return np.frombuffer(fb.read(viewport=(4,4,1,1),components=4,dtype='f4'),np.float32).copy()
            finally:
                for r in reversed(resources):r.release()
        for material in ['Normal','Detail','Grass']:
            for bearing in [0,1]:
                for world in [(0,0,0),(5000,0,0),(-5000,0,0),(0,0,5000),(0,0,-5000),(750,1000,-250),(2048,0,0),(-2048,0,0),(2048.1,0,0),(-2048.1,0,0)]:
                    old=pixel('baseline',material,0,world,bearing)
                    for enabled in [0,1]:
                        got=pixel('current',material,enabled,world,bearing)
                        inside=abs(world[0]-.75*world[1])<=2048 and abs(world[2]+.25*world[1])<=2048
                        expected=old.copy()
                        if enabled and material=='Detail' and bearing and inside:expected[:3]=.25
                        if not np.allclose(got,expected,rtol=0,atol=1e-6):raise AssertionError(('receiver',material,bearing,world,enabled,got,expected))
                        rows.append('receiver diagnostic/disabled/excluded/outside/shear')
        for p in programs.values():p.release()
        return len(rows),{k:ctx.info.get(k) for k in ['GL_RENDERER','GL_VERSION','GL_VENDOR']}
    finally:
        for r in [fbo,target,vao,buf,program]:r.release()
        ctx.release()


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--engine-root',type=Path,default=Path(__file__).resolve().parents[2]/'engine',help='Complete current engine source tree');p.add_argument('--legacy-landscape',required=True,type=Path);p.add_argument('--baseline',required=True,type=Path);p.add_argument('--compiler',default=os.environ.get('CXX','clang++'));p.add_argument('--report',type=Path);a=p.parse_args()
    cloud_source=(a.engine_root/'PoseidonGL33/EngineGL33_CloudRegistration.cpp').read_text()
    guards={
        'authored_clamp_u':'glSamplerParameteri(_cloudSampler,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE)' in cloud_source,
        'authored_clamp_v':'glSamplerParameteri(_cloudSampler,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE)' in cloud_source,
        'reject_nonclamped_faces':(
            'const auto sampler = Poseidon::render::BuildRenderPassDescriptor(Poseidon::render::SplitLegacy(face.Special())).sampler;' in cloud_source
            and 'if(face.N()!=4 || !sampler.clampU || !sampler.clampV || !texture || !_textBank){complete=false;break;}' in cloud_source),
    }
    if not all(guards.values()):raise AssertionError(('authored sampler source guards',guards))
    native=native_checks(a.engine_root,a.legacy_landscape,a.compiler);checks,driver=gl_checks(a.engine_root,a.baseline)
    files=[a.legacy_landscape,a.baseline,a.engine_root/'PoseidonGL33/EngineGL33_CloudRegistration.cpp',a.engine_root/'PoseidonGL33/EngineGL33_Shaders.cpp',a.engine_root/'Poseidon/World/Terrain/CloudInstanceLayout.hpp']
    report=dict(result='PASS',source_guards=guards,native_checks=native,GL_checks=checks,renderer=driver,source_sha256={str(f):hashlib.sha256(f.read_bytes()).hexdigest() for f in files},scope=__doc__,limits='No actual cloud assets/registration, engine queues, missing-texture/failure execution or hardware performance. State restoration tested separately.')
    if a.report:a.report.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))
if __name__=='__main__':main()
