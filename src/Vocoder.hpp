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
struct Bandpass {
 float low=0,band=0,f=0,q=0;
 void configure(float hz,float sr,float quality){
   f=2.f*std::sin(pi*clamp(hz,10.f,sr*.22f)/sr);q=1.f/quality;low=band=0;
 }
 float tick(float x){
   for(int j=0;j<2;++j){low+=f*band*.5f;float hi=x-low-q*band;band+=f*hi*.5f;}
   return band;
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
     float hz=140.f*std::pow(6800.f/140.f,(i+.5f)/kBands);
     ana_[i].configure(hz,sr_,2.8f);
     syn_[i].configure(hz,sr_,2.8f);
     env_[i].configure(sr_,attack_,release_);
   }
   phase_.fill(0.f);
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
     const float hz=140.f*std::pow(6800.f/140.f,(i+.5f)/kBands);
     syn_[i].configure(hz*std::pow(2.f,formant_/12.f),sr_,2.8f);
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
   // Preserve consonant transients using a high-frequency unvoiced excitation.
   const float high=input-sibilanceLow_;
   sibilanceLow_+=0.10f*high;
   const float transient=std::fabs(high);
   sibilanceEnv_=0.97f*sibilanceEnv_+0.03f*transient;
   noiseState_=noiseState_*1664525u+1013904223u;
   const float noise=float((noiseState_>>8)&0xffffu)/32767.5f-1.f;
   const float consonant=clamp(sibilanceEnv_*5.f,0.f,.65f);
   float sum=0.f;
   for(int i=0;i<kBands;++i){
     float amplitude=env_[i].tick(ana_[i].tick(input));
     const float excitation=(i>=17)?carrier*(1.f-consonant)+noise*consonant*.55f:carrier;
     sum+=syn_[i].tick(excitation)*clamp(amplitude*12.f,0.f,1.f)*bandLevel_[i];
   }
   float processed=std::tanh(sum*1.1f);
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
