#include <Poseidon/Graphics/Rendering/BuildRenderPassDescriptor.hpp>
#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <PoseidonGL33/TextureGL33.hpp>
#include <Poseidon/Graphics/Rendering/Shape/Shape.hpp>
#include <Poseidon/Graphics/Core/GLDepthStencilState.hpp>
#include <Poseidon/Graphics/Core/GLBlendState.hpp>
#include <Poseidon/Graphics/Core/GLPipelineState.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>
#include <glad/gl.h>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace {
constexpr int Resolution=512;
const char* cloudVS=R"(#version 330 core
layout(location=0) in vec2 position;
layout(location=1) in vec2 uv;
out vec2 texCoord;
void main(){texCoord=uv;gl_Position=vec4(position,0,1);}
)";
const char* cloudFS=R"(#version 330 core
in vec2 texCoord;
uniform sampler2D authoredCloud;
uniform float weatherAlpha;
out vec4 transmission;
void main(){float opacity=clamp(texture(authoredCloud,texCoord).a*weatherAlpha,0,1);transmission=vec4(0,0,0,opacity);}
)";
GLuint compileCloud(GLenum type,const char* text) {
 GLuint s=glCreateShader(type);glShaderSource(s,1,&text,nullptr);glCompileShader(s);
 GLint ok=0;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
 if(!ok){char log[1024]={};glGetShaderInfoLog(s,sizeof(log),nullptr,log);LOG_ERROR(Graphics,"Q cloud shader failed: {}",log);glDeleteShader(s);return 0;}return s;
}
struct CloudVertex {float x,z,u,v;};
struct Batch {GLuint texture;int first,count;};
// Scoped binding ownership includes allocation failures, distinct READ/DRAW FBOs,
// texture loading and both unit0 (source) and unit3 (receiver map).
struct Bindings {
 GLint read,draw,program,vao,array,active,viewport[4],polygon[2],textures[3],samplers[3];
 static constexpr GLenum caps[6]={GL_CULL_FACE,GL_SCISSOR_TEST,GL_SAMPLE_ALPHA_TO_COVERAGE,GL_COLOR_LOGIC_OP,GL_RASTERIZER_DISCARD,GL_FRAMEBUFFER_SRGB};
 GLboolean enabled[6],clip[8];int clips=0;
 Bindings(){
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&read);glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&draw);
  glGetIntegerv(GL_CURRENT_PROGRAM,&program);glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&vao);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&array);glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
  glGetIntegerv(GL_VIEWPORT,viewport);glGetIntegerv(GL_POLYGON_MODE,polygon);
  for(int i=0;i<3;++i){int unit=i==0?0:(i==1?3:7);glActiveTexture(GL_TEXTURE0+unit);glGetIntegerv(GL_TEXTURE_BINDING_2D,&textures[i]);glGetIntegeri_v(GL_SAMPLER_BINDING,unit,&samplers[i]);}
  for(int i=0;i<6;++i){enabled[i]=glIsEnabled(caps[i]);glDisable(caps[i]);}
  glGetIntegerv(GL_MAX_CLIP_DISTANCES,&clips);clips=std::min(clips,8);
  for(int i=0;i<clips;++i){clip[i]=glIsEnabled(GL_CLIP_DISTANCE0+i);glDisable(GL_CLIP_DISTANCE0+i);}
 }
 ~Bindings(){
  glBindFramebuffer(GL_READ_FRAMEBUFFER,read);glBindFramebuffer(GL_DRAW_FRAMEBUFFER,draw);
  glUseProgram(program);glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,array);
  glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);glPolygonMode(GL_FRONT_AND_BACK,polygon[0]);
  for(int i=0;i<3;++i){int unit=i==0?0:(i==1?3:7);glActiveTexture(GL_TEXTURE0+unit);glBindTexture(GL_TEXTURE_2D,textures[i]);glBindSampler(unit,samplers[i]);}
  glActiveTexture(active);
  for(int i=0;i<6;++i){if(enabled[i])glEnable(caps[i]);else glDisable(caps[i]);}
  for(int i=0;i<clips;++i){if(clip[i])glEnable(GL_CLIP_DISTANCE0+i);else glDisable(GL_CLIP_DISTANCE0+i);}
 }
};
}

