// VocKit stereo effect: audio modulator, internal chord-capable carrier.
// SPDX-License-Identifier: MIT
#include "mpc_plugin.h"
#include "Vocoder.hpp"
#include <cmath>
#include <cstdio>
class VocKit:public mpc::Plugin {
public:
 enum Param{Level,Pitch,Chord,Attack,Release,Mix,Input,Carrier,Tone,Mode,Detune,Sub,Help,Noise,Octave,Cutoff,Resonance,SynthAttack,SynthRelease,Band01,Waveform=Band01+24,LfoShape,LfoRate,LfoDepth,Monitor,NumParams};
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
   addParam(mpc::floatParam("cutoff","Filter Cutoff",0.f,100.f,100.f,"%",0));
   addParam(mpc::floatParam("resonance","Resonance",0.f,95.f,0.f,"%",0));
   addParam(mpc::floatParam("synthAttack","Synth Attack",1.f,500.f,1.f,"ms",0));
   addParam(mpc::floatParam("synthRelease","Synth Release",5.f,2000.f,5.f,"ms",0));
   addParam(mpc::choiceParam("help","Setup Steps",{"1 Voice: Submix1 Pan R","2 JURA: Submix1 Pan L","3 FX on Submix1 only","4 Mode: L Carrier R Voice","Mic: Audio Input Monitor","Internal: FX on Voice"},0));
   for(int i=0;i<24;++i){
     char id[16],name[16];
     std::snprintf(id,sizeof(id),"band%02d",i+1);
     std::snprintf(name,sizeof(name),"Band %02d",i+1);
     addParam(mpc::floatParam(id,name,0.f,200.f,100.f,"%",0));
   }

