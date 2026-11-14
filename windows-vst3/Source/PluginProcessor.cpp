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
 p.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"mode",1},"Carrier Mode",juce::StringArray{"Internal","Sidechain","Left Carrier / Right Voice","Right Carrier / Left Voice"},0));
 for(int i=0;i<24;++i)add("band"+juce::String(i+1),"Band "+juce::String(i+1),0,2,1);
 return {p.begin(),p.end()};
}
VocKitProcessor::VocKitProcessor():
 juce::AudioProcessor(BusesProperties().withInput("Voice / Modulator",juce::AudioChannelSet::stereo(),true)
  .withInput("Carrier Sidechain",juce::AudioChannelSet::stereo(),false)
  .withOutput("Output",juce::AudioChannelSet::stereo(),true)),
 params(*this,nullptr,"VocKit",makeParams()){}
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
 auto xml=params.copyState().createXml();copyXmlToBinary(*xml,dst);
}
void VocKitProcessor::setStateInformation(const void* data,int size){
 if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName(params.state.getType()))params.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessorEditor* VocKitProcessor::createEditor(){return new juce::GenericAudioProcessorEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new VocKitProcessor();}
