#include "PluginEditor.h"
VocKitEditor::VocKitEditor(VocKitProcessor& p):AudioProcessorEditor(&p),processor(p){
 setSize(520,300);
 modeLabel.setText("CARRIER MODE",juce::dontSendNotification);
 addAndMakeVisible(modeLabel);
 mode.addItemList({"Internal","External Sidechain (JURA)","Left Carrier / Right Voice","Right Carrier / Left Voice"},1);
 addAndMakeVisible(mode);
 modeAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.params,"mode",mode);
 auto* wet=knobs.add(new juce::Slider());
 wet->setSliderStyle(juce::Slider::LinearHorizontal);
 wet->setTextBoxStyle(juce::Slider::TextBoxRight,false,72,22);
 addAndMakeVisible(wet);
 wet->setRange(0.0,1.0,0.01);
 wet->setValue(1.0,juce::dontSendNotification);
 // Deliberately no APVTS SliderAttachment: isolate host UI crash.
 auto* wetLabel=knobLabels.add(new juce::Label());
 wetLabel->setText("WET / DRY",juce::dontSendNotification);
 addAndMakeVisible(wetLabel);
}
void VocKitEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour::fromRGB(23,31,41));
 g.setColour(juce::Colours::white);
 g.setFont(juce::Font(juce::FontOptions(23.f,juce::Font::bold)));
 g.drawText("VocKit FX - UNBOUND SLIDER TEST",20,15,480,45,juce::Justification::centredLeft);
 g.setFont(juce::Font(juce::FontOptions(14.f)));
 g.drawText("Slider has NO parameter binding; UI diagnostic only.",20,240,480,30,juce::Justification::centredLeft);
}
void VocKitEditor::resized(){
 modeLabel.setBounds(20,72,200,26);
 mode.setBounds(20,102,470,36);
 knobLabels[0]->setBounds(20,155,130,28);
 knobs[0]->setBounds(155,155,335,32);
}
