#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AmadeusAudioProcessorEditor::AmadeusAudioProcessorEditor (AmadeusAudioProcessor& p, juce::AudioTransportSource* transportSource)
    : AudioProcessorEditor (&p), audioProcessor (p), state (Stopped), thumbnailCache (5),
        thumbnailComp (512, formatManager, thumbnailCache), positionOverlay (transportSource)
{
  Component::addAndMakeVisible (&openButton);
  openButton.setButtonText ("Open...");
  openButton.onClick = [this] { openButtonClicked(); };

  addAndMakeVisible (&playButton);
  playButton.setButtonText ("Play");
  playButton.onClick = [this] { playButtonClicked(); };
  playButton.setColour (juce::TextButton::buttonColourId, juce::Colours::green);
  playButton.setEnabled (false);

  addAndMakeVisible (&stopButton);
  stopButton.setButtonText ("Stop");
  stopButton.onClick = [this] { stopButtonClicked(); };
  stopButton.setColour (juce::TextButton::buttonColourId, juce::Colours::red);
  stopButton.setEnabled (false);

  addAndMakeVisible (&uploadButton);
  uploadButton.setButtonText ("Upload");
  uploadButton.onClick = [this] { uploadButtonClicked(); };
  uploadButton.setColour (juce::TextButton::buttonColourId, juce::Colours::blue);
  uploadButton.setEnabled (false);
  
  addAndMakeVisible (&thumbnailComp);
  addAndMakeVisible (&positionOverlay);

  setSize (600, 400);

  formatManager.registerBasicFormats();
}

AmadeusAudioProcessorEditor::~AmadeusAudioProcessorEditor()
{
    // shutdownAudio();
}

//==============================================================================
void AmadeusAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Welcome to Amadeus!", getLocalBounds(), juce::Justification::centred, 1);
}

void AmadeusAudioProcessorEditor::resized()
{
    openButton.setBounds (10, 10, getWidth() - 20, 20);
    playButton.setBounds (10, 40, getWidth() - 20, 20);
    stopButton.setBounds (10, 70, getWidth() - 20, 20);
    uploadButton.setBounds (10, 100, getWidth() - 20, 20);

    juce::Rectangle<int> thumbnailBounds (10, 130, getWidth() - 20, getHeight() - 120);
    thumbnailComp.setBounds (thumbnailBounds);
    positionOverlay.setBounds (thumbnailBounds);
}

void AmadeusAudioProcessorEditor::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (source == transportSource)
        transportSourceChanged();
}
