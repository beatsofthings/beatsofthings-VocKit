// VocKit stereo effect: audio modulator, internal chord-capable carrier.
// SPDX-License-Identifier: MIT
#include "mpc_plugin.h"
#include "Vocoder.hpp"
#include "PostFx.hpp"
#include "SpectrumSnapshot.hpp"
#include <cmath>
#include <cstdio>
class VocKit:public mpc::Plugin {
public:
 enum Param{Level,Pitch,Chord,Attack,Release,Mix,Input,Carrier,Tone,Mode,Detune,Sub,Help,Noise,Octave,Cutoff,Resonance,SynthAttack,SynthRelease,Band01,Waveform=Band01+24,LfoShape,LfoRate,LfoDepth,Monitor,Osc2Wave,Osc2Level,Osc2Semi,Osc2Detune,Sibilance,FormantShift,Saturation,ChorusMix,ChorusRate,ChorusDepth,BandProfile,ProfileAmount,VoiceCharacter,RingMix,RingFreq,VoiceSensitivity,VoiceGate,SpectralSmooth,UnvoicedMix,CarrierBrightness,BandSpread,SpectralTilt,LowBandTime,HighBandTime,DynamicRange,StereoWidth,PulseWidth,PwmDepth,PwmRate,OscBlend,CarrierDrive,NoiseColor,UnisonVoices,UnisonSpread,Osc3Wave,Osc3Level,Osc3Semi,FilterMode,FilterEnvAmount,MidiCarrierMode,SeqEnable,SeqDivision,SeqLength,SeqStep01,KeyOctave=SeqStep01+16,Key01,PadAudioMode=Key01+12,PadBaseNote,KeyRecordArm,VoiceDesigner,SpectralFreeze,NumParams};
 static mpc::PluginInfo makeInfo(){
   mpc::PluginInfo i;
   i.name="VocKit";i.vendor="Beats of Things";i.uniqueId=MPC_FOURCC('B','t','V','K');
#ifdef VOcKIT_HYBRID
   i.name="VocKit Hybrid";i.uniqueId=MPC_FOURCC('B','t','V','H');
   i.version=1000;i.isInstrument=true;i.wantsMidi=true;i.numInputs=2;i.numOutputs=2;
#else
   i.version=1100;i.isInstrument=false;i.wantsMidi=true;i.numInputs=2;i.numOutputs=2;
#endif
   return i;
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
   addParam(mpc::choiceParam("mode","Input Mode",{"L Carrier / R Voice","R Carrier / L Voice"},0));
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
   addParam(mpc::choiceParam("osc2Wave","OSC 2 Wave",{"Saw","Triangle","Sine","Square"},0));
   addParam(mpc::floatParam("osc2Level","OSC 2 Level",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("osc2Semi","OSC 2 Semitone",-24.f,24.f,0.f,"st",0));
   addParam(mpc::floatParam("osc2Detune","OSC 2 Detune",-50.f,50.f,0.f,"ct",0));
   addParam(mpc::floatParam("sibilance","Consonant Clarity",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("formantShift","Formant Shift",-12.f,12.f,0.f,"st",1));
   addParam(mpc::floatParam("saturation","Saturation",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("chorusMix","Chorus Mix",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("chorusRate","Chorus Rate",0.1f,5.f,0.8f,"Hz",1));
   addParam(mpc::floatParam("chorusDepth","Chorus Depth",0.f,100.f,50.f,"%",0));
   addParam(mpc::choiceParam("bandProfile","Band Profile",{"Manual","Warm","Clear","Radio","Airy"},0));
   addParam(mpc::floatParam("profileAmount","Profile Amount",0.f,100.f,100.f,"%",0));
   addParam(mpc::floatParam("voiceCharacter","Voice Character",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("ringMix","Ring Mix",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("ringFreq","Ring Frequency",10.f,1000.f,80.f,"Hz",0));
   addParam(mpc::floatParam("voiceSensitivity","Band Sensitivity",25.f,400.f,100.f,"%",0));
   addParam(mpc::floatParam("voiceGate","Voice Threshold",-80.f,-10.f,-80.f,"dB",0));
   addParam(mpc::floatParam("spectralSmooth","Spectral Smoothing",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("unvoicedMix","Unvoiced Mix",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("carrierBrightness","Carrier EQ", -100.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("bandSpread","Band Spread",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("spectralTilt","Spectral Tilt",-100.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("lowBandTime","Low Band Time",25.f,400.f,100.f,"%",0));
   addParam(mpc::floatParam("highBandTime","High Band Time",25.f,400.f,100.f,"%",0));
   addParam(mpc::floatParam("dynamicRange","Dynamic Range",50.f,150.f,100.f,"%",0));
   addParam(mpc::floatParam("stereoWidth","Stereo Width",0.f,200.f,100.f,"%",0));
   addParam(mpc::floatParam("pulseWidth","Pulse Width",10.f,90.f,45.f,"%",0));
   addParam(mpc::floatParam("pwmDepth","PWM Depth",0.f,40.f,0.f,"%",0));
   addParam(mpc::floatParam("pwmRate","PWM Rate",0.f,12.f,0.f,"Hz",1));
   addParam(mpc::floatParam("oscBlend","OSC 2 Blend",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("carrierDrive","Carrier Drive",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("noiseColor","Noise Color",0.f,100.f,0.f,"%",0));
   addParam(mpc::choiceParam("unisonVoices","Unison Voices",{"1","2","3","4","5","6","7","8"},0));
   addParam(mpc::floatParam("unisonSpread","Unison Detune",0.f,35.f,0.f,"ct",0));
   addParam(mpc::choiceParam("osc3Wave","OSC 3 Wave",{"Saw","Triangle","Sine","Pulse"},0));
   addParam(mpc::floatParam("osc3Level","OSC 3 Level",0.f,100.f,0.f,"%",0));
   addParam(mpc::floatParam("osc3Semi","OSC 3 Semitone",-24.f,24.f,0.f,"st",0));
   addParam(mpc::choiceParam("filterMode","Filter Mode",{"Lowpass","Highpass","Bandpass"},0));
   addParam(mpc::floatParam("filterEnvAmount","Filter Env Amount",0.f,100.f,0.f,"%",0));
   addParam(mpc::choiceParam("midiCarrierMode","Carrier Note Mode",{"Fixed Pitch","MIDI Notes","MIDI Test Tone"},0));
   addParam(mpc::choiceParam("seqEnable","Sequencer",{"Off","On"},0));
   addParam(mpc::choiceParam("seqDivision","Seq Rate",{"1/4","1/8","1/16","1/32"},2));
   addParam(mpc::floatParam("seqLength","Seq Length",1.f,16.f,16.f,"steps",0));
   for(int step=0;step<16;++step){
     char id[20],name[20];
     std::snprintf(id,sizeof(id),"seqStep%02d",step+1);
     std::snprintf(name,sizeof(name),"Step %02d Note",step+1);
     addParam(mpc::floatParam(id,name,0.f,84.f,step==0?48.f:0.f,"MIDI",0));
   }
   addParam(mpc::floatParam("keyOctave","Keyboard Octave",2.f,6.f,4.f,"",0));
   static const char* keyNames[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
   for(int k=0;k<12;++k){
     char id[20],name[24];
     std::snprintf(id,sizeof(id),"key%02d",k+1);
     std::snprintf(name,sizeof(name),"Key %s",keyNames[k]);
     addParam(mpc::choiceParam(id,name,{"Off","On"},0));
   }
   addParam(mpc::choiceParam("padAudioMode","Pad Audio Input",{"Off","4 Tone Pilot"},0));
   addParam(mpc::floatParam("padBaseNote","Pad Base Note",36.f,72.f,48.f,"MIDI",0));
   addParam(mpc::choiceParam("keyRecordArm","Key Record",{"Off","Arm"},0));
   addParam(mpc::choiceParam("voiceDesigner","Voice Designer",{"Neutral","Robot","Deep","Whisper","Radio","Choir"},0));
   addParam(mpc::choiceParam("spectralFreeze","Spectral Freeze",{"Off","Hold"},0));
   // Plain values, exact parameter order; 24 band gains follow setup help.
   addProgram("Natural Speech", {80.f, 48.f, 0.f, 3.f, 65.f, 100.f, 100.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 25.f, 0.f, 0.f, 0.f, 0.8f, 50.f, 0.f, 100.f, 0.f, 0.f, 80.f, 100.f, -80.f, 12.f, 18.f, 0.f, 0.f, 0.f, 100.f, 100.f, 100.f, 100.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 2.f, 16.f, 48.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 48.f, 0.f, 0.f, 0.f});
   addProgram("Robot Clarity", {80.f, 48.f, 0.f, 2.f, 50.f, 100.f, 100.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 38.f, 0.f, 0.f, 0.f, 0.8f, 50.f, 0.f, 100.f, 45.f, 8.f, 80.f, 100.f, -80.f, 4.f, 0.f, 0.f, 0.f, 0.f, 100.f, 100.f, 100.f, 100.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 2.f, 16.f, 48.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 48.f, 0.f, 0.f, 0.f});
   addProgram("Warm Vocoder", {80.f, 48.f, 0.f, 12.f, 140.f, 100.f, 100.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.8f, 50.f, 1.f, 85.f, 0.f, 0.f, 80.f, 100.f, -80.f, 0.f, 0.f, 0.f, 0.f, -15.f, 100.f, 100.f, 100.f, 100.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 2.f, 16.f, 48.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 48.f, 0.f, 0.f, 0.f});
   addProgram("Bright Rap", {80.f, 48.f, 0.f, 2.f, 38.f, 100.f, 100.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 55.f, 0.f, 0.f, 0.f, 0.8f, 50.f, 2.f, 90.f, 0.f, 0.f, 80.f, 100.f, -80.f, 0.f, 30.f, 0.f, 0.f, 0.f, 100.f, 100.f, 100.f, 100.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 2.f, 16.f, 48.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 48.f, 0.f, 0.f, 0.f});
   addProgram("Radio Voice", {80.f, 48.f, 0.f, 5.f, 90.f, 100.f, 100.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 20.f, 0.f, 0.8f, 50.f, 3.f, 100.f, 0.f, 0.f, 80.f, 100.f, -80.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 100.f, 100.f, 100.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 2.f, 16.f, 48.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 48.f, 0.f, 0.f, 0.f});
   addProgram("Airy Choir", {80.f, 48.f, 0.f, 24.f, 240.f, 100.f, 100.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 30.f, 0.8f, 40.f, 4.f, 75.f, 0.f, 0.f, 80.f, 100.f, -80.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 100.f, 100.f, 100.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 2.f, 16.f, 48.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 48.f, 0.f, 0.f, 0.f});
   addProgram("Dark Machine", {80.f, 48.f, 0.f, 6.f, 115.f, 100.f, 100.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.8f, 50.f, 0.f, 100.f, 70.f, 15.f, 80.f, 100.f, -80.f, 0.f, 0.f, 0.f, 0.f, -40.f, 100.f, 100.f, 100.f, 100.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 2.f, 16.f, 48.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 48.f, 0.f, 0.f, 0.f});
   addProgram("Electro Funk", {80.f, 48.f, 0.f, 3.f, 65.f, 100.f, 100.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 20.f, 0.f, 25.f, 14.f, 0.8f, 50.f, 0.f, 100.f, 0.f, 10.f, 80.f, 100.f, -80.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 100.f, 100.f, 100.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 2.f, 16.f, 48.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 48.f, 0.f, 0.f, 0.f});
   addProgram("BoT Deep Robot", {80.f, 48.f, 0.f, 4.f, 120.f, 100.f, 100.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 30.f, -4.f, 22.f, 0.f, 0.8f, 50.f, 0.f, 100.f, 0.f, 0.f, 80.f, 100.f, -80.f, 15.f, 18.f, 0.f, 0.f, 0.f, 100.f, 100.f, 100.f, 100.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 2.f, 16.f, 48.f, 0.f, 0.f, 0.f, 2.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 48.f, 0.f, 0.f, 0.f});
   addProgram("BoT Whisper", {80.f, 48.f, 0.f, 2.f, 80.f, 100.f, 100.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 60.f, 0.f, 0.f, 0.f, 0.8f, 50.f, 0.f, 100.f, 0.f, 0.f, 80.f, 100.f, -55.f, 12.f, 45.f, 0.f, 0.f, 0.f, 100.f, 100.f, 100.f, 100.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 2.f, 16.f, 48.f, 0.f, 0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 48.f, 0.f, 0.f, 0.f});
   addProgram("BoT Choir", {80.f, 48.f, 0.f, 30.f, 280.f, 100.f, 100.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 25.f, 0.f, 0.f, 48.f, 0.8f, 65.f, 0.f, 100.f, 0.f, 0.f, 80.f, 100.f, -80.f, 45.f, 18.f, 0.f, 0.f, 0.f, 100.f, 100.f, 100.f, 100.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 2.f, 16.f, 48.f, 0.f, 0.f, 0.f, 5.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 48.f, 0.f, 0.f, 0.f});
   addProgram("BoT Freeze Hold", {80.f, 48.f, 0.f, 8.f, 400.f, 100.f, 100.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 25.f, 0.f, 0.f, 0.f, 0.8f, 50.f, 1.f, 80.f, 0.f, 0.f, 80.f, 100.f, -80.f, 12.f, 18.f, 0.f, 0.f, 0.f, 100.f, 100.f, 100.f, 100.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 2.f, 16.f, 48.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 48.f, 0.f, 0.f, 0.f});
   addProgram("BoT Radio Robot", {80.f, 48.f, 0.f, 4.f, 95.f, 100.f, 100.f, 70.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 100.f, 0.f, 1.f, 5.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 38.f, 0.f, 38.f, 0.f, 0.8f, 50.f, 3.f, 100.f, 0.f, 0.f, 80.f, 100.f, -80.f, 12.f, 18.f, 0.f, 0.f, 0.f, 100.f, 100.f, 100.f, 100.f, 45.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 2.f, 16.f, 48.f, 0.f, 0.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 48.f, 0.f, 0.f, 0.f});

 }
 void prepare(double sr,int)override{sr_=float(sr);voc_.prepare(sr_);post_.prepare(sr_);midiHeld_.fill(false);midiActive_=-1;}
 void reset()override{voc_.prepare(sr_);post_.prepare(sr_);midiHeld_.fill(false);midiActive_=-1;}
 void process(mpc::ProcessContext& ctx)override{
   float level=get(Level)*.01f,wet=get(Mix)*.01f,gain=get(Input)*.01f;
   float carrier=get(Carrier)*.01f;
   const int mode=getInt(Mode)+1; // External carrier only; preserve legacy DSP branches for old projects
   const int monitor=getInt(Monitor);
   voc_.setAttackRelease(get(Attack),get(Release));
   voc_.setWet(1.f);voc_.setGate(true);voc_.setChord(getInt(Chord));
   voc_.setTone(get(Tone)*.01f);
   voc_.setWaveform(getInt(Waveform));
   voc_.setLfo(getInt(LfoShape),get(LfoRate),get(LfoDepth));
   voc_.setOsc2(getInt(Osc2Wave),get(Osc2Level)*.01f,get(Osc2Semi),get(Osc2Detune));
   voc_.setCarrierAdvanced(getInt(UnisonVoices)+1,get(UnisonSpread),getInt(Osc3Wave),
      get(Osc3Level)*.01f,get(Osc3Semi),getInt(FilterMode),get(FilterEnvAmount)*.01f);
   voc_.setCarrierDesign(get(PulseWidth)*.01f,get(PwmDepth)*.01f,get(PwmRate),
      get(OscBlend)*.01f,get(CarrierDrive)*.01f,get(NoiseColor)*.01f);
   const int voiceStyle=getInt(VoiceDesigner);
   voc_.setSpectralFreeze(getInt(SpectralFreeze)==1);
   voc_.setSibilance(std::max(get(Sibilance)*.01f,voiceStyle==3?.55f:0.f));
   voc_.setVoiceProcessing(get(VoiceSensitivity)*.01f,get(SpectralSmooth)*.01f,get(BandSpread)*.01f,
       get(SpectralTilt)*.02f,get(CarrierBrightness)*.02f,get(LowBandTime)*.01f,
       get(HighBandTime)*.01f,get(DynamicRange)*.01f,get(VoiceGate),get(UnvoicedMix)*.01f);
   voc_.setDetune(get(Detune));voc_.setSubLevel(get(Sub)*.01f);
   voc_.setNoiseLevel(get(Noise)*.01f);voc_.setOctave(int(std::round(get(Octave))));
   voc_.setCarrierFilter(get(Cutoff)*.01f,get(Resonance)*.01f);
   voc_.setSynthEnvelope(get(SynthAttack),get(SynthRelease));
   voc_.setFrequency(440.f*std::pow(2.f,(get(Pitch)-69.f)/12.f));
   // Non-destructive band profile: multiply the stored manual band values
   // at render time; never overwrite saved parameters or host preset state.
   static constexpr float profiles[4][24]={
    {1.18f,1.17f,1.16f,1.14f,1.12f,1.10f,1.08f,1.06f,1.04f,1.02f,1.f,.98f,.96f,.95f,.94f,.92f,.90f,.88f,.86f,.84f,.82f,.80f,.78f,.76f},
    {.78f,.80f,.82f,.85f,.88f,.93f,.98f,1.05f,1.12f,1.20f,1.26f,1.30f,1.32f,1.30f,1.25f,1.18f,1.12f,1.08f,1.05f,1.02f,1.f,.98f,.95f,.92f},
    {.35f,.40f,.45f,.50f,.55f,.65f,.80f,1.f,1.20f,1.30f,1.40f,1.45f,1.45f,1.40f,1.30f,1.15f,1.f,.85f,.70f,.60f,.50f,.40f,.35f,.30f},
    {.68f,.72f,.76f,.80f,.84f,.90f,.96f,1.f,1.04f,1.08f,1.12f,1.16f,1.18f,1.20f,1.20f,1.18f,1.16f,1.14f,1.12f,1.10f,1.06f,1.02f,.98f,.94f}
   };
   const int profile=getInt(BandProfile);
   const float morph=get(ProfileAmount)*.01f;
   const float styleBand=voiceStyle==4?.65f:voiceStyle==5?.25f:0.f;
   const float character=get(VoiceCharacter)*.01f;
   // Character gradually lowers the formants and strengthens a mid-band tilt.
   const float styleFormant=voiceStyle==2?-7.f:voiceStyle==4?-3.f:voiceStyle==5?3.f:0.f;
   voc_.setFormant(get(FormantShift)-3.f*character+styleFormant);
   for(int i=0;i<24;++i){
     float band=get(Band01+i)*.01f;
     if(profile>=1&&profile<=4)band*=1.f+morph*(profiles[profile-1][i]-1.f);
     band*=1.f+character*(i<7?-.14f:(i<17?.12f:-.10f));
     if(styleBand>0.f)band*=1.f+styleBand*(i<7?-.55f:(i<17?.25f:-.4f));
     voc_.setBandLevel(i,band);
   }
   // Some MPC instrument hosts may provide zero audio inputs despite the
   // declared stereo input bus. Never dereference absent buffers.
   const float* l=(ctx.numInputs>0 && ctx.inputs)?ctx.inputs[0]:nullptr;
   const float* r=(ctx.numInputs>1 && ctx.inputs)?ctx.inputs[1]:l;
   float* ol=ctx.outputs[0];float* orr=ctx.numOutputs>1?ctx.outputs[1]:ol;
   int midiIndex=0;
   int noteMode=getInt(MidiCarrierMode);
#ifdef VOcKIT_HYBRID
   // Hybrid is a true instrument: prefer MIDI notes for its internal carrier.
   // Existing FX keeps its user-selected carrier mode.
   if(noteMode==0)noteMode=1;
#endif
   const bool seqEnabled=getInt(SeqEnable)==1 && mode==0 && ctx.transport.valid && ctx.transport.ppqValid && ctx.transport.playing;
   const int seqLen=std::max(1,std::min(16,int(std::round(get(SeqLength)))));
   const double stepsPerQuarter=std::pow(2.0,double(getInt(SeqDivision)));
   const double bpm=ctx.transport.tempoValid?ctx.transport.bpm:120.0;
   const double ppqAdvance=(bpm>0.0?bpm:120.0)/(60.0*double(sr_));
   const bool midiMode=noteMode!=0 && mode==0;
   int keyNote=-1;
   for(int k=0;k<12;++k)if(getInt(Key01+k)==1)keyNote=12*(getInt(KeyOctave)+1)+k;
   const bool keyRecord=getInt(KeyRecordArm)==1 && ctx.transport.valid && ctx.transport.recording &&
      ctx.transport.ppqValid && mode==0;
   // Capture key-on edges to the closest sequencer step; normal plugin parameters
   // persist with this VocKit instance in the saved MPC project.
   if(keyRecord && keyNote>=0 && keyNote!=lastRecordedKey_){
     const double pos=std::max(0.0,ctx.transport.ppqPos*stepsPerQuarter);
     const int step=int(std::floor(pos+0.5))%seqLen;
     set(SeqStep01+step,float(keyNote));
   }
   lastRecordedKey_=keyNote;
   const bool keyEnabled=keyNote>=0 && mode==0 && (!seqEnabled || keyRecord);
   const bool pilotEnabled=getInt(PadAudioMode)==1 && mode==0 && !seqEnabled && !keyEnabled;
   for(int n=0;n<ctx.numFrames;++n){
     if(keyEnabled){
       voc_.setGate(true);
       voc_.setFrequency(440.f*std::pow(2.f,(float(keyNote)-69.f)/12.f));
     }else if(seqEnabled){
       const double ppq=ctx.transport.ppqPos+double(n)*ppqAdvance;
       const double position=std::max(0.0,ppq*stepsPerQuarter);
       const int step=int(std::floor(position))%seqLen;
       const int note=int(std::round(get(SeqStep01+step)));
       const double phase=position-std::floor(position);
       voc_.setGate(note>=36 && phase<.85);
       if(note>=36)voc_.setFrequency(440.f*std::pow(2.f,(float(note)-69.f)/12.f));
     }else if(pilotEnabled){
       // Right channel carries a dedicated control tone; left remains live mic.
       // Only four well-separated pilot frequencies are recognized in v0.7.2.
       const float sample=ctx.numInputs>1?r[n]:0.f;
       pilotRing_[pilotWrite_]=sample;
       pilotWrite_=(pilotWrite_+1)&511;
       if(++pilotCounter_>=128){pilotCounter_=0;detectPilot();}
       voc_.setGate(pilotActive_>=0);
       if(pilotActive_>=0){
         const float note=get(PadBaseNote)+float(pilotActive_);
         voc_.setFrequency(440.f*std::pow(2.f,(note-69.f)/12.f));
       }
     }else if(midiMode){
       while(midiIndex<ctx.numMidiEvents && ctx.midi && ctx.midi[midiIndex].sampleOffset<=n){
         const auto& e=ctx.midi[midiIndex++];
         if(e.isAllNotesOff()){midiHeld_.fill(false);midiActive_=-1;}
         else if(e.isNoteOn() && e.note()>=0 && e.note()<128){midiHeld_[e.note()]=true;midiActive_=e.note();}
         else if(e.isNoteOff() && e.note()>=0 && e.note()<128){
           midiHeld_[e.note()]=false;
           if(midiActive_==e.note()){
             midiActive_=-1;for(int k=127;k>=0;--k)if(midiHeld_[k]){midiActive_=k;break;}
           }
         }
       }
       voc_.setGate(midiActive_>=0);
       if(midiActive_>=0)voc_.setFrequency(440.f*std::pow(2.f,(float(midiActive_)-69.f)/12.f));
     } else if(n==0){voc_.setGate(true);midiHeld_.fill(false);midiActive_=-1;}

     float dryL=l?l[n]:0.f,dryR=r?r[n]:0.f;
     // Diagnostic tone proves MIDI delivery independently of mic/vocoder routing.
     // Only active when explicitly selected on the MIDI page.
     if(noteMode==2 && mode==0){
       float test=0.f;
       if(midiActive_>=0){
         const float hz=440.f*std::pow(2.f,(float(midiActive_)-69.f)/12.f);
         midiTestPhase_+=hz/sr_;
         if(midiTestPhase_>=1.f)midiTestPhase_-=std::floor(midiTestPhase_);
         test=.15f*std::sin(6.28318530718f*midiTestPhase_);
       }
       ol[n]=test;if(orr!=ol)orr[n]=test;
       continue;
     }
     if(monitor!=0){
       const float test=monitor==1?dryL:(monitor==2?dryR:0.5f*(dryL+dryR));
       const float sample=std::tanh(test*level);
       ol[n]=sample;if(orr!=ol)orr[n]=sample;
       continue;
     }
     float mod=(mode==0?(pilotEnabled?dryL:.5f*(dryL+dryR)):(mode==1?dryR:dryL))*gain;
     float external=(mode==1?dryL:dryR);
     float fx=(mode==0?voc_.tick(mod):voc_.tick(mod,external,true))*carrier;
     const float dry=mode==0?(pilotEnabled?dryL:.5f*(dryL+dryR)):mod;
     float left=std::tanh(((mode==0?dryL:dry)*(1.f-wet)+fx*wet)*level);
     float right=std::tanh(((mode==0?(pilotEnabled?dryL:dryR):dry)*(1.f-wet)+fx*wet)*level);
     post_.process(left,right,get(Saturation)*.01f,get(ChorusMix)*.01f,get(ChorusRate),get(ChorusDepth)*.01f,get(RingMix)*.01f,get(RingFreq));
     const float width=get(StereoWidth)*.01f;
     if(width!=1.f){const float mid=.5f*(left+right),side=.5f*(left-right)*width;
       left=std::max(-1.f,std::min(1.f,mid+side));right=std::max(-1.f,std::min(1.f,mid-side));}
     ol[n]=left;if(orr!=ol)orr[n]=right;
   }
   // Publish measured FFT bins only once per process block, never as
   // VST parameters. The future UI adapter can read this snapshot safely.
   if(monitor==0 && ctx.numFrames>0) spectrumSnapshot_.publish(voc_.spectrum());
 }
private:
 void detectPilot(){
   // Coherent tone bank: 512 samples, hop 128. Pilot tones are far above speech fundamentals.
   static constexpr float freqs[4]={750.f,1000.f,1250.f,1500.f};
   float energy=0.f;
   for(float x:pilotRing_)energy+=x*x;
   if(energy<.000025f){pilotActive_=-1;return;}
   float scores[4]={};
   for(int k=0;k<4;++k){
     const float w=6.28318530718f*freqs[k]/sr_;
     const float coeff=2.f*std::cos(w);
     float q0=0.f,q1=0.f,q2=0.f;
     for(int i=0;i<512;++i){
       q0=pilotRing_[(pilotWrite_+i)&511]+coeff*q1-q2;
       q2=q1;q1=q0;
     }
     scores[k]=q1*q1+q2*q2-coeff*q1*q2;
   }
   int best=0;
   for(int k=1;k<4;++k)if(scores[k]>scores[best])best=k;
   // Reject broadband noise and false positives using a dominance criterion.
   float other=0.f;
   for(int k=0;k<4;++k)if(k!=best)other+=scores[k];
   pilotActive_=(scores[best]>other*3.f && scores[best]>energy*20.f)?best:-1;
 }
 std::array<float,512> pilotRing_{};
 int pilotWrite_=0,pilotCounter_=0,pilotActive_=-1;
 vockit::Vocoder voc_;vockit::PostFx post_;
 vockit::SpectrumSnapshot spectrumSnapshot_;
 float sr_=44100.f,midiTestPhase_=0.f;
 std::array<bool,128> midiHeld_{};
 int midiActive_=-1,lastRecordedKey_=-1;
};
MPC_PLUGIN_ENTRY(VocKit)
