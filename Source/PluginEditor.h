/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

class VCS3FilterAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    VCS3FilterAudioProcessorEditor (VCS3FilterAudioProcessor&, AudioProcessorValueTreeState&);
    ~VCS3FilterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AudioProcessorValueTreeState& valueTreeState;

    Label biasLabel;
    Slider biasSlider;
    std::unique_ptr<SliderAttachment> biasAttachment;

    Label gainLabel;
    Slider gainSlider;
    std::unique_ptr<SliderAttachment> gainAttachment;

    int labelHeight = 20;
    int sliderHeight = 30;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    VCS3FilterAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VCS3FilterAudioProcessorEditor)
};
