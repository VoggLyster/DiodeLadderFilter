/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VCS3FilterAudioProcessor::VCS3FilterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
    : parameters (*this, nullptr, Identifier("VCS3Filter"),
        {
            std::make_unique<AudioParameterFloat>("freq", "Frequency", NormalisableRange<float>(30.0f, 20000.0f, 0.f, 0.25f), 10000.0f),
            std::make_unique<AudioParameterFloat>("gain", "Gain", 0.0f, 10.0f, 1.0f) /*Gain from 0-10 -> Modeling of the EMS VCS3 Voltage-Controlled
            Filter as a Nonlinear Filter Network sec. II A*/,
        })
{
    biasParameter = parameters.getRawParameterValue("freq");
    gainParameter = parameters.getRawParameterValue("gain");
}

VCS3FilterAudioProcessor::~VCS3FilterAudioProcessor()
{
}

//==============================================================================
const juce::String VCS3FilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VCS3FilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VCS3FilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VCS3FilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VCS3FilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VCS3FilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VCS3FilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VCS3FilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VCS3FilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void VCS3FilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VCS3FilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    VC1 = 0.0f;
    VC2 = 0.0f;
    VC3 = 0.0f;
    VC4 = 0.0f;
    u1 = 0.0f;
    u2 = 0.0f;
    u3 = 0.0f;
    u4 = 0.0f;
    u5 = 0.0f;
    s1 = 0.0f;
    s2 = 0.0f;
    s3 = 0.0f;
    s4 = 0.0f;
    Vout = 0.0f;
    VoutPrev = 0.0f;

    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = jmax(getMainBusNumInputChannels(), getMainBusNumOutputChannels());

    oversampling.reset(new dsp::Oversampling<float>(spec.numChannels, 2, dsp::Oversampling<float>::filterHalfBandFIREquiripple, false));
    oversampling->initProcessing(static_cast<size_t> (samplesPerBlock));
    Fs = getSampleRate() * 4;
    
}

void VCS3FilterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VCS3FilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void VCS3FilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    dsp::AudioBlock<float> block = dsp::AudioBlock<float>(buffer);
    dsp::AudioBlock<float> oversampledBlock = oversampling->processSamplesUp(block);
    auto* leftChannel = oversampledBlock.getChannelPointer(0);
    auto* rightChannel = oversampledBlock.getChannelPointer(1);

    auto I0 = 8 * C * VT * 2 * Fs * tan((MathConstants<float>::pi * *biasParameter) / Fs);
    float K = *gainParameter;

    for (auto n = 0; n < oversampledBlock.getNumSamples(); n++)
    {
        Vin = leftChannel[n];
        iteration = 0;
        while (1) {
            u1 = tanh((Vin - VoutPrev) / (2 * VT));
            VC1 = (I0 / (4.0 * C * Fs)) * (u2 + u1) + s1;
            u2 = tanh((VC2 - VC1) / (2 * gamma));
            VC2 = (I0 / (4.0 * C * Fs) * (u3 - u2)) + s2;
            u3 = tanh((VC3 - VC2) / (2 * gamma));
            VC3 = (I0 / (4.0 * C * Fs) * (u4 - u3)) + s3;
            u4 = tanh((VC4 - VC3) / (2 * gamma));
            VC4 = (I0 / (4.0 * C * Fs) * (-u5 - u4)) + s4;
            u5 = tanh(VC4 / (6.0f * gamma));
            Vout = (K + 0.5f) * VC4;
            if (abs(Vout - VoutPrev) >= Mp * abs(VoutPrev) || iteration > maxNrIterations)
            {
                VoutPrev = Vout;
                break;
            }
            VoutPrev = Vout;
            iteration++;
        }
        s1 = 1 / (2 * Fs) * u1 + VC1;
        s2 = 1 / (2 * Fs) * u2 + VC2;
        s3 = 1 / (2 * Fs) * u3 + VC3;
        s4 = 1 / (2 * Fs) * u4 + VC4;
        leftChannel[n] = float(Vout);
        rightChannel[n] = float(Vout);
    }

    oversampling->processSamplesDown(block);
    block.copyTo(buffer);
    
}

//==============================================================================
bool VCS3FilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VCS3FilterAudioProcessor::createEditor()
{
    return new VCS3FilterAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void VCS3FilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void VCS3FilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VCS3FilterAudioProcessor();
}
