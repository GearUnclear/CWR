#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <cmath>
#include <cstdio>
#include <initializer_list>
#include <algorithm>
using namespace Poseidon::remaster;
const float kPi=3.14159265f;
Rgb Face(const LightRig& rig, float e, float off){ float c=std::cos(off*kPi/180); float st=std::max(std::cos(e*kPi/180)*c,0.f); float t=0.5f+0.5f*c;
 return rig.sun*st + Mix(rig.skyHorizon, rig.skyHorizonSun, t);}
float Sh(float x,float ex){ return ToDisplay(x*1.6f*ex);}
int main(){
  for (int cs=0; cs<2; ++cs) for (float st: {1.0f, 0.6f, 0.3f}) for (float fg: {0.4f, 0.55f, 0.7f}) {
    std::printf("clouds%d thr %.1f fog %.2f:", cs, st, fg);
    for (float e: {5.f, 8.f, 12.f, 20.f, 30.f}) {
      LightRigInputs in; float r=e*kPi/180, a=200*kPi/180;
      in.towardSun[0]=std::cos(r)*std::sin(a); in.towardSun[1]=std::sin(r); in.towardSun[2]=std::cos(r)*std::cos(a);
      in.fogColor=Rgb(fg,fg,fg*1.05f); in.skyThrough=st; in.nightEffect = e>5?0:1; in.cloudShadowsActive = cs==1;
      in.classicSunDiffuse=Rgb(0.82f,0.5f,0.3f);
      LightingTuning on; LightingTuning off; off.features.lowSunRig=0;
      LightRig ron=ComputeLightRig(in,on), roff=ComputeLightRig(in,off);
      auto ratio=[&](const LightRig& g){ return Sh(Luminance(Face(g,e,135)),g.exposure)/Sh(Luminance(Face(g,e,45)),g.exposure);};
      std::printf(" e%.0f %.2f->%.2f |", e, ratio(roff), ratio(ron));
    }
    std::printf("\n");
  }
}
