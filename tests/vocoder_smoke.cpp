#include "../src/Vocoder.hpp"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>
int main() {
  for(float sr:{44100.f,48000.f}) {
    vockit::Vocoder v;
    v.prepare(sr);
    v.setGate(true);
    v.setFrequency(110.f);
    v.setWet(1.f);
    v.setAttackRelease(5.f,70.f);
    std::vector<float> in(256),out(256);
    float energy=0;
    for(int b=0;b<200;++b) {
      for(int i=0;i<256;++i) {
        const float t=(b*256+i)/sr;
        in[i]=.3f*std::sin(2.f*vockit::kPi*220.f*t)+.2f*std::sin(2.f*vockit::kPi*440.f*t);
      }
      v.process(in.data(),out.data(),out.size());
      for(float s:out) { assert(std::isfinite(s)); assert(std::fabs(s)<=1.00001f); energy+=s*s; }
    }
    assert(energy>1e-6f);
    v.prepare(sr); v.setGate(false);
    std::fill(in.begin(),in.end(),0.f);
    v.process(in.data(),out.data(),out.size());
    for(float s:out) assert(std::fabs(s)<1e-7f);
  }
  std::puts("VocKit 24-band DSP smoke tests passed");
}
