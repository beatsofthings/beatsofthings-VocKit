#include "PluginEditor.h"
#include <cmath>
VocKitEditor::VocKitEditor(VocKitProcessor& p):juce::AudioProcessorEditor(&p),processor(p){
 setSize(1050,590);rebuild();startTimerHz(20);
}
VocKitEditor::~VocKitEditor(){stopTimer();}
void VocKitEditor::rebuild(){
 controls.clear();
 auto add=[this](juce::String id,juce::String label,float x,float y,float w,float h){
  controls.push_back({std::move(id),std::move(label),{x,y,w,h}});
 };
 if(page==0){
  for(int i=0;i<24;++i)add("band"+juce::String(i+1),juce::String(i+1).paddedLeft('0',2),
    25.f+i*41.6f,135.f,32.f,290.f);
  const char* ids[]={"mix","input","carrier","attack","release","level"};
  const char* names[]={"WET / DRY","VOICE INPUT","CARRIER","ATTACK","RELEASE","OUTPUT"};
  for(int i=0;i<6;++i)add(ids[i],names[i],30.f+i*170.f,454.f,150.f,50.f);
 }else if(page==1){
  const char* ids[]={"pitch","octave","tone","detune","sub","noise","cutoff","resonance","envAttack","envRelease","carrier","level"};
  const char* names[]={"PITCH (MIDI)","OCTAVE","SAW / PULSE","DETUNE (CENTS)","SUB OSC","NOISE OSC","FILTER CUTOFF","RESONANCE","SYNTH ATTACK","SYNTH RELEASE","CARRIER GAIN","OUTPUT"};
  for(int i=0;i<12;++i)add(ids[i],names[i],35.f+(i%4)*250.f,150.f+(i/4)*118.f,215.f,87.f);
 }else{
  add("mode","CARRIER MODE",35.f,150.f,970.f,95.f);
  add("input","VOICE INPUT",35.f,290.f,300.f,95.f);
  add("carrier","CARRIER GAIN",370.f,290.f,300.f,95.f);
  add("mix","WET / DRY",705.f,290.f,300.f,95.f);
 }
}
void VocKitEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour::fromRGB(16,25,35));
 g.setColour(juce::Colours::white);
 g.setFont(juce::Font(juce::FontOptions(28.f,juce::Font::bold)));
 g.drawText("VOCKIT  /  SPECTRAL VOCODER",24,12,740,45,juce::Justification::centredLeft);
 const char* tabs[]={"VOCODER - 24 BANDS","INTERNAL SYNTH","ROUTING"};
 for(int i=0;i<3;++i){
  auto rect=juce::Rectangle<float>(24.f+i*335.f,72.f,320.f,42.f);
  g.setColour(i==page?juce::Colour::fromRGB(27,133,155):juce::Colour::fromRGB(34,49,64));
  g.fillRoundedRectangle(rect,8.f);
  g.setColour(juce::Colours::white);g.setFont(juce::Font(juce::FontOptions(15.f,juce::Font::bold)));
  g.drawText(tabs[i],rect.toNearestInt(),juce::Justification::centred);
 }
 for(const auto& ctrl:controls){
  auto* p=processor.params.getParameter(ctrl.id);
  if(!p)continue;
  const float v=p->getValue();
  const bool band=ctrl.id.startsWith("band");
  g.setColour(juce::Colour::fromRGB(31,45,60));g.fillRoundedRectangle(ctrl.rect,7.f);
  g.setColour(juce::Colours::lightgrey);g.setFont(juce::Font(juce::FontOptions(band?12.f:14.f)));
  if(band){
   const auto rail=juce::Rectangle<float>(ctrl.rect.getCentreX()-4.f,ctrl.rect.getY()+18.f,8.f,ctrl.rect.getHeight()-58.f);
   g.setColour(juce::Colour::fromRGB(54,74,90));g.fillRoundedRectangle(rail,4.f);
   const float fill=rail.getHeight()*v;
   g.setColour(juce::Colour::fromRGB(38,202,205));
   g.fillRoundedRectangle(rail.getX(),rail.getBottom()-fill,rail.getWidth(),fill,3.f);
   g.setColour(juce::Colours::white);
   g.drawText(ctrl.label,juce::Rectangle<int>(int(ctrl.rect.getX()),int(ctrl.rect.getBottom()-30),int(ctrl.rect.getWidth()),25),juce::Justification::centred);
  }else{
   g.drawText(ctrl.label,juce::Rectangle<int>(int(ctrl.rect.getX()+8),int(ctrl.rect.getY()+7),int(ctrl.rect.getWidth()-16),24),juce::Justification::centredLeft);
   const float x=ctrl.rect.getX()+12.f,y=ctrl.rect.getBottom()-25.f,w=ctrl.rect.getWidth()-24.f;
   g.setColour(juce::Colour::fromRGB(66,86,101));g.fillRoundedRectangle(x,y,w,9.f,4.f);
   g.setColour(juce::Colour::fromRGB(38,202,205));g.fillRoundedRectangle(x,y,std::max(1.f,w*v),9.f,4.f);
   g.setColour(juce::Colours::white);
   const float actual=p->convertFrom0to1(v);
   const juce::String value=ctrl.id=="mode"?p->getCurrentValueAsText():juce::String(actual,1);
   g.drawText(value,int(x),int(y-24),int(w),22,juce::Justification::centredRight);
  }
 }
 if(page==2){
  g.setColour(juce::Colours::lightgrey);g.setFont(juce::Font(juce::FontOptions(15.f)));
  g.drawFittedText("PRESETS (use MPC host preset menu above): Retro Robot | JURA Carrier L / Voice R | JURA Carrier R / Voice L",30,393,990,24,juce::Justification::centredLeft,1);
  g.drawFittedText("INTERNAL: VocKit on voice track. MIC: Audio track, input monitoring ON, headphones.",30,420,990,24,juce::Justification::centredLeft,1);
  g.drawFittedText("JURA: JURA -> Submix 1 pan LEFT; Voice -> Submix 1 pan RIGHT; VocKit only on Submix 1.",30,448,990,24,juce::Justification::centredLeft,1);
  g.drawFittedText("Set Carrier Mode to L Carrier / R Voice. Verify stereo separation before the effect.",30,475,990,24,juce::Justification::centredLeft,1);
  g.drawFittedText("TIPS: bright saw carrier, no reverb before VocKit; 100% Wet for testing; shorter release for fast rap.",30,502,990,24,juce::Justification::centredLeft,1);
  g.drawFittedText("Guides, updates and tools: https://mpctoolkit.com",30,529,990,24,juce::Justification::centredLeft,1);
 }
}
void VocKitEditor::setControl(int index,float normalized){
 if(index<0||index>=int(controls.size()))return;
 if(auto* p=processor.params.getParameter(controls[size_t(index)].id))
  p->setValueNotifyingHost(juce::jlimit(0.f,1.f,normalized));
 repaint();
}
void VocKitEditor::mouseDown(const juce::MouseEvent& e){
 const auto pt=e.position;
 if(pt.y>=72.f&&pt.y<114.f){
  for(int i=0;i<3;++i)if(pt.x>=24.f+i*335.f&&pt.x<344.f+i*335.f){page=i;rebuild();repaint();return;}
 }
 for(int i=0;i<int(controls.size());++i)if(controls[size_t(i)].rect.contains(pt)){
  active=i;
  if(auto* p=processor.params.getParameter(controls[size_t(i)].id)){
   p->beginChangeGesture();
   dragStart=p->getValue();
   if(controls[size_t(i)].id=="mode"){
    setControl(i,std::round(3.f*pt.x/float(getWidth()))/3.f);
   }else mouseDrag(e);
  }
  return;
 }
}
void VocKitEditor::mouseDrag(const juce::MouseEvent& e){
 if(active<0)return;
 const auto& c=controls[size_t(active)];
 const float v=c.id.startsWith("band")?
  (c.rect.getBottom()-e.position.y)/c.rect.getHeight():
  (e.position.x-c.rect.getX())/c.rect.getWidth();
 setControl(active,v);
}
void VocKitEditor::mouseUp(const juce::MouseEvent&){
 if(active>=0)if(auto* p=processor.params.getParameter(controls[size_t(active)].id))p->endChangeGesture();
 active=-1;
}
