#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AmadeusAudioProcessor::AmadeusAudioProcessor()
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
{
}

AmadeusAudioProcessor::~AmadeusAudioProcessor()
{
}

//==============================================================================
const juce::String AmadeusAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AmadeusAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AmadeusAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AmadeusAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AmadeusAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AmadeusAudioProcessor::getNumPrograms()
{
    return 1;   
}

int AmadeusAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AmadeusAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AmadeusAudioProcessor::getProgramName (int index)
{
    return {};
}

void AmadeusAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AmadeusAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    if (transportSource != nullptr)
        transportSource->prepareToPlay (samplesPerBlock, sampleRate);
}

void AmadeusAudioProcessor::releaseResources()
{
    if (transportSource != nullptr)
        transportSource->releaseResources();
}

void AmadeusAudioProcessor::processBlock (juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer& middiBuffer)
{
    if (transportSource != nullptr)
    {
        juce::AudioSourceChannelInfo audioSourceChannelInfo = juce::AudioSourceChannelInfo(audioBuffer);
        transportSource->getNextAudioBlock (audioSourceChannelInfo);
    }
}


#ifndef JucePlugin_PreferredChannelConfigurations
bool AmadeusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

//==============================================================================
bool AmadeusAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AmadeusAudioProcessor::createEditor()
{
    return new AmadeusAudioProcessorEditor (*this, transportSource);
}

//==============================================================================
void AmadeusAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AmadeusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AmadeusAudioProcessor();
}
