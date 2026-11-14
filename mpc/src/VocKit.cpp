// VocKit: stereo FX, live input is modulator, internal saw is carrier.
// SPDX-License-Identifier: MIT
#include "mpc_plugin.h"
#include "Vocoder.hpp"
#include <cmath>

class VocKit : public mpc::Plugin {
public:
 enum Param { Level, Pitch, Chord, Attack, Release, Mix, Input, Carrier, NumParams };
 static mpc::PluginInfo makeInfo() {
   mpc::PluginInfo i;
   i.name="VocKit"; i.vendor="Beats of Things";
   i.uniqueId=MPC_FOURCC('B','t','V','K');
   i.version=1000; i.isInstrument=false; i.numInputs=2; i.numOutputs=2;
   return i;
 }
 VocKit():mpc::Plugin(makeInfo()) {
   addParam(mpc::floatParam("level","Level",0.f,100.f,80.f,"%",0));
   addParam(mpc::floatParam("pitch","Pitch",36.f,84.f,48.f,"MIDI",0));
   addParam(mpc::choiceParam("chord","Chord",{"Single","Octave","Fifth","Minor","Major"},0));
   addParam(mpc::floatParam("attack","Attack",1.f,100.f,8.f,"ms",1));
   addParam(mpc::floatParam("release","Release",10.f,500.f,90.f,"ms",0));
   addParam(mpc::floatParam("mix","Mix",0.f,100.f,100.f,"%",0));
   addParam(mpc::floatParam("input","Input",0.f,200.f,100.f,"%",0));
   addParam(mpc::floatParam("carrier","Carrier",0.f,100.f,70.f,"%",0));
 }
 void prepare(double sampleRate,int) override {
   sr_=static_cast<float>(sampleRate);
   voc_.prepare(sr_); phase_=0.f;
 }
 void reset() override { voc_.prepare(sr_); phase_=0.f; }
 void process(mpc::ProcessContext& ctx) override {
   const float level=get(Level)*.01f;
   const float wet=get(Mix)*.01f;
   const float gain=get(Input)*.01f;
   const float carrierGain=get(Carrier)*.01f;
   voc_.setAttackRelease(get(Attack),get(Release));
   voc_.setWet(1.f);
   voc_.setGate(true);
   const float hz=440.f*std::pow(2.f,(get(Pitch)-69.f)/12.f);
   voc_.setFrequency(hz);
   const float* l=ctx.inputs[0];
   const float* r=ctx.numInputs>1 ? ctx.inputs[1] : l;
   float* ol=ctx.outputs[0];
   float* orr=ctx.numOutputs>1 ? ctx.outputs[1] : ol;
   for(int n=0;n<ctx.numFrames;++n) {
     const float dryL=l[n],dryR=r[n];
     const float mod=.5f*(dryL+dryR)*gain;
     // Current core exposes one saw oscillator. Chord and carrier level
     // are wired in the next DSP revision; do not misrepresent them as functional.
     const float effected=voc_.tick(mod)*carrierGain;
     const float outL=std::tanh((dryL*(1.f-wet)+effected*wet)*level);
     const float outR=std::tanh((dryR*(1.f-wet)+effected*wet)*level);
     ol[n]=outL; if(orr!=ol) orr[n]=outR;
   }
 }
private:
 vockit::Vocoder voc_;
 float sr_=44100.f,phase_=0.f;
};
MPC_PLUGIN_ENTRY(VocKit)
