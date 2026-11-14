// VocKit stereo effect: audio modulator, internal chord-capable carrier.
// SPDX-License-Identifier: MIT
#include "mpc_plugin.h"
#include "Vocoder.hpp"
#include <cmath>
class VocKit:public mpc::Plugin {
public:
 enum Param{Level,Pitch,Chord,Attack,Release,Mix,Input,Carrier,Tone,Mode,Detune,Sub,Help,Noise,Octave,NumParams};
 static mpc::PluginInfo makeInfo(){
   mpc::PluginInfo i;
   i.name="VocKit";i.vendor="Beats of Things";i.uniqueId=MPC_FOURCC('B','t','V','K');
   i.version=1100;i.isInstrument=false;i.numInputs=2;i.numOutputs=2;return i;
 }
 VocKit():mpc::Plugin(makeInfo()){
   addParam(mpc::floatParam("level","Level",0.f,100.f,80.f,"%",0));
   addParam(mpc::floatParam("pitch","Pitch",36.f,84.f,48.f,"MIDI",0));
   addParam(mpc::choiceParam("chord","Chord",{"Single","Octave","Fifth","Minor","Major"},0));
   addParam(mpc::floatParam("attack","Attack",1.f,100.f,8.f,"ms",1));
   addParam(mpc::floatParam("release","Release",10.f,500.f,90.f,"ms",0));
   addParam(mpc::floatParam("mix","Mix",0.f,100.f,100.f,"%",0));
   addParam(mpc::floatParam("input","Input",0.f,200.f,100.f,"%",0));
   addParam(mpc::floatParam("carrier","Carrier",0.f,100.f,70.f,"%",0));
   addParam(mpc::floatParam("tone","Tone",0.f,100.f,0.f,"%",0));
   addParam(mpc::choiceParam("mode","Input Mode",{"Internal","L Carrier / R Voice","R Carrier / L Voice"},0));
   addParam(mpc::floatParam("detune","Detune",0.f,35.f,0.f,"ct",0));
   addParam(mpc::floatParam("sub","Sub Osc",0.f,100.f,0.f,"%",0));
   // Non-audio help menu: setup instructions available on the touchscreen.
   addParam(mpc::floatParam("noise","Noise Osc",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("octave","Octave",-2.f,2.f,0.f,"",0));
   addParam(mpc::choiceParam("help","Setup Steps",{"1 Voice: Submix1 Pan R","2 JURA: Submix1 Pan L","3 FX on Submix1 only","4 Mode: L Carrier R Voice","Mic: Audio Input Monitor","Internal: FX on Voice"},0));
 }
 void prepare(double sr,int)override{sr_=float(sr);voc_.prepare(sr_);}
 void reset()override{voc_.prepare(sr_);}
 void process(mpc::ProcessContext& ctx)override{
   float level=get(Level)*.01f,wet=get(Mix)*.01f,gain=get(Input)*.01f;
   float carrier=get(Carrier)*.01f;
   const int mode=getInt(Mode);
   voc_.setAttackRelease(get(Attack),get(Release));
   voc_.setWet(1.f);voc_.setGate(true);voc_.setChord(getInt(Chord));
   voc_.setTone(get(Tone)*.01f);
   voc_.setDetune(get(Detune));voc_.setSubLevel(get(Sub)*.01f);
   voc_.setNoiseLevel(get(Noise)*.01f);voc_.setOctave(int(std::round(get(Octave))));
   voc_.setFrequency(440.f*std::pow(2.f,(get(Pitch)-69.f)/12.f));
   const float* l=ctx.inputs[0];
   const float* r=ctx.numInputs>1?ctx.inputs[1]:l;
   float* ol=ctx.outputs[0];float* orr=ctx.numOutputs>1?ctx.outputs[1]:ol;
   for(int n=0;n<ctx.numFrames;++n){
     float dryL=l[n],dryR=r[n];
     float mod=(mode==0?.5f*(dryL+dryR):(mode==1?dryR:dryL))*gain;
     float external=(mode==1?dryL:dryR);
     float fx=(mode==0?voc_.tick(mod):voc_.tick(mod,external,true))*carrier;
     const float dry=mode==0?.5f*(dryL+dryR):mod;
     float left=std::tanh(((mode==0?dryL:dry)*(1.f-wet)+fx*wet)*level);
     float right=std::tanh(((mode==0?dryR:dry)*(1.f-wet)+fx*wet)*level);
     ol[n]=left;if(orr!=ol)orr[n]=right;
   }
 }
private:
 vockit::Vocoder voc_;float sr_=44100.f;
};
MPC_PLUGIN_ENTRY(VocKit)
