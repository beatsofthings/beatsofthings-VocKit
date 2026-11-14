#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
juce::AudioProcessorValueTreeState::ParameterLayout VocKitProcessor::makeParams(){
 std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
 auto add=[&](const juce::String& id,const juce::String& name,float min,float max,float def){
  p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(min,max),def));
 };
 add("level","Output Level",0,1.5f,0.8f);
 add("input","Modulator Input",0,3,1);
 add("carrier","Carrier Gain",0,3,1);
 add("mix","Wet Mix",0,1,1);
 add("pitch","Internal Pitch MIDI",36,84,48);
 add("attack","Envelope Attack ms",1,100,8);
 add("release","Envelope Release ms",10,500,90);
 add("tone","Internal Saw Pulse",0,1,0);
 add("detune","Internal Detune Cents",0,35,0);
 add("sub","Internal Sub Osc",0,1,0);
 add("noise","Internal Noise",0,1,0);
 add("octave","Internal Octave",-2,2,0);
 add("cutoff","Synth Filter Cutoff",0,1,1);
 add("resonance","Synth Resonance",0,0.95f,0);
 add("envAttack","Synth Attack ms",1,500,1);
 add("envRelease","Synth Release ms",5,2000,5);
 p.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"mode",1},"Carrier Mode",juce::StringArray{"Internal","Sidechain","Left Carrier / Right Voice","Right Carrier / Left Voice"},0));
 for(int i=0;i<24;++i)add("band"+juce::String(i+1),"Band "+juce::String(i+1),0,2,1);
 return {p.begin(),p.end()};
}
VocKitProcessor::VocKitProcessor():
 juce::AudioProcessor(BusesProperties().withInput("Voice / Modulator",juce::AudioChannelSet::stereo(),true)
  .withInput("Carrier Sidechain",juce::AudioChannelSet::stereo(),false)
  .withOutput("Output",juce::AudioChannelSet::stereo(),true)),
 params(*this,nullptr,"VocKit",makeParams()){}

