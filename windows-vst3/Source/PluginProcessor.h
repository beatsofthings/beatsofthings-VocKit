#pragma once
#include <JuceHeader.h>
#include "Vocoder.hpp"
class VocKitProcessor final : public juce::AudioProcessor {
public:
 VocKitProcessor();
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override {return true;}
 const juce::String getName() const override {return "VocKit FX";}
 bool acceptsMidi() const override {return false;}
 bool producesMidi() const override {return false;}
 bool isMidiEffect() const override {return false;}
 double getTailLengthSeconds() const override {return 0.5;}
 int getNumPrograms() override {return 10;}
 int getCurrentProgram() override {return currentProgram;}
 void setCurrentProgram(int) override;
 const juce::String getProgramName(int) override;
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState params;
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout makeParams();
 vockit::Vocoder voc;
 int currentProgram=0;
 void applyPreset(int);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocKitProcessor)
};
