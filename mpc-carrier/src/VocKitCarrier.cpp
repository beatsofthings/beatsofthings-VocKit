// VocKit Carrier: independent MIDI-playable synth instrument for external VocKit FX.
// SPDX-License-Identifier: MIT
#include "mpc_plugin.h"
#include <algorithm>
#include <array>
#include <cmath>
class VocKitCarrier final : public mpc::Plugin {
 static constexpr float pi=3.14159265358979323846f;
 enum Param {Wave,Level,Attack,Release,Cutoff,Resonance,Unison,Detune,Sub,Drive,Octave,NumParams};
 std::array<bool,128> held_{};
 std::array<float,8> phase_{};
 float subPhase_=0.f,env_=0.f,filter_=0.f,sr_=44100.f;
 int active_=-1;
 static float clip(float x,float lo,float hi){return std::max(lo,std::min(x,hi));}
 static mpc::PluginInfo info(){
  mpc::PluginInfo i;i.name="VocKit Carrier";i.vendor="Beats of Things";
  i.uniqueId=MPC_FOURCC('B','t','V','C');i.version=1000;
  i.isInstrument=true;i.wantsMidi=true;i.numInputs=0;i.numOutputs=2;return i;
 }
 void chooseHeld(){active_=-1;for(int n=127;n>=0;--n)if(held_[n]){active_=n;break;}}
public:
 VocKitCarrier():mpc::Plugin(info()){
  addParam(mpc::choiceParam("wave","Waveform",{"Saw","Pulse","Triangle","Sine"},0));
  addParam(mpc::floatParam("level","Level",0.f,100.f,65.f,"%",0));
  addParam(mpc::floatParam("attack","Attack",1.f,500.f,5.f,"ms",0));
  addParam(mpc::floatParam("release","Release",5.f,2000.f,120.f,"ms",0));
  addParam(mpc::floatParam("cutoff","Cutoff",0.f,100.f,90.f,"%",0));
  addParam(mpc::floatParam("resonance","Resonance",0.f,90.f,0.f,"%",0));
  addParam(mpc::choiceParam("unison","Unison",{"1","2","3","4","5","6","7","8"},0));
  addParam(mpc::floatParam("detune","Detune",0.f,35.f,0.f,"ct",0));
  addParam(mpc::floatParam("sub","Sub",0.f,100.f,0.f,"%",0));
  addParam(mpc::floatParam("drive","Drive",0.f,100.f,0.f,"%",0));
  addParam(mpc::floatParam("octave","Octave",-2.f,2.f,0.f,"",0));
  addProgram("Clean Saw", {0.f,65.f,5.f,120.f,90.f,0.f,0.f,0.f,0.f,0.f,0.f});
  addProgram("Electro Pulse", {1.f,65.f,3.f,90.f,85.f,0.f,2.f,9.f,10.f,8.f,0.f});
  addProgram("Soft Choir", {2.f,65.f,40.f,400.f,70.f,0.f,3.f,7.f,0.f,0.f,0.f});
  addProgram("Dark Robot", {0.f,60.f,5.f,130.f,42.f,15.f,1.f,13.f,20.f,15.f,-1.f});
 }
 void prepare(double sr,int) override {sr_=float(sr);reset();}
 void reset() override {held_.fill(false);phase_.fill(0.f);subPhase_=env_=filter_=0.f;active_=-1;}
 void process(mpc::ProcessContext& ctx) override {
  int event=0;
  const int count=std::max(0,ctx.numMidiEvents);
  const int wave=getInt(Wave),voices=std::max(1,std::min(8,getInt(Unison)+1));
  const float detune=get(Detune),level=get(Level)*.01f,sub=get(Sub)*.01f;
  const float attack=std::max(1.f,get(Attack)),release=std::max(5.f,get(Release));
  const float cutoff=get(Cutoff)*.01f,res=get(Resonance)*.01f,drive=get(Drive)*.01f;
  const float octave=std::round(get(Octave));
  for(int n=0;n<ctx.numFrames;++n){
   while(ctx.midi&&event<count&&ctx.midi[event].sampleOffset<=n){
    const auto& m=ctx.midi[event++];
    if(m.isAllNotesOff()){held_.fill(false);active_=-1;}
    else if(m.isNoteOn()&&m.note()<128){held_[m.note()]=true;active_=m.note();}
    else if(m.isNoteOff()&&m.note()<128){held_[m.note()]=false;if(active_==m.note())chooseHeld();}
   }
   const float target=active_>=0?1.f:0.f;
   const float coeff=std::exp(-1000.f/(sr_*(target>env_?attack:release)));
   env_=coeff*env_+(1.f-coeff)*target;
   float value=0.f;
   if(active_>=0||env_>1e-5f){
    const float hz=440.f*std::pow(2.f,(float(std::max(0,active_))+octave*12.f-69.f)/12.f);
    for(int u=0;u<voices;++u){
     const float cents=voices==1?0.f:detune*(2.f*float(u)/float(voices-1)-1.f);
     phase_[u]+=hz*std::pow(2.f,cents/1200.f)/sr_;
     if(phase_[u]>=1.f)phase_[u]-=std::floor(phase_[u]);
     const float p=phase_[u];
     value+=(wave==0?2.f*p-1.f:wave==1?(p<.45f?1.f:-1.f):
             wave==2?1.f-4.f*std::fabs(p-.5f):std::sin(2.f*pi*p))/float(voices);
    }
    if(sub>0.f){
     subPhase_+=hz*.5f/sr_;if(subPhase_>=1.f)subPhase_-=std::floor(subPhase_);
     value=(value+sub*(2.f*subPhase_-1.f))/(1.f+sub);
    }
   }
   const float f=clip(2.f*pi*(90.f*std::pow(140.f,cutoff))/sr_,.0001f,.85f);
   filter_+=f*(value-filter_);
   const float filtered=value*(res*.25f)+filter_*(1.f-res*.25f);
   const float out=std::tanh(filtered*(1.f+drive*5.f))*env_*level*.7f;
   if(ctx.numOutputs>0&&ctx.outputs[0])ctx.outputs[0][n]=out;
   if(ctx.numOutputs>1&&ctx.outputs[1])ctx.outputs[1][n]=out;
  }
 }
};
MPC_PLUGIN_ENTRY(VocKitCarrier)