const juce::String VocKitProcessor::getProgramName(int index){
 static const char* names[]={"Init","Robot Voice","Clear Rap","Dark Vocoder","Saw Brass","Soft Strings","Aggressive"};
 return index>=0&&index<7?juce::String(names[index]):juce::String{};
}
void VocKitProcessor::applyPreset(int index){
 auto set=[this](const juce::String& id,float value){
  if(auto* p=params.getParameter(id)){
   p->beginChangeGesture();
   p->setValueNotifyingHost(p->convertTo0to1(value));
   p->endChangeGesture();
  }
 };
 // Reset the entire sound to a deterministic base before applying a preset.
 set("mode",0.f);set("level",.8f);set("input",1.f);set("carrier",1.f);
 set("mix",1.f);set("pitch",48.f);set("attack",8.f);set("release",90.f);
 set("tone",0.f);set("detune",0.f);set("sub",0.f);set("noise",0.f);
 set("octave",0.f);set("cutoff",1.f);set("resonance",0.f);
 set("envAttack",1.f);set("envRelease",5.f);
 for(int i=1;i<=24;++i)set("band"+juce::String(i),1.f);
 switch(index){
 case 1: // Robot Voice
  set("pitch",48.f);set("attack",5.f);set("release",70.f);break;
 case 2: // Clear Rap
  set("pitch",55.f);set("attack",2.f);set("release",35.f);
  set("mix",.92f);break;
 case 3: // Dark Vocoder
  set("pitch",43.f);set("attack",12.f);set("release",130.f);
  set("tone",.45f);set("sub",.2f);
  for(int i=18;i<=24;++i)set("band"+juce::String(i),.7f);
  break;
 case 4: // Saw Brass
  set("pitch",48.f);set("detune",7.f);set("sub",.12f);
  set("cutoff",.95f);break;
 case 5: // Soft Strings
  set("pitch",55.f);set("tone",.35f);set("detune",3.f);
  set("sub",.08f);set("cutoff",.78f);
  set("attack",18.f);set("release",160.f);break;
 case 6: // Aggressive
  set("pitch",48.f);set("tone",.7f);set("detune",15.f);
  set("sub",.3f);set("noise",.12f);set("resonance",.2f);
  set("attack",2.f);set("release",45.f);break;
 default:break;
 }
}
void VocKitProcessor::setCurrentProgram(int index){
 if(index<0||index>=getNumPrograms())return;
 currentProgram=index;
 applyPreset(index);
 updateHostDisplay(juce::AudioProcessor::ChangeDetails{}.withProgramChanged(true));
}
bool VocKitProcessor::isBusesLayoutSupported(const BusesLayout& b) const {
 return b.getMainInputChannelSet()==juce::AudioChannelSet::stereo() &&
 b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo() &&
 (b.inputBuses.size()<2 || b.getChannelSet(true,1)==juce::AudioChannelSet::disabled() ||
 b.getChannelSet(true,1)==juce::AudioChannelSet::stereo());
}
void VocKitProcessor::prepareToPlay(double sr,int){voc.prepare(float(sr));}
void VocKitProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&){
 juce::ScopedNoDenormals guard;
 const int n=buffer.getNumSamples();
 auto val=[&](const char* id){return params.getRawParameterValue(id)->load();};
 const int mode=int(val("mode"));
 const float level=val("level"),input=val("input"),carrierGain=val("carrier"),wet=val("mix");
 voc.setAttackRelease(val("attack"),val("release"));
 voc.setTone(val("tone"));
 voc.setDetune(val("detune"));
 voc.setSubLevel(val("sub"));
 voc.setNoiseLevel(val("noise"));
 voc.setOctave(int(std::round(val("octave"))));
 voc.setCarrierFilter(val("cutoff"),val("resonance"));
 voc.setSynthEnvelope(val("envAttack"),val("envRelease"));
 voc.setGate(true);
 voc.setWet(1.f);
 voc.setFrequency(440.f*std::pow(2.f,(val("pitch")-69.f)/12.f));
 for(int i=0;i<24;++i)voc.setBandLevel(i,val(("band"+juce::String(i+1)).toRawUTF8()));
 const float* inL=buffer.getReadPointer(0);
 const float* inR=buffer.getReadPointer(1);
 const int sideStart=(getBusCount(true)>1 && getChannelCountOfBus(true,1)==2)
  ? getChannelIndexInProcessBlockBuffer(true,1,0) : -1;
 const bool sideAvailable=mode==1 && getBusCount(true)>1 &&
 getChannelCountOfBus(true,1)==2 && sideStart>=0 && sideStart+1<buffer.getNumChannels();
 const float* sideL=sideAvailable?buffer.getReadPointer(sideStart):nullptr;
 const float* sideR=sideAvailable?buffer.getReadPointer(sideStart+1):nullptr;
 float* outL=buffer.getWritePointer(0);
 float* outR=buffer.getWritePointer(1);
 for(int i=0;i<n;++i){
  const float l=inL[i],r=inR[i];
  const float mod=(mode==2?r:mode==3?l:.5f*(l+r))*input;
  const float ext=mode==2?l:mode==3?r:sideAvailable?.5f*(sideL[i]+sideR[i]):0.f;
  const float processed=(mode==0?voc.tick(mod):voc.tick(mod,ext*carrierGain,true));
  const float fx=(mode==0?processed*carrierGain:processed);
  const float dry=mode==2?r:mode==3?l:.5f*(l+r);
  outL[i]=std::tanh((dry*(1.f-wet)+fx*wet)*level);
  outR[i]=outL[i];
 }
}
void VocKitProcessor::getStateInformation(juce::MemoryBlock& dst){
 auto state=params.copyState();state.setProperty("programIndex",currentProgram,nullptr);
 auto xml=state.createXml();copyXmlToBinary(*xml,dst);
}
void VocKitProcessor::setStateInformation(const void* data,int size){
 if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName(params.state.getType())){
  auto state=juce::ValueTree::fromXml(*xml);
  currentProgram=juce::jlimit(0,getNumPrograms()-1,int(state.getProperty("programIndex",0)));
  params.replaceState(state);
 }
}
juce::AudioProcessorEditor* VocKitProcessor::createEditor(){return new VocKitEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new VocKitProcessor();}
