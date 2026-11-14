#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace vockit {
constexpr int kBands = 24;
constexpr float kPi = 3.14159265358979323846f;
inline float clamp(float x, float lo, float hi) { return std::max(lo, std::min(x, hi)); }

struct Bandpass {
  float low = 0, band = 0, f = 0, q = 0;
  void configure(float hz, float sr, float quality) {
    f = 2.f * std::sin(kPi * clamp(hz, 10.f, sr * .22f) / sr);
    q = 1.f / quality;
    low = band = 0;
  }
  float process(float x) {
    // Two half-frequency SVF steps for improved numerical margin.
    for (int i=0;i<2;++i) {
      low += f * band * .5f;
      const float high = x - low - q * band;
      band += f * high * .5f;
    }
    return band;
  }
};
struct Follower {
  float env = 0, a = 0, r = 0;
  void configure(float sr, float attackMs, float releaseMs) {
    a = std::exp(-1000.f / (sr * std::max(attackMs, .1f)));
    r = std::exp(-1000.f / (sr * std::max(releaseMs, .1f)));
  }
  float process(float x) {
    x = std::fabs(x);
    env = (x > env ? a : r) * env + (1.f - (x > env ? a : r)) * x;
    return env;
  }
};
class Vocoder {
public:
  void prepare(float sampleRate) {
    sr_ = std::max(8000.f, sampleRate);
    for (int i=0;i<kBands;++i) {
      const float hz = 140.f * std::pow(6800.f/140.f, (i+.5f)/kBands);
      analysis_[i].configure(hz,sr_,5.f);
      synthesis_[i].configure(hz,sr_,5.f);
      followers_[i].configure(sr_,attackMs_,releaseMs_);
    }
    phase_ = 0;
  }
  void setAttackRelease(float attackMs, float releaseMs) {
    attackMs_=clamp(attackMs,.1f,200.f);
    releaseMs_=clamp(releaseMs,2.f,1000.f);
    for(auto &e:followers_) e.configure(sr_,attackMs_,releaseMs_);
  }
  void setWet(float wet) { wet_=clamp(wet,0.f,1.f); }
  void setFrequency(float hz) { frequency_=clamp(hz,20.f,4000.f); }
  void setGate(bool on) { gate_=on; }
  // The modulator is an audio input; the carrier is generated internally.
  float tick(float modulator) {
    if (!std::isfinite(modulator)) modulator=0.f;
    const float carrier = gate_ ? (2.f * phase_ - 1.f) * .35f : 0.f;
    phase_ += frequency_ / sr_;
    if (phase_>=1.f) phase_-=1.f;
    float sum=0.f;
    for(int i=0;i<kBands;++i) {
      const float level=followers_[i].process(analysis_[i].process(modulator));
      sum += synthesis_[i].process(carrier) * clamp(level * 8.f,0.f,1.f);
    }
    const float processed=std::tanh(sum * .8f);
    return clamp(modulator*(1.f-wet_)+processed*wet_,-1.f,1.f);
  }
  void process(const float* input,float* output,std::size_t frames) {
    for(std::size_t i=0;i<frames;++i) output[i]=tick(input ? input[i] : 0.f);
  }
private:
  float sr_=48000.f, phase_=0.f, frequency_=110.f, wet_=1.f;
  float attackMs_=8.f, releaseMs_=90.f;
  bool gate_=false;
  std::array<Bandpass,kBands> analysis_{}, synthesis_{};
  std::array<Follower,kBands> followers_{};
};
} // namespace vockit
