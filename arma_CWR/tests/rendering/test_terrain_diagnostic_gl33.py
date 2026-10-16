#!/usr/bin/env python3
"""P diagnostic-only descriptor and actual-fragment tests, no terrain beauty.

Compile exact frozen descriptor headers without mocks. Render extracted GLSL
with synthetic vertex outputs and controlled detail alpha, including bitwise
mode-off parity against a preserved shader baseline. Source wiring/reset guards
are not execution of engine ownership scopes, draw queues or real terrain data.
Requires C++17 compiler, numpy, moderngl and EGL/OpenGL 3.3. No engine launch.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import struct
import subprocess
import tempfile

import moderngl
import numpy as np

DESCRIPTOR_TEST = r'''
#include <Poseidon/Graphics/Rendering/BuildRenderPassDescriptor.hpp>
#include <iostream>
using namespace Poseidon::render;
int main() {
 const Backend bits[]={Backend::DetailTexture,Backend::SpecularTexture,Backend::GrassTexture,
  Backend::IsShadow,Backend::IsLight,Backend::IsWater,Backend::IsAlphaFog,
  Backend::IsAlpha,Backend::IsTransparent,Backend::NoZBuf,Backend::NoZWrite};
 const PassKindHint hints[]={PassKindHint::None,PassKindHint::Cockpit,PassKindHint::ScreenSpace3D};
 unsigned checks=0,eligible=0;
 for(unsigned mask=0;mask<(1u<<11);++mask)
 for(int owner=0;owner<2;++owner) for(int world=0;world<2;++world)
 for(int multi=0;multi<2;++multi) for(auto hint:hints)
 for(int sunOff=0;sunOff<2;++sunOff) for(int surface=0;surface<2;++surface) {
  LegacySpec spec{};
  for(int i=0;i<11;++i) if(mask&(1u<<i)) spec.backend=spec.backend|bits[i];
  if(sunOff) spec.material=Material::DisableSun;
  if(surface) spec.routing=Routing::OnSurface;
  BuildContext context;context.terrainBaseOwner=owner;context.isIn3DPass=world;
  context.isMultitexturing=multi;context.passKindHint=hint;
  auto d=BuildRenderPassDescriptor(spec,context);
  // Independent truth table: only the Detail bit may be set among these eleven.
  bool expected=owner && world && multi && hint==PassKindHint::None && !sunOff && !surface && mask==1;
  ++checks;if(d.terrainDetail!=expected) {std::cerr<<"classification mask="<<mask;return 1;}
  eligible+=d.terrainDetail;
  auto altered=d;altered.terrainDetail=!d.terrainDetail;
  ++checks;if(d==altered || !(d!=altered)) {std::cerr<<"equality omitted provenance";return 2;}
  context.terrainBaseOwner=false;
  ++checks;if(BuildRenderPassDescriptor(spec,context).terrainDetail) {std::cerr<<"nonowner reset";return 3;}
 }
 if(eligible!=1) {std::cerr<<"positive control missing";return 4;}
 std::cout<<checks<<" "<<eligible;
}
'''
VERTEX = '''#version 330 core
uniform float normalBearing, day, fog;
out vec4 vColor;out vec3 vLightNormal;out vec3 vIndirectLight;
flat out vec4 vSkyAmbient;flat out vec3 vForcedSun;flat out vec4 vSunTerm;flat out vec3 vSunDirection;
out vec4 vSpecColor;out vec2 vUV0;out vec2 vUV1;out float vFogTC;out vec3 vWorldRel;
void main(){
 vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);gl_Position=vec4(p*2.-1.,0,1);
 vColor=vec4(.2,.3,.4,.7);vLightNormal=vec3(0,0,1);vIndirectLight=vec3(.2);
 vSkyAmbient=vec4(.1,.1,.1,day);vForcedSun=vec3(.05);vSunTerm=vec4(.15,.15,.15,normalBearing);
 vSunDirection=vec3(0,0,1);vSpecColor=vec4(.01,.02,.03,0);
 vUV0=vUV1=vec2(.5);vFogTC=fog;vWorldRel=vec3(0);
}
'''
FRAGMENTS={'normal':'s_psNormalGLSL','detail':'s_psDetailGLSL','grass':'s_psGrassGLSL'}


def extract(source):
    return dict(re.findall(r'static const char (s_\w+GLSL)\[\] = R"\((.*?)\)";',source,re.S))


def descriptor_checks(root,compiler):
    with tempfile.TemporaryDirectory(prefix='cwr-terrain-descriptor-') as directory:
        p=Path(directory);(p/'test.cpp').write_text(DESCRIPTOR_TEST)
        subprocess.run(shlex.split(compiler)+['-std=c++17','-I',str(root),str(p/'test.cpp'),'-o',str(p/'test')],
                       check=True,capture_output=True,text=True)
        result=subprocess.run([str(p/'test')],check=True,capture_output=True,text=True)
        checks,positive=map(int,result.stdout.split())
        return dict(checks=checks,positive_configurations=positive,scope='Exact header execution; no engine queues or GL calls.')


def source_guards(root):
    paths={
        'queue':'PoseidonGL33/EngineGL33_Queue.cpp', 'mesh':'PoseidonGL33/EngineGL33_Mesh.cpp',
        'state':'PoseidonGL33/EngineGL33_State.cpp','render':'Poseidon/Graphics/Core/RenderState.hpp',
        'terrain':'Poseidon/World/Terrain/LandscapeRender.cpp','shader':'PoseidonGL33/EngineGL33_Shaders.cpp',
    }
    text={key:(root/path).read_text() for key,path in paths.items()}
    historical_reset = bool(re.search(r'd\.terrainDetail\s*&&\s*vertexInput\s*==\s*PipelineVertexInput::Mesh\s*\?\s*float\(terrainDiagnostic\)\s*:\s*0\.0f', text['state']))
    factored_reset = (
        'const bool terrainEligible = d.terrainDetail && vertexInput == PipelineVertexInput::Mesh;' in text['state']
        and 'terrainEligible ? float(terrainDiagnostic) : 0.0f,' in text['state']
        and 'terrainEligible && terrainRelief ? (terrainRelief == 1 ? .04f : .08f) : 0.0f,' in text['state'])
    checks={
        'captured_owner_mesh_only':bool(re.search(r'ctx\.terrainBaseOwner\s*=\s*vertexInput\s*==\s*PipelineVertexInput::Mesh\s*&&\s*_currentDrawItem\.terrainBaseOwner',text['queue'])),
        'capture_from_explicit_owner':'_currentDrawItem.terrainBaseOwner = GetTerrainBaseDraw();' in text['mesh'],
        'owner_defaults_false':'bool terrainBaseOwner = false;' in text['render'],
        'resolved_defaults_false':'bool terrainDetail = false;' in text['render'],
        'producer_scope_restores':'~TerrainBaseScope() { engine.SetTerrainBaseDraw(previous); }' in text['terrain'],
        'slot27_upload':'UploadPSConstant(27, terrainControl);' in text['state'],
        'ineligible_explicit_zero':historical_reset or factored_reset,
        'exact_mode1':'std::strcmp(value, "1") == 0' in text['state'],
        'exact_mode2':'std::strcmp(value, "2") == 0' in text['state'],
    }
    shaders=extract(text['shader'])
    for name,key in FRAGMENTS.items():
        checks[name+'_slot27']='vec4 terrainControl;' in shaders[key]
        checks[name+'_diagnostic_scope']=('fragColor.rgb = terrainControl.x' in shaders[key])==(name=='detail')
    failed=[k for k,v in checks.items() if not v]
    if failed:raise AssertionError(f'Source guards failed: {failed}')
    return checks


class Fixture:
    def __init__(self,current,baseline):
        self.ctx=moderngl.create_standalone_context(backend='egl',require=330)
        self.shaders={'current':extract(current),'baseline':extract(baseline)};self.programs={}

    def pixel(self,variant,fragment,mode,detail_alpha,normal_bearing,day,fog):
        key=(variant,fragment)
        if key not in self.programs:
            self.programs[key]=self.ctx.program(vertex_shader=VERTEX,fragment_shader=self.shaders[variant][FRAGMENTS[fragment]])
        program=self.programs[key]
        for name,value in [('normalBearing',normal_bearing),('day',day),('fog',fog)]:program[name].value=value
        ps=np.zeros(112,dtype='f4')
        def slot(i,data):ps[i*4:i*4+len(data)]=data
        slot(0,(.1,.2,.3,1));slot(3,(1,1,1,1));slot(5,(0,0,0,.75));slot(6,(0,0,0,.35));slot(7,(.3,.6,.1,1));slot(27,(mode,0,0,0))
        resources=[]
        try:
            ubo=self.ctx.buffer(ps.tobytes());resources.append(ubo);program['PSConstants'].binding=0;ubo.bind_to_uniform_block(0)
            for unit,name,rgba in [(0,'tex0',(.4,.5,.6,.8)),(1,'tex1',(.3,.4,.5,detail_alpha))]:
                if name in program:
                    tex=self.ctx.texture((1,1),4,struct.pack('4f',*rgba),dtype='f4');resources.append(tex);tex.use(unit);program[name].value=unit
            depth=self.ctx.texture_array((1,1,1),1,struct.pack('f',.5),dtype='f4');resources.append(depth);depth.use(2);program['shadowMap'].value=2
            target=self.ctx.texture((8,8),4,dtype='f4');resources.append(target)
            fbo=self.ctx.framebuffer([target]);resources.append(fbo);fbo.use();self.ctx.viewport=(0,0,8,8)
            vao=self.ctx.vertex_array(program,[]);resources.append(vao);fbo.clear();vao.render(vertices=3)
            return np.frombuffer(fbo.read(viewport=(4,4,1,1),components=4,dtype='f4'),np.float32).copy()
        finally:
            for resource in reversed(resources):resource.release()

    def close(self):
        for program in self.programs.values():program.release()
        self.ctx.release()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--engine-root',type=Path,required=True,help='Coherent source root containing Poseidon and PoseidonGL33')
    parser.add_argument('--baseline',type=Path,required=True,help='Preserved pre-diagnostic EngineGL33_Shaders.cpp')
    parser.add_argument('--compiler',default=os.environ.get('CXX','clang++'))
    parser.add_argument('--report',type=Path)
    args=parser.parse_args();root=args.engine_root
    shader=root/'PoseidonGL33/EngineGL33_Shaders.cpp';current=shader.read_bytes();baseline=args.baseline.read_bytes()
    descriptor=descriptor_checks(root,args.compiler);guards=source_guards(root)
    f=Fixture(current.decode(),baseline.decode());rows=[]
    try:
        for fragment in FRAGMENTS:
            for alpha in [0,.1,.5,.9,1]:
                for normal_bearing in [0,1]:
                    for day in [0,1]:
                        for fog in [0,1]:
                            old=f.pixel('baseline',fragment,0,alpha,normal_bearing,day,fog)
                            for mode in [0,1,2]:
                                actual=f.pixel('current',fragment,mode,alpha,normal_bearing,day,fog)
                                if mode and fragment=='detail' and normal_bearing:
                                    expected=np.array(([0,1,1] if mode==1 else [alpha]*3)+[float(old[3])])
                                    if not np.allclose(actual,expected,atol=1e-6,rtol=0):raise AssertionError(f'Diagnostic mismatch {fragment}/{mode}/{alpha}: {actual} != {expected}')
                                elif not np.array_equal(actual,old):
                                    raise AssertionError(f'Disabled/excluded path changed: {fragment}/{mode}/{alpha}/{normal_bearing}/{day}/{fog}')
                                rows.append(dict(fragment=fragment,mode=mode,detail_alpha=alpha,normal_bearing=normal_bearing,day=day,fog=fog,rgba=actual.tolist(),result='PASS'))
        files=[shader]+[root/'Poseidon/Graphics/Rendering'/name for name in ['RenderFlags.hpp','RenderPassDescriptor.hpp','BuildRenderPassDescriptor.hpp']]
        report=dict(result='PASS',descriptor=descriptor,source_guards=guards,GL_checks=len(rows),results=rows,
                    source_sha256={str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for p in files},
                    baseline_sha256=hashlib.sha256(baseline).hexdigest(),renderer={k:f.ctx.info.get(k) for k in ['GL_RENDERER','GL_VERSION','GL_VENDOR']},
                    scope=__doc__,limits='No actual terrain texture statistics, marker eligibility images, beauty effect, cutout rasterization or hardware performance tested.')
        output=json.dumps(report,indent=2)
        if args.report:args.report.write_text(output+'\n')
        print(output)
    finally:f.close()


if __name__=='__main__':main()
