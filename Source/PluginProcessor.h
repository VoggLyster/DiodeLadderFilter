/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class VCS3FilterAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    VCS3FilterAudioProcessor();
    ~VCS3FilterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    AudioProcessorValueTreeState parameters;
    std::atomic<float>* biasParameter = nullptr;
    std::atomic<float>* gainParameter = nullptr;
    double eta = 1.836f;
    double VT = 0.0260f;
    double gamma = eta * VT;
    double C = 1.0e-7f;
    double Mp = 1.0e-4f;
    
    double VC1, VC2, VC3, VC4;
    double u1, u2, u3, u4, u5;
    double s1, s2, s3, s4;
    double Vin;
    double Vout;
    double VoutPrev;
    double Fs;
    double inputFs;

    int iteration = 0;
    int maxNrIterations = 100;

    std::unique_ptr<dsp::Oversampling<float>> oversampling;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VCS3FilterAudioProcessor)
};
