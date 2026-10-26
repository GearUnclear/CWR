#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <cmath>
#include <initializer_list>
#include <cstdio>
using namespace Poseidon::remaster;
int main(){
  const float kPi=3.14159265f;
  float fogs[] = {0.25f, 0.4f, 0.55f, 0.7f};
  float throughs[] = {1.0f, 0.5f, 0.2f};
  for (float st: throughs) for (float fg: fogs) {
    std::printf("skyThrough %.2f fog %.2f:", st, fg);
    for (float e: {2.f, 5.f, 8.f, 12.f, 20.f, 30.f}) {
      LightRigInputs in; float r=e*kPi/180, a=200*kPi/180;
      in.towardSun[0]=std::cos(r)*std::sin(a); in.towardSun[1]=std::sin(r); in.towardSun[2]=std::cos(r)*std::cos(a);
      in.fogColor=Rgb(fg,fg,fg*1.05f); in.skyThrough=st; in.nightEffect = e>5?0:1;
      in.classicSunDiffuse=Rgb(0.82f,0.5f,0.3f);
      LightingTuning on; LightingTuning off; off.features.lowSunRig=0;
      LightRig ron=ComputeLightRig(in,on), roff=ComputeLightRig(in,off);
      float fill=LowSunFill(e);
      // unlimited anti-sun estimate: ratio expected roughly 1+2.6 fill on the sky part
      std::printf(" e%.0f fill%.2f anti x%.2f sun x%.2f |", e, fill, Luminance(ron.skyHorizon)/Luminance(roff.skyHorizon), Luminance(ron.skyHorizonSun)/Luminance(roff.skyHorizonSun));
    }
    std::printf("\n");
  }
}


