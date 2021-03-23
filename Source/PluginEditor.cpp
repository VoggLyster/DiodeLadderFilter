/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VCS3FilterAudioProcessorEditor::VCS3FilterAudioProcessorEditor (VCS3FilterAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState (vts)
{
    biasLabel.setText("Bias", juce::dontSendNotification);
    addAndMakeVisible(biasLabel);
    addAndMakeVisible(biasSlider);
    biasAttachment.reset(new SliderAttachment(valueTreeState, "bias", biasSlider));

    gainLabel.setText("Gain", juce::dontSendNotification);
    addAndMakeVisible(gainLabel);
    addAndMakeVisible(gainSlider);
    gainAttachment.reset(new SliderAttachment(valueTreeState, "gain", gainSlider));

    setSize (400, 300);
}

VCS3FilterAudioProcessorEditor::~VCS3FilterAudioProcessorEditor()
{
}

//==============================================================================
void VCS3FilterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void VCS3FilterAudioProcessorEditor::resized()
{
    int border = 20;
    int w = getWidth() - 2 * border;
    biasLabel.setBounds(border, border, w, labelHeight);
    biasSlider.setBounds(border, border + labelHeight, w, sliderHeight);
    gainLabel.setBounds(border, border * 2 + labelHeight + sliderHeight, w, labelHeight);
    gainSlider.setBounds(border, border * 2 + labelHeight * 2 + sliderHeight, w, sliderHeight);
}
