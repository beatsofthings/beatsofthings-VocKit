#include "PluginEditor.h"
VocKitEditor::VocKitEditor(VocKitProcessor& p):AudioProcessorEditor(&p),processor(p){
 setSize(520,260);
 modeLabel.setText("CARRIER MODE",juce::dontSendNotification);
 addAndMakeVisible(modeLabel);
 mode.addItemList({"Internal","External Sidechain (JURA)","Left Carrier / Right Voice","Right Carrier / Left Voice"},1);
 addAndMakeVisible(mode);
 modeAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.params,"mode",mode);
}
void VocKitEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour::fromRGB(23,31,41));
 g.setColour(juce::Colours::white);
 g.setFont(juce::Font(juce::FontOptions(23.f,juce::Font::bold)));
 g.drawText("VocKit FX - UI DIAGNOSTIC",20,15,480,45,juce::Justification::centredLeft);
 g.setFont(juce::Font(juce::FontOptions(15.f)));
 g.drawText("Minimal editor test. 24-band DSP remains active.",20,170,480,35,juce::Justification::centredLeft);
}
void VocKitEditor::resized(){
 modeLabel.setBounds(20,77,200,26);
 mode.setBounds(20,108,470,36);
}
