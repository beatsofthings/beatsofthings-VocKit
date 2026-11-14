#include "PluginEditor.h"
VocKitEditor::VocKitEditor(VocKitProcessor& p):AudioProcessorEditor(&p),processor(p){
 setSize(1100,330);
 modeLabel.setText("CARRIER SOURCE",juce::dontSendNotification);
 modeLabel.setColour(juce::Label::textColourId,juce::Colours::lightcyan);
 addAndMakeVisible(modeLabel);
 mode.addItemList({"Internal","External Sidechain (JURA)","Left Carrier / Right Voice","Right Carrier / Left Voice"},1);
 addAndMakeVisible(mode);
 modeAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.params,"mode",mode);
 for(int i=0;i<0;++i){
  auto* s=bands.add(new juce::Slider());
  s->setSliderStyle(juce::Slider::LinearVertical);
  s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,38,18);
  s->setColour(juce::Slider::trackColourId,juce::Colours::turquoise);
  s->setColour(juce::Slider::thumbColourId,juce::Colours::white);
  addAndMakeVisible(s);
  attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.params,"band"+juce::String(i+1),*s));
  auto* label=labels.add(new juce::Label());
  label->setText(juce::String(i+1).paddedLeft('0',2),juce::dontSendNotification);
  label->setJustificationType(juce::Justification::centred);
  label->setColour(juce::Label::textColourId,juce::Colours::lightgrey);
  addAndMakeVisible(label);
 }
 const char* ids[]={"level","input","carrier","mix","attack","release","pitch","tone"};
 const char* names[]={"OUTPUT","VOICE INPUT","CARRIER GAIN","WET / DRY","ATTACK","RELEASE","INT PITCH","SAW / PULSE"};
 for(int i=0;i<8;++i){
  auto* s=knobs.add(new juce::Slider());
  s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,76,20);
  s->setColour(juce::Slider::rotarySliderFillColourId,juce::Colours::turquoise);
  addAndMakeVisible(s);
  attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.params,ids[i],*s));
  auto* label=knobLabels.add(new juce::Label());
  label->setText(names[i],juce::dontSendNotification);
  label->setJustificationType(juce::Justification::centred);
  label->setColour(juce::Label::textColourId,juce::Colours::lightgrey);
  addAndMakeVisible(label);
 }
}
void VocKitEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour::fromRGB(23,31,41));
 g.setColour(juce::Colours::white);
 g.setFont(juce::Font(juce::FontOptions(27.f,juce::Font::bold)));
 g.drawText("VOCKIT FX / UI TEST: MAIN CONTROLS",24,12,700,45,juce::Justification::centredLeft);
 g.setColour(juce::Colours::lightgrey);
 g.setFont(juce::Font(juce::FontOptions(14.f)));
 g.drawText("INTERNAL CARRIER / EXTERNAL SIDECHAIN / DUAL MONO",24,59,700,26,juce::Justification::centredLeft);
 g.setColour(juce::Colour::fromRGB(34,47,58));
 g.fillRoundedRectangle(18.f,128.f,1060.f,185.f,12.f);
}
void VocKitEditor::resized(){
 modeLabel.setBounds(772,19,295,25);
 mode.setBounds(772,51,295,36);
 for(int i=0;i<0;++i){
  const int x=28+i*43;
  labels[i]->setBounds(x,143,38,24);
  bands[i]->setBounds(x,169,38,245);
 }
 for(int i=0;i<8;++i){
  const int x=27+i*132;
  knobLabels[i]->setBounds(x,140,124,24);
  knobs[i]->setBounds(x+15,169,94,125);
 }
}