bool EngineGL33::CloudRegistrationEnabled() const {
 static const bool enabled=[] {const char* p=std::getenv("CWR_CLOUD_REGISTRATION");return p && std::strcmp(p,"1")==0;}();
 return enabled;
}
void EngineGL33::DestroyCloudRegistration(){
 if (_cloudSampler || _cloudMapSampler) glBindSampler(3,0);
 if(_cloudMap){GL33Bind::OnTexDeleted(_cloudMap);glDeleteTextures(1,&_cloudMap);}if(_cloudFbo)glDeleteFramebuffers(1,&_cloudFbo);
 if(_cloudProgram)glDeleteProgram(_cloudProgram);if(_cloudVao){GL33Bind::OnVaoDeleted(_cloudVao);glDeleteVertexArrays(1,&_cloudVao);}
 if(_cloudVbo)glDeleteBuffers(1,&_cloudVbo);if(_cloudSampler)glDeleteSamplers(1,&_cloudSampler);if(_cloudMapSampler)glDeleteSamplers(1,&_cloudMapSampler);
 _cloudMap=_cloudFbo=_cloudProgram=_cloudVao=_cloudVbo=_cloudSampler=_cloudMapSampler=0;_cloudValid=false;_cloudFailed=false;
}
void EngineGL33::PrepareCloudRegistration(const CloudRegistrationInput& input){
 if (_cloudValid) InvalidatePipelineCache();
 _cloudValid=false;
 if(!CloudRegistrationEnabled() || _cloudFailed) return;
 if(!input.complete || input.count<0 || input.count>512 || input.extent!=4096 || input.towardSun.Y()<=.15f){
  static bool logged=false;if(!logged){LOG_WARN(Graphics,"Q cloud map unsupported/incomplete input; normal rendering retained");logged=true;}return;
 }
 FlushQueues();
 bool success=false;
 {
  Bindings bindings;
  Poseidon::render::depthstencil::ScopedBackground depth;
  Poseidon::render::pipeline::ScopedRgbOnly color;
  Poseidon::render::blend::ScopedAlphaOverlay blend;
  Poseidon::render::blend::Shadow(); // destination *= 1 - authored alpha
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glActiveTexture(GL_TEXTURE0);
  // No GL allocation occurs until input coverage has been validated.
  if(!_cloudProgram){
   GLuint vs=compileCloud(GL_VERTEX_SHADER,cloudVS),fs=compileCloud(GL_FRAGMENT_SHADER,cloudFS);
   if(vs&&fs){_cloudProgram=glCreateProgram();glAttachShader(_cloudProgram,vs);glAttachShader(_cloudProgram,fs);glLinkProgram(_cloudProgram);GLint ok=0;glGetProgramiv(_cloudProgram,GL_LINK_STATUS,&ok);if(!ok){glDeleteProgram(_cloudProgram);_cloudProgram=0;}}
   if(vs)glDeleteShader(vs);if(fs)glDeleteShader(fs);
   if(_cloudProgram){
    glGenTextures(1,&_cloudMap);glBindTexture(GL_TEXTURE_2D,_cloudMap);
    glTexImage2D(GL_TEXTURE_2D,0,GL_R16F,Resolution,Resolution,0,GL_RED,GL_FLOAT,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glGenFramebuffers(1,&_cloudFbo);glBindFramebuffer(GL_FRAMEBUFFER,_cloudFbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,_cloudMap,0);
    glGenVertexArrays(1,&_cloudVao);glGenBuffers(1,&_cloudVbo);glGenSamplers(1,&_cloudSampler);
    glSamplerParameteri(_cloudSampler,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glSamplerParameteri(_cloudSampler,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    // Authored cloud faces require ClampU|ClampV; reject other policies below.
    glSamplerParameteri(_cloudSampler,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glSamplerParameteri(_cloudSampler,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glSamplerParameteri(_cloudSampler,GL_TEXTURE_COMPARE_MODE,GL_NONE);
    glGenSamplers(1,&_cloudMapSampler);
    glSamplerParameteri(_cloudMapSampler,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glSamplerParameteri(_cloudMapSampler,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glSamplerParameteri(_cloudMapSampler,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glSamplerParameteri(_cloudMapSampler,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glSamplerParameteri(_cloudMapSampler,GL_TEXTURE_COMPARE_MODE,GL_NONE);
   }
  }
  if(!_cloudProgram || !_cloudMap || !_cloudFbo || !_cloudVao || !_cloudVbo || !_cloudSampler || !_cloudMapSampler){_cloudFailed=true;}
  else {
   glBindFramebuffer(GL_FRAMEBUFFER,_cloudFbo);
   if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE) _cloudFailed=true;
   else {
    std::vector<CloudVertex> vertices;std::vector<Batch> batches;
    bool complete=true;
    for(int i=0;i<input.count && complete;++i){
     const auto& instance=input.instances[i];const Shape* shape=instance.shape;
     if(!shape || shape->NFaces()!=1 || shape->NPos()!=4){complete=false;break;}
     const Poly& face=shape->Face(shape->BeginFaces());
     TextureGL33* texture=static_cast<TextureGL33*>(face.GetTexture());
     const auto sampler = Poseidon::render::BuildRenderPassDescriptor(Poseidon::render::SplitLegacy(face.Special())).sampler;
     if(face.N()!=4 || !sampler.clampU || !sampler.clampV || !texture || !_textBank){complete=false;break;}
     GL33Bind::Invalidate(); // Raw allocation/bindings above must not fool upload-unit cache.
     _textBank->UseMipmap(texture,0,0);GLuint handle=texture->GetHandle();
     if(!handle){complete=false;break;}
     Batch batch{handle,static_cast<int>(vertices.size()),6};
     static constexpr int indices[6]={0,1,2,0,2,3};
     for(int j:indices){int index=face.GetVertex(j);if(index<0 || index>=shape->NPos()){complete=false;break;}
      Vector3 p=instance.transform.FastTransform(shape->Pos(index));const auto& uv=shape->UV(index);
      float x=p.X()-input.towardSun.X()*(p.Y()-input.camera.Y())/input.towardSun.Y();
      float z=p.Z()-input.towardSun.Z()*(p.Y()-input.camera.Y())/input.towardSun.Y();
      if(!std::isfinite(x)||!std::isfinite(z)||!std::isfinite(uv.u)||!std::isfinite(uv.v)){complete=false;break;}
      vertices.push_back({(x-input.centreX)*2/input.extent,(z-input.centreZ)*2/input.extent,uv.u,uv.v});
     }
     batches.push_back(batch);
    }
    if (!complete) {
     static bool warned=false;
     if(!warned){LOG_WARN(Graphics,"Q cloud map rejected: unsupported face/UV or missing texture; normal rendering retained");warned=true;}
    }
    if(complete){
     glViewport(0,0,Resolution,Resolution);const float clear[4]={1,1,1,1};glClearBufferfv(GL_COLOR,0,clear);
     glUseProgram(_cloudProgram);glUniform1i(glGetUniformLocation(_cloudProgram,"authoredCloud"),0);
     glUniform1f(glGetUniformLocation(_cloudProgram,"weatherAlpha"),input.alpha);
     glBindVertexArray(_cloudVao);glBindBuffer(GL_ARRAY_BUFFER,_cloudVbo);
     glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(CloudVertex),vertices.data(),GL_STREAM_DRAW);
     glEnableVertexAttribArray(0);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(CloudVertex),nullptr);
     glEnableVertexAttribArray(1);glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(CloudVertex),reinterpret_cast<void*>(2*sizeof(float)));
     glActiveTexture(GL_TEXTURE0);glBindSampler(0,_cloudSampler);
     for(const auto& b:batches){glBindTexture(GL_TEXTURE_2D,b.texture);glDrawArrays(GL_TRIANGLES,b.first,b.count);}
     _cloudLookup[0]=(input.camera.X()-input.centreX)/input.extent+.5f;
     _cloudLookup[1]=(input.camera.Z()-input.centreZ)/input.extent+.5f;
     _cloudLookup[2]=1/input.extent;_cloudLookup[3]=1;
     _cloudProjection[0]=input.towardSun.X()/input.towardSun.Y();_cloudProjection[1]=input.towardSun.Z()/input.towardSun.Y();
     _cloudProjection[2]=_cloudProjection[3]=0;success=true;
     static int logged=0;static float phase=-100000,cy=-100000,cx=-100000,cz=-100000,sy=-1,alpha=-1;
     if(logged<64 && (std::abs(phase-input.phase)>=1 || std::abs(cy-input.camera.Y())>1 || cx!=input.centreX || cz!=input.centreZ || std::abs(sy-input.towardSun.Y())>.001f || std::abs(alpha-input.alpha)>.01f)){
      LOG_INFO(Graphics,"Q authored cloud map: instances={}, vertices={}, phase={}, alpha={}, sun=({},{},{}), centre=({},{}), referenceY={}, extent=4096, res=512; diagnostic only",input.count,vertices.size(),input.phase,input.alpha,input.towardSun.X(),input.towardSun.Y(),input.towardSun.Z(),input.centreX,input.centreZ,input.camera.Y());phase=input.phase;cy=input.camera.Y();cx=input.centreX;cz=input.centreZ;sy=input.towardSun.Y();alpha=input.alpha;++logged;
     }
    }
   }
  }
 }
 _cloudValid=success;GL33Bind::Invalidate();InvalidatePipelineCache();_pixelShaderSel=PSNone;
}
void EngineGL33::ApplyCloudRegistration(bool terrain){
 if(!CloudRegistrationEnabled()) return;
 float lookup[4]={_cloudLookup[0],_cloudLookup[1],_cloudLookup[2],terrain&&_cloudValid?1.0f:0.0f};
 UploadPSConstant(28,lookup);UploadPSConstant(29,_cloudProjection);
 GL33Bind::Tex2D(3,_cloudValid?_cloudMap:0);glBindSampler(3,_cloudValid?_cloudMapSampler:0);GL33Bind::ActiveUnit(0);
}
