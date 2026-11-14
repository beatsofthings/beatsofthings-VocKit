#pragma once
#include "PluginProcessor.h"
class VocKitEditor final : public juce::AudioProcessorEditor {
public:
 explicit VocKitEditor(VocKitProcessor&);
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 VocKitProcessor& processor;
 juce::ComboBox mode;
 juce::Label modeLabel;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
 juce::OwnedArray<juce::Slider> bands;
 juce::OwnedArray<juce::Label> labels;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
 juce::OwnedArray<juce::Slider> knobs;
 juce::OwnedArray<juce::Label> knobLabels;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocKitEditor)
};
