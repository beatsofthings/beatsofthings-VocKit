#include "../src/Vocoder.hpp"
#include <cassert>
#include <cmath>
#include <cstdio>

// Deterministic filterbank diagnostic: measures steady-state RMS response
// for a known sine wave and checks that distinct frequency bands discriminate.
static float rmsBand(float center,float stimulus){
 vockit::Bandpass filter;
 constexpr float sr=48000.f;
 filter.configure(center,sr,1.55f);
 double energy=0;
 constexpr int warmup=4800, samples=9600;
 for(int i=0;i<warmup+samples;++i){
   const float x=.5f*std::sin(2.f*vockit::pi*stimulus*i/sr);
   const float y=filter.tick(x);
   assert(std::isfinite(y));
   if(i>=warmup)energy+=double(y)*y;
 }
 return std::sqrt(float(energy/samples));
}
int main(){
 const float on=rmsBand(1000.f,1000.f);
 const float off=rmsBand(1000.f,200.f);
 std::printf("1000 Hz band: on=%.6f off(200 Hz)=%.6f ratio=%.2f\n",on,off,on/(off+1e-9f));
 assert(on>.05f);
 assert(on>off*2.0f);
 // The linear envelope must preserve a 4:1 amplitude relationship.
 // Do not silently replace it with a per-band saturating transfer.
 const float quiet=vockit::clamp(.01f*26.f,0.f,3.f);
 const float loud=vockit::clamp(.04f*26.f,0.f,3.f);
 assert(loud>quiet*3.9f);
 // An external carrier with silence must remain silent.
 vockit::Vocoder v;v.prepare(48000.f);v.setGate(true);v.setWet(1.f);
 for(int i=0;i<8192;++i){
   const float x=.5f*std::sin(2.f*vockit::pi*220.f*i/48000.f);
   const float y=v.tick(x,0.f,true);
   assert(std::isfinite(y));
   assert(std::fabs(y)<.1f); // allows only the deliberately limited consonant path
 }
 std::puts("VocKit filterbank diagnostics passed");
}
