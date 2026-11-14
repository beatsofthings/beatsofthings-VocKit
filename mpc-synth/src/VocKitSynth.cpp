// VocKit Synth: MIDI carrier with internal WAV modulator (MPC instrument).
// SPDX-License-Identifier: MIT
#include "mpc_plugin.h"
#include "dsp/wav.h"
#include "Vocoder.hpp"
#include <array>
#include <cmath>

class VocKitSynth : public mpc::Plugin {
public:
 enum Params { Level, Source, Loop, Speed, Pitch, Attack, Release, Mix, Tone, Input, Preview, PreviewLevel, Formant, Band01, NumParams=Band01+24 };
 static mpc::PluginInfo makeInfo() {
   mpc::PluginInfo i;
   i.name="VocKit Synth";i.vendor="Beats of Things";
   i.uniqueId=MPC_FOURCC('B','t','V','S');
   i.version=1000;i.isInstrument=true;i.numInputs=0;i.numOutputs=2;
   return i;
 }
 VocKitSynth():mpc::Plugin(makeInfo()) {
   addParam(mpc::floatParam("level","Level",0.f,100.f,80.f,"%",0));
   addParam(mpc::choiceParam("source","WAV Source",{"Pad 1","Pad 2","Pad 3","Pad 4","Pad 5","Pad 6","Pad 7","Pad 8"},0));
   addParam(mpc::boolParam("loop","Loop",true));
   addParam(mpc::floatParam("speed","Speed",25.f,200.f,100.f,"%",0));
   addParam(mpc::intParam("pitch","Transpose",-24,24,0,"st"));
   addParam(mpc::floatParam("attack","Attack",1.f,100.f,8.f,"ms",0));
   addParam(mpc::floatParam("release","Release",10.f,500.f,90.f,"ms",0));
   addParam(mpc::floatParam("mix","Mix",0.f,100.f,100.f,"%",0));
   addParam(mpc::floatParam("tone","Tone",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("input","Input",0.f,200.f,100.f,"%",0));
   addParam(mpc::boolParam("preview","Preview",true));
   addParam(mpc::floatParam("previewlevel","Preview Level",0.f,100.f,80.f,"%",0));
   addParam(mpc::floatParam("formant","Formant Shift",-12.f,12.f,0.f,"st",1));
   for(int i=0;i<24;++i){
     const std::string name="Band "+std::to_string(i+1);
     addParam(mpc::floatParam("band"+std::to_string(i+1),name.c_str(),0.f,200.f,100.f,"%",0));
   }
 }
 void prepare(double sr,int)override {
   sr_=float(sr);
   voc_.prepare(sr_);
   for(int i=0;i<8;++i) {
     const std::string path=mpc::contentPath("Samples/sample"+std::to_string(i+1)+".wav");
     dsp::loadWav(path,samples_[i]);
   }
   reset();
 }
 void reset()override {
   voc_.prepare(sr_);
   playing_=false;position_=0;note_=-1;held_.fill(false);previewPlaying_=false;previewPosition_=0;lastSource_=getInt(Source);
 }
 void process(mpc::ProcessContext& ctx)override {
   voc_.setWet(1.f);
   voc_.setTone(get(Tone)*.01f);
   voc_.setAttackRelease(get(Attack),get(Release));
   voc_.setFormant(get(Formant));
   for(int i=0;i<24;++i)voc_.setBandLevel(i,get(Band01+i)*.01f);
   const int source=getInt(Source);
   if(source!=lastSource_) { lastSource_=source; previewPosition_=0.f;previewPlaying_=getBool(Preview); }
   int pos=0;
   for(int e=0;e<ctx.numMidiEvents;++e) {
     const auto &ev=ctx.midi[e];
     int stop=std::max(pos,std::min(ctx.numFrames,ev.sampleOffset));
     render(ctx,pos,stop);pos=stop;
     if(ev.isNoteOn()) {
       const int n=ev.note();
       if(n>=0&&n<128)held_[n]=true;
       note_=n;position_=0;playing_=true;
     } else if(ev.isNoteOff()) {
       const int n=ev.note();
       if(n>=0&&n<128)held_[n]=false;
       if(note_==n) {
         note_=-1;
         for(int j=127;j>=0;--j)if(held_[j]){note_=j;break;}
       }
       if(note_<0)playing_=false;
     } else if(ev.isAllNotesOff()) {
       held_.fill(false);note_=-1;playing_=false;
     }
   }
   render(ctx,pos,ctx.numFrames);
 }
private:
 void render(mpc::ProcessContext& ctx,int from,int to) {
   float* l=ctx.outputs[0];
   float* r=ctx.numOutputs>1?ctx.outputs[1]:nullptr;
   int idx=std::max(0,std::min(7,getInt(Source)));
   const dsp::WavData &w=samples_[idx];
   const std::size_t frames=w.numFrames();
   const float rate=frames>0?float(w.sampleRate/sr_)*get(Speed)*.01f:0.f;
   const bool looping=getBool(Loop);
   const float gain=get(Level)*.01f;
   const float inputGain=get(Input)*.01f;
   const float wet=get(Mix)*.01f;
   const float pitch=float(note_+getInt(Pitch));
   voc_.setFrequency(440.f*std::pow(2.f,(pitch-69.f)/12.f));
   voc_.setGate(note_>=0);
   for(int i=from;i<to;++i) {
     float mod=0.f;
     if(previewPlaying_ && frames>1) {
       const std::size_t p=std::min<std::size_t>(std::size_t(previewPosition_),frames-1);
       const std::size_t q=std::min(p+1,frames-1);
       const float t=previewPosition_-float(p);
       float raw=w.channels[0][p]*(1.f-t)+w.channels[0][q]*t;
       if(w.numChannels()>1)raw=.5f*(raw+w.channels[1][p]*(1.f-t)+w.channels[1][q]*t);
       mod=raw;
       previewPosition_+=rate;
       if(previewPosition_>=float(frames))previewPlaying_=false;
     }
     if(playing_&&frames>1&&position_<float(frames)) {
       std::size_t p=std::min<std::size_t>(std::size_t(position_),frames-1);
       std::size_t q=std::min(p+1,frames-1);
       float t=position_-float(p);
       mod=w.channels[0][p]*(1.f-t)+w.channels[0][q]*t;
       if(w.numChannels()>1)mod=.5f*(mod+w.channels[1][p]*(1.f-t)+w.channels[1][q]*t);
       position_+=rate;
       if(position_>=float(frames)) {
         if(looping)position_=std::fmod(position_,float(frames));
         else playing_=false;
       }
     }
     float fx=voc_.tick(mod*inputGain);
     // Preview is audible without MIDI; MIDI notes use the vocoded carrier.
     const float previewOut=(previewPlaying_ && note_<0)?mod*get(PreviewLevel)*.01f:0.f;
     float out=std::tanh(((note_>=0?fx*wet+mod*(1.f-wet):previewOut))*gain);
     l[i]=out;if(r)r[i]=out;
   }
 }
 vockit::Vocoder voc_;
 std::array<dsp::WavData,8> samples_{};
 std::array<bool,128> held_{};
 float sr_=44100.f,position_=0.f;
 int note_=-1;
 bool playing_=false,previewPlaying_=false;
 float previewPosition_=0.f;
 int lastSource_=0;
};
MPC_PLUGIN_ENTRY(VocKitSynth)