   addParam(mpc::choiceParam("waveform","Waveform",{"Saw/Pulse","Triangle","Sine","Square"},0));
   addParam(mpc::choiceParam("lfoShape","LFO Shape",{"Sine","Triangle","Square"},0));
   addParam(mpc::floatParam("lfoRate","LFO Rate",0.f,20.f,0.f,"Hz",1));
   addParam(mpc::floatParam("lfoDepth","LFO Depth",0.f,100.f,0.f,"ct",1));
   addParam(mpc::choiceParam("monitor","Audio Monitor",{"Normal","Listen Left","Listen Right","Listen L+R"},0));
   // Plain values, exact parameter order; 24 band gains follow setup help.
   addProgram("Init", {80.f, 48.f, 0.f, 8.f, 90.f, 100.f, 100.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Robot Voice", {80.f, 48.f, 0.f, 5.f, 70.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Clear Rap", {80.f, 55.f, 0.f, 2.f, 35.f, 92.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Dark Vocoder", {80.f, 43.f, 0.f, 12.f, 130.f, 100.f, 100.f, 100.f, 45.f, 0.f, 0.f, 20.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 70.f, 70.f, 70.f, 70.f, 70.f, 70.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Saw Brass", {80.f, 48.f, 0.f, 8.f, 90.f, 100.f, 100.f, 100.f, 0.f, 0.f, 7.f, 12.f, 0.f, 0.f, 0.f, 95.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Soft Strings", {80.f, 55.f, 0.f, 18.f, 160.f, 100.f, 100.f, 100.f, 35.f, 0.f, 3.f, 8.f, 0.f, 0.f, 0.f, 78.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Aggressive", {80.f, 48.f, 0.f, 2.f, 45.f, 100.f, 100.f, 100.f, 70.f, 0.f, 15.f, 30.f, 0.f, 12.f, 0.f, 100.f, 20.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Retro Robot", {80.f, 48.f, 0.f, 4.f, 95.f, 100.f, 100.f, 100.f, 25.f, 0.f, 5.f, 8.f, 0.f, 2.f, 0.f, 96.f, 12.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("JURA L / Voice R", {80.f, 48.f, 0.f, 5.f, 85.f, 100.f, 100.f, 100.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("JURA R / Voice L", {80.f, 48.f, 0.f, 5.f, 85.f, 100.f, 100.f, 100.f, 0.f, 2.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   // Additional factory sounds: full plain-value lists preserve native MPC program recall.
   addProgram("Electric Choir", {80.f, 52.f, 4.f, 20.f, 180.f, 100.f, 100.f, 70.f, 30.f, 0.f, 9.f, 12.f, 0.f, 0.f, 0.f, 82.f, 10.f, 80.f, 380.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 1.f, 0.f, 0.35f, 12.f, 0.f});
   addProgram("Neon Talkbox", {80.f, 60.f, 0.f, 3.f, 60.f, 100.f, 100.f, 70.f, 10.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 3.f, 0.f, 5.f, 7.f, 0.f});
   addProgram("Space Transmission", {80.f, 43.f, 0.f, 12.f, 210.f, 100.f, 100.f, 70.f, 55.f, 0.f, 0.f, 22.f, 0.f, 8.f, 0.f, 62.f, 35.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 1.f, 1.8f, 24.f, 0.f});
   addProgram("Analog Whisper", {80.f, 57.f, 0.f, 4.f, 95.f, 100.f, 100.f, 70.f, 10.f, 0.f, 0.f, 0.f, 0.f, 4.f, 0.f, 90.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 2.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Dark Machine", {80.f, 40.f, 2.f, 6.f, 110.f, 100.f, 100.f, 70.f, 70.f, 0.f, 18.f, 32.f, 0.f, 8.f, -1.f, 64.f, 28.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 2.f, 3.2f, 18.f, 0.f});
   addProgram("Funky Pulse", {80.f, 55.f, 2.f, 2.f, 55.f, 100.f, 100.f, 70.f, 80.f, 0.f, 6.f, 15.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 3.f, 1.f, 5.f, 10.f, 0.f});

 }
 void prepare(double sr,int)override{sr_=float(sr);voc_.prepare(sr_);}
 void reset()override{voc_.prepare(sr_);}
 void process(mpc::ProcessContext& ctx)override{
   float level=get(Level)*.01f,wet=get(Mix)*.01f,gain=get(Input)*.01f;
   float carrier=get(Carrier)*.01f;
   const int mode=getInt(Mode);
   const int monitor=getInt(Monitor);
   voc_.setAttackRelease(get(Attack),get(Release));
   voc_.setWet(1.f);voc_.setGate(true);voc_.setChord(getInt(Chord));
   voc_.setTone(get(Tone)*.01f);
   voc_.setWaveform(getInt(Waveform));
   voc_.setLfo(getInt(LfoShape),get(LfoRate),get(LfoDepth));
   voc_.setDetune(get(Detune));voc_.setSubLevel(get(Sub)*.01f);
   voc_.setNoiseLevel(get(Noise)*.01f);voc_.setOctave(int(std::round(get(Octave))));
   voc_.setCarrierFilter(get(Cutoff)*.01f,get(Resonance)*.01f);
   voc_.setSynthEnvelope(get(SynthAttack),get(SynthRelease));
   voc_.setFrequency(440.f*std::pow(2.f,(get(Pitch)-69.f)/12.f));
   for(int i=0;i<24;++i)voc_.setBandLevel(i,get(Band01+i)*.01f);
   const float* l=ctx.inputs[0];
   const float* r=ctx.numInputs>1?ctx.inputs[1]:l;
   float* ol=ctx.outputs[0];float* orr=ctx.numOutputs>1?ctx.outputs[1]:ol;
   for(int n=0;n<ctx.numFrames;++n){
     float dryL=l[n],dryR=r[n];
     if(monitor!=0){
       const float test=monitor==1?dryL:(monitor==2?dryR:0.5f*(dryL+dryR));
       const float sample=std::tanh(test*level);
       ol[n]=sample;if(orr!=ol)orr[n]=sample;
       continue;
     }
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
