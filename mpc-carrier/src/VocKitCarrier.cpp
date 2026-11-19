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
   LfoShape,LfoRate,LfoDepth,Drive,NoiseColor,AmpDecay,AmpSustain,FilterAttack,FilterDecay,FilterSustain,FilterRelease,PolyVoices,ArpMode,ArpRate,ArpGate,ChorusMix,ChorusRate,DelayMix,DelayTime,DelayFeedback,NumParams};
 struct Voice{vockit::Vocoder synth;int note=-1;bool held=false;float amp=0.f;float filt=0.f;int ampStage=0,filterStage=0;unsigned age=0;};
 std::array<Voice,8> voices_{};
 std::array<bool,128> held_{};
 unsigned age_=0;
 float sr_=44100.f,arpPhase_=0.f,chorusPhase_=0.f;
 int arpIndex_=0,arpNote_=-1;
 std::array<float,65536> delay_{};
 int delayCursor_=0;
 static mpc::PluginInfo info(){
   mpc::PluginInfo i;i.name="VocKit Synth";i.vendor="Beats of Things";
   i.uniqueId=MPC_FOURCC('B','t','V','C');i.version=1100;
   i.isInstrument=true;i.wantsMidi=true;i.numInputs=0;i.numOutputs=2;return i;
 }
 void noteOn(int note){
   int slot=-1;unsigned oldest=~0u;
   for(int i=0;i<getInt(PolyVoices)+1;++i){
     if(voices_[i].note==note){slot=i;break;}
     if(!voices_[i].held && voices_[i].amp<.001f){slot=i;break;}
     if(voices_[i].age<oldest){oldest=voices_[i].age;slot=i;}
   }
   auto& v=voices_[slot];v.note=note;v.held=true;v.age=++age_;v.amp=0.f;v.filt=0.f;v.ampStage=1;v.filterStage=1;
 }
 void noteOff(int note){for(auto& v:voices_)if(v.note==note)v.held=false;}
 void allOff(){held_.fill(false);for(auto& v:voices_)v.held=false;arpNote_=-1;}
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
   addParam(mpc::floatParam("ampDecay","Amp Decay",1.f,2000.f,180.f,"ms",0));
   addParam(mpc::floatParam("ampSustain","Amp Sustain",0.f,100.f,75.f,"%",0));
   addParam(mpc::floatParam("filterAttack","Filter Attack",1.f,1000.f,15.f,"ms",0));
   addParam(mpc::floatParam("filterDecay","Filter Decay",1.f,2000.f,250.f,"ms",0));
   addParam(mpc::floatParam("filterSustain","Filter Sustain",0.f,100.f,35.f,"%",0));
   addParam(mpc::floatParam("filterRelease","Filter Release",5.f,3000.f,350.f,"ms",0));
   addParam(mpc::choiceParam("polyVoices","Poly Voices",{"1","2","3","4","5","6","7","8"},7));
   addParam(mpc::choiceParam("arpMode","Arp Mode",{"Off","Up","Down","UpDown"},0));
   addParam(mpc::choiceParam("arpRate","Arp Rate",{"1/4","1/8","1/16","1/32"},2));
   addParam(mpc::floatParam("arpGate","Arp Gate",10.f,100.f,70.f,"%",0));
   addParam(mpc::floatParam("chorusMix","Synth Chorus",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("chorusRate","Chorus Speed",0.1f,5.f,0.8f,"Hz",1));
   addParam(mpc::floatParam("delayMix","Synth Delay",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("delayTime","Delay Time",60.f,800.f,300.f,"ms",0));
   addParam(mpc::floatParam("delayFeedback","Delay Feedback",0.f,85.f,25.f,"%",0));
   addProgram("Init", {85.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 85.f, 10.f, 0.f, 8.f, 8.f, 280.f, 0.f, 0.f, 0.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Robot Voice", {85.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 81.f, 29.f, 0.f, 18.f, 7.f, 230.f, 0.f, 0.f, 0.f, 0.f, 0.f, 230.f, 48.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 10.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Clear Rap", {85.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 70.f, 14.f, 0.f, 8.f, 16.f, 450.f, 0.f, 0.f, 0.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Dark Vocoder", {85.f, 0.f, 45.f, 0.f, 0.f, 0.f, 20.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 70.f, 14.f, 0.f, 8.f, 16.f, 450.f, 0.f, 0.f, 0.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Saw Brass", {85.f, 0.f, 0.f, 0.f, 0.f, 7.f, 12.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 76.f, 23.f, 0.f, 30.f, 18.f, 260.f, 0.f, 0.f, 0.f, 0.f, 0.f, 170.f, 42.f, 12.f, 210.f, 26.f, 340.f, 7.f, 0.f, 2.f, 70.f, 6.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Soft Strings", {85.f, 0.f, 35.f, 0.f, 0.f, 3.f, 8.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 73.f, 9.f, 0.f, 12.f, 220.f, 1050.f, 0.f, 0.f, 0.f, 0.f, 0.f, 750.f, 78.f, 180.f, 850.f, 65.f, 1200.f, 7.f, 0.f, 2.f, 70.f, 28.f, 0.8f, 12.f, 300.f, 25.f});
   addProgram("Aggressive", {85.f, 0.f, 70.f, 0.f, 12.f, 15.f, 30.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 87.f, 21.f, 0.f, 15.f, 4.f, 290.f, 0.f, 0.f, 0.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 12.f, 0.8f, 10.f, 300.f, 25.f});
   addProgram("Retro Robot", {85.f, 0.f, 25.f, 0.f, 2.f, 5.f, 8.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 81.f, 29.f, 0.f, 18.f, 7.f, 230.f, 0.f, 0.f, 0.f, 0.f, 0.f, 230.f, 48.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 10.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("JURA L / Voice R", {85.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 70.f, 14.f, 0.f, 8.f, 16.f, 450.f, 0.f, 0.f, 0.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("JURA R / Voice L", {85.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 70.f, 14.f, 0.f, 8.f, 16.f, 450.f, 0.f, 0.f, 0.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Electric Choir", {85.f, 1.f, 30.f, 4.f, 0.f, 9.f, 12.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 73.f, 9.f, 0.f, 12.f, 220.f, 1050.f, 0.f, 0.35f, 12.f, 0.f, 0.f, 750.f, 78.f, 180.f, 850.f, 65.f, 1200.f, 7.f, 0.f, 2.f, 70.f, 28.f, 0.8f, 12.f, 300.f, 25.f});
   addProgram("Neon Talkbox", {85.f, 3.f, 10.f, 0.f, 0.f, 3.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 87.f, 21.f, 0.f, 15.f, 4.f, 290.f, 0.f, 5.f, 7.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 12.f, 0.8f, 10.f, 300.f, 25.f});
   addProgram("Space Transmission", {85.f, 0.f, 55.f, 0.f, 8.f, 0.f, 22.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 58.f, 33.f, 0.f, 8.f, 16.f, 450.f, 1.f, 1.8f, 24.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Analog Whisper", {85.f, 2.f, 10.f, 0.f, 4.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 73.f, 9.f, 0.f, 12.f, 220.f, 1050.f, 0.f, 0.f, 0.f, 0.f, 0.f, 750.f, 78.f, 180.f, 850.f, 65.f, 1200.f, 7.f, 0.f, 2.f, 70.f, 28.f, 0.8f, 12.f, 300.f, 25.f});
   addProgram("Dark Machine", {85.f, 0.f, 70.f, 2.f, 8.f, 18.f, 32.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 81.f, 29.f, 0.f, 18.f, 7.f, 230.f, 2.f, 3.2f, 18.f, 0.f, 0.f, 230.f, 48.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 10.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Funky Pulse", {85.f, 3.f, 80.f, 2.f, 0.f, 6.f, 15.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 62.f, 17.f, 0.f, 23.f, 3.f, 170.f, 1.f, 5.f, 10.f, 0.f, 0.f, 125.f, 60.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Electro Robot", {85.f, 0.f, 25.f, 0.f, 0.f, 6.f, 0.f, 0.f, 45.f, 0.f, 0.f, 3.f, 38.f, 12.f, 5.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 81.f, 29.f, 0.f, 18.f, 7.f, 230.f, 0.f, 0.f, 0.f, 0.f, 0.f, 230.f, 48.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 10.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Midnight Choir", {85.f, 1.f, 35.f, 4.f, 0.f, 8.f, 15.f, 0.f, 45.f, 0.f, 0.f, 1.f, 28.f, 0.f, -8.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 73.f, 9.f, 0.f, 12.f, 220.f, 1050.f, 0.f, 0.f, 0.f, 0.f, 0.f, 750.f, 78.f, 180.f, 850.f, 65.f, 1200.f, 7.f, 0.f, 2.f, 70.f, 28.f, 0.8f, 12.f, 300.f, 25.f});
   addProgram("Analog Talker", {85.f, 3.f, 55.f, 0.f, 0.f, 5.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 30.f, -12.f, 7.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 70.f, 14.f, 0.f, 8.f, 16.f, 450.f, 0.f, 0.f, 0.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Alter Ego", {85.f, 0.f, 12.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 87.f, 21.f, 0.f, 15.f, 4.f, 290.f, 0.f, 0.f, 0.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 12.f, 0.8f, 10.f, 300.f, 25.f});
   addProgram("Band Warm Voice", {85.f, 0.f, 18.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 70.f, 14.f, 0.f, 8.f, 16.f, 450.f, 0.f, 0.f, 0.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Band Clear Speech", {85.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 70.f, 14.f, 0.f, 8.f, 16.f, 450.f, 0.f, 0.f, 0.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Band Radio Robot", {85.f, 0.f, 25.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 81.f, 29.f, 0.f, 18.f, 7.f, 230.f, 0.f, 0.f, 0.f, 0.f, 0.f, 230.f, 48.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 10.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Band Airy Choir", {85.f, 0.f, 30.f, 4.f, 0.f, 7.f, 0.f, 0.f, 45.f, 0.f, 0.f, 1.f, 25.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 73.f, 9.f, 0.f, 12.f, 220.f, 1050.f, 0.f, 0.f, 0.f, 0.f, 0.f, 750.f, 78.f, 180.f, 850.f, 65.f, 1200.f, 7.f, 0.f, 2.f, 70.f, 28.f, 0.8f, 12.f, 300.f, 25.f});
   addProgram("Warm Chorus", {85.f, 1.f, 35.f, 4.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 30.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 70.f, 14.f, 0.f, 8.f, 16.f, 450.f, 0.f, 0.f, 0.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Robot Drive", {85.f, 3.f, 30.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 81.f, 29.f, 0.f, 18.f, 7.f, 230.f, 0.f, 0.f, 0.f, 0.f, 0.f, 230.f, 48.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 10.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Soft Alter Ego", {85.f, 0.f, 15.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 73.f, 9.f, 0.f, 12.f, 220.f, 1050.f, 0.f, 0.f, 0.f, 0.f, 0.f, 750.f, 78.f, 180.f, 850.f, 65.f, 1200.f, 7.f, 0.f, 2.f, 70.f, 28.f, 0.8f, 12.f, 300.f, 25.f});
   addProgram("Chrome Robot", {85.f, 3.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 81.f, 29.f, 0.f, 18.f, 7.f, 230.f, 0.f, 0.f, 0.f, 0.f, 0.f, 230.f, 48.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 10.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Electro Funk Lead", {85.f, 0.f, 25.f, 4.f, 0.f, 8.f, 18.f, 0.f, 45.f, 0.f, 0.f, 0.f, 30.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 62.f, 17.f, 0.f, 23.f, 3.f, 170.f, 0.f, 0.f, 0.f, 0.f, 0.f, 125.f, 60.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 12.f, 0.8f, 10.f, 300.f, 25.f});
   addProgram("Analog Brass Voice", {85.f, 0.f, 38.f, 2.f, 0.f, 14.f, 20.f, 0.f, 45.f, 0.f, 0.f, 0.f, 25.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 76.f, 23.f, 0.f, 30.f, 18.f, 260.f, 0.f, 0.f, 0.f, 0.f, 0.f, 170.f, 42.f, 12.f, 210.f, 26.f, 340.f, 7.f, 0.f, 2.f, 70.f, 6.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Cathedral Choir", {85.f, 1.f, 10.f, 4.f, 0.f, 0.f, 15.f, 0.f, 45.f, 0.f, 0.f, 0.f, 18.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 73.f, 9.f, 0.f, 12.f, 220.f, 1050.f, 0.f, 0.f, 0.f, 0.f, 0.f, 750.f, 78.f, 180.f, 850.f, 65.f, 1200.f, 7.f, 0.f, 2.f, 70.f, 28.f, 0.8f, 12.f, 300.f, 25.f});
   addProgram("Dark Transmission", {85.f, 3.f, 55.f, 0.f, 0.f, 0.f, 30.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 58.f, 33.f, 0.f, 8.f, 16.f, 450.f, 0.f, 0.f, 0.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("LoFi Broadcast", {85.f, 3.f, 60.f, 0.f, 0.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 58.f, 33.f, 0.f, 8.f, 16.f, 450.f, 0.f, 0.f, 0.f, 0.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("Whispering Synth", {85.f, 1.f, 20.f, 0.f, 35.f, 0.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 12.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 73.f, 9.f, 0.f, 12.f, 220.f, 1050.f, 0.f, 0.f, 0.f, 0.f, 0.f, 750.f, 78.f, 180.f, 850.f, 65.f, 1200.f, 7.f, 0.f, 2.f, 70.f, 28.f, 0.8f, 12.f, 300.f, 25.f});
   addProgram("Industrial Android", {85.f, 3.f, 75.f, 0.f, 0.f, 0.f, 30.f, 0.f, 45.f, 0.f, 0.f, 0.f, 22.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 81.f, 29.f, 0.f, 18.f, 7.f, 230.f, 0.f, 0.f, 0.f, 0.f, 0.f, 230.f, 48.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 10.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("BoT Funk Bass", {85.f, 0.f, 25.f, 0.f, 0.f, 8.f, 30.f, 0.f, 38.f, 15.f, 0.f, 0.f, 45.f, -12.f, 0.f, 0.f, 3.f, 22.f, 0.f, 2.f, 9.f, 62.f, 17.f, 0.f, 23.f, 3.f, 170.f, 0.f, 0.f, 0.f, 12.f, 0.f, 125.f, 60.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("BoT Analog Brass", {85.f, 0.f, 38.f, 2.f, 0.f, 12.f, 0.f, 0.f, 45.f, 0.f, 0.f, 0.f, 50.f, 7.f, 0.f, 0.f, 0.f, 28.f, 0.f, 3.f, 12.f, 76.f, 23.f, 0.f, 30.f, 18.f, 260.f, 0.f, 0.f, 0.f, 15.f, 0.f, 170.f, 42.f, 12.f, 210.f, 26.f, 340.f, 7.f, 0.f, 2.f, 70.f, 6.f, 0.8f, 0.f, 300.f, 25.f});
   addProgram("BoT Robot Lead", {85.f, 3.f, 65.f, 0.f, 0.f, 6.f, 12.f, 0.f, 28.f, 22.f, 4.f, 0.f, 22.f, 12.f, 0.f, 0.f, 3.f, 18.f, 0.f, 1.f, 6.f, 81.f, 29.f, 0.f, 18.f, 7.f, 230.f, 0.f, 4.f, 12.f, 28.f, 0.f, 230.f, 48.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 10.f, 0.8f, 10.f, 300.f, 25.f});
   addProgram("BoT Poly Dream", {85.f, 1.f, 25.f, 0.f, 0.f, 9.f, 30.f, 0.f, 38.f, 15.f, 0.f, 0.f, 40.f, 12.f, 0.f, 0.f, 3.f, 32.f, 0.f, 3.f, 11.f, 73.f, 9.f, 0.f, 12.f, 220.f, 1050.f, 0.f, 0.f, 0.f, 12.f, 0.f, 750.f, 78.f, 180.f, 850.f, 65.f, 1200.f, 7.f, 0.f, 2.f, 70.f, 28.f, 0.5.f, 12.f, 380.f, 35.f});
   addProgram("BoT Arp Laser", {85.f, 3.f, 62.f, 0.f, 0.f, 12.f, 30.f, 0.f, 38.f, 35.f, 0.f, 0.f, 28.f, 12.f, 0.f, 0.f, 3.f, 22.f, 0.f, 2.f, 9.f, 87.f, 21.f, 0.f, 15.f, 4.f, 290.f, 0.f, 0.f, 0.f, 12.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 1.f, 3.f, 42.f, 12.f, 1.4.f, 24.f, 190.f, 36.f});
   addProgram("BoT Funk Chords", {85.f, 0.f, 32.f, 0.f, 0.f, 6.f, 30.f, 0.f, 38.f, 15.f, 0.f, 0.f, 28.f, 7.f, 0.f, 0.f, 3.f, 22.f, 0.f, 2.f, 8.f, 70.f, 14.f, 0.f, 8.f, 16.f, 450.f, 0.f, 0.f, 0.f, 12.f, 0.f, 240.f, 68.f, 5.f, 180.f, 45.f, 260.f, 7.f, 0.f, 2.f, 70.f, 0.f, 0.9.f, 0.f, 250.f, 20.f});
   addProgram("BoT Soft Poly Pad", {85.f, 1.f, 15.f, 0.f, 0.f, 5.f, 30.f, 0.f, 38.f, 15.f, 0.f, 0.f, 35.f, 12.f, 0.f, 0.f, 3.f, 25.f, 0.f, 4.f, 12.f, 73.f, 9.f, 0.f, 12.f, 220.f, 1050.f, 0.f, 0.f, 0.f, 12.f, 0.f, 750.f, 78.f, 180.f, 850.f, 65.f, 1200.f, 7.f, 0.f, 2.f, 70.f, 28.f, 0.35.f, 12.f, 500.f, 40.f});
   addProgram("BoT Robot Arp", {85.f, 2.f, 50.f, 0.f, 0.f, 14.f, 30.f, 0.f, 38.f, 40.f, 0.f, 0.f, 40.f, 0.f, 0.f, 0.f, 3.f, 22.f, 0.f, 2.f, 9.f, 81.f, 29.f, 0.f, 18.f, 7.f, 230.f, 0.f, 0.f, 0.f, 12.f, 0.f, 230.f, 48.f, 5.f, 180.f, 45.f, 260.f, 7.f, 1.f, 2.f, 62.f, 10.f, 1.1.f, 24.f, 280.f, 44.f});
 }
 void prepare(double sr,int)override{
   sr_=float(sr);for(auto& v:voices_){v.synth.prepare(sr_);v.note=-1;v.held=false;v.amp=0.f;v.filt=0.f;v.ampStage=0;v.filterStage=0;}
   held_.fill(false);arpPhase_=0.f;arpIndex_=0;arpNote_=-1;delay_.fill(0.f);delayCursor_=0;
 }
 void reset()override{prepare(sr_,0);}
 void process(mpc::ProcessContext& ctx)override{
   for(auto& v:voices_){
     auto& e=v.synth;
     e.setWet(1.f);e.setChord(getInt(Chord));e.setWaveform(getInt(Waveform));
     e.setTone(get(Tone)*.01f);e.setDetune(get(Detune));
     e.setSubLevel(get(Sub)*.01f);e.setNoiseLevel(get(Noise)*.01f);
     e.setOctave(int(std::round(get(Octave))));
     e.setLfo(getInt(LfoShape),get(LfoRate),get(LfoDepth));
     e.setOsc2(getInt(Osc2Wave),get(Osc2Level)*.01f,get(Osc2Semi),get(Osc2Detune));
     e.setCarrierAdvanced(getInt(Unison)+1,get(UnisonDetune),getInt(Osc3Wave),
       get(Osc3Level)*.01f,get(Osc3Semi),getInt(FilterMode),get(FilterEnv)*.01f);
     e.setCarrierDesign(get(PulseWidth)*.01f,get(PwmDepth)*.01f,get(PwmRate),
       get(OscBlend)*.01f,get(Drive)*.01f,get(NoiseColor)*.01f);
     e.setSynthEnvelope(get(SynthAttack),get(SynthRelease));
   }
   int ev=0;
   for(int n=0;n<ctx.numFrames;++n){
     while(ctx.midi&&ev<ctx.numMidiEvents&&ctx.midi[ev].sampleOffset<=n){
       const auto& m=ctx.midi[ev++];
       if(m.isAllNotesOff())allOff();
       else if(m.isNoteOn()&&m.note()>=0&&m.note()<128){
         held_[m.note()]=true;if(getInt(ArpMode)==0)noteOn(m.note());
       }else if(m.isNoteOff()&&m.note()>=0&&m.note()<128){
         held_[m.note()]=false;if(getInt(ArpMode)==0)noteOff(m.note());
       }
     }
     if(getInt(ArpMode)!=0){
       int notes[128],count=0;
       for(int k=0;k<128;++k)if(held_[k])notes[count++]=k;
       const float bpm=ctx.transport.tempoValid?float(ctx.transport.bpm):120.f;
       const float advance=(bpm>0.f?bpm:120.f)*float(1<<getInt(ArpRate))/(60.f*sr_);
       arpPhase_+=advance;
       const bool arpStep=arpPhase_>=1.f;
       if(count==0){if(arpNote_>=0)noteOff(arpNote_);arpNote_=-1;arpPhase_=0.f;arpIndex_=0;}
       else{
         if((arpNote_<0&&arpIndex_==0)||arpStep){
           if(arpNote_>=0)noteOff(arpNote_);
           if(arpPhase_>=1.f)arpPhase_-=std::floor(arpPhase_);
           int index=arpIndex_++%count;
           if(getInt(ArpMode)==2)index=count-1-index;
           if(getInt(ArpMode)==3){int period=std::max(1,2*count-2);int j=(arpIndex_-1)%period;index=j<count?j:period-j;}
           arpNote_=notes[index];noteOn(arpNote_);
         }
         if(arpPhase_>get(ArpGate)*.01f&&arpNote_>=0){noteOff(arpNote_);arpNote_=-1;}
       }
     }
     float sum=0.f;
     const int poly=getInt(PolyVoices)+1;
     for(int i=0;i<poly;++i){
       auto& v=voices_[i];
       if(v.note<0)continue;
       if(!v.held){v.ampStage=4;v.filterStage=4;}
       float ampTarget=0.f,ampTime=get(SynthRelease);
       if(v.ampStage==1){ampTarget=1.f;ampTime=get(SynthAttack);}
       else if(v.ampStage==2){ampTarget=get(AmpSustain)*.01f;ampTime=get(AmpDecay);}
       else if(v.ampStage==3){ampTarget=get(AmpSustain)*.01f;ampTime=1.f;}
       const float ac=std::exp(-1000.f/(sr_*std::max(1.f,ampTime)));
       v.amp=ac*v.amp+(1.f-ac)*ampTarget;
       if(v.ampStage==1&&v.amp>.995f)v.ampStage=2;
       if(v.ampStage==2&&std::fabs(v.amp-ampTarget)<.002f)v.ampStage=3;
       float filterTarget=0.f,filterTime=get(FilterRelease);
       if(v.filterStage==1){filterTarget=1.f;filterTime=get(FilterAttack);}
       else if(v.filterStage==2){filterTarget=get(FilterSustain)*.01f;filterTime=get(FilterDecay);}
       else if(v.filterStage==3){filterTarget=get(FilterSustain)*.01f;filterTime=1.f;}
       const float fc=std::exp(-1000.f/(sr_*std::max(1.f,filterTime)));
       v.filt=fc*v.filt+(1.f-fc)*filterTarget;
       if(v.filterStage==1&&v.filt>.995f)v.filterStage=2;
       if(v.filterStage==2&&std::fabs(v.filt-filterTarget)<.002f)v.filterStage=3;
       v.synth.setGate(v.held||v.amp>.0001f);
       v.synth.setFrequency(440.f*std::pow(2.f,(float(v.note)-69.f)/12.f));
       const float cutoff=std::max(0.f,std::min(1.f,get(Cutoff)*.01f+get(FilterEnv)*.01f*(v.filt-.5f)));
       v.synth.setCarrierFilter(cutoff,get(Resonance)*.01f);
       sum+=v.synth.tickCarrier()*v.amp;
       if(!v.held&&v.amp<.0001f)v.note=-1;
     }
     const float dry=std::tanh(sum*get(Level)*.01f/std::sqrt(float(std::max(1,poly))));
     chorusPhase_+=get(ChorusRate)/sr_;if(chorusPhase_>=1.f)chorusPhase_-=1.f;
     const float wobble=std::sin(6.2831853f*chorusPhase_);
     const int chorusTap=(delayCursor_-int(sr_*(.018f+.006f*wobble))+65536)&65535;
     const float chorus=delay_[chorusTap];
     const float cm=get(ChorusMix)*.01f;
     const float signal=dry*(1.f-cm*.5f)+chorus*cm*.5f;
     const int delayTap=(delayCursor_-int(std::max(1.f,std::min(65535.f,sr_*get(DelayTime)*.001f)))+65536)&65535;
     const float echo=delay_[delayTap];
     const float dm=get(DelayMix)*.01f;
     const float out=signal*(1.f-dm)+echo*dm;
     delay_[delayCursor_]=std::tanh(signal+echo*get(DelayFeedback)*.01f);
     delayCursor_=(delayCursor_+1)&65535;
     if(ctx.numOutputs>0&&ctx.outputs&&ctx.outputs[0])ctx.outputs[0][n]=out;
     if(ctx.numOutputs>1&&ctx.outputs&&ctx.outputs[1])ctx.outputs[1][n]=out;
   }
 }
};
MPC_PLUGIN_ENTRY(VocKitCarrier)
