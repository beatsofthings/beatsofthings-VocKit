#include "../src/Vocoder.hpp"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <array>
#include <algorithm>

// Synthetic vowel-like spectral envelopes, NOT a speech intelligibility test.
// Carrier is a fixed harmonic sawtooth; each modulator has distinct formant peaks.
constexpr float sr=48000.f;
constexpr int n=48000;
struct Vowel {const char* name; float f1,f2;};
static constexpr Vowel vowels[]={
 {"A",800,1200},{"E",500,1900},{"I",300,2400},
 {"O",500,900},{"U",350,800}
};
static std::array<float,24> render(const Vowel& vowel){
 vockit::Vocoder voc;voc.prepare(sr);voc.setGate(true);voc.setWet(1.f);
 voc.setFrequency(110.f);voc.setAttackRelease(5.f,65.f);
 std::array<vockit::Bandpass,24> probes{};
 for(int i=0;i<24;++i){
  const float hz=100.f*std::pow(8500.f/100.f,(i+.5f)/24.f);
  probes[i].configure(hz,sr,5.f);
 }
 std::array<double,24> energy{};
 for(int i=0;i<n;++i){
  const float t=i/sr;
  // Harmonics near synthetic formant peaks.
  float mod=0.f;
  for(int h=1;h<=45;++h){
   const float hz=110.f*h;
   const float d1=(hz-vowel.f1)/160.f;
   const float d2=(hz-vowel.f2)/240.f;
   const float weight=std::exp(-.5f*d1*d1)+.9f*std::exp(-.5f*d2*d2);
   mod+=.09f*weight*std::sin(2.f*vockit::pi*hz*t);
  }
  const float out=voc.tick(mod);
  assert(std::isfinite(out));
  if(i>n/3)for(int b=0;b<24;++b){
   const float y=probes[b].tick(out);
   energy[b]+=double(y)*y;
  }else for(auto& probe:probes)probe.tick(out);
 }
 std::array<float,24> result{};
 double total=0;
 for(int b=0;b<24;++b){result[b]=float(energy[b]);total+=energy[b];}
 assert(total>1e-8);
 for(float& x:result)x=float(x/total);
 return result;
}
int main(){
 std::array<std::array<float,24>,5> spectra{};
 for(int i=0;i<5;++i){
  spectra[i]=render(vowels[i]);
  std::printf("%s: ",vowels[i].name);
  for(int b=0;b<24;++b)std::printf("%.3f%s",spectra[i][b],b==23?"\n":",");
 }
 for(int i=0;i<5;++i)for(int j=i+1;j<5;++j){
  float distance=0;
  for(int b=0;b<24;++b)distance+=std::fabs(spectra[i][b]-spectra[j][b]);
  std::printf("%s vs %s spectral distance %.4f\n",vowels[i].name,vowels[j].name,distance);
  // Verify a non-identical spectral envelope reaches the processed carrier.
  assert(distance>.025f);
 }
 std::puts("Synthetic vowel spectral contrast test passed (not speech recognition).");
}
