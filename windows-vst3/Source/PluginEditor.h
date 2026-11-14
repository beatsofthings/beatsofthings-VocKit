#pragma once
#include "PluginProcessor.h"
#include <array>
class VocKitEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
 explicit VocKitEditor(VocKitProcessor&);
 ~VocKitEditor() override;
 void paint(juce::Graphics&) override;
 void resized() override {}
 void mouseDown(const juce::MouseEvent&) override;
 void mouseDrag(const juce::MouseEvent&) override;
 void mouseUp(const juce::MouseEvent&) override;
private:
 struct Control {juce::String id,label;juce::Rectangle<float> rect;};
 VocKitProcessor& processor;
 std::vector<Control> controls;
 int page=0,active=-1;
 float dragStart=0.f;
 void rebuild();
 void timerCallback() override {repaint();}
 void setControl(int index,float normalized);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocKitEditor)
};
