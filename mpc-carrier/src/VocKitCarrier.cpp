// VocKit Synth: MIDI instrument using the exact VocKit FX internal carrier DSP.
// No alternate oscillator implementation; src/Vocoder.hpp is shared with FX.
#include "mpc_plugin.h"
#include "Vocoder.hpp"
#include <array>
#include <cmath>
#include <algorithm>
class VocKitCarrier final:public mpc::Plugin {
 enum Param{Level,Waveform,Tone,Chord,Octave,Detune,Sub,Noise,
   PulseWidth,PwmDepth,PwmRate,Osc2Wave,Osc2Level,Osc2Semi,Osc2Detune,OscBlend,
   Osc3Wave,Osc3Level,Osc3Semi,Unison,UnisonDetune,
   Cutoff,Resonance,FilterMode,FilterEnv,SynthAttack,SynthRelease,
   LfoShape,LfoRate,LfoDepth,Drive,NoiseColor,NumParams};
 vockit::Vocoder synth_;
 std::array<bool,128> held_{};
 int active_=-1;
 float sr_=44100.f;
 static mpc::PluginInfo info(){
   mpc::PluginInfo i;i.name="VocKit Synth";i.vendor="Beats of Things";
   i.uniqueId=MPC_FOURCC('B','t','V','C');i.version=1100;
   i.isInstrument=true;i.wantsMidi=true;i.numInputs=0;i.numOutputs=2;return i;
 }
 void fallback(){active_=-1;for(int n=127;n>=0;--n)if(held_[n]){active_=n;break;}}
public:
 VocKitCarrier():mpc::Plugin(info()){
   addParam(mpc::floatParam("level","Level",0.f,100.f,85.f,"%",0));
   addParam(mpc::choiceParam("waveform","Waveform",{"Saw/Pulse","Triangle","Sine","Square"},0));
   addParam(mpc::floatParam("tone","Tone",0.f,100.f,0.f,"%",0));
   addParam(mpc::choiceParam("chord","Chord",{"Single","Octave","Fifth","Minor","Major"},0));
   addParam(mpc::floatParam("octave","Octave",-2.f,2.f,0.f,"",0));
   addParam(mpc::floatParam("detune","Detune",0.f,35.f,0.f,"ct",0));
   addParam(mpc::floatParam("sub","Sub Osc",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("noise","Noise Osc",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("pulseWidth","Pulse Width",10.f,90.f,45.f,"%",0));
   addParam(mpc::floatParam("pwmDepth","PWM Depth",0.f,40.f,0.f,"%",0));
   addParam(mpc::floatParam("pwmRate","PWM Rate",0.f,12.f,0.f,"Hz",1));
   addParam(mpc::choiceParam("osc2Wave","OSC 2 Wave",{"Saw","Triangle","Sine","Square"},0));
   addParam(mpc::floatParam("osc2Level","OSC 2 Level",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("osc2Semi","OSC 2 Semitone",-24.f,24.f,0.f,"st",0));
   addParam(mpc::floatParam("osc2Detune","OSC 2 Detune",-50.f,50.f,0.f,"ct",0));
   addParam(mpc::floatParam("oscBlend","OSC 2 Blend",0.f,100.f,0.f,"%",0));
   addParam(mpc::choiceParam("osc3Wave","OSC 3 Wave",{"Saw","Triangle","Sine","Pulse"},0));
   addParam(mpc::floatParam("osc3Level","OSC 3 Level",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("osc3Semi","OSC 3 Semitone",-24.f,24.f,0.f,"st",0));
   addParam(mpc::choiceParam("unisonVoices","Unison Voices",{"1","2","3","4","5","6","7","8"},0));
   addParam(mpc::floatParam("unisonSpread","Unison Detune",0.f,35.f,0.f,"ct",0));
   addParam(mpc::floatParam("cutoff","Filter Cutoff",0.f,100.f,100.f,"%",0));
   addParam(mpc::floatParam("resonance","Resonance",0.f,95.f,0.f,"%",0));
   addParam(mpc::choiceParam("filterMode","Filter Mode",{"Low Pass","High Pass","Band Pass"},0));
   addParam(mpc::floatParam("filterEnvAmount","Filter Env Amount",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("synthAttack","Synth Attack",1.f,500.f,1.f,"ms",0));
   addParam(mpc::floatParam("synthRelease","Synth Release",5.f,2000.f,5.f,"ms",0));
   addParam(mpc::choiceParam("lfoShape","LFO Shape",{"Sine","Triangle","Square"},0));
   addParam(mpc::floatParam("lfoRate","LFO Rate",0.f,20.f,0.f,"Hz",1));
   addParam(mpc::floatParam("lfoDepth","LFO Depth",0.f,100.f,0.f,"ct",1));
   addParam(mpc::floatParam("carrierDrive","Carrier Drive",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("noiseColor","Noise Color",0.f,100.f,0.f,"%",0));
   addProgram("Init", {85.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Robot Voice", {85.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Clear Rap", {85.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Dark Vocoder", {85.f, 0.f, 45.f, 0.f, 0.f, 0.f, 20.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Saw Brass", {85.f, 0.f, 0.f, 0.f, 0.f, 7.f, 12.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 95.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Soft Strings", {85.f, 0.f, 35.f, 0.f, 0.f, 3.f, 8.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 78.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Aggressive", {85.f, 0.f, 70.f, 0.f, 12.f, 15.f, 30.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 20.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Retro Robot", {85.f, 0.f, 25.f, 0.f, 2.f, 5.f, 8.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 96.f, 0.f, 0.f, 12.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("JURA L / Voice R", {85.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("JURA R / Voice L", {85.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Electric Choir", {85.f, 1.f, 30.f, 4.f, 0.f, 9.f, 12.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 82.f, 0.f, 0.f, 10.f, 80.f, 0.f, 0.35f, 12.f, 0.f, 0.f});
   addProgram("Neon Talkbox", {85.f, 3.f, 10.f, 0.f, 0.f, 3.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 5.f, 7.f, 0.f, 0.f});
   addProgram("Space Transmission", {85.f, 0.f, 55.f, 0.f, 8.f, 0.f, 22.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 62.f, 0.f, 0.f, 35.f, 1.f, 1.f, 1.8f, 24.f, 0.f, 0.f});
   addProgram("Analog Whisper", {85.f, 2.f, 10.f, 0.f, 4.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 90.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Dark Machine", {85.f, 0.f, 70.f, 2.f, 8.f, 18.f, 32.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, -1.f, 64.f, 0.f, 0.f, 28.f, 1.f, 2.f, 3.2f, 18.f, 0.f, 0.f});
   addProgram("Funky Pulse", {85.f, 3.f, 80.f, 2.f, 0.f, 6.f, 15.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 1.f, 5.f, 10.f, 0.f, 0.f});
   addProgram("Electro Robot", {85.f, 0.f, 25.f, 0.f, 0.f, 6.f, 0.f, 0.f, 45.f, 0.f, 0.f, 3.f, 38.f, 12.f, 5.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 92.f, 0.f, 0.f, 9.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Midnight Choir", {85.f, 1.f, 35.f, 4.f, 0.f, 8.f, 15.f, 0.f, 45.f, 0.f, 0.f, 1.f, 28.f, 0.f, -8.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 75.f, 0.f, 0.f, 18.f, 90.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Analog Talker", {85.f, 3.f, 55.f, 0.f, 0.f, 5.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 30.f, -12.f, 7.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 88.f, 0.f, 0.f, 12.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Alter Ego", {85.f, 0.f, 12.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 92.f, 0.f, 0.f, 5.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Band Warm Voice", {85.f, 0.f, 18.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Band Clear Speech", {85.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Band Radio Robot", {85.f, 0.f, 25.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Band Airy Choir", {85.f, 0.f, 30.f, 4.f, 0.f, 7.f, 0.f, 0.f, 45.f, 0.f, 0.f, 1.f, 25.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Warm Chorus", {85.f, 1.f, 35.f, 4.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 30.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Robot Drive", {85.f, 3.f, 30.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Soft Alter Ego", {85.f, 0.f, 15.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Chrome Robot", {85.f, 3.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Electro Funk Lead", {85.f, 0.f, 25.f, 4.f, 0.f, 8.f, 18.f, 0.f, 45.f, 0.f, 0.f, 0.f, 30.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Analog Brass Voice", {85.f, 0.f, 38.f, 2.f, 0.f, 14.f, 20.f, 0.f, 45.f, 0.f, 0.f, 0.f, 25.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 75.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Cathedral Choir", {85.f, 1.f, 10.f, 4.f, 0.f, 0.f, 15.f, 0.f, 45.f, 0.f, 0.f, 0.f, 18.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Dark Transmission", {85.f, 3.f, 55.f, 0.f, 0.f, 0.f, 30.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 65.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("LoFi Broadcast", {85.f, 3.f, 60.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 70.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Whispering Synth", {85.f, 1.f, 20.f, 0.f, 35.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 12.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
   addProgram("Industrial Android", {85.f, 3.f, 75.f, 0.f, 0.f, 0.f, 30.f, 0.f, 45.f, 0.f, 0.f, 0.f, 22.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f});
 }
 void prepare(double sr,int)override{sr_=float(sr);synth_.prepare(sr_);held_.fill(false);active_=-1;}
 void reset()override{synth_.prepare(sr_);held_.fill(false);active_=-1;}
 void process(mpc::ProcessContext& ctx)override{
   synth_.setWet(1.f);synth_.setChord(getInt(Chord));
   synth_.setWaveform(getInt(Waveform));synth_.setTone(get(Tone)*.01f);
   synth_.setDetune(get(Detune));synth_.setSubLevel(get(Sub)*.01f);
   synth_.setNoiseLevel(get(Noise)*.01f);synth_.setOctave(int(std::round(get(Octave))));
   synth_.setLfo(getInt(LfoShape),get(LfoRate),get(LfoDepth));
   synth_.setOsc2(getInt(Osc2Wave),get(Osc2Level)*.01f,get(Osc2Semi),get(Osc2Detune));
   synth_.setCarrierAdvanced(getInt(Unison)+1,get(UnisonDetune),getInt(Osc3Wave),
      get(Osc3Level)*.01f,get(Osc3Semi),getInt(FilterMode),get(FilterEnv)*.01f);
   synth_.setCarrierDesign(get(PulseWidth)*.01f,get(PwmDepth)*.01f,get(PwmRate),
      get(OscBlend)*.01f,get(Drive)*.01f,get(NoiseColor)*.01f);
   synth_.setCarrierFilter(get(Cutoff)*.01f,get(Resonance)*.01f);
   synth_.setSynthEnvelope(get(SynthAttack),get(SynthRelease));
   int ev=0;
   for(int n=0;n<ctx.numFrames;++n){
     while(ctx.midi&&ev<ctx.numMidiEvents&&ctx.midi[ev].sampleOffset<=n){
       const auto& m=ctx.midi[ev++];
       if(m.isAllNotesOff()){held_.fill(false);active_=-1;}
       else if(m.isNoteOn()&&m.note()>=0&&m.note()<128){held_[m.note()]=true;active_=m.note();}
       else if(m.isNoteOff()&&m.note()>=0&&m.note()<128){
         held_[m.note()]=false;if(active_==m.note())fallback();
       }
     }
     synth_.setGate(active_>=0);
     if(active_>=0)synth_.setFrequency(440.f*std::pow(2.f,(float(active_)-69.f)/12.f));
     const float v=synth_.tickCarrier()*get(Level)*.01f;
     if(ctx.numOutputs>0&&ctx.outputs&&ctx.outputs[0])ctx.outputs[0][n]=v;
     if(ctx.numOutputs>1&&ctx.outputs&&ctx.outputs[1])ctx.outputs[1][n]=v;
   }
 }
};
MPC_PLUGIN_ENTRY(VocKitCarrier)
