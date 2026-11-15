#include "../src/PostFx.hpp"
#include <cmath>
#include <cstdio>
int main(){
 vockit::PostFx fx;
 fx.prepare(48000.f);
 for(int i=0;i<5000;++i){
  float l=.4f*std::sin(float(i)*.021f),r=.3f*std::cos(float(i)*.018f);
  const float beforeL=l,beforeR=r;
  fx.process(l,r,0.f,0.f,.8f,.5f);
  if(l!=beforeL||r!=beforeR)return 1; // Exact legacy bypass
 }
 fx.prepare(48000.f);
 float peak=0.f;
 for(int i=0;i<20000;++i){
  float l=.25f*std::sin(float(i)*.023f),r=.22f*std::sin(float(i)*.031f);
  fx.process(l,r,.25f,.45f,.8f,.6f);
  if(!std::isfinite(l)||!std::isfinite(r))return 2;
  peak=std::max(peak,std::max(std::fabs(l),std::fabs(r)));
 }
 if(peak<=0.f||peak>1.1f)return 3;
 std::puts("PostFx neutral bypass and active stereo chorus/saturation OK");
}
