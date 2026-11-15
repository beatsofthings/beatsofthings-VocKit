#include "../src/Vocoder.hpp"
#include "../src/PostFx.hpp"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
int main(){
 constexpr int sr=48000,seconds=120;
 vockit::Vocoder voc;vockit::PostFx fx;
 voc.prepare(sr);fx.prepare(sr);voc.setGate(true);
 voc.setWet(1.f);
 const auto start=std::chrono::steady_clock::now();
 double energy=0.;
 for(int i=0;i<sr*seconds;++i){
  const float voice=.35f*std::sin(6.28318530718f*180.f*float(i)/sr)
       +.12f*std::sin(6.28318530718f*450.f*float(i)/sr);
  float l=voc.tick(voice),r=l;
  fx.process(l,r,.15f,.30f,.8f,.5f);
  if(!std::isfinite(l)||!std::isfinite(r))return 1;
  energy+=double(l)*l+double(r)*r;
 }
 const double elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
 std::printf("VocKit 120s simulated audio: wall %.3fs, realtime factor %.2fx, energy %.2f\n",
   elapsed,double(seconds)/elapsed,energy);
 if(energy<=0.)return 2;
 // Host CI is not MPC hardware: report, do not assert a CPU threshold.
}
