#include "PluginEditor.h"
VocKitEditor::VocKitEditor(VocKitProcessor& p):AudioProcessorEditor(&p),processor(p){
 setSize(1080,590);
 modeLabel.setText("CARRIER SOURCE",juce::dontSendNotification);
 modeLabel.setColour(juce::Label::textColourId,juce::Colours::lightcyan);
 addAndMakeVisible(modeLabel);
 mode.addItemList({"Internal","External Sidechain (JURA)","Left Carrier / Right Voice","Right Carrier / Left Voice"},1);
 addAndMakeVisible(mode);
 modeAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.params,"mode",mode);
 for(int i=0;i<24;++i){
  auto* s=bands.add(new juce::Slider());
  s->setSliderStyle(juce::Slider::LinearVertical);
  s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,36,19);
  s->setColour(juce::Slider::trackColourId,i<12?juce::Colours::turquoise:juce::Colours::orange);
  s->setColour(juce::Slider::thumbColourId,juce::Colours::white);
  addAndMakeVisible(s);
  attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.params,"band"+juce::String(i+1),*s));
  auto* label=labels.add(new juce::Label());
  label->setText(juce::String(i+1).paddedLeft('0',2),juce::dontSendNotification);
  label->setJustificationType(juce::Justification::centred);
  label->setColour(juce::Label::textColourId,juce::Colours::lightgrey);
  addAndMakeVisible(label);
 }
 for(const auto& spec:std::initializer_list<std::pair<const char*,const char*>>{
   {"level","OUTPUT"},{"input","VOICE INPUT"},{"carrier","CARRIER GAIN"},{"mix","WET / DRY"},
   {"attack","ATTACK"},{"release","RELEASE"},{"pitch","INT PITCH"},{"tone","SAW / PULSE"}}){
  auto* s=knobs.add(new juce::Slider());
  s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,68,18);
  s->setColour(juce::Slider::rotarySliderFillColourId,juce::Colours::turquoise);
  addAndMakeVisible(s);
  attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.params,spec.first,*s));
  auto* label=knobLabels.add(new juce::Label());
  label->setText(spec.second,juce::dontSendNotification);
  label->setJustificationType(juce::Justification::centred);
  label->setColour(juce::Label::textColourId,juce::Colours::lightgrey);
  addAndMakeVisible(label);
 }
}
void VocKitEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour::fromRGB(18,29,39));
 g.setColour(juce::Colours::turquoise);
 g.setFont(juce::Font(juce::FontOptions(31.f,juce::Font::bold)));
 g.drawText("VOCKIT  /  24-BAND VOCODER",24,12,720,48,juce::Justification::centredLeft);
 g.setColour(juce::Colours::grey);
 g.setFont(juce::Font(juce::FontOptions(16.f)));
 g.drawText("EXTERNAL CARRIER  •  SIDECHAIN OR DUAL-MONO",24,59,750,28,juce::Justification::centredLeft);
 g.setColour(juce::Colour::fromRGB(31,46,59));
 g.fillRoundedRectangle(17.f,122.f,1046.f,304.f,12.f);
 g.setColour(juce::Colours::lightcyan);
 g.drawText("LOW",27,130,100,25,juce::Justification::left);
 g.drawText("HIGH",940,130,100,25,juce::Justification::right);
}
void VocKitEditor::resized(){
 modeLabel.setBounds(770,18,280,26);
 mode.setBounds(770,50,278,36);
 for(int i=0;i<24;++i){
  const int x=30+i*43;
  labels[i]->setBounds(x,158,37,24);
  bands[i]->setBounds(x,184,37,220);
 }
 for(int i=0;i<knobs.size();++i){
  const int x=23+i*130;
  knobLabels[i]->setBounds(x,445,123,24);
  knobs[i]->setBounds(x+15,470,95,101);
 }
}
