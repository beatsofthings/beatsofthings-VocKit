#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include "SpectralVocoder.hpp"
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
     ana_[i].configure(hz,sr_,4.0f);
     syn_[i].configure(hz,sr_,4.0f);
     env_[i].configure(sr_,attack_,release_);
     env_[i].env=0.f;
   }
   spectral_.prepare(sr_);
   phase_.fill(0.f);detunePhase_=subPhase_=lfoPhase_=osc2Phase_=0.f;
   filterLow_=filterBand_=synthEnvelope_=0.f;
   sibilanceLow_=0.f; sibilanceEnv_=0.f; noiseState_=0x12345678u;
 }
 void setAttackRelease(float a,float r){
   a=clamp(a,.1f,200.f);r=clamp(r,2.f,1000.f);
   if(a==attack_&&r==release_)return;
   attack_=a;release_=r;
   spectral_.setAttackRelease(a,r);
   for(auto &e:env_)e.configure(sr_,attack_,release_);
 }
 void setWet(float v){wet_=clamp(v,0.f,1.f);}
 void setFrequency(float hz){freq_=clamp(hz,20.f,4000.f);}
 void setGate(bool v){gate_=v;}
 void setChord(int v){chord_=std::max(0,std::min(4,v));}
 void setTone(float v){tone_=clamp(v,0.f,1.f);}
 void setWaveform(int shape){waveform_=std::max(0,std::min(3,shape));}
 void setLfo(int shape,float rateHz,float depthCents){lfoShape_=std::max(0,std::min(2,shape));lfoRate_=clamp(rateHz,0.f,20.f);lfoDepth_=clamp(depthCents,0.f,100.f);}
 void setOsc2(int shape,float level,float semitones,float cents){osc2Shape_=std::max(0,std::min(3,shape));osc2Level_=clamp(level,0.f,1.f);osc2Semi_=clamp(semitones,-24.f,24.f);osc2Cents_=clamp(cents,-50.f,50.f);}
 void setSibilance(float amount){sibilance_=clamp(amount,0.f,1.f);}
 void setDetune(float cents){detune_=clamp(cents,0.f,35.f);}
 void setSubLevel(float amount){subLevel_=clamp(amount,0.f,1.f);}
 void setNoiseLevel(float amount){noiseLevel_=clamp(amount,0.f,1.f);}
 void setOctave(int octave){octave_=std::max(-2,std::min(2,octave));}
 void setCarrierFilter(float cutoff,float resonance){cutoff_=clamp(cutoff,0.f,1.f);resonance_=clamp(resonance,0.f,.95f);}
 void setSynthEnvelope(float attack,float release){synthAttack_=clamp(attack,1.f,500.f);synthRelease_=clamp(release,5.f,2000.f);}
 void setBandLevel(int index,float level){if(index>=0&&index<kBands)bandLevel_[index]=clamp(level,0.f,2.f);}
 void setFormant(float semitones){
   semitones=clamp(semitones,-12.f,12.f);
   if(semitones==formant_)return;
   formant_=semitones;
   for(int i=0;i<kBands;++i){
     const float hz=100.f*std::pow(8500.f/100.f,(i+.5f)/kBands);
     syn_[i].configure(hz*std::pow(2.f,formant_/12.f),sr_,4.0f);
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
   lfoPhase_+=lfoRate_/sr_;
   if(lfoPhase_>=1.f)lfoPhase_-=1.f;
   const float lfoValue=lfoShape_==1?(1.f-4.f*std::fabs(lfoPhase_-.5f)):(lfoShape_==2?(lfoPhase_<.5f?1.f:-1.f):std::sin(2.f*pi*lfoPhase_));
   const float oscillatorFrequency=freq_*std::pow(2.f,float(octave_)+lfoDepth_*lfoValue/1200.f);
   float carrier=0.f;
   for(int i=0;i<voices;++i){
     float ratio=i==0?1.f:(chord_==1?2.f:(chord_==2?1.4983071f:ratios[i]));
     phase_[i]+=oscillatorFrequency*ratio/sr_;
     if(phase_[i]>=1.f)phase_[i]-=1.f;
     float saw=2.f*phase_[i]-1.f;
     float pulse=phase_[i]<.45f?1.f:-1.f;
     const float triangle=1.f-4.f*std::fabs(phase_[i]-.5f);
     const float sine=std::sin(2.f*pi*phase_[i]);
     const float wave=waveform_==1?triangle:(waveform_==2?sine:(waveform_==3?pulse:(saw*(1.f-tone_)+pulse*tone_)));
     carrier+=wave/voices;
   }
   if(!gate_)carrier=0.f;
   if(!useExternal && gate_ && osc2Level_>0.f){
     const float hz=oscillatorFrequency*std::pow(2.f,osc2Semi_/12.f+osc2Cents_/1200.f);
     osc2Phase_+=hz/sr_;
     if(osc2Phase_>=1.f)osc2Phase_-=std::floor(osc2Phase_);
     const float saw=2.f*osc2Phase_-1.f;
     const float triangle=1.f-4.f*std::fabs(osc2Phase_-.5f);
     const float sine=std::sin(2.f*pi*osc2Phase_);
     const float square=osc2Phase_<.5f?1.f:-1.f;
     const float wave=osc2Shape_==1?triangle:(osc2Shape_==2?sine:(osc2Shape_==3?square:saw));
     carrier=(carrier+osc2Level_*wave)/(1.f+osc2Level_);
   }
   // Additional detuned saw voice and sub octave for a richer internal carrier.
   // Only active in internal mode; external JURA audio is never altered here.
   if(!useExternal && gate_ && (detune_>0.f || subLevel_>0.f)){
     detunePhase_+=oscillatorFrequency*std::pow(2.f,detune_/1200.f)/sr_;
     if(detunePhase_>=1.f)detunePhase_-=1.f;
     subPhase_+=oscillatorFrequency*.5f/sr_;
     if(subPhase_>=1.f)subPhase_-=1.f;
     const float detunedSaw=2.f*detunePhase_-1.f;
     const float subSaw=2.f*subPhase_-1.f;
     carrier=.65f*carrier+.35f*detunedSaw+.30f*subLevel_*subSaw;
   }
   if(!useExternal && gate_ && noiseLevel_>0.f){
     noiseState_=noiseState_*1664525u+1013904223u;
     const float white=float((noiseState_>>8)&0xffffu)/32767.5f-1.f;
     carrier=carrier*(1.f-.5f*noiseLevel_)+white*.35f*noiseLevel_;
   }
   // Optional resonant two-pole low-pass; default cutoff=1 bypasses filter.
   if(!useExternal && cutoff_<.999f){
     const float hz=80.f*std::pow(160.f,cutoff_);
     const float f=clamp(2.f*std::sin(pi*hz/sr_),0.f,.8f);
     const float damp=1.8f-1.65f*resonance_;
     for(int j=0;j<2;++j){
       filterLow_+=f*filterBand_*.5f;
       const float high=carrier-filterLow_-damp*filterBand_;
       filterBand_+=f*high*.5f;
     }
     carrier=filterLow_;
   }
   // Voice-triggered carrier envelope. At default attack/release the FFT
   // envelope remains the main speech modulator, preserving classic sound.
   if(!useExternal && (synthAttack_>1.f || synthRelease_>5.f)){
     const float target=gate_?1.f:0.f;
     const float ms=target>synthEnvelope_?synthAttack_:synthRelease_;
     const float coeff=std::exp(-1000.f/(sr_*ms));
     synthEnvelope_=coeff*synthEnvelope_+(1.f-coeff)*target;
     carrier*=synthEnvelope_;
   }
   carrier*=.42f;
   if(useExternal)carrier=std::isfinite(externalCarrier)?clamp(externalCarrier,-2.f,2.f):0.f;
   // FFT spectral-envelope transfer, replacing the old bandpass engine.
   const float processed=spectral_.tick(input,carrier,bandLevel_);
   // Optional unvoiced consonant path: high-passed speech blended into wet output.
   // Default zero retains the established FFT vocoder sound unchanged.
   float unvoiced=0.f;
   if(sibilance_>0.f){
     const float a=std::exp(-2.f*pi*3500.f/sr_);
     sibilanceLow_=a*sibilanceLow_+(1.f-a)*input;
     unvoiced=(input-sibilanceLow_)*sibilance_*.35f;
   }
   return clamp(input*(1.f-wet_)+(processed+unvoiced)*wet_,-1.f,1.f);
 }
 void process(const float* in,float* out,std::size_t n){
   for(std::size_t i=0;i<n;++i)out[i]=tick(in?in[i]:0.f);
 }
private:
 float sr_=44100.f,freq_=110.f,wet_=1.f,attack_=8.f,release_=90.f,tone_=0.f,formant_=0.f;
 float detune_=0.f,subLevel_=0.f,noiseLevel_=0.f,detunePhase_=0.f,subPhase_=0.f;
 float osc2Phase_=0.f,osc2Level_=0.f,osc2Semi_=0.f,osc2Cents_=0.f,sibilance_=0.f;
 int osc2Shape_=0;
 float cutoff_=1.f,resonance_=0.f,synthAttack_=1.f,synthRelease_=5.f;
 float filterLow_=0.f,filterBand_=0.f,synthEnvelope_=0.f;
 int octave_=0,waveform_=0,lfoShape_=0;
 float lfoPhase_=0.f,lfoRate_=0.f,lfoDepth_=0.f;
 std::array<float,kBands> bandLevel_=[](){std::array<float,kBands> a{};a.fill(1.f);return a;}();
 bool gate_=false;int chord_=0;
 float sibilanceLow_=0.f,sibilanceEnv_=0.f;
 unsigned int noiseState_=0x12345678u;
 SpectralVocoder spectral_{};
 std::array<float,3> phase_{};
 std::array<Bandpass,kBands> ana_{},syn_{};
 std::array<Follower,kBands> env_{};
};
}
