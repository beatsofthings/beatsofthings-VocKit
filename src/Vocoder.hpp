#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
namespace vockit {
constexpr int kBands=24;
constexpr float pi=3.14159265358979323846f;
constexpr float kPi=pi; // Retain API compatibility with DSP regression tests.
inline float clamp(float v,float lo,float hi){return std::max(lo,std::min(v,hi));}
// Stable RBJ biquad: constant 0 dB peak band-pass, direct form II transposed.
// The old state-variable filter's resonance dominated the speech spectrum.
struct Bandpass {
 float b0=0.f,b2=0.f,a1=0.f,a2=0.f,z1=0.f,z2=0.f;
 void configure(float hz,float sr,float quality){
   const float w=2.f*pi*clamp(hz,40.f,sr*.45f)/sr;
   const float sn=std::sin(w),co=std::cos(w);
   const float alpha=sn/(2.f*std::max(.5f,quality));
   const float inv=1.f/(1.f+alpha);
   b0=alpha*inv;b2=-b0;
   a1=-2.f*co*inv;a2=(1.f-alpha)*inv;
   z1=z2=0.f;
 }
 float tick(float x){
   const float y=b0*x+z1;
   z1=z2-a1*y;
   z2=b2*x-a2*y;
   return y;
 }
};
struct Follower {
 float env=0,a=0,r=0;
 void configure(float sr,float atk,float rel){
   a=std::exp(-1000.f/(sr*std::max(.1f,atk)));
   r=std::exp(-1000.f/(sr*std::max(.1f,rel)));
 }
 float tick(float x){x=std::fabs(x);float c=x>env?a:r;env=c*env+(1.f-c)*x;return env;}
};
class Vocoder {
public:
 void prepare(float sr){
   sr_=std::max(8000.f,sr);
   for(int i=0;i<kBands;++i){
     float hz=100.f*std::pow(8500.f/100.f,(i+.5f)/kBands);
     ana_[i].configure(hz,sr_,1.55f);
     syn_[i].configure(hz,sr_,1.55f);
     env_[i].configure(sr_,attack_,release_);
     env_[i].env=0.f;
   }
   phase_.fill(0.f);
   sibilanceLow_=0.f; sibilanceEnv_=0.f; noiseState_=0x12345678u;
 }
 void setAttackRelease(float a,float r){
   a=clamp(a,.1f,200.f);r=clamp(r,2.f,1000.f);
   if(a==attack_&&r==release_)return;
   attack_=a;release_=r;
   for(auto &e:env_)e.configure(sr_,attack_,release_);
 }
 void setWet(float v){wet_=clamp(v,0.f,1.f);}
 void setFrequency(float hz){freq_=clamp(hz,20.f,4000.f);}
 void setGate(bool v){gate_=v;}
 void setChord(int v){chord_=std::max(0,std::min(4,v));}
 void setTone(float v){tone_=clamp(v,0.f,1.f);}
 void setBandLevel(int index,float level){if(index>=0&&index<kBands)bandLevel_[index]=clamp(level,0.f,2.f);}
 void setFormant(float semitones){
   semitones=clamp(semitones,-12.f,12.f);
   if(semitones==formant_)return;
   formant_=semitones;
   for(int i=0;i<kBands;++i){
     const float hz=100.f*std::pow(8500.f/100.f,(i+.5f)/kBands);
     syn_[i].configure(hz*std::pow(2.f,formant_/12.f),sr_,1.55f);
   }
 }
 float tick(float input){return tick(input,0.f,false);}
 float tick(float input,float externalCarrier,bool useExternal){
   if(!std::isfinite(input))input=0.f;
   // Oscillator pitch ratios: unison / octave / fifth / minor / major.
   constexpr float minor[]={1.f,1.1892071f,1.4983071f};
   constexpr float major[]={1.f,1.259921f,1.4983071f};
   const float* ratios=chord_==3?minor:major;
   int voices=(chord_==0)?1:(chord_==1||chord_==2)?2:3;
   float carrier=0.f;
   for(int i=0;i<voices;++i){
     float ratio=i==0?1.f:(chord_==1?2.f:(chord_==2?1.4983071f:ratios[i]));
     phase_[i]+=freq_*ratio/sr_;
     if(phase_[i]>=1.f)phase_[i]-=1.f;
     float saw=2.f*phase_[i]-1.f;
     float pulse=phase_[i]<.45f?1.f:-1.f;
     carrier+=(saw*(1.f-tone_)+pulse*tone_)/voices;
   }
   if(!gate_)carrier=0.f;
   carrier*=.42f;
   if(useExternal)carrier=std::isfinite(externalCarrier)?clamp(externalCarrier,-2.f,2.f):0.f;
   // True spectral-envelope transfer: each analysis band independently
   // controls its matching synthesis band. No broadband noise generator.
   // This preserves formant contrasts instead of producing a noise wash.
   float sum=0.f;
   for(int i=0;i<kBands;++i){
     const float speech=ana_[i].tick(input);
     const float envelope=env_[i].tick(speech);
     const float synthesized=syn_[i].tick(carrier);
     // Soft knee avoids clipping loud vowels while retaining weak consonants.
     const float gain=envelope/(envelope+.035f);
     sum+=synthesized*gain*bandLevel_[i];
   }
   // Light, speech-gated sibilant passthrough only above the main formants.
   // The input highpass is never fed into the carrier or noise source.
   const float high=input-sibilanceLow_;
   sibilanceLow_+=0.12f*high;
   sibilanceEnv_=0.995f*sibilanceEnv_+0.005f*std::fabs(high);
   const float articulation=clamp((sibilanceEnv_-.018f)*1.8f,0.f,.12f);
   const float processed=std::tanh(sum*.85f+high*articulation);
   return clamp(input*(1.f-wet_)+processed*wet_,-1.f,1.f);
 }
 void process(const float* in,float* out,std::size_t n){
   for(std::size_t i=0;i<n;++i)out[i]=tick(in?in[i]:0.f);
 }
private:
 float sr_=44100.f,freq_=110.f,wet_=1.f,attack_=8.f,release_=90.f,tone_=0.f,formant_=0.f;
 std::array<float,kBands> bandLevel_=[](){std::array<float,kBands> a{};a.fill(1.f);return a;}();
 bool gate_=false;int chord_=0;
 float sibilanceLow_=0.f,sibilanceEnv_=0.f;
 unsigned int noiseState_=0x12345678u;
 std::array<float,3> phase_{};
 std::array<Bandpass,kBands> ana_{},syn_{};
 std::array<Follower,kBands> env_{};
};
}
